/* 
 * File:   GroundTermBank.cpp
 * Author: Zhong Jian<77367632@qq.com>
 * Contents: 全局基项存储
 * Created on 2018年11月1日, 上午10:37
 */

#include "GroundTermBank.h"
#include "TermCell.h"

GroundTermBank::GroundTermBank() {
    extIndex.reserve(100000); //注意，只是预留10W空间，并没有初始化，(初始化可以考虑用resize)



    //初始化 TermCellStore
    termStore = TermCellStore();

    //创建一个特殊TrueTerm 
    TermCell* tmpTerm = new TermCell((FunCode) DerefType::TRUECODE, 0);
    tmpTerm->TermCellSetProp(TermProp::TPPredPos); //默认谓词符号

    trueTerm = GBInsert(tmpTerm, nullptr, DerefType::DEREF_NEVER);
    TermCell::TermFree(tmpTerm); //TermFree(term);

    //创建一个特殊FalseTerm 
    tmpTerm = new TermCell((FunCode) DerefType::FLASECODE);
    tmpTerm->TermCellSetProp(TermProp::TPPredPos);
    falseTerm = GBInsert(tmpTerm, nullptr, DerefType::DEREF_NEVER);

    TermCell::TermFree(tmpTerm); //TermFree(term);
    minTerm = nullptr;
}

GroundTermBank::GroundTermBank(const GroundTermBank& orig) {
}

GroundTermBank::~GroundTermBank() {
}

/*---------------------------------------------------------------------*/
/*                  Member Function-[public]                           */

/*---------------------------------------------------------------------*/
TermCell* GroundTermBank::GBInsert(TermCell* term) {
    assert(term);
    assert(0 == term->uVarCount);


    for (int i = 0; i < term->arity; ++i) {
        term->args[i] = GBInsert(term->args[i]);
    }
    term = GTermTopInsert(term);

    return term;
}
/*****************************************************************************
 * 插入项　t 到　TermBank 中．项t中的每个子项均已经存在于termBank中
 * Will reuse or destroy the top cell!
 ****************************************************************************/
TermCell* GroundTermBank::GTermTopInsert(TermCell* t) {
    assert(t);
    assert(!t->IsVar()); //确保插入的项不能是变元

    TermCell* newTerm = termStore.TermCellStoreInsert(t); //插入新项到 termStore中

    if (newTerm) /* TermCell node already existed, just add properties */ {
        newTerm->properties = (TermProp) ((int32_t) newTerm->properties | (int32_t) t->properties)/*& bank->prop_mask*/;
        DelPtr(t);
        return newTerm;
    } else {
        t->entryNo = ++(inCount);
        

        /* Groundness may change below */
        t->TermCellSetProp(TermProp::TPShareGround);

        t->weight = DEFAULT_FWEIGHT;

        for (int i = 0; i < t->arity; ++i) {
            assert(t->args[i]->IsShared() || t->args[i]->IsVar());

            t->weight += t->args[i]->weight;
            if (!t->args[i]->TermCellQueryProp(TermProp::TPIsGround)) {
                t->TermCellDelProp(TermProp::TPIsGround);
            }
        }
        assert(t->TermStandardWeight() == t->TermWeight(DEFAULT_VWEIGHT, DEFAULT_FWEIGHT));
        assert((t->IsGround() == 0) == (t->TBTermIsGround() == 0));
    }
    return t;
}