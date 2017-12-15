
/* 
 * File:   Scanner.h
 * Author: zj <sguergachi at gmail.com>
 *
 * Created on 2017年12月14日, 上午7:18
 */

#ifndef SCANNER_H
#define SCANNER_H
#include "StreamCell.h" 
#include "TokenCell.h"
using namespace std;
//输入文件的格式
enum class IOFormat {
    LOPFormat=0,
    TPTPFormat=2,
    TSTPFormat,
    AutoFormat,
};

class Scanner {
private:
    enum {MAXTOKENLOOKAHEAD=4};
public:
    StreamCell* source;
    string defaultDir;      // 读取的文件目录
    IOFormat format;        //读取的格式
    string accu;            /* Place for Multi-Token constructs or messages */
    bool ignoreComments;    /* Comments can be skipped completely. If not set, comments are accumulated 
                                                 * (but never delivered as tokens) */
    char* includeKey;      /* An Identifier,  e.g. "include" */
    TokenCell tokSequence[MAXTOKENLOOKAHEAD]; /* Need help? Bozo! */
    int current;            /* Pointer to current token in tok_sequence */
    char* includePos;      /* If created by "include", by which one? */

public:
    /// 构造函数创建一个扫描器 scanner
    /// \param type 扫描的类型 
    /// \param name 文件名称
    /// \param ignore_comments 是否忽略注释
    /// \param default_d    
    Scanner(StreamType type, char *name, bool ignore_comments, char *default_d);
    Scanner(const Scanner& orig);
    virtual ~Scanner();
private:

};

#endif /* SCANNER_H */

