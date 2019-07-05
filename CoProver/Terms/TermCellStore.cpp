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
#include "Global/Environment.h"
#include "TermCellStore.h"
#include "TermCell.h"

TermCellStore::TermCellStore(int hashSize) : TERM_STORE_HASH_SIZE(hashSize), TERM_STORE_HASH_MASK(hashSize - 1), entries(0), argCount(0) {
    /*E的代码： for(int i=0; i<TERM_STORE_HASH_SIZE; ++i)
     {
        store[i] = NULL;
     }*/
    //数组初始化改为：*store;//[TERM_STORE_HASH_SIZE];
    store = new TermCell*[TERM_STORE_HASH_SIZE];
    memset(store, 0, sizeof (TermCell*) * TERM_STORE_HASH_SIZE);
    storeEleNum = 0;
    this->hashConflict = 0;
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
/// 
/// \param term
/// \return 返回值, 若存在则返回存在的项(term是多余的) ,不存在 则返回null(term被插入hashtree中);

TermCell* TermCellStore::TermCellStoreInsert(TermCell* term) {

    //term->hashIdx = TermCellHash(term);
    TermCell* ret = TermTree::TermTreeInsert(&(store[ TermCellHash(term) ]), term);
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

void TermCellStore::printAllTerm(FILE* out) {
    cout << this->entries;
    for (int i = 0; i < this->TERM_STORE_HASH_SIZE; i++) {
        TermCell* elem = store[i];

        if (elem == nullptr)
            continue;
        middleTraverseSubT(out, elem);
        cout << endl;
    }
}

void TermCellStore::middleTraverseSubT(FILE* out, TermCell* root) {
    if (root == nullptr) {
        return;
    }
    middleTraverseSubT(out, root->lson);
    root->TermPrint(out);
    cout << " ";
    middleTraverseSubT(out, root->rson);
}

