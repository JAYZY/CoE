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

/* 命令行参数类型 */
enum class OptArgType {
    NoArg, //无参数
    NumArg, //数字参数,包括整数,浮点数
    StrArg //字符串参数
};

typedef struct optcell {
    int option_code;
    char shortopt; /* Single Character options */
    char* longopt; /* Double dash, GNU-Style */
    OptArgType type; /* What about Arguments? */
    char* arg_default; /* Default for optional argument (long style only */
    char* desc; /* Put the documentation in immediately! */
} OptCell, *Opt_p;

class CommandLine {
private:
    vector<OptCell*> opts; //解析的命令行列表
public:



    CommandLine();
    CommandLine(const CommandLine& orig);
    virtual ~CommandLine();
private:

};

#endif /* COMMANDLINE_H */

