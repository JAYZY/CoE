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

#include "Formula/Formula.h"
#include "PROOF/ProofControl.h"
#include "Alg/Resolution.h"
#include "INOUT/FileOp.h"
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
//
#else

int main(int argc, char** argv) {

    //命令行解析
    Env::tptpFileName = argv[1];

    //全局扫描器Scanner读取文件 argv[1]
    Env::IniScanner(nullptr, argv[1], true, nullptr);

    //生成公式集\子句-----------------
    Formula* fol = new Formula();
    fol->generateFormula(Env::getIn());
    PaseTime("GenFormula_");

    //输出原始子句 
    fol->printOrigalClaSet();

    //预处理操作---------------------       
    RESULT res = fol->preProcess();
     //添加等词公理
    if (StrategyParam::ADD_EQULITY&& fol->uEquLitNum>0) {
        fol->GenerateEqulitAxiom();
        fol->printEqulityAxioms();
    }
     FileOp::getInstance()->outInfo("\n#------ New Clauses ------\n");
    //输出预处理后子句 
   // fol->printProcessedClaSet(stdout);
    if (res == RESULT::SUCCES) {
        //演绎推理
        Resolution resolution;
      //  res = resolution.BaseAlgByOnlyBinaryCla(fol);
       // if (res == RESULT::UNKNOWN) {
            res = resolution.BaseAlg(fol); //使用记录路径的方式进行路径回退
        //}
    }

    FileOp::getInstance()->outGlobalInfo(" #Res:" + (100 == (int) res) ? "UNSAT" : "UNKNOWN");

    //fol->printClas(stdout);
    //Env::getGTbank()->GTPrintAllTerm(stdout);
    //cout << "每个子句的共享项==================" << endl;

    //ProofControl* proofCtr = new ProofControl(fol->getAxioms());

    //开始浸透算法
    //proofCtr->Saturate();
    //PaseTime("Saturate_", initial_time);
    Env::PrintRusage(stdout);

    Env::PrintRunInfo(stdout);
    return (int) res;
}
#endif
