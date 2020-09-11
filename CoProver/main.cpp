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
#include "Inferences/InferenceInfo.h"
#include "Global/Environment.h"
using namespace std;
#define ISFOF

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

    Options opt; //对命令行进行解析
    if (!opt.AnalyseOpt(argc, argv)) {
        exit((int) ErrorCodes::INPUT_PARAM_ERROR);
    }
    StrategyParam::isOutTPTP = true;
    //StrategyParam::CLAUSE_SEL_STRATEGY = ClaSelStrategy::Num_Prio_Post_Weight;
    StrategyParam::CLAUSE_SEL_STRATEGY = ClaSelStrategy::Num_Prio_Weight;
    cout << argv[0] << endl;
    //命令行解析
    char *path = NULL;
    path = getcwd(NULL, 0);
    puts(path);

    free(path);

    if (StrategyParam::IFormat == InFormat::FOF) {
        double startTime = CPUTime();
        string cmd = "timeout 3600  ./eprover2.1 " + Env::tptpFileName + " --cnf --output-file=" + FileOp::getInstance()->cnfFileName; //判断文件名称;
        int res4 = system(const_cast<char*> (cmd.c_str()));
        Env::tptpFileName = FileOp::getInstance()->cnfFileName;
        PaseTime("FOF2CNFByEprover_", startTime);
    }

    //全局扫描器Scanner读取文件 argv[1]
    Env::IniScanner(nullptr, const_cast<char*> (Env::tptpFileName.c_str()), true, nullptr);

    //生成公式集\子句-----------------
    Formula fol;
    fol.generateFormula(Env::getIn());
    PaseTime("GenFormula_");
   
    //输出原始子句 
    fol.printOrigalClaSet();

    vector<Clause*> factorClas;
    //预处理操作---------------------       FileOp::getInstance()->outInfo("\n#------ preProcessed Clauses ------\n");
    RESULT res = fol.preProcess(factorClas);

    //输出预处理后子句 
    // fol->printProcessedClaSet(stdout);
    if (res == RESULT::SUCCES) {

        //添加等词公理
        if (StrategyParam::ADD_EQULITY && fol.uEquLitNum > 0) {
            fol.GenerateEqulitAxiom();
            fol.printEqulityAxioms();
        }
        FileOp::getInstance()->outInfo("\n#------ New Clauses ------\n");
        //--- 输出factor .i 信息 --- 
        // <editor-fold defaultstate="collapsed" desc="輸出factor .i信息">

        for (int i = 0; i < factorClas.size(); i++) {
            Clause* newCla = factorClas[i];

            string strCla = "";
            newCla->getStrOfClause(strCla, false);
            string parentCla = "c" + to_string(*newCla->parentIds.begin());
            for_each(++newCla->parentIds.begin(), newCla->parentIds.end(), [&parentCla](uint32_t pClaId) {
                parentCla += ",c" + to_string(pClaId);
            });
            strCla += ",inference(" + InferenceInfo::getStrInfoType(InfereType::FACTOR) + ",[status(thm)],[" + parentCla + "])).\n";

            FileOp::getInstance()->outInfo(strCla);

        }
        // </editor-fold>

        vector<Clause*>().swap(factorClas); //factorClas.clear();
        //演绎推理
        Resolution resolution;
        //  res = resolution.BaseAlgByOnlyBinaryCla(fol);
        // if (res == RESULT::UNKNOWN) {
        //res = resolution.BaseAlg(&fol); //使用记录路径的方式进行路径回退
        res = resolution.BaseExtendAlg(&fol); //使用记录路径的方式进行路径回退
        //}
    }
    string strRes = "";
    if (RESULT::UNSAT == res) {
        strRes = "UNSAT # " + to_string(Env::GetTotalTime()) + " S";
        FileOp::getInstance()->GenrateEmptyPathNoRegex();
    } else {
        strRes = "UNKNOWN # " + to_string(Env::GetTotalTime()) + " S";
    }
    FileOp::getInstance()->outGlobalInfo(strRes);

    //    string strRes = ((RESULT.UNSAT == res) ? "UNSAT # " : "UNKNOWN # ") + to_string(Env::GetTotalTime()) + " S";
    //    FileOp::getInstance()->outGlobalInfo(strRes);
    //    if (100 == (int) res) {
    //        FileOp::getInstance()->GenrateEmptyPathNoRegex();
    //    }
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
    FileOp::getInstance()->CloseAll();
#ifdef  DELOUT
    FileOp::getInstance()->ClearOutDir();
#endif
    //删除临时输出文件

    return (int) res;
}
#endif
