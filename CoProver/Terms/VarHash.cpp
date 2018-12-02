/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   VarHash.cpp
 * Author: zj 
 * 
 * Created on 2017年3月10日, 上午10:08
 */

#include "VarHash.h"

/*---------------------------------------------------------------------*/
/*                    VarHashCell 相关方法　                            */
/*---------------------------------------------------------------------*/

/*****************************************************************************
 * 如hash数组中存在项var,存储次数val++,否则创建一个新的hash节点
 * Return the stored value. 
 ****************************************************************************/
long VarHashCell::VarHashAddValue(TermCell* var, long value) {
    //得到var的hash值
    int i = VarHashFunction(var);
    //得到var所对应的hash节点       
    VarHashEntry_p entry = hash[i]->VarHashListFind(var);
    if (entry) {
        entry->val += value; //存储次数加１
    } else {//存在冲突，冲突节点总是头节点
        entry = new VarHashEntryCell(var, value);
        entry->next = hash[i];
        hash[i] = entry;
    }
    return entry->val;
}

/***************************************************************************** 
 * Scans a term and adds the variable occurences to the hash, 
 * with each occurence being counted with the "add" value.
 * 主要运用与KBO中
 ****************************************************************************/
void VarHashCell::VarHashAddVarDistrib(TermCell* term, DerefType deref, long add) {
    vector<TermCell*> st;
    vector<DerefType>stDef;
    st.push_back(term);
    stDef.push_back(deref);
    while (!st.empty()) {
        deref = stDef.back();
        stDef.pop_back();
        term = st.back();
        st.pop_back();
        term = TermCell::TermDeref(term, deref);
        if (term->IsVar()) {
            VarHashAddValue(term, add);
        } else {
            for (int i = 0; i < term->arity; ++i) {
                st.push_back(term->args[i]);
                stDef.push_back(deref);
            }
        }
    }
    vector<TermCell*>().swap(st);
    vector<DerefType>().swap(stDef);
}
/*---------------------------------------------------------------------*/
/*                    Constructed Function                             */

/*---------------------------------------------------------------------*/
VarHashCell::VarHashCell() {
    memset(hash, 0, sizeof (VarHashEntry_p) * VAR_HASH_SIZE);
}

VarHashCell::VarHashCell(const VarHashCell& orig) {
}

VarHashCell::~VarHashCell() {
    for (int i = 0; i < VAR_HASH_SIZE; ++i) {
        delete(hash[i]);
    }
}

