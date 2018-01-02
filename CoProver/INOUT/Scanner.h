
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
#include "BASIC/SplayTree.h"
//#include <cctype>

/* Data type repesenting the various types of encodings for function
 * symols (including constants) and predicates. */
enum class FuncSymbType : uint8_t {
    FSNone,
    FSIdentVar, /* Ident, starts with capital letter or _ 以大写字母与_开头的变量名*/
    FSIdentFreeFun, /* Ident, starts with Lower case letter or SQString */
    FSIdentInt, /* Integer */
    FSIdentFloat, /* Floating point number */
    FSIdentRational, /* Rational number */
    FSIdentInterpreted, /* SemIdent */
    FSIdentObject /* String "in double quotes" */
};
//字符串数字类型

enum class StrNumType {
    SNNoNumber,
    SNInteger,
    SNRational,
    SNFloat
};

class Scanner {
private:

    enum {
        MAXTOKENLOOKAHEAD = 4
    };

public:

    /*---------------------------------------------------------------------*/
    /*                       Inline  Function                              */
    /*---------------------------------------------------------------------*/

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

    inline void AcceptInpTok(TokenType toks) {
        CheckInpTok(toks);
        NextToken();
    }

    inline void AcceptInpTokNoSkip(TokenType toks) {
        CheckInpTokNoSkip(toks);
        NextToken();
    }

    inline void AcceptInpId(const std::string& toks) {
        CheckInpId(toks);
        NextToken();
    }

    inline void NextToken() {
        ScanRealToken();
        current = TOKENREALPOS(current + 1);
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

    inline void TokenPosRep(std::string& retStr) {
        tokSequence[current].PosRep(retStr);
    }

    inline bool TestInpTok(TokenType tokType) {
        return AktToken()->TestTok(tokType);
    }

    inline bool TestInpId(const std::string& strId) {
        return AktToken()->TestId(strId);
    }

    inline bool TestInpIdNum(const std::string& strId) {
        return AktToken()->TestIdNum(strId);
    }

    inline bool TestInpNoSkip() {
        return !AktToken()->skipped;
    }

    inline bool TestInpTokNoSkip(TokenType tokType) {
        return !AktToken()->skipped && AktToken()->TestTok(tokType);
    }
    //   Skip a TSTP source field.
    //　跳过TSTP

    inline void TSTPSkipSource() {
        AcceptInpTok(TokenType::TSTPToken);
        if (TestInpTok(TokenType::OpenBracket)) {
            ParseSkipParenthesizedExpr();
        }
    }

/* Skip any expression containing balanced (), [], {}. Print error on missmatch. 
     * Note that no full syntax check is performed, we are only interested in the different braces.*/

    void ParseSkipParenthesizedExpr() {

        vector<IntOrP> paren_stack;
        TokenType tok;

        TokenType checkOpenToken = (TokenType) ((uint64_t) TokenType::OpenBracket | (uint64_t) TokenType::OpenCurly | (uint64_t) TokenType::OpenSquare);
        TokenType checkCloseToken = (TokenType) ((uint64_t) TokenType::CloseBracket | (uint64_t) TokenType::CloseCurly | (uint64_t) TokenType::CloseSquare);
        CheckInpTok(checkOpenToken);

        //PStackPushInt(paren_stack, in->AktTokenType());
        IntOrP tmp;
        tmp.i_val = (uint64_t) AktToken()->tok;
        paren_stack.push_back(tmp);
        NextToken();
        while (!paren_stack.empty()) {
            if (TestInpTok(checkOpenToken)) {
                IntOrP tmpIner;
                tmpIner.i_val = (uint64_t) AktToken()->tok;
                paren_stack.push_back(tmpIner);
                NextToken();
            } else if (TestInpTok(checkCloseToken)) {
                //tok = PStackPopInt(paren_stack);
                paren_stack.pop_back();
                IntOrP tmpIner;
                tmpIner = paren_stack.back();
                tok =(TokenType)tmpIner.i_val;
                switch (tok) {
                    case TokenType::OpenBracket:
                        AcceptInpTok(TokenType::CloseBracket);
                        break;
                    case TokenType::OpenCurly:
                        AcceptInpTok(TokenType::CloseCurly);
                        break;
                    case TokenType::OpenSquare:
                        AcceptInpTok(TokenType::CloseSquare);
                        break;
                    default:
                        assert(false && "Impossible value on parentheses stack");
                        break;
                }
            } else {
                NextToken();
            }
        }
        //PStackFree(paren_stack);
        paren_stack.clear();
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
    string includePos; /* If created by "include", by which one? */

public:
    /// 构造函数创建一个扫描器 scanner
    /// \param type 扫描的类型 
    /// \param name 文件名称
    /// \param ignore_comments 是否忽略注释
    /// \param default_d    
    Scanner(StreamType type, const char *name, bool ignore_comments, const char *default_d);
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
    /* 调用 CheckInpTok()之前,检查如果是 SkipTokens,则给出错误提示. */
    void CheckInpTokNoSkip(TokenType tokType);
    /* Check AktToken() 是否是一个 identifier with the desired value. Produce error if not.*/
    void CheckInpId(const string& strId); /* Produce a syntax error at the current token with the given message.  */
    void AktTokenError(const char* msg, bool syserr);
    /* Set the format of the scanner (in particular, guess a format if  */
    void ScannerSetFormat(IOFormat fmt);

    FuncSymbType FuncSymbParse(string& id);
    /* 获取整数的字符串形式并将其转换为整数形式,删除+和数字前的若干个0. */
    void NormalizeIntRep(string& outIntRep);
    /* 获取整数的字符串形式并将其转换为整数形式,删除+和数字前的若干个0. */
    void NormalizeRationalRep(string& outIntRep);
    /* Take a string representation of a floating point number and turn it into a normal form. */
    void NormalizeFloatRep(string& outFloatRep);

    long ParseInt();
    double ParseFloat();
    StrNumType ParseNumString();


    TokenCell* ScanRealToken();
    TokenCell* ScanTokenFollowIncludes();
    TokenCell* ScanToken();



    /* Parse a TPTP-Style include statement. Return a scanner for the included file, 
     * and put (optional) selected names into name_selector. If the file name is in skip_includes, 
     * skip the rest and return NULL.*/
    Scanner* ScannerParseInclude(SplayTree<StrTreeCell> &name_selector, SplayTree<StrTreeCell> &skip_includes);
};

#endif /* SCANNER_H */

