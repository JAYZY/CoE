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
/// 根据file name 创建一个文件流, 若 type==nullptr name='-' 表示为标准输入 stdin
/// \param type 
/// \param source
/// \param fail

StreamCell::StreamCell(StreamType type, char* source, bool fail)
: source(""), streamType(type) {
    //当为一个file,来解析
    if (type == nullptr) {
        if(!source || !strcmp(source,"-") ){
            this->source="<stdin>";
            this->file=stdin;            
        }
        else{
            this->source=source;
            //this->file=InputOpen()
        }
            



    }
}

StreamCell::StreamCell(const StreamCell& orig) {
}

StreamCell::~StreamCell() {
}

