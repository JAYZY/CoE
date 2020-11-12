/* 
 * File:   SortRule.h
 * Author: Zhong Jian<77367632@qq.com>
 * Contents: 定义一系列的 排序规则
 * Created on 2018年10月27日, 下午12:58
 */

#ifndef SORTRULE_H
#define SORTRULE_H

#include "CLAUSE/Clause.h"
#include "StrategyParam.h"

class SortRule {
public:
    static int iActLitFuncDep;
    static int iParentNodeUseTimes;

    SortRule(void) {
    };

    ~SortRule(void) {
    };

    static inline float ComputeUBC(int qulity, int usedCount, int parentUsedCount) {
        if (usedCount == 0) //若没有被使用过 则 值为无穷大
            return INT_MAX * 1.0f;
        float C = 0.7071068f; // 1 / rsqrt(2) ; //常数,经验值为 2开方. 值越大越偏向广度搜索.越小偏向深度搜索.
        return qulity / usedCount + C * sqrt(2 * log(parentUsedCount) / usedCount);
    }


    /// 主动文字排序规则
    /// \param litA
    /// \param litB
    /// \return 

    static inline bool ActLitCmp(Literal* litA, Literal* litB) {


        //对子句中的文字进行排序 使用次数 冗余次数  稳定度  
        /*1. 优先使用次数少的文字;2,优先使用稳定低的文字 3.优先使用正文字; */
        if (litA->aUsedCount != litB->aUsedCount)
            return litA->aUsedCount < litB->aUsedCount; //优先使用次数少的文字
        long weight = 0;

        weight = StrategyParam::Weight_Sort == ActLitSteady::ASC ? litA->StandardWeight() - litB->StandardWeight()
                : litB->StandardWeight() - litA->StandardWeight(); //稳定度由低到高

        if (0 == weight) {
            if (litA->IsNegative() && litB->IsPositive())
                return true;
            if (litB->IsNegative() && litA->IsPositive())
                return false;
        }
        return weight > 0; //稳定度由低到高
    }



    /// 子句中文字的排序规则
    /// \param litA
    /// \param litB
    /// \return 

    static inline bool LitCmp(Literal* litA, Literal* litB) {

        //对子句中的文字进行排序 使用次数 冗余次数  稳定度  
        /*1. 优先使用次数少的文字;2,优先使用稳定低的文字 3.优先使用正文字; */
        //if (litA->aUsedCount != litB->aUsedCount)

        return litA->GetActUCB(litA->claPtr->userTimes) > litB->GetActUCB(litA->claPtr->userTimes); //选择值大的。

        //优先使用次数少的文字
        // long weight = StrategyParam::Weight_Sort == ActLitSteady::ASC ? litA->StandardWeight() - litB->StandardWeight()
        //         : litB->StandardWeight() - litA->StandardWeight(); //稳定度由低到高

        //        if (weight != 0) {
        //            return weight > 0; //稳定度由低到高
        //        }
        //        if (litA->IsNegative())
        //            return true;
        //
        //        return false;

    }

    /*--------------------------------------------------------------------------
    / 被动归结文字排序（排序的文字在同一个谓词对应的文字列表中）                /
    /1.文字使用最少的;1.子句所在文字数最少;2.用weight(KBO)代替稳定度，由低到高，  【文字稳定度(默认由低到高)】
     3.等词公理排后
    /-------------------------------------------------------------------------*/
    static bool PoslitCmp(Literal*litA, Literal*litB) {
        //排序规则 : 优先选择结构相同的.再选择文字少的     

        if (litA->claPtr->isAxiom()) {
            return false;
        }
        if (litB->claPtr->isAxiom()) {
            return true;
        }

        int32_t litNumDiff = litB->claPtr->LitsNumber() - litA->claPtr->LitsNumber();
        int litPostUseDiff = litB->pUsedCount - litA->pUsedCount; //因为存在回退问题,因此排序不考虑 使用次数问题
        int structLikeDiff = 0;
        if (iActLitFuncDep == -1) {
            structLikeDiff = (StrategyParam::Weight_Sort == ActLitSteady::ASC) ? litB->StandardWeight() - litA->StandardWeight() //由低到高
                    : litA->StandardWeight() - litB->StandardWeight();
        } else {
            structLikeDiff = (ABS((int) litB->StandardWeight() - iActLitFuncDep) - ABS((int) litA->StandardWeight() - iActLitFuncDep)); //结构相似 值越小 越相似
        }
        //所在子句文字数少的优先
        if (litNumDiff != 0) {
            return litNumDiff > 0;
        } else if (structLikeDiff != 0) {

            return structLikeDiff > 0;
        }
        return litPostUseDiff > 0;
    }
    /// 被动候选文字的排序规则,等词公理排后
    /// \param litA
    /// \param litB
    /// \return 

    static bool PoslitCmpByUCB(Literal*litA, Literal*litB) {
        
         if (litA->claPtr->isAxiom()) {
            return false;
        }
        if (litB->claPtr->isAxiom()) {
            return true;
        }
        int litUCB = iParentNodeUseTimes == -1 ? 0
                : litA->GetPasUCB(iParentNodeUseTimes) - litB->GetPasUCB(iParentNodeUseTimes); //选择值大的。

        if (litUCB != 0) {
            return litUCB > 0;
        }

        //所在子句文字数少的优先
        int32_t litNumDiff = litB->claPtr->LitsNumber() - litA->claPtr->LitsNumber();
        if (litNumDiff != 0) {
            return litNumDiff > 0;
        }

        int structLikeDiff = 0;
        if (iActLitFuncDep == -1) {
            //结构相似
            structLikeDiff = (StrategyParam::Weight_Sort == ActLitSteady::ASC) ? litB->StandardWeight() - litA->StandardWeight() //由低到高
                    : litA->StandardWeight() - litB->StandardWeight();
        } else {
            //字符-反映变元数

            structLikeDiff = (ABS((int) litB->StandardWeight() - iActLitFuncDep) - ABS((int) litA->StandardWeight() - iActLitFuncDep)); //结构相似 值越小 越相似
        }

        return structLikeDiff > 0;

    }


    /************************************************************************/
    /*    子句排序--选择排序规则                                            */
    /************************************************************************/
    static bool ClaCmp(const Clause*pc1, const Clause * pc2);

    /// 单元子句比较
    /// \param litA
    /// \param litB
    /// \return 

    static inline bool UnitClaCmp(const Clause* pc1, const Clause* pc2) {

        return LitCmp(pc1->literals, pc2->literals);
    }

    //优先级比较

    static inline bool ClaCmpByPrio(const Clause* pc1, const Clause * pc2) {
        return (pc1->priority > pc2->priority);
    }
};


#endif /* SORTRULE_H */

