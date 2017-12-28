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
#include <iostream>
using namespace std;

int main(int argc, char** argv) {

    IOFormat parseFormat = IOFormat::AutoFormat;

    Scanner* in = new Scanner(nullptr, argv[1], true, nullptr);
    in->ScannerSetFormat(parseFormat);
    if (parseFormat == IOFormat::AutoFormat && in->format == IOFormat::TSTPFormat) {
        Options::OutputFormat = IOFormat::TSTPFormat;                
    }
    Clause* cla=new Clause(new Literal());
    cout<<cla->getWeight()<<endl;
    
    int a[]={1,2,3};
    cout<< (((intptr_t)a))<<endl;
    cout<< (((intptr_t)a)>>4)<<endl;
    
    //生成子句
    
    return 0;
}

