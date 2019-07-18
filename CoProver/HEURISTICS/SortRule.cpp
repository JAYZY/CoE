/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "SortRule.h"
#include "CLAUSE/Clause.h"

/************************************************************************/
/*    子句排序--选择排序规则   优先级 文字 权重                         */
/************************************************************************/
//

bool SortRule::ClaCmp(const Clause* pc1, const Clause* pc2) {
    int prio = pc1->priority - pc2->priority; //优先级越大越好

    if (prio != 0)
        return prio > 0;

    int litsNum = pc1->LitsNumber() - pc2->LitsNumber(); //文字数比较 - 由少到多
    int weightJ = pc1->weight - pc2->weight; //weight比较 - 越大越好 目前采用KBO序.后续可以用稳定度进行测试
    switch (StrategyParam::CLAUSE_SEL_STRATEGY) {
        case ClaSelStrategy::Num_Weight:
            if (litsNum != 0) {
                return litsNum < 0;
            }
            return weightJ > 0;
            break;
        case ClaSelStrategy::Weight_Num:
            if (weightJ != 0)
                return weightJ > 0;
            return litsNum < 0;
            break;
    }
    return false;
}