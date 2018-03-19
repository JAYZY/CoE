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

#include "Unify.h"
#include "TERMS/TermCell.h"

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
/// 检查 匹配项通过变元替换是否与被

/// \param varMatcher 匹配项(可变项)
/// \param ConMatched 被匹配项(不可变项)
/// \param subst  替换项
/// \return 

bool Unify::SubstComputeMatch(TermCell* varMatcher, TermCell* ConMatched, Subst* subst) {

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
    if (ConMatched->TermCellQueryProp(TermProp::TPPredPos) && varMatcher->IsVar()) {
        //ps:变元符号不与谓词符号匹配
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
            jobs.pop_back();
            varMatcher = jobs.back();
            jobs.pop_back();

            if (varMatcher->IsVar()) {
                if (varMatcher->binding) {
                    if (varMatcher->binding != ConMatched) {
                        res = false;
                        break;
                    }
                } else {
                    subst->SubstAddBinding(varMatcher, ConMatched);
                }
                matcher_weight += (ConMatched->TermStandardWeight() - DEFAULT_VWEIGHT);

                if (matcher_weight > to_match_weight) {
                    res = false;
                    break;
                }
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