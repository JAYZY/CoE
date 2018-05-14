
/* 
 * File:   ProofControl.h
 * Author: Zhong Jian<77367632@qq.com>
 * Content: 证明过程控制
 * Created on 2018年1月8日, 上午10:30
 */

#ifndef PROOFCONTROL_H
#define PROOFCONTROL_H
#include "Global/IncDefine.h"
#include "CLAUSE/Clause.h"
#include "Indexing/TermIndexing.h"
#include "Formula/FormulaSet.h"

class Processed {
private:
    list<Clause*> PosRules; //处理过的正文字 单元子句 (有序的,要么不是等词,要么已经左项>右项) 
    list<Clause*> PosEqns; //处理过的正文字单元子句(无序的等词文字)
    list<Clause*> NegUnits; //单元子句,只有一个 负文字
    list<Clause*> NonUnits; //其他非单元子句

    //TermIndexing *unitIndex; //单元索引
    TermIndexing *termIndex; //多文字索引


public:
    /*---------------------------------------------------------------------*/
    /*                    Constructed Function                             */
    /*---------------------------------------------------------------------*/
    //

    Processed() {
        termIndex = new DiscrimationIndexing();

    }

    virtual ~Processed() {
        PosRules.clear();
        PosEqns.clear();
        NegUnits.clear();
        NonUnits.clear();
        DelPtr(termIndex);

    }

    /*---------------------------------------------------------------------*/
    /*                       Inline  Function                              */
    /*---------------------------------------------------------------------*/
    /// 返回被处理过的 所有子句个数
    /// \return 

    inline uint32_t ClaNumber() {
        return PosRules.size() + PosEqns.size() + NegUnits.size() + NonUnits.size();
    }

    /// 返回被处理过的 单元子句个数
    /// \return 

    inline uint32_t UnitClaNum() {
        return PosRules.size() + PosEqns.size() + NegUnits.size();
    }

    inline void PrintIndex() {
        termIndex->Print();
    }

    inline void PrintNegIndex() {
        termIndex->Print();
    }

    /*---------------------------------------------------------------------*/
    /*                  Member Function-[public]                           */
    /*---------------------------------------------------------------------*/
    //

    void Proc(Clause* selCla);

    uint32_t Insert(Clause* cla);

};

class UnProcessed {
private:
    ClauseSet* unprocClaSet;
    list<Clause*> unprocLstCla;
public:
    /*---------------------------------------------------------------------*/
    /*                    Constructed Function                             */
    /*---------------------------------------------------------------------*/
    //

    UnProcessed(list<Clause*>&_unprocLstCla) : unprocLstCla(_unprocLstCla) {
    };

    UnProcessed(ClauseSet* _unprocClaset) : unprocClaSet(_unprocClaset) {

    };

    virtual ~UnProcessed() {
        unprocClaSet->FreeClauses();
    };
    /*---------------------------------------------------------------------*/
    /*                  Member Function-[public]                           */
    /*---------------------------------------------------------------------*/
    //

    Clause* GetBestClause() {
        return unprocClaSet->ClauseSetExtractFirst();
    }

    uint32_t getClaNum() {
        return unprocClaSet->Members();
    }

    void RemoveCla(Clause* cla) {
        unprocClaSet->RemoveClause(cla);
    }
    void Sort(){
        this->unprocClaSet->Sort();
    }
};

class ProofControl {
public:

    Processed *procedSet;
    UnProcessed *unprocSet;
    ClauseSet* axiom; //原始子句集
    FormulaSet* formulaSet; //公式集

public:
    //ProofControl( FormulaSet* formulaSet);
    ProofControl(ClauseSet* _claSet);
    ProofControl(const ProofControl& orig);
    virtual ~ProofControl();
public:
    /*---------------------------------------------------------------------*/
    /*                  Member Function-[public]                           */
    /*---------------------------------------------------------------------*/
    void parseInput();
    Clause* Saturate();
};

#endif /* PROOFCONTROL_H */

