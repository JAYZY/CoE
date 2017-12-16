/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   FileHelper.h
 * Author: Zhong Jian<77367632@qq.com>
 *
 * Created on 2017年12月15日, 上午11:03
 */

#ifndef FILEHELPER_H
#define FILEHELPER_H
#include "Global/IncDefine.h"

class FileHelper {
public:
    FileHelper();
    FileHelper(const FileHelper& orig);
    virtual ~FileHelper();

    /*static*/
    /// 打开输入文件
    /// \param name 文件名称
    /// \param fail 如果为true,则出现错误,终止程序,否则继续执行
    /// \return 打开的文件指针 FILE*

    static FILE* InputOpen(char* name, bool fail) {
        FILE* in;
        if (name && strcmp(name, "-") != 0) {
            Out::Err("Trying file ", name);
            in = fopen(name, "r");
            if (fail&& !in) {
                TmpErrno = errno;
                Out::SysError("Cannot open file %s for reading", ErrorCodes::FILE_ERROR, name);
            }
            if (fail) {
                Out::Err("Input file is ", name);
            }
        } else {
            Out::Err("Input is coming from <stdin>\n");
            in = stdin;
        }
        return in;
    }

private:

};


#endif /* FILEHELPER_H */

