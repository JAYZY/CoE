/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Sigcell.h
 * Author: cf 
 *
 * Created on 2017年3月6日, 上午10:35
 */

#ifndef SIGCELL_H
#define SIGCELL_H

#include "Global/IncDefine.h"

#include "BASIC/SplayTree.h"
#include "BASIC/TreeNodeDef.h"
#include "INOUT/Scanner.h"
//class Clause;
/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */

/*---------------------------------------------------------------------*/
typedef enum {
    FPIgnoreProps = 0, /* No properties, mask everything out */
    FPPredSymbol = 1, /* Symbol is a transformed predicate symbol */
    FPFuncSymbol = 2, /* Symbol is a real function symbol */
    /* If neither is set, we don't know it yet */
    FPFOFOp = 4, /* Symbol is encoded first order operator */
    FPSpecial = 8, /* Symbol is a special symbol introduced internally(内部引入的特殊符号) */
    FPAssociative = 16, /* Function symbol is binary and associative(结合律) */
    FPCommutative = 32, /* Function symbol is binary and commutates(交换律) */
    FPIsAC = FPAssociative | FPCommutative,
    FPInterpreted = 64, /* Interpreted symbol $ident */
    FPIsInteger = 128, /* Sequence of digits, may be semi-interpreted */
    FPIsRational = 256, /* [-]a/b */
    FPIsFloat = 512, /* Floating point number */
    FPIsObject = 1024, /* ""-enclosed string, by definition denotes
                            unique object." */
    FPDistinctProp = FPIsObject | FPIsInteger | FPIsRational | FPIsFloat,
    FPOpFlag = 2048, /* Used for temporary oerations, by
                           * defintion off if not in use! */
    FPClSplitDef = 4096, /* Predicate is a clause split defined
                           * symbol. */
    FPPseudoPred = 8192 /* Pseudo-predicate used for side effects
                           * only, does not conceptually contribute to
                           * truth of clause */
} FunctionProperties;

class FuncCell {
public:
    /* fCode 代表了项在数组中的位置.- f_code is implicit by position in the array    */
    string name;
    int arity;
    int alphaRank; /* We sometimes need an arbitrary but stable order on symbols and use alphabetic. */
    FunctionProperties properties;

    FuncCell(const string& n1, int ari, int alpha, FunctionProperties funProp) :
    name(n1), arity(ari), alphaRank(alpha), properties(funProp) {
    }
};
typedef FuncCell *Func_p;



/*---------------------------------------------------------------------*/
/*                             宏定义                                   */
/*---------------------------------------------------------------------*/

//#define DEFAULT_SIGNATURE_SIZE 16
//#define DEFAULT_SIGNATURE_GROW 2

/* Special constant for internal operations */
//#define SIG_TRUE_CODE  1
//#define SIG_FALSE_CODE 2
//#define SIG_NIL_CODE   3
//#define SIG_CONS_CODE  4


/* Tokens that always are stand-alone identifiers */
//#define FuncSymbToken  (Identifier | SemIdent | SQString | String)

/* Tokens that may start a (composite or atomic) identifier */
//#define FuncSymbStartToken (Identifier | SemIdent | SQString | PosInt | String | Plus | Hyphen) 


class Scanner;
class Clause;

class Sigcell {
public:
    static bool SigSupportLists;
    bool alphaRanksValid; /* The alpha-ranks are up to date */
    //long                   size;                         /* 数组大小,no need!*/
    //FunCode                fCount;                       /* 数组的存储的元素个数.Largest used f_code */
    FunCode internalSymbols; /* Largest auto-inserted internal symbol */
    vector<Func_p> fInfo; /* 函数项数组,注意:fInfo[0]=null, 因此每个元素fCode对应数组下标,且不能为0 */
    SplayTree<StrTreeCell> fIndex; /* Back-assoc:通过函数项名称查找它的索引index */
    vector<Clause*> acAxioms; /* All recognized AC axioms */

    /* The following are special symbols needed for pattern manipulation. 
     * We want very efficient access to them! Also resused in FOF parsing. */
    FunCode eqnCode;
    FunCode neqnCode;
    FunCode cnilCode;
    vector<IntOrP> ornCodes;
    /* The following is for encoding first order formulae as terms. 
     * I do like to reuse the robust sharing infrastructure for CNFization and formula rewriting 
     * (inspired by Tommi Juntilla's reuse of the same in MathSAT). */
    FunCode notCode;
    FunCode qexCode;
    FunCode qallCode;
    FunCode andCode;
    FunCode orCode;
    FunCode implCode;
    FunCode equivCode;
    FunCode nandCode;
    FunCode norCode;
    FunCode bimplCode;
    FunCode xorCode;
    /* And here are codes for interpreted symbols */
    FunCode answerCode; /* For answer literals */
    /* Counters for generating new symbols 计数变量-新生成的符号统计*/
    long skolemCount;
    long newpredCount;
    /* Which properties are used for recognizing implicit distinctness?*/
    FunctionProperties distinctProps;

public:



    /*---------------------------------------------------------------------*/
    /*                    Constructed Function                             */
    /*---------------------------------------------------------------------*/
    Sigcell();
    Sigcell(const Sigcell& orig);
    virtual ~Sigcell();

    /*---------------------------------------------------------------------*/
    /*                        Inline functions                             */

    /*---------------------------------------------------------------------*/
    inline FunCode fCount() {
        return fInfo.size() - 1;
    }

    /*属性相关操作 设置 删除 查询*/
    inline void SigSetFuncProp(FunCode fcode, int prop) {
        Func_p fp = fInfo[fcode];
        SetProp(fp->properties, prop);
    }

    inline void SigDelFuncProp(FunCode fcode, FunctionProperties prop) {
        Func_p fp = fInfo[fcode];
        DelProp(fp->properties, prop);
    }

    /* 查询某个属性是否存在 */
    inline bool SigQueryFuncProp(FunCode fcode, FunctionProperties prop) {
        Func_p fp = fInfo[fcode];
        return QueryProp(fp->properties, prop); //2017-03-21 wgf add ->properties
    }

    /* 在属性集合中,返回查询的属性 */
    inline int FuncIsAnyPropSet(FunCode fcode, int prop) {
        Func_p fp = fInfo[fcode];
        return IsAnyPropSet(fp->properties, prop); //2017-03-21 wgf add ->properties
    }

    inline FunCode SigExternalSymbols() {
        return fCount() - internalSymbols;
    }

    //是否为常元项

    inline bool SigIsFunConst(FunCode fcode) {
        return SigFindArity(fcode) == 0 && SigIsPredicate(fcode);
    }

    /* 是否为简单的 answerPred ?*/
    inline bool SigIsSimpleAnswerPred(FunCode fcode) {
        return fcode == answerCode;
    }

    /* Return the value of the Function field for a function symbol. 
     * 查询该元素是否是一个函数元素.     */
    inline bool SigIsFunction(FunCode fcode) {
        assert(fcode > 0);
        assert(fcode <= fCount());
        return SigFindArity(fcode) > 0 && SigQueryFuncProp(fcode, FPFuncSymbol);
    }

    /* Return the value of the predicate field for a function symbol.
     * 查询该元素是否是一个谓词元素.*/
    inline bool SigIsPredicate(FunCode f_code) {
        assert(f_code > 0); //确保不是变元
        assert(f_code <= fCount()); //确保一定是存在的
        return SigQueryFuncProp(f_code, FPPredSymbol);
    }

    /*  Return the value of the special field for a function symbol.
     * 查询该元素是否是一个特殊项 */

    inline bool SigIsSpecial(FunCode f_code) {
        assert(f_code > 0);
        assert(f_code <= fCount());
        return SigQueryFuncProp(f_code, FPSpecial);
    }

    /*****************************************************************************
     *  给定一个函数项的fCode,返回fInfo数组中所在函数项的arity.
     ****************************************************************************/
    inline int SigFindArity(FunCode fCode) {
        assert(fCode > 0);
        assert(fCode <= fCount());
        return (fInfo[fCode])->arity;
    }

    /*****************************************************************************
     * 给定一个函数项的fCode,返回fInfo数组中所在函数项的name.
     ****************************************************************************/
    inline void SigFindName(FunCode fCode, string& rtnName) {
        assert(fCode > 0);
        assert(fCode <= fCount());
        rtnName = fInfo[fCode]->name;
    }

    /*****************************************************************************
     * Return the FunCode for $eq or $neq, create them if non-existant. 
     ****************************************************************************/
    inline FunCode SigGetEqnCode(bool positive) {

        if (positive) {
            if (eqnCode) {
                return eqnCode;
            }
            eqnCode = SigInsertId("$eq", 2, true);
            assert(eqnCode);
            SigSetPredicate(eqnCode, true);
            return eqnCode;
        } else {
            if (neqnCode) {
                return neqnCode;
            }
            neqnCode = SigInsertId("$neq", 2, true);
            assert(neqnCode);
            SigSetPredicate(neqnCode, true);
            return neqnCode;
        }
    }

    /***************************************************************************** 
     * As above, for $or
     ****************************************************************************/
    inline FunCode SigGetOrCode() {
        if (orCode) {
            return orCode;
        }
        orCode = SigInsertId("$or", 2, true);
        assert(orCode);
        return orCode;
    }

    /***************************************************************************** 
     * As above, for $cnil
     ****************************************************************************/
    inline FunCode SigGetCNilCode() {

        if (cnilCode) {
            return cnilCode;
        }
        cnilCode = SigInsertId("$cnil", 0, true);
        assert(cnilCode);
        return cnilCode;
    }


    /*---------------------------------------------------------------------*/
    /*                        member functions                             */
    /*---------------------------------------------------------------------*/

    /*将所有FOF操作放入 sig中,前提条件:sig中为空.*/
    void SigInsertInternalCodes();
    /*计算所有符号元素的 alpha-Rank.*/
    void SigComputeAlphaRanks();

    void SigPrint(FILE* out);
    void SigPrintSpecial(FILE* out);
    void SigPrintACStatus(FILE* out); /*输出AC状态符号,特殊符号*/
    void SigPrintOperator(FILE* out, FunCode op, bool comments); /*输出操作符,特殊符号*/


    /* 将所有元素设置为特殊项属性.  * isDel - 是否删除该属性 */
    void SigSetAllSpecial(bool isDel);
    /* 将一个元素设置为谓词项属性.  * isDel - 是否删除该属性 */
    void SigSetPredicate(FunCode f_code, bool isDel);
    /* 将一个元素设置为函数项属性.  * isDel - 是否删除该属性 */
    void SigSetFunction(FunCode f_code, bool isDel);
    /* 将一个元素设置为特殊项属性.  * isDel - 是否删除该属性 */
    void SigSetSpecial(FunCode f_code, bool isDel);



    /* 给定一个fCode,查询对应元素(项)的 alphaRank; */
    int SigGetAlphaRank(FunCode f_code);
    int SigFindMaxUsedArity();
    int SigFindMaxPredicateArity();
    int SigFindMinPredicateArity();
    int SigFindMaxFunctionArity();
    int SigFindMinFunctionArity();
    int SigCountAritySymbols(int arity, bool predicates);
    int SigCountSymbols(bool predicates);
    int SigAddSymbolArities(vector<IntOrP*> distrib, bool predicates, long selection[]);

    /* 根据元素名称,伸展树进行查找,找到返回元素的索引,否则返回0 */
    FunCode SigFindFCode(const string& name);
    FunCode SigInsertId(const string& name, int arity, bool special_id);
    FunCode SigInsertFOFOp(const string& name, int arity);
    FunCode SigParseKnownOperator(Scanner* in);
    FunCode SigParseSymbolDeclaration(Scanner* in, bool special_id);
    FunCode SigParse(Scanner* in, bool special_ids);
    /* Special functions for dealing with special symbols */
    FunCode SigGetOtherEqnCode(FunCode f_code);
    FunCode SigGetOrNCode(int arity);
    FunCode SigGetNewSkolemCode(int arity);
    FunCode SigGetNewPredicateCode(int arity);

    static inline FunCode SigGetEqnCode(Sigcell* sig, bool positive) {
        assert(sig);
        if (positive) {
            if (sig->eqnCode) {
                return sig->eqnCode;
            }
            sig->eqnCode = sig->SigInsertId("$eq", 2, true);
            assert(sig->eqnCode);
            sig->SigSetPredicate(sig->eqnCode, true);
            return sig->eqnCode;
        } else {
            if (sig->neqnCode) {
                return sig->neqnCode;
            }
            sig->neqnCode = sig->SigInsertId("$neq", 2, true);
            assert(sig->neqnCode);
            sig->SigSetPredicate(sig->neqnCode, true);
            return sig->neqnCode;
        }
    }
};
/*****************************************************************************
 * 增加 Sigcell指针定义
 ****************************************************************************/
typedef Sigcell* Sig_p;
#endif /* SIGCELL_H */

