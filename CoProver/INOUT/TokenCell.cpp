/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   TokenCell.cpp
 * Author: Zhong Jian<77367632@qq.com>
 * 
 * Created on 2017年12月15日, 上午8:29
 */

#include "TokenCell.h"
#include <sys/param.h>

 const TokenRepCell TokenCell::token_print_rep[] = {
    { TokenType::NoToken, "No token (probably EOF)"},
    { TokenType::WhiteSpace, "White space (spaces, tabs, newlines...)"},
    { TokenType::Comment, "Comment"},
    { TokenType::Ident, "Identifier not terminating in a number"},
    { TokenType::Idnum, "Identifier terminating in a number"},
    { TokenType::SemIdent, "Interpreted function/predicate name ('$name')"},
    { TokenType::String, "String enclosed in double quotes (\"\")"},
    { TokenType::SQString, "String enclosed in single quote ('')"},
    { TokenType::PosInt, "Integer (sequence of decimal digits) convertable to an 'unsigned long'"},
    /* May need LargePosInt here... */
    { TokenType::OpenBracket, "Opening bracket ('(')"},
    { TokenType::CloseBracket, "Closing bracket (')')"},
    { TokenType::OpenCurly, "Opening curly brace ('{ TokenType::')"},
    { TokenType::CloseCurly, "Closing curly brace ('}')"},
    { TokenType::OpenSquare, "Opening square brace ('[')"},
    { TokenType::CloseSquare, "Closing square brace (']')"},
    { TokenType::LesserSign, "\"Lesser than\" sign ('<')"},
    { TokenType::GreaterSign, "\"Greater than\" sign ('>')"},
    { TokenType::EqualSign, "Equal Predicate/Sign ('=')"},
    { TokenType::NegEqualSign, "Negated Equal Predicate ('!=')"},
    { TokenType::TildeSign, "Tilde ('~')"},
    { TokenType::Exclamation, "Exclamation mark ('!')"},
    { TokenType::QuestionMark, "Question mark ('?')"},
    { TokenType::Comma, "Comma (',')"},
    { TokenType::Semicolon, "Semicolon (';')"},
    { TokenType::Colon, "Colon (':')"},
    { TokenType::Hyphen, "Hyphen ('-')"},
    { TokenType::Plus, "Plus sign ('+')"},
    { TokenType::Mult, "Multiplication sign ('*')"},
    { TokenType::Fullstop, "Fullstop ('.')"},
    { TokenType::Dollar, "Dollar sign ('$')"},
    { TokenType::Slash, "Slash ('/')"},
    { TokenType::Pipe, "Vertical bar ('|')"},
    { TokenType::Ampersand, "Ampersand ('&')"},
    { TokenType::FOFLRImpl, "Implication/LRArrow ('=>')"},
    { TokenType::FOFRLImpl, "Back Implicatin/RLArrow ('<=')"},
    { TokenType::FOFEquiv, "Equivalence/Double arrow ('<=>')"},
    { TokenType::FOFXor, "Negated Equivalence/Xor ('<~>')"},
    { TokenType::FOFNand, "Nand ('~&')"},
    { TokenType::FOFNor, "Nor ('~|'')"},
    { TokenType::NoToken, nullptr}

};
bool TokenCell::SigSupportLists=false;
TokenCell::TokenCell() {
}

TokenCell::TokenCell(const TokenCell& orig) {
}

TokenCell::~TokenCell() {
}

/*****************************************************************************
 * Return a pointer to a description of a position in a file. 
 * The description is valid until the function is called the next time. 
 ****************************************************************************/
void TokenCell::PosRep(string &retMsg) {
    
    if (streamType == nullptr) {
        assert(source.length() <= MAXPATHLEN);

        retMsg += source + ":" + to_string(line) + ":(Column " + to_string(column) + "):";
        //sprintf(buff, "%s:%ld:", source, line, column);
    } else {

        retMsg =retMsg+ streamType + ": \"";

        if (source.length() > MAXPATHLEN - 4) {
            retMsg += this->source.substr(0, MAXPATHLEN - 4) + "...";
        } else {
            retMsg += this->source;
        }
        retMsg += "\":" + to_string(this->line) + ":(Column " + to_string(column) + "):";

    }
}

/*****************************************************************************
 *  Test whether the len lenght start of str is contained in the set id of strings 
 * (encoded in a single string with elements separated by |). 
 * 例：str="abc123"  ids="abc|123" 或者 str="123"  ids="abc|123"
 ****************************************************************************/
bool TokenCell::strNElement(const string &str, const string &ids, int len) {
    if (str == ids)return true; //完全相等　　 
    for (int i = 0; i < ids.length(); ++i) {
        int k = 0;
        for (k = 0; k < len && i < ids.length(); k++, i++) {
            if (ids[i] != str[k])break;
        }
        if (k == len && (i == ids.length() || ids[i] == '|')) {
            return true;
        }
        while (i < ids.length() && ids[i] != '|') {
            i++;
        }
    }
    return false;
}

/*****************************************************************************
 * Return a pointer to a description of the set of tokens described by tok. 
 * The caller has to free the space of this description!
 ****************************************************************************/
void TokenCell::DescribeToken(TokenType ttype, string &descirbe) {
      
    string res;
    bool found = false;
    for (int i = 0; token_print_rep[i].rep; ++i) {
        if ((uint64_t)ttype & (uint64_t)token_print_rep[i].key) {
            res += (found ? " or " : "");
            res += token_print_rep[i].rep;
            found = true;
        }
    }
    if (!found) {
        res += token_print_rep[0].rep;
    }
    descirbe = res;

    //help = DStrCopy(res);
    //DStrFree(res);

}