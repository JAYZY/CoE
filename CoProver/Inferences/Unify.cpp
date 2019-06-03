/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Unify.cpp
 * Author: Zhong Jian<77367632@qq.com>
 * 
 * Created on 2018年3月1日, 下午1:16
 */

#include <queue>

#include "Unify.h"
#include "Terms/TermCell.h"
#include "CLAUSE/Literal.h"
CPUTIME_DEFINE(MguTimer);

Unify::Unify() {

}

Unify::Unify(const Unify& orig) {
}

Unify::~Unify() {

}

/*---------------------------------------------------------------------*/
/*                  Member Function-[public]                           */
/*---------------------------------------------------------------------*/
//
/// 检查 匹配项通过变元替换是否为相同变元

/*-----------------------------------------------------------------------
//
// Function: occur_check()
//
//   Occur check for variables, possibly more efficient than the
//   general TermIsSubterm()
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool Unify::OccurCheck(Term_p term, Term_p var) {
    term = TermCell::TermDerefAlways(term);

    if (UNLIKELY(term == var)) { //如果两个项相同 直接返回true;
        return true;
    }

    for (int i = 0; i < term->arity; i++) {
        if (OccurCheck(term->args[i], var)) {
            return true;
        }
    }
    return false;
}
/// \param varMatcher 匹配项(可变项)
/// \param ConMatched 被匹配项(不可变项)
/// \param subst  替换项
/// \return 

bool Unify::SubstComputeMatch(TermCell* varMatcher, TermCell* ConMatched, Subst* subst) {
    if (ConMatched->TermCellQueryProp(TermProp::TPPredPos)) {

        //均为谓词 但不相同
        if (varMatcher->TermCellQueryProp(TermProp::TPPredPos) && ConMatched->fCode != varMatcher->fCode) {
            return false;
        }
        //ps:变元符号不与谓词符号匹配
        if (varMatcher->IsVar())
            return false;
    }
    long matcher_weight = varMatcher->TermStandardWeight(),
            to_match_weight = ConMatched->TermStandardWeight();
    bool res = true;

    assert(varMatcher->TermStandardWeight() ==
            varMatcher->TermWeight(DEFAULT_VWEIGHT, DEFAULT_FWEIGHT));

    assert(ConMatched->TermStandardWeight() ==
            ConMatched->TermWeight(DEFAULT_VWEIGHT, DEFAULT_FWEIGHT));


    if (matcher_weight > to_match_weight) { //ps:需要检查的项to_match中变元数量< matcher中的变元数
        return false;
    }

    /* New block to get fresh local variables */
    {
        vector<TermCell*> jobs;

        int backtrack = subst->Size(); /* For backtracking */

        int i;
        jobs.push_back(varMatcher);
        jobs.push_back(ConMatched);

        while (!jobs.empty()) {

            ConMatched = jobs.back();
            ConMatched = TermCell::TermDerefAlways(ConMatched); //因为 con为固定的 因此可以 获取该文字变元项的绑定            
            jobs.pop_back();
            
            varMatcher = jobs.back();            
            jobs.pop_back();
            
            if (varMatcher->IsVar()) {
                if (varMatcher->binding) { //若存在绑定 检查是否与固定项相同 注意 不同子句中的 变元 x1 是不相同的
                    if (varMatcher->binding != ConMatched) {
                        res = false;
                        break;
                    }
                } else if (varMatcher != ConMatched) {
                    subst->SubstAddBinding(varMatcher, ConMatched);
                }

                //                matcher_weight += (ConMatched->TermStandardWeight() - DEFAULT_VWEIGHT);
                //
                //                if (matcher_weight > to_match_weight) {
                //                    res = false;
                //                    break;
                //                }
            } else {
                if (varMatcher->fCode != ConMatched->fCode) {
                    res = false;
                    break;
                } else {
                    for (i = varMatcher->arity - 1; i >= 0; i--) {
                        jobs.push_back(varMatcher->args[i]);
                        jobs.push_back(ConMatched->args[i]);
                    }
                }
            }
        }
        jobs.clear();
        vector<TermCell*>().swap(jobs);

        if (!res) { //回滚替换            
            subst->SubstBacktrackToPos(backtrack);
        }
        return res;
    }
}

bool Unify::SubstComputeMgu(Term_p t1, Term_p t2, Subst_p subst) {
    //printf("Unify %lu %lu\n", t1->entry_no, t2->entry_no);
#ifdef MEASURE_UNIFICATION
    UnifAttempts++; //开启记录 合一的次数
#endif

    CPUTIME_ENTRY(MguTimer);
    if ((t1->TermCellQueryProp(TermProp::TPPredPos) && t2->IsVar()) ||
            (t2->TermCellQueryProp(TermProp::TPPredPos) && t1->IsVar())) {// P  x 谓词与变元不能合一
        CPUTIME_EXIT(MguTimer);
        return false;
    }
    int backtrack = subst->Size(); /* For backtracking */
    bool res = true;
    deque<TermCell*> jobs; //工作队列
    jobs.push_back(t1);
    jobs.push_back(t2);

    while (!jobs.empty()) {
        t2 = TermCell::TermDerefAlways(jobs.back());
        jobs.pop_back();
        t1 = TermCell::TermDerefAlways(jobs.back());
        jobs.pop_back();


        if (t2->IsVar()) {
            SWAP(t1, t2);
        }

        if (t1->IsVar()) {
            if (t1 != t2) {
                /* Sort check and occur check - remember, variables are elementary and shared! */
                if (OccurCheck(t2, t1)) {
                    res = false;
                    break;
                } else {
                    subst->SubstAddBinding(t1, t2);
                }
            }
        } else {
            if (t1->fCode != t2->fCode) {
                res = false;
                break;
            } else {
                for (int i = t1->arity - 1; i >= 0; i--) {
                    /* Delay variable bindings */
                    if (t1->args[i]->IsVar() || t2->args[i]->IsVar()) {
                        jobs.push_front(t2->args[i]);
                        jobs.push_front(t1->args[i]);
                    } else {
                        jobs.push_back(t1->args[i]);
                        jobs.push_back(t2->args[i]);
                    }
                }
            }
        }
    }
    if (!res) {
        subst->SubstBacktrackToPos(backtrack);
    } else {
#ifdef MEASURE_UNIFICATION
        UnifSuccesses++;
#endif
    }
    CPUTIME_EXIT(MguTimer);
    return res;
}

/*一般文字的合一检查
 * --不检查等词的可交换性
 */
bool Unify::literalMgu(Literal* litA, Literal* litB, Subst_p subst) {
    long backtrack = subst->Size();
    bool res = SubstComputeMgu(litA->lterm, litB->lterm, subst);
    if (res) {
        res = SubstComputeMgu(litA->rterm, litB->rterm, subst);
    }
    if (!res) {
        subst->SubstBacktrackToPos(backtrack);
    }
    return res;
}

/*等词合一测试---需要检查等词的可交换性[deal with commutativity of  equality.] 
 * Test wether two equations are unifyable. If yes, return true
 * and extend subst to give the substitution, otherwise just return true 
 * and let subst unmodifies. Equations are treated as 2-sets of terms 
 * unless both are oriented.
 */
bool Unify::literalEMgu(Literal* litA, Literal* litB, Subst_p subst) {
    bool res = literalMgu(litA, litB, subst);
    if (res || (litA->IsOriented() && litB->IsOriented())) {
        return res;
    }

    litA->swapSides();
    res = literalMgu(litA, litB, subst);
    litA->swapSides();

    return res;
}


//
//int Unify::SubstAddBinding(TermCell* var, TermCell* bind, vector<TermCell*> &vecSubst) {
//    assert(var);
//    assert(bind);
//    assert(var->IsVar());
//    assert(!(var->binding));
//    assert(!bind->TermCellQueryProp(TermProp::TPPredPos));
//    /* printf("# %ld <- %ld \n", var->f_code, bind->f_code); */
//
//    var->binding = bind;
//    vecSubst.push_back(var);
//}
//
///***************************************************************************** 
// * Backtrack a single binding and remove it from the substitution (if possible). 
// * Return true if successful, false if the substitutuion is empty.
// ****************************************************************************/
//bool Unify::SubstBacktrackSingle(vector<TermCell*> &vecSubst) {
//    Term_p handle;
//    if (vecSubst.empty()) {
//        return false;
//    }
//    handle = vecSubst.back();
//    vecSubst.pop_back();
//    handle->binding = NULL;
//    return true;
//}
//
///***************************************************************************** 
// * 回滚替换变元,直到给定的位置pos -- Backtrack variable bindings up to (down to?) a given stack pointer position.
// ****************************************************************************/
//int Unify::SubstBacktrackToPos(int pos, vector<TermCell*> &vecSubst) {
//    int ret = 0;
//    while (vecSubst.size() > pos) {
//        SubstBacktrackSingle(vecSubst);
//        ++ret;
//    }
//    return ret;
//}
//
///***************************************************************************** 
// * Undo all stored variable binding in subst.
// ****************************************************************************/
//int Unify::SubstBacktrack(vector<TermCell*> &vecSubst) {
//    int ret = 0;
//    while (SubstBacktrackSingle(vecSubst)) {
//        ++ret;
//    }
//    return ret;
//}