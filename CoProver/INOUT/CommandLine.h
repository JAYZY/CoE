/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   CommandLine.h
 * Author: Zhong Jian<77367632@qq.com>
 *
 * Created on 2018年4月26日, 上午8:04
 */

#ifndef COMMANDLINE_H
#define COMMANDLINE_H
#include "Global/IncDefine.h"


class CommandLine {
private:
  //  vector<OptCell*> opts; //解析的命令行列表
public:
    


    CommandLine(int argc, char** argv);
    CommandLine(const CommandLine& orig);
    virtual ~CommandLine();
private:

};

#endif /* COMMANDLINE_H */

