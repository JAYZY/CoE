/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Environment.cpp
 * Author: Zhong Jian<77367632@qq.com>
 * 
 * Created on 2017年12月23日, 下午2:24
 */

#include "Environment.h"
#include "LIB/Out.h"
/*---------------------------------------------------------------------*/
/*                          Static Function                            */
/*---------------------------------------------------------------------*/
string Env::tptpFileName = ""; //判定文件名
string Env::ExePath="";//程序所在目录绝对地址
//string Env::ProgName="Coprover";//程序 

uint32_t Env::global_formula_counter = 0u;
uint32_t Env::global_clause_counter = 0u; //全局子句个数,从1开始编号
uint32_t Env::backword_CMP_counter = 0u;
uint32_t Env::backword_Finded_counter = 0u;
uint32_t Env::forward_Finded_counter = 0u;

uint16_t Env::S_OverMaxLitLimit_Num = 0u;
uint16_t Env::S_ASame2R_Num = 0u; //主界线文字与前面R文字相同次数
uint16_t Env::S_ASame2A_Num = 0u; //主界线文字相同次数


IOFormat Env::parseFormat = IOFormat::TSTPFormat;
Scanner* Env::in = nullptr;
GTermBank_p Env::GTBank = nullptr;
Sigcell* Env::sig = nullptr;

Env::Env() {
}

Env::~Env() {
}

double Env::GetTotalTime() {
    struct rusage usage, cusage;

    if (getrusage(RUSAGE_SELF, &usage)) {
        TmpErrno = errno;
        Out::SysError("Unable to get resource usage information", ErrorCodes::SYS_ERROR);
    }
    if (getrusage(RUSAGE_CHILDREN, &cusage)) {
        TmpErrno = errno;
        Out::SysError("Unable to get resource usage information", ErrorCodes::SYS_ERROR);
    }
    usage.ru_utime.tv_sec += cusage.ru_utime.tv_sec;
    usage.ru_utime.tv_usec += cusage.ru_utime.tv_usec;
    usage.ru_stime.tv_sec += cusage.ru_stime.tv_sec;
    usage.ru_stime.tv_usec += cusage.ru_stime.tv_usec;

    double spanTime = (usage.ru_utime.tv_sec + usage.ru_stime.tv_sec)+
            ((usage.ru_utime.tv_usec + usage.ru_stime.tv_usec) / 1000000.0);
    return spanTime;
}

void Env::PrintRusage(char* sRusage) {
    struct rusage usage, cusage;

    if (getrusage(RUSAGE_SELF, &usage)) {
        TmpErrno = errno;
        Out::SysError("Unable to get resource usage information", ErrorCodes::SYS_ERROR);
    }
    if (getrusage(RUSAGE_CHILDREN, &cusage)) {
        TmpErrno = errno;
        Out::SysError("Unable to get resource usage information", ErrorCodes::SYS_ERROR);
    }
    usage.ru_utime.tv_sec += cusage.ru_utime.tv_sec;
    usage.ru_utime.tv_usec += cusage.ru_utime.tv_usec;
    usage.ru_stime.tv_sec += cusage.ru_stime.tv_sec;
    usage.ru_stime.tv_usec += cusage.ru_stime.tv_usec;

    // outStr = "\n# -------------------------------------------------\n";
    //        outStr+="# Maximum ClauseID         : "+to_string(Env::global_clause_counter)+"\n";
    //        outStr+="# User time                : "+to_string((usage.ru_utime.tv_sec)+(usage.ru_utime.tv_usec) / 1000000.0)+" s\n";
    //        outStr+="# System time              : "+to_string((usage.ru_stime.tv_sec)+(usage.ru_stime.tv_usec) / 1000000.0)+" s\n"
    string sOut =
            "\n%% -------------------------------------------------\n%% SZS output end Proof.\n";
    sOut += "%% User time                   : %.3f s\n";
    sOut += "%% System time                 : %.3f s\n";
    sOut += "%% Total time                  : %.3f s\n";
    sOut += "%% Maximum resident set size   : %ld pages\n";
    sOut += "%% Maximum ClauseID            : %d \n";
    float userTime = (usage.ru_utime.tv_sec)+(usage.ru_utime.tv_usec) / 1000000.0f;
    float sysTime = (usage.ru_stime.tv_sec)+(usage.ru_stime.tv_usec) / 1000000.0f;
    float totalTime = (usage.ru_utime.tv_sec + usage.ru_stime.tv_sec)+((usage.ru_utime.tv_usec + usage.ru_stime.tv_usec) / 1000000.0f);

    sprintf(sRusage, sOut.c_str(), userTime, sysTime, totalTime, usage.ru_maxrss, Env::global_clause_counter);
    //        sprintf(sRusage,
    //                "%% User time                : %.3f s\n", sOut,
    //                (usage.ru_utime.tv_sec)+(usage.ru_utime.tv_usec) / 1000000.0);
    //        sprintf(sRusage,
    //                "%% System time              : %.3f s\n",
    //                (usage.ru_stime.tv_sec)+(usage.ru_stime.tv_usec) / 1000000.0);
    //        sprintf(sRusage,
    //                "%% Total time               : %.3f s\n",
    //                (usage.ru_utime.tv_sec + usage.ru_stime.tv_sec)+
    //                ((usage.ru_utime.tv_usec + usage.ru_stime.tv_usec) / 1000000.0));
    //        sprintf(sRusage,
    //                "%% Maximum resident set size: %ld pages\n", usage.ru_maxrss);
    //        sprintf(sRusage,
    //                "%% Maximum ClauseID         : %d \n", Env::global_clause_counter);
}
