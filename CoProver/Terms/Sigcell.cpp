/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Sigcell.cpp
 * Author: cf 
 * 
 * Created on 2017年3月6日, 上午10:35
 */



#include "Sigcell.h"
#include "INOUT/FileOp.h"
#include "TermCell.h"   
bool Sigcell::SigSupportLists = false;

/***************************************************************************** 
 ****************************************************************************/
Sigcell::Sigcell() {

    alphaRanksValid = false;
    fInfo.reserve(DEFAULT_SIGNATURE_SIZE); //扩大    
    fInfo.push_back(nullptr); // 第一个元素为NULL. 

    /* 为fIndex插入１StrTreeCell节点*/

    StrTreeCell* StrTreeNode = new StrTreeCell();

    fIndex.Insert(StrTreeNode);
    acAxioms.reserve(8);

    SigInsertId("$true", 0, true); //插入一个元素到signature数组中,并返回数组下标作为元素的fCode
    assert(SigFindFCode("$true") == (FunCode)DerefType::TRUECODE);

    SigSetFuncProp((FunCode)DerefType::TRUECODE, (int)FPPredSymbol | (int)FPInterpreted);
    SigInsertId("$false", 0, true); //插入一个元素到signature数组中,并返回数组下标作为元素的fCode

    assert(SigFindFCode("$false") == (FunCode)DerefType::FLASECODE);
    SigSetFuncProp( (FunCode)DerefType::FLASECODE, (int)FPPredSymbol | (int)FPInterpreted);

    if (SigSupportLists) {
        SigInsertId("$nil", 0, true);
        assert(SigFindFCode("$nil") == (FunCode)DerefType::NILCODE);
        SigInsertId("$cons", 2, true);
        assert(SigFindFCode("$cons") == (FunCode)DerefType::CONSCODE);
    }
    internalSymbols = fCount(); //表示最大的可用下标
    ornCodes.reserve(DEFAULT_SIGNATURE_SIZE);
    eqnCode = 0;
    neqnCode = 0;
    cnilCode = 0;
    orCode = 0;
    notCode = 0;
    qexCode = 0;
    qallCode = 0;
    andCode = 0;
    orCode = 0;
    implCode = 0;
    equivCode = 0;
    nandCode = 0;
    norCode = 0;
    bimplCode = 0;
    xorCode = 0;
    answerCode = 0;
    skolemCount = 0;
    newpredCount = 0;
    distinctProps = FPDistinctProp;
    SigInsertInternalCodes();
}

Sigcell::Sigcell(const Sigcell& orig) {
}

Sigcell::~Sigcell() {
    if (!fInfo.empty()) {
        //        for(int i=0;i<fCount();++i)
        //        {
        //            delete fInfo[i];
        //        }        
        vector<Func_p>().swap(fInfo);
    }
    if (!acAxioms.empty()) {
        vector<Clause*>().swap(acAxioms);
    }
    if (!ornCodes.empty()) {
        vector<IntOrP>().swap(ornCodes);
    }

}

/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

/*****************************************************************************
 * Print a single operator
 ****************************************************************************/
void Sigcell::SigPrintOperator(FILE* out, FunCode op, bool comments) {
    Func_p fp = fInfo[op];
    if (comments) {
        fprintf(out, "   %-13s : %2d    #  %2ld %2d \n", fInfo[op]->name.c_str(), fInfo[op]->arity, op,
                fp->properties);
    } else {
        fprintf(out, "   %-13s : %2d\n", fp->name.c_str(), fp->arity);
    }
}

/*****************************************************************************
 * Print the signature in external representation,with comments showing internal structure. 
 ****************************************************************************/
void Sigcell::SigPrint(FILE* out) {
    FunCode i;
    fprintf(out, "# Signature (%2ld symbols out of %2ld allocated):\n", fCount(), fInfo.capacity());
    fprintf(out, "#     -Symbol-    -Arity- -Encoding-\n");

    for (i = 1; i <= fCount(); ++i) {
        SigPrintOperator(out, i, true);
    }
}

/*****************************************************************************
 * Print the external special symbols from sig. 
 ****************************************************************************/
void Sigcell::SigPrintSpecial(FILE* out) {
    FunCode i;
    fputs("# Special symbols:\n", out);
    for (i = 1; i <= fCount(); ++i) {
        if (SigIsSpecial(i)) {
            SigPrintOperator(out, i, true);
        }
    }
}

/***************************************************************************** .
 * print its status as a comment.For each function symbol which is A, C, or AC,
 ****************************************************************************/
void Sigcell::SigPrintACStatus(FILE* out) {
    FunCode i;
    for (i = 1; i < fCount(); ++i) {
        if (SigQueryFuncProp(i, FPIsAC)) {
            fprintf(out, "# %s is AC\n", fInfo[i]->name.c_str());
            continue;
        }
        if (SigQueryFuncProp(i, FPAssociative)) {
            fprintf(out, "# %s is associative\n", fInfo[i]->name.c_str());
            continue;
        }
        if (SigQueryFuncProp(i, FPCommutative)) {
            fprintf(out, "# %s is commutative\n", fInfo[i]->name.c_str());
            continue;
        }
    }
}

/*****************************************************************************
 * For all symbols in sig compute the alpha-rank of the symbol.
 * 计算所有符号元素的 alpha-Rank.
 ****************************************************************************/
void Sigcell::SigComputeAlphaRanks() {
    long count = 0;
    /*zj理解: 遍历伸展树,根据名字的排序,设置每个元素的alpha-rank
     * 因此需要从小到大遍历 伸展树. E源码是先将树节点遍历到一个栈中,再出栈操作
     * stack = StrTreeTraverseInit(sig->fIndex); 将树节点遍历到一个栈中
     *  while((handle = StrTreeTraverseNext(stack))); 出栈操作 */

    /*改进算法--本质就是一个非递归的树 中序遍历*/
    vector<StrTree_p> st;
    StrTree_p ptr = fIndex.GetRoot();
    while (ptr != NULL || !st.empty()) {
        while (ptr != NULL) {
            st.push_back(ptr);
            ptr = ptr->lson;
        }
        if (!st.empty()) {
            ptr = st.back();
            fInfo[ptr->val1.i_val]->alphaRank = count++;
            st.pop_back();
            ptr = ptr->rson;
        }
    }
    //st.clear();
    vector<StrTree_p>().swap(st);
    alphaRanksValid = true;
}

/***************************************************************************** 
 * Put all the FOF operators as function symbols into sig. Sig should be empty,
 * so that sig->internal symbols can be properly initialized.
 * Note that this will be used for plain term signatures.
 * It reuses some equivalent fields of signatures used for patterns,but morphs the f_codes into internal symbols. 
 * 将所有FOF操作放入 sig中,前提条件:sig中为空.
 ****************************************************************************/
void Sigcell::SigInsertInternalCodes() {
    assert((SigSupportLists && internalSymbols == (FunCode)DerefType::CONSCODE ) ||
            (!SigSupportLists && internalSymbols ==  (FunCode)DerefType::FLASECODE));

    eqnCode = SigInsertId("$eq", 2, true);
    SigSetPredicate(eqnCode, true);
    neqnCode = SigInsertId("$neq", 2, true);
    SigSetPredicate(neqnCode, true);

    qexCode = SigInsertId("$qex", 2, true);
    qallCode = SigInsertId("$qall", 2, true);

    notCode = SigInsertFOFOp("$not", 1);
    andCode = SigInsertFOFOp("$and", 2);
    orCode = SigInsertFOFOp("$or", 2);
    implCode = SigInsertFOFOp("$impl", 2);
    equivCode = SigInsertFOFOp("$equiv", 2);
    nandCode = SigInsertFOFOp("$nand", 2);
    norCode = SigInsertFOFOp("$nor", 2);
    bimplCode = SigInsertFOFOp("$bimpl", 2);
    xorCode = SigInsertFOFOp("$xor", 2);
    xorCode = SigInsertFOFOp("$xor", 2);
    answerCode = SigInsertId("$answer", 1, true);

    SigSetPredicate(answerCode, true);
    SigSetFuncProp(answerCode, FPInterpreted | FPPseudoPred);

    internalSymbols = fCount();
}

/*****************************************************************************
 * Set the value of the predicate field for a function symbol. 
 * 将一个元素设置为谓词项属性.  * isDel - 是否删除该属性
 ****************************************************************************/
void Sigcell::SigSetPredicate(FunCode f_code, bool isDel) {
    assert(f_code > 0);
    assert(f_code <= fCount());
    if (isDel) {
        SigSetFuncProp(f_code, FPPredSymbol);
    } else {
        SigDelFuncProp(f_code, FPPredSymbol);
    }
}

/*****************************************************************************
 * Return the value of the predicate field for a function symbol.
 * 查询该元素是否是一个谓词元素.
 ****************************************************************************/
bool Sigcell::SigIsPredicate(FunCode f_code) {
    assert(f_code > 0);         //确保不是变元
    assert(f_code <= fCount()); //确保一定是存在的
    return SigQueryFuncProp(f_code, FPPredSymbol);
}

/***************************************************************************** 
 * Set the value of the function field for a function symbol.
 * 将一个元素设置为函数项属性.  * isDel - 是否删除该属性
 ****************************************************************************/
void Sigcell::SigSetFunction(FunCode f_code, bool isDel) {
    assert(f_code > 0);
    assert(f_code <= fCount());
    if (isDel) {
        SigSetFuncProp(f_code, FPFuncSymbol);
    } else {
        SigDelFuncProp(f_code, FPFuncSymbol);
    }
}

/*****************************************************************************
 * Return the value of the Function field for a function symbol. 
 * 查询该元素是否是一个函数元素.
 ****************************************************************************/
bool Sigcell::SigIsFunction(FunCode f_code) {
    assert(f_code > 0);
    assert(f_code <= fCount());
    return SigQueryFuncProp(f_code, FPFuncSymbol);
}

/*****************************************************************************
 * Set the special value of all symbols in sig. 
 * 将所有元素设置为特殊项属性.  * isDel - 是否删除该属性
 ****************************************************************************/
void Sigcell::SigSetAllSpecial(bool isDel) {
    for (FunCode i = 1; i <= fCount(); ++i) {
        SigSetSpecial(i, isDel);
    }
}

/***************************************************************************** 
 * Set the value of the special field for a function symbol.
 * 将一个元素设置为特殊项属性.  * isDel - 是否删除该属性
 ****************************************************************************/
void Sigcell::SigSetSpecial(FunCode f_code, bool isDel) {
    assert(f_code > 0);
    assert(f_code <= fCount());
    if (isDel) {
        SigSetFuncProp(f_code, FPSpecial);
    } else {
        SigDelFuncProp(f_code, FPSpecial);
    }
}

/***************************************************************************** 
 * Return the value of the special field for a function symbol.
 * 查询该元素是否是一个特殊项
 ****************************************************************************/
bool Sigcell::SigIsSpecial(FunCode f_code) {
    assert(f_code > 0);
    assert(f_code <= fCount());
    return SigQueryFuncProp(f_code, FPSpecial);
}

/*****************************************************************************
 * Given an function symbol code, return the symbols alpha-rank.
 * 给定一个fCode,查询对应元素(项)的 alphaRank;
 ****************************************************************************/
int Sigcell::SigGetAlphaRank(FunCode f_code) {
    assert(f_code > 0);
    assert(f_code <= fCount());
    if (!alphaRanksValid) {
        SigComputeAlphaRanks();
    }
    assert(alphaRanksValid);
    return fInfo[f_code]->alphaRank;
}

/***************************************************************************** 
 * Insert the symbol name with arity into the signature.Return the f_code 
 * assigned to the name or 0 if the same name has already been used with a different arity.
 * 插入一个元素到signature数组中,并返回数组下标作为元素的fCode,若存在相同名称但是不同arity的则,返回 0;
 * specialId - 是否为特殊元素
 ****************************************************************************/
FunCode Sigcell::SigInsertId(const string& name, int ari, bool isSpecialId) {//函数有问题

    long pos = SigFindFCode(name); //根据元素名,在当前存储中查找该元素,得到元素的索引

    if (pos) /* name is already known */ {
        if (fInfo[pos]->arity != ari) /*找到的元素不一致输出,返回0*/ {
            printf("Problem: %s %d != %d\n", name.c_str(), ari, fInfo[pos]->arity);
            return 0; /* ...but incompatible */
        }
        if (isSpecialId) {
            SigSetSpecial(pos, true); /* 处理特殊ID*/
        }
        return pos; /* all is fine... */
    }
    //插入元素
    Func_p newFunc = new FuncCell(name, ari, 0, FPIgnoreProps);
    fInfo.push_back(newFunc);

    /* 没有该元素,插入新元素 注意下标是从1开始的 fInfo[0]存储的为NULL*/
    int fInd = fCount();

    //插入到伸展树
    StrTree_p newSTNode = new StrTreeCell();
    newSTNode->key = name;
    newSTNode->val1.i_val = fInd;
    StrTree_p newRoot = fIndex.Insert(newSTNode);
    assert(newRoot == NULL);



    SigSetSpecial(fInd, isSpecialId);
    alphaRanksValid = false;
    return fCount() ;
}

/*****************************************************************************
 * Return the index of the entry name in sig, or 0 if name is not in sig. 
 * 根据元素名称,伸展树进行查找,找到返回元素的索引,否则返回0
 ****************************************************************************/
FunCode Sigcell::SigFindFCode(const string& name) {
    StrTree_p entry = fIndex.FindByKey(name);
    if (entry) {
        return (entry->val1).i_val;
    }
    return 0;
}

/*****************************************************************************
 * Insert a special function symbol used to encode a first-order operator. 
 * 插入一个特殊的符号 一阶逻辑操作符号 ,并得到funCode [数组下标索引]
 ****************************************************************************/
FunCode Sigcell::SigInsertFOFOp(const string& name, int arity) {
    FunCode res = SigInsertId(name, arity, true);
    SigSetFuncProp(res, FPFOFOp);
    return res;
}

/*****************************************************************************
 * Parse an operator, return its FunCode. Error, if operator is not in sig. 
 ****************************************************************************/
FunCode Sigcell::SigParseKnownOperator(Scanner* in) {
    string strId;
    in->FuncSymbParse(strId);
    FunCode res = SigFindFCode(strId);

    if (!res) {
        string errpos;
        in->AktToken()->PosRep(errpos);
        errpos += ' ' + strId + " undeclared!";
        //Error(DStrView(errpos), SYNTAX_ERROR);
        cout << "SYNTAX_ERROR:" + errpos << endl;
    }
    return res;
}

/***************************************************************************** 
 *  Parse a single symbol declaration (f:3) and insert it into sig.
 ****************************************************************************/
FunCode Sigcell::SigParseSymbolDeclaration(Scanner* in, bool isSpecialId) {


    string strId;
    in->FuncSymbParse(strId);
    in->AcceptInpTok(TokenType::Colon);
    int arity = in->AktToken()->numval;
    in->AcceptInpTok(TokenType::PosInt);
    FunCode res = SigInsertId(strId, arity, isSpecialId);
    if (!res) {
        string errpos;
        in->AktToken()->PosRep(errpos);
        errpos += ' ' + strId + " declared with arity " + to_string(arity) + " but registered with arity ";
        cout << "SYNTAX_ERROR:" + errpos << " " + SigFindArity(SigFindFCode(strId)) << endl;
    }
    return res;
}

/***************************************************************************** 
 * Parse a list of declarations into a signature.
 ****************************************************************************/
FunCode Sigcell::SigParse(Scanner* in, bool special_ids) {
    FunCode res = 0;

    /* FIXME: Cannot handle complex symbols here! */
    while (in->TestInpTok(TokenType::FuncSymbToken) 
            && in->LookToken(1)->TestTok(TokenType::Colon)) {
        res = SigParseSymbolDeclaration(in, special_ids);
    }
    return res;
}

/*****************************************************************************
 * Return the largest arity of any function symbol used in the signature. 
 ****************************************************************************/
int Sigcell::SigFindMaxUsedArity() {
    int res = 0;
    FunCode i;

    for (i = 1; i <= fCount(); i++) {
        res = MAX(res, SigFindArity(i));
    }
    return res;
}

/*****************************************************************************
 * Return the largest arity of any predicate function symbol used in the signature.  
 ****************************************************************************/
int Sigcell::SigFindMaxPredicateArity() {
    FunCode i;
    int res = 0, arity;

    for (i = internalSymbols + 1; i <= fCount(); ++i) {
        if (SigQueryFuncProp(i, FPPredSymbol) &&
                !SigQueryFuncProp(i, FPSpecial)) {
            arity = SigFindArity(i);
            res = MAX(res, arity);
        }
    }
    return res;
}

/*****************************************************************************
 * Return the smallest arity of any predicate function symbol used in the signature.  
 ****************************************************************************/
int Sigcell::SigFindMinPredicateArity() {
    FunCode i;
    int res = INT_MAX, arity;

    for (i = internalSymbols + 1; i <= fCount(); ++i) {
        if (SigQueryFuncProp(i, FPPredSymbol) &&
                !SigQueryFuncProp(i, FPSpecial)) {
            arity = SigFindArity(i);
            res = MIN(res, arity);
        }
    }
    return res;
}

/***************************************************************************** 
 *  Return the largest arity of any real function symbol used in the signature.
 ****************************************************************************/
int Sigcell::SigFindMaxFunctionArity() {
    FunCode i;
    int res = 0, arity;
    for (i = internalSymbols + 1; i <= fCount(); i++) {
        if (!FuncIsAnyPropSet(i, FPPredSymbol | FPSpecial)) {
            arity = SigFindArity(i);
            res = MAX(res, arity);
        }
    }
    return res;
}

/***************************************************************************** 
 * Return the smallest arity of any real function symbol used in the signature. 
 ****************************************************************************/
int Sigcell::SigFindMinFunctionArity() {
    FunCode i;
    int res = INT_MAX, arity;
    for (i = internalSymbols + 1; i <= fCount(); i++) {
        if (!FuncIsAnyPropSet(i, FPPredSymbol | FPSpecial)) {
            arity = SigFindArity(i);
            res = MIN(res, arity);
        }
    }
    return res;
}

/***************************************************************************** 
 * Count number of symbols with a given arity. If predictates is true, 
 * count predicates, otherwise count function symbols.
 ****************************************************************************/
int Sigcell::SigCountAritySymbols(int arity, bool predicates) {
    FunCode i;
    int res = 0, tmp_arity;
    for (i = internalSymbols + 1; i <= fCount(); ++i) {
        if (EQUIV(SigIsPredicate(i), predicates)
                &&(!SigIsSpecial(i))) {
            tmp_arity = SigFindArity(i);
            if (tmp_arity == arity) {
                res++;
            }
        }
    }
    return res;
}

/*****************************************************************************  
 * Count number of symbols. If predictates is true, 
 * count predicates, otherwise count function symbols.
 ****************************************************************************/
int Sigcell::SigCountSymbols(bool predicates) {
    FunCode i;
    int res = 0;
    for (i = internalSymbols + 1; i <= fCount(); ++i) {
        if (!SigIsSpecial(i)) {
            if (predicates && SigIsPredicate(i)) {
                res++;
            } else if (!predicates && SigIsFunction(i)) {
                res++;
            }
        }
    }
    return res;
}

/*****************************************************************************
 * Count the occurences of symbols of a given arity (by adding one
 * for each symbol to the corresponding entry in distrib). If
 * predicates is true, count predicate symbols only, otherwise count
 * function symbols only. Only looks at symbols where select[symbol]
 * is true. Return maximal arity of relevant symbols. 
 ****************************************************************************/
int Sigcell::SigAddSymbolArities(vector<IntOrP*> distrib, bool predicates, long selection[]) {
    FunCode i;
    int max_arity = -1, arity;

    for (i = 1; i <= fCount(); ++i) {
        if (EQUIV(SigIsPredicate(i), predicates) &&
                selection[i]) {
            arity = SigFindArity(i);
            max_arity = MAX(arity, max_arity);
            //PDArrayElementIncInt(distrib, arity, 1);
            /***************************************************************************** 
             *改写 PDArrayElementIncInt
             ****************************************************************************/
            distrib[arity]->i_val += 1;
        }
    }
    return max_arity;
}

/***************************************************************************** 
 * Return FunCode for $orn, create them if non-existant.
 ****************************************************************************/
FunCode Sigcell::SigGetOrNCode(int arity) {

    FunCode res = 0;
    if (arity < ornCodes.size()) {
        res = ornCodes[arity].i_val;
    }

    if (res) {
        return res;
    }

    char tmpStr[16]; /* large enough for "or" + digits of INT_MAX */
    sprintf(tmpStr, "$or%d", arity);

    string str = tmpStr;
    res = SigInsertId(str, arity, true);
    assert(res);
    res = ornCodes[arity].i_val; //???
    return res;

}

/*****************************************************************************
 * If eqn_code is passed in, return neqn_code, and vice versa. 
 * Assumes FOF-initialized signature.
 ****************************************************************************/
FunCode Sigcell::SigGetOtherEqnCode(FunCode f_code) {
    if (f_code == eqnCode) {
        return neqnCode;
    }
    assert(f_code == neqnCode);
    return eqnCode;
}

/*****************************************************************************
 * Return a new skolem symbol with arity n. The symbol will be of
 * the form esk<count>_<ar>, and is guaranteed to be new to sig.
 ****************************************************************************/
FunCode Sigcell::SigGetNewSkolemCode(int arity) {
    FunCode res;
    char new_symbol[24];
    skolemCount++;
    sprintf(new_symbol, "esk%ld_%d", skolemCount, arity);
    while (SigFindFCode(new_symbol)) {
        skolemCount++;
        sprintf(new_symbol, "esk%ld_%d", skolemCount, arity);
    }
    res = SigInsertId(new_symbol, arity, false);
    SigSetFuncProp(res, FPFuncSymbol);
    return res;
}

/*****************************************************************************
 * Return a new predicate symbol with arity n. The symbol will be of
 * the form epred<count>_<ar>, and is guaranteed to be new to sig. 
 ****************************************************************************/
FunCode Sigcell::SigGetNewPredicateCode(int arity) {
    FunCode res;
    char new_symbol[26];

    newpredCount++;
    sprintf(new_symbol, "epred%ld_%d", newpredCount, arity);
    while (SigFindFCode(new_symbol)) {
        newpredCount++;
        sprintf(new_symbol, "epred%ld_%d", newpredCount, arity);
    }
    res = SigInsertId(new_symbol, arity, false);
    SigSetFuncProp(res, FPPredSymbol);

    return res;
}