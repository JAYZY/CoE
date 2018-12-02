/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   StrategyParam.h
 * Author: Zhong Jian<77367632@qq.com>
 *
 * Created on 2018年11月8日, 下午12:07
 */

#ifndef STRATEGYPARAM_H
#define STRATEGYPARAM_H

#include <cstdint>

enum class POSLIT_STEADY : uint8_t {
    NumASC = 1, //子句文字数升序 由少到多
    NumDesc, //子句文字数降序 有多到少

};
//主界线文字限制

enum class ALimit : uint8_t {
    NoLimit = 1, //无限制
    NoStructSameA, //不允许主界线文字 结构相同
    NoUnifySameA, //不允许主界线文字 合一相同

};

class StrategyParam {
public:
    StrategyParam();
    StrategyParam(const StrategyParam& orig);
    virtual ~StrategyParam();
    /*---------------------------------------------------------------------*/
    /*                          Static  Param                              */
    /*---------------------------------------------------------------------*/
    //
    //新的启发式策略
    // static INT32 SEL_STARTLit_STRATEGY; //起步文字选择策略
    // static INT32 SEL_STARTLIT_NUMLIT; //起步文字文字数策略
    // static INT32 SEL_STARTLIT_STEADY; //起步文字稳定度策略
    // static bool SEL_STARTLIT_R; //起步策略--归结式起步
    // static INT32 SEL_ACTLIT_STRATEGY; //主动归结文字选择策略
    // sstatic INT32 SEL_ACTLIT_STEADY; //主动归结文字文字数策略
    //static INT32 SEL_POSLIT_STRATEGY; //被动归结文字选择策略
    static POSLIT_STEADY SEL_POSLIT_STEADY; //被动归结文字子句所在文字数策略
    static ALimit ALIT_LIMIT; //主界线文字限制
    //static INT32 SEL_POSLIT_STEADY; //被动归结文字文字数策略
};

#endif /* STRATEGYPARAM_H */

