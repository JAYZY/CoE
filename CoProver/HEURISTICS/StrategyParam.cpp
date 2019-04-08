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

string StrategyParam::tptpFileName="";


//初始化参数==================================	
bool StrategyParam::ADD_EQULITY = false;
ClaSelStrategy StrategyParam::CLAUSE_SEL_STRATEGY=ClaSelStrategy::Num_Weight;
POSLIT_STEADY StrategyParam::SEL_POSLIT_STEADY = POSLIT_STEADY::NumDesc;
ALimit StrategyParam::ALIT_LIMIT = ALimit::NoLimit;
int32_t StrategyParam::IterCount_LIMIT = INT_MAX; 
uint32_t StrategyParam::R_MAX_LITNUM=10;
uint32_t StrategyParam::HoldLits_NUM_LIMIT = 10; //归结过程中剩余文字最大文字数限制
uint32_t StrategyParam::R_MAX_FUNCLAYER=3;


 

