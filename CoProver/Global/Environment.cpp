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

uint32_t Env::global_formula_counter = 0u;
uint32_t Env::global_clause_counter = 0u; //全局子句个数,从1开始编号
uint32_t Env::backword_CMP_counter = 0u;
uint32_t Env::backword_Finded_counter = 0u;
uint32_t Env::forward_Finded_counter = 0u;


IOFormat Env::parseFormat = IOFormat::AutoFormat;
Scanner* Env::in = nullptr;
GTermBank_p Env::GTBank = nullptr;
Sigcell* Env::sig = nullptr;

Env::Env() {
}

Env::~Env() {
}

