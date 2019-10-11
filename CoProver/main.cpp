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
#include "Global/GlobalFunc.h"
#include "Formula/Formula.h"
#include "PROOF/ProofControl.h"
#include "Alg/Resolution.h"
#include "INOUT/FileOp.h"
using namespace std;
//#define TEST

#ifdef  TEST

int main(int argc, char** argv) {
    //    int size = 1000000000;
    //    int olds = size / 3;
    //    int *x;
    //    x = new int[size];
    //    double initial_time = cpuTime();
    //    memset(x, 0, size * sizeof (int));
    //
    //    paseTime("allSet_", initial_time);
    //
    //
    //    initial_time = cpuTime()    memset(x, 0, olds * sizeof (int));
    //
    //    paseTime("allSet_", initial_time);
    //    DelArrayPtr(x);


     FileOp::GenrateEmptyPathNoRegex();
    return 0;
}

#else

int main(int argc, char** argv) {


    bool isCNF = true;
    //命令行解析
    Env::tptpFileName = argv[1];

    if (!isCNF) {
        string cmd = "timeout 3600  ./eprover2.1 " + Env::tptpFileName + +" --cnf --output-file=" + FileOp::getInstance()->cnfFileName; //判断文件名称;
        int res4 = system(const_cast<char*> (cmd.c_str()));
        Env::tptpFileName = FileOp::getInstance()->cnfFileName;
    }
    //全局扫描器Scanner读取文件 argv[1]
    Env::IniScanner(nullptr, const_cast<char*> (Env::tptpFileName.c_str()), true, nullptr);

    //生成公式集\子句-----------------
    Formula fol;
    fol.generateFormula(Env::getIn());
    PaseTime("GenFormula_");

    //输出原始子句 
    fol.printOrigalClaSet();
    FileOp::getInstance()->outInfo("\n#------ New Clauses ------\n");
    //预处理操作---------------------       
    RESULT res = fol.preProcess();
    //添加等词公理
    if (StrategyParam::ADD_EQULITY && fol.uEquLitNum > 0) {
        fol.GenerateEqulitAxiom();
        fol.printEqulityAxioms();
    }

    //输出预处理后子句 
    // fol->printProcessedClaSet(stdout);
    if (res == RESULT::SUCCES) {

        //演绎推理
        Resolution resolution;
        //  res = resolution.BaseAlgByOnlyBinaryCla(fol);
        // if (res == RESULT::UNKNOWN) {
        res = resolution.BaseAlg(&fol); //使用记录路径的方式进行路径回退
        // res = resolution.BaseExtendAlg(&fol); //使用记录路径的方式进行路径回退
        //}
    }
    string strRes = ((100 == (int) res) ? "UNSAT # " : "UNKNOWN # ") + to_string(Env::GetTotalTime()) + " S";
    FileOp::getInstance()->outGlobalInfo(strRes);
    if (100 == (int) res) {
        FileOp::getInstance()->GenrateEmptyPathNoRegex();
    }
    //fol->printClas(stdout);
    //Env::getGTbank()->GTPrintAllTerm(stdout);
    //cout << "每个子句的共享项==================" << endl;

    //ProofControl* proofCtr = new ProofControl(fol->getAxioms());

    //开始浸透算法
    //proofCtr->Saturate();
    //PaseTime("Saturate_", initial_time);
    // char sRusage[10000];
    //Env::PrintRusage(sRusage);
    //cout << sRusage << endl;

    // Env::PrintRunInfo(stdout);
    return (int) res;
}
#endif
