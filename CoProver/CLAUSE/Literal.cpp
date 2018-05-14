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
    this->properties = EqnProp::EPNoProps;
    pos = 0;
    lterm = nullptr;
    rterm = nullptr;
    next = nullptr;
    xyW = 0.0f;
    zjlitWight = 0;
}

Literal::Literal(Term_p lt, Term_p rt, bool positive) {
    EqnAlloc(lt, rt, Env::getTb(), positive);
}

Literal::Literal(const Literal& orig) {
}

Literal::~Literal() {
}

/*---------------------------------------------------------------------*/
/*                  Member Function-[private]                           */
/*---------------------------------------------------------------------*/
//   Parse an equation with optional external sign and depending on
//   wether FOF or CNF is being parsed.

bool Literal::eqn_parse_real(Scanner* in, TermBank* bank, Term_p *lref, Term_p *rref, bool fof) {
    bool positive = true;
    bool negate = false;
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

            positive = eqn_parse_prefix(lref, rref);

            break;
        case IOFormat::TSTPFormat:
            if (in->TestInpTok(TokenType::TildeSign)) {
                negate = true;
                in->NextToken();
            }
            positive = eqn_parse_infix(lref, rref);
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
        Env::getSig()->SigSetPredicate(lterm->fCode, true);
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
    TokenType equalToke = (TokenType) ((uint64_t) TokenType::NegEqualSign | (uint64_t) TokenType::EqualSign);
    //bank->sig->SigSetPredicate(lterm->fCode, true);
    Sigcell* sig = Env::getSig();
    if (!lterm->IsVar() && sig->SigIsPredicate(lterm->fCode)) {
        rterm = bank->trueTerm; /* Non-Equational literal */
    } else {
        if (lterm->IsVar() || sig->SigIsFunction(lterm->fCode)) {

            if (in->TestInpTok(TokenType::NegEqualSign)) {
                positive = !positive;
            }
            in->AcceptInpTok(equalToke);
            rterm = bank->TBTermParseReal(in, true); //TBTermParse(in, bank);
            if (!rterm->IsVar()) {
                if (sig->SigIsPredicate(rterm->fCode)) {
                    in->AktTokenError("Predicate symbol used as function symbol in preceding atom", false);
                }
                sig->SigSetFunction(rterm->fCode, true);
            }
        } else if (in->TestInpTok(equalToke)) { /* Now both sides must be terms */

            sig->SigSetFunction(lterm->fCode, true);
            if (in->TestInpTok(TokenType::NegEqualSign)) {
                positive = !positive;
            }
            in->AcceptInpTok(equalToke);
            rterm = bank->TBTermParseReal(in, true); //TBTermParse(in, bank);
            if (!rterm->IsVar()) {
                if (sig->SigIsPredicate(rterm->fCode)) {
                    in->AktTokenError("Predicate symbol used as function symbol in preceding atom", false);
                }
                sig->SigSetFunction(rterm->fCode, true);
            }
        } else { /* It's a predicate */
            rterm = bank->trueTerm; /* Non-Equational literal */
            sig->SigSetPredicate(lterm->fCode, true);
        }
    }
    *lref = lterm;
    *rref = rterm;

    return positive;
}

/*---------------------------------------------------------------------*/
/*                  Member Function-[public]                           */
/*---------------------------------------------------------------------*/

/*****************************************************************************
 * Print a literal in TSTP format.
 ****************************************************************************/
void Literal::EqnTSTPPrint(FILE* out, bool fullterms) {
    if (EqnIsPropFalse()) {
        fputs("$false", out);
    } else {
        if (EqnIsEquLit()) {

            Env::getTb()->TBPrintTerm(out, lterm, fullterms);
            fprintf(out, "%s", EqnIsNegative() ? "!=" : "=");
            Env::getTb()->TBPrintTerm(out, rterm, fullterms);
        } else {
            if (EqnIsNegative()) {
                fputc('~', out);
            }
            Env::getTb()->TBPrintTerm(out, lterm, fullterms);
            // bank->TBPrintTerm(out, lterm, fullterms);
        }
    }
}

/*****************************************************************************
 * Create a copy of list with disjoint variables (using the even/odd convention).
 * 传递的是子句的第一个文字指针  
 ****************************************************************************/
Literal* Literal::EqnListCopyDisjoint() {
    Literal* newlist = nullptr;
    Literal** insert = &newlist;
    Literal* handle = this;
    while (handle) {
        *insert = handle->EqnCopyDisjoint();
        insert = &((*insert)->next);
        handle = handle->next;
    }
    *insert = NULL;

    return newlist;
}

/***************************************************************************** 
 * Return a flat copy of the given list, reusing the existing terms.
 ****************************************************************************/
Literal* Literal::EqnListFlatCopy() {
    Literal* newlist = nullptr;
    Literal* *insert = &newlist;
    Literal* lit = this;
    while (lit) {
        *insert = lit->EqnFlatCopy();
        insert = &((*insert)->next);
        lit = lit->next;
    }
    *insert = nullptr;
    return newlist;
}

/*****************************************************************************
 * Copy an equation into the same term bank, 
 * but with disjoint (odd->even or vice versa) variable. 
 ****************************************************************************/
Literal* Literal::EqnCopyDisjoint() {
    Literal* handle;

    Term_p lt = Env::getTb()->TBInsertDisjoint(lterm);
    Term_p rt = Env::getTb()->TBInsertDisjoint(rterm);

    handle = new Literal(lt, rt, false); /* Properties will be taken care of later! */
    handle->properties = properties;

    return handle;
}

/***************************************************************************** 
 * Create a copy of eq with terms from bank. Does not copy the next pointer. 
 * Properties of the original terms are not copied.
 ****************************************************************************/
Literal* Literal::EqnCopy() {

    Term_p ltermCpy;
    Term_p rtermCpy;

    ltermCpy = Env::getTb()->TBInsertNoProps(this->lterm, DerefType::DEREF_ALWAYS);
    rtermCpy = Env::getTb()->TBInsertNoProps(this->rterm, DerefType::DEREF_ALWAYS);

    /* Properties will be taken care of later! */
    Literal* handle = new Literal(ltermCpy, rtermCpy, false);
    handle->properties = this->properties;
    if (!EqnIsOriented()) {
        handle->EqnDelProp(EqnProp::EPMaxIsUpToDate);
    }
    return handle;
}

/*----------------------------------------------------------------------- 
//   Create a flat copy of eq. 
/----------------------------------------------------------------------*/

Literal* Literal::EqnFlatCopy() {
    Literal* eq = this;
    Literal* handle;
    Term_p lterm, rterm;

    lterm = eq->lterm;
    rterm = eq->rterm;

    handle = new Literal(lterm, rterm, false); /* Properties will be
						    taken care of
						    later! */
    handle->properties = eq->properties;
    if (!handle->EqnQueryProp(EqnProp::EPIsOriented)) {
        handle->EqnDelProp(EqnProp::EPMaxIsUpToDate);
    }
    return handle;
}

/*****************************************************************************
 *  Copy an instantiated equation into the same term bank 
 * (using the common optimizations possible in that case). 
 ****************************************************************************/
Literal* Literal::EqnCopyOpt() {

    Term_p lNewTerm = Env::getTb()->TBInsertOpt(lterm, DerefType::DEREF_ALWAYS);
    Term_p rNewTerm = Env::getTb()->TBInsertOpt(rterm, DerefType::DEREF_ALWAYS);
    Literal* handle = new Literal(lNewTerm, rNewTerm, false);
    /* Properties will be taken care of later! */
    handle->properties = properties;
    handle->EqnDelProp(EqnProp::EPMaxIsUpToDate);
    handle->EqnDelProp(EqnProp::EPIsOriented);
    return handle;
}

Term_p Literal::EqnTermsTBTermEncode(bool EqnDirIsReverse) {

    Term_p handle;
    TermBank* bank = Env::getTb();

    assert(bank);
    assert(bank->TBFind(lterm));
    assert(bank->TBFind(rterm));

    handle = bank->DefaultSharedTermCellAlloc();
    handle->arity = 2;
    handle->fCode = Env::getSig()->SigGetEqnCode(this->EqnIsPositive());
    assert(handle->fCode);
    handle->args = new TermCell*[2];
    if (EqnDirIsReverse) {
        handle->args[0] = rterm;
        handle->args[1] = lterm;
    } else {
        handle->args[0] = lterm;
        handle->args[1] = rterm;
    }

    handle = bank->TBTermTopInsert(handle);

    return handle;
}

/*---------------------------------------------------------------------*/
/*                          Static Function                            */
/*---------------------------------------------------------------------*/
//

void Literal::EqnFOFParse(Scanner* in, TB_p bank) {

    //Term_p lterm, rterm;


    bool positive = eqn_parse_real(in, bank, &this->lterm, &rterm, true);
    EqnAlloc(this->lterm, this->rterm, bank, positive);
   
}

void Literal::EqnListFree(Literal* lst) {
    Literal* handle;
    while (lst) {
        handle = lst;
        lst = lst->next;
        DelPtr(handle);
    }
}
