/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.

 * File:   SplayTree.h
 * Author: zj 
 *
 * Created on 2017年3月7日, 上午8:59
 */

#ifndef SPLAYTREE_H
#define SPLAYTREE_H
#include "Global/IncDefine.h"
#include "TreeNodeDef.h"
#include <typeinfo>
#include <iostream>

/*---------------------------------------------------------------------*/
/*                             模板类-伸展树                             */
/*---------------------------------------------------------------------*/
//


class PTreeCell;

template<typename T>
class SplayTree {
private:
    T* root;
public:
    ObjDelFun delFun;
private:

    /*使用模板　根据输入的　key值对伸展树执行　splay 操作.
     * 注：传入key值需要临时生成一个T 树节点
     */
    template<typename V>
    static T* Splay(T*rootTmp, const V& key) {
        T* left;
        T* right;
        T* tmp;
        T newnode;
        if (!rootTmp) return rootTmp;
        newnode.lson = nullptr;
        newnode.rson = nullptr;
        left = &newnode;
        right = &newnode;
        int cmp;
        for (;;) {
            cmp = rootTmp->Compare(key);
            if (cmp > 0) // left nodes　　
            {
                tmp = (T*) rootTmp->lson;
                if (!tmp)
                    break; //没有左节点

                if (tmp->Compare(key) > 0) //zig-zig形式
                {
                    /*  leftNode 绕root 右旋转 (right rotate)
                     *        root            leftNode
                     *         /                /   \
                     *    leftNode    ==>      K   root
                     *      /   \                    /
                     *     K  right              right
                     */
                    rootTmp->lson = tmp->rson;
                    tmp->rson = rootTmp;
                    rootTmp = tmp;
                    if (!rootTmp->lson)
                        break;
                }
                //将当前树的根和根的右子树挂在右树上
                right->lson = rootTmp;
                right = rootTmp;
                rootTmp = rootTmp->lson;
            } else if (cmp < 0) //right nodes
            {
                tmp = rootTmp->rson;
                if (!tmp)break;
                if (tmp->Compare(key) < 0) //zag-zag形式
                {
                    /*  rightNode 绕root 左旋转 (left rotate)
                     *       root            rightNode
                     *         \               /   \
                     *    rightNode    ==>   root   K
                     *      /   \              \
                     *    left   K             left
                     */
                    rootTmp->rson = tmp->lson;
                    tmp->lson = rootTmp;
                    rootTmp = tmp;
                    if (!rootTmp->rson)
                        break;
                }
                //将当前树的根和根的左子树挂在左树上    
                left->rson = rootTmp;
                left = rootTmp;
                rootTmp = rootTmp->rson;

            } else //find this node
            {
                break;
            }
        }
        left->rson = rootTmp->lson;
        right->lson = rootTmp->rson;
        rootTmp->lson = newnode.rson;
        rootTmp->rson = newnode.lson;
        return rootTmp;
    }

    //重载

    template<typename V, typename Pred>
    static T* Splay(T* rootTmp, const V& key, Pred cmpFun) {
        T* left;
        T* right;
        T* tmp;
        T newnode;
        if (!rootTmp) return rootTmp;
        newnode.lson = nullptr;
        newnode.rson = nullptr;
        left = &newnode;
        right = &newnode;
        int cmp;
        for (;;) {
            cmp = cmpFun(rootTmp->key, key);
            if (cmp > 0) // left nodes　　
            {
                tmp = (T*) rootTmp->lson;
                if (!tmp)
                    break; //没有左节点
                if (cmpFun(rootTmp->key, key) > 0) //zig-zig形式                
                {
                    /*  leftNode 绕root 右旋转 (right rotate)
                     *        root            leftNode
                     *         /                /   \
                     *    leftNode    ==>      K   root
                     *      /   \                    /
                     *     K  right              right
                     */
                    rootTmp->lson = tmp->rson;
                    tmp->rson = rootTmp;
                    rootTmp = tmp;
                    if (!rootTmp->lson)
                        break;
                }
                //将当前树的根和根的右子树挂在右树上
                right->lson = rootTmp;
                right = rootTmp;
                rootTmp = rootTmp->lson;
            } else if (cmp < 0) //right nodes
            {
                tmp = rootTmp->rson;
                if (!tmp)break;
                if (cmpFun(tmp->key, key) < 0)//zag-zag形式
                {
                    /*  rightNode 绕root 左旋转 (left rotate)
                     *       root            rightNode
                     *         \               /   \
                     *    rightNode    ==>   root   K
                     *      /   \              \
                     *    left   K             left
                     */
                    rootTmp->rson = tmp->lson;
                    tmp->lson = rootTmp;
                    rootTmp = tmp;
                    if (!rootTmp->rson)
                        break;
                }
                //将当前树的根和根的左子树挂在左树上    
                left->rson = rootTmp;
                left = rootTmp;
                rootTmp = rootTmp->rson;

            } else //find this node
            {
                break;
            }
        }
        left->rson = rootTmp->lson;
        right->lson = rootTmp->rson;
        rootTmp->lson = newnode.rson;
        rootTmp->rson = newnode.lson;
        return rootTmp;
    }


public:
    /*---------------------------------------------------------------------*/
    /*                          inline Function                            */
    /*---------------------------------------------------------------------*/
    //

    inline T* GetRoot() {
        return root;
    }

    inline bool IsEmpty() {
        return root == nullptr;
    }

    /// 获取所有比ele大的 key>ele.key的节点元素
    /// \param key 指定的节点ele的key
    /// \param vect 返回的集合

    template<typename V>
    void GetAllBigerEle(V key, vector<T*>&vect) {
        T* ptr = FindByKey(key);
        if (!ptr) return;
        ptr = ptr->rson;
        GetAllEle(ptr, vect);
    }
    /// 获取所有比ele小的(key<ele.key)的节点元素
    /// \param key 指定的节点ele的key
    /// \param vect 返回的集合

    template<typename V>
    void GetAllSmallerEle(V key, vector<T*>&vect) {
        T* ptr = FindByKey(key);
        if (!ptr) return;
        ptr = ptr->rson;
        GetAllEle(ptr, vect);
    }
    /// 从小到大取出所有树节点(中序遍历)
    /// \param ele
    /// \param vect

    void GetAllEle(T* ele, vector<T*>&vect) {
        if (!ele)
            return;
        if (ele->lson) {
            GetAllEle(ele->lson, vect);
        }
        vect.push_back(ele);
        if (ele->rson) {
            GetAllEle(ele->rson, vect);
        }
    }

    inline void TraverseInit(vector<T*>&vect) {
        T* ptr = root;
        while (ptr != nullptr) {
            vect.push_back(ptr);
            ptr = ptr->lson;
        }
    }

    T* TraverseNext(vector<T*>&vect) {
        if (vect.empty())
            return nullptr;
        T* ele = vect.back();
        vect.pop_back();
        T* handle = ele->rson;
        while (handle != nullptr) {
            vect.push_back(handle);
            handle = handle->lson;
        }
        return ele;

    }

    /*---------------------------------------------------------------------*/
    /*                  Member Function-[public]                           */
    /*---------------------------------------------------------------------*/

    /*插入新的节点　到伸展树中*/
    T* Insert(T* newnode) {
        if (!root) {
            newnode->lson = newnode->rson = nullptr;
            root = newnode;
            return nullptr;
        }
        root = Splay(root, newnode->key);
        int cmp = newnode->CompareObj(root);
        if (cmp < 0) {
            newnode->lson = (root)->lson;
            newnode->rson = root;
            root->lson = nullptr;
            root = newnode;
            return nullptr;
        } else if (cmp > 0) {
            newnode->rson = (root)->rson;
            newnode->lson = root;
            root->rson = nullptr;
            root = newnode;
            return nullptr;
        }
        return root;
    }

    template<typename Pred>
    T* Insert(T* newnode, Pred cmpFun) {
        if (!root) {
            newnode->lson = newnode->rson = nullptr;
            root = newnode;
            return nullptr;
        }
        root = Splay(root, newnode->key, cmpFun);
        int cmp = cmpFun(newnode->key, root->key);
        if (cmp < 0) {
            newnode->lson = (root)->lson;
            newnode->rson = root;
            root->lson = nullptr;
            root = newnode;
            return nullptr;
        } else if (cmp > 0) {
            newnode->rson = (root)->rson;
            newnode->lson = root;
            root->rson = nullptr;
            root = newnode;
            return nullptr;
        }
        return root;
    }

    /*****************************************************************************
     * 查找 －－基于伸展树的查找
     ****************************************************************************/
    template<typename V>
    T* FindByKey(V& key) {
        if (root) {
            root = Splay(root, key);
            if (root->Compare(key) == 0) {
                return root;
            }
        }
        return nullptr;
    }
    /// 查找key节点右分支中值最小的节点.也就是右分支中的最左叶子节点
    /// \param key
    /// \return 
    template<typename V>
    T* FindNextBigerEle(V& key) {

        if (root) {
            root = Splay(root, key);
            if (root->Compare(key) == 0) {
                T* nextBigerEle = root->rson;
                while (nextBigerEle->lson) {
                    nextBigerEle = nextBigerEle->lson;
                }
                return nextBigerEle;
            }
        }
        return nullptr;
    }

    template<typename V, typename Pred>
    T* Find(V& key, Pred cmpFun) {
        if (root) {
            root = Splay(root, key, cmpFun);
            if (cmpFun(root->key, key) == 0) {
                return root;
            }
        }
        return nullptr;
    }

    /*****************************************************************************
     * 查找－－基于二叉树的查找 
     ****************************************************************************/
    template<typename V>
    T* BinaryFind(V& key) {
        T* tNode = root;
        while (tNode) {
            int cmp = tNode->Compare(key);
            if (cmp > 0) {
                tNode = tNode->lson;
            } else if (cmp < 0) {
                tNode = tNode->rson;
            } else {
                break;
            }
        }
        return tNode;
    }

    /***************************************************************************** 
     * 输出　print
     ****************************************************************************/
    void Print() {
        Print(root);
    }

    void Print(T* node) {
        if (!node)
            return;
        cout << node->key;
        if (node->lson) {
            cout << "->left:";
            Print(node->lson);
        }
        if (node->rson) {
            cout << "->right:";
            Print(node->rson);
        }
        cout << endl;
    }

    /*****************************************************************************
     * 根据key 得到树节点 
     ****************************************************************************/
    template<typename V>
    T* TreeExtractEntry(const V& key) {
        T* x;
        if (!root) {
            return nullptr;
        }
        root = Splay(root, key);
        if (root->Compare(key) == 0) {
            if (!root->lson) {
                x = root->rson;
            } else {
                x = Splay(root->lson, key);
                x->rson = root->rson;
            }
            T* cell = root;
            cell->lson = cell->rson = nullptr;
            root = x;
            return cell;
        }
        return nullptr;
    }

    /*****************************************************************************
     * 根据key 以及比较函数 得到树节点 
     ****************************************************************************/
    template<typename V, typename Pred>
    T* TreeExtractEntry(const V& key, Pred cmpFun) {
        T* x;
        if (!root) {
            return nullptr;
        }
        root = Splay(root, key, cmpFun);
        if (cmpFun(root->key, key) == 0) {//error修改if (cmpFun(root, key) == 0)
            if (!root->lson) {
                x = root->rson;
            } else {
                x = Splay(root->lson, key, cmpFun);
                x->rson = root->rson;
            }
            T* cell = root;
            cell->lson = cell->rson = nullptr;
            root = x;
            return cell;
        }
        return nullptr;
    }

    /*****************************************************************************
     * 节点类型为PTree,其中key 为void*．根据key查找节点， １．删除节点，２．返回　key (void*)
     ****************************************************************************/
    template<typename V>
    void* PTreeExtractKey(const V& key) {
        T* handle;
        void* res = nullptr;
        handle = TreeExtractEntry(key);
        if (handle) {
            res = handle->key;
            assert(typeid (*handle) == typeid (PTreeCell));
            DelPtr(handle);
        }
        return res;
    }

    template<typename V, typename Pred>
    void* PTreeExtractKey(const V& key, Pred cmpFun) {
        T* handle;
        void* res = nullptr;
        handle = TreeExtractEntry(key, cmpFun);
        if (handle) {
            res = handle->key;
            assert(typeid (*handle) == typeid (PTreeCell));

            DelPtr(handle);

        }
        return res;
    }

    void* PTreeExtractRootKey() {
        if (root) {
            return PTreeExtractKey(root->key);
        }
        return nullptr;
    }

    /*****************************************************************************
     * 删除　key 指定的树节点 
     ****************************************************************************/
    template<typename V>
    bool TreeDeleteEntry(const V& key) {
        T* cell = TreeExtractEntry(key);
        if (cell) {
            if (typeid (*cell) == typeid (PTreeCell))
                DelPtr(cell);
            else
                TreeCellFree(cell);
            return true;
        }
        return false;
    }
    /*****************************************************************************
     * 根据ｋｅy 查找节点，并删除节点，返回节点的ｋｅｙ值
     ****************************************************************************/

    /***************************************************************************** 
     * 树合并 Merge
     ****************************************************************************/
    bool Merge(SplayTree<T>& addTree) {
        //PStack_p stack = PStackAlloc();
        vector<T*> st;

        bool res = false;
        T* addNode = addTree.GetRoot();
        st.push_back(addNode);

        while (!st.empty()) {
            addNode = st.back(); // PStackPopP(stack);
            if (addNode) {
                st.push_back(addNode->lson);
                st.push_back(addNode->rson);
                T* tmp = Insert(addNode);
                if (tmp) {
                    if (typeid (*addNode) == typeid (PTreeCell))
                        DelPtr(addNode);
                    else
                        TreeCellFree(addNode);

                } else {
                    res = true;
                }
            }
            st.pop_back();
        }
        st.clear();
        return res;
    }

    /***************************************************************************** 
     * 树的插入　
     ****************************************************************************/
    void InsertTree(T* addNode) {
        // PStack_p stack = PStackAlloc();
        vector<T*> st;
        //
        //   PStackPushP(stack, add);
        st.push_back(addNode);
        //
        while (!st.empty()) {
            addNode = st.back(); // PStackPopP(stack);
            if (addNode) {
                st.push_back(addNode->lson);
                st.push_back(addNode->rson);
                TreeStore(addNode->key);
            }
        }
        st.clear();
    }

    /*****************************************************************************
     * 树的copy 
     ****************************************************************************/
    SplayTree<T>* TreeCopy() {
        SplayTree<T>* rtnSp = new SplayTree<T>();
        T* handle;
        vector<T*> st;
        st.push_back(root);
        while (!(st.empty())) {
            handle = st.back();
            if (handle) {
                rtnSp->TreeStore(handle->key);
                st.push_back(handle->lson);
                st.push_back(handle->rson);
            }
            st.pop_back();
        }
        st.clear();
        return rtnSp;
    }

    template<typename V>
    bool TreeStore(V& key, IntOrP val1, IntOrP val2) {

        T* handle = new T();
        handle->key = key;
        handle->val1 = val1;
        handle->val2 = val2;

        T* newNode = Insert(handle);
        if (newNode) {
            if (typeid (*handle) == typeid (PTreeCell))
                DelPtr(handle);
            else if (typeid (*handle) == typeid (StrTreeCell))
                DelPtr(handle);
            else
                TreeCellFree(handle);
            return false;
        }
        return true;
    }

    /***************************************************************************** 
     * 根据　key 生成一个节点，并插入到一棵树中
     ****************************************************************************/
    template<typename V>
    bool TreeStore(V& key) {
        T* handle = new T();
        handle->key = key;

        T* newNode = Insert(handle);
        if (newNode) {
            if (typeid (*handle) == typeid (PTreeCell))
                DelPtr(handle);
            else
                TreeCellFree(handle);
            return false;
        }
        return true;
    }

    template<typename V, typename Pred>
    bool TreeStore(V& key, Pred cmpFun) {
        T* handle = new T();
        handle->key = key;

        T* newNode = Insert(handle, cmpFun);
        if (newNode) {
            if (typeid (*handle) == typeid (PTreeCell))
                DelPtr(handle);
            else
                TreeCellFree(handle);

            return false;
        }
        return true;
    }

    template<typename V, typename Pred>
    T* TreeObjStore(V& key, Pred cmpFun) {
        T* handle = new T();
        handle->key = key;

        T* newNode = Insert(handle, cmpFun);
        if (newNode) {
            if (typeid (*handle) == typeid (PTreeCell))
                DelPtr(handle);
            else
                TreeCellFree(handle);

            return newNode;
        }
        return NULL;
    }

    /*****************************************************************************
     * 检查tree2中是否有元素在tree1中 返回该树节点（注:返回的不是key值，需要使用者转换一次）
     ****************************************************************************/
    T* TreeSharedElement(SplayTree<T>&tree2) {

        vector<T*> st;
        st.reserve(8);
        st.push_back(tree2.GetRoot());

        T* handle;
        T* res = nullptr;
        while (!st.empty()) {
            handle = st.back();
            st.pop_back();
            if (handle) {
                res = FindByKey(handle->key);
                if (res) {//找到了                    
                    break;
                }
                st.push_back(handle->lson);
                st.push_back(handle->rson);
            }
        }
        st.clear();
        vector<T*>().swap(st);
        return res;
    }

    long int TreeNodesSize() {
        vector<T*>vect;
        vect.reserve(16);
        T* ptr = root;
        while (ptr != nullptr) {
            vect.push_back(ptr);
            ptr = ptr->lson;
        }
        int iCount = vect.size();
        while (!vect.empty()) {
            T* ele = vect.back();
            vect.pop_back();
            T* handle = ele->rson;
            while (handle != nullptr) {
                vect.push_back(handle);
                handle = handle->lson;
                ++iCount;
            }
        }
    }

    /*****************************************************************************
     * 树节点删除，内存操作 
     ****************************************************************************/
    void TreeCellFree(T* nodecell) {
        if (nodecell) {
            vector<T*> vecStack;
            vecStack.push_back(nodecell);

            T* junk;
            while (!vecStack.empty()) {
                junk = vecStack.back();
                vecStack.pop_back();
                if (junk->lson)
                    vecStack.push_back(junk->lson);
                if (junk->rson)
                    vecStack.push_back(junk->rson);

                if (delFun)
                    delFun((void*) &(junk->key));
                delete junk;
                junk = nullptr;

            }
            vecStack.clear();
        }
    }

    void Destroy() {
        if (root) {
            if (typeid (*root) == typeid (PTreeCell))
                DelPtr(root);
            else
                TreeCellFree(root);

            //delete root;
            root = nullptr;
        }
    }
    /*---------------------------------------------------------------------*/
    /*                    Constructed Function                             */
    /*---------------------------------------------------------------------*/
    //

    SplayTree() {
        root = nullptr;
        delFun = nullptr;
    };

    SplayTree(ObjDelFun delF) {
        SplayTree();
        delFun(delF);
    };

    SplayTree(const SplayTree& orig) {
    };

    virtual ~SplayTree() {
        Destroy();
    };
};

#endif /* SPLAYTREE_H */

