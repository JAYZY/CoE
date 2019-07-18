/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Clauseset.cpp
 * Author: Zhong Jian<77367632@qq.com>
 * 
 * Created on 2018年4月23日, 下午8:06
 */

#include "ClauseSet.h"
#include "INOUT/FileOp.h"

/*---------------------------------------------------------------------*/
/*                    Constructed Function                             */
/*---------------------------------------------------------------------*/
//

ClauseSet::ClauseSet() {
    this->members = 0;
    this->litNum = 0;
    //this->date = SysDateCreationTime();
   //SysDateInc(&this->date);

    //    this->demod_index = NULL;
    //    this->fvindex = NULL;
    //this->eval_indices.resize(4); // = PDArrayAlloc(4, 4);
    //    this->eval_no = 0;
    this->identifier = "";
}

ClauseSet::ClauseSet(const ClauseSet& orig) {
}

ClauseSet::~ClauseSet() {
    FreeAllClas();
}
/*---------------------------------------------------------------------*/
/*                  Member Function-[public]                           */
/*---------------------------------------------------------------------*/
//插入子句

void ClauseSet::InsertCla(Clause* newCla) {
    claLst.push_back(newCla);
    ++members;
    this->litNum += newCla->LitsNumber();
}
//插入子句集

long ClauseSet::InsertSet(ClauseSet* otherSet) {
    Clause* handle;
    long res = 0;
    while (!otherSet->ClauseSetEmpty()) {
        handle = otherSet->ClauseSetExtractFirst();
        this->InsertCla(handle);
        ++res;
    }
    members += res;
    return res;
}

void ClauseSet::CopyClalst(list<Clause*>&retClaSet) {
    for (auto&cla : this->claLst) {
        retClaSet.push_back(cla);
    }
}

void ClauseSet::Print(FILE* out) {
    string oriCla = "";
    for (auto&cla : this->claLst) {
        cla->getStrOfClause(oriCla);
        //cla->ClausePrint(out,true); 
    }
    FileOp::getInstance()->outInfo(oriCla);
}
//删除子句集中所有子句

void ClauseSet::FreeAllClas() {
    Clause* handle;
    while (!claLst.empty()) {
        handle = claLst.front();
        claLst.pop_front();
        DelPtr(handle);     
    }
    this->litNum=0;
    this->members=0;
            
}
//删除子句,只是从集合中删除并不是真正删除该子句
void ClauseSet::RemoveClause(Clause* cla) {
    cla->ClauseSetProp(ClauseProp::CPDeleteClause);//设置被删除子句
    this->claLst.remove(cla);
    this->litNum-= cla->LitsNumber();
    --this->members;
    //DelPtr(cla);
}

