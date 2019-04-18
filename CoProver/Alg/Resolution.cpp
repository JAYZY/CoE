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
    //定义起步子句
    Clause* selCla = nullptr;
    map<Clause*, int16_t> claWight;
    //程序结束条件1、得到归结结论；2、用户设置的时间限制，3、内存限制。
    list<Clause*>::iterator itSelCla = fol->getWorkClas()->begin();

    uint32_t iterNum = 0;
    //不能作为起步子句集合--A.单元子句,B等词公理不能作为起步子句
    set<Clause*> notStartClaSet;
    RESULT res = RESULT::UNKNOWN;
    TriAlg triAlg(fol);
    while (StrategyParam::IterCount_LIMIT > iterNum) {
        //        if (fol->getWorkClas()->size() < 2) {
        //            return RESULT::SAT;
        //        }
        //构建三角形

        if ((*itSelCla)->isDel()) {
            ++itSelCla;
            if (itSelCla == fol->getWorkClas()->end())
                itSelCla = fol->getWorkClas()->begin();
            continue;
        }
        res = triAlg.GenreateTriLastHope(*itSelCla);

        // ======  子句起步没有构建任何三角形
        if (res == RESULT::NOMGU) {
            notStartClaSet.insert(*itSelCla);
            if (notStartClaSet.size() == fol->getWorkClas()->size()) {
                fprintf(stdout, "Find start clause failed\n"); // 所有子句起步均找不到符合限制的合一路径，可能限制太严格，也可能为SAT！
                return RESULT::UNKNOWN;
            }
            fprintf(stdout, "Clause %u,constructing Contradiction failed\n", (*itSelCla)->ident);
            (*itSelCla)->priority -= StrategyParam::CLA_NOMGU_WIGHT;
            if ((++itSelCla) == fol->getWorkClas()->end())
                itSelCla = fol->getWorkClas()->begin();
            continue;
        }
        //记录三角形构建次数
        ++iterNum;


        //输出到.r/.i文件
        triAlg.outTri();
        triAlg.outR(nullptr);
        /*判定不可满足*/
        if (res == RESULT::UNSAT || triAlg.vNewR.empty()) {
            fprintf(stdout, "Start From Clause %u,proof found!\n", (*itSelCla)->ident);
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
                        FileOp::getInstance()->outRun(strLog);
                    }
                }

            }
            int pri = 0;
            for_each(triAlg.vNewR.begin(), triAlg.vNewR.end(), [&pri](Literal * lit) {
                pri += lit->claPtr->priority;
            }
            );
            /*改变R的权重	 R的权重为文字权重的平均--遍历第一个△路径除外 取整*/
            newCla->priority = pri /(triAlg.vNewR.size());
            fol->insertNewCla(newCla);
            triAlg.outNewClaInfo(newCla, InfereType::SCS);
        }

        //改变参与归结的子句优先级,减1;
        for_each(triAlg.setUsedCla.begin(), triAlg.setUsedCla.end(), [](Clause * cla) {
            --cla->priority;
        });
        itSelCla = fol->getNextStartClause();
    }

}

RESULT Resolution::BaseAlgByRecodePath(Formula* fol) {
    //从目标子句集中的子句起步，目前策略基于目标的归结
    /*目标子句集, 既包含单元子句,和多文字子句.选取的方式,    
     * 1.单元子句因为都放在 前面的,因此选择起步子句时候不选择单元目标子句起步.
     * 2.若原始子句集中只有单元目标子句,则从单元子句集合中选择
     */
    //排序目标子句--文字由多到少.
    std::sort(fol->goalClaset.begin(), fol->goalClaset.end(), [](Clause* c1, Clause * c2)->bool {
        return c1->LitsNumber() > c2->LitsNumber();
    });
    //单元子句排序
    fol->unitClasSort();

    RESULT res = RESULT::UNKNOWN;
    TriAlg triAlg(fol);

    list<Clause*>::iterator itSelCla = fol->getWorkClas()->begin();
    uint32_t iterNum = 0;
    while (StrategyParam::IterCount_LIMIT > iterNum && itSelCla != fol->getWorkClas()->end()) {
        ++iterNum;
        res = triAlg.GenerateOrigalTriByRecodePath((*itSelCla));
        if (res == RESULT::UNSAT) {
            return res;
        }
        ++itSelCla;

    }
    //    for (int i = 0; i < fol->goalClaset.size(); i++) {
    //        Clause* gCla = fol->goalClaset[i];
    //        //  gCla->ClausePrint(stdout,true);
    //       // res = triAlg.GenerateTriByRecodePath(gCla);
    //        res=triAlg.GenerateOrigalTriByRecodePath(gCla);
    //        if (res == RESULT::UNSAT) {
    //            return res;
    //        }
    //    }

    cout << (int) res << endl;
    return res;
}