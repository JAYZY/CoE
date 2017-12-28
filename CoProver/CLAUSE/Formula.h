/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Formula.h
 * Author: Zhong Jian<77367632@qq.com>
 *
 * Created on 2017年12月22日, 下午6:03
 */

#ifndef FORMULA_H
#define FORMULA_H
#include "INOUT/Scanner.h"
class Formula {
private:
    Scanner* in;
public:
    Formula(Scanner *_in);
    Formula(const Formula& orig);
    virtual ~Formula();
private:
    //读取tptp公式集
    void WFormulaTPTPParse();

};

#endif /* FORMULA_H */

