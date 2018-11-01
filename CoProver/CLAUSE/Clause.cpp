/* 
 * File:   Clause.cpp
 * Author: Zhong Jian<77367632@qq.com>  
 * Created on 2017年12月22日, 下午8:41
 */

#include <stdint.h>

#include "Clause.h"
#include "Indexing/TermIndexing.h"
#include "HEURISTICS/SortRule.h"

/*---------------------------------------------------------------------*/
/*                    Constructed Function                             */
/*---------------------------------------------------------------------*/

//

Clause::Clause()
: ident(++Env::global_clause_counter), properties(ClauseProp::CPIgnoreProps), info(nullptr), literals(nullptr)
, negLitNo(0), posLitNo(0), weight(123), parent1(nullptr), parent2(nullptr) {

}

Clause::Clause(const Clause* orig) {
    this->ident = orig->ident; //子句创建时确定的唯一识别子句的id   PS:一般为子句编号 
    this->properties = orig->properties; //子句属性
    this->info = orig->info; //子句信息

    this->literals = nullptr; //文字列表
    this->negLitNo = 0; //负文字个数
    this->posLitNo = 0; //正文字个数
    this->weight = 0; //子句权重

    this->parent1 = orig->parent1; //父子句1;
    this->parent2 = orig->parent2; //父子句2;
}

Clause::Clause(Literal* lits) : Clause() {
    //Clause_p handle = EmptyClauseAlloc();


    Literal *pos_lits = nullptr, *neg_lits = nullptr;
    Literal* *pos_append = &pos_lits;
    Literal* *neg_append = &neg_lits;
    Literal* next = nullptr;


    while (lits) {
        lits->claPtr = this; /*指定当前文字所在子句*/
        next = lits->next;
        if (lits->IsPositive()) {
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
        cout << " zjw:" << handle->zjlitWight << " EW:" << handle->StandardWeight();
        cout << " T:" << TermIndexing::constTermNum[handle->lterm];
        while (handle->next) {
            handle = handle->next;
            fputs(sep.c_str(), out);
            handle->EqnTSTPPrint(out, fullterms);
            cout << " zjw:" << handle->zjlitWight << " EW:" << handle->StandardWeight();
            cout << " T:" << TermIndexing::constTermNum[handle->lterm];
        }
    }
}

//对子句中的文字进行排序

void Clause::SortLits() {
    uint32_t uLitNum = LitsNumber();
    if (1 == uLitNum) return;
    sort(this->literals, literals + uLitNum, SortRule::LitCmp);
    //SortRule::QuickSort(LitPtr, 0, uLitNum-1);
    for (uint32_t uCol = 0; uCol < uLitNum; ++uCol) //重新编码 新列号
    {
        literals[uCol].pos = (uCol + 1);
    }
}

//识别子句的类型

ClauseProp Clause::ClauseTypeParse(Scanner* in, string legal_types) {
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

void Clause::ClauseParse(Scanner* in, TermBank* t) {

    //Scanner* in = Env::getIn();
    //TermBank* t = Env::getTb();


    this->claTB->VarBankClearExtNames(); //清除变量集合 clear varbank

    ClauseProp type = ClauseProp::CPTypeAxiom; //子句默认属性为 公理集
    //读取子句相关信息(info) 子句名称-i_0_266,原始字符串,所在行 所在列,
    ClauseInfo* info = new ClauseInfo(nullptr, (in->AktToken())->source.c_str(), (in->AktToken())->line, (in->AktToken())->column);


    //创建文字
    //Literal* concl = new Literal();

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
        //此处解析完子句
        EqnListParse(TokenType::Comma);
        //concl->EqnListParse(TokenType::Comma, this->claVarTerms);

        in->AcceptInpTok(TokenType::CloseSquare);
        in->AcceptInpTok(TokenType::CloseBracket);
    } else if (in->format == IOFormat::TSTPFormat) {

        in->AcceptInpId("cnf");
        in->AcceptInpTok(TokenType::OpenBracket);
        info->name = (in->AktToken())->literal.c_str(); //赋值子句名称 如:i_0_266
        in->AcceptInpTok(TokenType::NamePosIntSQStr);
        in->AcceptInpTok(TokenType::Comma);

        string strId = "axiom|definition|theorem|assumption|hypothesis|negated_conjecture|lemma|unknown|plain|watchlist";
        //判断子句类型
        type = ClauseTypeParse(in, strId);
        //跳过 冒号
        in->AcceptInpTok(TokenType::Comma);

        if (in->TestInpTok(TokenType::OpenBracket)) {
            in->AcceptInpTok(TokenType::OpenBracket);
            //此处生成 文字List
            EqnListParse(TokenType::Pipe);

            in->AcceptInpTok(TokenType::CloseBracket);
        } else {
            //此处 文字List
            EqnListParse(TokenType::Pipe);
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
        cout << "文字解析错误" << endl;
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

    //Clause * handle = new Clause(concl);
    this->ClauseSetTPTPType(type);
    this->ClauseSetProp((ClauseProp) ((int32_t) ClauseProp::CPInitial | (int32_t) ClauseProp::CPInputFormula));
    this->info = info;
}

/// 将子句中的变元进行重命名
/// \param fresh_vars

void Clause::ClauseNormalizeVars(VarBank_p renameVarbank) {
    Subst_p subst = new Subst();
    renameVarbank->VarBankResetVCount();
    Literal* lit = this->literals;

}

Clause* Clause::renameCopy(VarBank_p renameVarbank) {
    Clause* newCla = new Clause(this);
    Literal* newlist = nullptr;
    Literal* *insert = &newlist;
    Literal* lit = this->literals;
    while (lit) {
        *insert = lit->renameCopy(renameVarbank);
        insert = &((*insert)->next);
        lit = lit->next;
    }
    *insert = nullptr;
    newCla->literals = newlist;

    return newCla;
}

