/*
 * 实现　项的 hash 存储
 * File:   TermCellStore.h
 * Author: zj 
 *
 * Created on 2017年3月1日, 下午5:38
 */

#ifndef TERMCELLSTORE_H
#define TERMCELLSTORE_H
#include "Global/IncDefine.h"
#include "TermCell.h"
#include "TermTree.h"





//#define TERM_STORE_HASH_SIZE 32768  //(8192*4) 神仙数字　2^13*4=２^15 int的表示范围　-32768¬32767
//#define TERM_STORE_HASH_MASK (TERM_STORE_HASH_SIZE-1)

class TermCellStore {
public:
    int TERM_STORE_HASH_SIZE;
    int TERM_STORE_HASH_MASK;

    long entries; //存储的项的个数（包括子项，eg. f1(a1,a3) entries=3
    long argCount; //存储函数项的元个数　eg. f1(a1,f2(a3))  argCount=2+1=3      
    //zj-add hash冲突次数
    long hashConflict;
    long storeEleNum;

    TermCell* *store; //[TERM_STORE_HASH_SIZE];
public:
    /*---------------------------------------------------------------------*/
    /*                    Constructed Function                             */
    /*---------------------------------------------------------------------*/
    // TermCellStore();
    TermCellStore(int hashSize);
    TermCellStore(const TermCellStore& orig);
    virtual ~TermCellStore();
    void TermCellStoreExit();

    /*---------------------------------------------------------------------*/
    /*                          inline Function                            */
    /*---------------------------------------------------------------------*/

    /* 构建第一个项的hash编码 #define tcs_arity0hash(term) ((term)->fCode) */
    inline FunCode arity0hash(TermCell* term) {
        return term->fCode;
    }

    /* 构建第二个项的hash编码 #define tcs_arity1hash(term) (tcs_arity0hash(term)^(((intptr_t)(term)->args[0])>>3)) */
    inline FunCode arity1hash(TermCell* term) {
        return term->fCode ^(((intptr_t) (term)->args[0]) >> 3);
    }

    /* 构建第n个项的hash编码 */
    inline FunCode aritynhash(TermCell* term) {
        return (arity1hash(term)^(((intptr_t) (term)->args[1]) >> 4));
    }

    /* 得到term的hash编码 */
    inline FunCode TermCellHash(TermCell* term) {
//        return (((term)->arity == 0) ? arity0hash(term) : (((term)->arity == 1) ? arity1hash(term) :
//                aritynhash(term)))&TERM_STORE_HASH_MASK;
        return (((term)->arity == 0) ? arity0hash(term) : (((term)->arity == 1) ? arity1hash(term) :
                aritynhash(term)))%TERM_STORE_HASH_SIZE;//  TERM_STORE_HASH_MASK;
    }

    inline FunCode BKDRHash(const TermCell* termPtr) {
        //UINT32 seed=31;
        FunCode hash = 15 * termPtr->fCode;

        for (int i = 0; i < termPtr->arity; ++i) {
            hash = (hash * 15 + BKDRHash(termPtr->args[i])); //转换为正数;
        }
        return hash & LONG_MAX;
    }

    inline FunCode BKDRHash1(const TermCell* termPtr) {
        //UINT32 seed=31;
        FunCode hash = 131 + termPtr->fCode;
        //hash = hash * 131 + (intptr_t) (termPtr->args[0])>>3;
        //hash = hash * 131 + (intptr_t) (termPtr->args[1])>>4; // /*& 0x7FFFFFFF*/); //转换为正数;

        return hash & LONG_MAX;
    }

    inline FunCode BKDRHash2(const TermCell* term) {
        //UINT32 seed=31;

        long hash = 15 * term->fCode;
        for (int i = 0; i < term->arity; ++i) {
            hash = hash * 15 + ((intptr_t) (term->args[i])>>(3 + i)); // /*& 0x7FFFFFFF*/); //转换为正数;           
        }
        return hash & LONG_MAX/*& 0x7FFFFFFF*/;

    }

    inline long TermCellStoreNodes() {
        return entries;
    }




    /*---------------------------------------------------------------------*/
    /*                       Member Function                               */
    /*---------------------------------------------------------------------*/

    /* Insert a term cell into the store.  */
    TermCell* TermCellStoreInsert(TermCell* term);

    TermCell* TermCellStoreFind(TermCell* term) {
        return TermTree::TermTreeFind(&(store[TermCellHash(term)]), term);
    }

    /*****************************************************************************
     * Extract a term cell from the store, return it. 
     ****************************************************************************/
    TermCell* TermCellStoreExtract(TermCell* term) {
        TermCell* ret;
        ret = TermTree::TermTreeExtract(&(store[TermCellHash(term)]), term);
        if (ret) {
            entries--;
            argCount -= term->arity;
        }
        assert(entries >= 0);
        return ret;
    }

    /* 从伸展树中删除树节点 -- Delete a node from the store. */
    bool TermCellStoreDelete(TermCell* term);


    /* 返回,所有项个数 -- Return the number of nodes in the term cell store. */
    long TermCellStoreCountNodes();
    void printAllTerm(FILE* out);


private:
    void middleTraverseSubT(FILE* out, TermCell* root);
};

#endif /* TERMCELLSTORE_H */

