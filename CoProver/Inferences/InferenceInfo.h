/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   InferenceInfo.h
 * Author: Zhong Jian<77367632@qq.com>
 *
 * Created on 2019年3月21日, 上午10:17
 */

#ifndef INFERENCEINFO_H
#define INFERENCEINFO_H
#include "Global/IncDefine.h"
//#include <bits/stdint-uintn.h>

enum class InfereType : uint16_t {
    RN, //rename
    SCS//矛盾体分离    
};

class InferenceInfo {
private:
    static string TOInferTypeNames[];

public:
    InferenceInfo();
    InferenceInfo(const InferenceInfo& orig);
    virtual ~InferenceInfo();
public:
    static inline string getStrInfoType(InfereType inferType){
        return TOInferTypeNames[(uint16_t)inferType];
    }

};

#endif /* INFERENCEINFO_H */

