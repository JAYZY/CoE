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
#include <iostream>
#include <set>
using namespace std;

int main(int argc, char** argv) {




    Env::iniScanner(nullptr, argv[1], true, nullptr);
    


    //生成子句
    double initial_time = cpuTime();





    StrTree_p s;
    SplayTree<StrTreeCell> root;
    for (int i = 0; i < 10000000; i++) {
        s = new StrTreeCell();
        s->key = to_string(i);
        root.Insert(s);
    }
    printf("|  Parse time:           %12.2f s                                       |\n", cpuTime() - initial_time);

    initial_time = cpuTime();

    root.Find("26666");
    paseTime("sFind-Parse", initial_time);


    initial_time = cpuTime();
    set<string> r;
    for (int i = 0; i < 10000000; i++) {

        r.insert(to_string(i));
    }

    paseTime("Parse", initial_time);

    initial_time = cpuTime();
    r.find("26666");
    paseTime("setF-Parse", initial_time);


    return 0;
}

