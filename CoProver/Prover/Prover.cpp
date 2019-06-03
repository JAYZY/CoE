/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Prover.cpp
 * Author: Zhong Jian<77367632@qq.com>
 * 
 * Created on 2017年12月22日, 下午4:02
 */

#include "Prover.h"
#include "Formula/Formula.h"

Prover::Prover() {
    //实现步骤
    //一.预处理:1.格式转换;2.全部直接包含冗余和恒真冗余处理;3*.后续包括读入的公式集特征分析等
    //二.构建2个集合, 从第一个集合选取子句,与第二个集合子句进行归结,得到归结式
    //试图构建一个单元子句集合.

}

Prover::Prover(const Prover& orig) {
}

Prover::~Prover() {
}


