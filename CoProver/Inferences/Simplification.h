/* 
 * File:   Simplification.h
 * Author: Zhong Jian<77367632@qq.com>
 *
 * Created on 2018年2月23日, 下午1:59
 */

#ifndef SIMPLIFICATION_H
#define SIMPLIFICATION_H
#include "CLAUSE/Clause.h"
#include "Indexing/TermIndexing.h"

class Simplification {
protected:

    //vector<vector<TermCell*>* >stChgVars; //记录有绑定的变元项

public:

    Simplification();
    Simplification(const Simplification& orig);
    virtual ~Simplification();

public:

    //检查子句 genCla是否有效,有任意替换r使得 g*r = c ?
    static bool ForwordSubsumption(Clause* genCla, TermIndexing* indexing);

    static bool BackWordSubsumption(Clause* genCla, TermIndexing* indexing);

private:

    /// 检查文字列表是否是包含冗余
    /// \param subsumer
    /// \param subsumed
    /// \param subst
    /// \return 
    static bool LitListSubsume(Literal* subsumVarLst, Literal* exceptLit, Literal* subsumConLst, Subst*subst, int8_t* pickLst);

    /// 检查单个文字相互是否是包含冗余
    /// \param subsumer
    /// \param subsumed
    /// \param subst
    /// \return 
    static bool LitSubsume(Literal* subsumer, Literal* subsumed, Subst* subst);

};

#endif /* SIMPLIFICATION_H */

