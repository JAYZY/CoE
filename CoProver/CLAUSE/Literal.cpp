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
#include "Orderings/Ordering.h"
#include "Clause.h"

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
    EqnAlloc(lt, rt, positive);
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

bool Literal::eqn_parse_real(Scanner* in,Term_p *lref, Term_p *rref, bool fof) {
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
            if (in->TestInpTok(TokenType::TildeSign)) {//检查是否为 ~ 负文字
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

bool Literal::eqn_parse_prefix(TermCell * *lref, TermCell * *rref,VarBank_p varbank) {
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
        lterm = bank->TBTermParseReal(in,varbank, true); //TBTermParse();
        //BOOL_TERM_NORMALIZE(lterm);
        if (lterm == bank->falseTerm) {
            lterm = bank->trueTerm;
            positive = !positive;
        }
        in->AcceptInpTok(TokenType::Comma);
        rterm = bank->TBTermParseReal(in,varbank, true); //TBTermParse(in, bank);
        //BOOL_TERM_NORMALIZE(rterm);
        if (rterm == bank->falseTerm) {
            rterm = bank->trueTerm;
            positive = !positive;
        }
        in->AcceptInpTok(TokenType::CloseBracket);
    } else {
        lterm = bank->TBTermParseReal(in,varbank, true); //TBTermParse(in, bank);
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
    TermBank* bank = this->claPtr->claTB;

    //lterm = TBTermParse(in, bank);
    lterm = bank->TBTermParseReal(in,varbank, true);
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
            rterm = bank->TBTermParseReal(in,varbank, true); //TBTermParse(in, bank);
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
            rterm = bank->TBTermParseReal(in,varbank, true); //TBTermParse(in, bank);
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
    if (IsPropFalse()) {
        fputs("$false", out);
    } else {
        if (EqnIsEquLit()) {

            Env::getTb()->TBPrintTerm(out, lterm, fullterms);
            fprintf(out, "%s", IsNegative() ? "!=" : "=");
            Env::getTb()->TBPrintTerm(out, rterm, fullterms);
        } else {
            if (IsNegative()) {
                fputc('~', out);
            }
            Env::getTb()->TBPrintTerm(out, lterm, fullterms);
            // bank->TBPrintTerm(out, lterm, fullterms);
        }
    }
}

/*-----------------------------------------------------------------------
// Function: EqnOrient()
//  文字左右项交换.交换规则: 只有当左项 小于 右项 时候进行交换
 * 1.左项== true
 * 2.指定比较规则下 左项小
// Global Variables: -
// 
// Side Effects    : May change term references
//
/----------------------------------------------------------------------*/

bool Literal::EqnOrient() {
    CompareResult relation = CompareResult::toUncomparable;
    bool res = false;

    if ( this->EqnQueryProp(EqnProp::EPMaxIsUpToDate)) {
        return false;
    }
    if (this->lterm == this->rterm) {
        relation = CompareResult::toEqual;
    } else if (this->lterm == Env::getTb()->trueTerm) {
        relation =CompareResult::toLesser;
    } else if (this->rterm == Env::getTb()->trueTerm) {
        relation =CompareResult::toGreater;
    } else {
        /* printf("EqnOrient: ");
        TermPrint(stdout, eq->lterm, eq->bank->sig, DEREF_ALWAYS);
        printf(" # ");
        TermPrint(stdout, eq->rterm, eq->bank->sig, DEREF_ALWAYS);
        printf("\n");*/
        relation =Ordering::Tocompare(this->lterm,this->rterm,DerefType::DEREF_ALWAYS,DerefType::DEREF_ALWAYS);
    }
    switch (relation) {
        case CompareResult::toUncomparable:
        case CompareResult::toEqual:
            this->EqnDelProp(EqnProp::EPIsOriented);
            break;
        case CompareResult::toGreater:
            this->EqnSetProp(EqnProp::EPIsOriented);            
            break;
        case CompareResult::toLesser:
            this->swapSides();            
            this->EqnSetProp(EqnProp::EPIsOriented);
            res = true;
            break;
        default:
            assert(false);
            break;
    }    
this->EqnSetProp(EqnProp::EPMaxIsUpToDate);            
    return res;
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
  Literal* Literal::renameCopy(VarBank_p varbank){
      
      Term_p lt=lterm->renameCopy(varbank);
      Term_p rt=rterm->renameCopy(varbank);
      Literal* newLit=new Literal(lt,rt,false);
      newLit->properties = this->properties;
      return newLit;
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
VarBank_p varbank=this->claPtr->claVarTerms;
 
    ltermCpy = Env::getTb()->TBInsertNoProps(this->lterm,varbank, DerefType::DEREF_ALWAYS);
    rtermCpy = Env::getTb()->TBInsertNoProps(this->rterm,varbank, DerefType::DEREF_ALWAYS);

    /* Properties will be taken care of later! */
    Literal* handle = new Literal(ltermCpy, rtermCpy, false);
    handle->properties = this->properties;
    if (!IsOriented()) {
        handle->EqnDelProp(EqnProp::EPMaxIsUpToDate);
    }
    return handle;
}

/*----------------------------------------------------------------------- 
//   Create a flat copy of eq. 
/----------------------------------------------------------------------*/
//

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
    
    VarBank_p varbank=this->claPtr->claVarTerms;
    
    Term_p lNewTerm = Env::getTb()->TBInsertOpt(lterm,varbank, DerefType::DEREF_ALWAYS);
    Term_p rNewTerm = Env::getTb()->TBInsertOpt(rterm, varbank,DerefType::DEREF_ALWAYS);
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
    handle->fCode = Env::getSig()->SigGetEqnCode(this->IsPositive());
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

void Literal::EqnFOFParse(Scanner* in, TermBank_p bank) {
    //Term_p lterm, rterm;
    bool positive = eqn_parse_real(in,  &this->lterm, &rterm, bank->shareVars,true);
    EqnAlloc(this->lterm, this->rterm, positive);
}

/*-----------------------------------------------------------------------
 *E的比较规则如下:
 * (1)谁被指定谁大.
 * (2)若两个文字均为指定的文字,并且一正一负,不能比较.因为指定发生冲突了,不知道究竟是正文字大还是负文字大.
 * (3)若两个文字均没有被指定,则
 *      (3.1)两者同为正或同为负
 *      (3.2)两者一正一负
/*---------------------------------------------------------------------*/
/// 文字比较
/// \param lit 比较的文字
/// \return 

CompareResult Literal::Compare(Literal* lit) {
    if (this->IsSelected()) {
        if (lit->IsSelected()&&!EqnAreEquivProps(lit, EqnProp::EPIsPositive))
            return CompareResult::toUncomparable; //两者都被指定且一正一负不能比较
        if (!lit->IsSelected())
            return CompareResult::toGreater;
    } else if (lit->IsSelected()) {
        return CompareResult::toLesser;
    }
    assert(!this->IsSelected());
    assert(!lit->IsSelected());

    if (this->EqnAreEquivProps(lit, EqnProp::EPIsPositive)) {//同正同负
        return this->ComparePosToPos(lit);
    } else if (lit->IsNegative()) {//pos-neg
        return this->ComparePosToNeg(lit);
    } else { //neg-pos
        assert(lit->IsPositive());
        assert(this->IsNegative());
        return Ordering::InverseRelation(lit->ComparePosToNeg(this));
    }
    return CompareResult::toUncomparable; //应该到不了这个语句

}


/*-----------------------------------------------------------------------
 * 对两个正文字进行比较,采用E的算法如下:
 *  
 * (1)相等 iff  l1=l2&&r1=r2 或者 l1=r2 && r1=l2
 * (2)>toEqn 考虑了等词情况
 *           iff (l1>l2 & l1>r2) v (r1>l2 & r1>r2) v
 *               (l1>=l2 & r1>=r2) v (r1>=l2 & l1>=r2)
 * (3)<toEqn 
 *           iff (l1<l2 & r1<l2) v  (l1<=l2 & r1<=r2)  v
 *               (r1<=l2 & l1<=r2) v (l1<r2 & r1<r2) 
 * (4) uncomparable
 * 简化操作,若两个文字是有序的 也就是说确保 l1>r1. 
 * Then (2) and (3) of the above comparison can be simplified in the following way:
 * (5) {l1,r1} >> {l2,r2}  <==>  (l1>l2 & l1>r2) v (l1>=l2 & r1>=r2)
 * (6) {l1,r1} << {l2,r2}  <==>  l1<l2 v (l1<=l2 & r1<=r2) v
 *                               (r1<=l2 & l1<=r2) v l1<r2
//
//   Assume that l2>r2 holds. Then (2) and (3) of the above comparison
//   can be simplified in the following way:
//
//     (7) {l1,r1} >> {l2,r2}  <==>  l1>l2 v
//                                   (l1>=l2 & r1>=r2) v
//                                   (r1>=l2 & l1>=r2) v
//                                   r1>l2
//
//     (8) {l1,r1} << {l2,r2}  <==>  (l1<l2 & r1<l2) v
//                                   (r1<=l2 & l1<=r2)
//
//   Assume that l1>r1 and l2>r2 hold. Then (1), (2) and (3) of the
//   above comparison can be simplified in the following way:
//
//     (9)  {l1,r1} == {l2,r2}  <==>  l1=l2 & r1=r2
//
//     Assume that {l1,r1} =/= {l2,r2}. Then,
//
//     (10) {l1,r1} >> {l2,r2}  <==>  l1>l2 v (l1=l2 & r1>r2)
//     (11) {l1,r1} << {l2,r2}  <==>  l1<l2 v (l1=l2 & r1<r2)
//
//   The simplified versions (5)-(11) are not yet implemented!
//
//
// Global Variables: -
//
/*---------------------------------------------------------------------*/

/// \param toEqn
/// \return 

CompareResult Literal::ComparePosToPos(Literal* posEqn) {
    assert(this->EqnAreEquivProps(posEqn, EqnProp::EPIsPositive));

    CompareResult l1l2 = Ordering::Tocompare(this->lterm, posEqn->lterm, DerefType::DEREF_ALWAYS, DerefType::DEREF_ALWAYS);
    CompareResult r1r2 = Ordering::Tocompare(this->rterm, posEqn->rterm, DerefType::DEREF_ALWAYS, DerefType::DEREF_ALWAYS);
    if (l1l2 == CompareResult::toEqual && r1r2 == CompareResult::toEqual) {
        return CompareResult::toEqual;
    }
    //(l1>=l2 & r1>=r2)
    if ((l1l2 == CompareResult::toGreater || l1l2 == CompareResult::toEqual)&&
            (r1r2 == CompareResult::toGreater || r1r2 == CompareResult::toEqual)) {
        return CompareResult::toGreater;
    }
    //(l1<=l2 & r1<=r2)
    if ((l1l2 == CompareResult::toLesser || l1l2 == CompareResult::toEqual) &&
            (r1r2 == CompareResult::toLesser || r1r2 == CompareResult::toEqual)) {
        return CompareResult::toLesser;
    }
    //下面判定只有等词才会出现  
    /* (2)>toEqn 考虑了等词情况
     *           iff (l1>l2 & l1>r2) v (r1>l2 & r1>r2) v
     *               (l1>=l2 & r1>=r2) v (r1>=l2 & l1>=r2)  */
    CompareResult l1r2 = Ordering::Tocompare(this->lterm, posEqn->rterm, DerefType::DEREF_ALWAYS, DerefType::DEREF_ALWAYS);
    // (l1>l2 & l1>r2)
    if (l1l2 == CompareResult::toGreater && l1r2 == CompareResult::toGreater) {
        return CompareResult::toGreater;
    }
    //(r1<r2 & l1<r2)
    if (r1r2 == CompareResult::toLesser && l1r2 == CompareResult::toLesser) {
        return CompareResult::toLesser;
    }

    CompareResult r1l2 = Ordering::Tocompare(this->rterm, posEqn->lterm, DerefType::DEREF_ALWAYS, DerefType::DEREF_ALWAYS);

    if ((l1r2 == CompareResult::toEqual) && (r1l2 == CompareResult::toEqual)) {
        return CompareResult::toEqual; /* Case (1) */
    }

    if (((r1l2 == CompareResult::toGreater) || (r1l2 == CompareResult::toEqual))
            && ((l1r2 == CompareResult::toGreater) || (l1r2 == CompareResult::toEqual))) {
        return CompareResult::toGreater; /* Case (2) */
    }


    if ((r1l2 == CompareResult::toGreater) && (r1r2 == CompareResult::toGreater)) {
        return CompareResult::toGreater; /* Case (2) */
    }

    if ((l1l2 == CompareResult::toLesser) && (r1l2 == CompareResult::toLesser)) {
        return CompareResult::toLesser; /* Case (3) */
    }

    if (((r1l2 == CompareResult::toLesser) || (r1l2 == CompareResult::toEqual))
            && ((l1r2 == CompareResult::toLesser) || (l1r2 == CompareResult::toEqual))) {
        return CompareResult::toLesser; /* Case (3) */
    }
    return CompareResult::toUncomparable;
}
/*-----------------------------------------------------------------------
 * Note: this literal 是正文字,negEqn 是负文字
 * 比较算法如下:
 * (1) >negEqn ==> (l1>l2 && l1>r2 ) V (r1>l2 && r1>r2)
 * (2) <negEqn ==> (l1<=l2 v l1<=r2) && (r1<=l2 v r1<=r2)
 * 如果 文字是有序的 l1>r1 则判定算法简化为
 * (3) >negEqn ==> l1>l2 & l1>r2
 * (4) <negEqn ==> l1<=l2 v l1<=r2
/*---------------------------------------------------------------------*/
/// \param toEqn
/// \return 

CompareResult Literal::ComparePosToNeg(Literal* negEqn) {
    assert(this->IsPositive());
    assert(!negEqn->IsPositive());
    CompareResult l1l2 = Ordering::Tocompare(this->lterm, negEqn->lterm, DerefType::DEREF_ALWAYS, DerefType::DEREF_ALWAYS);
    if (this->IsOriented()) {
        //(4) <negEqn ==> l1<=l2 
        if (l1l2 == CompareResult::toLesser || l1l2 == CompareResult::toEqual) {
            return CompareResult::toLesser;
        }

        CompareResult l1r2 = Ordering::Tocompare(this->lterm, negEqn->rterm, DerefType::DEREF_ALWAYS, DerefType::DEREF_ALWAYS);
        //(4) <negEqn ==> l1<=r2
        if (l1r2 == CompareResult::toLesser || l1r2 == CompareResult::toEqual) {
            return CompareResult::toLesser;
        }

        //(3) >negEqn ==> l1>l2 & l1>r2
        if (l1l2 == CompareResult::toGreater && l1r2 == CompareResult::toGreater) {
            return CompareResult::toGreater;
        }
    } else {
        assert(!this->IsOriented());
        CompareResult l1r2 = Ordering::Tocompare(this->lterm, negEqn->rterm, DerefType::DEREF_ALWAYS, DerefType::DEREF_ALWAYS);
        //(1) >negEqn ==> (l1>l2 && l1>r2 ) 
        if (l1l2 == CompareResult::toGreater && l1r2 == CompareResult::toGreater) {
            return CompareResult::toGreater;
        }
        CompareResult r1l2 = Ordering::Tocompare(this->rterm, negEqn->lterm, DerefType::DEREF_ALWAYS, DerefType::DEREF_ALWAYS);
        CompareResult r1r2 = Ordering::Tocompare(this->rterm, negEqn->rterm, DerefType::DEREF_ALWAYS, DerefType::DEREF_ALWAYS);
        //(1) >negEqn ==> (r1>l2 && r1>r2)
        if (r1l2 == CompareResult::toGreater && r1r2 == CompareResult::toGreater) {
            return CompareResult::toGreater;
        }
        //(2) <negEqn ==> (l1<=l2 v l1<=r2) && (r1<=l2 v r1<=r2)

        if (((l1l2 == CompareResult::toLesser) || (l1l2 == CompareResult::toEqual) || (l1r2 == CompareResult::toLesser || (l1r2 == CompareResult::toEqual))
                && ((r1l2 == CompareResult::toLesser) || (r1l2 == CompareResult::toEqual) || (r1r2 == CompareResult::toLesser) || (r1r2 == CompareResult::toEqual)))) {
            return CompareResult::toLesser; /* Case (3) Buggy, changed by StS */
        }
    }
    return CompareResult::toUncomparable;
}
/*---------------------------------------------------------------------*/
/*                          Static Function                            */
/*---------------------------------------------------------------------*/
//

void Literal::EqnListFree(Literal* lst) {
    Literal* handle;
    while (lst) {
        handle = lst;
        lst = lst->next;
        DelPtr(handle);
    }
}
