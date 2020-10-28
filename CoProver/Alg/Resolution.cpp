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
#include "HEURISTICS/SortRule.h"
#include "TriAlgExt.h"

Resolution::Resolution() {
}

Resolution::Resolution(const Resolution& orig) {
}

Resolution::~Resolution() {
}


/*-----------------------------------------------------------------------
 *算法: 标准延拓
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
    //策略改进,正文字子句优先级高 
    //  vector<Clause*>::iterator itSelCla = fol->getNextGoalClause();  //fol->getWorkClas()->begin();
    list<Clause*>::iterator itSelCla = fol->getNextStartClause(); //
    uint32_t iterNum = 0;
    //不能作为起步子句集合--A.单元子句,B等词公理不能作为起步子句
    set<Clause*> notStartClaSet;
    RESULT res = RESULT::UNKNOWN;
    TriAlg triAlg(fol);
    StrategyParam::isFullUC = true;
    double startTime = CPUTime();
    bool isCheckT = false;
    set<Clause*> setDealedUnitCla; //已经 处理过得单元子句不再二元归结（只做一次自己和自己归结）

    StrategyParam::IS_RollBackGoalPath = false;
    int iGoalInd = 0;
    bool isGoal = false;
    while (StrategyParam::IterCount_LIMIT > iterNum) {

        if (!StrategyParam::IS_RollBackGoalPath && CPUTime() - startTime > 36) { //30秒
            StrategyParam::IS_RollBackGoalPath = true;
            cout << "Set IS_RollBackGoalPath=true" << endl;
        }
        Clause* selCla = *itSelCla; //fol->goalClaset[iGoalInd]; //
        //构建三角形               
        ++iterNum;
        //单元子句排序         // fol->unitClasSort();        //可满足判定  if (fol->getWorkClas()->size() < 2) {  return RESULT::SAT;   }
        //debug        if(iterNum==20950)            cout<<"iterNum:"<<iterNum<<endl;
        assert(selCla);
        // res = triAlg.GenreateTriLastHope(selCla);
        res = triAlg.GenBaseTriByLearn(selCla);
        /*判定不可满足*/
        if (res == RESULT::UNSAT) {
            assert(triAlg.vNewR.empty());
            fprintf(stdout, "Start From Clause %u,proof found!\n", (*itSelCla)->ident);
            return RESULT::UNSAT;
        }
        // ======  子句起步没有构建任何三角形 ======
        if (RESULT::NOMGU == res) {
            notStartClaSet.insert(*itSelCla);
            if (notStartClaSet.size() == fol->getWorkClas()->size()) {
                // fprintf(stdout, "Find start clause failed\n"); // 所有子句起步均找不到符合限制的合一路径，可能限制太严格，也可能为SAT！
                if (++modifyLitNumCount > 10000) {
                    cout << "UNKNOWN" << endl;
                    return RESULT::UNKNOWN;
                }
                //修改文字个数限制--只修改 ,新子句中文字的个数，R延拓的个数不增加
                if (++StrategyParam::MaxLitNumOfNewCla > fol->uMaxLitNumOfCla) {
                    //StrategyParam::MaxLitNumOfR = 1;
                    StrategyParam::MaxLitNumOfNewCla = StrategyParam::MaxLitNumOfR;
                }
                FileOp::getInstance()->outLog("修改R_MAX_LITNUM限制:" + to_string(StrategyParam::MaxLitNumOfR) + "\n");
                notStartClaSet.clear();
                itSelCla = fol->getNextStartClause();
                // itSelCla = fol->getNextGoalClause();
            } else {
                //fprintf(stdout, "Clause %u,constructing Contradiction failed\n", (*itSelCla)->ident);
                (*itSelCla)->priority -= 1; //StrategyParam::CLA_NOMGU_WIGHT;

                if ((++itSelCla) == fol->getWorkClas()->end())
                    itSelCla = fol->getWorkClas()->begin();
                /*if ((++itSelCla) == fol->goalClaset.end())
                    itSelCla = fol->goalClaset.begin();*/
                // selCla = *itSelCla;
            }
            continue;
        }// ======  △构建成功 ======
        else if (RESULT::SUCCES == res) {

            triAlg.subst->Clear();
            // for (Clause* newCla : triAlg.newClas) {//对生成的新子句进行处理,A.backword subsump B.等词处理
            uint32_t uNewClaSize = triAlg.newClas.size();
            for (int i = 0; i < triAlg.newClas.size(); i++) {

                Clause* newCla = triAlg.newClas[i];
                //debug if (newCla->ident >= 16765) cout << "debug" << endl;
                triAlg.OutNewClaInfo(newCla);

                //--- 若为单元子句,A. 检查是否有其他单元子句合一（UNSAT) B.用该单元子句与自己本身的子句进行二元归结，得到特殊子句优先使用 
                if (newCla->isUnit()) {
                    if (fol->isUnsat(newCla)) {
                        return RESULT::UNSAT;
                    }

                    //切记，若单元子句与父子句进行归结（自归结） 结果为NOMGU,表明自归结失败不能再做尝试。
                }

                //=== 若新子句为单元子句则做BS冗余检查,workset子句集中的子句是否为冗余子句
                {
                    //            if (newCla->isUnit()) {
                    //                set<Clause*>outDelClas;
                    //                newCla->ClauseTSTPPrint(stdout, true, true);
                    //                cout << endl;
                    //                if (Simplification::BackWardSubsumption(newCla, fol->allTermIndex, outDelClas)) {
                    //                    /*
                    //                     * 对冗余子句进行处理  若删除一个子句, 1,删除文字,2.全局索引需要删除 3.其他索引需要删除
                    //                     * 因此删除子句比较麻烦, 处理思路, 给子句做标注, 然后放到子句集末尾,.....
                    //                     */
                    //                    string newClaId = to_string(newCla->ident);
                    //                    for (Clause* delCla : outDelClas) {
                    //                        // fol->removeWorkCla(delCla);
                    //                        delCla->ClauseSetProp(ClauseProp::CPDeleteClause); //设置被删除子句
                    //                        string strLog = "[BS]C" + to_string(delCla->ident) + " removed by C" + newClaId + "\n";
                    //                        FileOp::getInstance()->outLog(strLog);
                    //
                    //                    }
                    //                }
                    //            }
                    //            if (newCla->LitsNumber() > StrategyParam::MaxLitNumOfNewCla) {
                    //                DelPtr(newCla);
                    //                continue;
                    //            }
                }

                //=== 修改新子句权重-------------
                int pri = 0;
                if (newCla->isGoal()) {
                    newCla->priority = 100;
                    //selCla = newCla;
                } else {
                    for (Literal* tmpLit = newCla->literals; tmpLit; tmpLit = tmpLit->next) {
                        pri += tmpLit->parentLitPtr->claPtr->priority;
                    }
                    /*改变新子句的权重R的权重为文字权重的平均--遍历第一个△路径除外 取整*/
                    //注意:由于目标子句初始化权重为100 因此 平均值后 若新子句中有目标子句参与自然权重会较高
                    newCla->priority = pri / (int) (newCla->LitsNumber());
                }
                fol->insertNewCla(newCla);

            }
        }
        //修改起步子句优先级
        //改变参与归结的子句优先级,减1;  ---  理由:参与归结的子句 本质上是由 文字决定的,因此不需要改变参与子句的优先级,只需要改变起步子句的优先级即可
        //        for_each(triAlg.setUsedCla.begin(), triAlg.setUsedCla.end(), [](Clause * cla) {
        //            --cla->priority;
        //        });
        //只修改起步子句的优先级


        //--- 选择起步子句 --- 
        --(selCla->priority);
        //itSelCla = fol->getNextGoalClause();
        itSelCla = fol->getNextStartClause();
        //        do{            
        //            if (( getNextGoalClause) == fol->getWorkClas()->end())
        //                itSelCla = fol->getWorkClas()->begin();
        //            selCla = *itSelCla;
        //        }while (selCla->isDel()) ;

        //先从特殊子句集开始

        //        if (!triAlg.vSpecialCla.empty()) {
        //            selCla = triAlg.vSpecialCla.back();
        //            triAlg.vSpecialCla.pop_back();
        //        } else if (isGoal) {
        //            ++iGoalInd;
        //            if (iGoalInd >= fol->goalClaset.size()) {
        //                isGoal = false;
        //                selCla = *itSelCla;
        //            } else
        //                selCla = fol->goalClaset[iGoalInd]; //
        //        } else {
        //            --(*itSelCla)->priority;
        //            ++itSelCla;
        //            if (itSelCla == fol->getWorkClas()->end())
        //                itSelCla = fol->getWorkClas()->begin();
        //            selCla = *itSelCla;
        //            while (selCla->isDel()) {
        //                ++itSelCla;
        //                if (itSelCla == fol->getWorkClas()->end())
        //                    itSelCla = fol->getWorkClas()->begin();
        //                selCla = *itSelCla;
        //            }
        //        }
    }


    return RESULT::UNKNOWN;
}

/// 扩展三角形
/*-----------------------------------------------------------------------
 *算法描述: 
 * 1. 选择起步子句. A.非单元子句; B.非等词公理子句S
 * 2. 完成三角形构建 选择子句，放入
 * 3. 重新选择起步子句
 * 
/*---------------------------------------------------------------------*/
/// \param fol
/// \return 

RESULT Resolution::BaseExtendAlg(Formula * fol) {
    //排序目标子句--文字由多到少.
    list<Clause*>* lsWorkClas = fol->getWorkClas();
    RESULT res = RESULT::UNKNOWN;
    TriAlgExt triAlgExt(fol);
    int iter = 0;
    while (true) {
        string soutInfo = "------扩展△次数:" + to_string(++iter) + "非单元集大小:" + to_string(fol->getWorkClas()->size()) + "------\n";
        FileOp::getInstance()->outRun(soutInfo);
        FileOp::getInstance()->outTriExt(soutInfo);

        //非单元子句+目标子句 排序
        fol->getWorkClas()->sort(SortRule::ClaCmp);

        //单元子句排序
        fol->unitClasSort();

        //拓展三角形
        res = triAlgExt.ExtendTri();

        //判断结果是否为UNSAT        
        if (RESULT::UNSAT == res) {
            break;
        }
    }
    return res;

}

/// 只对二元子句进行单元子句处理合一
/// \param fol
/// \return 
#define OverMaxLitNumOfCla 2
#define MaxModifyCount 100

RESULT Resolution::BaseAlgByLearn(Formula * fol) {
    
    RESULT res = RESULT::UNKNOWN;
    uint16_t modifyLimitCount = 0; //修改限制条件次数
    //不能作为起步子句集合--A.单元子句,B等词公理不能作为起步子句
    set<Clause*> notStartClaSet;
    //1.选择起步子句 -- 目标子句
    // vector<Clause*>::iterator itSelCla = fol->vgoalClas.begin();
    //vector<Clause*>::iterator itSelCla = fol->getNextGoalClause();
    int selClaInd = fol->GetNextGoalClauseIndex();
    TriAlg triAlg(fol);
    double startTimeResolute = CPUTime();
    uint32_t iterNum = 0;
    //int ind = distance(fol->vgoalClas.begin(), itSelCla);
    while (true) {
        if (StrategyParam::IterCount_LIMIT < ++iterNum) {
            res = RESULT::UNKNOWN;
            string strInfo = "# 到达最大演绎次数:" + to_string(StrategyParam::IterCount_LIMIT);
            FileOp::getInstance()->outLog(strInfo);

            cout << strInfo << endl;
            break;
        }
        //zj debug  cout << "# === 演绎次数:" << iterNum << endl;
        // if (iterNum == 1312)                        cout << "debug" << endl;
        Clause* selCla = fol->vgoalClas[selClaInd];

        //2.构建一次多元演绎
        //res = triAlg.GenBaseTriByLearn(selCla);
        res = triAlg.GenBaseTriByOneLearn(selCla);

        //3.判断结果
        /* 3.1 判定不可满足*/
        if (RESULT::UNSAT == res) {
            assert(triAlg.vNewR.empty());
            fprintf(stdout, "Start From Clause %u,proof found!\n", selCla->ident);
            break;
        }
        uint32_t uNewClaSize = triAlg.newClas.size();
        /* 3.2 子句起步没有构建任何三角形 */
        if (0 == uNewClaSize) {
            notStartClaSet.insert(fol->vgoalClas[selClaInd]);
            if (notStartClaSet.size() == fol->vgoalClas.size()) {
                if (++modifyLimitCount > MaxModifyCount) {/*记录修改次数*/
                    res = RESULT::UNKNOWN;
                    string strInfo = "# 到达最大限制修改次数:" + to_string(MaxModifyCount);
                    FileOp::getInstance()->outLog(strInfo);
                    cout << strInfo << endl;
                    break;
                } else { /*说明限制条件需要调整*/
                    if (StrategyParam::MaxLitNumOfR < (fol->uMaxLitNumOfCla + OverMaxLitNumOfCla)) {
                        ++StrategyParam::MaxLitNumOfR; //调整剩余文字数(R)的限制
                    }
                    ++StrategyParam::MaxFuncLayerOfR; //函数嵌套层限制
                    string strInfo = "[修改限制]最大文字数:" + to_string(StrategyParam::MaxLitNumOfR)
                            + " 最大函数嵌套层:" + to_string(StrategyParam::MaxFuncLayerOfR);
                    FileOp::getInstance()->outLog(strInfo);
                    FileOp::getInstance()->outRun("\n" + strInfo);
                    //cout << strInfo << endl;
                    notStartClaSet.clear();
                    //itSelCla = fol->getNextGoalClause(); //选择一个开始
                    //  selClaInd = 0;
                    selClaInd = fol->GetNextGoalClauseIndex();
                }
            } else {

                if (++selClaInd == fol->vgoalClas.size()) {
                    selClaInd = 0;
                }

                //                if (*itSelCla == nullptr || itSelCla == fol->vgoalClas.end())
                //                    itSelCla = fol->vgoalClas.begin();
            }
        }/* 3.3 多元演绎构建成功*/
        else {//if (RESULT::SUCCES == res) {
            triAlg.subst->Clear();
            //添加新子句到子句集.

            for (int i = 0; i < uNewClaSize; i++) {
                Clause* newCla = triAlg.newClas[i];

                //debug if (newCla->ident >= 16765) cout << "debug" << endl;
                triAlg.OutNewClaInfo(newCla); //输出新子句
                fol->insertNewCla(newCla);
                double pri = 0;
                for (Literal* tmpLit = newCla->literals; tmpLit; tmpLit = tmpLit->next) {
                    pri += tmpLit->parentLitPtr->claPtr->priority;
                }
                /*改变新子句的权重R的权重为文字权重的平均--遍历第一个△路径除外 取整*/
                //注意:由于目标子句初始化权重为100 因此 平均值后 若新子句中有目标子句参与自然权重会较高
                newCla->priority = pri / newCla->LitsNumber();
                fol->vgoalClas.push_back(newCla);
            }
            //选择下一个起步子句
            if (++selClaInd == fol->vgoalClas.size()) {
                selClaInd = 0;
            }
            //            (++itSelCla);
            //            if ((*itSelCla) == nullptr || itSelCla == fol->vgoalClas.end())
            //                itSelCla = fol->vgoalClas.begin();

        }
        

    }
    PaseTime("BaseAlgByLearn ", startTimeResolute); //输出演绎的时间
    return res;
}