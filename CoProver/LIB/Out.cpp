/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Out.cpp
 * Author: Zhong Jian<77367632@qq.com>
 * 
 * Created on 2017年12月15日, 下午4:18
 */

#include "Out.h"
 

char* ErrStr=nullptr;
/* Saving errno from the system call originating it */

int TmpErrno;
__attribute__ ((noreturn))
void Out::SysError(const char* message, ErrorCodes ret, ...) {
    va_list ap;
    va_start(ap, ret);
    fprintf(stderr, "%s: ", ProgInfo::ProgName.c_str());
    vfprintf(stderr, message, ap);
    fprintf(stderr, "\n");
    errno = TmpErrno;
    perror(ProgInfo::ProgName.c_str());
    va_end(ap);
    exit((int)ret);
}
void Out::Warning(const char* message, ...)
{
   va_list ap;
   va_start(ap, message);

   fprintf(stderr, "%s: Warning: ", ProgInfo::ProgName.c_str());
   vfprintf(stderr, message, ap);
   fprintf(stderr, "\n");

   va_end(ap);
}
void Out::SysWarning(const char* message, ...)
{
   va_list ap;
   va_start(ap, message);

   fprintf(stderr, "%s: Warning: ", ProgInfo::ProgName.c_str());
   vfprintf(stderr, message, ap);
   fprintf(stderr, "\n");
   errno = TmpErrno;
   perror(ProgInfo::ProgName.c_str());

   va_end(ap);
}

Out::Out() {
}

Out::Out(const Out& orig) {
}

Out::~Out() {
}

