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
#include "CLAUSE/Clause.h"
map<TermCell*, int> TermIndexing::constTermNum;

TermIndexing::TermIndexing() {
    subst = new Subst();

}

TermIndexing::TermIndexing(const TermIndexing& orig) {
}

TermIndexing::~TermIndexing() {
    this->destroy();
    DelPtr(subst);
    vector<TermCell*>().swap(flattenTerm);
}

void TermIndexing::InsertTerm(TermIndNode* treeNode, TermCell * term) {
    cout << "调用Base class InsertTerm方法错误!" << endl;
}

void TermIndexing::Insert(Literal* lit) {
    cout << "调用Base class Insert方法错误!" << endl;
}

void TermIndexing::InsertTerm(TermCell* term) {
    cout << "调用Base class InsertTerm方法错误!" << endl;
}

void TermIndexing::Print() {

    cout << "调用Base class Print 方法错误!" << endl;
}

TermIndNode* TermIndexing::Subsumption(Literal* lit, SubsumpType subsumtype) {
    cout << "调用Base class  Subsumption 方法错误!" << endl;
    return nullptr;
}

TermIndNode* TermIndexing::NextForwordSubsump() {
    cout << "调用Base class NextForwordSubsump 方法错误!" << endl;
    return nullptr;
}

//TermIndNode* TermIndexing::BackSubsumption(Literal* lit) {
//    cout << "调用Base class BackSubsumption 方法错误!" << endl;
//    return nullptr;
//}

TermIndNode* TermIndexing::NextBackSubsump() {
    cout << "调用Base class NextBackSubsump 方法错误!" << endl;
    return nullptr;
}

void TermIndexing::ClearVarLst() {
    cout << "调用Base class ClearVarLst方法错误!" << endl;
}

/// flattening-term  扁平化项
/// \param term 需要扁平化的term

void TermIndexing::FlattenTerm(TermCell* term) {
    vector<TermCell*>vecTmp;
    vecTmp.reserve(32);

    vecTmp.push_back(term);
    while (!vecTmp.empty()) {
        TermCell* t = TermCell::TermDerefAlways(&*(vecTmp.back()));
        vecTmp.pop_back();

        flattenTerm.push_back(t);
        for (int32_t i = t->arity - 1; i>-1; --i)
            vecTmp.push_back(t->args[i]);
    }
     vector<TermCell*>().swap(vecTmp);
}

void TermIndexing::PrintFlattenTerms(FILE* out) {
    if (flattenTerm.empty()) {
        fprintf(out, "%s", "Flattern Terms is empty");
        fputs("\n", out);
        return;
    }
    for (TermCell*t : flattenTerm) {
        t->PrintTermSig(out);
    }
}


/*=====================================================================*/
/*                    [DiscrimationIndexing]                           */
/*=====================================================================*/
//

void DiscrimationIndexing::Insert(Literal * lit) {

    TermIndNode* termIndNode = getRoot(lit); // lit->EqnIsPositive() ? posRoot : negRoot;

    //    if (lit->lterm->TBTermIsGround()) { //添加基项
    //        termIndNode->groundTermMap[lit->lterm]++; //等词的rterm不判断是否为基项
    //    }

    termIndNode = InsertTerm(&termIndNode, lit->lterm);

    if (lit->EqnIsEquLit())
        termIndNode = InsertTerm(&termIndNode, lit->rterm);

    termIndNode->leafs.push_back(lit);


}
/// insert term into TreeIndexing
/// \param treeNode 当前的树索引节点
/// \param term     要插入的项
/// \return 

TermIndNode* DiscrimationIndexing::InsertTerm(TermIndNode** treeNode, TermCell * term) {
    //zj:记录基项个数
    if (term->TBTermIsGround())
        ++constTermNum[term];
    vector<TermCell*> vecTerm;

    vecTerm.reserve(MAX_SUBTERM_SIZE); //假设大部分项的符号个数不超过32个;

    TermCell *node;

    //遍历项Term
    TermIndNode* p = *treeNode;

    vecTerm.push_back(term);


    while (!vecTerm.empty()) {
        node = vecTerm.back();
        vecTerm.pop_back();

        //生成一个新的树节点(for search or  insert)
        TermIndNode *tiNode = new TermIndNode(node);

        set<TermIndNode*, TermIndNode::cmp>::iterator nodeIt = p->subTerms.find(tiNode);

        //若当前树节点子节点没有查询的节点,则插入数据,否则删除
        if (nodeIt == p->subTerms.end()) {
            p = *(p->subTerms.insert(tiNode).first);
        } else {
            p = *nodeIt;
            DelPtr(tiNode);
        }

        if (node->TBTermIsGround()) { //添加基项
            p->groundTermMap[node]++; //等词的rterm不判断是否为基项
        }

        for (int i = node->arity - 1; i>-1; --i) {
            vecTerm.push_back(node->args[i]);
        }
    }
    return p;
}

/*---------------------------------------------------------------------*/
/*                  Member Function-[public]                           */
/*---------------------------------------------------------------------*/


//
/// 包含冗余检查
/// \param t
/// \return 

TermIndNode * DiscrimationIndexing::Subsumption(Literal* lit, SubsumpType subsumptype) {


    backpoint.clear();
    TermIndNode* rootNode = getRoot(lit);    
    TermIndNode* tIndnode=new TermIndNode(lit->lterm);
    
    set<TermIndNode*, TermIndNode::cmp>::iterator parentNodeIt = rootNode->subTerms.find(tIndnode);
    DelPtr(tIndnode);
    
    if (parentNodeIt == rootNode->subTerms.end()) return nullptr; //谓词不存在
    //queryTerm的位置,排除谓词符号

    if (!lit->EqnIsEquLit())
        assert((Env::getSig()->SigIsPredicate((*parentNodeIt)->curTermSymbol->fCode))); //检查是否为谓词符号

    if ((*parentNodeIt)->subTerms.empty()) {
        assert(lit->lterm->arity == 0);
        return (*parentNodeIt); //该谓词下没有子项,表示为一个命题类型的项
    }
    //扁平化文字
    FlattenLiteral(lit);

    assert(!flattenTerm.empty());

    set<TermIndNode*, TermIndNode::cmp>::iterator subNodeIt = (*parentNodeIt)->subTerms.begin();

    if (subNodeIt == (*parentNodeIt)->subTerms.end()) return nullptr;

    TermIndNode * rtnLit = nullptr;
    switch (subsumptype) {
        case SubsumpType::Forword:
            rtnLit = FindForwordSubsumption(1, parentNodeIt, subNodeIt);
            break;
        case SubsumpType::Backword:
            rtnLit = FindBackwordSubsumption(1, parentNodeIt, subNodeIt);

            break;
        default:

            static_assert(true, "错误:无该包含类型处理方法!");

            break;
    }
    return rtnLit;


}
/// 检查查询文字q归入子句C, 是否存在替换r 使得 q(r)== c, PS:检查是否存在子句C是冗余的,C为索引树对应的叶节点项
/*算法描述: 逐一扫描q的子项,若遇变元(fcode<0) 则替换为indexing tree 的节点项.*/
/// \param qTermPos  //queryTerm的位置
/// \param parentNodeIt
/// \param subNodeIt
/// \return 

TermIndNode* DiscrimationIndexing::FindBackwordSubsumption(uint32_t qTermPos,
        set<TermIndNode*, TermIndNode::cmp>::iterator&parentNodeIt,
        set<TermIndNode*, TermIndNode::cmp>::iterator&subNodeIt) {

    TermCell *queryTerm = nullptr;

    //遍历项Term
    while (qTermPos < flattenTerm.size()) {

        queryTerm = flattenTerm[qTermPos];

        assert(queryTerm);

        bool isRollback = false;

        if (queryTerm->IsVar()) //若为变元 检查绑定记录回退点
        {
            if (this->varLst[-queryTerm->fCode].empty()) {

                BindingVar(qTermPos, 0, parentNodeIt, subNodeIt); //变元没有绑定则进行绑定 且 记录回退点

            } else {//变元有绑定进行检查,1.成功 skip {1}; 2.不成功回退 rollBack{2}

                isRollback = !this->CheckVarBinding(queryTerm, parentNodeIt, subNodeIt);

            }

        } else { //查询项非变元 在索引树上查找子项
            //skip
            subNodeIt = (*parentNodeIt)->subTerms.find(new TermIndNode(queryTerm));

            if (subNodeIt == (*parentNodeIt)->subTerms.end()) {
                isRollback = true;
            } else {
                parentNodeIt = subNodeIt;
                if (!(*parentNodeIt)->subTerms.empty())
                    subNodeIt = (*parentNodeIt)->subTerms.begin();
            }
        }
        if (isRollback) {
            //回滚 rollback
            if (backpoint.empty())return nullptr;
            qTermPos = backpoint.back()->queryTermPos;
            parentNodeIt = backpoint.back()->parentNodeIt;
            subNodeIt = backpoint.back()->subNodeIt;
            uint32_t varChPos = backpoint.back()->chgVarPos[0];
            uint32_t varChIdPos = backpoint.back()->chgVarPos[1];

            backpoint.pop_back();
            //变元绑定回退
            while (stVarChId.size() > varChIdPos) {
                this->varLst[stVarChId.back()].clear();
                stVarChId.pop_back();
            }
            assert(stVarChId.back() == -flattenTerm[qTermPos]->fCode);

            stVarChId.pop_back();


            vector<TermCell*>&varCh = this->varLst[-(flattenTerm[qTermPos]->fCode)];
            int32_t funcLevel = 0;
            if (!varCh.empty()) {
                funcLevel = -1;
                while (varCh.size() > varChPos) {
                    funcLevel = funcLevel - (&(*varCh.back()))->arity + 1;
                    varCh.pop_back();
                }
                if (varCh.empty())
                    assert(funcLevel == 0);
            }
            BindingVar(qTermPos, funcLevel, parentNodeIt, subNodeIt);

        }

        ++qTermPos;
    }
    assert(!(*subNodeIt)->leafs.empty());

    return (*subNodeIt);
}

TermIndNode* DiscrimationIndexing::NextBackSubsump() {

    //回滚 rollback
    if (backpoint.empty())return nullptr;

    uint32_t qTermPos = backpoint.back()->queryTermPos;

    assert(flattenTerm[qTermPos]->IsVar());

    set<TermIndNode*, TermIndNode::cmp>::iterator parentNodeIt = backpoint.back()->parentNodeIt;

    set<TermIndNode*, TermIndNode::cmp>::iterator subNodeIt = backpoint.back()->subNodeIt;

    assert(!(*parentNodeIt)->subTerms.empty());
    //cout << "err" << endl;

    uint32_t varChPos = backpoint.back()->chgVarPos[0];
    uint32_t varChIdPos = backpoint.back()->chgVarPos[1];
    backpoint.pop_back();

    //变元绑定回退
    while (stVarChId.size() > varChIdPos) {
        this->varLst[stVarChId.back()].clear();
        stVarChId.pop_back();
    }

    assert(stVarChId.back() == -flattenTerm[qTermPos]->fCode);
    stVarChId.pop_back();

    vector<TermCell*>&varCh = this->varLst[-(flattenTerm[qTermPos]->fCode)];

    int32_t funcLevel = -1;
    if (!varCh.empty()) {
        while (varCh.size() > varChPos) {
            funcLevel = funcLevel - (&(*varCh.back()))->arity + 1;
            varCh.pop_back();
        }

        if (varCh.empty()) funcLevel = 0;
        BindingVar(qTermPos, funcLevel, parentNodeIt, subNodeIt);
    }

    return FindBackwordSubsumption(++qTermPos, parentNodeIt, subNodeIt);
}

/// 在indexing Tree中查找与query Term冗余的节点
/// forwordSubsumption 
/// \param qTermPos
/// \param parentNodeIt
/// \param subNodeIt
/// \return 

TermIndNode * DiscrimationIndexing::FindForwordSubsumption(uint32_t qTermPos,
        set<TermIndNode*, TermIndNode::cmp>::iterator&parentNodeIt,
        set<TermIndNode*, TermIndNode::cmp>::iterator & subNodeIt) {

    TermCell* queryTerm = nullptr;
    uint32_t* chgVPos;
    //遍历项Term
    while (qTermPos < flattenTerm.size()) {
        queryTerm = flattenTerm[qTermPos];
        bool isRollback = false;
        if ((*subNodeIt)->IsVar()) {//若为变元 检查绑定记录回退点
            if ((*subNodeIt)->curTermSymbol->binding == nullptr) {
                //  (*subNodeIt)->curTermSymbol->binding = queryTerm;  chgVars->push_back((*subNodeIt)->curTermSymbol);
                set<TermIndNode*, TermIndNode::cmp>::iterator tmpIt = subNodeIt;
                if ((++tmpIt) != (*parentNodeIt)->subTerms.end()) {
                    //记录回退点
                    chgVPos = new uint32_t[1];
                    chgVPos[0] = subst->Size();
                    backpoint.push_back(new BackPoint(qTermPos, chgVPos, parentNodeIt, tmpIt));
                }
                subst->SubstAddBinding((*subNodeIt)->curTermSymbol, queryTerm); //记录已经有绑定的变元项   
            } else if (!(*subNodeIt)->curTermSymbol->binding->TermIsSubterm(queryTerm, DerefType::DEREF_NEVER, TermEqulType::StructEqual)) {
                isRollback = true; //比较是否相同
            }
            uint32_t funLevel = queryTerm->arity;
            //skip 该函数
            while (funLevel > 0) {
                funLevel += flattenTerm[++qTermPos]->arity - 1;
            }
        } else {
            subNodeIt = (*parentNodeIt)->subTerms.find(new TermIndNode(queryTerm));
            if (subNodeIt == (*parentNodeIt)->subTerms.end()) {
                isRollback = true;
            }
        }
        if (isRollback) {
            /*回退*/
            if (backpoint.empty()) return nullptr;
            //回滚
            //roll back 回滚
            qTermPos = backpoint.back()->queryTermPos;
            parentNodeIt = backpoint.back()->parentNodeIt;
            subNodeIt = backpoint.back()->subNodeIt;
            uint32_t chgVarPos = backpoint.back()->chgVarPos[0];
            subst->SubstBacktrackToPos(chgVarPos);
            backpoint.pop_back();
        } else {
            parentNodeIt = subNodeIt;
            //if ((*parentNodeIt)->subTerms.empty()) return nullptr;
            subNodeIt = (*parentNodeIt)->subTerms.begin();
            ++qTermPos;
        }
    }
    assert(!(*parentNodeIt)->leafs.empty());

    return (*parentNodeIt);

}

TermIndNode * DiscrimationIndexing::NextForwordSubsump() {
    if (backpoint.empty())return nullptr;
    //roll back 回滚
    uint32_t qTermPos = backpoint.back()->queryTermPos;
    set<TermIndNode*, TermIndNode::cmp>::iterator parentNodeIt = backpoint.back()->parentNodeIt;
    set<TermIndNode*, TermIndNode::cmp>::iterator subNodeIt = backpoint.back()->subNodeIt;
    uint32_t chgVarPos = backpoint.back()->chgVarPos[0];
    subst->SubstBacktrackToPos(chgVarPos);
    backpoint.pop_back();

    TermIndNode * rtnLit = FindForwordSubsumption(qTermPos, parentNodeIt, subNodeIt);

    return rtnLit;
}

Literal * DiscrimationIndexing::FindNextDemodulator(TermCell *term, bool isEqual) {

    backpoint.clear();
    set<TermIndNode*, TermIndNode::cmp>::iterator subNodeIt
            = isEqual ? posRoot->subTerms.find(new TermIndNode(new TermCell(0))) : posRoot->subTerms.find(new TermIndNode(term));
    if (subNodeIt == posRoot->subTerms.end()) return nullptr; //谓词不存在
    //扁平化项
    FlattenTerm(term);

    if ((*subNodeIt)->subTerms.empty()) return nullptr;
    set<TermIndNode*, TermIndNode::cmp>::iterator tmpTNodeIt = (*subNodeIt)->subTerms.begin();
    Literal * rtnLit = FindDemodulator(1, subNodeIt, tmpTNodeIt);

    return rtnLit;
}

Literal * DiscrimationIndexing::FindDemodulator(uint32_t qTermPos, set<TermIndNode*, TermIndNode::cmp>::iterator&parentNodeIt, set<TermIndNode*, TermIndNode::cmp>::iterator & subNodeIt) {

    return nullptr;

}

void DiscrimationIndexing::TraverseTerm(TermIndNode* indNode, bool isPosLit, int level) {
    printf(" ");
    ++level;
    printf("->(%d)", level);

    if (indNode->curTermSymbol == nullptr) {
        cout << "root";
    } else if (indNode->curTermSymbol->IsVar()) {
        indNode->curTermSymbol->VarPrint(stdout);
    } else {
        if (!isPosLit && level == 2)cout << "~";
        cout << Env::getSig()->fInfo[indNode->curTermSymbol->fCode]->name;
    }
    if (!indNode->leafs.empty()) {
        cout << "  [leaf node]:" << indNode->leafs.size() << endl;
    } else {

        for (auto& subT : indNode->subTerms) {

            TraverseTerm(subT, isPosLit, level);
        }
    }


}


/// 存在一个变元替换,检查树分支中是否有相同替换的路径分支,若存在则返回 相同替换的分支的最后一个子节点,否则返回nullptr
/// \param qTerm 查询的项
/// \param treePos 返回skip后的节点位置
/// \return     

bool DiscrimationIndexing::CheckVarBinding(TermCell* qTerm, set<TermIndNode*, TermIndNode::cmp>::iterator&parentNodeIt,
        set<TermIndNode*, TermIndNode::cmp>::iterator&subPosIt) {

    assert(qTerm->IsVar());

    vector<TermCell*>&varCh = this->varLst[-qTerm->fCode];

    assert(!varCh.empty());

    //assert(qTerm->fCode == (*treePosIt)->curTermSymbol->fCode);
    //    if (varCh[0]->fCode != (*treePosIt)->curTermSymbol->fCode)
    //        return false;
    for (uint32_t iPos = 0; iPos < varCh.size(); ++iPos) {
        subPosIt = (*parentNodeIt)->subTerms.find(new TermIndNode(varCh[iPos]));
        if (subPosIt == (*parentNodeIt)->subTerms.end())
            return false;
        parentNodeIt = subPosIt;


    }

    if (!(*parentNodeIt)->subTerms.empty())
        subPosIt = (*parentNodeIt)->subTerms.begin();
    return true;

}



/// bingding vars 
/// \param qTermPos
/// \param funcLevel
/// \param treePosIt  注意:treePosIt对应的是 变元项所在的父节点

void DiscrimationIndexing::BindingVar(const uint32_t qTermPos, int32_t funcLevel,
        std::set<TermIndNode*, TermIndNode::cmp>::iterator&parentNodeIt,
        std::set<TermIndNode*, TermIndNode::cmp>::iterator& subNodeIt) {

    assert(!flattenTerm.empty() && flattenTerm[qTermPos]->IsVar());

    vector<TermCell*>&varCh = this->varLst[-(flattenTerm[qTermPos]->fCode)]; //获取变元替换列表
    stVarChId.push_back(-(flattenTerm[qTermPos]->fCode));
    uint32_t* chgVPos = nullptr;

    //添加回退点
    std::set<TermIndNode*, TermIndNode::cmp>::iterator tmpIt;

    while (funcLevel >-1) {

        if (subNodeIt != (*parentNodeIt)->subTerms.end()) {
            tmpIt = subNodeIt;
            if (++tmpIt != (*parentNodeIt)->subTerms.end()) {
                //添加回退点
                chgVPos = new uint32_t[2];
                chgVPos[0] = varCh.size();
                chgVPos[1] = stVarChId.size();
                backpoint.push_back(new BackPoint(qTermPos, chgVPos, parentNodeIt, tmpIt));
            }
        }

        //add var-binding
        varCh.push_back((*subNodeIt)->curTermSymbol);

        //assert(!(*parentNodeIt)->subTerms.empty());

        funcLevel += ((*subNodeIt)->curTermSymbol->arity - 1);

        //skip  
        parentNodeIt = subNodeIt;

        if ((*parentNodeIt)->subTerms.empty()) break;
        subNodeIt = (*parentNodeIt)->subTerms.begin();

    }


    //测试输出绑定的项
    //    cout << "变元绑定:";
    //    flattenTerm[qTermPos]->VarPrint(stdout);
    //    printf("=>");
    //    for (TermCell* tt : varCh) {
    //        if (tt->fCode > 0)
    //            cout << Env::getSig()->fInfo[tt->fCode]->name;
    //        else
    //            tt->VarPrint(stdout);
    //        printf(" ");
    //    }
    //    printf("\n");
}

void DiscrimationIndexing::ClearVarLst() {

    for (int i = 0; i < 1000; ++i) {
        //varLst[i].clear();
        vector<TermCell*>().swap(varLst[i]);
    }
    
    vector<BackPoint*>().swap(backpoint);
    vector<uint32_t>().swap(stVarChId);
    this->subst->Clear();

}
