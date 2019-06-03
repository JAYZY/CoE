/* 
 * File:   SortRule.h
 * Author: Zhong Jian<77367632@qq.com>
 * Contents: 定义一系列的 排序规则
 * Created on 2018年10月27日, 下午12:58
 */

#ifndef SORTRULE_H
#define SORTRULE_H

#include "CLAUSE/Clause.h"
#include "StrategyParam.h"

class SortRule {
public:

    SortRule(void) {
    };

    ~SortRule(void) {
    };

    static inline bool LitCmp(Literal* litA, Literal* litB) {

        //对子句中的文字进行排序
        /*1. 优先使用次数少的文字;2,优先使用稳定低的文字 3.优先使用负文字; */
        if (litA->usedCount != litB->usedCount)
            return litA->usedCount < litB->usedCount; //优先使用次数少的文字
        long weight = litA->StandardWeight() - litB->StandardWeight(); //稳定度由低到高
        if (0 == weight) {
            if (litA->IsNegative() && litB->IsPositive())
                return true;
            if (litB->IsNegative() && litA->IsPositive())
                return false;
        }
        return weight<0; //稳定度由低到高
    }

    /*--------------------------------------------------------------------------
    / 被动归结文字排序（排序的文字在同一个谓词对应的文字列表中）                /
    /1.子句所在文字数最少;2.用weight(KBO)代替稳定度，由低到高，  【文字稳定度(默认由低到高)】
    /-------------------------------------------------------------------------*/
    static bool PoslitCmp(Literal*litA, Literal*litB) {

        int32_t litNumDiff = litB->claPtr->LitsNumber() - litA->claPtr->LitsNumber();
        if (litNumDiff == 0) {
            return (StrategyParam::SEL_POSLIT_STEADY == POSLIT_STEADY::NumDesc) ? litB->StandardWeight() > litA->StandardWeight() //由高到低
                    : litA->StandardWeight() < litB->StandardWeight(); //由低到高 -- KOB 反应 文字的复杂程度
        }
        return litNumDiff > 0;

    }



    /************************************************************************/
    /*    子句排序--选择排序规则                                            */
    /************************************************************************/
    static bool ClaCmp(const Clause*pc1, const Clause * pc2);

    //优先级比较

    static inline bool ClaCmpByPrio(const Clause* pc1, const Clause * pc2) {
        return (pc1->priority > pc2->priority);
    }
};


#endif /* SORTRULE_H */

