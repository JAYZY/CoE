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

Clause::Clause(Literal* literal_s) : Clause() {
    //Clause_p handle = EmptyClauseAlloc();

    Literal* next;

    ident = ++Env::global_clause_counter;

    //    while (literal_s) {
    //        next = literal_s->next;
    //        if (literal_s->EqnIsPositive()) {
    //            posLitNo++;
    //            *pos_append = literal_s;
    //            pos_append = &((*pos_append)->next);
    //        } else {
    //            negLitNo++;
    //            *neg_append = literal_s;
    //            neg_append = &((*neg_append)->next);
    //        }
    //        literal_s = next;
    //    }
    //    *pos_append = neg_lits;
    //    *neg_append = nullptr;
    //    literals = pos_lits;
    //return handle;
}

Clause::~Clause() {
}
/*---------------------------------------------------------------------*/
/*                  Member Function-[public]                           */
/*---------------------------------------------------------------------*/

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
        concl->EqnListParse(  TokenType::Comma);

        in->AcceptInpTok(TokenType::CloseSquare);
        in->AcceptInpTok(TokenType::CloseBracket);
    } else if (in->format == IOFormat::TSTPFormat) {
        in->AcceptInpId("cnf");
        in->AcceptInpTok(TokenType::OpenBracket);
        info->name = (in->AktToken())->literal.c_str();
        in->AcceptInpTok(TokenType::NamePosIntSQStr);
        in->AcceptInpTok(TokenType::Comma);
        string strId = "axiom|definition|theorem|assumption|hypothesis|negated_conjecture|lemma|unknown|plain|watchlist";
        type = ClauseTypeParse(in, strId);

        in->AcceptInpTok(TokenType::Comma);

        if (in->TestInpTok(TokenType::OpenBracket)) {
            in->AcceptInpTok(TokenType::OpenBracket);
            //此处过滤term　EqnListParse(in, bank, Pipe);
            concl->EqnListParse(TokenType::Pipe);
            in->AcceptInpTok(TokenType::CloseBracket);
        } else {
            //此处过滤term　EqnListParse(in, bank, Pipe);
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
    }


}


