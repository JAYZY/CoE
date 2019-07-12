/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Formula.h
 * Author: Zhong Jian<77367632@qq.com>
 *
 * Created on 2017年12月22日, 下午6:03
 */

#ifndef FORMULA_H
#define FORMULA_H

#include "Formula/ClauseSet.h"
#include "Indexing/TermIndexing.h"
#include "CLAUSE/LiteralCompare.h"
#include "INOUT/FileOp.h"

class Formula {
private:
    //谓词索引------
    map<int32_t, vector<Literal*>> g_PostPred; //正谓词
    map<int32_t, vector<Literal*>> g_NegPred; //负谓词
    map<TermCell*, set<TermCell*>> g_PostEqn; //正等词列表 a=b a=c a=d;  //只有基项 才可以  
    map<TermCell*, set<TermCell*>> g_NegEqn; //负等词 a!=b a!=c

    //单文字子句
    // TermIndexing *unitTermIndex = nullptr; //单文字子句索引
    /*Statistics(统计信息)--    */



    //list<Clause*> axioms;
    ClauseSet* origalClaSet; //原始子句集合
    ClauseSet* workClaSet; //处理后的子句集,(或工作子句集) --不包含单子子句和等词公理
    Scanner* in;


public:
    //公式集的相关信息
    uint32_t uEquLitNum; //等词个数
    uint32_t uNonHornClaNum; //非Horn子句个数
    uint32_t uMaxLitNum; //最大文字个数
    uint32_t uMaxFuncLayer; //最大函数嵌套层数


    TermIndexing* unitClaIndex; //正单元子句索引
    TermIndexing *allTermIndex; //Discrimation Indexing 主要用于forward subsume

    vector<Clause*> goalClaset; //目标子句集--注意,包括单元子句,也包括其他子句
    vector<Clause*> vNegUnitClas; //负单元子句,只有一个 负文字
    vector<Clause*> vPosUnitClas; //正单元子句

    vector<Clause*> vEqulityAxiom; //等词公理
    Formula();
    Formula(const Formula& orig);
    virtual ~Formula();
private:
    //读取tptp公式集
    void WFormulaTPTPParse();
public:
    /*---------------------------------------------------------------------*/
    /*                       Inline  Function                              */
    /*---------------------------------------------------------------------*/
    //
    //初始化公式信息

    inline void iniFolInfo() {
        this->uEquLitNum = 0; //等词个数
        this->uNonHornClaNum = 0; //非Horn子句个数
        this->uMaxLitNum = 0; //最大文字个数
        this->uMaxFuncLayer = 0; //最大函数嵌套层数
    }

    inline ClauseSet* getOrigalSet() {
        return origalClaSet;
    }

    inline list<Clause*>* getWorkClas() {
        return workClaSet->getClaSet();
    }
    //初始化所有的索引

    inline void initIndex() {
        if (this->allTermIndex) {
            DelPtr(allTermIndex);
        }
        allTermIndex = new DiscrimationIndexing();

        if (this->unitClaIndex) {
            DelPtr(unitClaIndex);
        }
        unitClaIndex = new DiscrimationIndexing();
    }

    inline void addGoalClas(Clause* goalCla) {
        goalClaset.push_back(goalCla);
    }

    //    inline void ClauseSetInsert(Clause* cla) {
    //        assert(cla);
    //        claSet->InsertCla(cla);
    //        //暂时没有给定子句的评估函数
    //    }

    inline void printOrigalClaSet() {
        FILE* out = FileOp::getInstance()->getInfoFile();
        string sinfo = "#------ Input Clauses ------\n";
        FileOp::getInstance()->outInfo(sinfo);
        origalClaSet->Print(out);
    }

    inline void printProcessedClaSet(FILE* out) {
        workClaSet->Print(out);
        cout << endl;
    }

    inline void printEqulityAxioms() {
       string sinfo = "\n#------ EquAxioms Clauses ------\n";

        for (Clause* cla : vEqulityAxiom) {
            cla->getStrOfClause(sinfo, true);
        }
        FileOp::getInstance()->outInfo(sinfo);
    }

    /**
     * 对单元子句列表进行排序
     */
    inline void unitClasSort() {
        //对单元子句集合进行排序 
        sort(vNegUnitClas.begin(), vNegUnitClas.end(), UnitClaCompareWithWeight());
        sort(vPosUnitClas.begin(), vPosUnitClas.end(), UnitClaCompareWithWeight());
    }
    



    /*---------------------------------------------------------------------*/
    /*                          Memeber Function                            */
    /*---------------------------------------------------------------------*/
    // <editor-fold defaultstate="collapsed" desc="添加等词公理">
    void GenerateEqulitAxiom();
    void GenerateEqulitAxiomByFunction();
    // </editor-fold>


    //读取公式
    void generateFormula(Scanner * in);
    //公式预处理
    RESULT preProcess();

    // <editor-fold defaultstate="collapsed" desc="归入冗余判断">

    bool leftLitsIsRundacy(Literal** pasClaLeftLits, uint16_t uPosLeftLitInd, Literal* actLits, uint16_t uActLeftLitInd, vector<Literal*>&vNewR);
    bool leftLitsIsRundacy(Literal* pasClaHoldLits, uint16_t uPasHoldLitInd, vector<Literal*>&vNewR, set<Cla_p>&setUsedCla);
    bool holdLitsIsRundacy(Literal** arrayHoldLits, uint16_t arraySize, set<Cla_p>&setUsedCla, Clause* pasCla);
    //单文字是否归入冗余
    bool unitLitIsRundacy(Literal* unitLit);

    // </editor-fold>



    //插入新子句到
    void insertNewCla(Cla_p cla);
    //删除子句
    void removeWorkCla(Cla_p cal);
    //添加谓词符号到全局列表中  
    void AddPredLst(Clause * cla);
    vector<Literal*>* getPredLst(Literal * lit);
    /*得到互补谓词候选文字集合*/
    vector<Literal*>* getPairPredLst(Literal * lit);
    //输出公式集的信息
    void printOrigalClasInfo(FILE * out);


    //根据策略，得到下一次演绎起步子句
    list<Clause*>::iterator getNextStartClause();
};

#endif /* FORMULA_H */

