/*
 *Contents:
 *  Definitions for term banks - i.e. shared representations of terms as defined in cte_terms.h. 
 *  Uses the same struct, but adds administrative stuff and functionality for sharing.
 *  
 *  There are two sets of funktions for the manangment of term trees:
 *  Funktions operating only on the top cell, and functions descending
 *  the term structure. Top level functions implement a conventional splay 
 *  tree with key f_code.masked_properties.entry_nos_of_args and are implemented in cte_termtrees.[ch]
 * 
 * File:   TermBank.cpp
 * Author: zj 
 * 
 * Created on 2017年2月24日, 下午4:46
 */
#include "TermBank.h"
#include "INOUT/FileOp.h"
#include "TermCell.h"
#include "Global/Environment.h"

bool TermBank::TBPrintInternalInfo = false;
bool TermBank::TBPrintDetails = false;

/*---------------------------------------------------------------------*/
/*                    Constructed Function                             */

/*---------------------------------------------------------------------*/
TermBank::TermBank(uint16_t claIdent) : claId(claIdent) {

    //assert(sig);
    inCount = 0;
    //rewriteSteps = 0; //replace次数 
    //extIndex.reserve(10000); //注意，只是预留10W空间，并没有初始化，(初始化可以考虑用resize)
    garbageState = TermProp::TPIgnoreProps;
    //初始化 varBank
    shareVars = nullptr; //若是全局基项bank 则不占内存空间 ； new VarBank();
    //初始化 TermCellStore
     termStore = new TermCellStore(8);
}

TermBank::~TermBank() {
    //assert(!sig);

    /* printf("TBFree(): %ld\n", TermCellStoreNodes(&(junk->term_store)));*/
   // termStore->TermCellStoreExit();
    DelPtr(termStore);
    DelPtr(shareVars);
    extIndex.clear();
    vector<TermCell*>().swap(extIndex);
    //assert(assert.IsEmpty());
    //TBCellFree(junk);
}


/*---------------------------------------------------------------------*/
/*                  Member Function--private                           */
/*---------------------------------------------------------------------*/

/***************************************************************************** 
 * 根据插入顺序输出 -- Print the terms as a dag in the order of insertion.
 ****************************************************************************/
void TermBank::tb_print_dag(FILE *out, NumTree_p spNode) {

    //中序遍历 伸展树 注意.spNode是节点
    if (spNode == nullptr) {
        return;
    }

    tb_print_dag(out, spNode->lson);

    TermCell* term = (TermCell*) spNode->val1.p_val;

    fprintf(out, "*%ld : ", term->entryNo);

    if (term->IsVar()) {
        term->VarPrint(out);
    } else {
        string sigName;
        Env::getSig()->SigFindName(term->fCode, sigName);
        fputs(sigName.c_str(), out);
        if (!term->IsConst()) {
            assert(term->arity >= 1);
            assert(term->args);
            putc('(', out);

            fprintf(out, "*%ld", term->args[0]->TBCellIdent());
            for (int i = 1; i < term->arity; ++i) {
                putc(',', out);
                fprintf(out, "*%ld", term->args[i]->TBCellIdent());
            }
            putc(')', out);
        }
        printf("   =   ");
        term->TermPrint(out, DerefType::DEREF_NEVER);
        //zj-add
        //fprintf(out, " :weight->%lf", term->zjweight);
    }
    if (TermBank::TBPrintInternalInfo) {
        fprintf(out, "\t/*  Properties: %10d */", (int) term->properties);
    }
    fprintf(out, "\n");
    tb_print_dag(out, spNode->rson);
}

/***************************************************************************** 
 * 转换一个子项 --Parse a subterm, i.e. a term which cannot start with a predicate symbol.
 ****************************************************************************/
TermCell* TermBank::tb_subterm_parse(Scanner* in) {
    TermCell* res = TBTermParseReal(in, true);
    Sigcell* sig = Env::getSig();
    if (!res->IsVar()) {
        if (sig->SigIsPredicate(res->fCode)) {//此处有错误
            in->AktTokenError("Predicate used as function symbol in preceeding term", false);
        }
        sig->SigSetFunction(res->fCode, true);
    }
    return res;
}

/*****************************************************************************
 * 转换一个用逗号作为分割符的项列表 -- Parse a list of terms (comma-separated and enclosed in brackets)
 * into an array of (shared) term pointers. See TermParseArgList() in cte_terms.c for more. 
 ****************************************************************************/
int TermBank::tb_term_parse_arglist(Scanner* in, TermCell*** argAnchor, bool isCheckSymbProp) {
    in->AcceptInpTok(TokenType::OpenBracket);
    if (in->TestInpTok(TokenType::CloseBracket)) {
        in->NextToken();
        *argAnchor = nullptr;
        return 0;
    }


    vector<TermCell*> vectTerm;
    vectTerm.reserve(10);
    //TermCell* tmp = isCheckSymbProp ? tb_subterm_parse(in) : TBTermParseReal(in, false);

    vectTerm.push_back(isCheckSymbProp ? tb_subterm_parse(in) : TBTermParseReal(in, false));

    int arity = 1;
    //arity++;
    while (in->TestInpTok(TokenType::Comma)) {
        in->NextToken();

        vectTerm.push_back(isCheckSymbProp ? tb_subterm_parse(in) : TBTermParseReal(in, false));

        arity++;
    }
    in->AcceptInpTok(TokenType::CloseBracket);
    assert(arity == vectTerm.size());
    *argAnchor = new TermCell*[arity]; // TermArgArrayAlloc(arity);
    for (int i = 0; i < arity; ++i) {
        (*argAnchor)[i] = vectTerm[i];
    }
    vector<TermCell*>().swap(vectTerm);
    //SizeFree(handle, size * sizeof (TermCell*));
    return arity;
}

int TermBank::tb_term_parse_arglist(Scanner* in, TermCell* term, bool isCheckSymbProp) {
    in->AcceptInpTok(TokenType::OpenBracket);
    term->weight = DEFAULT_FWEIGHT;

    if (in->TestInpTok(TokenType::CloseBracket)) {
        //没有子项
        in->NextToken();
        term->args = nullptr;
        return 0;
    }
    vector<TermCell*> vectTerm;
    vectTerm.reserve(8);
    TermCell* subT = nullptr;
    //读取所有子项
    uint16_t uFuncLayer = 0;
    while (true) {
        subT = isCheckSymbProp ? tb_subterm_parse(in) : TBTermParseReal(in, false);
        term->weight += subT->weight;
        term->uVarCount += subT->uVarCount; //记录变元个数
        //计算函数嵌套层数
        uFuncLayer = MAX(uFuncLayer, subT->uMaxFuncLayer);
        ++term->arity;
        vectTerm.push_back(subT);
        if (!in->TestInpTok(TokenType::Comma)) {
            break;
        }
        in->NextToken();
    }
    //debug    if(term->fCode==245)         cout<<endl;


    in->AcceptInpTok(TokenType::CloseBracket);

    assert(term->arity == vectTerm.size());
    term->uMaxFuncLayer = (0 == term->arity) ? 0 : (++uFuncLayer);

    term->args = new TermCell*[term->arity]; // TermArgArrayAlloc(arity);
    memcpy(term->args, &vectTerm[0], sizeof (TermCell*) * term->arity);
    vector<TermCell*>().swap(vectTerm);

    return term->arity;
}

/***************************************************************************** 
 * Insert a term into the term bank for which the subterms are 
 * already in the term bank. Will reuse or destroy the top cell!
 * 插入一个项到termbank中.但该项的子项已经存在,重用或删除 top cell! 
 ****************************************************************************/
TermCell* TermBank::TBTermTopInsert(TermCell* t) {
    assert(t);
    assert(!t->IsVar());

    if (0 == t->uVarCount) {
        t->TermCellSetProp(TermProp::TPIsGround);
        return Env::getGTbank()->GTermTopInsert(t);
    }
    //含变元的项
    assert(!t->TermCellQueryProp(TermProp::TPIsGround));
    TermCell* newTerm = termStore->TermCellStoreInsert(t);
    if (newTerm) { /* TermCell node already existed, just add properties */
        newTerm->properties = (TermProp) ((int32_t) newTerm->properties | (int32_t) t->properties)/*& bank->prop_mask*/;
        DelPtr(t);
        return newTerm;
    } else { /*TermCell node not existed, insert subterms.PS,计算变元个数和函数嵌套个数*/
        t->entryNo = ++(inCount);
        assert(t->TermStandardWeight() == t->TermWeight(DEFAULT_VWEIGHT, DEFAULT_FWEIGHT));
        assert((t->IsGround() == false) == (t->TBTermIsGround() == false));
    }
    return t;
}

/***************************************************************************** 
 * Parse a LOP list into an (shared) internal $cons list.
 ****************************************************************************/
TermCell* TermBank::tb_parse_cons_list(Scanner * in, bool isCheckSymbProp) {
    assert(Sigcell::SigSupportLists);
    in->AcceptInpTok(TokenType::OpenSquare);
    TermCell* handle = DefaultSharedTermCellAlloc();
    TermCell* current = handle;
    vector<TermCell*> st;
    if (!in->TestInpTok(TokenType::CloseSquare)) {
        current->fCode = (FunCode) DerefType::CONSCODE;
        current->arity = 2;
        current->args = new TermCell*[2];

        current->args[0] = TBTermParseReal(in, isCheckSymbProp);
        current->args[1] = DefaultSharedTermCellAlloc();
        current = current->args[1];
        st.push_back(current);

        while (in->TestInpTok(TokenType::Comma)) {
            in->NextToken();
            current->fCode = (FunCode) DerefType::CONSCODE;
            current->arity = 2;
            current->args = new TermCell*[2];
            current->args[0] = TBTermParseReal(in, isCheckSymbProp);
            current->args[1] = new TermCell();
            current = current->args[1];
            st.push_back(current);
        }
        current = st.back();
        st.pop_back();
    }
    in->AcceptInpTok(TokenType::CloseSquare);
    current->fCode = (FunCode) DerefType::NILCODE;

    /* Now insert the list into the bank */
    handle = TBTermTopInsert(current);

    while (!st.empty()) {
        current = st.back();
        current->args[1] = handle;
        handle = TBTermTopInsert(current);
        st.pop_back();
    }
    vector<TermCell*>().swap(st);
    return handle;
}

TermCell* TermBank::tb_termtop_insert(TermCell* t) {

    assert(t);
    assert(!t->IsVar());

    TermCell* newTerm = termStore->TermCellStoreInsert(t);

    if (newTerm) /* Term node already existed, just add properties */ {
        newTerm->properties = (TermProp) ((int32_t) newTerm->properties | (int32_t) t->properties)/*& bank->prop_mask*/;
        DelPtr(t);
        return newTerm;
    } else {

        t->entryNo = ++(inCount);
        t->TermCellAssignProp(TermProp::TPGarbageFlag, garbageState);
        t->TermCellSetProp(TermProp::TPIsGround); /* Groundness may change below */

        t->weight = DEFAULT_FWEIGHT;
        for (int i = 0; i < t->arity; ++i) {
            assert(t->args[i]->IsShared() || t->args[i]->IsVar());
            t->weight += t->args[i]->weight;
            if (!t->args[i]->TermCellQueryProp(TermProp::TPIsGround)) {
                t->TermCellDelProp(TermProp::TPIsGround);
            }
        }
        assert(t->TermStandardWeight() == t->TermWeight(DEFAULT_VWEIGHT, DEFAULT_FWEIGHT));
        assert((t->IsGround() == 0) == (t->TBTermIsGround() == 0));
    }
    return t;
}



/*---------------------------------------------------------------------*/
/*                  Member Function--public                           */
/*---------------------------------------------------------------------*/

/*****************************************************************************
 * 解析scanner对象为一个Term,并存储到termbank中. 
 * Supports abbreviations. This function will _not_ set the TPTopPos property on top terms 
 * while parsing. It will or will not check and set symbol properties (function symbol, 
 * predicate symbol), depending on the check_symb_prop parameter. 
 ****************************************************************************/
TermCell* TermBank::TBTermParseReal(Scanner * in, bool isCheckSymbProp) {
    FuncSymbType idType;
    TokenCell* token = in->AktToken();
    TermCell* handle = nullptr;
    string errpos;
    /* Test for abbreviation */
    if (in->TestInpTok(TokenType::Mult)) {//读取 符号"*"
        TermProp properties = TermProp::TPIgnoreProps;
        in->NextToken();
        long abbrev = in->ParseInt();
        if (in->TestInpTok((TokenType) ((uint64_t) TokenType::Colon | (uint64_t) TokenType::Slash))) { /* 读取 符号":"或者"\"  -- This _defines_ the abbrev! */
            if (extIndex[abbrev] != nullptr) {//[abbrev]  PDArrayElementP(bank->ext_index, abbrev))           
                /* Error: Abbreviation defined twice */
                errpos = "";
                in->AktToken()->PosRep(errpos);
                errpos += "Abbreviation *" + to_string(abbrev) + " already defined";
                cout << "SYNTAX_ERROR:" + errpos << endl;
            }
            if (in->TestInpTok(TokenType::Slash)) {//读取 或者"\"
                in->NextToken();
                properties = (TermProp) in->ParseInt();
            }
            in->NextToken();
            handle = TBTermParseReal(in, isCheckSymbProp); /* Elegant, aint it? */
            if ((uint64_t) properties) {
                TBRefSetProp(&handle, properties);
            }
            /* printf("# TermCell %ld = %ld\n", abbrev, handle->entry_no); */
            extIndex[abbrev] = handle; //存储缩写
        } else { /* This references the abbrev */
            handle = extIndex[abbrev];
            if (!handle) {
                /* Error: Undefined abbrev */
                errpos = "";
                in->AktToken()->PosRep(errpos);
                errpos += "Abbreviation *" + to_string(abbrev) + " undefined";
                cout << "SYNTAX_ERROR:" + errpos << endl;
            }
        }
    } else {
        /* 正常term Normal term stuff, bloated because of the nonsensical SETHEO
           syntax */
        if (Sigcell::SigSupportLists && in->TestInpTok(TokenType::OpenSquare)) { //测试符号 [] 应该表示为常量列表
            handle = tb_parse_cons_list(in, isCheckSymbProp);
        } else {
            //ZJ主要读取 流程
            string idStr;
            FuncSymbType id_type;
            if ((id_type = TermCell::TermParseOperator(in, idStr)) == FuncSymbType::FSIdentVar) {

                //若为变元符号    将该项插入文字所在子句的 子句级共享变元集中
                handle = this->VarInert(idStr, this->claId);
                handle->uVarCount = 1; //设置变元数=1
                handle->uMaxFuncLayer = 0;
                handle->TermCellDelProp(TermProp::TPIsGround);
                handle->weight = DEFAULT_VWEIGHT;

            } else {
                //非变元符号  要么是基项,要么是含变元的函数项 
                handle = new TermCell();
                handle->TermCellSetProp(TermProp::TPIsShared); //设置项为共享项
                Sigcell* sig = Env::getSig();
                if (in->TestInpTok(TokenType::OpenBracket)) {//有括号说明有子项存在
                    //整数数字不能直接作为子项 如 f(1,2);
                    if ((id_type == FuncSymbType::FSIdentInt)&&((int32_t) sig->distinctProps & (int32_t) FPIsInteger)) {
                        in->AktTokenError("Number cannot have argument list(consider --free-numbers)", false);
                    }
                    //浮点数数字不能直接作为子项 如 f(1.2,2.3);
                    if ((id_type == FuncSymbType::FSIdentFloat)&&((int32_t) sig->distinctProps & (int32_t) FPIsFloat)) {
                        in->AktTokenError("Floating point number cannot have argument list(consider --free-numbers)", false);
                    }
                    //有理数数字不能直接作为子项 如 f(1.2,2.3);
                    if ((id_type == FuncSymbType::FSIdentRational) &&(sig->distinctProps & FPIsRational)) {
                        in->AktTokenError("Rational number cannot have argument list(consider --free-numbers)", false);
                    }
                    if ((id_type == FuncSymbType::FSIdentObject)&&((int32_t) sig->distinctProps & (int32_t) FPIsObject)) {
                        in->AktTokenError("Object cannot have argument list (consider --free-objects)", false);
                    }
                    //very important coding --读取每个子项
                    handle->arity = tb_term_parse_arglist(in, handle, isCheckSymbProp);


                } else {
                    handle->arity = 0;
                    handle->uMaxFuncLayer = 0;
                    handle->TermCellSetProp(TermProp::TPIsGround);
                }

                //查询 sigcell对象获取 读取符号的 fcode
                handle->fCode = TermCell::TermSigInsert(sig, idStr, handle->arity, false, id_type);
                if (!handle->fCode) {
                    errpos = "";
                    in->AktToken()->PosRep(errpos);
                    errpos += " used with arity " + to_string((long) handle->arity) + ", but registered with arity ";
                    errpos += to_string((long) (sig->fInfo[sig->SigFindFCode(idStr)]->arity));
                    cout << "SYNTAX_ERROR:" + errpos << endl;
                }
                //基项插入 全局基项bank 带变元的函数项 插入子句共享bank;
                if (0 == handle->uVarCount) {
                    handle->TermCellSetProp(TermProp::TPIsGround);
                }

                handle = TBTermTopInsert(handle);
            }

        }
    }
    return handle;
}

/*****************************************************************************
 * Make ref point to a term of the same structure as *ref, but with properties prop set. 
 * Properties do not work for variables! 
 ****************************************************************************/
void TermBank::TBRefSetProp(TermCell** ref, TermProp prop) {
    TermCell* term;

    assert(!(*ref)->IsVar());

    term = *ref;
    if (term->TermCellQueryProp(prop) || term->IsVar()) {
        return;
    }

    TermCell* newTerm = TermCell::TermTopCopy(term);
    newTerm->TermCellSetProp(prop);
    newTerm = TBTermTopInsert(newTerm);
    *ref = newTerm;
    /* Old term will be garbage-collected eventually */
}

/***************************************************************************** 
 * Return a reasonably initialized term cell for shared terms.
 ****************************************************************************/
TermCell* TermBank::DefaultSharedTermCellAlloc(void) {
    TermCell* handle = new TermCell();
    handle->TermCellSetProp(TermProp::TPIsShared);
    return handle;
}

/***************************************************************************** 
 * 插入一个Term到　termbank中
 * 输入参数 -- term:插入的Term指针; deref:决定term变元binding遍历的类型
 ****************************************************************************/
//TermCell* TermBank::TBInsert(TermCell* term, VarBank_p varbank, DerefType deref) {
//    assert(term);
//    term = TermCell::TermDeref(term, deref); /*根据deref类型,获取term的绑定项*/
//    /*非共享term！－This is an unshared　term cell at the　moment, */
//    TermCell* t = term->TermEquivCellAlloc(this); //得到term的副本copy
//
//    if (!t->IsVar()) {
//        // assert(SysDateIsCreationDate(t->rw_data.nf_date[0]));
//        // assert(SysDateIsCreationDate(t->rw_data.nf_date[1]));
//
//        for (int i = 0; i < t->arity; ++i) {
//            t->args[i] = TBInsert(t->args[i], varbank, deref);
//        }
//        t = TermTopInsert(t);
//    }
//    return t;
//} 

/*****************************************************************************
 * 插入项　t 到　TermBank 中．新项t的属性 0; 与TBInsert比较多了一个属性初始化为0;
 * As TBInsert, but will set all properties of the new term to 0 first.
 ****************************************************************************/
//TermCell* TermBank::TBInsertNoProps(TermCell* term, DerefType deref) {
//    assert(term);
//    term = TermCell::TermDeref(term, deref); /*根据deref类型,获取term的绑定项*/
//    /*非共享term！－This is an unshared　term cell at the　moment, */
//
//    TermCell* t = term->TermEquivCellAlloc(this); //得到term的副本copy
//    if (!t->IsVar()) {
//        t->properties = TermProp::TPIgnoreProps;
//        //assert(SysDateIsCreationDate(t->rw_data.nf_date[0]));
//        // assert(SysDateIsCreationDate(t->rw_data.nf_date[1]));
//        for (int i = 0; i < t->arity; ++i) {
//            t->args[i] = TBInsertNoProps(t->args[i], deref);
//        }
//        t = TermTopInsert(t);
//    }
//    return t;
//}

/***************************************************************************** 
 * Insert term into bank under the assumption that it it already is
 * in the bank (except possibly for variables appearing as
 * bindings). This allows us to just return term for ground terms.
 ****************************************************************************/
TermCell* TermBank::TBInsertOpt(TermCell* term, DerefType deref) {
    int i;

    assert(term);

    term = TermCell::TermDeref(term, deref);

    if (term->IsGround()) {
        return term;
    }
    Term_p t = term->TermEquivCellAlloc(this);
    /* This is an unshared term cell at the moment, */

    if (!t->IsVar()) {
        // assert(SysDateIsCreationDate(t->rw_data.nf_date[0]));
        // assert(SysDateIsCreationDate(t->rw_data.nf_date[1]));
        for (i = 0; i < t->arity; i++) {
            t->args[i] = TBInsertOpt(t->args[i], deref);
        }
        t = tb_termtop_insert(t);
    }
    return t;
}

/***************************************************************************** 
 * As TBInsertNoProps, but when old is encountered as a subterm(regardless of instantiation), 
 * replace it with uninstantiated repl (which _must_ be in bank).
 * 输入参数: term 插入的项; deref 遍历绑定的类型; old 已经存在的项 repl 返回的项目
 * 与TBInsertNoProps 相比较,多了一个判定步骤.若插入的元素==old.则返回 repl.
 * old 与 repl的关系 一个是实例化的 一个为非实例化的 例如 old=f(a)  repl=f(x)
 ****************************************************************************/
//TermCell* TermBank::TBInsertRepl(TermCell* term, DerefType deref, TermCell* old, TermCell* repl) {
//    assert(term);
//    if (term == old) {
//        assert(TBFind(repl));
//        return repl;
//    }
//    TermCell::TermDeref(term, deref); /*根据deref类型,获取term的绑定项*/
//    /*非共享term！－This is an unshared　term cell at the　moment, */
//    TermCell* t = term->TermEquivCellAlloc(this); //得到term的副本copy
//    if (!t->IsVar()) {
//        t->properties = TermProp::TPIgnoreProps;
//        //assert(SysDateIsCreationDate(t->rw_data.nf_date[0]));
//        //assert(SysDateIsCreationDate(t->rw_data.nf_date[1]));
//        for (int i = 0; i < t->arity; ++i) {
//            t->args[i] = TBInsertRepl(t->args[i], deref, old, repl);
//        }
//        t = TermTopInsert(t);
//    }
//    return t;
//}

/*****************************************************************************
 * 该方法用于rewrite 
 * Insert a term into the termbank under the assumption that it is 
 * a right side of a rule (or equation) composed of terms from bank,
 * and (possibly) instantiated with terms from bank - i.e. don't insert terms 
 * that are bound to variables and ground terms, but assume 
 * that they are in the term bank. Properties in newly created nodes are deleted.   
 * 
 ****************************************************************************/
//TermCell* TermBank::TBInsertInstantiated(TermCell* term) {
//    assert(term);
//    /*若项为基项,则不插入直接返回该项 (前提条件-一定在termbank中存在)*/
//    if (term->TermCellQueryProp(TermProp::TPIsGround)) {
//        assert(TBFind(term));
//        return term;
//    }
//    /*若存在绑定元素,则不插入直接返回该绑定项 */
//    if (term->binding) {
//        assert(TBFind(term->binding));
//        return term->binding;
//    }
//    /*非共享term！－This is an unshared　term cell at the　moment, */
//    TermCell* t = term->TermEquivCellAlloc(this); //得到term的副本copy 
//    if (!t->IsVar()) {
//        t->properties = TermProp::TPIgnoreProps;
//        //assert(SysDateIsCreationDate(t->rw_data.nf_date[0]));
//        //assert(SysDateIsCreationDate(t->rw_data.nf_date[1]));
//        for (int i = 0; i < t->arity; ++i) {
//            t->args[i] = TBInsertInstantiated(t->args[i]);
//        }
//        t = TermTopInsert(t);
//    }
//    return t;
//}

/*****************************************************************************
 * 插入项　t 到　TermBank 中．项t中的每个子项均已经存在于termBank中
 * Will reuse or destroy the top cell!
 ****************************************************************************/
//TermCell* TermBank::TermTopInsert(TermCell* t) {
//    assert(t);
//    assert(!t->IsVar()); //确保插入的项不能是变元
//
//    TermCell* newTerm = termStore->TermCellStoreInsert(t); //插入新项到 termStore中
//
//    if (newTerm) /* TermCell node already existed, just add properties */ {
//        newTerm->properties = (TermProp) ((int32_t) newTerm->properties | (int32_t) t->properties)/*& bank->prop_mask*/;
//        DelPtr(t);
//        return newTerm;
//    } else {
//        t->entryNo = ++(inCount);
//        t->TermCellAssignProp(TermProp::TPGarbageFlag, garbageState); //设置属性
//        t->weight = DEFAULT_FWEIGHT;
//
//        for (int i = 0; i < t->arity; ++i) {
//            assert(t->args[i]->IsShared() || t->args[i]->IsVar());
//
//            t->weight += t->args[i]->weight;
//            t->uVarCount += t->args[i]->uVarCount;
//            if (!t->args[i]->TermCellQueryProp(TermProp::TPIsGround)) {
//                t->TermCellDelProp(TermProp::TPIsGround);
//            }
//        }
//        if (0 == t->uVarCount) { /* Groundness may change below */
//            t->TermCellSetProp(TermProp::TPIsGround);
//        }
//        assert(t->TermStandardWeight() == t->TermWeight(DEFAULT_VWEIGHT, DEFAULT_FWEIGHT));
//        assert((t->IsGround() == 0) == (t->TBTermIsGround() == 0));
//    }
//    return t;
//}

/*****************************************************************************
 * 只查找基项 Find a term in the term cell bank and return it. 
 ****************************************************************************/
TermCell* TermBank::TBFind(TermCell* term) {
    assert(term->IsGround());
    //    if (term->IsVar()) {
    //        return vars->FindByFCode(term->fCode);
    //    }
    return termStore->TermCellStoreFind(term);
}

/*****************************************************************************
 * 返回基项的总数--Return the number of term nodes (variables and non-variables) in the term bank. 
 ****************************************************************************/
long TermBank::TBTermNodes() {
    assert(termStore->entries == termStore->TermCellStoreCountNodes());
    //变元节点数+非变元节点数
    return termStore->entries; //+ vars->vctFCodes.size();

}

///*****************************************************************************
// * 根据参数source，若为变元则在varBank中查找该变元项，否则拷贝该项；返回非共享项指针，
// * 注：非共享项，指变元项和不存储到TermBank中的项． 
// ****************************************************************************/
//
//TermCell* TermBank::TermEquivCellAlloc(TermCell* source, VarBank_p vars) {
//    TermCell* handle;
//
//    if (source->IsVar()) //变元项　
//    {
//        handle = vars->Insert(source->fCode);
//    } else {
//        handle = source->TermTopCopy();
//    }
//    return handle;
//}

/***************************************************************************** 
 * Print the DAG in the order of ascending entry_no. 
 * E做法:先将所有项,放入一颗排序树中(伸展树),再进行树遍历进行输出
 * 改进做法:自己构建一个排序二叉树,
 ****************************************************************************/
void TermBank::TBPrintBankInOrder(FILE* out) {

    SplayTree<NumTreeCell> splayTree;
    for (long i = 0; i < termStore->TERM_STORE_HASH_SIZE; i++) {
        TermTree::TermTreeTraverseInit(termStore->store[i], splayTree);
    }

    tb_print_dag(out, splayTree.GetRoot());
    splayTree.Destroy();

}

/*****************************************************************************
 * Print a term bank term. Introduce abbreviations for all subterms
 * encountered. Subterms with TPOutputFlag are not 
 * printed, but are assumed to be known. Does _not_ follow bindings
 * (they are temporary and as such make little sense in the term bank context)  
 ****************************************************************************/
void TermBank::TBPrintTermCompact(FILE* out, TermCell* term) {
    int i;

    if (term->TermCellQueryProp(TermProp::TPOutputFlag)) {
        fprintf(out, "*%ld", term->entryNo);
    } else {
        if (term->IsVar()) {
            term->VarPrint(out);
        } else {
            fprintf(out, "*%ld:", term->entryNo);
            term->TermCellSetProp(TermProp::TPOutputFlag);
            string name = "";
            Env::getSig()->SigFindName(term->fCode, name);
            fputs(name.c_str(), out);
            if (!term->IsConst()) {
                fputc('(', out);
                assert(term->args && (term->arity > 0));
                TBPrintTermCompact(out, term->args[0]);
                for (i = 1; i < term->arity; i++) {
                    fputc(',', out);
                    TBPrintTermCompact(out, term->args[i]);
                }
                fputc(')', out);
            }
        }
    }
}

/***************************************************************************** 
 * Print a term from a term bank either in compact form (with abbreviations) or as a conventional term.
 ****************************************************************************/
void TermBank::TBPrintTerm(FILE* out, Term_p term, bool fullterms, DerefType deref) {
    if (fullterms) {
        //TBPrintTermFull(out, term);
        term->TermPrint(out, deref);
        //term->PrintDerefAlways(out);
        //term->TermPrint(out, DerefType::DEREF_NEVER);
        //fprintf(out,"\n");
    } else {
        TBPrintTermCompact(out, term);
    }

}
//   

/***************************************************************************** 
 * Create a copy of (uninstantiated) term with disjoint variables. 
 * This assumes that all variables in term are odd or even, 
 * the returned copy will have variable ids shifted by -1.
 ****************************************************************************/
TermCell* TermBank::TBInsertDisjoint(TermCell* term) {
    int i;
    Term_p t;
    assert(term);
    if (term->IsGround()) {
        return term;
    }

    if (term->IsVar()) {
        t = shareVars->VarBankFCodeAssertAlloc(term->fCode + 1);
    } else {
        t = term->TermEquivCellAlloc(this); /* This is an unshared
                                                   term cell at the
                                                   moment, */
        //  assert(SysDateIsCreationDate(t->rw_data.nf_date[0]));
        // assert(SysDateIsCreationDate(t->rw_data.nf_date[1]));

        for (i = 0; i < t->arity; i++) {
            t->args[i] = TBInsertDisjoint(t->args[i]);
        }
        t = TBTermTopInsert(t);
    }
    return t;
}

/***************************************************************************** 
 * If bank->min_term exists, return it. Otherwise create and return it. 
 ***************************************************************************
Term_p TermBank::TBCreateMinTerm(FunCode min_const) {
    if (!minTerm) {
        Term_p t = TermCell::TermConstCellAlloc(min_const);
        minTerm = TBInsert(t, DerefType::DEREF_NEVER);
        TermCell::TermFree(t);

    }
    assert(minTerm->fCode == min_const);
    return minTerm;
}*/