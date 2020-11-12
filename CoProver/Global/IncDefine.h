/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   IncDefine.h
 * Author: Zhong Jian<77367632@qq.com>
 *
 * Created on 2017年12月14日, 上午8:22
 */

#ifndef INCDEFINE_H
#define INCDEFINE_H
#include <cstdint>
#include <cstdio>
#include <climits>
#include <string>
#include <cstring>
#include <algorithm>
#include <unistd.h>
#include <list>
#include <vector>
#include <set>
#include <cmath>
//#include "Environment.h"
#include <sys/param.h>
#include <sys/wait.h>

//#define DELOUT
#define NDEBUG

#include <cassert>
using namespace std;
#define New
//#define  OUTINFO
#define WEI 0.0f

#define MAX_ERRMSG_ADD   512
#define MAX_ERRMSG_LEN   MAX_ERRMSG_ADD+MAXPATHLEN


extern const uint32_t MAX_SUBTERM_SIZE; //预设最大子项个数,用于优化 vector 2^n
extern const uint32_t MAX_VAR_SIZE; //预设项中,最大变元数

typedef char* StreamType;
/* 定义函数符为一个　正整数　Function symbols in terms are represented by positive numbers,
   变元符号为一个　负整数　　variables by negative numbers.This alias allows clearer specifications. */
typedef long FunCode;
/*---------------------------------------------------------------------*/
/*                       【项顺序】相关枚举                              */
/*---------------------------------------------------------------------*/

/* 枚举 -- 比较结果类型枚举  */
enum class CompareResult : uint8_t {
    toUnknown = 0,
    toUncomparable = 1,
    toEqual = 2,
    toGreater = 3,
    toLesser = 4,
    toNotgteq, /* For caching partial LPO results */
    toNotleeq
};

enum class InfereType : uint8_t {
    NONE, //原始子句
    BI, // BinerayInference
    UD, 
    RN, //rename
    RD, //REDUCT 合一下拉
    FACTOR,//因子归结
    SCSA,//演绎过程生成的子句(单元)
    SCS//矛盾体分离 


};

enum class ErrorCodes {
    NO_ERROR = 0,
    PROOF_FOUND = 0,
    SATISFIABLE = 1,
    OUT_OF_MEMORY = 2,
    SYNTAX_ERROR = 3,
    USAGE_ERROR = 4,
    FILE_ERROR = 5,
    SYS_ERROR = 6,
    CPU_LIMIT_ERROR = 7,
    RESOURCE_OUT = 8,
    INCOMPLETE_PROOFSTATE = 9,
    OTHER_ERROR = 10,
    INPUT_SEMANTIC_ERROR = 11,
    INPUT_PARAM_ERROR = 12

};

enum class ResRule : uint8_t {
    Unknown,

    ALitSameRLit, /*与前面剩余文字相同(A文字与B文字相同)*/
    ALItPairRLit, /*与前面剩余文字直接互补(A文字与B文字相同)*/
    ASameOrPairRLit, /*与前面剩余文字相同或互补(A文字与B文字相同)*/
   
    ALitSameALits, /*主界线文字相同(A文字集中有相同文字)*/
    ALitPairALits, /*主界线文字直接互补(A文字集中有直接互补文字)*/
    ASameOrPairALit, /*主界线文字相同或直接互补*/

    DelHoldLit, /*剩余文字被删除*/
    ChgPasLit, //换被归结文字
    ChgActLit, //换主界线文字(主动归结文字)    
    MoreFunclayer/*函数复合层过多*/, MoreLit/*剩余文字过多*/, NoLeftLit/*没有剩余文字*/, SingleLit, TAUTOLOGY/*R为恒真*/,
    EqnTautology, RSubsump/*R包含冗余*/, RULEOK/*规则检查通过*/
};



/*---------------------------------------------------------------------*/
/*                       【全局返回类型】相关枚举                      */
/*---------------------------------------------------------------------*/
//枚举定义返回类型
//返回：100 不可满足;  101 可满足;  102 不可判定;103 无子句参与归结 200 错误起步ID; 201创建网络连接错误;  202 后台输出目录错误; 203 程序调用错误,
//-101 文件格式错误

enum class RESULT {
    READERR = -101, READOK = -100, NO_ERROR = -1, NOCLAUSE = 0, UNSAT = 100, SAT = 101, UNKNOWN = 102, OverLimit, MoreLit, MoreFuncLayer, Factor,
    ERR_STARTID = 200, ERR_NET, ERR_OUTFOLDER, ERR_INVOKE, OUT_OF_MEMORY = 204, CPU_LIMIT_ERROR, SYS_ERROR, UnknownFile,

    NOMGU/*没有合一*/, SUCCES, NOLits/*没有文字*/, RSubsump, RollBack, FAIL
};

/*---------------------------------------------------------------------*/
/*                          宏定义-两个函数指针                           */
/*---------------------------------------------------------------------*/
typedef void (*ObjDelFun)(void *junk);

/* 注意这里是一个　联合体．要么存储数字，要么存储指针．
 * Trick the stupid type concept for polymorphic indices (hashes,trees) with int/pointer type. */
typedef union int_or_p {
    long i_val;
    void *p_val;
} IntOrP;

/* 指针删除模板 -- 确保每次 delete 后都要==nullptr*/
template<typename T> inline void DelPtr(T*&p) {
    if (p) {
        delete p;
        p = nullptr;
    }
}

template<typename T> inline void DelArrayPtr(T*&p) {
    delete[] p;
    p = nullptr;
}

template <class T>
int GetArrayLen(T& array) {
    //使用模板定义一 个函数getArrayLen,该函数将返回数组array的长度
    return (sizeof (array) / sizeof (array[0]));

}
/*---------------------------------------------------------------------
 * Macros for dealing with 1 bit properties of objects (well,structs). 
 * It requires the object to be dealt with to have a field named "properties" 
 * that is of some integer or enumeration type. This is pretty ugly, 
 * but I did not want to spend to much time on it.
 *---------------------------------------------------------------------*/

//#define SetProp(obj, prop) ((obj)->properties = (obj)->properties | (prop))

template<typename T, typename V>
inline void SetProp(T&obj, V prop) {
    obj = T((int32_t) obj | (int32_t) prop);
}

template<typename T, typename V>
inline void DelProp(T&obj, V prop) {
    obj = T((int32_t) obj & (~((int32_t) prop)));
}

template<typename T, typename V>
inline void AssignProp(T&obj, V sel, V prop) {
    DelProp(obj, sel);
    SetProp(obj, (int32_t) sel & (int32_t) prop);
}

template<typename T, typename V>
inline void FlipProp(T&obj, V prop) {
    obj = T((int32_t) obj ^ (int32_t) prop);
}

template<typename T, typename V>
inline T GiveProps(T&obj, V prop) {
    return T((int32_t) obj & (int32_t) prop);
}

template<typename T, typename V>
inline bool QueryProp(T&obj, V prop) {
    return ((int32_t) obj & (int32_t) prop) == (int32_t) prop;
}
//#define IsAnyPropSet(obj, prop) ((obj) & (prop))

template<typename T, typename V>
inline bool IsAnyPropSet(T&obj, V prop) {
    return ((int32_t) obj & (int32_t) prop) == (int32_t) prop;
}
//判断两个对象是否有相同的属性prop

template<typename T, typename V>
inline bool PropsAreEquiv(T&obj1, T&obj2, V prop) {
    return ((int32_t) obj1 & (int32_t) prop) == ((int32_t) obj2 & (int32_t) prop);
}

#define ASSERT_MSG(expr,msg) (void)((expr)?((void)0):assert_msg(msg))

#undef ABS
#define ABS(x) ((x)>0?(x):-(x))

#undef XOR
#define XOR(x,y) (!(x)!=!(y))

#undef EQUIV
#define EQUIV(x,y) (!(x)==!(y))

#undef SWAP
#define SWAP(x,y) do{ __typeof__ (x) tmp =(x); (x)=(y); (y)=(tmp);}while(0)


#define LIKELY(x) __builtin_expect(!!(x), 1)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)

#define UNUSED(x) (void)(x)

#define KILO 1024
#define MEGA (1024*1024)

/*****************************************************************************
 * 定义系统时间宏定义
 ****************************************************************************/
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>

//宏定义开关--定义cpu时间输出
#ifdef DEF_OUT_CPUTIME

#define CPUTIME_DEFINE(name)  long long name = 0; long long name##_store = 0
#define CPUTIME_DECL(name)    extern long long name; extern long long name##_store
#define CPUTIME_RESET(name)   name = 0
#define CPUTIME_ENTRY(name)   name##_store = CPUTime()
#define CPUTIME_EXIT(name)    name+=(CPUTime()-(name##_store));name##_store=0
#define CPUTIME_PRINT(out, name) fprintf((out), "# PC%-34s : %f\n", "(" #name ")", ((float)name)/1000000.0)
#else
#define CPUTIME_DEFINE(name)  enum { name##_store } // Used to silence compiler warnings
#define CPUTIME_DECL(name)    enum { name }         // about extra semicolons.
#define CPUTIME_RESET(name)
#define CPUTIME_ENTRY(name)
#define CPUTIME_EXIT(name)
#define CPUTIME_PRINT(out, name)
#endif


/* 四种时间精度
 * clock()函数的精确度是10毫秒(ms)；
 * times()函数的精确度是10毫秒(ms)；
 * gettimofday()函数的精确度是微秒(μs)；
 * getrusage()函数的精确度是纳秒(ns)。
 */
static inline double CPUTime(void);
static inline double GetTotalCPUTime(void);

static inline double CPUTime(void) {
    struct rusage ru;
    getrusage(RUSAGE_SELF, &ru);
    return (double) ru.ru_utime.tv_sec + (double) ru.ru_utime.tv_usec / 1000000;
}

static inline void PaseTime(const char* tip, double initial_time) {
    printf("|  %stime:           %12.2f s                 |\n", tip, CPUTime() - initial_time);
}

static inline void PaseTime(const char* tip) {
    printf("|  %stime:           %12.2f ms                 |\n", tip, GetTotalCPUTime());
}

static inline long long GetUSecTime(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (long long) tv.tv_sec * 1000000ll + tv.tv_usec;
}

static inline long long GetUSecClock(void) {
    long long res = (clock()*1000000ll) / CLOCKS_PER_SEC;
    return res;
}

/*--------------------------------------------------------------------------
/* 得到程序运行时间
/-------------------------------------------------------------------------*/
static inline double GetTotalCPUTime(void) {
    double res = -1;
    struct rusage usage;
    if (!getrusage(RUSAGE_SELF, &usage)) {
        res = (usage.ru_utime.tv_sec + usage.ru_stime.tv_sec)+
                ((usage.ru_utime.tv_usec + usage.ru_stime.tv_usec) / 1000000.0);
    }
    return res;
}







#endif /* INCDEFINE_H */

