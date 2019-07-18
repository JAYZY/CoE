

/*****************************************************************************
 * TermTree 使用伸展树的方式　来存储　TermCell  相同hash值的项存储到同一个　TermTree中
 ****************************************************************************
 * File:   TermTree.h
 * Author: zj 
 * Created on 2017年3月3日, 上午11:34
 */

#ifndef TERMTREE_H
#define TERMTREE_H

#include "TermCell.h"

class TermTree {
private:
    static TermCell* SplayTermTree(TermCell* tree, TermCell* splay);
public:
    static void TermTreeFree(TermCell* junk);
    static int TermTopCompare(TermCell* t1, TermCell* t2);
    static TermCell* TermTreeFind(TermCell* *root, TermCell* term);
    static TermCell* TermTreeInsert(TermCell* *root, TermCell* term);
    static TermCell* TermTreeExtract(TermCell* *root, TermCell* term);
    static bool TermTreeDelete(TermCell* *root, TermCell* term);
    static void TermTreeSetProp(TermCell* root, TermProp props);
    static void TermTreeDelProp(TermCell* root, TermProp props);
    static long TermTreeNodes(TermCell* root);
    /* zj新增方法:遍历TermTree 将结点放入一个新的树中 */
    static void TermTreeTraverseInit(TermCell* t, SplayTree<NumTreeCell>&splayTree);

    /*---------------------------------------------------------------------*/
    /*                    Constructed Function                             */
    /*---------------------------------------------------------------------*/
    TermTree();
    TermTree(const TermTree& orig);
    virtual ~TermTree();
};


#endif /* TERMTREE_H */

