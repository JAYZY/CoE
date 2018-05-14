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
#include "Clause.h"
#include <list>

class Formula {
private:
    list<Clause*> axioms;
    Scanner* in;

public:
    Formula();
    Formula(const Formula& orig);
    virtual ~Formula();
private:
    //读取tptp公式集
    void WFormulaTPTPParse();
public:
    /*---------------------------------------------------------------------*/
    /*                       Inline  Function                              */

    /*---------------------------------------------------------------------*/
    inline list<Clause*>&getAxioms() {
        return axioms;
    }

    inline void ClauseSetInsert(Clause* cla) {
        assert(cla);
        axioms.push_back(cla);
        //暂时没有给定子句的评估函数
    }

    inline void printClas() {
        for (auto cla : axioms) {
            cla->ClausePrint(stdout, true);
            printf("\n");
        }

    }


};

#endif /* FORMULA_H */

