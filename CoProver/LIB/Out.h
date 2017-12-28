/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Out.h
 * Author: Zhong Jian<77367632@qq.com>
 *
 * Created on 2017年12月15日, 下午4:18
 */

#ifndef OUT_H
#define OUT_H
#include <cstdio>
#include <cerrno>
#include <cstdarg>
#include <cstdlib>
#include <string>
#include "HEURISTICS/Options.h"
extern char* ErrStr;
 
extern int TmpErrno;
 
enum class ErrorCodes {
    NO_ERROR = 0,
    PROOF_FOUND = 0,
    SATISFIABLE = 1,
    OUT_OF_MEMORY = 2,
    SYNTAX_ERROR = 3,
    USAGE_ERROR = 4,
    FILE_ERROR = 5,
    SYS_ERROR = 6,
    CPU_LIMIT_ERROR = 7,
    RESOURCE_OUT = 8,
    INCOMPLETE_PROOFSTATE = 9,
    OTHER_ERROR = 10,
    INPUT_SEMANTIC_ERROR = 11
};

class Out {
public:

    static inline void Err(const char* arg) {
        fprintf(stderr, "%s: %s", (Options::ProgName).c_str(), (arg));
        fflush(stderr);
    }

    static inline void Err(const char* arg1, const char* arg2) {
        fprintf(stderr, "%s: %s%s\n", Options::ProgName.c_str(), (arg1), (arg2));
        fflush(stderr);
    }

    __attribute__ ((noreturn))
    static inline void Error(const char* message, ErrorCodes ret, ...) {
        va_list ap;
        va_start(ap, ret);
        fprintf(stderr, "%s: ", Options::ProgName.c_str());
        vfprintf(stderr, message, ap);
        fprintf(stderr, "\n");
        va_end(ap);
        exit((int)ret);
    }
    __attribute__ ((noreturn))
    static void SysError(const char* message, ErrorCodes ret, ...);
    static void Warning(char* message, ...);
    static void SysWarning(char* message, ...);

public:
    Out();
    Out(const Out& orig);
    virtual ~Out();
private:

};

#endif /* OUT_H */

