/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Resolution.cpp
 * Author: Zhong Jian<77367632@qq.com>
 * 
 * Created on 2018年11月23日, 下午3:52
 */

#include <bits/algorithmfwd.h>
#include <stdbool.h>

#include "Resolution.h"
#include "TriAlg.h"
#include "Inferences/Simplification.h"

Resolution::Resolution() {
}

Resolution::Resolution(const Resolution& orig) {
}

Resolution::~Resolution() {
}


/*-----------------------------------------------------------------------
 *算法: 
 * 1. 选择起步子句. A.非单元子句; B.非等词公理子句
 * 2. 完成三角形构建 A.单元子句在构建过程中添加到子句集合.B.生成的新子句在构建完成后添加到子句集
 * 3. 重新选择起步子句
 * 
/*---------------------------------------------------------------------*/
//

RESULT Resolution::BaseAlg(Formula* fol) {
    uint16_t modifyLitNumCount = 0; //修改文字长度次数

    map<Clause*, int16_t> claWight;
    //程序结束条件1、得到归结结论；2、用户设置的时间限制，3、内存限制。
    list<Clause*>::iterator itSelCla = fol->getNextStartClause(); // fol->getWorkClas()->begin();

    uint32_t iterNum = 0;
    //不能作为起步子句集合--A.单元子句,B等词公理不能作为起步子句
    set<Clause*> notStartClaSet;
    RESULT res = RESULT::UNKNOWN;
    TriAlg triAlg(fol);
    while (StrategyParam::IterCount_LIMIT > iterNum) {
        //        if (fol->getWorkClas()->size() < 2) {  return RESULT::SAT;   }
        //构建三角形
        if ((*itSelCla)->isDel()) {//|| (isBinaryCla&&(*itSelCla)->LitsNumber()<3)) {
            ++itSelCla;
            if (itSelCla == fol->getWorkClas()->end())
                itSelCla = fol->getWorkClas()->begin();
            ++iterNum;
            continue;
        }
       //debug if(iterNum==354)                    printf("Debug:");
        //定义起步子句
        Clause* selCla = *itSelCla;
        res = triAlg.GenreateTriLastHope(selCla);

        // ======  子句起步没有构建任何三角形 ======
        if (res == RESULT::NOMGU) {
            notStartClaSet.insert(*itSelCla);
            if (notStartClaSet.size() == fol->getWorkClas()->size()) {
                fprintf(stdout, "Find start clause failed\n"); // 所有子句起步均找不到符合限制的合一路径，可能限制太严格，也可能为SAT！
                if (++modifyLitNumCount > 20)
                    return RESULT::UNKNOWN;

                //修改文字个数限制
                ++StrategyParam::MaxLitNumOfR;
                StrategyParam::MaxLitNumOfNewCla =+2;
                if (StrategyParam::MaxLitNumOfR > fol->uMaxLitNum - 1) {
                    StrategyParam::MaxLitNumOfR = 1;
                    StrategyParam::MaxLitNumOfNewCla = 1;
                }


                FileOp::getInstance()->outLog("修改R_MAX_LITNUM限制:" + to_string(StrategyParam::MaxLitNumOfR) + "\n");
                notStartClaSet.clear();
            }

            fprintf(stdout, "Clause %u,constructing Contradiction failed\n", (*itSelCla)->ident);
            (*itSelCla)->priority -= StrategyParam::CLA_NOMGU_WIGHT;
            if ((++itSelCla) == fol->getWorkClas()->end())
                itSelCla = fol->getWorkClas()->begin();
            triAlg.subst->Clear();
            triAlg.disposeRNUnitCla();
            continue;
        }
        if (0 == Env::S_OverMaxLitLimit_Num % 30000 && StrategyParam::HoldLits_NUM_LIMIT < fol->uMaxLitNum + 2) {
            Env::S_OverMaxLitLimit_Num = 0;
            //修改文字个数限制          
            ++StrategyParam::HoldLits_NUM_LIMIT;
            FileOp::getInstance()->outLog("修改R长度限制:" + to_string(StrategyParam::HoldLits_NUM_LIMIT) + "\n");
        }
        //记录三角形构建次数
        ++iterNum;
        //输出到.r/.i文件
        triAlg.outTri();
        triAlg.outR(nullptr);
        /*判定不可满足*/
        if (res == RESULT::UNSAT || triAlg.vNewR.empty()) {
            fprintf(stdout, "Start From Clause %u,proof found!\n", (*itSelCla)->ident);
            //cnf(c_0_6,plain,    ( $false ),    inference(sr,[status(thm)],[c_0_4,c_0_5]),    [proof]).          
            triAlg.outNewClaInfo(nullptr, InfereType::SCS);
            return RESULT::UNSAT;
        }

        for (Clause* newCla : triAlg.newClas) {//对生成的新子句进行处理,A.backword subsump B.等词处理
            if (newCla->isUnit()) {
                set<Clause*>outDelClas;
                newCla->ClauseTSTPPrint(stdout, true, true);
                cout << endl;
                if (Simplification::BackWardSubsumption(newCla, fol->allTermIndex, outDelClas)) {
                    /*
                     * 对冗余子句进行处理  若删除一个子句, 1,删除文字,2.全局索引需要删除 3.其他索引需要删除
                     * 因此删除子句比较麻烦, 处理思路, 给子句做标注, 然后放到子句集末尾,.....
                     */
                    string newClaId = to_string(newCla->ident);
                    for (Clause* delCla : outDelClas) {
                        // fol->removeWorkCla(delCla);
                        delCla->ClauseSetProp(ClauseProp::CPDeleteClause); //设置被删除子句
                        string strLog = "[BS]C" + to_string(delCla->ident) + " removed by C" + newClaId + "\n";
                        FileOp::getInstance()->outLog(strLog);

                    }
                }
            }
            if (newCla->LitsNumber() > StrategyParam::MaxLitNumOfNewCla) {
                DelPtr(newCla);
                continue;
            }
            int pri = 0;
            for_each(triAlg.vNewR.begin(), triAlg.vNewR.end(), [&pri](Literal * lit) {
                pri += lit->claPtr->priority;
            }
            );
            /*改变新子句的权重R的权重为文字权重的平均--遍历第一个△路径除外 取整*/
            //注意:由于目标子句初始化权重为100 因此 平均值后 若新子句中有目标子句参与自然权重会较高
            newCla->priority = pri / (int) (triAlg.vNewR.size());
            fol->insertNewCla(newCla);
            newCla->ClausePrint(stdout, true);
            //输出新子句到文件
            triAlg.outNewClaInfo(newCla, InfereType::SCS);

        }

        //改变参与归结的子句优先级,减1;  ---  理由:参与归结的子句 本质上是由 文字决定的,因此不需要改变参与子句的优先级,只需要改变起步子句的优先级即可
        //        for_each(triAlg.setUsedCla.begin(), triAlg.setUsedCla.end(), [](Clause * cla) {
        //            --cla->priority;
        //        });
        //只修改起步子句的优先级
        --(*itSelCla)->priority;
        itSelCla = fol->getNextStartClause();
        triAlg.subst->Clear();
        triAlg.disposeRNUnitCla();
    }
    return RESULT::UNKNOWN;
}

/// 只对二元子句进行单元子句处理合一
/// \param fol
/// \return 

RESULT Resolution::BaseAlgByOnlyBinaryCla(Formula* fol) {

    //排序目标子句--文字由多到少.
    list<Clause*>* lsWorkClas = fol->getWorkClas();
    //    std::sort(lsWorkClas->begin(), lsWorkClas->end(), [](Clause* c1, Clause * c2)->bool {
    //        return c1->LitsNumber() > c2->LitsNumber();});
    Subst_p subst = new Subst();
    TriAlg triAlg(fol);
    for (Clause* cla : *lsWorkClas) {
        if (cla->LitsNumber() > 2)
            break;
        if (triAlg.GenByBinaryCla(cla) == RESULT::UNSAT) {
            fprintf(stdout, "Start From Clause %u,proof found!\n", cla->ident);
            return RESULT::UNSAT;
        }
        //将二元子句 全部放到工作集的末尾,也就是说,后续操作 不用二元子句起步 ,需要改变权重
        cla->priority = INT_MIN;

    }
    return RESULT::UNKNOWN;
}