/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Formula.cpp
 * Author: Zhong Jian<77367632@qq.com>
 * 
 * Created on 2017年12月22日, 下午6:03
 */

#include "Formula.h"


Formula::Formula(Scanner *in_):in(in_) {
    while(in->TestInpId("input_formula|input_clause|fof|cnf|tff|include"))    {
        if(in->TestInpId("include")){//对include进行读取和解析
            
        }
            
    }
}

Formula::Formula(const Formula& orig) {
}

Formula::~Formula() {
}

