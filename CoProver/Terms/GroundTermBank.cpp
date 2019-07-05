/* 
 * File:   GroundTermBank.cpp
 * Author: Zhong Jian<77367632@qq.com>
 * Contents: 全局基项存储
 * Created on 2018年11月1日, 上午10:37
 */

#include "GroundTermBank.h"

GroundTermBank::GroundTermBank() {
    extIndex.reserve(1000); //注意，只是预留10W空间，并没有初始化，(初始化可以考虑用resize)

    //初始化 TermCellStore
    termStore = TermCellStore();

    //创建一个特殊TrueTerm 
    TermCell* tmpTerm = new TermCell((FunCode) DerefType::TRUECODE, 0);
    tmpTerm->TermCellSetProp(TermProp::TPPredPos); //默认谓词符号

    trueTerm = GBInsert(tmpTerm);

    // TermCell::TermFree(tmpTerm); //TermFree(term);

    //创建一个特殊FalseTerm 
    tmpTerm = new TermCell((FunCode) DerefType::FLASECODE);
    tmpTerm->TermCellSetProp(TermProp::TPPredPos);

    falseTerm = GBInsert(tmpTerm);
    
    tmpTerm = nullptr;
    // TermCell::TermFree(tmpTerm); //TermFree(term);
    minTerm = nullptr;
}

GroundTermBank::GroundTermBank(const GroundTermBank& orig) {
}

GroundTermBank::~GroundTermBank() {
}

/*---------------------------------------------------------------------*/
/*                  Member Function-[public]                           */
/*---------------------------------------------------------------------*/
//

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
    assert(0 == t->uVarCount);
    TermCell* newTerm = termStore.TermCellStoreInsert(t); //插入新项到 termStore中

    if (newTerm) /* TermCell 已经存在,就需要delete t. node already existed, just add properties */ {
        newTerm->properties = (TermProp) ((int32_t) newTerm->properties | (int32_t) t->properties)/*& bank->prop_mask*/;
        DelPtr(t);
        return newTerm;
    } else {
        t->entryNo = ++(inCount);
        /* Groundness may change below */
        t->TermCellSetProp(TermProp::TPIsGround);
//        t->weight = DEFAULT_FWEIGHT;
//        for (int i = 0; i < t->arity; ++i) {
//            assert(t->args[i]->IsShared() || t->args[i]->IsVar());
//            t->weight += t->args[i]->weight;
//            if (!t->args[i]->TermCellQueryProp(TermProp::TPIsGround)) {
//                t->TermCellDelProp(TermProp::TPIsGround);
//            }
//        }
       
        assert(t->weight == t->TermWeight(DEFAULT_VWEIGHT, DEFAULT_FWEIGHT));
        assert((t->IsGround() == 0) == (t->TBTermIsGround() == 0));
    }
    return t;
}


void GroundTermBank::GTPrintAllTerm(FILE *out) {

    this->termStore.printAllTerm(out);
   

    
//    
//    //中序遍历 伸展树 注意.spNode是节点
//    if (spNode == nullptr) {
//        return;
//    }
//    GTPrintAllTerm(out, spNode->lson);
//
//    TermCell* term = (TermCell*) spNode->val1.p_val;
//
//    fprintf(out, "*%ld : ", term->entryNo);
//
//    if (term->IsVar()) {
//        term->VarPrint(out);
//    } else {
//        string sigName;
//        Env::getSig()->SigFindName(term->fCode, sigName);
//        fputs(sigName.c_str(), out);
//        if (!term->IsConst()) {
//            assert(term->arity >= 1);
//            assert(term->args);
//            putc('(', out);
//
//            fprintf(out, "*%ld", term->args[0]->TBCellIdent());
//            for (int i = 1; i < term->arity; ++i) {
//                putc(',', out);
//                fprintf(out, "*%ld", term->args[i]->TBCellIdent());
//            }
//            putc(')', out);
//        }
//        printf("   =   ");
//        term->TermPrint(out, DerefType::DEREF_NEVER);
//        //zj-add
//        fprintf(out, " :weight->%lf", term->zjweight);
//    }
//    if (TermBank::TBPrintInternalInfo) {
//        fprintf(out, "\t/*  Properties: %10d */", (int) term->properties);
//    }
//    fprintf(out, "\n");
//    GTPrintAllTerm(out, spNode->rson);
}
