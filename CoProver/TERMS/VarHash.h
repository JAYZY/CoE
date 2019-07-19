/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   VarHash.h
 * Author: zj 
 *
 * Created on 2017年3月10日, 上午10:08
 */

#ifndef VARHASH_H
#define VARHASH_H
#include "VarBank.h"
#include "TermCell.h"

/*---------------------------------------------------------------------*/
/*                            宏定义　                                  */
/*---------------------------------------------------------------------*/

#define VAR_HASH_SIZE 16 /* 2^n */
#define VAR_HASH_MASK (VAR_HASH_SIZE - 1)  /* 2^n-1 */

/* 
 * hash节点类，组成hash链表
 */
class VarHashEntryCell {
public:
    TermCell* key; //项term 
    long val; //表示该项存储次数？
    VarHashEntryCell *next; //有冲突的项
public:

    /*****************************************************************************
     * Find an entry in the linear list of hash entries. Return NULL on failure.  
     ****************************************************************************/
    VarHashEntryCell* VarHashListFind(TermCell* var) {
        VarHashEntryCell* list = this;
        while (list) {
            if (list->key == var) {
                break;
            }
            list = list->next;
        }
        return list;
    }
    /*---------------------------------------------------------------------*/
    /*                    Constructed Function                             */

    /*---------------------------------------------------------------------*/
    VarHashEntryCell(TermCell* var, long value) :
    key(var), val(value), next(NULL) {
    };

    ~VarHashEntryCell() {
        VarHashEntryCell* handle;
        VarHashEntryCell* list = next;
        while (list) {
            handle = list->next;
            delete(list);
            list = handle;
        }
    }
}
typedef VarHashEntryCell, *VarHashEntry_p;

/* 
 * hash数组，存储不同hash值的首节点 
 */
class VarHashCell {
public:
    VarHashEntry_p hash[VAR_HASH_SIZE];
public:

    /*****************************************************************************
     * 返回项var所对应的hash节点 (NULL if non-existant). 
     ****************************************************************************/
    inline VarHashEntry_p VarHashFind(TermCell* var) {
        //得到var的hash值
        int i = VarHashFunction(var);
        //得到var所对应的hash节点
        return hash[i]->VarHashListFind(var);
    }

    /*****************************************************************************
     * 根据term的fCode计算hash值，该值对应　hash的下标 
     ****************************************************************************/
    inline int VarHashFunction(TermCell* var) {
        assert(var->fCode < 0);
        //assert(var->fCode < 0);
        return (-(var->fCode) & VAR_HASH_MASK);
    }

    /*****************************************************************************
     * 如hash数组中存在项var,存储次数val++,否则创建一个新的hash节点
     * Return the stored value. 
     ****************************************************************************/
    long VarHashAddValue(TermCell* var, long value);

    /***************************************************************************** 
     * Scans a term and adds the variable occurences to the hash, 
     * with each occurence being counted with the "add" value.
     * 主要运用与KBO中
     ****************************************************************************/
    void VarHashAddVarDistrib(TermCell* term, DerefType deref, long add);


    /*---------------------------------------------------------------------*/
    /*                    Constructed Function                             */
    /*---------------------------------------------------------------------*/
    VarHashCell();
    VarHashCell(const VarHashCell& orig);
    virtual ~VarHashCell();
private:

};
typedef VarHashCell *VarHash_p;
#endif /* VARHASH_H */

