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

// <editor-fold defaultstate="collapsed" desc="IO相关">
InFormat StrategyParam::IFormat=InFormat::CNF;
// </editor-fold>


uint8_t StrategyParam::CLA_NOMGU_WIGHT = 3; //子句起步找不到归结,改变的优先级 (说明该子句不适合起步)
uint8_t StrategyParam::CLA_REDUNDANCY_WIGHT = 2; //子句归结中发现冗余,改变的优先级 
uint8_t StrategyParam::CLA_OVERLIMIT_WIGHT = 1; //子句超过限制,改变的优先级

uint8_t StrategyParam::LIT_REDUNDANCY_WIGHT = 2; //文字归结中发现冗余,改变的优先级 (说明该文字不适合起步)
uint8_t StrategyParam::LIT_OVERLIMIT_WIGHT = 2; //文字超过限制,改变的优先级
//初始化参数==================================	

bool StrategyParam::ADD_EQULITY = true;
bool StrategyParam::ADD_CR = true;
bool StrategyParam::IS_ALitEqualR = false;
bool StrategyParam::IS_ALitNoEqual = false;
bool StrategyParam::ISFactor = true;
bool StrategyParam::IS_RollBackGoalPath=false;

ClaSelStrategy StrategyParam::CLAUSE_SEL_STRATEGY = ClaSelStrategy::Num_Prio_Weight; //Num_Weight_Prio;
POSLIT_STEADY StrategyParam::SEL_POSLIT_STEADY = POSLIT_STEADY::NumDesc;
SORT_STRATEGY StrategyParam::Weight_Sort=SORT_STRATEGY::DESC;


ALimit StrategyParam::ALIT_LIMIT = ALimit::NoLimit;
int32_t StrategyParam::IterCount_LIMIT = INT_MAX;


uint32_t StrategyParam::MaxLitNumOfR = 5; //R 的最大文字数限制 △完成后，产生的新子句文字数限制
uint32_t StrategyParam::MaxLitsNumOfTriNewCla = 1; //△过程中生成新最大文字数限制
uint32_t StrategyParam::R_MAX_FUNCLAYER = 3;
uint32_t StrategyParam::MaxLitNumOfNewCla = 1; //完成△中，新子句加入到子句集的文字数限制



bool StrategyParam::isFullUC = true;
bool StrategyParam::isOutTPTP=false;

