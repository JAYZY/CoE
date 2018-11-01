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
#include "Global/IncDefine.h"
#include "INOUT/Scanner.h"
#include "CLAUSE/Clause.h"
#include "Formula/ClauseSet.h"
#include "Indexing/TermIndexing.h"
#include "CLAUSE/LiteralCompare.h"
class Formula {
private:
    //谓词索引------
    map<int32_t, vector<Literal*>> g_PostPred; //正谓词
    map<int32_t, vector<Literal*>> g_NegPred; //负谓词
    map<TermCell*, set<TermCell*>> g_PostEqn; //正等词列表 a=b a=c a=d;
    map<TermCell*, set<TermCell*>> g_NegEqn; //负等词 a!=b a!=c



    //单文字子句
    // TermIndexing *unitTermIndex = nullptr; //单文字子句索引
    /*Statistics(统计信息)--    */

    vector<Clause*>goalClaset;

    //list<Clause*> axioms;
    ClauseSet* claSet;
    Scanner* in;


public:
    uint32_t uEquLitNum;
    uint32_t uNonHornClaNum;
    TermIndexing* negUnitClaIndex; //正单元子句索引
    TermIndexing* posUnitClaIndex; //负单元子句索引
    vector<Clause*> vNegUnitClas; //负单元子句,只有一个 负文字
    vector<Clause*> vPosUnitClas; //正单元子句

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
    inline ClauseSet* getAxioms() {
        return claSet;
    }

    inline void addGoalClas(Clause* goalCla) {
        goalClaset.push_back(goalCla);
    }

    inline void ClauseSetInsert(Clause* cla) {
        assert(cla);
        claSet->InsertCla(cla);
        //暂时没有给定子句的评估函数
    }

    inline void printClas(FILE* out) {
        claSet->Print(out);
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
    /*                          Static Function                            */
    /*---------------------------------------------------------------------*/
    //读取公式
    void GenerateFormula(Scanner* in);
    //添加谓词符号到全局列表中
    void AddPredLst(Clause* cla);
    vector<Literal*>* getPredLst(Literal* lit);
    /*得到互补谓词候选文字集合*/
    vector<Literal*>* getPairPredLst(Literal* lit);
    //输出公式集的信息
    void printInfo(FILE* out);

};

#endif /* FORMULA_H */

