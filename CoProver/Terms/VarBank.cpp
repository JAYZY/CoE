/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   VarBank.cpp
 * Author: zj 
 * 
 * Created on 2017年2月24日, 下午4:50
 */
//#include <bits/stdint-uintn.h>

#include "VarBank.h"
#include "BASIC/TreeNodeDef.h"
#include "TermCell.h"

/*****************************************************************************
 *  Set the given properties in all variables.
 ****************************************************************************/
void VarBank::VarBankVarsSetProp(TermProp prop) {

    assert(vctFCodes.size() > 0);
    for (TermCell* elem : vctFCodes) {
        if (elem) {
            SetProp(elem->properties, prop);
        }
    }
}

/*****************************************************************************
 *  Delete the given properties in all variables.
 ****************************************************************************/
void VarBank::VarBankVarsDelProp(TermProp prop) {
    assert(vctFCodes.size() > 0);
    for (TermCell* elem : vctFCodes) {
        if (elem) {
            DelProp(elem->properties, prop);
        }
    }
}

/*****************************************************************************
 * [根据变元项编码fCode] -- 插入一个变元项到varBanks中 
 * 如果存在返回varTerm的指针．否则，插入到varBank中   
 ****************************************************************************/
TermCell* VarBank::Insert(FunCode fCode, uint16_t claId) {
    assert(fCode < 0);
    TermCell* var = FindByFCode(fCode); //通过FCode 查找变元项
    if (var == nullptr) {
        var = new TermCell();
        var->claId = claId;
        var->fCode = fCode;
        var->entryNo = fCode;
        SetProp(var->properties, TermProp::TPIsShared); //给变元项－shared 属性
        vctFCodes.push_back(var);

        // if (litPos > 0)//记录变元项与文字位置的对应关系
        //   SetBitValue(this->mapVarToLitPos[-fcode], litPos);

        assert(!QueryProp(var->properties, TermProp::TPIsGround)); //确保项不是常元项
        return var;
    }
}

/*****************************************************************************
 * [根据变元项名称] -- 插入一个变元项到varBanks中
 * 如果存在返回varTerm的指针．
 * 否则，插入到varBank中
 ****************************************************************************/
TermCell * VarBank::Insert(const string& name, uint16_t claId) {
    TermCell* var = VarBankExtNameFind(name);
    if (var == nullptr) {
        var = Insert(-(vctFCodes.size() + 1), claId); //fCode从-1开始
        StrTree_p handle = new StrTreeCell();
        handle->key = name;
        handle->val1.p_val = var;
        handle->val2.i_val = var->fCode;
        StrTree_p test = extIndex.Insert(handle);
        assert(test == NULL);
    }
    return var;
}

/**************************************************************************
 * 返回一个　在vector中新创建的最后一个　变元项.
 * 注意　整个vector只有　偶数下标存储变元项，　
 * 奇数下标保留作为创建子句备份
 * （create clause copies that are　guaranteed to be variable-disjoint.）
 **************************************************************************/
TermCell * VarBank::VarBankGetFreshVar(uint16_t claId) {
    TermCell* var = Insert(-(vctFCodes.size() + 1), claId);
    assert(var);
    return var;
}

/*****************************************************************************
 * Return a pointer to the variable with the given f_code in the variable bank. 
 * Create the variable if it does not exist. 
 ****************************************************************************/
Term_p VarBank::VarBankFCodeAssertAlloc(FunCode f_code) {
    Term_p var;

    assert(f_code < 0);
    var = FindByFCode(f_code);
    //var = VarBankFCodeFind(f_code);
    if (!var) {
        var = new TermCell();
        var->claId = 0;
        var->entryNo = f_code;
        var->fCode = f_code;
        var->TermCellSetProp(TermProp::TPIsShared);
        //PDArrayAssignP(bank->f_code_index, -f_code, var);
        vctFCodes.push_back(var);

    }
    assert(!var->TermCellQueryProp(TermProp::TPIsGround));
    return var;
}

/*-----------------------------------------------------------------------
//
// Function: VarBankCheckBindings()
//
//   For all variables in bank, check if they are bound. If sig!=0,
//   print the variable and binding as a comment, otherwise just print
//   variable number. Return number of bound variables.
//
// Global Variables: -
//
// Side Effects    : Output, Memory
//
/----------------------------------------------------------------------*/

FunCode VarBank::VarBankCheckBindings(FILE* out, Sig_p sig) {

    VarBank* bank = this;
    TermCell* term;
    long res = 0;

    fprintf(out, "#  VarBankCheckBindings() started...\n");
    for (int i = 0; i < bank->vctFCodes.size(); i++) {
        term = bank->vctFCodes[i];
        if (term) {
            assert(term->IsVar());
            if (term->binding) {
                res++;
                if (sig) {
                    fprintf(out, "# %ld: ", term->fCode);
                    term->TermPrint(out, DerefType::DEREF_NEVER);
                    fprintf(out, " <--- ");
                    term->TermPrint(out, DerefType::DEREF_ONCE);
                    fprintf(out, "\n");
                } else {
                    fprintf(out, "# Var%ld <---- %p\n",
                            term->fCode,
                            (void*) term->binding);
                }
            }
        }
    }
    fprintf(out, "#  ...VarBankCheckBindings() completed\n");
    return res;
}


/*---------------------------------------------------------------------*/
/*                    Constructed Function                           */

/*---------------------------------------------------------------------*/
VarBank::VarBank() {
    vctFCodes.reserve(8);

}

VarBank::~VarBank() {
    VarBankClearExtNames();
}