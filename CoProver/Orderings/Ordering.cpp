/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Ordering.cpp
 * Author: Zhong Jian<77367632@qq.com>
 * 
 * Created on 2018年8月6日, 下午3:40
 */

#include "Ordering.h"
#include "KBO.h"
TermOrderType Ordering::type = TermOrderType::KBO;

Ordering::Ordering() {
}

Ordering::Ordering(const Ordering& orig) {
}

Ordering::~Ordering() {
}

/*---------------------------------------------------------------------*/
/*                          Static Function                            */
/*---------------------------------------------------------------------*/
//

CompareResult Ordering::Tocompare(TermCell* t1, TermCell* t2, DerefType derefT1, DerefType derefT2) {
    CompareResult res = CompareResult::toUnknown;
    KBO kbo6;
    switch (type) {
        case(TermOrderType::LPO):
            Out::Error("no LPO ordering", ErrorCodes::USAGE_ERROR);
            break;
        case(TermOrderType::KBO):
            res = kbo6.KBO6Compare(t1, t2, derefT1, derefT2);
            break;
        default:
            assert(false);
            Out::Error("no set ordering", ErrorCodes::USAGE_ERROR);
            break;
    }
    return res;
}
