/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   TermCell.cpp
 * Author: zj 
 * 
 * Created on 2017年3月22日, 下午4:10
 */

#include "TermCell.h"
#include "INOUT/FileOp.h"
#include "CLAUSE/Clause.h"
#include "VarBank.h"

bool TermCell::TermPrintLists = true;
/*---------------------------------------------------------------------*/
/*                  Member Function-[private]                          */
/*---------------------------------------------------------------------*/

/***************************************************************************** 
 * Traverse a tree and check if any one term cell occurs more than once on any branch 
 * (which would make the term cyclic). Return the first inconsistency found or NULL.
 ****************************************************************************/
TermCell* TermCell::term_check_consistency_rek(SplayTree<PTreeCell>&branch, DerefType deref) {
    TermCell* term = TermDeref(term, deref);
    TermCell* res = nullptr;
    putc('.', stdout);
    if (!branch.TreeStore(term)) {
        return term;
    }
    for (int i = 0; i < term->arity; i++) {
        if ((res = term->args[i]->term_check_consistency_rek(branch, deref))) {
            break;
        }
    }
    branch.TreeDeleteEntry(term);
    return res;
}



/*---------------------------------------------------------------------*/
/*                    Constructed Function                             */

/*---------------------------------------------------------------------*/
TermCell::TermCell() : properties(TermProp::TPIgnoreProps), fCode(0), uVarCount(0), arity(0), claId(0), binding(nullptr), args(nullptr), uTermWeight(DEFAULT_FWEIGHT) {

    this->uMaxFuncLayer = 0;
    this->uMaxVarDepth = 0;
    // zjweight = 0.0f;
    //rw_data.nf_date[0] = SysDateCreationTime();
    //rw_data.nf_date[1] = SysDateCreationTime();
}

/*构造函数 - 创建一个constant term 如:ａ ,b */
TermCell::TermCell(long symbol) : TermCell() {
    this->uTermWeight = DEFAULT_FWEIGHT;
    //  zjweight = 1.0f;
    this->fCode = symbol;
}

/*构造函数 - 创建一个function term 函数符如:f */
TermCell::TermCell(long f_code, int arity) : TermCell() {

    this->fCode = f_code;
    this->arity = arity;
    if (this->arity) {
        this->args = new TermCell*[this->arity]; //创建一个动态二维数组
    }
}

/* 构造函数,copy TermCell */
//TermCell::TermCell(TermCell& orig) : TermCell() {
//    /* All other properties are tied to the specific term! */
//    properties = (TermProp) ((int32_t) orig.properties & (int32_t) TermProp::TPPredPos);
//    /* As it gets a new id below */
//    this->TermCellDelProp(TermProp::TPOutputFlag);
//    fCode = orig.fCode;
//    arity = orig.arity;
//    claId = orig.claId;
//    args = orig.TermArgListCopy();
//    binding = nullptr;
//    lson = nullptr;
//    rson = nullptr;
//} 
/*---------------------------------------------------------------------
 - 创建Term  
 ---------------------------------------------------------------------*/

/*****************************************************************************
 * [重要方法]-- 根据scanner 创建term
 * Parse a term from the given scanner object into the internal termrepresentation.
 ****************************************************************************/
//TermCell* TermCell::TermParse(Scanner* in, TermBank* tb) {
//    string idStr;
//    Sig_p sig = Env::getSig();
//    TermCell* handle = nullptr;
//    VarBank_p vars = tb->GetShareVar();
//    if (Sigcell::SigSupportLists && in->TestInpTok(TokenType::OpenSquare)) {
//        handle = parse_cons_list(in, tb);
//    } else {
//        FuncSymbType idType;
//        //变元项判断
//        if ((idType = TermParseOperator(in, idStr)) == FuncSymbType::FSIdentVar) {
//            handle = vars->Insert(idStr, tb->claId);
//            handle->uVarCount = 1; //变元项个数
//            handle->uMaxFuncLayer = 0; //函数深度
//        } else {
//            handle = new TermCell();
//            if (in->TestInpTok(TokenType::OpenBracket)) {
//                if ((idType == FuncSymbType::FSIdentInt)&&(sig->distinctProps & FPIsInteger)) {
//                    in->AktTokenError("Number cannot have argument list (consider --free-numbers)", false);
//                }
//                if ((idType == FuncSymbType::FSIdentObject)&&(sig->distinctProps & FPIsObject)) {
//                    in->AktTokenError("Object cannot have argument list (consider --free-objects)", false);
//                }
//
//                handle->arity = TermParseArgList(in, &(handle->args), tb);
//            } else {
//                handle->arity = 0;
//            }
//            handle->fCode = TermCell::TermSigInsert(sig, idStr, handle->arity, false, idType);
//            if (!handle->fCode) {
//                string errpos; //
//                in->AktToken()->PosRep(errpos);
//                errpos += ' ' + idStr + " used with arity " + to_string((long) handle->arity)
//                        + " but registered with arity ";
//                errpos += to_string((long) sig->SigFindArity(sig->SigFindFCode(idStr)));
//                //Error(DStrView(errpos), SYNTAX_ERROR);
//                cout << "SYNTAX_ERROR:" + errpos << endl;
//            }
//        }
//        //DStrReleaseRef(source_name);
//        //DStrFree(id);
//    }
//    assert(true);
//    return nullptr;
//}

/// 获取子句函数嵌套
/// \return 

uint8_t TermCell::ComputeMaxFuncDepth() {
    uint8_t ldepth;
    if (this->IsGround())
        return this->uMaxFuncLayer;
    this->uMaxFuncLayer = 0;
    for (int i = 0; i < arity; ++i) {
        TermCell* term = TermCell::TermDerefAlways(args[i]);
        ldepth = term->ComputeMaxFuncDepth();
        this->uMaxFuncLayer = MAX(this->uMaxFuncLayer, ldepth);
    }
    if (this->uMaxFuncLayer > UINT16_MAX - 1) {
        Out::Error("Error:函数嵌套层超过最大限制(65535)!", ErrorCodes::SYNTAX_ERROR);
    }
    return ++this->uMaxFuncLayer;
}

/* 重新计算项的变元嵌套深度 - the depth of a term. */
uint16_t TermCell::TermDepth() {
    if (0 == arity || this->IsVar())
        return 0;
    uint16_t maxdepth = 0, ldepth;
    for (int i = 0; i < arity; ++i) {
        ldepth = args[i]->TermDepth();
        maxdepth = MAX(maxdepth, ldepth);
    }
    return maxdepth + 1;
}


/// 检查最大函数嵌套层限制. 包括了 变元绑定
/// \return >0 -- 函数嵌套层数 -1 -- 不符合限制

int16_t TermCell::CheckTermDepthLimit() {
    int16_t maxdepth = 0, ldepth = 0;
    //debug
    if (this->arity == 0) {
        maxdepth = 0;
    } else if (this->IsGround()) {
        maxdepth = this->uMaxFuncLayer;
        if (maxdepth > StrategyParam::MaxFuncLayerOfR)
            return -1;
    } else {
        maxdepth = 0;
        for (int i = 0; i < arity; ++i) {
            TermCell* term = TermCell::TermDerefAlways(args[i]);
            ldepth = term->IsGround() ? term->GetFuncLayer() : term->CheckTermDepthLimit();
            if (-1 == ldepth)
                return -1;
            if (maxdepth < ldepth) {
                maxdepth = ldepth;
                if (maxdepth + 1 > StrategyParam::MaxFuncLayerOfR)
                    return -1;
            }
        }
        ++maxdepth;
    }
    return maxdepth;
}

TermCell* TermCell::TermDeref(TermCell* term, DerefType deref) {
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

/***************************************************************************** 
 * Parse an operator (i.e. an optional $, followed by an identifier), 
 * store the representation into id and determine the type.using the following rules:
 * - If it starts with a $, it's a TermIdentInterpreted (LOP global variables 
 * are treated as interpreted constants).
 * - If it is a PosInt, it is a TermIdentNumber
 * - If its a String, it is a TermIdentObject
 * - If it is an upper-case or underscore Ident and no opening bracket follows, 
 * its a TermIdentVariable
 * - Otherwise its a free function symbol (cases are SQString and Identifier 
 * starting with lower-case letter.
 ****************************************************************************/
FuncSymbType TermCell::TermParseOperator(Scanner* in, string&idStr) {

    FuncSymbType res = in->FuncSymbParse(idStr);

#ifndef STRICT_TPTP
    if ((isupper(idStr[0]) || (idStr[0] == '_'))
            && in->TestInpTok(TokenType::OpenBracket)) {
        res = FuncSymbType::FSIdentFreeFun;
    }
#endif   
    return res;
}

/***************************************************************************** 
 * Parse a LOP list into an internal $cons list.
 ****************************************************************************/
//TermCell* TermCell::parse_cons_list(Scanner* in, TermBank* tb) {
//    in->AcceptInpTok(TokenType::OpenSquare);
//    TermCell* current = new TermCell();
//    if (!in->TestInpTok(TokenType::CloseSquare)) {
//        current->fCode = (FunCode) DerefType::CONSCODE;
//        current->arity = 2;
//        current->args = new TermCell*[2];
//        current->args[0] = TermCell::TermParse(in, tb);
//        current->args[1] = new TermCell();
//        current = current->args[1];
//        while (in->TestInpTok(TokenType::Comma)) {
//            in->NextToken();
//            current->fCode = (FunCode) DerefType::CONSCODE;
//            current->arity = 2;
//            current->args = new TermCell*[2];
//            current->args[0] = TermCell::TermParse(in, tb);
//            current->args[0]-> TermCellDelProp(TermProp::TPTopPos);
//            current->args[1] = new TermCell();
//            current = current->args[1];
//        }
//    }
//    in->AcceptInpTok(TokenType::CloseSquare);
//    current->fCode = (FunCode) DerefType::NILCODE;
//    return current;
//}

/*-----------------------------------------------------------------------
//
// Function: TermParseArgList()
//
//   Parse a list of terms (comma-separated and enclosed in brackets)
//   into an array of term pointers. Return the number of terms
//   parsed, and make arg_anchor point to the array. Note: The array
//   has to have exactly the right size, as it will be handled by
//   Size[Malloc|Free] for efficiency reasons and may otherwise lead
//   to a memory leak. This leads to some complexity...
//   If the arglist is empty, return 0 and use the NULL pointer as
//   anchor. 
//
// Global Variables: -
//
// Side Effects    : Input, memory operations
//
/----------------------------------------------------------------------*/

//int TermCell::TermParseArgList(Scanner* in, TermCell*** arg_anchor, TermBank* tb) {
//    // VarBank_p vars = tb->shareVars;
//    TermCell* *handle;
//    int arity;
//    int size;
//    int i;
//
//    in-> AcceptInpTok(TokenType::OpenBracket);
//    if (in->TestInpTok(TokenType::CloseBracket)) {
//        in->NextToken();
//        *arg_anchor = NULL;
//        return 0;
//    }
//    size = 2; //TERMS_INITIAL_ARGS;
//    handle = new TermCell*[size]; // (TermCell**)SizeMalloc(size*sizeof(TermCell*));
//    arity = 0;
//    handle[arity] = TermCell::TermParse(in, tb);
//
//    arity++;
//    while (in->TestInpTok(TokenType::Comma)) {
//        in->NextToken();
//        if (arity == size) {
//            size += TERMS_INITIAL_ARGS;
//            handle = new TermCell*[size]; // (TermCell**)SecureRealloc(handle, size*sizeof(TermCell*));
//        }
//        handle[arity] = TermCell::TermParse(in, tb);
//        arity++;
//    }
//    in-> AcceptInpTok(TokenType::CloseBracket);
//    *arg_anchor = new TermCell*[arity]; // TermArgArrayAlloc(arity);
//    for (i = 0; i < arity; i++) {
//        (*arg_anchor)[i] = handle[i];
//    }
//    delete[] handle;
//    //SizeFree(handle, size*sizeof(TermCell*));
//
//    return arity;
//}

TermCell** TermCell::TermArgListCopy() {
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

TermCell* TermCell::RenameCopy(TermBank* tb, Lit_p litptr, DerefType deref) {

    //debug    if (this->fCode == -4) {        cout << "debug" << endl;    }
    TermCell* source = TermDeref(this, deref);
    //测试zj 变元替换-重写
    //    TermCell*replaceTerm = tb->FindRewriteTerm(source);
    //    if (replaceTerm != nullptr) {
    //
    //        //存在替换
    //        source = replaceTerm;
    //        tb->isRewrite = true;
    //    }
    if (source->TBTermIsGround())//若为全局共享基项,则直接返回GTermBank中的全局共享基项
        return source;
    Term_p t;
    if (source->IsVar()) {
        string varName;
        source->getVarName(varName);
        t = tb->VarInert(varName, litptr); // vars->VarBankFCodeAssertAlloc(this->fCode);
        t->SetVarCount(1);
        t->SetTermWeight(DEFAULT_VWEIGHT);
        t->SetFuncLayer(0);


    } else {
        uint16_t uFuncLayer = 0;
        t = new TermCell(source->fCode);
        t->arity = source->arity;
        t->properties = (TermProp) ((int32_t) source->properties & (int32_t) TermProp::TPPredPos); // t = TermCell::TermTopCopy(source); //创建一个 unshared term at the moment

        if (t->arity > 0) {
            t->args = new TermCell*[t->arity];
            for (int i = 0; i < t->arity; ++i) {
                t->args[i] = source->args[i]-> RenameCopy(tb, litptr);
                t->SetTermWeight(t->GetTermWeight() + t->args[i]->GetTermWeight());
                t->SetVarCount(t->GetVarCount() + t->args[i]->GetVarCount());
                uFuncLayer = MAX(uFuncLayer, t->args[i]->GetFuncLayer());
            }

            ++uFuncLayer;
        } else {
            t->SetVarCount(0);
            t->SetTermWeight(DEFAULT_FWEIGHT);
            uFuncLayer = 0;
        }

        t->SetFuncLayer(uFuncLayer);
        // TermBank::tb_termtop_insert(this);  重命名的项不存储到 termbank中        
        t = tb->TBTermTopInsert(t);
    }

    return t;
}

//P1(f(x1))  x1-y1->a1;   P1(f(a1))

TermCell* TermCell::TermCopy(TermBank* tb, Lit_p litptr, DerefType deref) {

    TermCell* source = TermDeref(this, deref);
    TermCell* handle = source->TermEquivCellAlloc(tb, litptr);

    for (int i = 0; i < handle->arity; ++i) /* Hack: Loop will not be entered if arity = 0 */ {
        handle->args[i] = handle->args[i]->TermCopy(tb, litptr, deref);
    }
    return handle;
}

/*析构函数*/
TermCell::~TermCell() {

    TermTopFree();
}

/*****************************************************************************
 * 删除函数项
 ****************************************************************************/
void TermCell::TermTopFree() {

    if (arity) {
        assert(args);
        DelArrayPtr(args);
        arity = 0;
    } else {
        assert(!args);
    }
}

void TermCell::TermFree(TermCell* junk) {
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

/*---------------------------------------------------------------------*/
/*                  Member Function-[public]                           */
/*---------------------------------------------------------------------*/
//

TermCell* TermCell::TermTopCopy(TermCell* source) {
    TermCell* t = new TermCell(source->fCode);

    /* All other properties are tied to the specific term! */
    t->properties = (TermProp) ((int32_t) source->properties & (int32_t) TermProp::TPPredPos);

    /* As it gets a new id below */
    t->TermCellDelProp(TermProp::TPOutputFlag);
    t->arity = source->arity;
    t->binding = nullptr;
    t->args = source->TermArgListCopy();
    t->SetFuncLayer(source->GetFuncLayer());
    t->SetMaxVarId(source->GetMaxVarId());
    t->SetVarCount(source->GetVarCount());
    t->lson = nullptr;
    t->rson = nullptr;

    return t;
}
//变元项输出

void TermCell::VarPrint(FILE* ot) {
    //    assert(fCode < 0);
    //    char id = 'X';
    //    if (fCode % 2) {
    //        id = 'Y';
    //    }
    //    fprintf(ot, "%c%ld", id, -((fCode - 1) / 2));
    string varName = "";
    fprintf(ot, "X%s", this->getVarName(varName));
}

void TermCell::TermPrint(FILE* out, DerefType deref) {
    TermCell* term = this;
    assert(term);
    // assert(sig || term->IsVar());

    term = TermCell::TermDeref(term, deref);
    //zj
    //cout << "idx:" << term->hashIdx<<" ";
#ifdef NEVER_DEFINED
    if (TermCellQueryProp(term, TPRestricted)) {
        fprintf(out, "*");
    }
    if (TermCellQueryProp(term, TPIsRewritten)) {
        if (term->TermIsTopRewritten()) {
            fprintf(out, "=");
        } else {
            fprintf(out, "+");
        }
    }
#endif
    if (Sigcell::SigSupportLists && TermCell::TermPrintLists &&
            ((term->fCode == (FunCode) DerefType::NILCODE) ||
            (term->fCode == (FunCode) DerefType::CONSCODE))) {
        term->print_cons_list(out, deref);
    } else {
        if (term->IsVar()) {
            term->VarPrint(out);
        } else {
            string tmpStr;
            Env::getSig()->SigFindName(term->fCode, tmpStr);
            fputs(tmpStr.c_str(), out);
            if (!term->IsConst()) {
                assert(term->args);
                term->TermPrintArgList(out, term->arity, deref);
            }
        }
    }
}

void TermCell::getStrOfTerm(string&outStr, DerefType deref) {

    TermCell* term = TermCell::TermDeref(this, deref);
    //zj     //cout << "idx:" << term->hashIdx<<" ";

    if (Sigcell::SigSupportLists && TermCell::TermPrintLists &&
            ((term->fCode == (FunCode) DerefType::NILCODE) ||
            (term->fCode == (FunCode) DerefType::CONSCODE))) {
        cout << "该输入格式暂时不支持" << endl;
        assert(false); //暂时无法实现
        //term->print_cons_list(out, deref);
    } else {
        if (term->IsVar()) {
            string varName = "";
            term->getVarName(varName);
            outStr += "X" + varName;

        } else {
            string tmpStr;
            Env::getSig()->SigFindName(term->fCode, tmpStr);
            outStr += tmpStr;
            if (!term->IsConst()) {
                assert(term->args);
                term->getStrOfTermArgList(outStr, term->arity, deref);
            }
        }
    }

}

void TermCell::PrintDerefAlways(FILE* out) {
    TermCell* term = this;
    assert(term);
    // assert(sig || term->IsVar());

    term = TermCell::TermDerefAlways(term);
    //zj
    //cout << "idx:" << term->hashIdx<<" ";
#ifdef NEVER_DEFINED
    if (TermCellQueryProp(term, TPRestricted)) {
        fprintf(out, "*");
    }
    if (TermCellQueryProp(term, TPIsRewritten)) {
        if (term->TermIsTopRewritten()) {
            fprintf(out, "=");
        } else {
            fprintf(out, "+");
        }
    }
#endif
    if (Sigcell::SigSupportLists && TermCell::TermPrintLists &&
            ((term->fCode == (FunCode) DerefType::NILCODE) ||
            (term->fCode == (FunCode) DerefType::CONSCODE))) {
        term->print_cons_list(out, DerefType::DEREF_ALWAYS);
    } else {
        if (term->IsVar()) {
            VarPrint(out);
        } else {
            string tmpStr;
            Env::getSig()->SigFindName(term->fCode, tmpStr);
            fputs(tmpStr.c_str(), out);
            if (!term->IsConst()) {
                assert(term->args);
                term->TermPrintArgList(out, term->arity, DerefType::DEREF_ALWAYS);
            }
        }
    }
}

void TermCell::TermPrintArgList(FILE* out, int arity, DerefType deref) {
    assert(arity >= 1);
    putc('(', out);
    args[0]->TermPrint(out, deref);

    for (int i = 1; i < arity; ++i) {
        putc(',', out);
        args[i]->TermPrint(out, deref);
    }
    putc(')', out);
}

void TermCell::getStrOfTermArgList(string&outStr, int arity, DerefType deref) {
    assert(arity >= 1);
    outStr += '(';
    args[0]->getStrOfTerm(outStr, deref);

    for (int i = 1; i < arity; ++i) {
        outStr += ',';

        args[i]->getStrOfTerm(outStr, deref);
    }
    outStr += ')';
}

void TermCell::PrintTermSig(FILE* out) {
    string sigName;
    Env::getSig()->SigFindName(this->fCode, sigName);
    fputs(sigName.c_str(), out);

}

/*-----------------------------------------------------------------------
//   Print a list of $cons'ed terms, terminated with $nil. Abort on
//   not well-formed lists (no cons pairs!).
/----------------------------------------------------------------------*/

void TermCell::print_cons_list(FILE* out, DerefType deref) {
    TermCell* tlist = this;
    assert(Sigcell::SigSupportLists);
    putc('[', out);
    if (tlist->fCode == (FunCode) DerefType::CONSCODE) {
        assert(tlist->args);
        tlist->args[0]->TermPrint(out, deref);
        tlist = tlist->args[1];
        while (tlist->fCode == (FunCode) DerefType::CONSCODE) {
            putc(',', out);
            /* putc(' ', out); */
            assert(tlist->args);
            tlist->args[0]-> TermPrint(out, deref);
            tlist = tlist->args[1];
        }
        assert(tlist->fCode == (FunCode) DerefType::NILCODE);
    }
    putc(']', out);
}

/***************************************************************************** 
 * Set the properties in all term cells belonging to term.
 ****************************************************************************/
void TermCell::TermSetProp(DerefType dt, TermProp prop) {
    TermCell* term = this;
    stack<IntOrP> mystack; // = PStackAlloc();
    IntOrP tmp;
    tmp.p_val = this;
    mystack.push(tmp);
    tmp.i_val = (long) prop;
    mystack.push(tmp);

    while (!mystack.empty()) {
        dt = (DerefType) ((IntOrP) mystack.top()).i_val;
        mystack.pop();
        TermCell* term = (TermCell*) ((IntOrP) mystack.top()).p_val;
        term = TermCell::TermDeref(term, dt);
        term->TermCellSetProp(prop);
        for (int i = 0; i < term->arity; i++) {
            tmp.p_val = term->args[i];
            mystack.push(tmp);
            //PStackPushP(stack, term->args[i]);
            tmp.i_val = (long) dt;
            mystack.push(tmp);
            // PStackPushInt(stack, deref);
        }
    }

    //PStackFree(stack);
    //C++ 退出后自己释放
}

/***************************************************************************** 
 * Delete the properties in all term cells belonging to term.
 ****************************************************************************/
void TermCell::TermDelProp(DerefType deref, TermProp prop) {

    TermCell* term = this;
    stack<IntOrP>myStack;
    int i;
    IntOrP tmp;
    tmp.p_val = term;
    myStack.push(tmp);
    //PStackPushP(stack, term);
    tmp.i_val = (long) deref;
    myStack.push(tmp);
    //   PStackPushInt(stack, deref);

    while (!myStack.empty()) {
        //deref = PStackPopInt(stack);
        deref = (DerefType) ((IntOrP) myStack.top()).i_val;
        myStack.pop();
        //term  = PStackPopP(stack);
        term = (TermCell*) ((IntOrP) myStack.top()).p_val;
        myStack.pop();
        term = TermCell::TermDeref(term, deref);
        term->TermCellDelProp(prop);
        for (i = 0; i < term->arity; i++) {
            tmp.p_val = term->args[i];
            myStack.push(tmp);
            //PStackPushP(stack, term->args[i]);
            tmp.i_val = (long) deref;
            myStack.push(tmp);
            // PStackPushInt(stack, deref);
        }
    }
    //PStackFree(stack);

}

/*-----------------------------------------------------------------------
//
// Function: TermVarDelProp()
//
//   Delete the properties in all variable cells belonging to term.
//
// Global Variables: -
//
// Side Effects    : Changes properties (even in shared terms! Beware!)
//
/----------------------------------------------------------------------*/

void TermCell::TermVarDelProp(DerefType deref, TermProp prop) {
    TermCell* term = this;
    //PStack_p stack = PStackAlloc();
    stack<IntOrP> myStack;
    int i;
    IntOrP tmp;
    tmp.p_val = (void*) term;
    myStack.push(tmp);

    //PStackPushP(stack, term);
    tmp.i_val = (long) deref;
    myStack.push(tmp);
    //PStackPushInt(stack, deref);

    while (!myStack.empty()) {

        //deref = PStackPopInt(stack);
        deref = (DerefType) ((IntOrP) myStack.top()).i_val;
        myStack.pop();
        term = (TermCell*) ((IntOrP) myStack.top()).p_val;
        myStack.pop();
        //term  = PStackPopP(stack);
        term = TermCell::TermDeref(term, deref);
        if (term->IsVar()) {
            term->TermCellDelProp(prop);
        }
        for (i = 0; i < term->arity; i++) {
            tmp.p_val = term->args[i];
            myStack.push(tmp);
            //PStackPushP(stack, term->args[i]);
            tmp.i_val = (long) deref;
            myStack.push(tmp);
            // PStackPushInt(stack, deref);
        }
    }
    // PStackFree(stack);

}

/***************************************************************************** 
 * If prop is set in any subterm of term, return true, otherwise false. 
 ****************************************************************************/
bool TermCell::TermSearchProp(DerefType deref, TermProp prop) {
    TermCell* term = this;
    stack<IntOrP> myStack;
    bool res = false;
    IntOrP tmp;
    tmp.p_val = term;
    myStack.push(tmp);
    tmp.i_val = (long) deref;
    myStack.push(tmp);
    while (!myStack.empty()) {
        deref = (DerefType) ((IntOrP) myStack.top()).i_val;
        myStack.pop();
        //deref = PStackPopInt(stack);
        //term  = PStackPopP(stack);
        term = (TermCell*) ((IntOrP) myStack.top()).p_val;
        myStack.pop();
        term = TermCell::TermDeref(term, deref);
        if (term->TermCellQueryProp(prop)) {
            res = true;
            break;
        }
        for (int i = 0; i < term->arity; i++) {

            tmp.p_val = term->args[i];
            myStack.push(tmp);
            //PStackPushP(stack, term->args[i]);
            tmp.i_val = (long) deref;
            myStack.push(tmp);
            // PStackPushInt(stack, deref);
        }
    }
    // PStackFree(stack);
    return res;
}

/***************************************************************************** 
 * Set the properties in all variable cells belonging to term.
 ****************************************************************************/
void TermCell::TermVarSetProp(DerefType deref, TermProp prop) {
    TermCell* term = this;
    //PStack_p stack = PStackAlloc();
    stack<IntOrP> myStack;
    IntOrP tmp;
    tmp.p_val = term;
    myStack.push(tmp);
    //PStackPushP(stack, term);
    tmp.i_val = (long) deref;
    //PStackPushInt(stack, deref);
    myStack.push(tmp);
    while (!myStack.empty()) {
        //deref = PStackPopInt(stack);
        deref = (DerefType) ((IntOrP) myStack.top()).i_val;
        myStack.pop();
        //term  = PStackPopP(stack);
        term = (TermCell*) ((IntOrP) myStack.top()).p_val;
        myStack.pop();
        term = TermCell::TermDeref(term, deref);
        if (term->IsVar()) {
            term->TermCellSetProp(prop);
        }
        for (int i = 0; i < term->arity; ++i) {
            tmp.p_val = term->args[i];
            myStack.push(tmp);
            //PStackPushP(stack, term->args[i]);
            tmp.i_val = (long) deref;
            myStack.push(tmp);
            // PStackPushInt(stack, deref);
        }
    }
    //PStackFree(stack);
}

/***************************************************************************** 
 * If prop is set in any variable cell in term, return true, otherwise false.
 ****************************************************************************/
bool TermCell::TermVarSearchProp(DerefType deref, TermProp prop) {
    TermCell* term = this;
    stack<IntOrP> myStack;


    bool res = false;
    IntOrP tmp;
    tmp.p_val = term;
    myStack.push(tmp);
    //PStackPushP(stack, term);
    tmp.i_val = (long) deref;
    myStack.push(tmp);
    //PStackPushInt(stack, deref);

    while (!myStack.empty()) {
        deref = (DerefType) ((IntOrP) myStack.top()).i_val;
        myStack.pop();
        //deref = PStackPopInt(stack);
        term = (TermCell*) ((IntOrP) myStack.top()).p_val;
        myStack.pop();
        //term  = PStackPopP(stack);
        term = TermCell::TermDeref(term, deref);
        if (term->IsVar() && term->TermCellQueryProp(prop)) {
            res = true;
            break;
        }
        for (int i = 0; i < term->arity; ++i) {
            tmp.p_val = term->args[i];
            myStack.push(tmp);
            //PStackPushP(stack, term->args[i]);
            tmp.i_val = (long) deref;
            myStack.push(tmp);
            // PStackPushInt(stack, deref);
        }
    }
    // PStackFree(stack);
    return res;
}

/*****************************************************************************
 * 应该放入 sig.cpp中 Thin wrapper around SigInsertId that also sets corresponding
 * properties for different identifier types.
 ****************************************************************************/
FunCode TermCell::TermSigInsert(Sig_p sig, const string& name, int arity, bool special_id, FuncSymbType type) {
    FunCode res;

    res = sig->SigInsertId(name, arity, special_id);
    switch (type) {
        case FuncSymbType::FSIdentInt:
            sig->SigSetFuncProp(res, FPIsInteger);
            break;
        case FuncSymbType::FSIdentFloat:
            sig->SigSetFuncProp(res, FPIsFloat);
            break;
        case FuncSymbType::FSIdentRational:
            sig->SigSetFuncProp(res, FPIsRational);
            break;
        case FuncSymbType::FSIdentObject:
            sig->SigSetFuncProp(res, FPIsObject);
            break;
        case FuncSymbType::FSIdentInterpreted:
            sig->SigSetFuncProp(res, FPInterpreted);
            break;
        default:
            /* Nothing */
            break;
    }
    return res;
}

/*****************************************************************************
 * 检查项是否为基项 ground 不包括变元项
 * 注意不检查　变元绑定情况 
 ****************************************************************************/
bool TermCell::IsGround() {
    if (TBTermIsGround())return true;

    if (IsVar())
        return false;
    vector<TermCell*> st;
    st.reserve(32);
    for (int i = 0; i < arity; ++i) {
        if (args[i]->IsVar()) {
            vector<TermCell*>().swap(st);
            return false;
        }
        st.push_back(args[i]);
    }
    TermCell* term;
    while (!st.empty()) {
        term = st.back();
        st.pop_back();
        for (int i = 0; i < term->arity; ++i) {
            if (term->args[i]->IsVar()) {
                vector<TermCell*>().swap(st);
                return false;
            }
            st.push_back(term->args[i]);
        }
        //st.pop_back();
    }
    //释放内存
    vector<TermCell*>().swap(st);
    this->TermCellSetProp(TermProp::TPIsGround);
    return true;
}

/*****************************************************************************
 * Return true if f occurs in term, false otherwise. 
 ****************************************************************************/
bool TermCell::TermHasFCode(FunCode f) {

    if (fCode == f) {
        return true;
    }
    //zj:原来E使用的是递归， 现改为非递归，堆栈形式
    /*if(term->f_code == f)
      return true;   
   for(i=0; i<term->arity; i++){
      if(TermHasFCode(term->args[i], f))
         return true;      
   }
   return false;*/

    vector<TermCell*> st;
    st.reserve(arity);

    for (int i = 0; i < arity; ++i) {
        if (args[i]->fCode == f) {
            vector<TermCell*>().swap(st);
            return true;
        }
        st.push_back(args[i]);
    }
    TermCell* term;
    while (!st.empty()) {
        term = st.back();
        for (int i = 0; i < term->arity; ++i) {
            if (term->args[i]->fCode == f) {
                vector<TermCell*>().swap(st);
                return true;
            }
            st.push_back(term->args[i]);
        }
        st.pop_back();
    }
    //释放内存
    vector<TermCell*>().swap(st);
    return false;
}

/***************************************************************************** 
 * Return if the term contains unbound variables.#
 * Does not follow bindings.
 ****************************************************************************/
bool TermCell::TermHasUnboundVariables() {
    vector<TermCell*> st;
    st.push_back(this);
    TermCell* term;
    while (!st.empty()) {
        term = st.back();
        st.pop_back();
        if (term->IsVar()) {
            if (!term->binding) {
                vector<TermCell*>().swap(st);
                return true;
            }
        } else {
            for (int i = 0; i < term->arity; i++) {
                st.push_back(term->args[i]);
            }
        }
    }
    vector<TermCell*>().swap(st);
    return false;
}

/***************************************************************************** 
 * Return true if t is of the form f(X1...Xn) with n>=arity.
 ****************************************************************************/
bool TermCell::TermIsDefTerm(int minArity) {
    int i;


    if (IsVar()) {
        return false;
    }
    if (arity < minArity) {
        return false;
    }
    if (this->ComputeTermStandardWeight() != (DEFAULT_FWEIGHT + arity * DEFAULT_VWEIGHT)) {
        return false;
    }
    for (i = 0; i < arity; ++i) {
        args[i]-> TermCellDelProp(TermProp::TPOpFlag);
    }
    //这个代码有问题？
    for (i = 0; i < arity; ++i) {
        if (args[i]->TermCellQueryProp(TermProp::TPOpFlag)) {
            return false;
        }
        args[i]->TermCellSetProp(TermProp::TPOpFlag);
    }
    return true;
}

/***************************************************************************** 
 * Insert all variables with properties prop in term into tree. 
 * Return number of new variables. 
 ****************************************************************************/
long TermCell::TermCollectPropVariables(SplayTree<PTreeCell> *tree, TermProp prop) {
    long res = 0;
    vector<TermCell*> st;
    st.push_back(this);
    TermCell* term;
    while (!st.empty()) {
        term = st.back();
        st.pop_back();
        if (term->IsVar() && term->TermCellQueryProp(prop)) {
            if (tree->TreeStore(term)) {
                res++;
            }
        } else {
            for (int i = 0; i < term->arity; ++i) {
                st.push_back(term->args[i]);
            }
        }
    }
    vector<TermCell*>().swap(st);
    return res;
}

/*-----------------------------------------------------------------------
//
// Function: TermFindMaxVarCode()
//
//   Return largest (absolute, i.e. largest negative) f_code of any
//   variable in term.
//
// Global Variables: 
//
// Side Effects    : 
//
/----------------------------------------------------------------------*/

FunCode TermCell::TermFindMaxVarCode() {
    TermCell* term = this;
    int i;
    long res, tmp;

    if (term->IsVar()) {
        return term->fCode;
    } else {
        res = 0;
        for (i = 0; i < term->arity; i++) {
            tmp = term->args[i]->TermFindMaxVarCode();
            res = MIN(res, tmp);
        }
    }
    return res;
}


/*--------------------------------------------------------------------------- 
 - E中Term 类中原有的方法
 ---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: TermCopyKeepVars()
//
//   Return a copy of a given term. The new term will be unshared
//   (except, of coure, for the variables) even if the original term
//   was shared. Variable cells will not be copied. Note that printing
//   such a term might be confusing, since two variables with the same
//   f_code may indeed be different!
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

TermCell* TermCell::TermCopyKeepVars(DerefType deref) {
    TermCell* handle;
    int i;


    TermCell* source = this;

    source = TermCell::TermDeref(source, deref);

    if (source->IsVar()) {
        return source;
    }
    handle = source->TermEquivCellAlloc(NULL, NULL);

    for (i = 0; i < handle->arity; i++) /* Hack: Loop will not be entered if
				     arity = 0 */ {
        handle->args[i] = handle->args[i]->TermCopyKeepVars(deref);
    }
    return handle;
}

/*****************************************************************************
 * 根据参数source，若为变元则在varBank中查找该变元项，否则拷贝该项；返回非共享项指针，
 * 注：非共享项，指变元项和不存储到TermBank中的项． 
 ****************************************************************************/

TermCell* TermCell::TermEquivCellAlloc(TermBank* tb, Lit_p litptr) {
    TermCell* handle = nullptr; //new TermCell();

    if (IsVar()) //变元项　
    {
        handle = tb->VarInert(fCode, litptr);
    } else {
        // handle=new TermCell(*this);
        handle = TermCell::TermTopCopy(this);
    }
    return handle;
}

bool TermCell::equalStruct(TermCell* term) {
    if (this->TBTermIsGround() && term->TBTermIsGround()) {
        if (this == term) //都是基文字且相同 
            return true;
        else
            return false;
    }
    TermCell* t1 = TermCell::TermDerefAlways(this);
    TermCell* t2 = TermCell::TermDerefAlways(term);


    if (t1->fCode != t2->fCode) {
        return false;
    }
    if (t1 == t2) {
        return true;
    } else if (t1->IsVar() && t2->IsVar()) {//若两者均为变元，则不相同则为false
        return false;
    }

    //注意,尽管 不同子句的x1 存储地方不同,地址不相同,但是fcode一定是相同的.
    for (int i = 0; i < t1->arity; i++) {
        if (!t1->args[i]->equalStruct(t2->args[i])) {
            return false;
        }
    }
    return true;
}

/*-----------------------------------------------------------------------
//
// Function: TermStructEqual()
//
//   Return true if the two terms have the same structure. Follows
//   bindings. 
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/


bool TermCell::TermStructEqual(TermCell* t1, TermCell* t2) {

    t1 = TermCell::TermDerefAlways(t1);
    t2 = TermCell::TermDerefAlways(t2);

    if (t1 == t2) {
        return true;
    }
    if (t1->fCode != t2->fCode) {
        return false;
    }
    for (int i = 0; i < t1->arity; i++) {
        if (!TermStructEqual(t1->args[i], t2->args[i])) {
            return false;
        }
    }
    return true;
}

/*-----------------------------------------------------------------------
//
// Function: TermStructEqualNoDeref()
//
//   Return true if the two terms have the same structures. Ignores
//   bindings. 
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/


bool TermCell::TermStructEqualNoDeref(TermCell* t1, TermCell* t2) {

    int i;
    if (t1 == t2) {
        return true;
    }
    if (t1->fCode != t2->fCode) {
        return false;
    }
    for (i = 0; i < t1->arity; i++) {
        if (!TermCell::TermStructEqualNoDeref(t1->args[i], t2->args[i])) {
            return false;
        }
    }
    return true;
}

/*-----------------------------------------------------------------------
//
// Function: TermStructEqualNoDerefHardVars()
//
//   Return true if the two terms have the same structures. Compares
//   variables by pointer, everything else by structure.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool TermCell::TermStructEqualNoDerefHardVars(TermCell* t1, TermCell* t2) {
    if (t1 == t2) {
        return true;
    }
    if (t1->IsVar()) /* Variables are only equal if the pointers are */ {
        return false;
    }
    if (t1->fCode != t2->fCode)
        return false;

    for (int i = 0; i < t1->arity; ++i) {
        if (!TermCell::TermStructEqualNoDerefHardVars(t1->args[i], t2->args[i])) {
            return false;
        }
    }
    return true;
}

/*-----------------------------------------------------------------------
//
// Function: TermStructEqualDeref()
//
//   Return true if the two terms have the same
//   structures. Dereference both terms as designated by deref_1,
//   deref_2. 
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool TermCell::TermStructEqualDeref(TermCell* t1, TermCell* t2, DerefType deref_1,
        DerefType deref_2) {
    int i;

    t1 = TermCell::TermDeref(t1, deref_1);
    t2 = TermCell::TermDeref(t2, deref_2);

    if ((t1 == t2) && (deref_1 == deref_2)) {
        return true;
    }
    if (t1->fCode != t2->fCode) {
        return false;
    }
    for (i = 0; i < t1->arity; i++) {
        if (!TermCell::TermStructEqualDeref(t1->args[i], t2->args[i], deref_1, deref_2)) {
            return false;
        }
    }
    return true;
}

/*-----------------------------------------------------------------------
//
// Function: TermStructEqualDerefHardVars()
//
//   Return true if the two terms have the same
//   structures. Dereference both terms as designated by deref_1,
//   deref_2. Variables are compared by pointer, not by f_code.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool TermCell::TermStructEqualDerefHardVars(TermCell* t1, TermCell* t2, DerefType deref_1,
        DerefType deref_2) {
    int i;

    t1 = TermDeref(t1, deref_1);
    t2 = TermDeref(t2, deref_2);

    if ((t1 == t2) && (deref_1 == deref_2)) {
        return true;
    }
    if (t1->fCode != t2->fCode || t1->IsVar()) {
        return false;
    }
    for (i = 0; i < t1->arity; i++) {
        if (!TermCell::TermStructEqualDerefHardVars(t1->args[i], t2->args[i], deref_1, deref_2)) {
            return false;
        }
    }
    return true;
}

/*-----------------------------------------------------------------------
//
// Function: TermStructWeightCompare()
//
//   Compare two terms based on just structural criteria: First
//   compare standard-weight, then compare top-symbol arity, then
//   compare subterms lexicographically. $true is always minimal.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

int TermCell::TermStructWeightCompare(TermCell* t1, TermCell* t2) {

    long res;
    short i;
    CompareResult subres;
    assert(t2);
    if (t1->fCode == (FunCode) DerefType::TRUECODE) {
        assert(t1->arity == 0);
        if (t2->fCode == (FunCode) DerefType::TRUECODE) {
            assert(t2->arity == 0);
            return 0;
        }
        return -1;
    }
    if (t2->fCode == (FunCode) DerefType::TRUECODE) {
        assert(t2->arity == 0);
        return 1;
    }
    res = t1->ComputeTermStandardWeight() - t2->ComputeTermStandardWeight();
    if (res) {
        return res;
    }
    if (t1->IsVar()) { /* Then t2 also is a variable due to equal weights! */
        assert(t2->IsVar());
        return 0;
    }
    res = t1->arity - t2->arity;
    if (res) {
        return res;
    }
    for (i = 0; i < t1->arity; i++) {
        subres = (CompareResult) TermCell::TermStructWeightCompare(t1->args[i], t2->args[i]);
        if ((int) subres > 0) {
            return (int) subres;
        }
    }
    return 0;
}

/*-----------------------------------------------------------------------
//
// Function: TermLexCompare()
//
//   Compare two terms lexicographically by f_codes.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

int TermCell::TermLexCompare(TermCell* t1, TermCell* t2) {

    long res;
    int i;

    res = t1->fCode - t2->fCode;
    if (res) {
        return res;
    }
    assert(t1->arity == t2->arity);
    for (i = 0; i < t1->arity; i++) {
        res = TermLexCompare(t1->args[i], t2->args[i]);
        if (res) {
            return res;
        }
    }
    return 0;
}

/*-----------------------------------------------------------------------
//
// Function: TermIsSubterm()
//
//   Return true if test is a subterm to super. Parameterized by the
//   Equal-Funktion for terms.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool TermCell::TermIsSubterm(TermCell* test, DerefType deref, TermEqulType EqualTest) {
    TermCell* super = this;
    int i;

    super = TermCell::TermDeref(super, deref);
    bool isEqual = false;
    switch (EqualTest) {
        case TermEqulType::PtrEquel:
            isEqual = (super == test);
            break;
        case TermEqulType::StructEqual:
            switch (deref) {
                case DerefType::DEREF_NEVER:
                    isEqual = TermStructEqualNoDeref(super, test);
                    break;
                case DerefType::DEREF_ALWAYS:
                    isEqual = TermStructEqual(super, test);
                    break;
                default:
                    assert(false);
                    cout << "比较给定DerefType参数出错" << endl;
                    break;
            }

            break;
        default:
            break;
    }
    if (isEqual) {
        return true;
    }
    for (i = 0; i < super->arity; i++) {
        if (super->args[i]->TermIsSubterm(test, deref, EqualTest)) {
            return true;
        }
    }
    return false;
}

bool TermCell::TermIsStructSubterm(TermCell* test) {
    return TermIsSubterm(test, DerefType::DEREF_ALWAYS, TermEqulType::StructEqual);
}

/*-----------------------------------------------------------------------
//
// Function: TermIsSubtermDeref()
//
//   Return true if test is a subterm to super. Uses
//   TermStructEqualDeref() for equal test.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool TermCell::TermIsSubtermDeref(TermCell* test, DerefType
deref_super, DerefType deref_test) {
    int i;
    TermCell* super = this;
    super = TermCell::TermDeref(super, deref_super);
    if (TermCell::TermStructEqualDeref(super, test, deref_super, deref_test)) {
        return true;
    }
    for (i = 0; i < super->arity; i++) {
        if (super->args[i]->TermIsSubtermDeref(test, deref_super, deref_test)) {
            return true;
        }
    }
    return false;
}

/// 计算项的相关统计信息 包括权重,函数嵌套,最大变元ID
/// \param t

void ComputeTermInfo(TermCell* t) {
    t->SetTermWeight(0);
    t->SetFuncLayer(0);
    t->SetVarCount(0);
    t->SetMaxVarDepth(0);

    if (0 == t->arity) {//没有子项返回
        if (t->IsVar()) {
            t->SetVarCount(1);
            t->SetMaxVarId(-t->fCode);
        }
        return;
    }
    for (int i = 0; i < t->arity; ++i) {
        TermCell* subT = t->args[i];
        ComputeTermInfo(subT);
        t->SetTermWeight(t->GetTermWeight() + subT->GetTermWeight());
        t->SetFuncLayer(MAX(t->GetFuncLayer(), (subT->GetFuncLayer() + 1)));

        if (subT->GetVarCount() > 0) {
            t->SetMaxVarDepth(MAX(t->GetMaxVarDepth(), (subT->GetMaxVarDepth() + 1)));
            t->SetMaxVarId(MAX(t->GetMaxVarId(), subT->GetMaxVarId())); //最大变元ID
            t->SetVarCount(t->GetVarCount() + subT->GetVarCount());

        }
    }
}

/***************************************************************************** 
 * 遍历项-计算项的统计信息:包括1.字符权重.2.变元个数,3.函数嵌套最大深度,4.变元嵌套深度;5. counting variables as vweight and function symbols as fweight.
 ****************************************************************************/
long TermCell::ComputeTermWeight(long vweight, long fweight) {
    TermCell* term = this;
    long res = 0;
    stack<TermCell*> myStack;
    assert(term);
    TermCell* handle = nullptr;
    myStack.push(term);
    while (!myStack.empty()) {
        handle = myStack.top();
        myStack.pop();
        handle = TermCell::TermDerefAlways(handle);
        if (handle->IsVar()) {
            res += vweight;
        } else if (handle->TBTermIsGround()) {
            res += handle->GetTermWeight();
        } else {
            res += fweight;
            for (int i = 0; i < handle->arity; ++i) {
                myStack.push(handle->args[i]);
            }
        }
    }
    //weight = res;
    return res;
}

/*****************************************************************************
 * Return a weighted sum of the function symbols weights (and variable weights) in the term. 
 ****************************************************************************/
long TermCell::TermFsumWeight(long vweight, long flimit, vector<long>&fweights, long default_fweight) {
    TermCell* term = this;
    long res = 0;
    stack<TermCell*>myStack;

    myStack.push(term);
    TermCell* handle;
    assert(term);

    while (!myStack.empty()/*PStackEmpty(stack)*/) {
        handle = myStack.top();
        myStack.pop();
        if (handle->IsVar()) {
            res += vweight;
        } else {
            int i;

            res += (handle->fCode < flimit) ?
                    fweights[handle->fCode] : default_fweight;

            for (i = 0; i < handle->arity; ++i) {

                myStack.push(handle->args[i]);
            }
        }
    }
    // PStackFree(stack);

    return res;

}

/*-----------------------------------------------------------------------
//
// Function: TermNonLinearWeight()
//
//   Compute the weight of a term, counting variables that occur for
//   the first time as vlweight, varaibes that reoccur as vweight, and
//   function symbols as fweight.
//
// Global Variables: -
//
// Side Effects    : Memory operations for the stack used.
//
/----------------------------------------------------------------------*/

long TermCell::TermNonLinearWeight(long vlweight, long vweight, long fweight) {

    TermCell* term = this;
    long res = 0;
    // PStack_p stack = PStackAlloc();
    stack<TermCell*> myStack;
    TermCell* handle;
    assert(term);

    term->TermDelProp(DerefType::DEREF_NEVER, TermProp::TPOpFlag);
    myStack.push(term);
    while (/*!PStackEmpty(stack)*/!myStack.empty()) {
        //handle = PStackPopP(stack);
        handle = myStack.top();
        myStack.pop();
        if (handle->IsVar()) {
            if (handle->TermCellQueryProp(TermProp::TPOpFlag)) {
                res += vweight;
            } else {
                handle->TermCellSetProp(TermProp::TPOpFlag);
                res += vlweight;
            }
        } else {
            int i;

            res += fweight;

            for (i = 0; i < handle->arity; i++) {

                myStack.push(handle->args[i]);
                // PStackPushP(stack,);
            }
        }
    }
    //PStackFree(stack);

    return res;
}

/*-----------------------------------------------------------------------
//
// Function: TermSymTypeWeight() 
//
//   Compute the weight of a term, giving different weight to
//   variables, constants, function symbols and predicates.
//
// Global Variables: -
//
// Side Effects    : Memory operations for the stack.
//
/----------------------------------------------------------------------*/

long TermCell::TermSymTypeWeight(long vweight, long fweight, long
        cweight, long pweight) {
    TermCell* term = this;
    long res = 0;
    //PStack_p stack = PStackAlloc();
    stack<TermCell*> myStack;
    TermCell* handle;

    assert(term);

    //PStackPushP(stack, term);
    myStack.push(term);
    while (!myStack.empty()/*!PStackEmpty(stack)*/) {
        //handle = PStackPopP(stack);
        handle = myStack.top();
        myStack.pop();
        if (handle->IsVar()) {
            res += vweight;
        } else {
            int i;

            if (handle->TermCellQueryProp(TermProp::TPPredPos)) {
                res += pweight;
            } else if (handle->arity == 0) {
                res += cweight;
            } else {
                res += fweight;
            }
            for (i = 0; i < handle->arity; i++) {

                myStack.push(handle->args[i]);
                // PStackPushP(stack, );
            }
        }
    }
    //PStackFree(stack); 
    return res;
}



//zj:replace.cpp 中相关的方法

///* 给项term添加重写项replace，以及replace所在子句 demod  */
//void TermCell::TermAddRWLink(TermCell* replace, Clause* demod, bool sos, RWResultType type) {
//
//    assert(replace);
//    assert(this != replace);
//    TermCellSetProp(TPIsRewritten);
//    if (type == RWAlwaysRewritable) {
//        TermCellSetProp(TPIsRRewritten);
//    }
//    rw_data.rw_desc.replace = replace;
//    rw_data.rw_desc.demod = demod;
//
//
//    if (sos) {
//        TermCellSetProp(TPIsSOSRewritten);
//    }
//}

/*****************************************************************************
 * Delete rewrite link from term.  
 ****************************************************************************/

//void TermCell::TermDeleteRWLink() {
//
//    TermCellDelProp(TPIsRewritten | TPIsRRewritten | TPIsSOSRewritten);
//    rw_data.rw_desc.replace = nullptr;
//    rw_data.rw_desc.demod = nullptr;
//
//}

/***************************************************************************** 
 * 返回一个项的最后重写项 Return the last term in an existing rewrite link chain.
 ****************************************************************************/
//TermCell* TermCell::TermFollowRWChain() {
//    TermCell* term;
//    /* printf("Starting chain\n"); */
//    while (TermIsRewritten()) {
//        term = TermRWReplaceField();
//        /* printf("Following chain\n"); */
//        assert(term);
//    }
//    return term;
//}

/*-----------------------------------------------------------------------
//
// Function: TermAddSymbolDistributionLimited()
//
//   Count occurences of function symbols with f_code<limit in
//   dist_array. Terms are not dereferenced!
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void TermCell::TermAddSymbolDistributionLimited(vector<long>&dist_array, long limit) {
    TermCell* term = this;
    //PStack_p stack = PStackAlloc();
    stack<TermCell*> tmpStack;
    assert(term);

    //PStackPushP(stack, term);
    tmpStack.push(term);
    while (/*!PStackEmpty(stack)*/!tmpStack.empty()) {
        //term = PStackPopP(stack);
        term = tmpStack.top();
        tmpStack.pop();
        assert(term);

        if (!term->IsVar()) {
            int i;

            assert(term->fCode > 0);
            if (term->fCode < limit) {
                ++(dist_array[term->fCode]);
            }
            for (i = 0; i < term->arity; i++) {

                assert(term->args);

                tmpStack.push(term->args[i]);
                //PStackPushP(stack, );
            }
        }
    }
    //PStackFree(stack);
}

/*-----------------------------------------------------------------------
//
// Function: TermAddSymbolDistribExist()
//
//   Compute the distribution of symbols in term. Push all occuring
//   symbols onto exists (once ;-).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void TermCell::TermAddSymbolDistExist(vector<long>&dist_array, stack<long>& exists) {

    TermCell* term = this;
    //PStack_p stack = PStackAlloc();
    stack<TermCell*> tmpStack;

    assert(term);

    tmpStack.push(term);

    //PStackPushP(stack, term);

    while (/*!PStackEmpty(stack)*/!tmpStack.empty()) {
        //term = PStackPopP(stack);
        term = tmpStack.top();
        tmpStack.pop();
        assert(term);

        if (!term->IsVar()) {
            int i;

            assert(term->fCode > 0);
            if (!dist_array[term->fCode]) {
                exists.push(term->fCode);
                //PStackPushInt(exists,);
            }
            ++(dist_array[term->fCode]);

            for (i = 0; i < term->arity; i++) {

                assert(term->args);
                tmpStack.push(term->args[i]);
                //PStackPushP(stack, );
            }
        }
    }
    //PStackFree(stack);

}

/***************************************************************************** 
 * Add function symbol frequencies and deepest depth of a function
   symbol to the two arrays. This is an extension of the function
   above, this one does the extendet task in a single term
   traversal. Note that function symbols >=limit are counted in
   array[0] for both depth and frequency. 
// Side Effects    : Changes the arrays.
 * 记录 在重写过程中，每次重写函数项的使用频率以及函数嵌套层的变化。 *  eg.f(f(f(a))) -> f(b)
 ****************************************************************************/
void TermCell::TermAddSymbolFeaturesLimited(long depth, long *freq_array, long* depth_array, long limit) {
    TermCell* term = this;
    if (!term->IsVar()) {
        int i;

        if (term->fCode < limit) {
            freq_array[term->fCode]++;
            depth_array[term->fCode] = MAX(depth, depth_array[term->fCode]);
        } else {
            freq_array[0]++;
            depth_array[0] = MAX(depth, depth_array[0]);
        }
        for (i = 0; i < term->arity; i++) {

            term->args[i]->TermAddSymbolFeaturesLimited(depth + 1, freq_array, depth_array, limit);
        }
    }

}

/***************************************************************************** 
 * Add function symbol frequencies and deepest depth of a function
   symbol to the array. offset should be 0 for positive literals, 2
   for negative literals. Thus, the 4 features for a given f
   are stored at indices follows:
   - 4*f_code:   |C^+|_f
   - 4*f_code+1: d_f(C^+)
   - 4*f_code+2: |C^-|_f
   - 4*f_code+3: d_f(C^-)

 Side Effects    : Changes the arrays.
 ****************************************************************************/
void TermCell::TermAddSymbolFeatures(vector<long> &mod_stack, long depth, vector<long>&feature_array, long offset) {
    TermCell* term = this;
    if (!term->IsVar()) {
        int i;
        long findex = 4 * term->fCode + offset;

        if (feature_array[findex] == 0) {
            mod_stack.push_back(findex);
            //PStackPushInt(mod_stack, findex);
        }

        ++feature_array[findex];
        feature_array[findex + 1] = MAX(depth, feature_array[findex + 1]);
        for (i = 0; i < term->arity; i++) {

            term->args[i]->TermAddSymbolFeatures(mod_stack, depth + 1, feature_array, offset);
        }
    }

}

/*-----------------------------------------------------------------------
//
// Function: TermComputeFunctionRanks()
//
//   Assing an occurance rank to each symbol in term.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void TermCell::TermComputeFunctionRanks(long *rank_array, long *count) {

    TermCell* term = this;
    int i;

    if (term->IsVar()) {
        return;
    }
    for (i = 0; i < term->arity; i++) {
        term->args[i]-> TermComputeFunctionRanks(rank_array, count);
    }
    if (!rank_array[term->fCode]) {

        rank_array[term->fCode] = (*count)++;
    }
}

/*-----------------------------------------------------------------------
//
// Function: TermLinearize()
//
//   Put all subterms of term onto PStack in left-right preorder. Note
//   that for an empty stack, that makes the index of s on the stack
//   equal to its TermCPos. Returns number of subterms.
//
// Global Variables: -
//
// Side Effects    : Changes stack
//
/----------------------------------------------------------------------*/

long TermCell::TermLinearize(stack<TermCell*> &tmpStack) {
    TermCell* term = this;
    long res = 1;
    int i;
    tmpStack.push(term);
    //PStackPushP(stack, term);
    for (i = 0; i < term->arity; i++) {

        res += term->args[i]->TermLinearize(tmpStack);
    }
    return res;
}

/***************************************************************************** 
 * Traverse a tree and check if any one term cell occurs more than
//   once on any branch (which would make the term cyclic). Return the
//   first inconsistency found or NULL.
 ****************************************************************************/
TermCell* TermCell::TermCheckConsistency(DerefType deref) {

    SplayTree<PTreeCell> branch;

    printf("TermCheckConsistency...\n");
    TermCell* res = term_check_consistency_rek(branch, deref);
    assert(branch.IsEmpty());
    printf("...TermCheckConsistency\n");

    return res;
}

/***************************************************************************** 
 * Delete properties prop in term, return number of term cells with this property. 
 * Does assume that all subterms of a term without this property also do not carry it!
 ****************************************************************************/
long TermCell::TBTermDelPropCount(TermProp prop) {
    long count = 0;
    int i;
    TermCell* term;
    //PStack_p stack = PStackAlloc();
    vector<TermCell*> vstack;
    vstack.reserve(8);
    //PStackPushP(stack, term);
    vstack.push_back(this);
    while (!vstack.empty()) {
        term = vstack.back();
        vstack.pop_back();
        if (term->TermCellQueryProp(prop)) {
            term->TermCellDelProp(prop);
            count++;
            for (i = 0; i < term->arity; i++) {
                //PStackPushP(stack, term->args[i]);
                vstack.push_back(term->args[i]);
            }
        }
    }
    vector<TermCell*>().swap(vstack);

    return count;
}

long TermCell::TermCollectVariables(SplayTree<PTreeCell> *tree) {

    return TermCollectPropVariables(tree, TermProp::TPIgnoreProps);
}

/***************************************************************************** 
 * Add all new occurences of function symbol to res_stack and mark them 
 * as no-longer-new in f_occur. Return number of new function symbols added.
 ****************************************************************************/
long TermCell::TermAddFunOcc(vector<int>*fOccur, vector<IntOrP>*resStack) {
    long res = 0;
    vector<TermCell*> vecSt;
    vecSt.reserve(8);
    vecSt.push_back(this);
    TermCell* term;
    while (!vecSt.empty()) {
        term = vecSt.back();
        vecSt.pop_back();
        if (!term->IsVar()) {
            if (!fOccur->at(term->fCode))//PDArrayElementInt(f_occur, term->f_code))
            {
                ++res;
                IntOrP p;
                resStack->push_back(p);
                resStack->back().i_val = term->fCode;
                fOccur->at(term->fCode) = 1;

            }
            for (int i = 0; i < term->arity; ++i) {
                vecSt.push_back(term->args[i]);
            }
        }
    }
    vecSt.clear();
    vector<TermCell*>().swap(vecSt);

    return res;
}

/*****************************************************************************
 * Given a compact term position in t, encode it into the given full postion. 
 ****************************************************************************/
void TermCell::UnpackTermPos(vector<IntOrP>& pos, long cpos)//object = Term_p t
{
    int i;
    Term_p t = this;
    //PStackReset(pos);
    pos.clear();
    while (cpos > 0) {
        assert(!t->IsVar());
        cpos -= DEFAULT_FWEIGHT;
        assert(cpos >= 0);

        //PStackPushP(pos, t);
        IntOrP tmpPtr;
        tmpPtr.p_val = t;
        pos.push_back(tmpPtr);
        for (i = 0; i < t->arity; i++) {
            if (cpos < t->args[i]->ComputeTermStandardWeight()) {
                //PStackPushInt(pos, i);
                IntOrP tmp;
                tmp.i_val = i;
                pos.push_back(tmp);
                t = t->args[i];
                break;
            }
            cpos -= t->args[i]->ComputeTermStandardWeight();
            assert(cpos >= 0);
        }
    }
}
