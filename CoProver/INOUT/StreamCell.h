/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   StreamCell.h
 * Author: Zhong Jian<77367632@qq.com>
 *
 * Created on 2017年12月15日, 上午9:20
 */

#ifndef STREAMCELL_H
#define STREAMCELL_H
#include "Global/IncDefine.h"
using namespace std;

class StreamCell {
private:

    enum {
        MAXLOOKAHEAD = 64
    };

    inline char StreamCurrChar() {
        return this->buffer[this->current];
    }

    inline int StreamRealPos(int pos) {
        return ((pos) % MAXLOOKAHEAD);
    }

public:

    /******* inline **********/
    inline char StreamLookChar(int look) {
        assert(look < MAXLOOKAHEAD);
        return this->buffer[StreamRealPos(this->current + look)];
    }
public:
    StreamCell* next;
    string source;
    StreamType streamType; /* Only constant strings allowed here! */
    long stringPos;
    FILE* file;
    bool eofSeen;
    long line;
    long column;
    int buffer[MAXLOOKAHEAD];
    int current;
    bool isSucc;
public:
    StreamCell(StreamType type, const char* source, bool fail);
    // StreamCell(StreamCell** stack, StreamType type, char* source, bool fail);
    //StreamCell(const StreamCell& orig);
    virtual ~StreamCell();
public:
    int32_t ReadChar();

    char StreamNextChar();

    friend void CloseStackedInput(StreamCell ** stack);

    friend StreamCell* OpenStackedInput(StreamCell ** stack, StreamType type, const char* source,bool fail) ;

    };

#endif /* STREAMCELL_H */

