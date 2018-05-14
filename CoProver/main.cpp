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
    cout << "argc:" << argc << endl;
    for (int i = 0; i < argc; ++i) {
        cout << argv[i] << endl;
    }
    //命令行解析

    //初始化全局扫描器Scanner
    Env::iniScanner(nullptr, argv[1], true, nullptr);

    //生成子句
    double initial_time = cpuTime();

    FormulaSet* formulaSet = new FormulaSet();
    ClauseSet* claSet = new ClauseSet();
    SplayTree<StrTreeCell>name_selector, skip_includes;

    long formulaNum = formulaSet->FormulaAndClauseSetParse(Env::getIn(), claSet, name_selector, skip_includes);
    //formula->printClas();
    claSet->Sort();
    paseTime("GenFormula_", initial_time);
    initial_time = cpuTime();


    ProofControl* proofCtr = new ProofControl(claSet);


    //               for(auto& cla:formulaSet->getAxioms())
    //                {
    //                    Literal* lit=cla->Lits();
    //                    cout<<"claID "<a<cla->GetClaId()<<":"<<endl;
    //                    while(lit){
    //                        lit->EqnTSTPPrint(stdout,true);                
    //                        fprintf(stdout," w:%2ld zjw:%5.2f\n",lit->StandardWeight(),lit->zjlitWight);
    //                        lit=lit->next;
    //                    }
    //                    fprintf(stdout,"\n");            
    //                }
    //开始浸透算法
    proofCtr->Saturate();
    paseTime("Saturate_", initial_time);

    return 0;
}

