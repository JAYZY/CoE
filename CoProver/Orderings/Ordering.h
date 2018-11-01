/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Ordering.h
 * Author: Zhong Jian<77367632@qq.com>
 *
 * Created on 2018年8月6日, 下午3:40
 */

#ifndef ORDERING_H
#define ORDERING_H
#include "../Global/IncDefine.h"
#include "../TERMS/TermCell.h"

enum class TermOrderType {
    LPO,
    KBO
};

class Ordering {
public:
    static TermOrderType type; //默认KBO
public:
    Ordering();
    Ordering(const Ordering& orig);
    virtual ~Ordering();
public:

    static CompareResult Tocompare(TermCell* t1, TermCell* t2, DerefType derefT1, DerefType derefT2);
    //反转偏序关系

    inline static CompareResult InverseRelation(CompareResult relation) {
        CompareResult res=relation;
        switch (res) {
            case CompareResult::toEqual:
            case CompareResult::toUncomparable:
                break;
            case CompareResult::toGreater:
                res = CompareResult::toLesser;
                break;
            case CompareResult::toLesser:
                res = CompareResult::toGreater;
                break;
            case CompareResult::toNotgteq:
                res = CompareResult::toNotleeq;
                break;
            case CompareResult::toNotleeq:
                res = CompareResult::toNotgteq;
                break;
            default:
                assert(false);
                break;
        }
        return res;
    }
private:

};

#endif /* ORDERING_H */

