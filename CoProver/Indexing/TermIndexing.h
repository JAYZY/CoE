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
#include "Inferences/Subst.h"
using namespace std;

enum class SubsumpType : uint8_t {
    Forword,
    Backword
};

class TermIndNode {
    
public:
    //算法改进
    set<TermCell*> groundTermSet;
public:

    struct cmp {

        bool operator()(const TermIndNode* a, const TermIndNode * b) const {
            return (b->curTermSymbol->fCode - a->curTermSymbol->fCode) > 0 ? true : false;
        }
    };

public:
    uint32_t size;
    TermCell* curTermSymbol; //当前symbol
    
    set<TermIndNode*, cmp> subTerms; //子项列表
    
    vector<Literal*> leafs;

    TermIndNode() : curTermSymbol(nullptr), size(0) {

    }

    TermIndNode(TermCell* term) : curTermSymbol(term), size(0) {

    }

    virtual ~TermIndNode() {
        subTerms.clear();
        vector<Literal*>().swap(leafs);
    };

    inline bool IsVar() {
        return this->curTermSymbol->IsVar();
    }


};

class TermIndexing {
public:
    //vector<TermCell*> chgVars; //记录有绑定的变元项
    Subst* chgVars;
protected:

    //回退点定义

    struct BackPoint {
        uint32_t queryTermPos; //pos of queryterm
        uint32_t* chgVarPos; // 记录相关位置信息 如:变元替换列表位置  变元替换回退位置 
        set<TermIndNode*, TermIndNode::cmp>::iterator parentNodeIt; // iterator of treeNode(parent node)
        set<TermIndNode*, TermIndNode::cmp>::iterator subNodeIt; // iterator of treeSubNode

        /// \param qTermPos 查询项位置
        /// \param chgVPos  变元替换堆栈的位置
        /// \param ParentTNIt   父节点
        /// \param it           子节点      

        BackPoint(uint32_t qTermPos, uint32_t* chgVPos, set<TermIndNode*, TermIndNode::cmp>::iterator&ParentTNIt,
                set<TermIndNode*, TermIndNode::cmp>::iterator&it)
        : queryTermPos(qTermPos), chgVarPos(chgVPos), parentNodeIt(ParentTNIt), subNodeIt(it) {
        };

        virtual ~BackPoint() {
            DelArrayPtr(chgVarPos);
        }
    };

    vector<TermCell*> flattenTerm; //项的扁平化表示

    TermIndNode* posRoot; //正文字索引
    TermIndNode* negRoot; //负文字索引
    TermIndNode* posEqnRoot; //正等词索引
    TermIndNode* negEqnRoot; //负等词索引

public:

    TermIndexing();
    TermIndexing(const TermIndexing& orig);
    virtual ~TermIndexing();
private:
    virtual void InsertTerm(TermIndNode* treeNode, TermCell * term);
public:

    inline TermIndNode* getRoot(Literal* lit) {

        if (lit->EqnIsEquLit()) {
            return lit->EqnIsPositive() ? posEqnRoot : negEqnRoot;
        } else
            return lit->EqnIsPositive() ? posRoot : negRoot;


    }
    //返回文字谓词子节点个数

    inline int getNodeNum(Literal*lit) {
        TermIndNode* r = getRoot(lit);
        set<TermIndNode*, TermIndNode::cmp>::iterator subNodeIt = r->subTerms.find(new TermIndNode(lit->lterm));
        if (subNodeIt == r->subTerms.end())return 0;
        cout << "(*subNodeIt)->size" << r->size << endl;
        cout << "subTerms.size()+leafs.size()" << (*subNodeIt)->subTerms.size()+(*subNodeIt)->leafs.size() << endl;
        return (*subNodeIt)->subTerms.size()+(*subNodeIt)->leafs.size();

    }
    /*---------------------------------------------------------------------*/
    /*                  Member Function-[public]                           */
    /*---------------------------------------------------------------------*/
    //************************ Insert **************************************/
    virtual void Insert(Literal* lit);
    /*insert Term*/
    virtual void InsertTerm(TermCell* t);

    //************************ Remove **************************************/

    virtual void Print();

    virtual TermIndNode* Subsumption(Literal* lit, SubsumpType subsumtype);
    virtual TermIndNode* NextForwordSubsump();
    virtual TermIndNode* NextBackSubsump();

    // virtual TermIndNode* BackSubsumption(Literal* lit);
    //virtual bool DelClaFromIndex(Clause* cla);

    virtual void ClearVarLst();

    void DelIndexNode(TermIndNode* root) {

        vector<TermIndNode*> stNode;
        stNode.reserve(64);
        stNode.push_back(root);
        TermIndNode* p = nullptr;
        while (!stNode.empty()) {
            p = stNode.back();
            stNode.pop_back();
            for (auto&subT : p->subTerms) {
                stNode.push_back(subT);
            }
            DelPtr(p);
        }
        vector<TermIndNode*>().swap(stNode);
    }



    /// 扁平化文字项
    /// \param lit 需要扁平化的文字 lit

    void FlattenLiteral(Literal* lit) {
        flattenTerm.clear();
        flattenTerm.reserve(32);
        this->FlattenTerm(lit->lterm);
        if (lit->EqnIsEquLit())
            this->FlattenTerm(lit->rterm);
    }

    /// flattening-term  扁平化项
    /// \param term 需要扁平化的term
    void FlattenTerm(TermCell* term);

    /// 输出flattenTerms    
    void PrintFlattenTerms(FILE* out);

};

/*=====================================================================*/
/*                    [Discrimation Tree Indexing]                     */
/*=====================================================================*/
//

class DiscrimationIndexing : public TermIndexing {
private:

    vector<TermCell*> varLst[8000]; //上限 一个term中最多只能有1000个变元
    vector<uint32_t> stVarChId; //记录有存在替换的变量ID
    vector<BackPoint*> backpoint; /*回退点*/
    /// 在节点treeNode 后面插入 项t的所有符号
    /// \param treeNode
    /// \param t
    TermIndNode* InsertTerm(TermIndNode** treeNode, TermCell * term);

public:
    /*---------------------------------------------------------------------*/
    /*                    Constructed Function                             */

    /*---------------------------------------------------------------------*/
    DiscrimationIndexing() {
        posRoot = new TermIndNode();
        negRoot = new TermIndNode();
        posEqnRoot = new TermIndNode(); //正等词索引
        negEqnRoot = new TermIndNode(); //负等词索
        backpoint.reserve(32);
        //chgVars.reserve(32);
    }
    /*---------------------------------------------------------------------*/
    /*                       Inline  Function                              */
    /*---------------------------------------------------------------------*/
    //打印树绑定的

    inline void Print() {
        cout << "pos Indexing Tree:" << endl;
        TraverseTerm(posRoot);
        cout << "neg Indexing Tree:" << endl;
        TraverseTerm(negRoot, false);
        cout << "posEqnRoot Indexing Tree:" << endl;
        TraverseTerm(posEqnRoot, false);
        cout << "negEqnRoot Indexing Tree:" << endl;
        TraverseTerm(negEqnRoot, false);
    }

    /*---------------------------------------------------------------------*/
    /*                  Member Function-[public]                           */
    /*---------------------------------------------------------------------*/
    //
    void Insert(Literal* lit);



    void TraverseTerm(TermIndNode* indNode, bool isPosLit = true, int level = 0);




    TermIndNode* Subsumption(Literal* lit, SubsumpType subsumtype);

    TermIndNode* FindBackwordSubsumption(uint32_t qTermPos, set<TermIndNode*, TermIndNode::cmp>::iterator&parentNodeIt,
            set<TermIndNode*, TermIndNode::cmp>::iterator&subNodeIt);
    TermIndNode* NextBackSubsump();



    TermIndNode* FindForwordSubsumption(uint32_t qTermPos, set<TermIndNode*, TermIndNode::cmp>::iterator&parentNodeIt,
            set<TermIndNode*, TermIndNode::cmp>::iterator&subNodeIt);
    TermIndNode* NextForwordSubsump();







    Literal* FindNextDemodulator(TermCell *t, bool isEqual = false);
    Literal* FindDemodulator(uint32_t qTermPos, set<TermIndNode*, TermIndNode::cmp>::iterator&parentNodeIt, set<TermIndNode*, TermIndNode::cmp>::iterator&subNodeIt);


    /// 存在一个变元替换,检查树分支中是否有相同替换的路径分支,若存在则返回 相同替换的分支的最后一个子节点,否则返回nullptr
    /// \param qTerm 查询的项
    /// \param treePos 返回skip后的节点位置
    /// \return     

    bool CheckVarBinding(TermCell* qTerm, set<TermIndNode*, TermIndNode::cmp>::iterator&parentNodeIt,
            set<TermIndNode*, TermIndNode::cmp>::iterator&subPosIt);



    bool CheckOccurs();

    /// bingding vars 
    /// \param qTermPos
    /// \param funcLevel
    /// \param treePosIt  注意:treePosIt对应的是 变元项所在的父节点
    void BindingVar(const uint32_t qTermPos, int32_t funcLevel, std::set<TermIndNode*, TermIndNode::cmp>::iterator& parentNodeIt, std::set<TermIndNode*, TermIndNode::cmp>::iterator& treePosIt);

    void ClearVarLst();
    void VarLstBacktrackToPos(uint32_t varPos);

};

#endif /* TERMINDEXING_H */

