/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   StreamCell.cpp
 * Author: Zhong Jian<77367632@qq.com>
 * 
 * Created on 2017年12月15日, 上午9:20
 */

#include "StreamCell.h"
#include "FileOp.h"
/// 根据file name 创建一个文件流, 若 type==nullptr name='-' 表示为标准输入 stdin
/// \param type 
/// \param source
/// \param fail

StreamCell::StreamCell(StreamType type, const char* source, bool fail)
: source(""), streamType(type) {
    this->isSucc=false;
    //当为一个file,来解析
    if (type == nullptr) {
        if (!source || !strcmp(source, "-")) {
            this->source = "<stdin>";
            this->file = stdin;
        } else {
            this->source = source;
            this->file = FileOp::InputOpen(source, fail);
            if (!this->file) {
                this->source = "";
            }
        }
        Out::Err("Opened ", this->source.c_str());
    } else {
        this->source = source;
        this->stringPos = 0;
    }
    this->next = nullptr;
    this->eofSeen = false;
    this->line = 1;
    this->column = 1;
    this->current = 0;
    this->isSucc=true;
    for (int i = 0; i < MAXLOOKAHEAD; i++) {
        this->buffer[i] = ReadChar();
    }
}


//StreamCell::StreamCell(const StreamCell& orig) {
//}

StreamCell::~StreamCell() {
    if (this->streamType == nullptr) {
        if (this->file != stdin) {
            if (fclose(this->file) != 0) {
                TmpErrno = errno;
                sprintf(ErrStr, "Cannot close file %s", this->source.c_str());
                Out::SysError(ErrStr, ErrorCodes::FILE_ERROR);
            }
        }
        Out::Err("Closing ", this->source.c_str());
    }
}

/*-----------------------------*/
/// 读一个char 并返回
/// \return 

int32_t StreamCell::ReadChar() {
    int32_t ch;
    if (this->eofSeen) {
        ch = EOF;
    } else {
        if (this->streamType != nullptr) {
            ch = (int32_t) this->source[this->stringPos];
            if (ch) {
                ++this->stringPos;
            } else {
                ch = EOF;
                this->eofSeen = true;
            }
        } else {
            ch = getc(this->file);
            if (ch == EOF) {
                this->eofSeen = true;
            }
        }
    }
    return ch;
}

/*-----------------------------------------------------------------------
//
// Function: StreamNextChar()
//
//   Move the current window on the input stream one character
//   forward. Return the new CurrChar().
//
// Global Variables: -
//
// Side Effects    : Reads one character from the input, update the
//                   stream information about the current position.
//
/----------------------------------------------------------------------*/

char StreamCell::StreamNextChar() {
    if (StreamCurrChar() == '\n') {
        this->line++;
        this->column = 1;
    } else {
        this->column++;
    }
    this->current = StreamRealPos(current + 1);
    this->buffer[StreamRealPos(current + MAXLOOKAHEAD - 1)] = ReadChar();
    return (char) StreamCurrChar();
}

void CloseStackedInput(StreamCell ** stack) {
    StreamCell* handle;

    assert(*stack);
    handle = *stack;
    *stack = handle->next;
    delete handle;
    //DestroyStream(handle);
}

StreamCell* OpenStackedInput(StreamCell ** stack, StreamType type, const char* source,bool fail) {
    StreamCell* handle = new StreamCell(type, source,fail);
    if (handle->isSucc) {
        handle->next = *stack;
        *stack = handle;
    }
    return handle;
}


