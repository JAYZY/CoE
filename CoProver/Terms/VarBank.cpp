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
 * 插入一个变元项到varBanks中－[根据插入的变元项编码　fCode]
 * 如果存在返回varTerm的指针．否则，插入到varBank中   
 ****************************************************************************/
TermCell* VarBank::Insert(FunCode fCode) {
    assert(fCode < 0);
    TermCell* var = FindByFCode(fCode); //通过FCode 查找变元项
    if (var == nullptr) {
        var = new TermCell();
        var->fCode = fCode;
        var->entryNo = fCode;
        SetProp(var->properties, TermProp::TPIsShared); //给变元项－shared 属性
        vctFCodes[-fCode] = var; //vctFCodes.push_back(var);
        //PDArrayAssignP(bank->f_code_index, -f_code, var);
        maxVar = MAX(-fCode, maxVar);
    }
    assert(!QueryProp(var->properties, TermProp::TPIsGround)); //确保项不是常元项
    return var;
}

/*****************************************************************************
 * 插入一个变元项到varBanks中－[根据插入的变元项名称]
 * 如果存在返回varTerm的指针．
 * 否则，插入到varBank中
 ****************************************************************************/
TermCell* VarBank::Insert(const string& name) {
    TermCell* var = VarBankExtNameFind(name);
    if (!var) {
        var = VarBankGetFreshVar();
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
TermCell* VarBank::VarBankGetFreshVar() {
    vCount += 2;
    vctFCodes.push_back(NULL); //奇数下标保留        
    TermCell* var = Insert(-vCount);
    assert(var);
    return var;
}


//   Return a pointer to a unshared termcell equivalent to source. If
//   source is a variable, get the cell from the varbank, otherwise
//   copy the cell via TermTopCopy().

TermCell* VarBank::TermEquivCellAlloc(TermCell* term) {
    TermCell* handle;
    VarBank* vars = this;
    if (term->IsVar()) {
        handle = vars->Insert(term->fCode);
    } else {
        handle = TermCell::TermTopCopy(term);
    }
    return handle;
}
/*---------------------------------------------------------------------*/
/*                    Constructed Function                             */

/*---------------------------------------------------------------------*/
VarBank::VarBank() : vCount(0), maxVar(0) {
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

/*****************************************************************************
 * Return a pointer to the variable with the given f_code in the variable bank. 
 * Create the variable if it does not exist. 
 ****************************************************************************/
Term_p VarBank::VarBankFCodeAssertAlloc(FunCode f_code) {
    Term_p var;

    assert(f_code < 0);
    var = VarBankFCodeFind(f_code);
    if (!var) {
        var = new TermCell();
        var->entryNo = f_code;
        var->fCode = f_code;
        var->TermCellSetProp(TermProp::TPIsShared);
        //PDArrayAssignP(bank->f_code_index, -f_code, var);
        vctFCodes[-f_code] = var;
        maxVar = MAX(-f_code, maxVar);
    }
    assert(!var->TermCellQueryProp(TermProp::TPIsGround));
    return var;
}

/*****************************************************************************
 * Return the pointer to the variable associated with given f_code if it exists in the VarBank, 
 * NULL otherwise.   
 ****************************************************************************/
Term_p VarBank::VarBankFCodeFind(FunCode f_code) {
    assert(f_code < 0);
    return vctFCodes[-f_code];
}

VarBank::VarBank(const VarBank& orig) {
}

VarBank::~VarBank() {
    extIndex.Destroy();
    for (int i = 0; i < vctFCodes.size(); ++i) {
        DelPtr(vctFCodes[i]);
    }
    vctFCodes.clear();
    vector<TermCell*>().swap(vctFCodes);
}