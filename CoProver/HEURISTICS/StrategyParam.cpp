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

string StrategyParam::tptpFileName = "";

uint8_t StrategyParam::CLA_NOMGU_WIGHT = 255; //子句起步找不到归结,改变的优先级 (说明该子句不适合起步)
uint8_t StrategyParam::CLA_REDUNDANCY_WIGHT = 10; //子句归结中发现冗余,改变的优先级 
uint8_t StrategyParam::CLA_OVERLIMIT_WIGHT = 5; //子句超过限制,改变的优先级

uint8_t StrategyParam::LIT_REDUNDANCY_WIGHT = 5; //文字归结中发现冗余,改变的优先级 (说明该文字不适合起步)
uint8_t StrategyParam::LIT_OVERLIMIT_WIGHT = 10; //文字超过限制,改变的优先级
//初始化参数==================================	
bool StrategyParam::ADD_EQULITY = false;
ClaSelStrategy StrategyParam::CLAUSE_SEL_STRATEGY = ClaSelStrategy::Num_Weight;
POSLIT_STEADY StrategyParam::SEL_POSLIT_STEADY = POSLIT_STEADY::NumDesc;
ALimit StrategyParam::ALIT_LIMIT = ALimit::NoLimit;
int32_t StrategyParam::IterCount_LIMIT = INT_MAX;
uint32_t StrategyParam::R_MAX_LITNUM = 5;
uint32_t StrategyParam::HoldLits_NUM_LIMIT = 5; //归结过程中剩余文字最大文字数限制
uint32_t StrategyParam::R_MAX_FUNCLAYER = 6;


// <editor-fold defaultstate="collapsed" desc="运行时的状态">


 uint16_t StrategyParam::S_OverMaxLitLimit_Num=0;

// </editor-fold>

