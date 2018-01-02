/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Environment.cpp
 * Author: Zhong Jian<77367632@qq.com>
 * 
 * Created on 2017年12月23日, 下午2:24
 */

#include "Environment.h"

/*---------------------------------------------------------------------*/
/*                          Static Function                            */
/*---------------------------------------------------------------------*/
uint64_t Env::global_clause_counter = 0; //全局子句个数,从1开始编号
IOFormat Env::parseFormat = IOFormat::AutoFormat;
Scanner* Env::in=nullptr;
TermBank* Env::termBank=nullptr;

Env::Env() {
}

Env::~Env() {
}

