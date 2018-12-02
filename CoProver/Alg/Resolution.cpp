/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Resolution.cpp
 * Author: Zhong Jian<77367632@qq.com>
 * 
 * Created on 2018年11月23日, 下午3:52
 */

#include <bits/algorithmfwd.h>
#include <stdbool.h>

#include "Resolution.h"
#include "TriAlg.h"

Resolution::Resolution() {
}

Resolution::Resolution(const Resolution& orig) {
}

Resolution::~Resolution() {
}

RESULT Resolution::BaseAlgByRecodePath(Formula* fol) {
    //从目标子句集中的子句起步，目前策略基于目标的归结
    /*目标子句集, 既包含单元子句,和多文字子句.选取的方式,    
     * 1.单元子句因为都放在 前面的,因此选择起步子句时候不选择单元目标子句起步.
     * 2.若原始子句集中只有单元目标子句,则从单元子句集合中选择
     */
    //排序 文字由多到少.
    std::sort(fol->goalClaset.begin(), fol->goalClaset.end(), [](Clause* c1, Clause * c2)->bool {
        return c1->LitsNumber() > c2->LitsNumber();
    });
    RESULT res = RESULT::UNKNOWN;
    TriAlg triAlg(fol);
    for (Clause* gCla : fol->goalClaset) {
      //  gCla->ClausePrint(stdout,true);
        res = triAlg.GenerateTriByRecodePath(gCla);
        if (res == RESULT::UNSAT){
            return res;
        }
    }

    cout << (int) res << endl;
    return res;
}