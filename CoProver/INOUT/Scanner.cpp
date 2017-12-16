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

Scanner::Scanner(StreamType type, char *name, bool ignore_comments, char *default_d)
:source(nullptr),defaultDir(""),format(IOFormat::LOPFormat),accu(""),ignoreComments(ignore_comments),includeKey(nullptr)
{
    StreamCell* stream=new StreamCell(type,name,true);
    if( (type==nullptr&& strcmp(name,"-")==0)||(type!=nullptr) )    {
        stream->OpenStackedInput(this->source);        
    }
    else{
        assert(type==nullptr);
        if(FileNameIsAbsoulu)
        
    }
    
    TokenCell tokSequence[MAXTOKENLOOKAHEAD]; /* Need help? Bozo! */
    int current;            /* Pointer to current token in tok_sequence */
    char* includePos;      /* If created by "include", by which one? */
    
}

Scanner::Scanner(const Scanner& orig) {
}

Scanner::~Scanner() {
}

