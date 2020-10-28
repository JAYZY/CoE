/* 
 * File:   Wformula.h
 * Author: Zhong Jian<77367632@qq.com>
 *
 * Created on 2018年4月23日, 下午4:24
 */

#ifndef WFORMULA_H
#define WFORMULA_H
#include "Global/IncDefine.h"
#include "CLAUSE/Clause.h"

//#include "Literal.h"
//#include "INOUT/Scanner.h" 
using namespace std;
/*---------------------------------------------------------------------*/
/*                   【Wformula】公式集相关枚举                           */
/*---------------------------------------------------------------------*/

/* 枚举 -- 子句属性类型 */
enum class WFormulaProp {
    WPIgnoreProps = 0, /* For masking properties out */
    WPInitial = 1, /* Input formula */
    WPInputFormula = (int) ClauseProp::CPInputFormula, /* _Really_ initial in TSTP sense */
    WPIniORInput = WFormulaProp::WPInitial | WFormulaProp::WPInputFormula,
    WPType1 = (int) ClauseProp::CPType1, /* 128 */
    WPType2 = (int) ClauseProp::CPType2,
    WPType3 = (int) ClauseProp::CPType3,
    WPTypeMask = (int) ClauseProp::CPTypeMask,
    WPTypeUnknown = 0, /* Also used as wildcard */
    WPTypeAxiom = (int) ClauseProp::CPTypeAxiom, /* Formula is Axiom */
    WPTypeHypothesis = (int) ClauseProp::CPTypeHypothesis, /* Formula is Hypothesis */
    WPTypeConjecture = (int) ClauseProp::CPTypeConjecture, /* Formula is Conjecture */
    WPTypeLemma = (int) ClauseProp::CPTypeLemma, /* Formula is Lemma */
    WPTypeNegConjecture = (int) ClauseProp::CPTypeNegConjecture, /* Formula is NegConjecture */
    WPTypeQuestion = (int) ClauseProp::CPTypeQuestion,
//    WPIsProofClause = (int) ClauseProp::CPIsProofClause,
//    WPIsRelevant = (int) ClauseProp::CPIsRelevant
};
typedef Term_p TFormula_p;

class FormulaSet;

class WFormula {
public:
    static long FormulaIdentCounter;
    static bool FormulaTermEncoding;
    static long FormulaDefLimit;

public:
    WFormulaProp properties;
    long ident;
    TermBank_p terms;
    ClauseInfo* info;
    // PStack_p derivation;
    Term_p tformula;
    FormulaSet* set; /* Is the formula in a set? */
    //为了实现链表功能
    WFormula* pred; /* For fomula sets = doubly  */
    WFormula* succ; /* linked lists */
public:
    WFormula();
    WFormula(TermBank_p terms, TFormula_p formula);
    WFormula* WFormulaParse(Scanner* in, TermBank_p terms);
    WFormula(const WFormula& orig);
    virtual ~WFormula();
public:
    /*---------------------------------------------------------------------*/
    /*                       Inline  Function                              */
    /*---------------------------------------------------------------------*/
    //

    inline void FormulaSetProp(WFormulaProp prop) {
        SetProp(this->properties, prop);
    }

    inline void FormulaDelProp(WFormulaProp prop) {
        DelProp(this->properties, (prop));
    }

    inline WFormulaProp FormulaGiveProps(WFormulaProp prop) {
        return GiveProps(this->properties, (prop));
    }

    inline void FormulaSetType(WFormulaProp type) {
        FormulaDelProp(WFormulaProp::WPTypeMask);
        FormulaSetProp((type));
    }

    /*---------------------------------------------------------------------*/
    /*                  Member Function-[public]                           */
    /*---------------------------------------------------------------------*/
    void WFormulaTPTPParse(Scanner* in, TermBank_p terms);
    void WFormulaTSTPParse(Scanner* in, TermBank_p terms);
    bool WFormulaSimplify(TermBank_p tbs);

    /*****************************************************************************
     *                  [TFormula_p]相关方法
     ****************************************************************************/

    inline bool TFormulaEqual(TermCell* f1, TermCell* f2) {
        return ((f1) == (f2));
    }
    //是否是谓词符号(文字判断)

    inline bool TFormulaIsLiteral(Sigcell* sig, TFormula_p form) {
        return sig->SigIsPredicate((form)->fCode);
    }
    //是否是函数符号(子项>1)

    inline bool TFormulaHasSubForm1(Sigcell* sig, TFormula_p form) {
        return sig->SigQueryFuncProp((form)->fCode, FPFOFOp) && ((form)->arity >= 1);
    }
    //是否是函数符号(子项>2)

    inline bool TFormulaHasSubForm2(Sigcell* sig, TFormula_p form) {
        return (sig->SigQueryFuncProp((form)->fCode, FPFOFOp) && ((form)->arity >= 2));
    }
    //是否是量词

    inline bool TFormulaIsQuantified(Sigcell* sig, TFormula_p form) {
        return ((form)->fCode == sig->qexCode || (form)->fCode == sig->qallCode);
    }

    inline bool TFormulaIsPropTrue(Sigcell* sig, TFormula_p form) {
        return TFormulaIsPropConst(sig, form, true);
    }

    inline bool TFormulaIsPropFalse(Sigcell* sig, TFormula_p form) {
        return TFormulaIsPropConst(sig, form, false);
    }

    //Return a formula equivalent to ~form. If form is of the form ~f,return f, otherwise ~form.

    inline TFormula_p TFormulaNegAlloc(TermBank_p terms, TFormula_p form) {
        if (form->fCode == Env::getSig()->notCode) {
            return form->args[0];
        }
        return TFormulaFCodeAlloc(terms, Env::getSig()->notCode, form, nullptr);
    }
    //   Allocate a formula representing a propositional constant (true or false). 


    bool TFormulaIsPropConst(Sig_p sig, TFormula_p form, bool positive);
    bool TFormulaVarIsFree(TermBank_p bank, TFormula_p form, Term_p var);
    TFormula_p TFormulaPropConstantAlloc(TermBank_p tbs, bool positive);

    TFormula_p tprop_arg_return_other(Sig_p sig, TFormula_p arg1, TFormula_p arg2, bool positive);
    TFormula_p tprop_arg_return(Sig_p sig, TFormula_p arg1, TFormula_p arg2, bool positive);




    TFormula_p TFormulaTPTPParse(Scanner* in, TermBank_p terms);
    TFormula_p TFormulaTSTPParse(Scanner* in, TermBank_p terms);
    TFormula_p TFormulaSimplify(TermBank_p terms, TFormula_p form);

private:


    /// Parse an elementary formula in TPTP/TSTP format.
    /// \param in  I/O
    /// \return 
    TFormula_p elem_tform_tptp_parse(Scanner* in, TermBank_p terms);
    TFormula_p literal_tform_tstp_parse(Scanner* in, TermBank_p terms);
    ///Parse a sequence of formulas connected by a single AC operator and return it.    
    TFormula_p assoc_tform_tstp_parse(Scanner* in, TermBank_p terms, TFormula_p head);

    /// Parse and return a TPTP quantor. Rather trivial ;-)
    /// \param in  Input
    /// \return 
    FunCode tptp_quantor_parse(Scanner* in);


    /// R eturn the f_code corresponding to a given token. Rather trivial ;-)
    /// \param tok
    /// \return 

    FunCode tptp_operator_convert(TokenType tok);


    /// Parse a TPTP operator and return the corresponding f_code. Rather trivial ;-) 
    /// \param in
    /// \return 

    FunCode tptp_operator_parse(Scanner* in);

    /// Parse a quantified TPTP/TSTP formula. At this point, the quantor
    //   has already been read (and is passed into the function), and we
    //   are at the first (or current) variable.
    /// \param in
    /// \param terms
    /// \param quantor
    /// \return 
    TFormula_p quantified_tform_tptp_parse(Scanner* in, TermBank_p terms, FunCode quantor);
    TFormula_p quantified_tform_tstp_parse(Scanner* in, TermBank_p terms, FunCode quantor);

    ///  Allocate a formula given an f_code and two subformulas (the second one may be NULL).
    /// \param bank
    /// \param op
    /// \param arg1
    /// \param arg2
    /// \return 
    TFormula_p TFormulaFCodeAlloc(TermBank_p bank, FunCode op, TFormula_p arg1, TFormula_p arg2);


    /// Allocate a literal term formula. The equation is _not_ freed!
    /// \param literal
    /// \return 

    TFormula_p TFormulaLitAlloc(Literal* literal) {
        assert(literal);
        return literal->EqnTermsTBTermEncode();
    }


};
#endif /* WFORMULA_H */

