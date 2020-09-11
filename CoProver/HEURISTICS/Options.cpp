/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Options.cpp
 * Author: Zhong Jian<77367632@qq.com>
 * 
 * Created on 2017年12月22日, 下午4:13
 */


#include <iostream>

#include "Options.h"
#include "StrategyParam.h"
#include "LIB/Out.h"
#include "INOUT/FileOp.h"
#include "Global/GlobalFunc.h"
uint32_t Options::step_limit = UINT32_MAX; //运行总次数

const OptCell Options::Opts[] = {

    { OptCodes::Help, "h", "help", OptArgType::NoArg, nullptr,
        "Print a short description of program usage and options."},

    { OptCodes::Version, "v", "version", OptArgType::NoArg, nullptr,
        "Print the version number of the prover. Please include this"
        " with all bug reports (if any)."},
    { OptCodes::Format, "f", "format", OptArgType::StrArg, "CNF", "input file's format"}

};
//int OptsLen = sizeof(Opts) / sizeof(Opts[0]);

bool Options::AnalyseOpt(int argc, char** argv) {
    if (argc < 2)
        return false;
    //命令行解析 coprover xxx  -f=fof --format=cnf    
    string fileFullName = argv[0];
    
    Env::ExePath = FileOp::FileNameDirName(fileFullName) + "/";
    ProgInfo::ProgName = FileOp::FileNameBaseName(fileFullName);
    Env::tptpFileName = argv[1];
    
    cout << "fileName:" << fileFullName << endl;
    cout << "Env::tptpFileName:" << Env::tptpFileName << endl;
    
    int ind = 2;
    bool isshort;
    int pos;
    while (ind < argc) {

        string str = argv[ind];
        string optName = "";
        string optValue = "";


        if ((pos = FindSubStr(str, "--"))>-1) {
            isshort = false;

        } else if ((pos = FindSubStr(str, '-'))>-1) {
            isshort = true;
        } else {
            continue;
        }

        int endPos = str.find_first_of("=", pos);
        if (endPos>-1) {
            optName = str.substr(pos + 1, endPos - pos - 1);
            optValue = str.substr(endPos + 1);
        } else {
            optName = optName = str.substr(pos + 1);
            optValue = "";
        }

        const OptCell* optcell = GetOption(optName, isshort);
        ModifyOpt(optcell, optValue);
        ++ind;
    }
    return true;
}

void Options::ModifyOpt(const OptCell* optcell, string& value) {
    switch (optcell->option_code) {
        case OptCodes::Version:
            fprintf(stdout, "CoProver 1.0\n");
            exit((int) ErrorCodes::NO_ERROR);
        case OptCodes::Help:
            fprintf(stdout, optcell->desc);
            exit((int) ErrorCodes::NO_ERROR);
            break;
        case OptCodes::Format:
            transform(value.begin(), value.end(), value.begin(), ::toupper);
            StrategyParam::IFormat = value == "FOF" ? InFormat::FOF : InFormat::CNF;
            break;
    }
}

Options::Options() {


}

Options::Options(const Options& orig) {
}

Options::~Options() {
}

int Options::GetOptSize() {
    return sizeof (Opts) / sizeof (Opts[0]);
}