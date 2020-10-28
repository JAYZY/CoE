/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   InferenceInfo.cpp
 * Author: Zhong Jian<77367632@qq.com>
 * 
 * Created on 2019年3月21日, 上午10:17
 */

#include "InferenceInfo.h"
#include "CLAUSE/Literal.h"
#include "Global/GlobalFunc.h"

string DeductPath::print() {
    string s = "#";
//    startLit->getLitInfo(s);
//    s += " -> ";
//    endLit->getLitInfo(s);
//    string strW = "";
//    strW = Float2Str(edgeWeight,2);
//    s += "(" + strW + ")";
    return s;
}






string InferenceInfo::TOInferTypeNames[] = {
    "NONE",
    "BI", //Bineray Inference
    "UD", //unit reduce 单元子句约减
    "RN", //rename
    "RD", //reduce 合一下拉
    "FACTOR",
    "SCSA",
    "SCS" //矛盾体分离
};

InferenceInfo::InferenceInfo() {
}

InferenceInfo::InferenceInfo(const InferenceInfo& orig) {
}

InferenceInfo::~InferenceInfo() {
}

