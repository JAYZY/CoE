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
#include "FileOp.h"
#include "TermCell.h"
#include "SysDate.h"
bool TermBank::TBPrintInternalInfo = false;
bool TermBank::TBPrintDetails = false;

/*---------------------------------------------------------------------*/
/*                    Constructed Function                             */

/*---------------------------------------------------------------------*/
TermBank::TermBank()//Sig_p sigValue) : sig(sigValue) {
    //初始化TermBank的时候,插入两个特殊的项 $True $False

    //assert(sig);
    inCount = 0;
    rewriteSteps = 0; //replace次数 
    extIndex.reserve(100000); //注意，只是预留10W空间，并没有初始化，(初始化可以考虑用resize)
    garbageState = TPIgnoreProps;

    //初始化 varBank
    vars = new VarBank();
    //初始化 TermCellStore
    termStore = TermCellStore();

    //创建一个特殊TrueTerm 
    TermCell* tmpTerm = new TermCell((FunCode) DerefType::TRUECODE, 0);
    tmpTerm->TermCellSetProp(TPPredPos); //默认谓词符号
    trueTerm = TBInsert(tmpTerm, DEREF_NEVER);
    TermCell::TermFree(tmpTerm); //TermFree(term);

    //创建一个特殊FalseTerm 
    tmpTerm = new TermCell((FunCode) DerefType::FLASECODE);
    tmpTerm->TermCellSetProp(TPPredPos);
    falseTerm = TBInsert(tmpTerm, DEREF_NEVER);

    TermCell::TermFree(tmpTerm); //TermFree(term);
    minTerm = NULL;
    //freevarsets = NULL;
}

TermBank::TermBank(const TermBank& orig) {
}

TermBank::~TermBank() {
    assert(!sig);

    /* printf("TBFree(): %ld\n", TermCellStoreNodes(&(junk->term_store)));*/
    termStore.TermCellStoreExit();
    DelPtr(vars);
    extIndex.clear();
    vector<TermCell*>().swap(extIndex);
    assert(freeVarSets.IsEmpty());
    //TBCellFree(junk);
}


/*---------------------------------------------------------------------*/
/*                  Member Function--private                           */
/*---------------------------------------------------------------------*/

/***************************************************************************** 
 * 根据插入顺序输出 -- Print the terms as a dag in the order of insertion.
 ****************************************************************************/
void TermBank::tb_print_dag(FILE *out, NumTree_p spNode, Sig_p sig) {
    //中序遍历 伸展树 注意.spNode是节点
    if (spNode == nullptr) {
        return;
    }
    tb_print_dag(out, spNode->lson, sig);

    TermCell* term = (TermCell*) spNode->val1.p_val;

    fprintf(out, "*%ld : ", term->entryNo);

    if (term->IsVar()) {
        term->VarPrint(out);
    } else {
        string sigName;
        sig->SigFindName(term->fCode, sigName);
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
        term->TermPrint(out, sig, DEREF_NEVER);
    }
    if (TermBank::TBPrintInternalInfo) {
        fprintf(out, "\t/*  Properties: %10d */", term->properties);
    }
    fprintf(out, "\n");
    tb_print_dag(out, spNode->rson, sig);
}

/***************************************************************************** 
 * 转换一个子项 --Parse a subterm, i.e. a term which cannot start with a predicate symbol.
 ****************************************************************************/
TermCell* TermBank::tb_subterm_parse(ScannerCell* in) {
    TermCell* res = TBTermParseReal(in, true);

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
int TermBank::tb_term_parse_arglist(ScannerCell* in, TermCell*** argAnchor, bool isCheckSymbProp) {
    in->AcceptInpTok(OpenBracket);
    if (in->TestInpTok(CloseBracket)) {
        in->NextToken();
        *argAnchor = nullptr;
        return 0;
    }


    vector<TermCell*> vectTerm;
    vectTerm.reserve(10);
    //    TermCell* tmp = isCheckSymbProp ? tb_subterm_parse(in) : TBTermParseReal(in, false);

    vectTerm.push_back(isCheckSymbProp ? tb_subterm_parse(in) : TBTermParseReal(in, false));
    int arity = 0;
    arity++;
    while (in->TestInpTok(Comma)) {
        in->NextToken();
        vectTerm.push_back(isCheckSymbProp ? tb_subterm_parse(in) : TBTermParseReal(in, false));
        arity++;
    }
    in->AcceptInpTok(CloseBracket);
    assert(arity == vectTerm.size());
    *argAnchor = new TermCell*[arity]; // TermArgArrayAlloc(arity);
    for (int i = 0; i < arity; ++i) {
        (*argAnchor)[i] = vectTerm[i];
    }
    vector<TermCell*>().swap(vectTerm);
    //SizeFree(handle, size * sizeof (TermCell*));
    return arity;
}

/***************************************************************************** 
 * Insert a term into the term bank for which the subterms are 
 * already in the term bank. Will reuse or destroy the top cell!
 * 插入一个项到termbank中.但该项的子项已经存在,重用或删除 top cell! 
 ****************************************************************************/
TermCell* TermBank::TBTermTopInsert(TermCell* t) {
    assert(t);
    assert(!t->IsVar());
    TermCell* newTerm = termStore.TermCellStoreInsert(t);
    if (newTerm) /* TermCell node already existed, just add properties */ {
        newTerm->properties = (TermProperties) (newTerm->properties | t->properties)/*& bank->prop_mask*/;
        DelPtr(t);
        return newTerm;
    } else {
        t->entryNo = ++(inCount);
        t->TermCellAssignProp(TPGarbageFlag, garbageState);
        t->TermCellSetProp(TPIsShared | TPIsGround); /* Groundness may change below */
        t->weight = DEFAULT_FWEIGHT;
        for (int i = 0; i < t->arity; ++i) {
            assert(t->args[i]->IsShared() || t->args[i]->IsVar());
            t->weight += t->args[i]->weight;
            if (!t->args[i]->TermCellQueryProp(TPIsGround)) {
                t->TermCellDelProp(TPIsGround);
            }
        }
        assert(t->TermStandardWeight() == t->TermWeight(DEFAULT_VWEIGHT, DEFAULT_FWEIGHT));
        assert((t->TermIsGround() == false) == (t->TBTermIsGround() == false));
    }
    return t;
}

/***************************************************************************** 
 * Parse a LOP list into an (shared) internal $cons list.
 ****************************************************************************/
TermCell* TermBank::tb_parse_cons_list(ScannerCell* in, bool isCheckSymbProp) {
    assert(Sigcell::SigSupportLists);
    in->AcceptInpTok(OpenSquare);
    TermCell* handle = DefaultSharedTermCellAlloc();
    TermCell* current = handle;
    vector<TermCell*> st;
    if (!in->TestInpTok(CloseSquare)) {
        current->fCode = SIG_CONS_CODE;
        current->arity = 2;
        current->args = new TermCell*[2];

        current->args[0] = TBTermParseReal(in, isCheckSymbProp);
        current->args[1] = DefaultSharedTermCellAlloc();
        current = current->args[1];
        st.push_back(current);

        while (in->TestInpTok(Comma)) {
            in->NextToken();
            current->fCode = SIG_CONS_CODE;
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
    in->AcceptInpTok(CloseSquare);
    current->fCode = SIG_NIL_CODE;

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

    TermCell* newTerm = termStore.TermCellStoreInsert(t);

    if (newTerm) /* Term node already existed, just add properties */ {
        newTerm->properties = (TermProperties) (newTerm->properties | t->properties)/*& bank->prop_mask*/;
        DelPtr(t);
        return newTerm;
    } else {

        t->entryNo = ++(inCount);
        t->TermCellAssignProp(TPGarbageFlag, garbageState);
        t->TermCellSetProp(TPIsShared | TPIsGround); /* Groundness may
                                                  * change below */

        t->weight = DEFAULT_FWEIGHT;
        for (int i = 0; i < t->arity; ++i) {
            assert(t->args[i]->IsShared() || t->args[i]->IsVar());
            t->weight += t->args[i]->weight;
            if (!t->args[i]->TermCellQueryProp(TPIsGround)) {
                t->TermCellDelProp(TPIsGround);
            }
        }
        assert(t->TermStandardWeight() == t->TermWeight(DEFAULT_VWEIGHT, DEFAULT_FWEIGHT));
        assert((t->TermIsGround() == 0) == (t->TBTermIsGround() == 0));
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
TermCell* TermBank::TBTermParseReal(ScannerCell* in, bool isCheckSymbProp) {
    FuncSymbType idType;
    Token_p token = in->AktToken();
    TermCell* handle = nullptr;
    string errpos;
    /*缩写测试- Test for abbreviation */
    if (in->TestInpTok(Mult)) {
        TermProperties properties = TPIgnoreProps;
        in->NextToken();
        long abbrev = in->ParseInt();
        if (in->TestInpTok(Colon | Slash)) { /* 如果已经存在,则报错 -- This _defines_ the abbrev! */
            if (extIndex[abbrev] != nullptr)//[abbrev]  PDArrayElementP(bank->ext_index, abbrev))
            {
                /* Error: Abbreviation defined twice */
                errpos = "";
                in->AktToken()->PosRep(errpos);
                errpos += "Abbreviation *" + itos(abbrev) + " already defined";
                cout << "SYNTAX_ERROR:" + errpos << endl;
            }
            if (in->TestInpTok(Slash)) {
                in->NextToken();
                properties = (TermProperties) in->ParseInt();
            }
            in->NextToken();
            handle = TBTermParseReal(in, isCheckSymbProp); /* Elegant, aint it? */
            if (properties) {
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
                errpos += "Abbreviation *" + itos(abbrev) + " undefined";
                cout << "SYNTAX_ERROR:" + errpos << endl;
            }
        }
    } else {
        /* 正常term Normal term stuff, bloated because of the nonsensical SETHEO
           syntax */
        if (Sigcell::SigSupportLists && in->TestInpTok(OpenSquare)) {
            handle = tb_parse_cons_list(in, isCheckSymbProp);
        } else {
            string idStr;
            FuncSymbType id_type;
            if ((id_type = TermCell::TermParseOperator(in, idStr)) == FSIdentVar) {
                //若为变元符号
                handle = vars->Insert(idStr);
            } else {
                handle = DefaultSharedTermCellAlloc();

                if (in->TestInpTok(OpenBracket)) {
                    if ((id_type == FSIdentInt)&&(sig->distinctProps & FPIsInteger)) {
                        in->AktTokenError("Number cannot have argument list(consider --free-numbers)", false);
                    }
                    if ((id_type == FSIdentFloat)&&(sig->distinctProps & FPIsFloat)) {
                        in->AktTokenError("Floating point number cannot have argument list(consider --free-numbers)", false);
                    }
                    if ((id_type == FSIdentRational)
                            &&(sig->distinctProps & FPIsRational)) {
                        in->AktTokenError("Rational number cannot have argument list(consider --free-numbers)", false);
                    }
                    if ((id_type == FSIdentObject)&&(sig->distinctProps & FPIsObject)) {
                        in->AktTokenError("Object cannot have argument list (consider --free-objects)", false);
                    }
                    handle->arity = tb_term_parse_arglist(in, &(handle->args), isCheckSymbProp);
                } else {
                    handle->arity = 0;
                }
                handle->fCode = TermCell::TermSigInsert(sig, idStr, handle->arity, false, id_type);
                if (!handle->fCode) {
                    errpos = "";
                    in->AktToken()->PosRep(errpos);
                    errpos += " used with arity " + itos((long) handle->arity) + ", but registered with arity ";
                    errpos += itos((long) (sig->fInfo[sig->SigFindFCode(idStr)]->arity));
                    cout << "SYNTAX_ERROR:" + errpos << endl;
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
void TermBank::TBRefSetProp(TermCell** ref, TermProperties prop) {
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
    handle->TermCellSetProp(TPIsShared);
    return handle;
}

/***************************************************************************** 
 * 插入一个Term到　termbank中
 * 输入参数 -- term:插入的Term指针; deref:决定term变元binding遍历的类型
 ****************************************************************************/
TermCell* TermBank::TBInsert(TermCell* term, int deref) {
    assert(term);
    term = TermCell::TermDeref(term, deref); /*根据deref类型,获取term的绑定项*/
    /*非共享term！－This is an unshared　term cell at the　moment, */
    TermCell* t = term->TermEquivCellAlloc(vars); //得到term的副本copy

    if (!t->IsVar()) {
        assert(SysDateIsCreationDate(t->rw_data.nf_date[0]));
        assert(SysDateIsCreationDate(t->rw_data.nf_date[1]));

        for (int i = 0; i < t->arity; ++i) {
            t->args[i] = TBInsert(t->args[i], deref);
        }
        t = TermTopInsert(t);
    }
    return t;
}

/*****************************************************************************
 * 插入项　t 到　TermBank 中．新项t的属性 0; 与TBInsert比较多了一个属性初始化为0;
 * As TBInsert, but will set all properties of the new term to 0 first.
 ****************************************************************************/
TermCell* TermBank::TBInsertNoProps(TermCell* term, int deref) {
    assert(term);
    term = TermCell::TermDeref(term, deref); /*根据deref类型,获取term的绑定项*/
    /*非共享term！－This is an unshared　term cell at the　moment, */

    TermCell* t = term->TermEquivCellAlloc(vars); //得到term的副本copy
    if (!t->IsVar()) {
        t->properties = TPIgnoreProps;
        assert(SysDateIsCreationDate(t->rw_data.nf_date[0]));
        assert(SysDateIsCreationDate(t->rw_data.nf_date[1]));
        for (int i = 0; i < t->arity; ++i) {
            t->args[i] = TBInsertNoProps(t->args[i], deref);
        }
        t = TermTopInsert(t);
    }
    return t;
}

/***************************************************************************** 
 * Insert term into bank under the assumption that it it already is
 * in the bank (except possibly for variables appearing as
 * bindings). This allows us to just return term for ground terms.
 ****************************************************************************/
TermCell* TermBank::TBInsertOpt(TermCell* term, DerefType deref) {
    int i;

    assert(term);

    term = TermCell::TermDeref(term, deref);

    if (term->TermIsGround()) {
        return term;
    }
    Term_p t = term->TermEquivCellAlloc(vars);
    /* This is an unshared term cell at the moment, */

    if (!t->IsVar()) {
        assert(SysDateIsCreationDate(t->rw_data.nf_date[0]));
        assert(SysDateIsCreationDate(t->rw_data.nf_date[1]));
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
TermCell* TermBank::TBInsertRepl(TermCell* term, int deref, TermCell* old, TermCell* repl) {
    assert(term);
    if (term == old) {
        assert(TBFind(repl));
        return repl;
    }
    TermCell::TermDeref(term, deref); /*根据deref类型,获取term的绑定项*/
    /*非共享term！－This is an unshared　term cell at the　moment, */
    TermCell* t = term->TermEquivCellAlloc(vars); //得到term的副本copy
    if (!t->IsVar()) {
        t->properties = TPIgnoreProps;
        //assert(SysDateIsCreationDate(t->rw_data.nf_date[0]));
        //assert(SysDateIsCreationDate(t->rw_data.nf_date[1]));
        for (int i = 0; i < t->arity; ++i) {
            t->args[i] = TBInsertRepl(t->args[i], deref, old, repl);
        }
        t = TermTopInsert(t);
    }
    return t;
}

/*****************************************************************************
 * 该方法用于rewrite 
 * Insert a term into the termbank under the assumption that it is 
 * a right side of a rule (or equation) composed of terms from bank,
 * and (possibly) instantiated with terms from bank - i.e. don't insert terms 
 * that are bound to variables and ground terms, but assume 
 * that they are in the term bank. Properties in newly created nodes are deleted.   
 * 
 ****************************************************************************/
TermCell* TermBank::TBInsertInstantiated(TermCell* term) {
    assert(term);
    /*若项为基项,则不插入直接返回该项 (前提条件-一定在termbank中存在)*/
    if (term->TermCellQueryProp(TPIsGround)) {
        assert(TBFind(term));
        return term;
    }
    /*若存在绑定元素,则不插入直接返回该绑定项 */
    if (term->binding) {
        assert(TBFind(term->binding));
        return term->binding;
    }
    /*非共享term！－This is an unshared　term cell at the　moment, */
    TermCell* t = term->TermEquivCellAlloc(vars); //得到term的副本copy 
    if (!t->IsVar()) {
        t->properties = TPIgnoreProps;
        //assert(SysDateIsCreationDate(t->rw_data.nf_date[0]));
        //assert(SysDateIsCreationDate(t->rw_data.nf_date[1]));
        for (int i = 0; i < t->arity; ++i) {
            t->args[i] = TBInsertInstantiated(t->args[i]);
        }
        t = TermTopInsert(t);
    }
    return t;
}

/*****************************************************************************
 * 插入项　t 到　TermBank 中．项t中的每个子项均已经存在于termBank中
 * Will reuse or destroy the top cell!
 ****************************************************************************/
TermCell* TermBank::TermTopInsert(TermCell* t) {
    assert(t);
    assert(!t->IsVar()); //确保插入的项不能是变元

    TermCell* newTerm = termStore.TermCellStoreInsert(t); //插入新项到 termStore中

    if (newTerm) /* TermCell node already existed, just add properties */ {
        newTerm->properties = (TermProperties) (newTerm->properties | t->properties)/*& bank->prop_mask*/;
        DelPtr(t);
        return newTerm;
    } else {
        t->entryNo = ++(inCount);
        t->TermCellAssignProp(TPGarbageFlag, garbageState); //设置属性

        /* Groundness may change below */
        t->TermCellSetProp(TPIsShared | TPIsGround);

        t->weight = DEFAULT_FWEIGHT;

        for (int i = 0; i < t->arity; ++i) {
            assert(t->args[i]->IsShared() || t->args[i]->IsVar());

            t->weight += t->args[i]->weight;
            if (!t->args[i]->TermCellQueryProp(TPIsGround)) {
                t->TermCellDelProp(TPIsGround);
            }
        }
        assert(t->TermStandardWeight() == t->TermWeight(DEFAULT_VWEIGHT, DEFAULT_FWEIGHT));
        assert((t->TermIsGround() == 0) == (t->TBTermIsGround() == 0));
    }
    return t;
}

/*****************************************************************************
 * Find a term in the term cell bank and return it. 
 ****************************************************************************/
TermCell* TermBank::TBFind(TermCell* term) {
    if (term->IsVar()) {
        return vars->FindByFCode(term->fCode);
    }
    return termStore.TermCellStoreFind(term);
}

/*****************************************************************************
 * 返回项的总数--Return the number of term nodes (variables and non-variables) in the term bank. 
 ****************************************************************************/
long TermBank::TBTermNodes() {
    assert(termStore.entries == termStore.TermCellStoreCountNodes());
    //变元节点数+非变元节点数
    return termStore.entries + vars->vctFCodes.size();

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
    for (long i = 0; i < TERM_STORE_HASH_SIZE; i++) {
        TermTree::TermTreeTraverseInit(termStore.store[i], splayTree);
    }

    tb_print_dag(out, splayTree.GetRoot(), sig);
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

    if (term->TermCellQueryProp(TPOutputFlag)) {
        fprintf(out, "*%ld", term->entryNo);
    } else {
        if (term->IsVar()) {
            term->VarPrint(out);
        } else {
            fprintf(out, "*%ld:", term->entryNo);
            term->TermCellSetProp(TPOutputFlag);
            string name = "";
            sig->SigFindName(term->fCode, name);
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
void TermBank::TBPrintTerm(FILE* out, Term_p term, bool fullterms) {
    if (fullterms) {
        //TBPrintTermFull(out, term);
        term->TermPrint(out, sig, DEREF_NEVER);
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


    if (term->TermIsGround()) {
        return term;
    }

    if (term->IsVar()) {
        t = vars->VarBankFCodeAssertAlloc(term->fCode + 1);
    } else {
        t = term->TermEquivCellAlloc(vars); /* This is an unshared
                                                   term cell at the
                                                   moment, */
        assert(SysDateIsCreationDate(t->rw_data.nf_date[0]));
        assert(SysDateIsCreationDate(t->rw_data.nf_date[1]));

        for (i = 0; i < t->arity; i++) {
            t->args[i] = TBInsertDisjoint(t->args[i]);
        }
        t = TBTermTopInsert(t);
    }
    return t;
}

/***************************************************************************** 
 * If bank->min_term exists, return it. Otherwise create and return it. 
 ****************************************************************************/
Term_p TermBank::TBCreateMinTerm(FunCode min_const) {
    if (!minTerm) {
        Term_p t = TermCell::TermConstCellAlloc(min_const);
        minTerm = TBInsert(t, DEREF_NEVER);
        TermCell::TermFree(t);

    }
    assert(minTerm->fCode == min_const);
    return minTerm;
}