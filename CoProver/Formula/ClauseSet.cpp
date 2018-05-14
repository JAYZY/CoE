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

/*---------------------------------------------------------------------*/
/*                    Constructed Function                             */
/*---------------------------------------------------------------------*/
//

ClauseSet::ClauseSet() {
   // this->members = 0;
    this->literals = 0;
    this->date = SysDateCreationTime();
    SysDateInc(&this->date);

    //    this->demod_index = NULL;
    //    this->fvindex = NULL;
    this->eval_indices.resize(4); // = PDArrayAlloc(4, 4);
    //    this->eval_no = 0;
    this->identifier = "";
}

ClauseSet::ClauseSet(const ClauseSet& orig) {
}

ClauseSet::~ClauseSet() {
    FreeClauses();
}
/*---------------------------------------------------------------------*/
/*                  Member Function-[public]                           */
/*---------------------------------------------------------------------*/
//插入子句
void ClauseSet::InsertCla( Clause* newCla) {
    
    claSet.push_back(newCla);
    this->literals+=newCla->LitsNumber();
}
//插入子句集
long ClauseSet::InsertSet(ClauseSet* otherSet){
    Clause* handle;
    long res = 0;
    while (!otherSet->ClauseSetEmpty()) {
        handle = otherSet->ClauseSetExtractFirst();
        this->InsertCla(handle);
        res++;
    }
    return res;
}


 void ClauseSet::CopyClalst(list<Clause*>&retClaSet){
     for(auto&cla:this->claSet){
         retClaSet.push_back(cla);
     }
 }
void ClauseSet::Print(){
    for(auto&cla:this->claSet){
        cla->ClausePrint(stdout,true);
        
        cout<<endl;
    }
}
//删除子句集中所有子句
void ClauseSet::FreeClauses() {
    Clause* handle;
    while (!claSet.empty()) {
        handle = claSet.front();
        claSet.pop_front();
        RemoveClause(handle);
    }
}
//删除子句
void ClauseSet::RemoveClause(Clause* cla) {
    this->literals -= cla->LitsNumber();       
    DelPtr(cla);
}

