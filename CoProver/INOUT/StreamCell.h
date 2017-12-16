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
    enum {MAXLOOKAHEAD = 64};    
public:
   StreamCell*      next;
   string           source;
   StreamType       streamType; /* Only constant strings allowed here! */
   long             stringPos;
   FILE*            file;
   bool             eofSeen;
   long             line;
   long             column;
   int              buffer[MAXLOOKAHEAD];
   int              current;
public:
    StreamCell(StreamType type,char* source,bool fail);
    StreamCell(const StreamCell& orig);
    virtual ~StreamCell();
public:
    int32_t ReadChar();
    void OpenStackedInput(StreamCell** stack);

};

#endif /* STREAMCELL_H */

