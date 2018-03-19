/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Clause.cpp
 * Author: Zhong Jian<77367632@qq.com>
 * 
 * Created on 2017年12月22日, 下午8:41
 */

#include "Clause.h"

/*---------------------------------------------------------------------*/
/*                    Constructed Function                             */
/*---------------------------------------------------------------------*/

//

Clause::Clause()
: ident(-1), properties(ClauseProp::CPIgnoreProps), info(nullptr), literals(nullptr)
, negLitNo(0), posLitNo(0), weight(123), parent1(nullptr), parent2(nullptr) {

}

Clause::Clause(Literal* lits) : Clause() {
    //Clause_p handle = EmptyClauseAlloc();


    Literal *pos_lits = nullptr, *neg_lits = nullptr;
    Literal* *pos_append = &pos_lits;
    Literal* *neg_append = &neg_lits;
    Literal* next = nullptr;
    ident = ++Env::global_clause_counter;

    while (lits) {
        lits->claPtr = this; /*指定当前文字所在子句*/
        next = lits->next;
        if (lits->EqnIsPositive()) {
            posLitNo++;
            *pos_append = lits;
            pos_append = &((*pos_append)->next);
        } else {
            negLitNo++;
            *neg_append = lits;
            neg_append = &((*neg_append)->next);
        }
        lits = next;
    }
    *pos_append = neg_lits;
    *neg_append = nullptr;
    literals = pos_lits;
}

Clause::~Clause() {
    /* 删除子句，并且需要删除删除子句中的文字或项 */
    //assert(!setClause);
    //DelPtr(evaluations);
    Literal::EqnListFree(literals);
    // children.Destroy(); //PTreeFree( );
    DelPtr(info); //ClauseInfoFree(junk->info);
    // if (!derivation.empty()) {
    //    derivation.clear();
    //   vector<IntOrP>().swap(derivation);
    //PStackFree(junk->derivation);
}
 
/*---------------------------------------------------------------------*/
/*                  Member Function-[public]                           */
/*---------------------------------------------------------------------*/

/*****************************************************************************
 *  Function: ClausePrint(). Print a clause in the most canonical representation.
 ****************************************************************************/
void Clause::ClausePrint(FILE* out, bool fullterms) {

    /* fprintf(out, "(%ld, %ld)", clause->proof_depth,
       clause->proof_size); */

    /* #define PRINT_SOS_PROP 1 */
#ifdef PRINT_SOS_PROP
    if (ClauseQueryProp(CPIsSOS)) {
        fprintf(out, "/* SoS */");
    } else {
        fprintf(out, "/* --- */");
    }
#endif

    if (Options::OutputFormat == IOFormat::TPTPFormat) {
        ClausePrintTPTPFormat(out);
    } else if (Options::OutputFormat == IOFormat::TSTPFormat) {
        //cout<<"ident="<<ident<<endl;
        ClauseTSTPPrint(out, fullterms, true);
    } else
        fprintf(out, "IOFormate is Error!");
    //else {
    //ClausePrintLOPFormat(out, fullterms);
    //}
}

/***************************************************************************** 
 * Function: ClausePrintTPTPFormat()
 * Print a clause in TPTP format. 
 ****************************************************************************/
void Clause::ClausePrintTPTPFormat(FILE* out) {

    int source;
    string typeName = "";
    switch (ClauseQueryTPTPType()) {
        case (int) ClauseProp::CPTypeAxiom:
            typeName = "axiom";
            break;
        case (int) ClauseProp::CPTypeHypothesis:
            typeName = "hypothesis";
            break;
        case (int) ClauseProp::CPTypeConjecture:
        case (int) ClauseProp::CPTypeNegConjecture:
            typeName = "conjecture";
            break;
        case (int) ClauseProp::CPTypeLemma:
            typeName = "lemma";
            break;
        case (int) ClauseProp::CPTypeWatchClause:
            typeName = "watchlist";
            break;
        default:
            typeName = "unknown";
            break;
    }
    source = ClauseQueryCSSCPASource();

    if (ident >= 0) {
        fprintf(out, "input_clause(c_%d_%ld,%s,[", source, ident, typeName.c_str());
    } else {
        fprintf(out, "input_clause(i_%d_%ld,%s,[", source, ident - LONG_MIN, typeName.c_str());
    }
    //literals->EqnListPrint(out, ",", false, true);
    fprintf(out, "]).");
}

/***************************************************************************** 
 * Print a clause in TSTP format. If complete is true, terminate clause properly, 
 * otherwise stop after the logical part.
 ****************************************************************************/
void Clause::ClauseTSTPPrint(FILE* out, bool fullterms, bool complete) {
    int source;
    string type_name = "plain";

    switch (ClauseQueryTPTPType()) {
        case (int) ClauseProp::CPTypeAxiom:
            if (ClauseQueryProp(ClauseProp::CPInputFormula)) {
                type_name = "axiom";
            }
            break;
        case (int) ClauseProp::CPTypeHypothesis:
            type_name = "hypothesis";
            break;
        case (int) ClauseProp::CPTypeConjecture:
            type_name = "conjecture";
            break;
        case (int) ClauseProp::CPTypeLemma:
            type_name = "lemma";
            break;
        case (int) ClauseProp::CPTypeWatchClause:
            type_name = "watchlist";
            break;
        case (int) ClauseProp::CPTypeNegConjecture:
            type_name = "negated_conjecture";
            break;
        default:
            break;
    }
    source = ClauseQueryCSSCPASource();
    if (ident >= 0) {
        fprintf(out, "cnf(c_%d_%ld, ", source, ident);
    } else {
        fprintf(out, "cnf(i_%d_%ld, ", source, ident - LONG_MIN);
    }
    fprintf(out, "%s, ", type_name.c_str());
    ClauseTSTPCorePrint(out, fullterms);
    if (complete) {
        fprintf(out, ").");
    }
}

/***************************************************************************** 
 * Print a core clause in TSTP format.
 ****************************************************************************/
void Clause::ClauseTSTPCorePrint(FILE* out, bool fullterms) {
    fputc('(', out);
    if (ClauseIsEmpty()) {
        fprintf(out, "$false");
    } else {
        EqnListTSTPPrint(out, literals, "|", fullterms);
    }
    fputc(')', out);
}

/*****************************************************************************
 * Same as above, but without negation and uses TSTP literal format. 
 ****************************************************************************/
void Clause::EqnListTSTPPrint(FILE* out, Literal* lst, string sep, bool fullterms) {
    Literal* handle = lst;

    if (handle) {
        handle->EqnTSTPPrint(out, fullterms);

        while (handle->next) {
            handle = handle->next;
            fputs(sep.c_str(), out);
            handle->EqnTSTPPrint(out, fullterms);
        }
    }
}


//识别子句的类型

ClauseProp Clause::ClauseTypeParse(Scanner* in, string &legal_types) {
    ClauseProp res;

    in->CheckInpId(legal_types);

    if (in->TestInpId("axiom|definition|theorem")) {
        res = ClauseProp::CPTypeAxiom;
    } else if (in->TestInpId("question")) {
        res = ClauseProp::CPTypeQuestion;
    } else if (in->TestInpId("conjecture")) {
        res = ClauseProp::CPTypeConjecture;
    } else if (in->TestInpId("assumption|negated_conjecture")) {
        res = ClauseProp::CPTypeNegConjecture;
    } else if (in->TestInpId("hypothesis")) {
        res = ClauseProp::CPTypeHypothesis;
    } else if (in->TestInpId("lemma")) {
        res = ClauseProp::CPTypeLemma;
    } else if (in->TestInpId("watchlist")) {
        res = ClauseProp::CPTypeWatchClause;
    } else {
        res = ClauseProp::CPTypeUnknown;
    }

    in->AcceptInpTok(TokenType::Ident);
    return res;
}


/// 解析子句
/// \param in 扫描器scanner

Clause* Clause::ClauseParse() {

    Scanner* in = Env::getIn();
    TermBank* t = Env::getTb();

    cout << endl;
    t->vars->VarBankClearExtNames();

    ClauseProp type = ClauseProp::CPTypeAxiom;

    ClauseInfo* info = new ClauseInfo(nullptr, (in->AktToken())->source.c_str(), (in->AktToken())->line, (in->AktToken())->column);

    Literal* concl = new Literal();

    if (in->format == IOFormat::TPTPFormat) {
        in->AcceptInpId("input_clause");
        in->AcceptInpTok(TokenType::OpenBracket);
        info->name = (in->AktToken())->literal.c_str();
        in->AcceptInpTok(TokenType::Name);
        in->AcceptInpTok(TokenType::Comma);
        string strId = "axiom|hypothesis|conjecture|lemma|unknown|watchlist";
        type = ClauseTypeParse(in, strId);

        if (type == ClauseProp::CPTypeConjecture) {
            type = ClauseProp::CPTypeNegConjecture; /* Old TPTP syntax lies ;-) */
        }
        in->AcceptInpTok(TokenType::Comma);
        in->AcceptInpTok(TokenType::OpenSquare);

        //此处过滤项bank  EqnListParse(in, bank,Comma);
        //此处解析完提条子句
        concl->EqnListParse(TokenType::Comma);

        in->AcceptInpTok(TokenType::CloseSquare);
        in->AcceptInpTok(TokenType::CloseBracket);
    } else if (in->format == IOFormat::TSTPFormat) {

        in->AcceptInpId("cnf");
        in->AcceptInpTok(TokenType::OpenBracket);
        info->name = (in->AktToken())->literal.c_str();
        in->AcceptInpTok(TokenType::NamePosIntSQStr);
        in->AcceptInpTok(TokenType::Comma);

        string strId = "axiom|definition|theorem|assumption|hypothesis|negated_conjecture|lemma|unknown|plain|watchlist";
        //判断子句类型
        type = ClauseTypeParse(in, strId);

        in->AcceptInpTok(TokenType::Comma);

        if (in->TestInpTok(TokenType::OpenBracket)) {
            in->AcceptInpTok(TokenType::OpenBracket);
            //此处生成 文字List
            concl->EqnListParse(TokenType::Pipe);

            in->AcceptInpTok(TokenType::CloseBracket);
        } else {
            //此处 文字List
            concl->EqnListParse(TokenType::Pipe);
        }

        if (in->TestInpTok(TokenType::Comma)) {
            in->AcceptInpTok(TokenType::Comma);
            in->TSTPSkipSource();
            if (in->TestInpTok(TokenType::Comma)) {

                in->AcceptInpTok(TokenType::Comma);

                in->CheckInpTok(TokenType::OpenSquare);

                in->ParseSkipParenthesizedExpr();
            }
        }
        in->AcceptInpTok(TokenType::CloseBracket);
    } else {
        //        //此处过滤term　EqnListParse(in, bank, Pipe);
        //        //cout<<"into EqnListParse"<<endl;
        //        concl->EqnListParse( TokenType::Semicolon);
        //        //cout<<"out EqnListParse"<<endl;
        //        if (in->TestInpTok(TokenType::Colon)) {
        //            if (concl->EqnListLength() > 1) {
        //                in->AktTokenError("Procedural rule cannot have more than one head literal", false);
        //            }
        //            procedural = true;
        //        } else if (in->TestInpTok(TokenType::QuestionMark)) {
        //            if (concl->EqnListLength() > 0) {
        //                in->AktTokenError("Query should consist only of tail literals", false);
        //            }
        //            type = ClauseProp::CPTypeNegConjecture;
        //            /* printf("CPTypeConjecture\n"); */
        //        }
        //        if (in->TestInpTok(TokenType::Fullstop)) {
        //            if (concl->EqnListLength() > 1) {
        //                in->AktTokenError("Procedural fact cannot have more than one literal", false);
        //            }
        //            procedural = true;
        //        } else {
        //            in->AcceptInpTok(LesserSign | Colon | QuestionMark);
        //            in->AcceptInpTokNoSkip(Hyphen);
        //            //此处过滤term　EqnListParse(in, bank, Pipe);
        //            precond->EqnListParse(in, bank, Comma);
        //
        //            if (procedural && precond->EqnListLength() == 0) {
        //                in->AktTokenError("Procedural rule or query needs at least one tail literal (Hey! I did not make this syntax! -StS)", false);
        //            }
        //            precond->EqnListNegateEqns();
        //            concl = Literal::EqnListAppend(&concl, precond);
        //            //concl->EqnListAppend(precond);
        //        }
    }


    in->AcceptInpTok(TokenType::Fullstop);

    Clause * handle = new Clause(concl);
    handle->ClauseSetTPTPType(type);
    handle->ClauseSetProp((ClauseProp) ((int32_t) ClauseProp::CPInitial | (int32_t) ClauseProp::CPInputFormula));
    handle->info = info;
    return handle;



}


