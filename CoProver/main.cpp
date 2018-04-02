/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.cpp
 * Author: Sammy Guergachi <sguergachi at gmail.com>
 *
 * Created on 2017年12月13日, 下午1:50
 */

#include "Global/IncDefine.h"
#include "INOUT/Scanner.h"
#include "CLAUSE/Clause.h"
#include "Global/Environment.h"
#include "BASIC/SplayTree.h"
#include "CLAUSE/Formula.h"
#include "PROOF/ProofControl.h"
#include <iostream>
#include <set>
#include <map>
using namespace std;

int main(int argc, char** argv) {

    //初始化全局扫描器Scanner
    Env::iniScanner(nullptr, argv[1], true, nullptr);

    //生成子句
    double initial_time = cpuTime();
    Formula* formula = new Formula();
    formula->printClas();

    paseTime("GenFormula_", initial_time);
    initial_time = cpuTime();
   
     ProofControl* proofCtr = new ProofControl(formula->getAxioms());
//    for(auto& cla:formula->getAxioms())
//    {
//        Literal* lit=cla->Lits();
//        cout<<"claID:"<<cla->GetClaId()<<" ";
//        while(lit){
//            lit->EqnTSTPPrint(stdout,true);
//            int x=lit->zjLitWeight();
//            fprintf(stdout," w:%2ld zjw:%d max:%ld\t",lit->StandardWeight(),x,lit->zjlitWight);
//            lit=lit->next;
//        }
//        fprintf(stdout,"\n");
//        
//    }
    //开始浸透算法
     proofCtr->Saturate();
    paseTime("Saturate_", initial_time);

    return 0;
}

