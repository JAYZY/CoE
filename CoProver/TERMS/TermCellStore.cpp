/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   TermCellStore.cpp
 * Author: zj 
 * 
 * Created on 2017年3月1日, 下午5:38
 */

#include <vector>

#include "TermCellStore.h"
#include "TermCell.h"

TermCellStore::TermCellStore() : entries(0), argCount(0) {
    /*E的代码： for(int i=0; i<TERM_STORE_HASH_SIZE; ++i)
     {
        store[i] = NULL;
     }*/
    //数组初始化改为：
    memset(store, 0, sizeof (TermCell*) * TERM_STORE_HASH_SIZE);
}

TermCellStore::TermCellStore(const TermCellStore& orig) {
}

TermCellStore::~TermCellStore() {
    //TermCellStoreExit();
}

/*---------------------------------------------------------------------*/
/*                  Member Function-[public]                           */
/*---------------------------------------------------------------------*/

/* Insert a term cell into the store.  */
TermCell* TermCellStore::TermCellStoreInsert(TermCell* term) {
    TermCell* ret;
    ret = TermTree::TermTreeInsert(&(store[TermCellHash(term)]), term);
    if (!ret) {
        entries++;
        argCount += term->arity;
    }
    return ret;
}

void TermCellStore::TermCellStoreExit() {
    for (int i = 0; i < TERM_STORE_HASH_SIZE; ++i) {
        TermTree::TermTreeFree(store[i]);
        store[i] = nullptr;
    }
}

/***************************************************************************** 
 * 从伸展树中删除树节点 -- Delete a node from the store.
 ****************************************************************************/
bool TermCellStore::TermCellStoreDelete(TermCell* term) {
    bool ret;
    ret = TermTree::TermTreeDelete(&(store[TermCellHash(term)]), term);
    if (ret) {
        entries--;
        argCount -= term->arity;
    }
    assert(entries >= 0);
    return ret;
}

/*****************************************************************************
 * 返回,所有项个数 -- Return the number of nodes in the term cell store.
 ****************************************************************************/
long TermCellStore::TermCellStoreCountNodes() {
    long res = 0;
    for (int i = 0; i < TERM_STORE_HASH_SIZE; ++i) {
        res += TermTree::TermTreeNodes(store[i]);
    }
    return res;
}