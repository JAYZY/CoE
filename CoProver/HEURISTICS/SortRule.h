/* 
 * File:   SortRule.h
 * Author: Zhong Jian<77367632@qq.com>
 * Contents: 定义一系列的 排序规则
 * Created on 2018年10月27日, 下午12:58
 */

#ifndef SORTRULE_H
#define SORTRULE_H

#include "CLAUSE/Literal.h"
#include "StrategyParam.h"

class SortRule {
public:

    SortRule(void) {
    };

    ~SortRule(void) {
    };

    inline static bool LitCmp(Literal* litA, Literal* litB) {

        //对子句中的文字进行排序
        /*建议 先负后正,先稳定低后稳定高  */
        if (litA->IsNegative() && litB->IsPositive())
            return true;
        if (litB->IsNegative() && litA->IsPositive())
            return false;
        return litA->StandardWeight() < litB->StandardWeight(); //稳定度由低到高
    }

    /************************************************************************/
    /*被动归结文字排序（排序的文字在同一个谓词对应的文字列表中）						*/

    /*1.子句所在文字数最少;2.文字稳定度(默认由低到高)
    /************************************************************************/
    static bool PoslitCmp( Literal*litA,  Literal*litB) {

        int32_t litNumDiff = litB->claPtr->LitsNumber() - litA->claPtr->LitsNumber();
//        if (litNumDiff == 0)
//            return (StrategyParam::SEL_POSLIT_STEADY == POSLIT_STEADY::NumDesc) ? litB->StandardWeight() - litA->StandardWeight() : litA->StandardWeight() - litB->StandardWeight();
//        else 
            return litNumDiff>0;

    }
};


#endif /* SORTRULE_H */

