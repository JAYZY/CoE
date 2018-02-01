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
    //开始浸透算法
    proofCtr->Saturate();
    paseTime("Saturate_", initial_time);

    //ti.PrintTree();
    //    for (TermCell* term : tb->termStore.store) {
    //        if (term) {
    //            cout<<term->fCode<<":";
    //            cout<<Env::getSig()->fInfo[term->fCode]->name<<endl;
    //            //tb->TBPrintTerm(stdout,term,true);
    //            //term->TermPrint(stdout,DerefType::DEREF_ALWAYS);
    //            //cout << endl;                        
    //        }
    //    }
    //Env::getSig()->SigPrint(stdout);
    //tb->TBPrintBankInOrder(stdout);
    // cout<< ((int)FPPredSymbol | (int)FPInterpreted)<<endl;


    //
    //    StrTree_p s;
    //    SplayTree<StrTreeCell> root;
    //    int size=10000000;
    //    for (int i = 0; i < size; i++) {
    //        s = new StrTreeCell();
    //        s->key = to_string(i);
    //        s->val1.i_val=i;
    //        root.Insert(s);
    //    }
    //    printf("|  Parse time:           %12.2f s                                       |\n", cpuTime() - initial_time);
    //
    //    initial_time = cpuTime();
    //
    //    root.Find("26666");
    //    paseTime("sFind-Parse", initial_time);
    //
    //
    //    initial_time = cpuTime();
    //    map<string,int> r;
    //   
    //    for (int i = 0; i < size; i++) {
    //
    //        r[to_string(i)]=i;
    //    }
    //
    //    paseTime("Parse", initial_time);
    //
    //    initial_time = cpuTime();
    //    r.find("26666");
    //    paseTime("setF-Parse", initial_time);


    return 0;
}

