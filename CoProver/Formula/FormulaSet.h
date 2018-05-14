/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   FormulaSet.h
 * Author: Zhong Jian<77367632@qq.com>
 *
 * Created on 2018年4月23日, 下午7:23
 */

#ifndef FORMULASET_H
#define FORMULASET_H
#include "Global/IncDefine.h"
#include  "WFormula.h"
#include "ClauseSet.h"
//公式集合

class FormulaSet {
private:
    WFormula* anchor;
    long members;
    string identifier;
public:
    /*---------------------------------------------------------------------*/
    /*                    Constructed Function                             */
    /*---------------------------------------------------------------------*/
    FormulaSet();
    FormulaSet(const FormulaSet& orig);
    virtual ~FormulaSet();
public:

    /*---------------------------------------------------------------------*/
    /*                       Inline  Function                              */
    /*---------------------------------------------------------------------*/
    /// 返回公式集的数量
    /// \return 

    inline long FormulaSetCardinality() {
        return members;
    }
    /// 判断公式集合是否为空
    /// \return 

    inline bool FormulaSetEmpty() {
        return anchor->succ == anchor;
    }
    /// 修改Member的值
    /// \param value
    /// \return 

    inline long ChangeMember(long value) {
        members += value;
        return members;
    }

    /// 若存在返回公式集中第一个公式,否则返回 nullptr
    /// \return 第一个公式,或nullptr

    inline WFormula* FormulaSetExtractFirst() {

        if (FormulaSetEmpty()) {
            return nullptr;
        }
        return FormulaSetExtractEntry(this->anchor->succ);
    }
    /// 将指定的公式form从公式集中删除,并返回该公式
    /// \param form 需要删除的公式
    /// \return 该公式

    inline WFormula* FormulaSetExtractEntry(WFormula* form) {
        assert(form);
        assert(form->set);
        form->pred->succ = form->succ;
        form->succ->pred = form->pred;
        form->set->members--;
        form->set = nullptr;
        form->succ = nullptr;
        form->pred = nullptr;
        return form;
    }
    /*---------------------------------------------------------------------*/
    /*                  Member Function-[public]                           */
    /*---------------------------------------------------------------------*/
    long FormulaAndClauseSetParse(Scanner* in, ClauseSet* cset, SplayTree<StrTreeCell>&name_selector, SplayTree<StrTreeCell>&skip_includes);

    /// 将一个公式插入公式集合
    /// \param newform
    void FormulaSetInsert(WFormula* newform);
    /// 合并公式集formset
    /// \param from 需要合并的公式集合
    /// \return 合并的公式集中公式的数量
    long FormulaSetInsertSet(FormulaSet* fromset);

    long FormulaSetCNF(FormulaSet* set, FormulaSet* archive, ClauseSet* clauseset, TB_p terms, VarBank* fresh_vars);
    long FormulaSetSimplify(TB_p terms);
};

#endif /* FORMULASET_H */




