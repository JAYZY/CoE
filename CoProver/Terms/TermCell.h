/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   TermCell.h
 * Author: zj 
 *
 * Created on 2017年3月22日, 下午4:10
 */

#ifndef TERMCELL_H
#define TERMCELL_H



#include "Sigcell.h"
#include <stack>
#include <stdint.h>
#include <bits/stdint-uintn.h>
#include "HEURISTICS/StrategyParam.h"

enum class DerefType : uint8_t {
    TRUECODE = 1,
    FLASECODE = 2,
    NILCODE = 3,
    CONSCODE = 4,
    DEREF_NEVER = 0,
    DEREF_ONCE,
    DEREF_ALWAYS
};

enum class TermEqulType : uint8_t {
    PtrEquel,
    StructEqual
};

/* 项属性枚举定义[TermProperties] */
enum class TermProp : int32_t {
    TPIgnoreProps = 0, /* For masking properties out */
    TPRestricted = 1, /* Rewriting is restricted on this term */
    TPTopPos = 2, /* This cell is a entry point */
    TPIsGround = 4, /* Shared term is ground */
    TPPredPos = 8, /* This is an original predicate　position morphed into a term */
    TPIsRewritable = 16, /*TermCell is known to be rewritable with respect to a current rule or rule
                          set. Used for removing backward-rewritable clauses. Absence of
                                   this flag does not mean that the term is in any kind of normal form! POWNRS */
    TPIsRRewritable = 32, /* TermCell is rewritable even if rewriting is restricted to proper
                            instances at the top level.*/
    TPIsSOSRewritten = 64, /* TermCell has been rewritten with a SoS clause (at top level) */
    TPSpecialFlag = 128, /* For internal use with normalizing variables*/
    TPOpFlag = 256, /* For internal use */
    TPCheckFlag = 512, /* For internal use */

    TPOutputFlag = 1024, /* Has this term already been printed (and thus defined)? */

    TPIsSpecialVar = 2048, /* Is this a meta-variable generated by term
                                   top operations and the like? */
    TPIsRewritten = 4096, /* TermCell has been rewritten (for the new
                                   rewriting scheme) */
    TPIsRRewritten = 8192, /* TermCell has been rewritten at a subterm position or with a real
                              instance (for the new rewriting scheme) */
    TPIsShared = 16384, /* TermCell is in a term bank */
    TPGarbageFlag = 32768, /* For the term bank garbage collection */
    TPIsFreeVar = 65536, /* For Skolemization */
    TPPotentialParamod = 131072, /* This position needs to be tried for paramodulation */
    TPPosPolarity = 1 << 18, /* In the term encoding of a formula,this occurs with positive polarity. */
    TPNegPolarity = 1 << 19, /* In the term encoding of a formula,this occurs with negative polarity. */

    TPShareGround = (TermProp::TPIsShared | TermProp::TPIsGround),
};


/*---------------------------------------------------------------------*/
/*                              宏定义                                  */
/*---------------------------------------------------------------------*/

#define DEFAULT_VWEIGHT  1  /* This has to be an integer > 0! */
#define DEFAULT_FWEIGHT  2  /* This has to be >= DEFAULT_VWEIGHT */
/*TermFunc.h中相关宏定义*/

#define TERMS_INITIAL_ARGS 10

#define RewriteAdr(level) (assert(level),(level)-1)

#define REWRITE_AT_SUBTERM 0  

#define REWRITE_AT_SUBTERM 0



class Clause;
class TermBank;
class VarBank;

class TermCell {
private:

    //   typedef struct {
    //        long nf_date[FullRewrite]; /* If term is not rewritten,it is in normal form with respect to the demodulators at this date */
    //
    //        struct {
    //            TermCell* replace; /* ...otherwise, it has been
    //                                             rewritten to this term */
    //            // long               demod_id;        /* 0 means subterm! */
    //            Clause* demod; /* NULL means subterm! */
    //        } rw_desc;
    //    } RewriteState;

public:
    static bool TermPrintLists;
    FunCode fCode; /* Top symbol of term */
    TermProp properties; /* Like basic, lhs, top */
    uint32_t claId; //增加项所在子句编号,主要用于变元项的识别
    int arity; /* Redundant, but saves handing around the signature all the time */
    TermCell* *args; /* Pointer to array of arguments */
    TermCell* binding; /* For variable bindings,potentially for temporary a rewrites 
                    * - it might be possible to combine the previous two in a union. */
    long entryNo; /* Counter for terms in a given termbank - needed for administration and external representation */
    long weight; /* Weight of the term, if term is in term bank */

    //float zjweight;
    // RewriteState rw_data; /* See above */
    TermCell* lson; /* For storing shared term nodes in */
    TermCell* rson; /* a splay tree - see cte_termcellstore.[ch] */

    uint16_t uVarCount; //变元计数
    //FunCode hashIdx; //hash列表中存储的index(hash值)

private:
    static TermCell* parse_cons_list(Scanner* in, TermBank* tb);
    TermCell* term_check_consistency_rek(SplayTree<PTreeCell> &branch, DerefType deref);
    void print_cons_list(FILE* out, DerefType deref);
public:
    /*---------------------------------------------------------------------*/
    /*                    Constructed Function                             */
    /*---------------------------------------------------------------------*/
    TermCell();
    /*构造函数 - 创建一个constant term 如　ａ ,b */
    TermCell(long symbol);
    /*构造函数 - 创建一个function term 函数符如　f */
    TermCell(long fCode, int arity);
    /*构造函数,copy TermCell*/
    TermCell(TermCell& orig);
    virtual ~TermCell(); //代替 TermFree方法

    static void TermFree(TermCell* junk) {
        assert(junk);
        if (!junk->IsVar()) {
            assert(!junk->TermCellQueryProp((TermProp) ((int32_t) TermProp::TPIsShared)));
            if (junk->arity) {
                int i;

                assert(junk->args);
                for (i = 0; i < junk->arity; i++) {
                    TermFree(junk->args[i]);
                }
            } else {
                assert(!junk->args);
            }
            DelPtr(junk);
        }
    }

    bool operator<(const TermCell* term)const //重载运算符
    {
        return this->fCode - term->fCode;

    }
    /* 删除函数项 */
private:
    void TermTopFree();
public:



    /*---------------------------------------------------------------------*/
    /*                          inline Function                            */
    /*---------------------------------------------------------------------*/
    //

    inline void TermCellSetProp(TermProp prop) {
        SetProp(this->properties, prop);
    }

    inline void TermCellDelProp(TermProp prop) {
        DelProp(this->properties, (prop));
    }

    inline void TermCellAssignProp(TermProp sel, TermProp prop) {
        AssignProp((properties), sel, prop);
    }

    inline bool TermCellQueryProp(TermProp prop) {
        return QueryProp((properties), (prop));
    }

    inline bool TermCellIsAnyPropSet(TermProp prop) {
        return IsAnyPropSet((properties), (prop));
    }

    inline TermProp TermCellGiveProps(TermProp props) {
        return GiveProps((properties), (props));
    }

    inline void TermCellFlipProp(TermProp props) {
        FlipProp(properties, props);
    }

    inline bool IsVar() {
        return fCode < 0;
    } /*is variable?*/

    inline bool IsConst() {
        return (!(fCode < 0)) && (arity == 0);
    }

    inline bool IsFunc() {
        return ((fCode > 0)) && (arity > 0);
    }

    inline bool IsShared() {
        return TermCellQueryProp(TermProp::TPIsShared);
    }

    inline bool TermIsRewritten() {
        return TermCellQueryProp(TermProp::TPIsRewritten);
    }

    inline bool TermIsRRewritten() {
        return TermCellQueryProp(TermProp::TPIsRRewritten);
    }

    //    inline long TermNFDate(int i) {
    //        return TermIsRewritten() ? 0 : rw_data.nf_date[i];
    //    }

    long TermCollectPropVariables(SplayTree<PTreeCell> *tree, TermProp prop);

    inline long TermStandardWeight() {
        return this->IsGround() ? weight : TermWeight(DEFAULT_VWEIGHT, DEFAULT_FWEIGHT);
    }

    /* Return the depth of a term. */
    inline long TermDepth() {
        long maxdepth = 0, ldepth;
        for (int i = 0; i < arity; ++i) {
            ldepth = args[i]->TermDepth();
            maxdepth = MAX(maxdepth, ldepth);
        }
        return maxdepth + 1;
    }
    //检查最大函数嵌套层限制. >0 -- 符合限制 -1 -- 不符合限制

    inline int CheckFuncLayerLimit() {
        long maxdepth = 0, ldepth;
        for (int i = 0; i < arity; ++i) {
            TermCell* term=TermCell::TermDerefAlways(args[i]);
            ldepth =term->CheckFuncLayerLimit();
            if(-1==ldepth) return -1;
            if (maxdepth < ldepth) {
                maxdepth = ldepth;
                if (maxdepth > StrategyParam::R_MAX_FUNCLAYER)
                    return -1;
            }
        }
        return  maxdepth + 1;
    }

    /*拷贝子项中的内容　args--Return a copy of the argument array of source. */
    inline TermCell** TermArgListCopy() {
        TermCell* *handle;
        if (arity) {
            handle = new TermCell*[arity];
            for (int i = 0; i < arity; ++i) {
                handle[i] = args[i];
            }
        } else {
            handle = nullptr;
        }
        return handle;
    }

    inline const char* getVarName(string &termName, string splitCh = "") {
        termName = to_string(this->claId) + splitCh;
        termName += to_string(-((fCode - 1) / 2));
        return termName.c_str();
    }
    //#define TermIsTopRewritten(term) (TermIsRewritten(term)&&TermRWDemodField(term))

    //    inline bool TermIsTopRewritten() {
    //        return TermIsRewritten() && TermRWDemodField();
    //    }

    /* Get the logical value of the replaced term / demodulator */
    //#define TermRWReplace(term) (TermIsRewritten(term)?TermRWTargetField(term):nullptr)

    //    inline bool TermRWReplace() {
    //        return TermIsRewritten() ? TermRWTargetField() : nullptr;
    //    }
    //#define TermRWDemod(term) (term->TermIsRewritten()?TermRWDemodField(term):nullptr)

    //    inline Clause* TermRWDemod() {
    //        return TermIsRewritten() ? TermRWDemodField() : nullptr;
    //    }

    /* Absolutely get the value of the replace and demod fields */
    //#define TermRWReplaceField(term) ((term)->rw_data.rw_desc.replace)

    //    inline TermCell* TermRWReplaceField() {
    //        return rw_data.rw_desc.replace;
    //    }

    //#define TermRWDemodField(term)   ((term)->rw_data.rw_desc.demod)

    //    inline Clause* TermRWDemodField() {
    //        return rw_data.rw_desc.demod;
    //    }

    //TermBank

    /* 返回项标示:变元为fCode,非变元为entryNo-即termbank中的下标位置 */
    inline long TBCellIdent() {
        return IsVar() ? fCode : entryNo;
    }

    /* 返回项是否为特殊项 $true */
    inline bool TermIsTrueTerm() {
        return fCode == (FunCode) DerefType::TRUECODE; //SIG_TRUE_CODE;
    }

    /* 返回是否为t1的子项 */
    inline bool TBTermIsSubterm(TermCell* subT) {
        return TermIsSubterm(subT, DerefType::DEREF_NEVER, TermEqulType::PtrEquel);
    }

    /* 返回项是否为Ground项 */
    inline bool TBTermIsGround() {
        return TermCellQueryProp(TermProp::TPIsGround);
    }
    /* 检查项是否为基项 ground 不包括变元项 注:不检查　变元绑定情况 */
    bool IsGround();

    /* 返回项是否为TypeTerm 即权重==3 P(x)*/
    inline bool TBTermIsTypeTerm() {
        return weight == (DEFAULT_VWEIGHT + DEFAULT_FWEIGHT);
    }

    /* 返回项是否为XTypeTerm,全部是变元项 P(x,x,...,x)*/
    inline bool TBTermIsXTypeTerm() {
        return (arity > 0) && (weight == (DEFAULT_FWEIGHT + arity * DEFAULT_VWEIGHT));
    }


    /*---------------------------------------------------------------------*/
    /*                  Member Function-[public]                           */
    /*---------------------------------------------------------------------*/



    /* 变元项输出 */
    void VarPrint(FILE* out);
    void TermPrint(FILE* out, DerefType deref = DerefType::DEREF_ALWAYS);
    /**
     * 得到项的输出字符串
     */
    void getStrOfTerm(string&outStr, DerefType deref = DerefType::DEREF_ALWAYS);
    void PrintDerefAlways(FILE* out);

    void TermPrintArgList(FILE* out, int arity, DerefType deref);
    /**
     * 得到项的子项字符串
     */
    void getStrOfTermArgList(string&outStr, int arit, DerefType deref);
    void PrintTermSig(FILE* out);

    void TermSetProp(DerefType dt, TermProp prop);
    bool TermSearchProp(DerefType deref, TermProp prop);
    void TermDelProp(DerefType deref, TermProp prop);
    void TermVarSetProp(DerefType deref, TermProp prop);
    bool TermVarSearchProp(DerefType deref, TermProp prop);
    void TermVarDelProp(DerefType deref, TermProp prop);




    /* fcode 是否存该项的子项中,eg.判断a1,是否是f(a1,a2)的子项 */
    bool TermHasFCode(FunCode f);
    /* Return if the term contains unbound variables.Does not follow bindings. */
    bool TermHasUnboundVariables();
    /* Return true if t is of the form f(X1...Xn) with n>=arity.  */
    bool TermIsDefTerm(TermCell* term, int min_arity);
    /* Return true if t is of the form f(X1...Xn) with n>=arity. */
    bool TermIsDefTerm(int min_arity);
    //bool TermIsSubterm(TermCell* test, DerefType deref, TermEqualTestFun EqualTest);
    bool TermIsSubterm(TermCell* test, DerefType deref, TermEqulType EqualTest);
    bool TermIsStructSubterm(TermCell* test);


    FunCode TermFindMaxVarCode();
    TermCell* TermEquivCellAlloc(TermBank* tb);
    TermCell* renameCopy(TermBank* tb, DerefType deref = DerefType::DEREF_ALWAYS);
    TermCell* TermCopy(TermBank* tb, DerefType deref);
    TermCell* TermCopyKeepVars(DerefType deref);
    TermCell* TermCheckConsistency(DerefType deref);

    void TermAddSymbolDistExist(vector<long>& dist_array, stack<long>& exists);
    void TermAddSymbolDistributionLimited(vector<long> &dist_array, long limit);
    void TermAddSymbolFeaturesLimited(long depth, long *freq_array, long* depth_array, long limit);
    void TermAddSymbolFeatures(vector<long> &mod_stack, long depth, vector<long>&feature_array, long offset);
    void TermComputeFunctionRanks(long *rank_array, long *count);
    long TermLinearize(stack<TermCell*> &tmpStack);
    bool TermIsSubtermDeref(TermCell* test, DerefType deref_super, DerefType deref_test);


    long TermWeight(long vweight, long fweight);
    long TermFsumWeight(long vweight, long flimit, vector<long>&fweights, long default_fweight);
    long TermNonLinearWeight(long vlweight, long vweight, long fweight);
    long TermSymTypeWeight(long vweight, long fweight, long cweight, long pweight);
    //zj:replace.cpp 中相关的方法

    /* 添加项term的重写项replace以及相关子句  */
    //void TermAddRWLink(TermCell* replace, Clause* demod, bool sos, RWResultType type);
    /* 删除项term的重写项replace以及相关子句 replace Delete rewrite link from term.  */
    void TermDeleteRWLink();
    /* Return the last term in an existing rewrite link chain. */
    TermCell* TermFollowRWChain();
    long TBTermDelPropCount(TermProp prop);
    long TermCollectVariables(SplayTree<PTreeCell> *tree);
    long TermAddFunOcc(vector<int>*f_occur, vector<IntOrP>*res_stack);
    void UnpackTermPos(vector<IntOrP>& pos, long cpos);

    /// 这个比较采用不同termbank进行存储时候的项 
    /// \param term
    /// \return 
    bool equalStruct(TermCell* term);
    /*---------------------------------------------------------------------*/
    /*                         Static Function                             */
    /*---------------------------------------------------------------------*/

    /// 这个项相同比较,前提是所有项,均在一个 Termbank情况下;
    // 因此采用了地址相同来判断    
    static bool TermStructEqual(TermCell* t1, TermCell* t2);
    static bool TermStructEqualNoDeref(TermCell* t1, TermCell* t2);
    static bool TermStructEqualNoDerefHardVars(TermCell* t1, TermCell* t2);
    static bool TermStructEqualDeref(TermCell* t1, TermCell* t2, DerefType deref_1, DerefType deref_2);
    static bool TermStructEqualDerefHardVars(TermCell* t1, TermCell* t2, DerefType deref_1, DerefType deref_2);

    static int TermStructWeightCompare(TermCell* t1, TermCell* t2);
    static int TermLexCompare(TermCell* t1, TermCell* t2);
    //临时方法 -------------

    /*创建一个constant term 如　ａ ,b */
    static TermCell* TermConstCellAlloc(long symbol) {
        TermCell* t = new TermCell();
        t->weight = DEFAULT_FWEIGHT;
        t->fCode = symbol;
        return t;
    }

    /*创建一个function term 函数符如　f */
    static TermCell* TermTopAlloc(long f_code, int arity) {
        TermCell* res = new TermCell();

        res->fCode = f_code;
        res->arity = arity;
        if (arity) {
            res->args = new TermCell*[arity]; //创建一个动态二维数组
        }
        return res;
    }


    static TermCell* TermTopCopy(TermCell* t);

    /* 根据定义的类型－－决定term变元binding遍历的类型,返回项绑定的元素项 */
    static inline TermCell* TermDeref(TermCell* term, DerefType deref) {
        assert((term->IsVar()) || !(term->binding));
        if (deref == DerefType::DEREF_ALWAYS) {
            while (term->binding) {
                term = term->binding;
            }
        } else {
            int ideref = (int) deref;
            while (ideref) {
                if (!term->binding) {
                    break;
                }
                term = term->binding;
                --ideref;
            }
        }
        return term;
    }

    static inline TermCell* TermDerefAlways(TermCell* term) {
        assert((term->IsVar()) || !(term->binding));
        while (term->binding) {
            term = term->binding;
        }
        return term;

    }

    static FuncSymbType TermParseOperator(Scanner* in, string&idStr);
    static int TermParseArgList(Scanner* in, TermCell*** arg_anchor, TermBank* tb);
    static TermCell* TermParse(Scanner* in, TermBank* tb);


    static FunCode TermSigInsert(Sigcell* sig, const string &name, int arity, bool special_id, FuncSymbType type);

};
typedef TermCell *Term_p;
#endif /* TERMCELL_H */

