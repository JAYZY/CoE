/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   CommandLine.cpp
 * Author: Zhong Jian<77367632@qq.com>
 * 
 * Created on 2018年4月26日, 上午8:04
 */

#include "CommandLine.h"

CommandLine::CommandLine(int argc, char** argv) {
    //解析命令行  coprover 文件名  --cnf=12    -cnf=12   一个长命令 一个短命令 统一解析为命令
//    Env::tptpFileName = argv[1];
//    for (int i = 2; i < argc; i++) {
//        string arg = argv[i];
//        if (arg[0] == '-') {//短命令
//            int pos=1;
//            if (arg[1] == '-') {//长命令
//                pos=2;
//            }
//            int endPos=arg.find_first_of('=');//查找=
//            cmd=arg.substr(pos,endPos-pos);
//        }
//    }




}

CommandLine::CommandLine(const CommandLine& orig) {
}

CommandLine::~CommandLine() {
}

