/* 
 * File:   SortRule.h
 * Author: Zhong Jian<77367632@qq.com>
 * Contents: 定义一系列的 排序规则
 * Created on 2018年10月27日, 下午12:58
 */

#ifndef SORTRULE_H
#define SORTRULE_H

#include "CLAUSE/Literal.h"

class SortRule {
public:

    SortRule(void) {
    };

    ~SortRule(void) {
    };

    inline static bool LitCmp(  Literal&litA,   Literal&litB) {

        return litA.StandardWeight() > litB.StandardWeight();

    }
};


#endif /* SORTRULE_H */

