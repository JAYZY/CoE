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
#include "Formula/Formula.h"
#include "PROOF/ProofControl.h"
#include <iostream>
#include <set>
#include <map>
using namespace std;
//#define TEST

#ifdef  TEST

int main(int argc, char** argv) {
    int size = 1000000000;
    int olds = size / 3;
    int *x;
    x = new int[size];
    double initial_time = cpuTime();
    memset(x, 0, size * sizeof (int));

    paseTime("allSet_", initial_time);


    initial_time = cpuTime();
    memset(x, 0, olds * sizeof (int));

    paseTime("allSet_", initial_time);
    DelArrayPtr(x);
    return 0;
}

#else

int main(int argc, char** argv) {
    cout << "argc:" << argc << endl;
    for (int i = 0; i < argc; ++i) {
        cout << argv[i] << endl;
    }
    //命令行解析

    //全局扫描器Scanner读取文件 argv[1]
    Env::IniScanner(nullptr, argv[1], true, nullptr);

    //生成公式集\子句-----------------
    double initial_time = CPUTime();
    Formula* fol = new Formula();
    fol->GenerateFormula(Env::getIn());
    PaseTime("GenFormula_", initial_time);

    //预处理操作---------------------       
    ProofControl::Preprocess(fol);

    //ProofControl* proofCtr = new ProofControl(fol->getAxioms());

    //开始浸透算法
    //proofCtr->Saturate();
    //PaseTime("Saturate_", initial_time);

    return 0;
}


#endif
