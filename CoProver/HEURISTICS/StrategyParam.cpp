/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   StrategyParam.cpp
 * Author: Zhong Jian<77367632@qq.com>
 * 
 * Created on 2018年11月8日, 下午12:07
 */

#include "StrategyParam.h"

StrategyParam::StrategyParam() {
}

StrategyParam::StrategyParam(const StrategyParam& orig) {
}

StrategyParam::~StrategyParam() {
}


//初始化参数==================================	
POSLIT_STEADY StrategyParam::SEL_POSLIT_STEADY = POSLIT_STEADY::NumDesc;
ALimit StrategyParam::ALIT_LIMIT=ALimit::NoLimit;