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
#include "Global/IncDefine.h"
//排序策略--被动文字稳定度 由低到高，由高到低

enum class PasLitSteady : uint8_t {
    NumASC = 1, //子句文字数升序 由少到多
    NumDesc, //子句文字数降序 有多到少

};


//排序策略--主动文字稳定度 由低到高，由高到低

enum class ActLitSteady : uint8_t {
    ASC = 1, //升序
    DESC, //降序
    MINIV, //近似值
};

//主界线文字限

enum class ALimit : uint8_t {
    NoLimit = 1, //无限制
    NoStructSameA, //不允许主界线文字 结构相同
    NoUnifySameA, //不允许主界线文字 合一相同
};

//子句选择策略

enum class ClaSelStrategy : uint8_t {
    /*
     * 0, "NS(子句冗余次数最少>子句文字数>子句稳定度>主动归结次数最少)"
     * 1, "SN(子句冗余次数最少>子句稳定度>子句文字数>主动归结次数最少)"
     */
    Num_Prio_Weight,
    Num_Weight_Prio,

    Prio_Weight_Num,
    Prio_Num_Weight,

    Weight_Num_Prio,
    Weight_Prio_Num,

    Num_Prio_Post_Weight,
    Num_Prio_Weight_Post

};

/* 枚举型输入格式*/
enum class InFormat {
    FOF,
    CNF
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

    // <editor-fold defaultstate="collapsed" desc="IO相关">
    static InFormat IFormat;

    // </editor-fold>

    // <editor-fold defaultstate="collapsed" desc="权重相关">
    //static int8_t ClaNoMGUWeight; //子句起步找不到归结,改变的优先级 (说明该子句不适合起步)
    static int8_t ClaRedundancyWeight; //子句归结中发现冗余,改变的优先级 
    static int8_t ClaAllOverFuncLayer; //子句中所有文字超过函数嵌套限制,改变的优先级

    static int8_t LitRedunancyWeight; //文字归结中发现冗余,改变的优先级 
    static int8_t LitOverFuncLayer; //文字超过函数嵌套限制,改变的优先级



    // </editor-fold>

    // <editor-fold defaultstate="collapsed" desc="启发式策略">
    static ClaSelStrategy CLAUSE_SEL_STRATEGY; //
    static PasLitSteady SEL_POSLIT_STEADY; //被动归结文字子句所在文字数策略
    static ALimit ALIT_LIMIT; //主界线文字限制
    
    
    static ActLitSteady Weight_Sort; //权重排序策略
    //  static ALimit SEL_POSLIT_STEADY;               //被动归结文字文字数策略
    static uint32_t IterCount_LIMIT; //create triangle count limit


    /* 剩余子句集中最大文字数限制-- 决定了△的继续延拓（思考：与扩展▲的区别在于此）   
     * 若子句Cj加入△后不满足该限制条件，则换一个Cj,若Ci找不到一个Cj满足该条件的Cj则回退一次。 
     */
    static uint32_t MaxLitNumOfR; //分离式R中最大文字数
    //left literals number during the process limit. =0 noLimit
    static uint32_t MaxLitsNumOfTriNewCla; //做△过程中生成新子句的文字数限制
    static uint32_t MaxFuncLayerOfR; //最大函数嵌套层限制-不超过256
    static uint32_t MaxLitNumOfNewCla; //新子句最大的文字数限制（literals number of new clause limit. =0 noLimit）    
    // </editor-fold>


    static bool isOutTPTP; //新子句是否输出TPTP 格式文件
    static bool isFullUC; //是否充分单元子句下拉

    //新的启发式策略
    // static INT32 SEL_STARTLit_STRATEGY; //起步文字选择策略
    // static INT32 SEL_STARTLIT_NUMLIT; //起步文字文字数策略
    // static INT32 SEL_STARTLIT_STEADY; //起步文字稳定度策略
    // static bool SEL_STARTLIT_R; //起步策略--归结式起步
    // static INT32 SEL_ACTLIT_STRATEGY; //主动归结文字选择策略
    // sstatic INT32 SEL_ACTLIT_STEADY; //主动归结文字文字数策略
    //static INT32 SEL_POSLIT_STRATEGY; //被动归结文字选择策略

    static bool ADD_EQULITY; //whether add equlity axiom
    static bool ADD_CR;

    static bool RuleALitsAllowEqual; //规则：主界线文字是否允许相同
    static bool RuleALitsAllowEqualR; //主界线文字与R文字相同

    static bool IS_RollBackGoalPath; //是否回退含有目标子句的路径
    static bool ISFactor; //是否执行 因子规则 factor rule

    static bool ISSplitUnitCalIndex; //单独分割单元子句索引


};

#endif /* STRATEGYPARAM_H */

