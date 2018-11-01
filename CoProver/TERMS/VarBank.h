/*
 * 创建变元项
 * File:   VarBank.h
 * Author: zj 
 *  文件修改．．．．．
 * Created on 2017年2月24日, 下午4:50
 */

#ifndef VARBANK_H
#define VARBANK_H

//#include "Sigcell.h"
#include "TermCell.h"
//class TermCell;

class VarBank {
public:
    FunCode vCount; /* FunCode counter for new variables */
    FunCode maxVar; /* Largest variable ever created */
    /* 使用splayTree 来存储与该变量相关的名称　Associate names and cells */
    SplayTree<StrTreeCell>extIndex;
    /* 使用动态数组 来存储与该变量相关的变元ID(变元项) Associate FunCodes and cells */
    vector<TermCell*> vctFCodes;

public:

    /*---------------------------------------------------------------------*/
    /*                    　inline　 Function   　                          */
    /*---------------------------------------------------------------------*/
    //

    inline FunCode VarBankGetVCount() {
        return vCount;
    };

    inline void VarBankSetVCount(FunCode varCount) {
        vCount = varCount;
    };

    TermCell* FindByFCode(FunCode idx) {
        assert(idx < 0);
        FunCode id = -idx;
        if (!(id < vctFCodes.size())) {
            int s = vctFCodes.size();
            for (int i = s; i <= id; ++i)
                vctFCodes.push_back(nullptr);
            //vctFCodes.resize(id+1);            
        }
        return vctFCodes[id];
    };

    inline TermCell* VarBankExtNameFind(const string& name) {
        StrTree_p entry = extIndex.FindByKey(name);
        if (entry) {
            return (TermCell*) entry->val1.p_val;
        }
        return nullptr;
    };

    inline void VarBankClearExtNamesNoReset() {
        extIndex.Destroy();
    }

    inline void VarBankClearExtNames() {
        VarBankClearExtNamesNoReset();
        vCount = 0;
    }

    /**
     * 重置变元列表为空(设置总数量为空)
     */
    inline void VarBankResetVCount() {
        vCount = 0;
    }
    /*---------------------------------------------------------------------*/
    /*                    　member　 Function   　                          */
    /*---------------------------------------------------------------------*/

    /* Set the given properties in all variables. */
    void VarBankVarsSetProp(TermProp prop);

    /* Delete the given properties in all variables. */
    void VarBankVarsDelProp(TermProp prop);

    /* 根据编码插入一个变元项到varBanks中－查找变元项编码fCode 存在返回varTerm的指针．
     * 否则，构造新的变元项插入到varBank中 */
    TermCell* Insert(FunCode fCode);

    /* 根据变元项名称插入一个变元项到varBanks中 */
    TermCell* Insert(const string& name);

    /* 返回 vector中最后一个变元项.
     * 注意　整个vector只有　偶数下标存储变元项，奇数下标保留作为创建子句备份
     * （create clause copies that are　guaranteed to be variable-disjoint.） */
    TermCell* VarBankGetFreshVar();

    TermCell* TermEquivCellAlloc(TermCell* term);

    long VarBankCheckBindings(FILE* out, Sigcell* sig);

    Term_p VarBankFCodeAssertAlloc(FunCode f_code);

    Term_p VarBankFCodeFind(FunCode f_code);

    void print() {
        extIndex.Print();
    }

    /*---------------------------------------------------------------------*/
    /*                    Constructed Function                             */
    /*---------------------------------------------------------------------*/
    VarBank();
    VarBank(const VarBank& orig);
    virtual ~VarBank();

};
//定义类别名
typedef VarBank* VarBank_p;

#endif /* VARBANK_H */

