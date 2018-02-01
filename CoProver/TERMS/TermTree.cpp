/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   TermTree.cpp
 * Author: zj 
 * 
 * Created on 2017年3月3日, 上午11:34
 */

#include <vector>

#include "TermTree.h"

/* zj新增方法:遍历TermTree 将结点放入一个新的树中 */
void TermTree::TermTreeTraverseInit(TermCell* t, SplayTree<NumTreeCell>&sTree) {
    if(t==nullptr) return;
         
    vector<TermCell*> st;
    st.push_back(t);
    while (!st.empty()) {
        t = st.back();
        st.pop_back();
        NumTree_p newNode = new NumTreeCell();
        newNode->key = t->entryNo;
        newNode->val1.p_val = t;
        sTree.Insert(newNode);
        if (t->lson) {
            
            st.push_back(t->lson);
            //st.push_back(t->rson);
        }
    }
    vector<TermCell*>().swap(st);
}

/*****************************************************************************
 * TermTree 使用伸展树的方式　来存储　TermCell  相同hash值的项存储到同一个　TermTree中
 ****************************************************************************/
TermCell* TermTree::SplayTermTree(TermCell* tree, TermCell* splay) {

    int cmpres;

    if (!tree) {
        return tree;
    }
    TermCell* tmp;
    TermCell newTerm;
    newTerm.lson = nullptr;
    newTerm.rson = nullptr;
    TermCell* left = &newTerm;
    TermCell* right = &newTerm;

    for (;;) {
        cmpres = TermTopCompare(splay, tree);
        if (cmpres < 0) {
            if (!tree->lson) {
                break;
            }
            if (TermTopCompare(splay, tree->lson) < 0) {
                tmp = tree->lson;
                tree->lson = tmp->rson;
                tmp->rson = tree;
                tree = tmp;
                if (!tree->lson) {
                    break;
                }
            }
            right->lson = tree;
            right = tree;
            tree = tree->lson;
        } else if (cmpres > 0) {
            if (!tree->rson) {
                break;
            }
            if (TermTopCompare(splay, tree->rson) > 0) {
                tmp = tree->rson;
                tree->rson = tmp->lson;
                tmp->lson = tree;
                tree = tmp;
                if (!tree->rson) {
                    break;
                }
            }
            left->rson = tree;
            left = tree;
            tree = tree->rson;
        } else {
            break;
        }
    }
    left->rson = tree->lson;
    right->lson = tree->rson;
    tree->lson = newTerm.rson;
    tree->rson = newTerm.lson;

    return tree;
}

/*-----------------------------------------------------------------------
//
// Function: TermTreeFree(TermCell* junk)
//
//   Release the memory taken by a term top AVL tree. Do not free
//   variables, as they belong to a variable bank as well. Yes, this
//   is an ugly hack! *sigh*
//
// Global Variables: -
//
// Side Effects    : Memory operations, destroys tree
//
/----------------------------------------------------------------------*/

void TermTree::TermTreeFree(TermCell* junk) {
    if (junk) {
        vector<TermCell*> st;
        st.push_back(junk);
        while (!st.empty()) {
            junk = st.back();
            st.pop_back();
            if (junk->lson) {
                st.push_back(junk->lson);
            }
            if (junk->rson) {
                st.push_back(junk->rson);
            }
            if (!junk->IsVar()) {
                DelPtr(junk);
            }
        }
        st.clear();
        vector<TermCell*>().swap(st);
        //PStackFree(stack);
    }
}

/*-----------------------------------------------------------------------
//
// Function: TermTopCompare()
//
//   Compare two top level term cells as
//   f_code.masked_properties.args_as_pointers, return a value >0 if
//   t1 is greater, 0 if the terms are identical, <0 if t2 is
//   greater. 
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

int TermTree::TermTopCompare(TermCell* t1, TermCell* t2) {
    int i, res;

    res = t1->fCode - t2->fCode;
    if (res) {
        return res;
    }

    assert(t1->arity == t2->arity);
    for (i = 0; i < t1->arity; i++) {
        /* res = (t1->args[i]->entry_no) - (t2->args[i]->entry_no); */

        //这里需要仔细考虑下　为什么是指针大小比较而不是内容大小比较？
        res = PCmp(t1->args[i], t2->args[i]);
        if (res) {
            return res;
        }
    }
    return res;
}

/*-----------------------------------------------------------------------
//
// Function: TermTreeFind()
//
//   Find a entry in the term tree, given a cell with correct
//   (i.e. term-bank) argument pointers.
//   pointers
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

TermCell* TermTree::TermTreeFind(TermCell* *root, TermCell* key) {
    if (*root) {
        *root = SplayTermTree(*root, key);
        if (TermTopCompare(*root, key) == 0) {
            return *root;
        }
    }
    return nullptr;
}

/*-----------------------------------------------------------------------
//
// Function: TermTreeInsert() 
//
//   Insert a term with valid subterm pointers into the termtree. If
//   the entry already exists, return pointer to existing entry as
//   usual, otherwise return nullptr.
//
// Global Variables: -
//
// Side Effects    : Changes tree
//
/----------------------------------------------------------------------*/

TermCell* TermTree::TermTreeInsert(TermCell* *root, TermCell* newTerm) {
    int cmpres;

    if (!*root) {
        newTerm->lson = newTerm->rson = nullptr;
        *root = newTerm;
        return nullptr;
    }

    *root = SplayTermTree(*root, newTerm);

    cmpres = TermTopCompare(newTerm, *root);

    if (cmpres < 0) {
        newTerm->lson = (*root)->lson;
        newTerm->rson = *root;
        (*root)->lson = nullptr;
        *root = newTerm;
        return nullptr;
    } else if (cmpres > 0) {
        newTerm->rson = (*root)->rson;
        newTerm->lson = *root;
        (*root)->rson = nullptr;
        *root = newTerm;
        return nullptr;
    }
    return *root;
}

/*-----------------------------------------------------------------------
//
// Function: TermTreeExtract()
//
//   Remove a top term cell from the term tree and return a pointer to
//   it.
//
// Global Variables: -
//
// Side Effects    : Changes tree
//
/----------------------------------------------------------------------*/

TermCell* TermTree::TermTreeExtract(TermCell* *root, TermCell* key) {
    TermCell* x;

    if (!(*root)) {
        return nullptr;
    }
    *root = SplayTermTree(*root, key);
    if (TermTopCompare(key, (*root)) == 0) {
        if (!(*root)->lson) {
            x = (*root)->rson;
        } else {
            x = SplayTermTree((*root)->lson, key);
            x->rson = (*root)->rson;
        }
        TermCell* cell = *root;
        cell->lson = cell->rson = nullptr;
        *root = x;
        return cell;
    }
    return nullptr;
}

/*-----------------------------------------------------------------------
//
// Function: TermTreeDelete()
//
//   Delete a top term from the term tree.
//
// Global Variables: -
//
// Side Effects    : Changes tree, memory operations
//
/----------------------------------------------------------------------*/

bool TermTree::TermTreeDelete(TermCell* *root, TermCell* term) {
    TermCell* cell;

    cell = TermTreeExtract(root, term);
    if (cell) {
        DelPtr(cell);
        return true;
    }
    return false;
}

/*-----------------------------------------------------------------------
//
// Function: TermTreeSetProp()
//
//   Set the given properties for all term cells in the tree.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void TermTree::TermTreeSetProp(TermCell* root, TermProp props) {
    vector<TermCell*> stack;
    stack.push_back(root);
    while (!stack.empty()) {
        root = stack.back();
        stack.pop_back();
        if (root) {
            SetProp(root->properties, props);
            stack.push_back(root->lson);
            stack.push_back(root->rson);
        }
    }

}

/*-----------------------------------------------------------------------
//
// Function: TermTreeDelProp()
//
//   Delete the given properties for all term cells in the tree.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void TermTree::TermTreeDelProp(TermCell* root, TermProp props) {
    vector<TermCell*> stack;
    stack.push_back(root);

    while (!stack.empty()) {
        root = stack.back();
        stack.pop_back();
        if (root) {
            DelProp(root->properties, props);
            stack.push_back(root->lson);
            stack.push_back(root->rson);
        }
    }

}

/***************************************************************************** 
 * 返回伸展树上的节点个数 Return the number of nodes in the tree.
 ****************************************************************************/
long TermTree::TermTreeNodes(TermCell* root) {
    vector<TermCell*> stack;
    long res = 0;
    stack.push_back(root);
    while (!stack.empty()) {
        root = stack.back();
        stack.pop_back();
        if (root) {
            stack.push_back(root->lson);
            stack.push_back(root->rson);
            ++res;
        }
    }
    vector<TermCell*>().swap(stack);
    return res;
}

/*---------------------------------------------------------------------*/
/*                    Constructed Function                             */

/*---------------------------------------------------------------------*/
TermTree::TermTree() {
}

TermTree::TermTree(const TermTree & orig) {
}

TermTree::~TermTree() {
}

