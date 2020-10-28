/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   InferenceInfo.h
 * Author: Zhong Jian<77367632@qq.com>
 *
 * Created on 2019年3月21日, 上午10:17
 */

#ifndef INFERENCEINFO_H
#define INFERENCEINFO_H
#include "Global/IncDefine.h"
#include "BASIC/SplayTree.h"
#include "BASIC/TreeNodeDef.h"
#include <map>
//#include <bits/stdint-uintn.h>
class Literal;

///结构体:演绎边定义
 typedef struct alit {
    uint16_t reduceNum; //记录,后续的主界线下拉次数
    int16_t unitLitInd; //单文字列表中的下标 默认为 -1; 
    Literal* alit; //主动文字
    Literal* blit; //只记录第一次配对的延拓文字
} ALit, *ALit_p;

struct DeductPath { //演绎路径边记录
    Literal * startLit;
    Literal * endLit;
   // uint16_t 
    uint startSearchId; //主动文字搜索的起始位置
    float edgeWeight;   //边的权重 = (Q(pi) / N(pi)

    DeductPath(Literal * aLit, Literal * pLit, float w) : startLit(aLit), endLit(pLit), edgeWeight(w) {
    }
    string print();
};

class InferenceInfo {
private:
    static string TOInferTypeNames[];
    static vector<Literal*>vecPairLit;
    static map<Literal*, SplayTree<PTreeCell>*> mapDeductNet; //演绎网络--存储已经确定的路线
    static long deductEdgeNum;
public:
    InferenceInfo();
    InferenceInfo(const InferenceInfo& orig);
    virtual ~InferenceInfo();
public:

    static inline void AddDeductPath(Literal *aLit, Literal *pLit, float edgeW) {
        PTreeCell* node = new PTreeCell();
        node->key = pLit;
        node->val1.p_val = new DeductPath(aLit, pLit, edgeW);
        mapDeductNet[aLit]->Insert(node);
    }
    //查找最优节点
    
    
    /// 获取演绎信息
    /// \param inferType
    /// \return 
    static inline string getStrInfoType(InfereType inferType) {
        return TOInferTypeNames[(uint16_t) inferType];
    }

};

#endif /* INFERENCEINFO_H */

