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
#include <cassert>
#include <list>
#include <vector>
#include <set>
//#include "Environment.h"
#include <sys/param.h>
#include "LIB/Out.h"

using namespace std;


typedef char* StreamType;
/* 定义函数符为一个　正整数　Function symbols in terms are represented by positive numbers,
   变元符号为一个　负整数　　variables by negative numbers.This alias allows clearer specifications. */
typedef long FunCode;
/*---------------------------------------------------------------------*/
/*                       【项顺序】相关枚举                              */
/*---------------------------------------------------------------------*/

/* 枚举 -- 比较结果类型枚举  */
enum class CompareResult : uint8_t {
    to_unknown = 0,
    to_uncomparable = 1,
    to_equal = 2,
    to_greater = 3,
    to_lesser = 4,
    to_notgteq, /* For caching partial LPO results */
    to_notleeq
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

#endif /* INCDEFINE_H */

