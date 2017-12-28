/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Environment.h
 * Author: Zhong Jian<77367632@qq.com>
 *
 * Created on 2017年12月23日, 下午2:24
 */

#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H
#include "IncDefine.h"
class Env {
public:
    static uint64_t global_clause_counter;
 
public:
    Env();
 
    virtual ~Env();
private:

};

#endif /* ENVIRONMENT_H */

