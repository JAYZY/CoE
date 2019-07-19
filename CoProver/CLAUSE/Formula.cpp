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
#include "Global/Environment.h"

Formula::Formula() {
    in = Env::getIn();
//    long res = 0;
//    if (in->format != IOFormat::LOPFormat) {
//
//        //TPTP
//        while (in->TestInpId("input_formula|input_clause|fof|cnf|include")) {
//            if (in->TestInpId("include")) {//读取include 文件
//
//            }
//            assert(in->TestInpId("input_clause|cnf"));
//
//           // Clause* clause = Clause::ClauseParse();
//
//           // ClauseSetInsert(clause);
//
//            // axioms->ClauseSetInsert(clause);
//        }
//
//    }
//    while (in->TestInpId("input_formula|input_clause|fof|cnf|tff|include")) {
//        if (in->TestInpId("include")) {//对include进行读取和解析
//
//        }
//
//    }
}

Formula::Formula(const Formula& orig) {

}

Formula::~Formula() {
    axioms.clear();
}

