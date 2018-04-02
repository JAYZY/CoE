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
    int operator()( Literal*t1,  Literal* t2) {
        t1->zjLitWeight();t2->zjLitWeight();
        return (t1->zjlitWight-t2->zjlitWight);
    }
};


#endif /* LITERALCMP_H */

