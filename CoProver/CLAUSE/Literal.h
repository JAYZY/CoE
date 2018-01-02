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
    EPDominates = EPIsDominated, /* Double use of this property in potentially maximal or minimal clauses */
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
private:
    EqnProp properties; /*prositive ,maximal,equational */
    int pos;
    TermCell* lterm; /*左文字*/
    TermCell* rterm; /*等号右边文字,若非等词,则为$True;
    //TermBank ss;
    //TermBank* bank; /* Terms are from this bank */
    Literal* next;

public:
    Literal();
    Literal(const Literal& orig);
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
    void EqnListParse(TokenType sep) {
        //TokenType FuncSymbStartToken = Identifier | SemIdent | SQString | String | PosInt | Plus | Hyphen;
        //TokenType TermStartToken =Clause::SigSupportLists?(FuncSymbStartToken|OpenSquare|Mult):(FuncSymbStartToken|Mult);
        Scanner* in = Env::getIn();
        TB_p bank = Env::getTb();
        TokenType testTok = (TokenType) ((uint64_t) TokenCell::TermStartToken() | (uint64_t) TokenType::TildeSign);

        if (((in->format == IOFormat::TPTPFormat) && in->TestInpTok(TokenType::SymbToken)) ||
                ((in->format == IOFormat::LOPFormat) && in->TestInpTok(testTok)) ||
                ((in->format == IOFormat::TSTPFormat) && in->TestInpTok(testTok))) {

            //文字解析
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
    /*                  Member Function-[public]                           */
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
    // Function: EqnParse()
    // 过滤EqnParse(Scanner_p in, TB_p bank)

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
            assert(rt->fCode != (FunCode)DerefType::TRUECODE);
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


            assert(bank->sig->SigQueryFuncProp(lt->fCode, FPPredSymbol));
            lt->TermCellSetProp(TermProp::TPPredPos);
            if (bank->sig->SigQueryFuncProp(lt->fCode, FPPseudoPred)) {
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

};

#endif /* LITERAL_H */

