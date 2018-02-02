/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   TermIndexing.cpp
 * Author: Zhong Jian<77367632@qq.com>
 * 
 * Created on 2018年1月9日, 下午2:44
 */

#include <set>

#include "TermIndexing.h"
#include "CLAUSE/Literal.h"

TermIndexing::TermIndexing() {
    root = new IndexNodeCell();

}

TermIndexing::TermIndexing(const TermIndexing& orig) {
}

TermIndexing::~TermIndexing() {
}

void TermIndexing::InsertTerm(TermIndNode* treeNode, TermCell * term) {
    cout << "调用Base class InsertTerm方法错误!" << endl;
}

void TermIndexing::Insert(Literal* lit) {
    cout << "调用Base class Insert方法错误!" << endl;
}

void TermIndexing::InsertTerm(TermCell* term) {
    cout << "调用Base class 方法错误!" << endl;
}

Literal* TermIndexing::ForwordSubsumption(TermCell* t, bool isEqual) {
    cout << "调用Base class ForwordSubsumption 方法错误!" << endl;
    return nullptr;
}
/*=====================================================================*/
/*                    [DiscrimationIndexing]                           */
/*=====================================================================*/
//

void DiscrimationIndexing::Insert(Literal* lit) {
    TermCell* term = (lit->lterm);
    TermIndNode* termIndNode = r;
    InsertTerm(&termIndNode, term);

    if (lit->EqnIsEquLit())
        InsertTerm(&termIndNode, lit->rterm);

    termIndNode->leaf = lit;
}

void DiscrimationIndexing::InsertTerm(TermIndNode** treeNode, TermCell * term) {
    vector<TermCell*> vecTerm;
    vecTerm.reserve(32); //假设大部分项的符号个数最多有32个;
    TermCell *node;
    //遍历项Term
    TermIndNode* p = *treeNode;
    vecTerm.push_back(term);
    while (!vecTerm.empty()) {
        node = vecTerm.back();
        TermIndNode *tiNode = new TermIndNode(node);

        set<TermIndNode*>::iterator nodeIt;
        if (p->subTerms.empty())
            nodeIt = p->subTerms.insert(tiNode).first;
        else {
            nodeIt = p->subTerms.find(tiNode);
            if (nodeIt == p->subTerms.end()) {
                //若不存在,将节点插入到索引树中            
                nodeIt = p->subTerms.insert(tiNode).first;
            } else
                DelPtr(tiNode);
        }
        p = *nodeIt;

        vecTerm.pop_back();

        for (int i = node->arity - 1; i>-1; --i) {
            vecTerm.push_back(node->args[i]);
        }
    }
    *treeNode = p;
}

void DiscrimationIndexing::InsertTerm(TermCell* term) {
    vector<TermCell*> vecTerm;
    vecTerm.reserve(32);
    TermCell *node;
    //遍历项Term
    IndexNodeCell* p = root;
    vecTerm.push_back(term);
    while (!vecTerm.empty()) {
        node = vecTerm.back();
        NumTreeCell* ntCell = p->subTerm.FindByKey(node->fCode);

        if (!ntCell) {
            //若不存在插入值             
            ntCell = new NumTreeCell(node->fCode);
            IndexNodeCell* newNode = new IndexNodeCell(node);
            ntCell->val1.p_val = newNode;
            //将节点插入到索引树中
            p->subTerm.Insert(ntCell);
        }

        p = ((IndexNodeCell*) ntCell->val1.p_val)->next;
        if (!p) p = new IndexNodeCell();
        vecTerm.pop_back();

        for (int i = node->arity - 1; i>-1; --i) {
            vecTerm.push_back(node->args[i]);
        }
    }
}
/*---------------------------------------------------------------------*/
/*                  Member Function-[public]                           */
/*---------------------------------------------------------------------*/
//
/// 检查文字t 是否存在替换r 使得 tr==某个文字
/*算法描述: 逐一扫描t的子项,若遇变元(fcode<0) 则替换为indexing tree 上的项.*/
/// \param t
/// \return 

Literal* DiscrimationIndexing::ForwordSubsumption(TermCell* term, bool isEqual) {

    //TermIndNode* treeNode = r;
    backpoint.clear();
    set<TermIndNode*>::iterator subNodeIt
            = isEqual ? r->subTerms.find(new TermIndNode(new TermCell(0))) : r->subTerms.find(new TermIndNode(term));

    if (subNodeIt == r->subTerms.end()) return nullptr; //谓词不存在

    //扁平化项
    FlattenTerm(term);
    int32_t qTermPos = 0; //queryTerm的位置
    TermCell *queryTerm = nullptr;
    // assert(flattenTerm[0]==(*subNodeIt)->curTermSymbol);
    ++qTermPos;
    //遍历项Term
    while (qTermPos < flattenTerm.size()) {

        queryTerm = flattenTerm[qTermPos];

        assert(queryTerm);
        bool isRollback = false; //0--continue; 1-- skip; 2-- rollback

        if (queryTerm->IsVar()) //若为变元 检查绑定记录回退点
        {

            if (this->varLst[-queryTerm->fCode].empty()) {

                BindingVar(qTermPos, 0, subNodeIt); //变元没有绑定则进行绑定 且 记录回退点

            } else {//变元有绑定进行检查,1.成功 skip {1}; 2.不成功回退 rollBack{2}
                isRollback = !this->CheckOccurs(queryTerm, subNodeIt);
            }
        } else {
            //skip
            // queryTerm = flattenTerm[++qTermPos];
            set<TermIndNode*>::iterator tmpIt = (*subNodeIt)->subTerms.find(new TermIndNode(queryTerm));
            if (tmpIt == (*subNodeIt)->subTerms.end()) {
                isRollback = true;
            } else
                subNodeIt = tmpIt;


        }
        if (isRollback) {
            if (backpoint.empty()) return nullptr;
            //回滚
            //roll back 回滚
            qTermPos = backpoint.back()->queryTermPos;
            queryTerm = flattenTerm[++qTermPos];

            subNodeIt = backpoint.back()->subNodeIt;
            backpoint.pop_back();
            ++subNodeIt;

            //变元绑定回退
            int32_t funcLevel = 0;

            vector<TermCell*>&varCh = this->varLst[-queryTerm->fCode];
            TermCell* popTerm = nullptr;
            if (!varCh.empty()) {
                do {
                    popTerm = &(*varCh.back());
                    funcLevel -= popTerm->arity + 1;
                    varCh.pop_back();
                } while (popTerm != (*subNodeIt)->curTermSymbol);
            }
            BindingVar(qTermPos, funcLevel, subNodeIt);
        } else
            ++qTermPos;

    }

    assert((*subNodeIt)->leaf);

    return (*subNodeIt)->leaf;
}

Literal* DiscrimationIndexing::BackSubsumption(TermCell *t, bool isEqual = false) {
    return nullptr;
}

void DiscrimationIndexing::TraverseTerm(TermIndNode* indNode, int level) {
    printf(" ");
    ++level;
    printf("->(%d)", level);

    if (indNode->curTermSymbol == nullptr)
        cout << "root" << endl;
    else if (indNode->curTermSymbol->IsVar()) {
        indNode->curTermSymbol->VarPrint(stdout);
    } else {
        cout << Env::getSig()->fInfo[indNode->curTermSymbol->fCode]->name;
    }
    if (indNode->leaf) cout << endl;
    else {

        for (auto& subT : indNode->subTerms) {
            TraverseTerm(subT, level);
        }
    }


}
