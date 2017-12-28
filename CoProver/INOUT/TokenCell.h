/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   TokenCell.h
 * Author: Zhong Jian<77367632@qq.com>
 *
 * Created on 2017年12月15日, 上午8:29
 */

#ifndef TOKENCELL_H
#define TOKENCELL_H
#include "Global/IncDefine.h"
using namespace std;

enum class TokenType : uint64_t {
    NoToken = 1LL,
    WhiteSpace = 2LL,
    Comment = 4LL,
    Ident = 8LL,
    Idnum = 16LL,
    SemIdent = 32,
    String = 64,
    SQString = 128,
    PosInt = 256,
    OpenBracket = 512,
    CloseBracket = 1024,
    OpenCurly = 2048,
    CloseCurly = 4096,
    OpenSquare = 8192,
    CloseSquare = 16384,
    LesserSign = 32768,
    GreaterSign = 65536LL,
    EqualSign = 131072LL,
    NegEqualSign = 262144LL,
    TildeSign = (2 * TokenType::NegEqualSign),
    Exclamation = (2 * TokenType::TildeSign),
    AllQuantor = (TokenType::Exclamation),
    QuestionMark = (2 * TokenType::Exclamation),
    ExistQuantor = (TokenType::QuestionMark),
    Comma = (2 * TokenType::QuestionMark),
    Semicolon = (2 * TokenType::Comma),
    Colon = (2 * TokenType::Semicolon),
    Hyphen = (2 * TokenType::Colon),
    Plus = (2 * TokenType::Hyphen),
    Mult = (2 * TokenType::Plus),
    Fullstop = (2 * TokenType::Mult),
    Dollar = (2 * TokenType::Fullstop),
    Slash = (2 * TokenType::Dollar),
    Pipe = (2 * TokenType::Slash),
    FOFOr = (TokenType::Pipe),
    Ampersand = (2 * TokenType::Pipe),
    FOFAnd = (TokenType::Ampersand),
    FOFLRImpl = (2 * TokenType::Ampersand),
    FOFRLImpl = (2 * TokenType::FOFLRImpl),
    FOFEquiv = (2 * TokenType::FOFRLImpl),
    FOFXor = (2 * TokenType::FOFEquiv),
    FOFNand = (2 * TokenType::FOFXor),
    FOFNor = (2 * TokenType::FOFNand),
    SkipToken = (TokenType::WhiteSpace | TokenType::Comment),
    Identifier = (TokenType::Ident | TokenType::Idnum)
};
 typedef struct tokenRepcell {
        TokenType key;
        const char* rep;
    } TokenRepCell;
class TokenCell {
private:

   
    const static TokenRepCell token_print_rep[];
public:
    /****** inline ******/

    /*****************************************************************************
     * Test whether a given token is of type identifier and is one of a set of possible alternatives. 
     * This set is given as a single C-String, alternatives are separated by the '|' character.
     ****************************************************************************/
    inline bool TestId(const string& ids) {
        if (!TestTok(TokenType::Identifier)) {
            return false;
        }
        return strNElement(literal.c_str(), ids, literal.length());
    }

    /*****************************************************************************
     * As TestId(), but take only the non-numerical-part into account. 
     ****************************************************************************/
    inline bool TestIdNum(const string& ids) {
        int i, len = 0;
        char c;

        if (!TestTok(TokenType::Idnum)) {
            return false;
        }
        for (i = 0; c = literal[i]; i++) {
            if (!len && isdigit(literal[i])) {
                len = i;
            } else if (!isdigit(literal[i])) {
                len = 0;
            }
        }
        return strNElement(literal.c_str(), ids, len);
    }

    inline bool TestTok(TokenType toks) {
        return ((int64_t)this->tok & (int64_t) toks) != 0;
    }


public:
    TokenType tok; /* Type for AcceptTok(), TestTok() ...   */
    string literal; /* Verbatim copy of input for the token  */
    uint64_t numval; /* Numerical value (if any) of the token */
    string comment; /* Accumulated preceding comments        */
    bool skipped; /* Was this token preceded by SkipSpace? */
    string source; /* Ref. to the input stream source       */
    StreamType streamType; /* File or string? */
    long line; /* Position in this stream               */
    long column; /*  "               "                    */
public:
    TokenCell();
    TokenCell(const TokenCell& orig);
    virtual ~TokenCell();
public:
    /*****************************************************************************
     * Return a pointer to a description of a position in a file. 
     * The description is valid until the function is called the next time. 
     ****************************************************************************/
    void PosRep(string &retMsg);
    /*****************************************************************************
     *  Test whether the len lenght start of str is contained in the set id of strings 
     * (encoded in a single string with elements separated by |). 
     * 例：str="abc123"  ids="abc|123" 或者 str="123"  ids="abc|123"
     ****************************************************************************/
    bool strNElement(const string &str, const string &ids, int len);

    /*****************************************************************************
     * Return a pointer to a description of the set of tokens described by tok. 
     * The caller has to free the space of this description!
     ****************************************************************************/
    static void DescribeToken(TokenType ttype, string &descirbe);
};

#endif /* TOKENCELL_H */

