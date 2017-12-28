
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
//#include <cctype>



class Scanner {
private:

    enum {
        MAXTOKENLOOKAHEAD = 4
    };

public:

    /*********inline ***********/

    /*isalpha 字母（包括大写、小写）islower（小写字母）isupper（大写字母）isalnum（字母大写小写+数字）
     * isblank（space和\t）isspace（space、\t、\r、\n）*/
    inline bool ischar(const char ch) {
        return (ch) != EOF;
    }

    inline bool isstartidchar(const char ch) {
        return (isalpha(ch) || (ch) == '_');
    }
    //是否是字母大写小写+数字+'_'

    inline bool isidchar(const char ch) {
        return (isalnum(ch) || (ch) == '_');
    }
    //是否开始注释

    inline bool isstartcomment(const char ch) {
        return ((ch) == '#' || (ch) == '%');
    }

    inline char CurrChar() {
        return (char) source->buffer[source->current];
    }

    inline TokenCell* AktToken() {
        return &this->tokSequence[this->current];
    }

    inline int TOKENREALPOS(int pos) {
        return (pos) % MAXTOKENLOOKAHEAD;
    }

    inline TokenCell* LookToken(int look) {
        return &tokSequence[TOKENREALPOS(current + look)];
    }

    inline char NextChar() {
        return source->StreamNextChar();
    }

    inline char LookChar(int look) {
        return source->StreamLookChar(look);
    }

    inline void TokenPosRep(string& retStr) {
        tokSequence[current].PosRep(retStr);
    }

    inline bool TestInpTok(TokenType tokType) {
        return AktToken()->TestTok(tokType);
    }

    inline bool TestInpId(const string& strId) {
        return AktToken()->TestId(strId);
    }

    inline bool TestInpIdNum(const string& strId) {
        return AktToken()->TestIdNum(strId);
    }

    inline bool TestInpNoSkip() {
        return !AktToken()->skipped;
    }
public:
    StreamCell* source;
    string defaultDir; // 读取的文件目录
    IOFormat format; //读取的格式
    string accu; /* Place for Multi-Token constructs or messages */
    bool ignoreComments; /* Comments can be skipped completely. If not set, comments are accumulated 
                                                 * (but never delivered as tokens) */
    char* includeKey; /* An Identifier,  e.g. "include" */
    TokenCell tokSequence[MAXTOKENLOOKAHEAD]; /* Need help? Bozo! */
    int current; /* Pointer to current token in tok_sequence */
    char* includePos; /* If created by "include", by which one? */

public:
    /// 构造函数创建一个扫描器 scanner
    /// \param type 扫描的类型 
    /// \param name 文件名称
    /// \param ignore_comments 是否忽略注释
    /// \param default_d    
    Scanner(StreamType type, char *name, bool ignore_comments, char *default_d);
    Scanner(const Scanner& orig);
    virtual ~Scanner();
public:
    /*---------------------------------------------------------------------*/
    /*                           member Function                           */
    /*---------------------------------------------------------------------*/
    void ScanIdent();
    void ScanInt();
    void ScanWhite();
    void ScanLineComment();
    void ScanCommentC();
    void ScanString(char delim);
    /* Check whether AktTok(in) is of one of the desired types. Produce error if not */
    void CheckInpTok(TokenType tokType);

    /* Produce a syntax error at the current token with the given message.  */
    void AktTokenError(const char* msg, bool syserr);
    /* Set the format of the scanner (in particular, guess a format if  */
    void ScannerSetFormat(IOFormat fmt);
    
    TokenCell* ScanRealToken();
    TokenCell* ScanTokenFollowIncludes();
    TokenCell* ScanToken();
};

#endif /* SCANNER_H */

