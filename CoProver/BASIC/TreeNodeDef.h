/*
 * 定义二叉树的　不同节点类型．
 * 注意：所有二叉树　均使用STL 中的　set 数据结构
 * File:   TreeNodeDef.h
 * Author: zj 
 *
 * Created on 2017年3月4日, 下午4:35
 */
#ifndef TREENODEDEF_H
#define TREENODEDEF_H
#include "Global/IncDefine.h"
 
//这里的宏定义，定义了指针的大小比较
//#define PCmp(p1, p2)    (PGreater(p1, p2)-PLesser(p1,p2))  

#define PCmp(p1, p2)    (    ((uintptr_t)p1 > (uintptr_t)p2) - ((uintptr_t)p1 < (uintptr_t)p2))
/*神奇的代码return 1 if the first one is bigger, 0 if both are equal, and -1 if the second one is bigger.*/
#define PEqual(p1,p2)   ((uintptr_t)(p1))==((uintptr_t)(p2))
#define PGreater(p1,p2) ((uintptr_t)(p1))> ((uintptr_t)(p2))
#define PLesser(p1,p2)  ((uintptr_t)(p1))< ((uintptr_t)(p2))

/*****************************************************************************
 * float 树节点 
 ****************************************************************************/
class FloatTreeCell {
public:
    double key;
    IntOrP val1;
    IntOrP val2;
    FloatTreeCell *lson;
    FloatTreeCell *rson;
    //构造函数

    FloatTreeCell() : key(0.0), lson(NULL), rson(NULL) {
        val1.i_val = 0;
        val2.i_val = 0;
    };

    bool operator<(const FloatTreeCell&a2)const {
        return key < a2.key;
    }

    inline int CompareObj(const FloatTreeCell* a2) {
        if (key < a2->key) return -1;
        if (key > a2->key) return 1;
        return 0;
    }

    inline int Compare(double k) {
        if (key < k) return -1;
        if (key > k) return 1;
        return 0;
    }
};
typedef FloatTreeCell *FloatTree_p;

/***************************************************************************** 
 * long 树节点
 ****************************************************************************/
class NumTreeCell {
public:
    long key;
    IntOrP val1;
    IntOrP val2;
    NumTreeCell *lson;
    NumTreeCell *rson;

    //构造函数

    NumTreeCell() : key(0), lson(NULL), rson(NULL) {
        val1.i_val = val2.i_val = 0;
    }

    NumTreeCell(long k) : key(k), lson(nullptr), rson(nullptr) {
        val1.i_val = val2.i_val = 0;
    }
     
    
    bool operator<(const NumTreeCell& a2)const {
        return key < a2.key;
    }

    inline int CompareObj(const NumTreeCell* a2) {
        if (key < a2->key) return -1;
        if (key > a2->key) return 1;
        return 0;
    }

    inline int Compare(const long& k) {
        if (key < k) return -1;
        if (key > k) return 1;
        return 0;
    }
};
typedef NumTreeCell *NumTree_p;

/*****************************************************************************
 * string 树节点
 ****************************************************************************/
class StrTreeCell {
public:
    string key;
    IntOrP val1;
    IntOrP val2;
    StrTreeCell *lson;
    StrTreeCell *rson;
public:

    StrTreeCell() : key(""), lson(NULL), rson(NULL) {
        val1.i_val = val2.i_val = 0;
    }

    StrTreeCell(const string& k, IntOrP valA, IntOrP valB, StrTreeCell* lt, StrTreeCell* rt) :
    key(k) {
        val1 = valA;
        val2 = valB;
        this->lson = lt;
        this->rson = rt;
    };

    bool operator<(const StrTreeCell&a2)const {
        return key.compare(a2.key) < 0;
    }

    inline int CompareObj(const StrTreeCell* a2) {
        return key.compare(a2->key);
    }

    inline int Compare(const string& k) {
        return key.compare(k);
    }
    ~StrTreeCell(){key.shrink_to_fit();}
};
typedef StrTreeCell *StrTree_p;

/*****************************************************************************
 *  ptr 指针 树节点
 ****************************************************************************/
class PTreeCell {
public:
    void* key;
    IntOrP val1;
    IntOrP val2;
    PTreeCell *lson;
    PTreeCell *rson;
public:

    PTreeCell() {
        key = nullptr;
        lson = nullptr;
        rson = nullptr;
        val1.i_val = val2.i_val = 0;
    }

    bool operator<(const PTreeCell&a2)const {
        return PCmp(key, a2.key) < 0;
    }

    inline int CompareObj(const PTreeCell* a2) {
        return PCmp(key, a2->key);
    }

    inline int Compare(const void*k) {
        return PCmp(key, k);
    }

    inline long PObjTreeNodes() {
        PTreeCell* root = this;
        //PStack_p stack = PStackAlloc();
        vector<PTreeCell*> vs;
        long res = 0;

        //PStackPushP(stack, root);
        vs.push_back(root);
        while (!vs.empty())//PStackEmpty(stack))
        {
            root = vs.back(); // PStackPopP(stack);
            vs.pop_back();
            if (root) {
                //PStackPushP(stack, root->lson);
                vs.push_back(root->lson);
                //PStackPushP(stack, root->rson);
                vs.push_back(root->rson);
                res++;
            }
        }
        vs.clear();
        //PStackFree(stack);

        return res;
    }
};
typedef PTreeCell *PTree_p;
//
//class ObjTreeCell {
//public:
//    void* key;
//    IntOrP val1;
//    IntOrP val2;
//    ObjTreeCell *lson;
//    ObjTreeCell *rson;
//    ComparisonFunctionType cmpFun;
//public:
//
//    bool operator<(const ObjTreeCell&a2)const {
//         assert(cmpFun);
//        return cmpFun(key, a2.key) < 0;
//    }
//
//    inline int CompareObj(const ObjTreeCell* a2) {
//         assert(cmpFun);
//        return cmpFun(key, a2->key);
//    }
//
//    inline int Compare(const void*k) {
//        assert(cmpFun);
//        return cmpFun(key, k);
//    }
//
//    /*---------------------------------------------------------------------*/
//    /*                    Constructed Function                             */
//
//    /*---------------------------------------------------------------------*/
//    ObjTreeCell(ComparisonFunctionType cFun) :
//    key(NULL), lson(NULL), rson(NULL), cmpFun(cFun) {
//        val1.i_val = val2.i_val = 0;
//    }
//
//    ~ObjTreeCell() {
//    }
//
//};
#endif /* BINARYTREE_H */

