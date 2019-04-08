/*
 * 定义文字比较相关的类(仿函数)
 */

/* 
 * File:   LiteralCompare.h
 * Author: Zhong Jian<77367632@qq.com>
 *
 * Created on 2018年4月2日, 上午11:05
 */

#ifndef LITERALCMP_H
#define LITERALCMP_H
#include "Literal.h"
#include "Clause.h"

/**
 * 单元子句排序规则-根据文字权重排序
 */
class UnitClaCompareWithWeight {
public:
    bool operator()(Clause* cA, Clause* cB) {
        return cA->Lits()->StandardWeight() > cB->Lits()->StandardWeight();
    }
};

class EqnCompareKBO6 {
public:

    int operator()(Literal*t1, Literal* t2) {
        CompareResult res = t1->Compare(t2);
        if (res == CompareResult::toGreater)
            return 1;
        return -1;
    }
};

class SteadyCMP {
public:

    int operator()(Literal*t1, Literal* t2) {
        t1->zjLitWeight();
        t2->zjLitWeight();
        return (t1->zjlitWight - t2->zjlitWight);
    }
};
//只是简单统计变元项
//思想,变元越多,稳定度越好

class CountVarT {
public:

    int operator()(Literal*t1, Literal* t2) {

        int x1 = t1->VarNum();
        int x2 = t2->VarNum();
        return (x1 - x2);
    }
};

class ImprovementCMP {
public:

    float operator()(Literal* t1, Literal * t2) {

        if (t1->EqnIsEquLit()&&!t2->EqnIsEquLit())
            return -1;
        if (!t1->EqnIsEquLit() && t2->EqnIsEquLit())
            return 1;

        //fprintf(stdout,"t1->zjlitWight:%5.2f t2->zjlitWight:%5.2f\n",t1->zjlitWight,t2->zjlitWight);
        return t1->zjlitWight - t2->zjlitWight;
    }
};
//比较每个项

class SubTermCMP {
public:

    float operator()(Literal* t1, Literal * t2) {
        if (t1->EqnIsEquLit()&&!t2->EqnIsEquLit())
            return -1;
        if (!t1->EqnIsEquLit() && t2->EqnIsEquLit())

            return 1;
        //无位置<变元<常元

        //fprintf(stdout,"t1->zjlitWight:%5.2f t2->zjlitWight:%5.2f\n",t1->zjlitWight,t2->zjlitWight);
        return t1->zjlitWight - t2->zjlitWight;
    }
};

//只比较个数

class ConstLenCMP {
public:

    float operator()(Literal* t1, Literal * t2) {
        if (t1->EqnIsEquLit()&&!t2->EqnIsEquLit())
            return -1;
        if (!t1->EqnIsEquLit() && t2->EqnIsEquLit())

            return 1;
        //fprintf(stdout,"t1->zjlitWight:%5.2f t2->zjlitWight:%5.2f\n",t1->zjlitWight,t2->zjlitWight);
        return t1->ConstNum() - t2->ConstNum(); //变元越多越不稳定
    }
};
//只比较变元个数

class OnlyVarLenCMP {
public:

    float operator()(Literal* t1, Literal * t2) {
        if (t1->EqnIsEquLit()&&!t2->EqnIsEquLit())
            return -1;
        if (!t1->EqnIsEquLit() && t2->EqnIsEquLit())

            return 1;
        //fprintf(stdout,"t1->zjlitWight:%5.2f t2->zjlitWight:%5.2f\n",t1->zjlitWight,t2->zjlitWight);
        return t2->VarNum() - t1->VarNum(); //变元越多越不稳定
    }
};
//只比较文字深度

class OnlyMaxDepthCMP {
public:

    float operator()(Literal* t1, Literal * t2) {
        if (t1->EqnIsEquLit()&&!t2->EqnIsEquLit())
            return -1;
        if (!t1->EqnIsEquLit() && t2->EqnIsEquLit())

            return 1;
        //fprintf(stdout,"t1->zjlitWight:%5.2f t2->zjlitWight:%5.2f\n",t1->zjlitWight,t2->zjlitWight);
        return t1->EqnDepth() - t2->EqnDepth(); //变元越多越不稳定
    }
};

class StandardWCMP {
public:

    int operator()(Literal* t1, Literal * t2) {
        if (t1->EqnIsEquLit()&&!t2->EqnIsEquLit())
            return -1;
        if (!t1->EqnIsEquLit() && t2->EqnIsEquLit())
            return 1;
        long x = (t1->StandardWeight() - t2->StandardWeight());

        return x;
    }
};

//class ZXMSteadyCMP {
//public:
//
//    float operator()(Literal* t1, Literal * t2) {
//        if (t1->EqnIsEquLit()&&!t2->EqnIsEquLit())
//            return 1;
//        if (!t1->EqnIsEquLit() && t2->EqnIsEquLit())
//
//            return -1;
//        return t1->xyW - t2->xyW;
//    }
//};
//
//class XYSteadyCMP {
//public:
//
//    float operator()(Literal* t1, Literal * t2) {
//
//        if (t1->EqnIsEquLit()&&!t2->EqnIsEquLit())
//            return -1;
//        if (!t1->EqnIsEquLit() && t2->EqnIsEquLit())
//            return 1;
//        return t1->xyW - t2->xyW;
//    }
//};


#endif /* LITERALCMP_H */

