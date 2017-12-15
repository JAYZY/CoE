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


enum class TokenType:uint64_t{
    NoToken =1LL,
    WhiteSpace=2LL,
    Comment=4LL,
    Ident=8LL,
    idnum=16LL
    
}; 
class TokenCell {
public:
   TokenType     tok;         /* Type for AcceptTok(), TestTok() ...   */
   string        literal;     /* Verbatim copy of input for the token  */
   uint64_t      numval;      /* Numerical value (if any) of the token */
   string        comment;     /* Accumulated preceding comments        */
   bool          skipped;     /* Was this token preceded by SkipSpace? */
   string        source;      /* Ref. to the input stream source       */   
   StreamType    stream_type; /* File or string? */
   long          line;        /* Position in this stream               */
   long          column;      /*  "               "                    */
public:
    TokenCell();
    TokenCell(const TokenCell& orig);
    virtual ~TokenCell();
private:

};

#endif /* TOKENCELL_H */

