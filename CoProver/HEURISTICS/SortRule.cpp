/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "SortRule.h"
#include "CLAUSE/Clause.h"

int SortRule::iActLitFuncDep = -1;
/************************************************************************/
/*    子句排序--选择排序规则   优先级 文字 权重                         */
/************************************************************************/
// 优先保证 文字数优先， 文字数相同情况下 考虑优先级，优先级相同考虑复杂度

bool SortRule::ClaCmp(const Clause* pc1, const Clause* pc2) {

    int prio = pc1->priority - pc2->priority; //优先级越大越好
    int weightJ = pc1->claWeight - pc2->claWeight; //weight比较 - 越大越好 目前采用KBO序.后续可以用稳定度进行测试
    int litsNum = pc1->LitsNumber() - pc2->LitsNumber(); //文字数比较 - 由少到多
    int posLitsNum = pc1->posLitNo - pc2->posLitNo; //正文字越多越好

    switch (StrategyParam::CLAUSE_SEL_STRATEGY) {
        case ClaSelStrategy::Num_Prio_Weight:
            if (litsNum != 0) {
                return litsNum < 0;
            }  else if (prio != 0) {
                return prio > 0;
            }
            return weightJ > 0;
            break;
        case ClaSelStrategy::Num_Weight_Prio:
            if (litsNum != 0) {
                return litsNum < 0;
            } 
             else if (weightJ != 0) {
                return weightJ > 0;
            }
            return prio > 0;
            break;
        case ClaSelStrategy::Num_Prio_Post_Weight:
            if (litsNum != 0) {
                return litsNum < 0;
            } else if (prio != 0) {
                return prio > 0;
            } else if (posLitsNum != 0) {
                return posLitsNum > 0; //正文字越多越好
            }
            return weightJ > 0;
            break;
    }
    return false;
}