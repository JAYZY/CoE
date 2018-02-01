
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
class Processed {
private:
    list<Clause*> PosRules; //处理过的正文字 单元子句 (有序的,要么不是等词,要么已经左项>右项) 
    list<Clause*> PosEqns; //处理过的正文字单元子句(无序的等词文字)
    list<Clause*> NegUnits; //单元子句,只有一个 负文字
    list<Clause*> NonUnits; //其他非单元子句
    DiscrimationIndexing *ti;
public:
    /*---------------------------------------------------------------------*/
    /*                    Constructed Function                             */
    /*---------------------------------------------------------------------*/
    //

    Processed() {
        ti = new DiscrimationIndexing();
    }

    virtual ~Processed() {
        PosRules.clear();
        PosEqns.clear();
        NegUnits.clear();
        NonUnits.clear();
        DelPtr(ti);
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
    /*---------------------------------------------------------------------*/
    /*                  Member Function-[public]                           */
    /*---------------------------------------------------------------------*/
    //
    void Proc(Clause* selCla);
    void PrintTi(){ti->Print();}
};

class UnProcessed {
private:
    list<Clause*> unprocClaSet;
public:
    /*---------------------------------------------------------------------*/
    /*                    Constructed Function                             */
    /*---------------------------------------------------------------------*/
    //

    UnProcessed(list<Clause*>&_unproc) : unprocClaSet(_unproc) {       
    };

    virtual ~UnProcessed() {
        unprocClaSet.clear();
    };
    /*---------------------------------------------------------------------*/
    /*                  Member Function-[public]                           */
    /*---------------------------------------------------------------------*/
    //

    Clause* GetBestClause() {
        return unprocClaSet.back();
    }
    uint32_t getClaNum(){return unprocClaSet.size();}
    void RemoveCla(Clause* cla) {
        unprocClaSet.remove(cla);        
    }
};

class ProofControl {
private:

    Processed *procedSet;
    UnProcessed *unprocSet;
    list<Clause*> axiom; //原始子句集



public:
    ProofControl(list<Clause*>& _axiom);
    ProofControl(const ProofControl& orig);
    virtual ~ProofControl();
public:
    /*---------------------------------------------------------------------*/
    /*                  Member Function-[public]                           */
    /*---------------------------------------------------------------------*/
    Clause* Saturate();
};

#endif /* PROOFCONTROL_H */

