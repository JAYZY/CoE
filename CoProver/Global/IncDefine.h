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
#include "LIB/Out.h"


 
typedef  char* StreamType;

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


#endif /* INCDEFINE_H */

