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
    uint32_t members; /* 该子句中有多少子句,注意,删除的子句会放入claLst 末尾,所以 mmembers!= claLst.sizes */
    uint32_t litNum; /* And how many literals? */
   // SysDate date; /* Age of the clause set, used for optimizing rewriting. The special date SysCreationDate() is used to indicate  ignoring of dates when checking for irreducability. */
    list<Clause*> claLst; /* The clauses */
    string identifier;
    //子句评估相关
   // vector<int>eval_indices; // = PDArrayAlloc(4, 4);
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
        return claLst.empty();
    }
    /// 返回子句集大小(子句个数)

    inline long Members() {
        return claLst.size();
    }

    inline Clause* ClauseSetExtractFirst() {
        Clause* handle = claLst.front();
        claLst.pop_front();
        return handle;
    }
    //按文字降序排列

    inline void SortByLitNumDesc() {
        claLst.sort(
                [](Clause*c1, Clause * c2)->bool {
                    return c1->LitsNumber() > c2->LitsNumber();
                });
    }
    //按文字升序排列

    inline void SortByLitNumAsc() {
        claLst.sort(
                [](Clause*c1, Clause * c2)->bool {
                    return c1->LitsNumber() < c2->LitsNumber();
                });
    }
    
    inline list<Clause*>* getClaSet(){return &claLst;}
    /*---------------------------------------------------------------------*/
    /*                  Member Function-[public]                           */
    /*---------------------------------------------------------------------*/
    void InsertCla(Clause* newCla);
    long InsertSet(ClauseSet* set);

    void CopyClalst(list<Clause*>&retClaSet);

    void Print(FILE*out);

    /// 删除所有子句

    void FreeAllClas();
    void RemoveClause(Clause* cla);
};

#endif /* CLAUSESET_H */

