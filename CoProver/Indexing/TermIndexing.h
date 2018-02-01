/* 
 * File:   TermIndexing.h
 * Author: Zhong Jian<77367632@qq.com>
 * Context:项索引
 * Created on 2018年1月9日, 下午2:44
 */

#ifndef TERMINDEXING_H
#define TERMINDEXING_H
#include "Global/IncDefine.h"
#include "CLAUSE/Literal.h" 
#include "Global/Environment.h"

class TermIndNode {
private:

    struct cmp {

        bool operator()(const TermIndNode* a, const TermIndNode * b) const {
            return b->curTermSymbol->fCode - a->curTermSymbol->fCode;
        }
    };
public:
    TermCell* curTermSymbol; //当前symbol
    set<TermIndNode*, cmp> subTerms; //子项列表
    Literal* leaf;

    TermIndNode() : curTermSymbol(nullptr) {        
        leaf = nullptr;
    }

    TermIndNode(TermCell* term) : curTermSymbol(term), leaf(nullptr) {
    }
};

class TermIndexing {
protected:

    struct IndexNodeCell {
        FunCode keyCode; //Term->funcode
        TermCell* term; //
        // map<FunCode,IndexNodeCell*> subTerm;
        SplayTree<NumTreeCell> subTerm;
        struct IndexNodeCell *next;

        IndexNodeCell() {
            keyCode = 0;
            term = nullptr;
            next = nullptr;
        }

        IndexNodeCell(TermCell* t) : keyCode(t->fCode), term(t) {
            next = nullptr;
        }
    };


    //回退点

    struct BackPoint {
        int32_t queryTermPos; //pos of queryterm

        set<TermIndNode*>::iterator subNodeIt; // iterator of treeSubNode

        BackPoint(int32_t qTermPos, set<TermIndNode*>::iterator&it)
        : queryTermPos(qTermPos), subNodeIt(it) {
        };
    };

    IndexNodeCell* root;

    TermIndNode* r;
public:
    TermIndexing();
    TermIndexing(const TermIndexing& orig);
    virtual ~TermIndexing();
private:
    virtual void InsertTerm(TermIndNode* treeNode, TermCell * term);
public:
    /*---------------------------------------------------------------------*/
    /*                  Member Function-[public]                           */
    /*---------------------------------------------------------------------*/
    //

    virtual void Insert(Literal*lit);
    /*insert Term*/
    virtual void InsertTerm(TermCell* t);
    
    virtual Literal* ForwordSubsumption(TermCell* t,bool isEqual = false);

};
// Discrimation Tree Indexing

class DiscrimationIndexing : public TermIndexing {
private:
    vector<TermCell*> flattenTerm; //项的扁平化表示

    vector<TermCell*> varLst[50]; //上限 一个term中最多只能有50个变元
    vector<BackPoint*> backpoint;
    /// 在节点treeNode 后面插入 项t的所有符号
    /// \param treeNode
    /// \param t
    void InsertTerm(TermIndNode** treeNode, TermCell * term);

public:
    /*---------------------------------------------------------------------*/
    /*                    Constructed Function                             */

    /*---------------------------------------------------------------------*/
    DiscrimationIndexing() {
        r = new TermIndNode();
        backpoint.reserve(32);
    }
    /*---------------------------------------------------------------------*/
    /*                       Inline  Function                              */
    /*---------------------------------------------------------------------*/
    //打印树绑定的

    void Print() {
        TraverseTerm(r);
    }

    void GetBigEle(vector<NumTreeCell*>&vect) {
        FunCode code = 1;
        root->subTerm.GetAllBigerEle(code, vect);
    }

    /*---------------------------------------------------------------------*/
    /*                  Member Function-[public]                           */
    /*---------------------------------------------------------------------*/
    //
    void Insert(Literal* lit);

    void InsertTerm(TermCell * t);

    void TraverseTerm(TermIndNode* indNode, int level = 0);
    /// flattening-term  扁平化项
    /// \param term 需要扁平化的term

    void FlattenTerm(TermCell* term) {
        vector<TermCell*>vecTmp;
        vecTmp.reserve(32);
        flattenTerm.clear();
        flattenTerm.reserve(32);
        vecTmp.push_back(term);

        while (!vecTmp.empty()) {
            TermCell* t = &*(vecTmp.back());
            vecTmp.pop_back();
            flattenTerm.push_back(t);
            for (int32_t i = t->arity - 1; i>-1; --i)
                vecTmp.push_back(t->args[i]);
        }
    }
    Literal* ForwordSubsumption(TermCell* t, bool isEqual = false);

    /// 存在一个变元替换,检查树分支中是否有相同替换的路径分支,若存在则返回 相同替换的分支的最后一个子节点,否则返回nullptr
    /// \param qTerm 查询的项
    /// \param treePos 返回skip后的节点位置
    /// \return     

    bool CheckOccurs(TermCell* qTerm, set<TermIndNode*>::iterator&treePosIt) {
        assert(qTerm->IsVar());
        vector<TermCell*>&varCh = this->varLst[-qTerm->fCode];
        if (varCh[0] != (*treePosIt)->curTermSymbol) return false;
        set<TermIndNode*>::iterator subPosIt;
        for (int32_t iPos = 1; iPos < varCh.size(); ++iPos) {
            subPosIt = (*treePosIt)->subTerms.find(new TermIndNode(varCh[iPos]));
            if (subPosIt == (*treePosIt)->subTerms.end())
                return false;
            treePosIt=subPosIt;
        } 
        return true;
    }

    void BindingVar(int32_t qTermPos, int32_t funcLevel, std::set<TermIndNode*>::iterator& treePosIt) {

        assert(!flattenTerm.empty() && flattenTerm[qTermPos]->IsVar());

        vector<TermCell*>&varCh = this->varLst[-(flattenTerm[qTermPos]->fCode)]; //获取变元替换列表
        //add var-binding
        varCh.push_back((*treePosIt)->curTermSymbol); //添加变元替换

        set<TermIndNode*>::iterator tmpIt;

        if (funcLevel == 0)
            funcLevel += (*treePosIt)->curTermSymbol->arity;

        while (funcLevel > 0) {

            assert(!(*treePosIt)->subTerms.empty());
            //添加回退点
            tmpIt = treePosIt;
            if (++tmpIt != (*treePosIt)->subTerms.end())
                backpoint.push_back(new BackPoint(qTermPos, tmpIt));

            //skip           
            treePosIt = (*treePosIt)->subTerms.begin();
           // assert(!(*treePosIt)->subTerms.empty());

            //add var-binding
            varCh.push_back((*treePosIt)->curTermSymbol);

            funcLevel += ((*treePosIt)->curTermSymbol->arity - 1);

        }
        //测试输出绑定的项
        printf("变元替换项为: ");
        for(TermCell* tt: varCh){
            if(tt->fCode>0)
            cout<<Env::getSig()->fInfo[tt->fCode]->name;
            else
                tt->VarPrint(stdout);
            printf(" ");
        }
        printf("\n");
    }
};

#endif /* TERMINDEXING_H */

