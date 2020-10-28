/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Wformula.cpp
 * Author: Zhong Jian<77367632@qq.com>
 * 
 * Created on 2018年4月23日, 下午4:24
 */

#include "WFormula.h"
#include "LIB/Out.h"

WFormula::WFormula() {
    this->properties = WFormulaProp::WPIgnoreProps;
    this->ident = 0;
    this->terms = nullptr;
    this->info = nullptr;
    //this->derivation = NULL;
    this->tformula = nullptr;
    //this->set        = NULL;
    this->pred = nullptr;
    this->succ = nullptr;
    this->set = nullptr;
}

/*-----------------------------------------------------------------------
//
// Function: WTFormulaAlloc()
//
//   Allocate a wrapped formula given the essential information. id
//   will automagically be set to a new value.
//
// Global Variables: FormulaIdentCounter
//
// Side Effects    : Via DefaultWFormulaAlloc()
//
/----------------------------------------------------------------------*/

WFormula::WFormula(TermBank_p terms, TFormula_p formula) : WFormula() {

    this->terms = terms;
    this->tformula = formula;
    this->ident = ++Env::global_formula_counter;
}

WFormula::WFormula(const WFormula& orig) {
}

WFormula::~WFormula() {
}

WFormula* WFormula::WFormulaParse(Scanner* in, TermBank_p terms) {
    WFormula* wform = nullptr;
    switch (in->GetFormat()) {
        case IOFormat::LOPFormat:
            Out::Error("LOP currently does not support full FOF!", ErrorCodes::OTHER_ERROR);
            break;
        case IOFormat::TPTPFormat:
            WFormulaTPTPParse(in, terms);
            break;
        case IOFormat::TSTPFormat:
            WFormulaTSTPParse(in, terms);
            break;
        default:
            assert(false);
            break;
    }
    /* WFormulaPrint(stdout, wform, true);
       printf("\n"); */
    return wform;
}

/// Parse a formula in TPTP format.
/// \param in
/// \param terms
/// \return 

void WFormula::WFormulaTPTPParse(Scanner* in, TermBank_p tbs) {
    Term_p tform;
    WFormulaProp type;
    //WFormula* handle;
    ClauseInfo* info = new ClauseInfo("", in->AktToken()->source.c_str(), in->AktToken()->line, in->AktToken()->column);

    in->AcceptInpId("input_formula");
    in->AcceptInpTok(TokenType::OpenBracket);

    in->CheckInpTok(TokenType::NamePosInt); //Name | PosInt);

    this->info->name = (in->AktToken())->literal;
    in->NextToken();

    in->AcceptInpTok(TokenType::Comma);

    in->CheckInpId("axiom|hypothesis|negated_conjecture|conjecture|question|lemma|unknown");

    if (in->TestInpId("conjecture")) {
        type = WFormulaProp::WPTypeConjecture;
    } else if (in->TestInpId("question")) {
        type = WFormulaProp::WPTypeQuestion;
    } else if (in->TestInpId("negated_conjecture")) {
        type = WFormulaProp::WPTypeNegConjecture;
    } else if (in->TestInpId("hypothesis")) {
        type = WFormulaProp::WPTypeHypothesis;
    } else {
        type = WFormulaProp::WPTypeAxiom;
    }


    in->NextToken();
    in->AcceptInpTok(TokenType::Comma);
    tform = TFormulaTPTPParse(in, tbs);

    //handle = new WFormula(tbs, tform);
    this->terms = tbs;
    this->tformula = tform;
    this->ident = ++Env::global_formula_counter;


    in->AcceptInpTok(TokenType::CloseBracket);
    in->AcceptInpTok(TokenType::Fullstop);
    this->FormulaSetType(type);
    this->FormulaSetProp(WFormulaProp::WPIniORInput);
    this->info = info;


}

/// Parse a formula in TSTP format.
/// \param in
/// \param terms
/// \return 

void WFormula::WFormulaTSTPParse(Scanner* in, TermBank_p tbs) {
    TFormula_p tform;
    WFormulaProp type = WFormulaProp::WPTypeAxiom;
    // WFormulaProp initial = WFormulaProp::WPInputFormula;
    // WFormula* handle;

    ClauseInfo* info = new ClauseInfo("", in->AktToken()->source.c_str(), in->AktToken()->line, in->AktToken()->column);
    in->AcceptInpId("fof");
    in->AcceptInpTok(TokenType::OpenBracket);
    in->CheckInpTok(TokenType::NamePosIntSQStr); // Name | PosInt | SQString);
    this->info->name = (in->AktToken())->literal;


    in->NextToken();

    in->AcceptInpTok(TokenType::Comma);


    /* This is hairy! E's internal types do not map very well to
       TSTP types, and E uses the "initial" properties in ways that
       make it highly desirable that anything in the input is
       actually initial (the CPInitialProperty is actually set in
       all clauses in the initial unprocessed clause set. So we
       ignore the "derived" modifier, and use CPTypeAxiom for plain
       clauses. */
    type = (WFormulaProp) Clause::ClauseTypeParse(in,
            "axiom|hypothesis|definition|assumption|lemma|theorem|conjecture|question|negated_conjecture|plain|unknown");
    in->AcceptInpTok(TokenType::Comma);

    tform = TFormulaTSTPParse(in, tbs);

    // handle = new WFormula(terms, tform);
    //this->WFormula::WFormula(terms, tform); //调用构造函数
    this->terms = tbs;
    this->tformula = tform;
    this->ident = ++Env::global_formula_counter;

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
    in->AcceptInpTok(TokenType::Fullstop);

    this->FormulaSetType(type);
    this->FormulaSetProp(WFormulaProp::WPIniORInput); // initial | WPInitial);
    this->info = info;

}


///  Parse a formula in TPTP format.
/// \param in
/// \param terms
/// \return 

TFormula_p WFormula::TFormulaTPTPParse(Scanner* in, TermBank_p terms) {
    TFormula_p f1, f2, res;
    FunCode op;
    f1 = elem_tform_tptp_parse(in, terms);
    if (in->TestInpTok(TokenType::FOFBinOp)) {
        op = tptp_operator_parse(in);
        f2 = TFormulaTPTPParse(in, terms);
        res = TFormulaFCodeAlloc(terms, op, f1, f2);
    } else {
        res = f1;
    }
    return res;
}

/// Parse a formula in TSTP formuat.
/// \param in
/// \param terms
/// \return 

TFormula_p WFormula::TFormulaTSTPParse(Scanner* in, TermBank_p terms) {
    TFormula_p f1, f2, res;
    FunCode op;

    f1 = literal_tform_tstp_parse(in, terms);

    if (in->TestInpTok(TokenType::FOFAssocOp)) {
        res = assoc_tform_tstp_parse(in, terms, f1);
    } else if (in->TestInpTok(TokenType::FOFBinOp)) {
        op = tptp_operator_parse(in);
        f2 = literal_tform_tstp_parse(in, terms);
        res = TFormulaFCodeAlloc(terms, op, f1, f2);
    } else {
        res = f1;
    }
    return res;
}

/// Apply standard simplifications to the wrapped formula. Return true if the formula has changed. Outputs inferences!
/// \param tbs
/// \return 

bool WFormula::WFormulaSimplify(TermBank_p tbs) {

    bool res = false;

    assert(tbs->freeVarSets.IsEmpty());
    TFormula_p simplified = TFormulaSimplify(tbs, this->tformula);
    // TBVarSetStoreFree(terms);

    if (simplified != this->tformula) {
        this->tformula = simplified;
        //DocFormulaModificationDefault(form, inf_fof_simpl);
        //WFormulaPushDerivation(form, DCFofSimplify, nullptr, nullptr);
        res = true;
    }
    return res;
}


/// If one of the args is a propositional formula of the desired type, return the other one, else return NULL.
/// \param sig
/// \param arg1
/// \param arg2
/// \param positive
/// \return 

TFormula_p WFormula::tprop_arg_return_other(Sig_p sig, TFormula_p arg1, TFormula_p arg2, bool positive) {
    if (TFormulaIsPropConst(sig, arg1, positive)) {
        return arg2;
    } else if (TFormulaIsPropConst(sig, arg2, positive)) {
        return arg1;
    }
    return nullptr;
}

bool WFormula::TFormulaIsPropConst(Sig_p sig, TFormula_p form, bool positive) {
    FunCode f_code = Sigcell::SigGetEqnCode(sig, positive);

    if (form->fCode != f_code) {
        return false;
    }
    return (form->args[0]->TermIsTrueTerm())&& (form->args[1]->TermIsTrueTerm());
}


//Return true iff var is a free variable in form.

bool WFormula::TFormulaVarIsFree(TermBank_p bank, TFormula_p form, Term_p var) {
    bool res = false;
    int i;
    Sigcell* sig = Env::getSig();

    if (TFormulaIsLiteral(sig, form)) {
        res = TermBank::TBTermIsSubterm(form, var);

    } else if ((form->fCode == sig->qexCode) ||
            (form->fCode == sig->qallCode)) {
        if (TermBank::TBTermEqual(form->args[0], var)) {
            res = false;
        } else {
            res = TFormulaVarIsFree(bank, form->args[1], var);
        }
    } else {
        for (i = 0; i < form->arity; i++) {
            res = TFormulaVarIsFree(bank, form->args[i], var);
            if (res) {
                break;
            }
        }
    }
    return res;
}

//   Allocate a formula representing a propositional constant (true or false). 

TFormula_p WFormula::TFormulaPropConstantAlloc(TermBank_p tbs, bool positive) {

    Literal* handle = new Literal();
    handle->EqnAlloc(Env::getGTbank()->trueTerm, Env::getGTbank()->trueTerm, positive);
    TFormula_p res = TFormulaLitAlloc(handle);
    DelPtr(handle);
    return res;
}
//If one of the args is a propositional formula of the desired type, return it, else return NULL.

TFormula_p WFormula::tprop_arg_return(Sig_p sig, TFormula_p arg1, TFormula_p arg2, bool positive) {
    if (TFormulaIsPropConst(sig, arg1, positive)) {
        return arg1;
    } else if (TFormulaIsPropConst(sig, arg2, positive)) {
        return arg2;
    }
    return nullptr;
}

/*-----------------------------------------------------------------------
//
// Function: TFormulaSimplify()
//
//   Maximally simplify a formula using (primarily)
//   the simplification rules (from [NW:SmallCNF-2001]). 
//
//   P | P => P    P | T => T     P | F => P
//   P & P => F    P & T => P     P & F -> F
//   ~T = F        ~F = T
//   P <-> P => T  P <-> F => ~P  P <-> T => P
//   P <~> P => ~(P<->P)
//   P -> P => T   P -> T => T    P -> F => ~P
//   ...
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

TFormula_p WFormula::TFormulaSimplify(TermBank_p tbs, TFormula_p form) {
    TFormula_p handle, arg1 = NULL, arg2 = NULL, newform;
    FunCode f_code;
    bool modified = false;

    assert(tbs);
    Sigcell* sig = Env::getSig();
    if (TFormulaIsLiteral(sig, form)) {
        return form;
    }
    if (TFormulaHasSubForm1(sig, form)) {
        arg1 = TFormulaSimplify(tbs, form->args[0]);
        modified = arg1 != form->args[0];
    } else if (TFormulaIsQuantified(sig, form)) {
        arg1 = form->args[0];
    }

    if (TFormulaHasSubForm2(sig, form) ||
            TFormulaIsQuantified(sig, form)) {
        arg2 = TFormulaSimplify(tbs, form->args[1]);
        modified |= arg2 != form->args[1];
    }
    if (modified) {
        assert(tbs);
        form = TFormulaFCodeAlloc(tbs, form->fCode, arg1, arg2);
    }
    modified = true;
    while (modified) {
        modified = false;
        newform = form; /* Inelegant, fix when awake! */
        if (form->fCode == sig->notCode) {
            if (TFormulaIsLiteral(sig, form->args[0])) {
                f_code = sig->SigGetOtherEqnCode(form->args[0]->fCode);
                newform = TFormulaFCodeAlloc(tbs, f_code,
                        form->args[0]->args[0],
                        form->args[0]->args[1]);
            }
        } else if (form->fCode == sig->orCode) {
            if ((handle = tprop_arg_return_other(sig, form->args[0],
                    form->args[1], false))) {
                newform = handle;
            } else if ((handle = tprop_arg_return(sig, form->args[0],
                    form->args[1], true))) {
                newform = handle;
            } else if (TFormulaEqual(form->args[0], form->args[1])) {
                newform = form->args[0];
            }
        } else if (form->fCode == sig->andCode) {
            if ((handle = tprop_arg_return_other(sig, form->args[0],
                    form->args[1], true))) {
                newform = handle;
            } else if ((handle = tprop_arg_return(sig, form->args[0],
                    form->args[1], false))) {
                newform = handle;
            } else if (TFormulaEqual(form->args[0], form->args[1])) {
                newform = form->args[0];
            }
        } else if (form->fCode == sig->equivCode) {
            if ((handle = tprop_arg_return_other(sig, form->args[0],
                    form->args[1], true))) {
                /* p <=> T -> p */
                newform = handle;
            } else if ((handle = tprop_arg_return_other(sig, form->args[0], form->args[1], false))) {
                /* p <=> F -> ~p */
                newform = TFormulaNegAlloc(tbs, handle);
            } else if (TFormulaEqual(form->args[0], form->args[1])) {
                newform = TFormulaPropConstantAlloc(tbs, true);
            }
        } else if (form->fCode == sig->implCode) {
            if (TFormulaIsPropTrue(sig, form->args[0])) {
                /* T => p -> p */
                newform = form->args[1];
            } else if (TFormulaIsPropFalse(sig, form->args[0])) {
                /* F => p -> T */
                newform = TFormulaPropConstantAlloc(tbs, true);
            } else if (TFormulaIsPropFalse(sig, form->args[1])) {
                /* p => F -> ~p */
                newform = TFormulaNegAlloc(tbs, form->args[0]);
            } else if (TFormulaIsPropTrue(sig, form->args[1])) {
                /* p => T -> T */
                newform = TFormulaPropConstantAlloc(tbs, true);
            } else if (TFormulaEqual(form->args[0], form->args[1])) {
                /* p => p -> T */
                newform = TFormulaPropConstantAlloc(tbs, true);
            }
        } else if (form->fCode == sig->xorCode) {
            handle = TFormulaFCodeAlloc(tbs, sig->equivCode,
                    form->args[0], form->args[1]);
            newform = TFormulaFCodeAlloc(tbs, sig->notCode,
                    handle, nullptr);
            newform = TFormulaSimplify(tbs, newform);
        } else if (form->fCode == sig->bimplCode) {
            newform = TFormulaFCodeAlloc(tbs, sig->implCode,
                    form->args[1], form->args[0]);
            newform = TFormulaSimplify(tbs, newform);
        } else if (form->fCode == sig->norCode) {
            handle = TFormulaFCodeAlloc(tbs, sig->orCode, form->args[0], form->args[1]);
            newform = TFormulaFCodeAlloc(tbs, sig->notCode, handle, nullptr);
            newform = TFormulaSimplify(tbs, newform);
        } else if (form->fCode == sig->nandCode) {
            handle = TFormulaFCodeAlloc(tbs, sig->andCode, form->args[0], form->args[1]);
            newform = TFormulaFCodeAlloc(tbs, sig->notCode, handle, nullptr);
            newform = TFormulaSimplify(tbs, newform);
        }
        if ((form->fCode == sig->qexCode) ||
                (form->fCode == sig->qallCode)) {
            if (!TFormulaVarIsFree(tbs, form->args[1], form->args[0])) {
                newform = form->args[1];
            }
        }
        if (newform != form) {
            modified = true;
            form = newform;
        }
    }
    return newform;
}

/*---------------------------------------------------------------------*/
/*                          private Function                            */
/*---------------------------------------------------------------------*/


/// Parse an elementary formula in TPTP/TSTP format.
/// \param in
/// \param terms
/// \return 

TFormula_p WFormula::elem_tform_tptp_parse(Scanner* in, TermBank_p terms) {
    TFormula_p res, tmp;

    if (in->TestInpTok(TokenType::AllOrExistQuantor)) {
        FunCode quantor = tptp_quantor_parse(in);
        in->AcceptInpTok(TokenType::OpenSquare);
        res = quantified_tform_tptp_parse(in, terms, quantor);
    } else if (in->TestInpTok(TokenType::OpenBracket)) {
        in->AcceptInpTok(TokenType::OpenBracket);
        res = TFormulaTPTPParse(in, terms);
        in->AcceptInpTok(TokenType::CloseBracket);
    } else if (in->TestInpTok(TokenType::TildeSign)) {
        in->AcceptInpTok(TokenType::TildeSign);
        tmp = elem_tform_tptp_parse(in, terms);
        res = TFormulaFCodeAlloc(terms, Env::getSig()->notCode, tmp, nullptr);
    } else {
        Literal* lit = new Literal();
        lit->EqnFOFParse(in, terms);
        res = TFormulaLitAlloc(lit);
        DelPtr(lit);
    }
    return res;
}

TFormula_p WFormula::literal_tform_tstp_parse(Scanner* in, TermBank_p terms) {
    TFormula_p res, tmp;
    if (in->TestInpTok(TokenType::AllOrExistQuantor)) {//AllQuantor | ExistQuantor

        FunCode quantor = tptp_quantor_parse(in);
        in->AcceptInpTok(TokenType::OpenSquare);
        res = quantified_tform_tstp_parse(in, terms, quantor);
    } else if (in->TestInpTok(TokenType::OpenBracket)) {
        in->AcceptInpTok(TokenType::OpenBracket);
        res = TFormulaTSTPParse(in, terms);
        in->AcceptInpTok(TokenType::CloseBracket);
    } else if (in->TestInpTok(TokenType::TildeSign)) {
        in->AcceptInpTok(TokenType::TildeSign);
        tmp = literal_tform_tstp_parse(in, terms);
        res = TFormulaFCodeAlloc(terms, Env::getSig()->notCode, tmp, nullptr);
    } else {
        Literal* lit = new Literal();
        lit->EqnFOFParse(in, terms);
        res = TFormulaLitAlloc(lit);
        DelPtr(lit);
    }
    return res;
}

//Parse a sequence of formulas connected by a single AC operator and return it.

TFormula_p WFormula::assoc_tform_tstp_parse(Scanner* in, TermBank_p terms, TFormula_p head) {

    TokenType optok = in->AktTokenType();
    FunCode op = tptp_operator_convert(optok);
    while (in->TestInpTok(optok)) {
        in->AcceptInpTok(optok);
        TFormula_p f2 = literal_tform_tstp_parse(in, terms);
        head = TFormulaFCodeAlloc(terms, op, head, f2);
    }
    return head;
}


/// Parse and return a TPTP quantor. Rather trivial ;-)
/// \param in
/// \return 

FunCode WFormula::tptp_quantor_parse(Scanner* in) {
    FunCode res;
    Sig_p sig = Env::getSig();

    in->CheckInpTok(TokenType::AllOrExistQuantor);
    if (in->TestInpTok(TokenType::ExistQuantor)) {
        res = sig->qexCode;
    } else {
        res = sig->qallCode;
    }
    in->NextToken();
    return res;
}

/// R eturn the f_code corresponding to a given token. Rather trivial ;-)
/// \param tok
/// \return 

FunCode WFormula::tptp_operator_convert(TokenType tok) {
    FunCode res = 0;
    Sigcell* sig = Env::getSig();
    switch (tok) {
        case TokenType::FOFOr:
            res = sig->orCode;
            break;
        case TokenType::FOFAnd:
            res = sig->andCode;
            break;
        case TokenType::FOFLRImpl:
            res = sig->implCode;
            break;
        case TokenType::FOFRLImpl:
            res = sig->bimplCode;
            break;
        case TokenType::FOFEquiv:
            res = sig->equivCode;
            break;
        case TokenType::FOFXor:
            res = sig->xorCode;
            break;
        case TokenType::FOFNand:
            res = sig->nandCode;
            break;
        case TokenType::FOFNor:
            res = sig->norCode;
            break;
        default:
            assert(false && "Unknown/Impossibe operator.");
            break;
    }
    return res;
}

/// Parse a TPTP operator and return the corresponding f_code. Rather trivial ;-) 
/// \param in
/// \return 

FunCode WFormula::tptp_operator_parse(Scanner* in) {
    FunCode res = 0;
    in->CheckInpTok(TokenType::FOFBinOp);
    res = tptp_operator_convert(in->AktTokenType());
    in->NextToken();
    return res;
}

/// Parse a quantified TPTP/TSTP formula. At this point, the quantor
//   has already been read (and is passed into the function), and we
//   are at the first (or current) variable.
/// \param in
/// \param terms
/// \param quantor
/// \return 

TFormula_p WFormula::quantified_tform_tptp_parse(Scanner* in, TermBank_p terms, FunCode quantor) {
    Term_p var;
    TFormula_p rest, res;
    string source_name, errpos;
    long line, column;
    StreamType type;

    line = in->AktToken()->line;
    column = in->AktToken()->column;
    source_name = in->AktToken()->source;
    type = in->AktToken()->streamType;

    var = terms->TBTermParseReal(in, nullptr, true);
    if (!var->IsVar()) {
        errpos += in->PosRep(type, source_name, line, column);
        errpos += " Variable expected, non-variable term found";
        Out::Error(errpos.c_str(), ErrorCodes::SYNTAX_ERROR);

    }
    if (in->TestInpTok(TokenType::Comma)) {
        in->AcceptInpTok(TokenType::Comma);
        rest = quantified_tform_tptp_parse(in, terms, quantor);
    } else {
        in->AcceptInpTok(TokenType::CloseSquare);
        in->AcceptInpTok(TokenType::Colon);
        rest = elem_tform_tptp_parse(in, terms);
    }
    res = TFormulaFCodeAlloc(terms, quantor, var, rest);
    return res;
}

TFormula_p WFormula::quantified_tform_tstp_parse(Scanner* in, TermBank_p terms, FunCode quantor) {
    Term_p var;
    TFormula_p rest, res;
    string source_name, errpos;


    long line = in->AktToken()->line;
    long column = in->AktToken()->column;
    source_name = in->AktToken()->source;
    StreamType type = in->AktToken()->streamType;
    var = terms->TBTermParseReal(in, nullptr, true);

    if (!var->IsVar()) {
        errpos += in->PosRep(type, source_name, line, column);
        errpos += " Variable expected, non-variable term found";
        Out::Error(errpos.c_str(), ErrorCodes::SYNTAX_ERROR);

    }
    if (in->TestInpTok(TokenType::Comma)) {
        in->AcceptInpTok(TokenType::Comma);
        rest = quantified_tform_tstp_parse(in, terms, quantor);
    } else {
        in->AcceptInpTok(TokenType::CloseSquare);
        in->AcceptInpTok(TokenType::Colon);
        rest = literal_tform_tstp_parse(in, terms);
    }
    res = TFormulaFCodeAlloc(terms, quantor, var, rest);

    return res;
}


/// Allocate a formula given an f_code and two subformulas (the second one may be NULL).
/// \param bank
/// \param op
/// \param arg1
/// \param arg2
/// \return 

TFormula_p WFormula::TFormulaFCodeAlloc(TermBank_p bank, FunCode op, TFormula_p arg1, TFormula_p arg2) {

    Sigcell* sig = Env::getSig();

    int arity = sig->SigFindArity(op);
    assert(bank);
    assert((arity == 1) || (arity == 2));
    assert(EQUIV((arity == 2), arg2));

    TFormula_p res = TermCell::TermTopAlloc(op, arity);
    if (sig->SigIsPredicate(op)) {
        res->TermCellSetProp(TermProp::TPPredPos);
    }
    if (arity > 0) {
        res->args[0] = arg1;
        if (arity > 1) {
            res->args[1] = arg2;
        }
    }
    assert(bank);

    res = bank->TBTermTopInsert(res);

    return res;
}
