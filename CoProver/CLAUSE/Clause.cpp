/* 
 * File:   Clause.cpp
 * Author: Zhong Jian<77367632@qq.com>  
 * Created on 2017年12月22日, 下午8:41
 */

#include <stdint.h>
#include "Clause.h"
#include "Indexing/TermIndexing.h"
#include "HEURISTICS/SortRule.h"
using namespace std;
/*---------------------------------------------------------------------*/
/*                    Constructed Function                             */
/*---------------------------------------------------------------------*/

//

Clause::Clause()
: properties(ClauseProp::CPIgnoreProps), info(nullptr), literals(nullptr)
, negLitNo(0), posLitNo(0), weight(0), priority(0), parent1(nullptr), parent2(nullptr) {
    ident = ++Env::global_clause_counter;
    claTB = nullptr; // new TermBank(ident);
}

Clause::Clause(const Clause* orig) {
    this->ident = orig->ident; //子句创建时确定的唯一识别子句的id   PS:一般为子句编号 
    this->properties = orig->properties; //子句属性
    this->info = orig->info; //子句信息

    this->literals = nullptr; //文字列表
    this->negLitNo = 0; //负文字个数
    this->posLitNo = 0; //正文字个数
    this->weight = 0; //子句权重
    this->priority = 0;
    this->claTB = orig->claTB;

    this->parent1 = orig->parent1; //父子句1;
    this->parent2 = orig->parent2; //父子句2;
}

/**
 * 创建子句中的文字，顺序：正文字-负文字-等词
 * @param lits
 */
Clause::Clause(Literal* lits) : Clause() {
    //Clause_p handle = EmptyClauseAlloc();
    Literal *pos_lits = nullptr, *neg_lits = nullptr, *eqn_lits = nullptr;
    Literal* *pos_append = &pos_lits;
    Literal* *neg_append = &neg_lits;
    Literal* *eqn_append = &eqn_lits;
    Literal* next = nullptr;
    while (lits) {
        lits->claPtr = this; /*指定当前文字所在子句*/
        lits->EqnSetProp(EqnProp::EPIsHold);
        next = lits->next;

        if (lits->IsPositive()) {
            posLitNo++;
            if (lits->EqnIsEquLit()) {
                *eqn_append = lits;
                eqn_append = &((*eqn_append)->next);
            } else {
                *pos_append = lits;
                pos_append = &((*pos_append)->next);
            }
        } else {
            negLitNo++;
            if (lits->EqnIsEquLit()) {
                *eqn_append = lits;
                eqn_append = &((*eqn_append)->next);
            } else {
                *neg_append = lits;
                neg_append = &((*neg_append)->next);
            }
        }
        lits = next;
    }
    *neg_append = eqn_lits;
    *pos_append = neg_lits;
    *eqn_append = nullptr;
    //*neg_append = nullptr;
    literals = pos_lits;
}

Clause::~Clause() {
    /* 删除子句，并且需要删除删除子句中的文字或项 */
    //assert(!setClause);
    //DelPtr(evaluations);
    Literal::EqnListFree(literals);
    // children.Destroy(); //PTreeFree( );
    DelPtr(info); //ClauseInfoFree(junk->info);
    DelPtr(claTB);
    // if (!derivation.empty()) {
    //    derivation.clear();
    //   vector<IntOrP>().swap(derivation);
    //PStackFree(junk->derivation);
}

/*---------------------------------------------------------------------*/
/*                  Member Function-[public]                           */

/*---------------------------------------------------------------------*/

/// 给一个没有绑定文字链的子句,绑定一个文字链 lit
/// \param lit

void Clause::bindingLits(Literal* litLst) {
    assert(literals == nullptr);
    assert(litLst);
    Literal *pos_lits = nullptr, *neg_lits = nullptr, *eqn_lits = nullptr;
    Literal* *pos_append = &pos_lits;
    Literal* *neg_append = &neg_lits;
    Literal* *eqn_append = &eqn_lits;
    Literal* next = nullptr;

    int iLitPos = 0;
    while (litLst) {
        litLst->EqnSetProp(EqnProp::EPIsHold);
        litLst->claPtr = this; /*指定当前文字所在子句*/
        litLst->pos = ++iLitPos;
        next = litLst->next;
        if (litLst->IsPositive()) {
            posLitNo++;
            if (litLst->EqnIsEquLit()) {
                *eqn_append = litLst;
                eqn_append = &((*eqn_append)->next);
            } else {
                *pos_append = litLst;
                pos_append = &((*pos_append)->next);
            }
        } else {
            negLitNo++;
            if (litLst->EqnIsEquLit()) {
                *eqn_append = litLst;
                eqn_append = &((*eqn_append)->next);
            } else {
                *neg_append = litLst;
                neg_append = &((*neg_append)->next);
            }
        }
        litLst = next;
    }
    //    *neg_append = eqn_lits;
    //    *pos_append = neg_lits;
    //    *eqn_append = nullptr;
    //   literals = pos_lits;
    *neg_append = nullptr;
    *pos_append = neg_lits;
    *eqn_append = pos_lits;
    literals = eqn_lits;
}

void Clause::bindingAndRecopyLits(const vector<Literal*>&vNewR) {
    //插入新子句
    Literal *pos_lits = nullptr, *neg_lits = nullptr, *eqn_lits = nullptr;
    Literal* *pos_append = &pos_lits;
    Literal* *neg_append = &neg_lits;
    Literal* *eqn_append = &eqn_lits;
    Literal* next = nullptr;
    uint16_t iLitPos = 0;
    auto litTmpPtr = vNewR.begin();
    while (litTmpPtr != vNewR.end()) {

        Literal* newLitP = (*litTmpPtr)->RenameCopy(this);
        newLitP->claPtr = this; /*指定当前文字所在子句*/
        newLitP->pos = ++iLitPos;
        newLitP->EqnSetProp(EqnProp::EPIsHold);
        if (newLitP->IsPositive()) {
            posLitNo++;
            if (newLitP->EqnIsEquLit()) {
                *eqn_append = newLitP;
                eqn_append = &((*eqn_append)->next);
            } else {
                *pos_append = newLitP;
                pos_append = &((*pos_append)->next);
            }
        } else {
            negLitNo++;
            if (newLitP->EqnIsEquLit()) {
                *eqn_append = newLitP;
                eqn_append = &((*eqn_append)->next);
            } else {
                *neg_append = newLitP;
                neg_append = &((*neg_append)->next);
            }
        }
        ++litTmpPtr;
    }
    //    *neg_append = eqn_lits;
    //    *pos_append = neg_lits;
    //    *eqn_append = nullptr;
    //    literals = pos_lits;
    *neg_append = nullptr;
    *pos_append = neg_lits;
    *eqn_append = pos_lits;
    literals = eqn_lits;
}

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

void Clause::getStrOfClause(string&outStr, bool complete) {
    if (Options::OutputFormat != IOFormat::TSTPFormat) {
        Out::Error("File Formate is Error!", ErrorCodes::FILE_ERROR);
    }
    string type_name = "plain";
    if (ClauseQueryProp(ClauseProp::CPTypeAxiom))
        type_name = "axiom";
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
    int source = ClauseQueryCSSCPASource();
    if (ident >= 0) {
        outStr += "cnf(c" /*+ to_string(source) + "_"*/ + to_string(ident) + ", ";
    } else {
        outStr += "cnf(i" /*+ to_string(source) + "_"*/ + to_string(ident - INT_MIN) + ", ";
    }
    outStr += type_name;

    // ClauseTSTPCorePrint(out, fullterms);
    outStr += ", (";
    if (ClauseIsEmpty()) {
        outStr += "$false";
    } else {
        this->getEqnListTSTP(outStr, "|", false);
        //EqnListTSTPPrint(out, literals, "|", fullterms);
    }
    outStr += (complete == true) ? ") ).\n" : ") ";

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
        fprintf(out, "input_clause(c_%d_%d,%s,[", source, ident, typeName.c_str());
    } else {
        fprintf(out, "input_clause(i_%d_%d,%s,[", source, ident - INT_MIN, typeName.c_str());
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
        fprintf(out, "cnf(c_%d_%d, ", source, ident);
    } else {
        fprintf(out, "cnf(i_%d_%d, ", source, ident - INT_MIN);
    }
    fprintf(out, "%s, ", type_name.c_str());
    ClauseTSTPCorePrint(out, fullterms);
    if (complete) {
        fprintf(out, ").\n");
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

/*-----------------------------------------------------------------------
// Function: ClauseStandardWeight()
//   Compute the standard weight of a clause (Vars = 1, Funs = 2,
//   everything counts equally.
/----------------------------------------------------------------------*/
void Clause::ClauseStandardWeight() {
    Literal* handle;
    for (handle = this->literals; handle; handle = handle->next) {
        this->weight += handle->StandardWeight();
    }
}

/*****************************************************************************
 * Same as above, but without negation and uses TSTP literal format. 
 ****************************************************************************/
void Clause::EqnListTSTPPrint(FILE* out, Literal* lst, string sep, bool fullterms) {
    Lit_p handle = lst;
    if (handle) {
        string litInfo = "";
        handle->getParentLitInfo(litInfo);
        litInfo.empty() ? fprintf(out, "{%hu}", handle->pos) : fprintf(out, "{%hu},[%s] ", handle->pos, litInfo.c_str());
        handle->EqnTSTPPrint(out, fullterms);
        while (handle->next) {
            handle = handle->next;
            fputs(sep.c_str(), out);
            litInfo = "";
            handle->getParentLitInfo(litInfo);
            litInfo.empty() ? fprintf(out, "{%hu}", handle->pos) : fprintf(out, "{%hu},[%s] ", handle->pos, litInfo.c_str());
            handle->EqnTSTPPrint(out, fullterms);

        }
    }
}

void Clause::getEqnListTSTP(string&outStr, string sep, bool colInfo) {
    Lit_p handle = this->literals;
    if (handle) {
        if (colInfo) {
            string litInfo = "";
            handle->getParentLitInfo(litInfo);
            string str1 = "{" + to_string(handle->pos) + "}";
            string str2 = ",[" + litInfo + "]";
            outStr = litInfo.empty() ? str1 : str1 + str2;
        }
        handle->getStrOfEqnTSTP(outStr);
        while (handle->next) {
            handle = handle->next;
            outStr += sep;
            if (colInfo) {
                string litInfo = "";
                handle->getParentLitInfo(litInfo);
                string str1 = "{" + to_string(handle->pos) + "}";
                string str2 = ",[" + litInfo + "]";
                outStr = litInfo.empty() ? str1 : str1 + str2;
            }
            handle->getStrOfEqnTSTP(outStr);
        }
    }
}
//对子句中的文字进行排序

/*建议 先负后正,先稳定低后稳定高  */
void Clause::SortLits() {

    uint32_t uLitNum = LitsNumber();
    if (1 == uLitNum) return;
    //this->ClausePrint(stdout,true);    cout<<endl;
    Lit_p *sortArray = new Lit_p[uLitNum];
    Lit_p lit = this->literals;
    int ind = -1;
    while (lit) {
        sortArray[++ind] = lit;
        lit = lit->next;
    }

    sort(sortArray, sortArray + uLitNum, SortRule::LitCmp);
    //将数组转换为Literal 链表
    this->literals = sortArray[0];
    Lit_p tmpPtr = this->literals;

    for (int i = 1; i < uLitNum; ++i) {//不需要重新编码  
        tmpPtr->next = sortArray[i];
        tmpPtr = tmpPtr->next;
    }
    tmpPtr->next = nullptr;
    tmpPtr = nullptr;
    DelArrayPtr(sortArray);
    this->ClauseSetProp(ClauseProp::CPIsOriented);
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

void Clause::ClauseParse(Scanner* in) {

    //Scanner* in = Env::getIn();
    //TermBank* t = Env::getTb();
    if (this->claTB)
        this->claTB->varsClearExtNames(); //清除变量集合 clear varbank

    ClauseProp type = ClauseProp::CPTypeAxiom; //子句默认属性为 公理集
    //读取子句相关信息(info) 子句名称-i_0_266,原始字符串,所在行 所在列,
    this->info = new ClauseInfo("", (in->AktToken())->source.c_str(), (in->AktToken())->line, (in->AktToken())->column);


    //创建文字
    //Literal* concl = new Literal();
    if (in->format == IOFormat::TPTPFormat) {
        in->AcceptInpId("input_clause");
        in->AcceptInpTok(TokenType::OpenBracket);
        this->info->name = (in->AktToken())->literal;
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

        this->info->name = (in->AktToken())->literal; //赋值子句名称 如:i_0_266
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
        return;
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

    //为了节约内存，若为基项则删除ClaTB
    if (this->ClauseQueryProp(ClauseProp::CPGroundCla)) {
        this->ClearClaTB();
    }
}

Clause* Clause::renameCopy(VarBank_p renameVarbank) {
    Clause* newCla = new Clause(this);
    Lit_p newlist = nullptr;
    Lit_p *insert = &newlist;
    Lit_p lit = this->literals;
    while (lit) {
        *insert = lit->RenameCopy(newCla);
        insert = &((*insert)->next);
        lit = lit->next;
    }
    *insert = nullptr;
    newCla->literals = newlist;

    return newCla;
}

/**
 * 设置文字的变元共享状态
 */
void Clause::SetEqnListVarState() {
    Lit_p lit = this->Lits();
    //  Lit_p *arrayLit = new Lit_p[this->LitsNumber()];
    Lit_p arrVarLit[500]; //假设变元项最多不超出500
    memset(arrVarLit, 0, 500);

    while (lit) {
        if (lit->IsGround()) {
            lit->varState = VarState::noVar;
            continue;
        }
        lit->varState = VarState::freeVar;
        vector<TermCell*> vecT;
        vecT.reserve(32);
        vecT.push_back(lit->lterm);

        while (!vecT.empty()) {
            TermCell* t = vecT.back();
            assert(-t->fCode < 500);
            vecT.pop_back();
            if (t->IsVar()) {
                Lit_p firstLit = arrVarLit[-t->fCode];
                if (firstLit == nullptr) {
                    arrVarLit[-t->fCode] = lit;
                } else {
                    lit->varState = VarState::shareVar;
                    firstLit->varState = VarState::shareVar;
                }
            }
            for (int i = 0; i < t->arity; i++) {
                if (t->args[i]->IsGround())
                    continue;
                vecT.push_back(t->args[i]);
            }
        }
        assert(vecT.empty());

        if (lit->EqnIsEquLit()) {
            vecT.push_back(lit->rterm);
            while (!vecT.empty()) {
                TermCell* t = vecT.back();
                assert(-t->fCode < 500);
                vecT.pop_back();
                if (t->IsVar()) {
                    Lit_p firstLit = arrVarLit[-t->fCode];

                    if (firstLit == nullptr) {
                        arrVarLit[-t->fCode] = lit;
                    } else {
                        lit->varState = VarState::shareVar;
                        firstLit->varState = VarState::shareVar;
                    }
                }
                for (int i = 0; i < t->arity; i++) {
                    if (t->args[i]->IsGround())
                        continue;
                    vecT.push_back(t->args[i]);
                }
            }
        }
        lit = lit->next;
    }

}

uint16_t Clause::calcMaxFuncLayer() const {
    uint16_t maxTermDepth = 0;
    Lit_p lit = this->literals;
    while (lit) {
        uint16_t litMaxTermDepth = lit->TermDepth();
        if (maxTermDepth < litMaxTermDepth)
            maxTermDepth = litMaxTermDepth;
        lit = lit->next;
    }
    return maxTermDepth;
}

void Clause::EqnListParse(TokenType sep) {
    Scanner* in = Env::getIn();
    bool isGroundCla = true;
    TokenType testTok = (TokenType) ((uint64_t) TokenCell::TermStartToken() | (uint64_t) TokenType::TildeSign);

    if (((in->format == IOFormat::TPTPFormat) && in->TestInpTok(TokenType::SymbToken))
            || ((in->format == IOFormat::LOPFormat) && in->TestInpTok(testTok))
            || ((in->format == IOFormat::TSTPFormat) && in->TestInpTok(testTok))) {

        Literal *pos_lits = nullptr, *neg_lits = nullptr;
        Lit_p *pos_append = &pos_lits;
        Lit_p *neg_append = &neg_lits;
        Lit_p handle = nullptr;
        int originLitPos = 0;
        while (true) {
            handle = new Literal(in, this);

            if (!handle->IsGround(false)) { //检查文字是否为基文字
                isGroundCla = false;
            }

            handle->pos = ++originLitPos; //记录文字在子句中的原始位置
            if (handle->IsPositive()) {
                ++posLitNo;
                *pos_append = handle;
                pos_append = &((*pos_append)->next);
            } else {
                ++negLitNo;
                *neg_append = handle;
                neg_append = &((*neg_append)->next);
            }
            if (!in->TestInpTok(sep))break;
            in->NextToken();
        }
        *pos_append = neg_lits;
        *neg_append = nullptr;
        this->literals = pos_lits;
    }
    //设置基子句属性
    if (isGroundCla) {
        this->ClauseSetProp(ClauseProp::CPGroundCla);
    }
}
