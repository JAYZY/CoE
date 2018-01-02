/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Literal.cpp
 * Author: Zhong Jian<77367632@qq.com>
 * 
 * Created on 2017年12月23日, 下午1:58
 */

#include "Literal.h"

Literal::Literal() {
}

Literal::Literal(const Literal& orig) {
}

Literal::~Literal() {
}

/*---------------------------------------------------------------------*/
/*                  Member Function-[public]                           */
/*---------------------------------------------------------------------*/
//   Parse an equation with optional external sign and depending on
//   wether FOF or CNF is being parsed.

bool Literal::eqn_parse_real(Term_p *lref, Term_p *rref, bool fof) {
    bool positive = true;
    bool negate = false;
    Scanner* in = Env::getIn();
    TermBank* bank = Env::getTb();
    //cout<<in->AktToken()->literal;
    switch (in->format) {
        case IOFormat::LOPFormat:
            if (in->TestInpTok(TokenType::TildeSign)) {
                negate = true;
                in->NextToken();
            }
            positive = eqn_parse_mixfix(lref, rref);
            break;
        case IOFormat::TPTPFormat:
            if (fof) {
                if (in->TestInpTok(TokenType::TildeSign)) {
                    negate = true;
                    in->NextToken();
                }
            } else {
                in->CheckInpTok(TokenType::SymbToken);
                if (in->TestInpTok(TokenType::Hyphen)) {
                    negate = true;
                    in->NextToken();
                    in->AcceptInpTokNoSkip(TokenType::Hyphen);
                } else {
                    in->NextToken();
                    in->AcceptInpTokNoSkip(TokenType::Plus);
                }
            }

            positive = eqn_parse_prefix( lref, rref);

            break;
        case IOFormat::TSTPFormat:
            if (in->TestInpTok(TokenType::TildeSign)) {
                negate = true;
                in->NextToken();
            }
            positive = eqn_parse_infix(  lref, rref);
            break;
        default:
            assert(false && "Format not supported");
    }
    if (negate) {
        positive = !positive;
    }
    return positive;
}


//   Parse a literal without external sign assuming that _all_
//   equational literals are prefix. Return sign. This is for TPTP

bool Literal::eqn_parse_prefix(TermCell * *lref, TermCell * *rref) {
    Scanner* in = Env::getIn();
    TermBank* bank = Env::getTb();
    Term_p lterm = nullptr;
    Term_p rterm = nullptr;
    bool positive = true;

    if (in->TestInpId("equal")) {
        in->NextToken();
        in->AcceptInpTok(TokenType::OpenBracket);

        //lterm = TBTermParse(in, bank);直接调用TBTermParseReal
        //解析项
        lterm = bank->TBTermParseReal(in, true); //TBTermParse();
        //BOOL_TERM_NORMALIZE(lterm);
        if (lterm == bank->falseTerm) {
            lterm = bank->trueTerm;
            positive = !positive;
        }
        in->AcceptInpTok(TokenType::Comma);
        rterm = bank->TBTermParseReal(in, true); //TBTermParse(in, bank);
        //BOOL_TERM_NORMALIZE(rterm);
        if (rterm == bank->falseTerm) {
            rterm = bank->trueTerm;
            positive = !positive;
        }
        in->AcceptInpTok(TokenType::CloseBracket);
    } else {
        lterm = bank->TBTermParseReal(in, true); //TBTermParse(in, bank);
        //BOOL_TERM_NORMALIZE(lterm);
        if (lterm == bank->falseTerm) {
            lterm = bank->trueTerm;
            positive = !positive;
        }
        rterm = bank->trueTerm; /* Non-Equational literal */
    }
    if (rterm == bank->trueTerm) {
        if (lterm->IsVar()) {
            in->AktTokenError("Individual variable used at predicate position", false);

        }
        bank->sig->SigSetPredicate(lterm->fCode, true);
    }
    *lref = lterm;
    *rref = rterm;

    return positive;
}

bool Literal::eqn_parse_mixfix(TermCell **lref, TermCell * *rref) {
    Scanner* in = Env::getIn();
    if (in->TestInpId("equal")) {
        return eqn_parse_prefix(lref, rref);
    }
    return eqn_parse_infix(lref, rref);
}

/*****************************************************************************
 * Parse a literal without external sign assuming that _all_equational 
 * literals are infix. Return sign. This is for TSTP 
 ****************************************************************************/
bool Literal::eqn_parse_infix(TermCell * *lref, TermCell * *rref) {
    Term_p lterm;
    Term_p rterm;
    bool positive = true;
    Scanner* in = Env::getIn();
    TermBank* bank = Env::getTb();

    //lterm = TBTermParse(in, bank);
    lterm = bank->TBTermParseReal(in, true);
    //BOOL_TERM_NORMALIZE(lterm);
    if (lterm == bank->falseTerm) {
        lterm = bank->trueTerm;
        positive = !positive;
    }
    TokenType equalToke=(TokenType) ((uint64_t) TokenType::NegEqualSign | (uint64_t) TokenType::EqualSign);
    //bank->sig->SigSetPredicate(lterm->fCode, true);
    if (!lterm->IsVar() && bank->sig->SigIsPredicate(lterm->fCode)) {
        rterm = bank->trueTerm; /* Non-Equational literal */
    } else {
        if (lterm->IsVar() || bank->sig->SigIsFunction(lterm->fCode)) {

            if (in->TestInpTok(TokenType::NegEqualSign)) {
                positive = !positive;
            }
            in->AcceptInpTok(equalToke);
            rterm = bank->TBTermParseReal(in, true); //TBTermParse(in, bank);
            if (!rterm->IsVar()) {
                if (bank->sig->SigIsPredicate(rterm->fCode)) {
                    in->AktTokenError("Predicate symbol used as function symbol in preceding atom", false);
                }
                bank->sig->SigSetFunction(rterm->fCode, true);
            }
        } else if (in->TestInpTok(equalToke)) { /* Now both sides must be terms */
            bank->sig->SigSetFunction(lterm->fCode, true);
            if (in->TestInpTok(TokenType::NegEqualSign)) {
                positive = !positive;
            }
            in->AcceptInpTok(equalToke);
            rterm = bank->TBTermParseReal(in, true); //TBTermParse(in, bank);
            if (!rterm->IsVar()) {
                if (bank->sig->SigIsPredicate(rterm->fCode)) {
                    in->AktTokenError("Predicate symbol used as function symbol in preceding atom", false);
                }
                bank->sig->SigSetFunction(rterm->fCode, true);
            }
        } else { /* It's a predicate */
            rterm = bank->trueTerm; /* Non-Equational literal */
            bank->sig->SigSetPredicate(lterm->fCode, true);
        }
    }
    *lref = lterm;
    *rref = rterm;

    return positive;
}