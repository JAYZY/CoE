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
class ConstLenCMP{
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
        //        if (t1->EqnIsEquLit()&&!t2->EqnIsEquLit())
        //            return 1;
        //        if (!t1->EqnIsEquLit() && t2->EqnIsEquLit())
        //            return -1;

        //        int x1 = t1->computJZD();
        //        int x2 = t2->computJZD();
        //        t1->EqnTSTPPrint(stdout, true);
        //        cout << "  x1JZD:" << x1 << endl;
        //        t2->EqnTSTPPrint(stdout, true);
        //        cout << "  x2JZD:" << x2 << endl;
        //        if (x1 == 0) return -1;
        //        if (x2 == 0) return 1;
        //        long x = (t1->StandardWeight() - t2->StandardWeight());
        //        if (x == 0) {



        //        int x1 = t1->computJZD();
        //        int x2 = t2->computJZD();
        ////        t1->EqnTSTPPrint(stdout, true);
        ////        cout << "  x1JZD:" << x1 << endl;
        ////        t2->EqnTSTPPrint(stdout, true);
        ////        cout << "  x2JZD:" << x2 << endl;
        //        if (x1 == 0) return -1;
        //        if (x2 == 0) return 1;
        //        if (x1 == x2) {
        if (t1->EqnIsEquLit()&&!t2->EqnIsEquLit())
            return -1;
        if (!t1->EqnIsEquLit() && t2->EqnIsEquLit())
            return 1;
        long x = (t1->StandardWeight() - t2->StandardWeight());
        return x;
        //        }
        //        return x1 - x2;

        //        }
        //        return (int) x;



    }
};

class ZXMSteadyCMP {
public:

    float operator()(Literal* t1, Literal * t2) {
        if (t1->EqnIsEquLit()&&!t2->EqnIsEquLit())
            return 1;
        if (!t1->EqnIsEquLit() && t2->EqnIsEquLit())
            return -1;
        return t1->xyW - t2->xyW;
    }
};

class XYSteadyCMP {
public:

    float operator()(Literal* t1, Literal * t2) {
        //        float x=(t1->xyW-t2->xyW);
        //        if(x>0.0f) return 1;
        //        else if(x<0.0f) return -1;
        //        else return 0;
        if (t1->EqnIsEquLit()&&!t2->EqnIsEquLit())
            return -1;
        if (!t1->EqnIsEquLit() && t2->EqnIsEquLit())
            return 1;
        return t1->xyW - t2->xyW;
    }
};
#endif /* LITERALCMP_H */

