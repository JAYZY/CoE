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
InFormat StrategyParam::IFormat = InFormat::CNF;
// </editor-fold>


int8_t StrategyParam::ClaAllOverFuncLayer = -10; //子句中所有文字超过限制,改变的优先级
int8_t StrategyParam::ClaRedundancyWeight = -3; //子句归结中发现冗余,改变的优先级 

int8_t StrategyParam::LitRedunancyWeight = -2; //文字归结中发现冗余,改变的优先级 
int8_t StrategyParam::LitOverFuncLayer = -2; //文字超过限制,改变的优先级
//初始化参数==================================	
bool StrategyParam::IsAddRefleSymEquAxiom = true;
bool StrategyParam::ADD_EQULITY = true;
bool StrategyParam::ADD_CR = true;
bool StrategyParam::RuleALitsAllowEqualR = false;
bool StrategyParam::RuleALitsAllowEqual = false;
bool StrategyParam::ISFactor = true;
bool StrategyParam::IS_RollBackGoalPath = false;
///谓词符号是否分离单元子句和非单元子句的
bool StrategyParam::ISSplitUnitCalIndex=true;


ClaSelStrategy StrategyParam::CLAUSE_SEL_STRATEGY = ClaSelStrategy::Num_Prio_Weight; //Num_Weight_Prio;
PasLitSteady StrategyParam::SEL_POSLIT_STEADY = PasLitSteady::NumDesc;
ActLitSteady StrategyParam::Weight_Sort = ActLitSteady::DESC;


ALimit StrategyParam::ALIT_LIMIT = ALimit::NoLimit;
uint32_t StrategyParam::IterCount_LIMIT = UINT32_MAX;


uint32_t StrategyParam::MaxLitNumOfR = 2; //R 的最大文字数限制 决定了延拓进行的限制; △完成后，产生的新子句文字数限制
uint32_t StrategyParam::MaxFuncLayerOfR = 6;    //R 的最大函数嵌套层

uint32_t StrategyParam::MaxLitsNumOfTriNewCla =3; //△演绎过程中,剩余文字数小于该限制则允许生成演绎过程新子句;
uint32_t StrategyParam::MaxLitNumOfNewCla = 1; //完成△中，新子句加入到子句集的文字数限制



bool StrategyParam::isFullUC = true;
bool StrategyParam::isOutTPTP = false;

