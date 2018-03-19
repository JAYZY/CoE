/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Literal.h
 * Author: Zhong Jian<77367632@qq.com>
 *
 * Created on 2017年12月23日, 下午1:58
 */

#ifndef LITERAL_H
#define LITERAL_H
#include "TERMS/TermCell.h"
#include "TERMS/TermBank.h"
#include "Global/Environment.h"
/*---------------------------------------------------------------------*/
/*                      【Literal】文字相关枚举                          */
/*---------------------------------------------------------------------*/

/* 枚举 -- 文字属性类型 */
enum class EqnProp : uint32_t {
    EPNoProps = 0, /* No properties set or selected */
    EPIsPositive = 1, /* s=t (as opposed to s!=t) */
    EPIsMaximal = 2, /* Eqn is maximal in a clause */
    EPIsStrictlyMaximal = 4, /* Eqn is strictly maximal */
    EPIsEquLiteral = 8, /* s=t, not s=$true */
    EPIsOriented = 16, /* s=>t  or s=t ? 该文字是有方向性的 */
    EPMaxIsUpToDate = 32, /* Orientation status is up to date */
    EPHasEquiv = 64, /* Literal has been used in	 multiset-comparison (and found an equivalent partner) */
    EPIsDominated = 128, /* Literal is dominated by another one */
    EPDominates = EqnProp::EPIsDominated, /* Double use of this property in potentially maximal or minimal clauses */
    EPIsUsed = 256, /* For non-injective subsumption and  pattern-generation */
    EPGONatural = 512, /* Set if left side is bigger in the special (total) ground ordering treating variables as small constants */
    EPIsSelected = 1024, /* For selective superpostion */
    EPIsPMIntoLit = 2048, /* For inheriting selection */
    EPFromClauseLit = 4096, /* This comes from the from clause in a paramod step */
    EPPseudoLit = 8192, /* This is a pseudo-literal that does not contribute to the semantic evaluation of the clause. */
    EPLPatMinimal = 16384, /* Eqn l=r is Pattern-Minimal */
    EPRPatMinimal = 32768, /* Eqn r=l is Pattern-Minimal */
    EPIsSplitLit = 65636 /* This literal has been introduced by splitting */
};

class Literal {
public:
    EqnProp properties; /*prositive ,maximal,equational */
    int pos;
    TermCell* lterm; /*左文字*/
    TermCell* rterm; /*等号右边文字,若非等词,则为$True;
    //TermBank ss;
    //TermBank* bank; /* Terms are from this bank */
    Literal* next; /*下一个文字*/

    /*所在子句信息*/
    Clause* claPtr; //所在子句
    Literal* parentLitPtr; //父子句文字
public:
    Literal();
    Literal(Term_p lt, Term_p rt, bool positive);
    Literal(const Literal& orig);
    //替代 EqnAlloc

    virtual ~Literal();
    /*---------------------------------------------------------------------*/
    /*                       Inline  Function                              */
    /*---------------------------------------------------------------------*/
    //

    inline void EqnSetProp(EqnProp prop) {
        SetProp(this->properties, prop);
    }

    inline void EqnDelProp(EqnProp prop) {
        DelProp(this->properties, prop);
    }

    inline void EqnFlipProp(EqnProp prop) {
        FlipProp(this->properties, prop);
    }

    inline bool EqnQueryProp(EqnProp prop) {
        return QueryProp(this->properties, prop);
    }

    inline bool EqnIsPositive() {
        return EqnQueryProp(EqnProp::EPIsPositive);
    }

    inline bool EqnIsNegative() {
        return !(EqnQueryProp(EqnProp::EPIsPositive));
    }

    inline bool EqnIsMaximal() {
        return EqnQueryProp(EqnProp::EPIsMaximal);
    }

    inline bool EqnIsOriented() {
        return EqnQueryProp(EqnProp::EPIsOriented);
    }

    inline bool EqnIsEquLit() {
        return EqnQueryProp(EqnProp::EPIsEquLiteral);
    }

    inline bool EqnIsPropFalse() {
        return ((lterm == rterm) && EqnIsNegative());
    }

    inline bool EqnIsTrivial() {
        return this->lterm == this->rterm;
    }

    inline long StandardWeight() {
        return lterm->TermStandardWeight() + rterm->TermStandardWeight();
    }

    /***************************************************************************** 
     * 将文字element 插入到文字pos的后面。Insert the element at the position defined by pos.
     ****************************************************************************/
    inline void EqnListInsertElement(Literal** pos, Literal* element) {
        element->next = *pos;
        *pos = element;
    }

    /***************************************************************************** 
     * Delete the given element from the list.
     ***************************************************************************/
    inline void EqnListDeleteElement(Literal** element) {
        Literal* handle = nullptr;
        handle = EqnListExtractElement(element);
        //释放文字对象，考虑需要重新把子句文字列表连接上
        DelPtr(handle);
        //EqnFree(handle);
    }

    /*****************************************************************************
     * 返回文字列表中的文字。Take the given element out of the list and return a pointer to it. 
     ****************************************************************************/
    inline Literal* EqnListExtractElement(Literal** element) {
        Literal* handle = *element;
        assert(handle);
        *element = handle->next;
        handle->next = nullptr;
        return handle;
    }

    /***************************************************************************** 
     * 解析文字,生成文字列表，EqnListParse(Scanner_p in, TB_p bank, TokenType sep)
     ****************************************************************************/
    inline void EqnListParse(TokenType sep) {

        Scanner* in = Env::getIn();
        TB_p bank = Env::getTb();

        TokenType testTok = (TokenType) ((uint64_t) TokenCell::TermStartToken() | (uint64_t) TokenType::TildeSign);

        if (((in->format == IOFormat::TPTPFormat) && in->TestInpTok(TokenType::SymbToken)) ||
                ((in->format == IOFormat::LOPFormat) && in->TestInpTok(testTok)) ||
                ((in->format == IOFormat::TSTPFormat) && in->TestInpTok(testTok))) {

            //单个文字 解析&生成
            this->EqnParse();

            Literal* handle = this;
            while (in->TestInpTok(sep)) {
                in->NextToken();
                //此处过滤项list = EqnParse(in, bank);
                handle->next = new Literal();
                handle->next->EqnParse();
                handle = handle->next;
            }
        }
        //return list;
    }

    /*---------------------------------------------------------------------*/
    /*                  Member Function-[private]                           */
    /*---------------------------------------------------------------------*/
private:

    /*Parse an equation with optional external sign and depending on wether FOF or CNF is being parsed.*/
    bool eqn_parse_real(TermCell * *lref, TermCell * *rref, bool fof);

    /*Parse a literal without external sign assuming that _all_equational literals are prefix. 
     * Return sign. This is for TPTP*/
    bool eqn_parse_prefix(TermCell * *lref, TermCell * *rref);

    /*Parse a literal without external sign, allowing both infix and prefix notations (this is for mixed LOP)*/
    bool eqn_parse_mixfix(TermCell * *lref, TermCell * *rref);

    /* Parse a literal without external sign assuming that _all_equational literals are infix. Return sign. 
     * This is for TSTP syntax and E-LOP style.*/
    bool eqn_parse_infix(TermCell* *lref, TermCell* *rref);
public:
    /*---------------------------------------------------------------------*/
    /*                  Member Function-[public]                           */
    /*---------------------------------------------------------------------*/
    //    

    void EqnParse() {

        Term_p lt = nullptr, rt = nullptr;
        bool positive = eqn_parse_real(&lt, &rt, false);
        EqnAlloc(lt, rt, positive);
    }

    void EqnAlloc(Term_p lt, Term_p rt, bool positive) {

        TermBank* bank = Env::getTb();

        this->pos = 0;
        this->properties = EqnProp::EPNoProps;

        if (positive) { //设置正文字属性
            EqnSetProp(EqnProp::EPIsPositive);
        }
        if (rt != bank->trueTerm) {//设置等词属性
            assert(rt->fCode != (FunCode) DerefType::TRUECODE);
            EqnSetProp(EqnProp::EPIsEquLiteral);
        } else {//非等词文字

            assert(rt->TermCellQueryProp(TermProp::TPPredPos));


            /*printf("# lterm->f_code: %ld <%s>\n", lterm->f_code,
              SigFindName(bank->sig,lterm->f_code));
              SigPrint(stdout,bank->sig);
              fflush(stdout); */
            assert(!lt->IsVar());

            /* TermPrint(stdout, lterm, bank->sig, DEREF_NEVER);
            printf("===");
            TermPrint(stdout, rterm, bank->sig, DEREF_NEVER);
            printf("\n"); */


            assert(Env::getSig()->SigQueryFuncProp(lt->fCode, FPPredSymbol));
            lt->TermCellSetProp(TermProp::TPPredPos);
            if (Env::getSig()->SigQueryFuncProp(lt->fCode, FPPseudoPred)) {
                EqnSetProp(EqnProp::EPPseudoLit);
            }
        }

        this->next = nullptr;
        this->lterm = lt;
        this->rterm = rt;

#ifndef NDEBUG
        //   EqnPrint(stdout, false, true);
        //  printf("\n");
#endif
    }
    /*****************************************************************************
     * Print a literal in TSTP format.
     ****************************************************************************/
    void EqnTSTPPrint(FILE* out, bool fullterms);

    Literal* EqnListCopyDisjoint();
    Literal* EqnListFlatCopy();
    Literal* EqnCopyDisjoint();

    Literal* EqnCopy();
    Literal* EqnFlatCopy();
    Literal* EqnCopyOpt();

    /*---------------------------------------------------------------------*/
    /*                          Static Function                            */
    /*---------------------------------------------------------------------*/
    static void EqnListFree(Literal* lst);


};

#endif /* LITERAL_H */

