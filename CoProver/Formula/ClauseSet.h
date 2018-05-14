/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Clauseset.h
 * Author: Zhong Jian<77367632@qq.com>
 *
 * Created on 2018年4月23日, 下午8:06
 */

#ifndef CLAUSESET_H
#define CLAUSESET_H
#include "CLAUSE/Clause.h"
#include "Global/SysDate.h"

class ClauseSet {
private:
    // long members; /* How many clauses are there? */
    long literals; /* And how many literals? */

    SysDate date; /* Age of the clause set, used for optimizing rewriting. 
                * The special date SysCreationDate() is used to indicate 
                * ignoring of dates when checking for irreducability. */
    list<Clause*> claSet; /* The clauses */
    string identifier;
    //子句评估相关
    vector<int>eval_indices; // = PDArrayAlloc(4, 4);
    int eval_no;
public:
    /*---------------------------------------------------------------------*/
    /*                    Constructed Function                             */
    /*---------------------------------------------------------------------*/
    ClauseSet();
    ClauseSet(const ClauseSet& orig);
    virtual ~ClauseSet();
public:
    /*---------------------------------------------------------------------*/
    /*                       Inline  Function                              */

    /*---------------------------------------------------------------------*/
    inline bool ClauseSetEmpty() {
        return claSet.empty();
    }
    /// 返回子句集大小(子句个数)

    inline long Members() {
        return claSet.size();
    }

    inline Clause* ClauseSetExtractFirst() {
        Clause* handle = claSet.front();
        claSet.pop_front();
        return handle;
    }
    inline void Sort(){
        claSet.sort(
        [](Clause*c1,Clause*c2)->bool{return c1->LitsNumber()>c2->LitsNumber();});
    }
    /*---------------------------------------------------------------------*/
    /*                  Member Function-[public]                           */
    /*---------------------------------------------------------------------*/
    void InsertCla(Clause* newCla);
    long InsertSet(ClauseSet* set);

    void CopyClalst(list<Clause*>&retClaSet);
    
    void Print();
    
    /// 删除所有子句

    void FreeClauses();
    void RemoveClause(Clause* cla);
};

#endif /* CLAUSESET_H */

