/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Scanner.cpp
 * Author: Sammy Guergachi <sguergachi at gmail.com>
 * 
 * Created on 2017年12月14日, 上午7:18
 */

#include "Scanner.h"
#include "FileOp.h"

/// 构造函数创建一个扫描器 scanner
/// \param type 扫描的类型 
/// \param name 文件名称
/// \param ignore_comments 是否忽略注释
/// \param default_d  

Scanner::Scanner(StreamType type, char *name, bool ignore_comments, char *default_dir)
: source(nullptr), defaultDir(""), format(IOFormat::LOPFormat), accu(""), ignoreComments(ignore_comments), includeKey(nullptr) {
    StreamCell* stream;
    //文件名称以"-"开头,类型为空指针
    if ((type == nullptr&& strcmp(name, "-") == 0) || (type != nullptr)) {
        stream = OpenStackedInput(&source, type, name, true);

        assert(stream);
    } else {
        //扫描器类型为空指针
        assert(type == nullptr);
        if (FileOp::FileNameIsAbsolute(name)) {
            //输入的文件目录是绝对路径
            stream = OpenStackedInput(&source, type, name, true);

            this->defaultDir += FileOp::FileNameDirName(name); //提取目录名称(绝对路径)
            assert(stream);
        } else {
            //输入的文件目录是相对路径--相对路径解析
            if (default_dir) {//输入有默认路径
                this->defaultDir += default_dir;
                assert(this->defaultDir.length() > 0 || FileOp::StrLastChar(this->defaultDir.c_str()));
            }
            //默认路径 = 输入的默认基本路径+ 文件名自带的相对路径
            this->defaultDir += FileOp::FileNameDirName(name);
            assert(this->defaultDir.length() > 0 || FileOp::StrLastChar(this->defaultDir.c_str()));
            string fullFileName = this->defaultDir + FileOp::FileNameBaseName(name);
            stream = OpenStackedInput(&this->source, type, fullFileName.c_str(), true);

            if (!stream) {
                //如果创建流失败 -- 例如,文件读取错误,再一次解析环境变量中的 TPTP_dir  再次进行打开文件流
                //[略]                
            }
        }
    }
    //预读取MAXTOKENLOOKAHEAD=4个字符 
    for (this->current = 0; this->current<this->MAXTOKENLOOKAHEAD; ++this->current) {
        AktToken()->tok = TokenType::NoToken;
        AktToken()->literal = "";
        AktToken()->comment = "";
        AktToken()->source = "";
        AktToken()->streamType = nullptr;
        ScanRealToken();
    }
    this->current = 0;
    this->includePos = nullptr;

}

Scanner::Scanner(const Scanner& orig) {
}

Scanner::~Scanner() {
}

/*****************************************/
void Scanner::ScanIdent() {
    long numstart = 0;

    for (long i = 0; isidchar(CurrChar()); ++i) {
        if (!numstart && isdigit(CurrChar())) {
            numstart = i;
        } else if (!isdigit(CurrChar())) {
            numstart = 0;
        }
        AktToken()->literal += CurrChar();
        NextChar();
    }
    if (numstart) {
        AktToken()->tok = TokenType::Idnum;
        AktToken()->numval =
                strtol((AktToken()->literal).c_str() + numstart, nullptr, 10);
        /* Errors are intentionally ignored to allow arbitrary
           identifiers */
    } else {
        AktToken()->tok = TokenType::Ident;
        AktToken()->numval = 0;
    }
}

void Scanner::ScanWhite() {
    AktToken()->tok = TokenType::WhiteSpace;
    while (isspace(CurrChar())) {
        AktToken()->literal += CurrChar();
        NextChar();
    }
}

void Scanner::ScanString(char delim) {
    bool escape = false;
    TokenCell* token = AktToken();
    token->tok = (delim == '\'') ? TokenType::SQString : TokenType::String;
    //AktToken()->tok 

    AktToken()->literal += CurrChar();
    NextChar();
    while (escape || (CurrChar() != delim)) {
        if (!isprint(CurrChar())) {
            AktTokenError("Non-printable character in string constan", false);
        }
        if (CurrChar() == '\\') {
            escape = !escape;
        } else {
            escape = false;
        }
        AktToken()->literal += CurrChar();
        NextChar();
    }
    AktToken()->literal += CurrChar();
    NextChar();
}

void Scanner::ScanCommentC() {
    AktToken()->tok = TokenType::Comment;

    while (!((CurrChar() == '*') && (LookChar(1) == '/'))) {
        AktToken()->literal += CurrChar();
        NextChar();
    }
    AktToken()->literal += CurrChar();
    NextChar();
    AktToken()->literal += CurrChar();
    NextChar();
}

void Scanner::ScanLineComment() {
    AktToken()->tok = TokenType::Comment;

    while ((CurrChar()) != '\n' && (CurrChar() != EOF)) {
        AktToken()->literal += CurrChar();
        NextChar();
    }
    AktToken()->literal += '\n';
    NextChar(); /* Should be harmless even at EOF */
}

void Scanner::ScanInt() {
    AktToken()->tok = TokenType::PosInt;
    while (isdigit(CurrChar())) {


        AktToken()->literal += CurrChar();

        NextChar();
    }
    errno = 0;
    AktToken()->numval =
            strtol(AktToken()->literal.c_str(), nullptr, 10);
    /* strtoul is not available on all systems....*/

    if (errno) {
        static char buff[10];
        char* term = strncpy(buff, AktToken()->literal.c_str(), 9);
        *term = '\0';
        strtol(buff, nullptr, 10);
        //Warning("Number truncated while reading %s. If this happens on 32 bit systems while parsing internal strings, it is harmless an can be ignored",  DStrView(AktToken(in)->literal));
    }
}

/***************************************************************************** 
 * Check whether AktTok(in) is of one of the desired types. Produce error if not  
 ****************************************************************************/
void Scanner::CheckInpTok(TokenType tokType) {
    if (!TestInpTok(tokType)) {

        accu = "";
        TokenCell::DescribeToken(tokType, accu); //得到token的描述

        accu += " expected, but ";

        string tmp = "";
        TokenCell::DescribeToken(AktToken()->tok, tmp); //得到token的描述
        accu += tmp + " read ";
        AktTokenError(accu.c_str(), false);
    }
}

/*****************************************************************************
 * Produce a syntax error at the current token with the given message.  
 ****************************************************************************/
void Scanner::AktTokenError(const char* msg, bool syserr) {
    string err;
    TokenPosRep(err);
    err += "(just read '";
    err += AktToken()->literal;
    err = err + "'): " + msg;
    if (syserr) {
        //cout << "syserr:" << err + " SYNTAX_ERROR " << endl;
        Out::SysError(err.c_str(), ErrorCodes::SYNTAX_ERROR);
        //SysError(DStrView(err), SYNTAX_ERROR);
    } else {
        Out::Error(err.c_str(), ErrorCodes::SYNTAX_ERROR);
        //cout << "Err:" << err << " SYNTAX_ERROR " << endl;
        //Error(DStrView(err), SYNTAX_ERROR);
    }

}

/***************************************************************************** 
 * Set the format of the scanner (in particular, guess a format if
 ****************************************************************************/
void Scanner::ScannerSetFormat(IOFormat fmt) {
    if (fmt == IOFormat::AutoFormat) {

        if (TestInpId("fof|cnf|tff|include")) {
            fmt = IOFormat::TSTPFormat;
        } else if (TestInpId("input_clause|input_formula")) {
            fmt = IOFormat::TPTPFormat;
        } else {
            fmt = IOFormat::LOPFormat;
        }
    }
    format = fmt;
}

/***************************************************************************** 
 * Scan tokens until a real token (i.e. not a SkipToken has been scanned.
 ****************************************************************************/
TokenCell* Scanner::ScanRealToken() {
    AktToken()->skipped = false;
    AktToken()->comment = "";

    ScanTokenFollowIncludes();

    while (AktToken()->TestTok(TokenType::SkipToken)) {
        AktToken()->skipped = true;
        if (!this->ignoreComments && TestInpTok(TokenType::Comment)) {
            AktToken()->comment += AktToken()->literal;
        }
        ScanTokenFollowIncludes();
    }
    return AktToken();
}

/***************************************************************************** 
 * Scan a token, follow include directives and pop back empty input streams.
 ****************************************************************************/
TokenCell* Scanner::ScanTokenFollowIncludes() {
    ScanToken();
    if (this->includeKey && (TestInpId(this->includeKey))) {
        string name = "";
        string tptpSource = getenv("TPTP"); //从环境变量中得到 TPTP目录
        if (!tptpSource.empty()) {
            name += tptpSource;
            int len = name.length();

            if (len > 0 && name[len - 1] != '/') {
                name += "/";
            }
        }
        ScanToken();

        CheckInpTok(TokenType::OpenBracket);
        ScanToken();
        int test = (int) TokenType::Identifier | (int) TokenType::String | (int) TokenType::SQString;
        CheckInpTok((TokenType) test);
        if (TestInpTok(TokenType::Identifier)) {
            name += AktToken()->literal;
        } else { //eg.name="abcde" ==> name=bcd; 去掉首尾字符
            name += AktToken()->literal.substr(1, AktToken()->literal.length() - 2);
        }

        OpenStackedInput(&source, nullptr, name.c_str(), true);

        ScanTokenFollowIncludes();
    } else if (includeKey && TestInpTok(TokenType::NoToken)) {
        if (source->next) {
            CloseStackedInput(&(source));
            ScanToken();
            CheckInpTok(TokenType::CloseBracket);
            ScanToken();
            CheckInpTok(TokenType::Fullstop);
            ScanTokenFollowIncludes();
        }
    }
    return AktToken();
}



/*****************************************************************************
 *  重要方法:Scans a token into tokSequence[current].
 *  Does _not_ move the AktToken-pointer - this is done only for real 
 * (i.e. non white,non-comment) tokens in the function NextToken(). 
 * TokenCell which does not contain any outside references. 
 ****************************************************************************/

/* .                           */
TokenCell* Scanner::ScanToken() {

    AktToken()->literal = ""; //DStrReset(AktToken(in)->literal); 
    AktToken()->source = source->source; //DStrGetRef(Source(in));   
    AktToken()->streamType = source->streamType;
    AktToken()->line = source->line;
    AktToken()->column = source->column;

    if (!ischar(CurrChar())) {
        AktToken()->tok = TokenType::NoToken;
    } else if (isspace(CurrChar())) {
        ScanWhite();
    } else if (isstartidchar(CurrChar())) {

        ScanIdent();
    } else if (isdigit(CurrChar())) {

        ScanInt();
    } else if (isstartcomment(CurrChar())) {
        ScanLineComment();
    } else if (CurrChar() == '/' && LookChar(1) == '*') {
        ScanCommentC();
    } else if ((CurrChar() == '"') || (CurrChar() == '\'')) {
        ScanString(CurrChar());
    } else if ((CurrChar() == '$') && isidchar(LookChar(1))) {
        AktToken()->literal += CurrChar();
        NextChar();
        ScanIdent();
        AktToken()->tok = TokenType::SemIdent;
    } else {
        switch (CurrChar()) {
            case '(':
                AktToken()->tok = TokenType::OpenBracket;
                break;
            case ')':
                AktToken()->tok = TokenType::CloseBracket;
                break;
            case '{':
                AktToken()->tok = TokenType::OpenCurly;
                break;
            case '}':
                AktToken()->tok = TokenType::CloseCurly;
                break;
            case '[':
                AktToken()->tok = TokenType::OpenSquare;
                break;
            case ']':
                AktToken()->tok = TokenType::CloseSquare;
                break;
            case '<':
                if ((LookChar(1) == '~' && (LookChar(2) == '>'))) {
                    AktToken()->literal += CurrChar();
                    NextChar();
                    AktToken()->literal += CurrChar();
                    NextChar();
                    AktToken()->tok = TokenType::FOFXor;
                } else if (LookChar(1) == '=') {
                    AktToken()->literal += CurrChar();
                    NextChar();
                    if (LookChar(1) == '>') {
                        AktToken()->literal += CurrChar();
                        NextChar();
                        AktToken()->tok = TokenType::FOFEquiv;
                    } else {
                        AktToken()->tok = TokenType::FOFRLImpl;
                    }
                } else {
                    AktToken()->tok = TokenType::LesserSign;
                    break;
                }
                break;
            case '>':
                AktToken()->tok = TokenType::GreaterSign;
                break;
            case '=':
                if (LookChar(1) == '>') {
                    AktToken()->literal += CurrChar();
                    NextChar();
                    AktToken()->tok = TokenType::FOFLRImpl;
                } else {
                    AktToken()->tok = TokenType::EqualSign;
                }
                break;
            case '~':
                switch (LookChar(1)) {
                    case '|':
                        AktToken()->literal += CurrChar();
                        NextChar();
                        AktToken()->tok = TokenType::FOFNor;
                        break;
                    case '&':
                        AktToken()->literal += CurrChar();
                        NextChar();
                        AktToken()->tok = TokenType::FOFNand;
                        break;

                    default:
                        AktToken()->tok = TokenType::TildeSign;
                        break;
                }
                break;
            case '!':
                if (LookChar(1) == '=') {
                    AktToken()->literal += CurrChar();
                    NextChar();
                    AktToken()->tok = TokenType::NegEqualSign;
                } else {
                    AktToken()->tok = TokenType::Exclamation;
                }
                break;
            case '?':
                AktToken()->tok = TokenType::QuestionMark;
                break;
            case ',':
                AktToken()->tok = TokenType::Comma;
                break;
            case ';':
                AktToken()->tok = TokenType::Semicolon;
                break;
            case ':':
                AktToken()->tok = TokenType::Colon;
                break;
            case '-':
                AktToken()->tok = TokenType::Hyphen;
                break;
            case '+':
                AktToken()->tok = TokenType::Plus;
                break;
            case '*':
                AktToken()->tok = TokenType::Mult;
                break;
            case '.':
                AktToken()->tok = TokenType::Fullstop;
                break;
            case '|':
                AktToken()->tok = TokenType::Pipe;
                break;
            case '/':
                AktToken()->tok = TokenType::Slash;
                break;
            case '&':
                AktToken()->tok = TokenType::Ampersand;
                break;
            case '$':
                AktToken()->tok = TokenType::Dollar;
                break;
            default:
                AktToken()->literal += CurrChar();
                //AktTokenError(in, "Illegal character", false);
                break;
        }
        AktToken()->literal += CurrChar();

        NextChar();
    }
    //AktToken()->PrintToken(stdout);
    return AktToken();
}
