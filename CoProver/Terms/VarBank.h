/*
 * 创建变元项
 * File:   VarBank.h
 * Author: zj 
 *  文件修改．．．．． * 
 * Created on 2017年2月24日, 下午4:50
 * Modify on  2019年07月05日 ： 
 *          注意，在三角形演绎过程中 只有子句中的变元才会存在共享情况。即：VarBank只会存在于带变元的子句中
 *  重要修改，取消变元X,Y， 预留2倍空间。改为直接使用 X
 * 
 */

#ifndef VARBANK_H
#define VARBANK_H

//#include "Sigcell.h"
//#include <bits/stdint-uintn.h>

#include "TermCell.h"
//class TermCell;

class VarBank {
public:
    //FunCode vCount; /* FunCode counter for new variables */
    // FunCode maxVar; /* Largest variable ever created */
    /* 使用splayTree 来存储与该变量相关的名称　Associate names and cells */
    //伸展数方式存储 项节点handle: key=name(如："x1" ); val1.p_val =var(项); val2.i_val = var->fCode(项的Fcode) 
    //注意：这个fcode本质上是 子句中 变元数组的下标。变元数组的下标
    SplayTree<StrTreeCell>extIndex;  //key = name; val1 = var; var2 = var.fcode
    ///  SplayTree<PTreeCell> freeVarSets;
    /* 使用动态数组 来存储与该变量相关的变元ID(变元项) Associate FunCodes and cells */
    vector<TermCell*> vctFCodes;
    //记录变元项对应的文字（位置pos)
    vector<uint32_t> mapVarToLitPos; //用位存储,--限制最多32个文字 64个文字占用内存太多了

public:

    /*-----------------------------------------------------------------------*/
    /*                    　inline　 Function   　                          */
    /*---------------------------------------------------------------------*/
    //

    inline int  VarBankGetVCount() {
        return vctFCodes.size();
    };

    /**
     * 通过变元下标Fcode 查找变元
     * @param idx 变元项Fcode
     * @return  找到返回数组中的变元项，否则返回 nullptr;
     */
    inline TermCell* FindByFCode(FunCode idx) {
        assert(idx < 0);
        FunCode id = -idx;
        if (id < vctFCodes.size()) {
            return vctFCodes[id];
        } else {
            return nullptr;
        }
    };

    /**
     * 通过变元名称查找变元项，查找目标为伸展树
     * @param name 变元名称 如 “x1"
     * @return 找到返回伸展树上的 变元项，否则返回nullptr
     */
    inline TermCell* VarBankExtNameFind(const string& name) {
        StrTree_p entry = extIndex.FindByKey(name);
        if (entry) {
            return (TermCell*) entry->val1.p_val;
        }
        return nullptr;
    };

    /*****************************************************************************
     * 清除变元项的字符存储
     ****************************************************************************/
    inline void VarBankClearExtNamesNoReset() {
        extIndex.Destroy();
    }

    /*****************************************************************************
     *  清除所有内存
     ****************************************************************************/
    inline void VarBankClearExtNames() {
        extIndex.Destroy();
        for (int i = 0; i < vctFCodes.size(); ++i) {
            DelPtr(vctFCodes[i]); //将所有变元项删除
        }
        vector<TermCell*>().swap(vctFCodes);
    }

    inline void print() {
        extIndex.Print();
    }

    /*---------------------------------------------------------------------*/
    /*                    　member　 Function   　                          */
    /*---------------------------------------------------------------------*/

    /* Set the given properties in all variables. */
    void VarBankVarsSetProp(TermProp prop);

    /* Delete the given properties in all variables. */
    void VarBankVarsDelProp(TermProp prop);

    /*  [根据变元项编码fCode] -- 插入一个变元项到varBanks中 */
    TermCell* Insert(FunCode fCode, uint16_t claId);//,int litPos


    /*  [根据变元项名称] -- 插入一个变元项到varBanks中*/
    TermCell* Insert(const string& name, uint16_t claId);

    TermCell* VarBankGetFreshVar(uint16_t claId);

    long VarBankCheckBindings(FILE* out, Sigcell * sig);

    Term_p VarBankFCodeAssertAlloc(FunCode f_code);



    /*---------------------------------------------------------------------*/
    /*                    Constructed Function                             */
    /*---------------------------------------------------------------------*/
    VarBank();

    virtual ~VarBank();

};
//定义类别名
typedef VarBank* VarBank_p;

#endif /* VARBANK_H */

