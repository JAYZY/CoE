/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   TriAlgExt.cpp
 * Author: zj
 * 
 * Created on 2019年8月15日, 下午8:19
 */

#include "TriAlgExt.h"
#include "INOUT/FileOp.h"
#include "Inferences/Simplification.h"
#include "HEURISTICS/SortRule.h"
#include "Global/IncDefine.h"

TriAlgExt::TriAlgExt(Formula* _fol) : fol(_fol) {
    subst = new Subst();
}

TriAlgExt::TriAlgExt(const TriAlgExt& orig) {
}

TriAlgExt::~TriAlgExt() {
    DelPtr(subst);
}

void TriAlgExt::IniVect() {
    vMainTri.clear();
    vMainTri.reserve(16);
    vUsedClas.clear();
    vUsedClas.reserve(8);

    vNewClas.clear();
    vNewClas.reserve(8);
}



/*-----------------------------------------------------------------------
 *算法流程：
 * 1. 选择子句givenCla 
 * 2. 对子句进行单元子句归结
 * 3. 对子句文字进行主界线归结
 * 4. 对R进行判断，是否满足要求，生成R，输出对角线和R
 * 5. 选择子句放入三角形中
 * 对放入主界线文字的思考 
 * 1. 单元子句，默认均在主界线文字上。
 * 2. 单元子句的拷贝(带变元的)是否放入主界线？结论，由于拷贝已经是有一次替换作用的，因此，下一次下拉约减直接使用原始单元子句。 
 * 单元子句的拷贝不放入主界线，但需要输出。 存在 单元子句 对不同文字替换变为相同文字的情况 p(a)  p(a)等-- 只是输出格式不太规范
 * 3. 对主界线文字的构成 ：单元子句，合一的子句
 * 
/*---------------------------------------------------------------------*/
//

RESULT TriAlgExt::ExtendTri() {

    //根据策略,获取given-clause 子句
    list<Clause*>*lstCla = fol->getWorkClas();

    //zj:9.12 copy all workcla
    ClauseSet* procedClaSet = new ClauseSet();

    list<Clause*>::iterator itSelCla = lstCla->begin();
    bool isDeduct = false;
    //=== 初始化 ===
    IniVect();
    subst->Clear();
    RESULT resTri = RESULT::NOMGU;
    //=== 输出,起步子句信息====
    string strOut = "# 起步子句C" + to_string((*itSelCla)->ident) + ":";
    (*itSelCla)->getStrOfClause(strOut);
    FileOp::getInstance()->outRun(strOut);
    //=== 开始△ ===== 将所有子句 一个一个的放入△中

    vector<Literal*>vHoldLits;
    vector<Literal*>vNewR;

    int iIter = 0;
    //cout << "=============" << endl;
    /*--- 按照文字数 从少到多选择子句----*/
    //for (; itSelCla != lstCla->end(); ++itSelCla) {
    bool addTri = false; //所选子句是否加入△
    while (!fol->workClaSet->ClauseSetEmpty()) {
        itSelCla = fol->getNextStartClause();
        ++iIter;
        addTri = false;
        //if (iIter == 2675 && (*itSelCla)->ident == 1028) {            cout << "debug:" << endl;        }
        Clause* givenCla = *itSelCla;
        if (givenCla->isDel())
            continue;
        vNewR.clear();
        vHoldLits.clear();
        isDeduct = false;
        vUsedClas.clear();

        //==子句中文字排序=== (策略;文字的使用次数 和 发生冗余的次数会影响它的排序)
        givenCla->SortLits();
        //=== 设置所有文字均为hold属性
        givenCla->SetAllLitsHold();
        uint32_t iIniSubStSize = subst->Size(); //记录合一回退点


        /*--- 与单元子句归结[下拉] ----*/
        Literal* triLit = nullptr;
        resTri = UnitClaReduce(&triLit, givenCla, vNewR);
        if (RESULT::UNSAT == resTri) {
            vUsedClas.push_back(givenCla);
            goto UnsatOut;
        } else if (RESULT::SUCCES == resTri) {
            isDeduct = true;
        }

        if (triLit != nullptr) {
            /*--- 与主界线文字归结[下拉] ----*/
            resTri = MainTriReduce(&triLit, givenCla, vNewR);
            if (RESULT::UNSAT == resTri) {
                vUsedClas.push_back(givenCla);
                goto UnsatOut;
            } else if (RESULT::SUCCES == resTri) {
                isDeduct = true;
            } else if (RESULT::FAIL == resTri) {
                isDeduct = false;
            }
        }
        //--- 无论是 fail 还是 nomug 说明该子句不应该加入 △；
        if (isDeduct) {
            if (triLit != nullptr) {
                for (Literal* lit = givenCla->literals; lit; lit = lit->next) {
                    if (lit->EqnQueryProp(EqnProp::EPIsHold)) {
                        vNewR.push_back(lit);
                    }

                }
            }
            assert(!vNewR.empty());
            /*--- 判断能否生成新子句加入S----*/
            if (vNewR.size() > StrategyParam::MaxLitNumOfNewCla) {
                isDeduct = false;
            } else {
                //--- 判断&生成新子句         
                Clause* newCla = new Clause();

                newCla->bindingAndRecopyLits(vNewR);
                if (newCla->minFuncLayer > StrategyParam::MaxFuncLayerOfR) {
                    DelPtr(newCla);
                    isDeduct = false;
                } else {
                    //--- 做Factor Condense Rule ---                
                    Clause* factorCla = Simplification::Factor(newCla);

                    if (factorCla) {
                        //检查是否可以将factorCla 加入到新子句集中
                        if (Add2NewClas(factorCla, InfereType::SCS)) {
                            //--- 输出factor .i 信息 --- 
                            OutNewClaInfo(factorCla, InfereType::FACTOR);
                            //边做加入子句     
                            // fol->insertNewCla(factorCla);
                        }
                        //--- 删除原始子句 ---
                        DelPtr(newCla);
                        //cout << "CNMMP " << endl;

                    } else {
                        //检查是否可以将newCla加入到新子句集中
                        if (Add2NewClas(newCla, InfereType::SCS)) {

                            vUsedClas.push_back(givenCla);
                            for (Clause*cla : vUsedClas) {
                                newCla->parentIds.insert(cla->ident);
                            }
                            //--- 输出.r 信息 --- 
                            OutTriAndR(vNewR);
                            //--- 输出.i 信息 --- 
                            OutNewClaInfo(newCla, InfereType::SCS);
                            //边做加入子句     
                            //fol->insertNewCla(newCla);
                        }
                    }
                    //fol->insertNewCla(newCla);
                    if ((triLit != nullptr) && vNewR.size() < StrategyParam::MaxLitNumOfR) { //超过限制，不放入主界线
                        //--- 目前放入主界线文字的策略：按照子句中文字顺序第一个保留的文字放入主界线
                        assert(triLit);
                        triLit->EqnDelProp(EqnProp::EPIsHold);
                        ++triLit->aUsedCount;
                        vMainTri.push_back(triLit);
                        addTri = true;
                    } else {
                        isDeduct = false;
                    }
                }
            }

        } else if (vMainTri.empty()) { //匹配失败的子句也加入主界线
            assert(triLit);
            triLit->EqnDelProp(EqnProp::EPIsHold);
            ++triLit->aUsedCount;
            vMainTri.push_back(triLit);
            addTri = true;

        }
        if (addTri) {
            string sTriout = "[CID:" + to_string(Env::global_clause_counter) + "] ";
            Literal* litTri = vMainTri.back();
            litTri->GetLitInfoWithSelf(sTriout);
            FileOp::getInstance()->outTriExt(sTriout + "\n");
            //对主界线文字排序
            sort(vMainTri.begin(), vMainTri.end(), SortRule::LitCmp);
        } else {
            givenCla->priority -= 50;
        }
        fol->workClaSet->RemoveClause(givenCla);
        procedClaSet->InsertCla(givenCla,false);

        // cout<<lstCla.size()<<endl;
        if (fol->workClaSet->ClauseSetEmpty())
            break;
    }
    fol->workClaSet = procedClaSet;
    //--- 完成一轮△，输出新生成的子句 ---
    for (int i = 0; i < vNewClas.size(); ++i) {
        //=== 修改新子句权重-------------
        int pri = 0;
        Literal* tmpLit = vNewClas[i]->literals;
        for (; tmpLit; tmpLit = tmpLit->next) {
            pri += tmpLit->parentLitPtr->claPtr->priority;
        }
        /*改变新子句的权重R的权重为文字权重的平均--遍历第一个△路径除外 取整*/
        //注意:由于目标子句初始化权重为100 因此 平均值后 若新子句中有目标子句参与自然权重会较高
        vNewClas[i]->priority = pri / (int) (vNewClas[i]->LitsNumber());
        fol->InsertNewCla(vNewClas[i]);

    }
    //--- 删除拷贝的单元子句
    for (int i = 0; i < delUnitCla.size(); ++i) {
        DelPtr(delUnitCla[i]);
    }
    delUnitCla.clear();
    return resTri;

UnsatOut:
    assert(RESULT::UNSAT == resTri);
    //=== 删除 .r 信息
    OutTriAndR(vNewR);
    //=== 输出 .i 信息
    OutUnsatInfo(InfereType::SCS);
    return resTri;
}



/// 扩展△算法流程：
/**
 * 完成一次△构建 
 * 1. 选择1个子句  （策略：优先选择 文字数少的，稳定度低的）
    1.1 选择该子句中文字（策略， 优先选择 正文字）
 *  A.与单元子句归结; B.与主界线文字归结
 *  1.2 再选择下一个文字
 */
///
/// \return 

RESULT TriAlgExt::ExtendTriByFull() {
    list<Clause*>*lstCla = fol->getWorkClas();
    ClauseSet* procedClaSet = new ClauseSet();
    list<Clause*>::iterator itSelCla = lstCla->begin();
    //vector<Literal*>vNewR;
    bool isDeduct = false;
    //初始化===
    IniVect();
    subst->Clear();
    RESULT resTri = RESULT::NOMGU;

    //=== 输出,起步子句信息====
    string strOut = "# 起步子句C" + to_string((*itSelCla)->ident) + ":";
    (*itSelCla)->getStrOfClause(strOut);
    FileOp::getInstance()->outRun(strOut);

    //=== 开始△ ===== 将所有子句 一个一个的放入△中
    while (!fol->workClaSet->ClauseSetEmpty()) {
        //选择最优子句
        itSelCla = fol->getNextStartClause();

        Clause* givenCla = *itSelCla;
        givenCla->SortLits();
        if (givenCla->isDel())
            continue;
        FileOp::getInstance()->outRun("#------插入子句：" + to_string(givenCla->ident) + "\n");
        isDeduct = false;
        vUsedClas.clear(); //清除被使用的子句集
        int pos = subst->Size();
        RESULT res = DeduceOnceByFullPath(givenCla);

        if (RESULT::UNSAT == res) {
            resTri = RESULT::UNSAT;
            break;
        } else if (RESULT::FAIL == res || RESULT::NOLits == res) {
            //处理方法2种：A.生成新子句，并将该子句造成的替换还原，继续△; B. △停止,生成新子句
            subst->SubstBacktrackToPos(pos);
        }

        fol->workClaSet->RemoveClause(givenCla);
        procedClaSet->InsertCla(givenCla,false);
    }
    //=== △构建完成 ===== 
    fol->workClaSet = procedClaSet;
    //procedClaSet->FreeAllClas();
    //--- 完成一轮△，输出新生成的子句 ---
    for (int i = 0; i < vNewClas.size(); ++i) {
        //=== 修改新子句权重-------------
        int pri = 0;
        Literal* tmpLit = vNewClas[i]->literals;
        for (; tmpLit; tmpLit = tmpLit->next) {
            pri += tmpLit->parentLitPtr->claPtr->priority;
        }
        /*改变新子句的权重R的权重为文字权重的平均--遍历第一个△路径除外 取整*/
        //注意:由于目标子句初始化权重为100 因此 平均值后 若新子句中有目标子句参与自然权重会较高
        vNewClas[i]->priority = pri / (int) (vNewClas[i]->LitsNumber());
        fol->InsertNewCla(vNewClas[i]);
    }

    //--- 删除拷贝的单元子句
    for (int i = 0; i < delUnitCla.size(); ++i) {
        DelPtr(delUnitCla[i]);
    }
    delUnitCla.clear();

    return resTri;


}

/// 单元子句对actCla中的文字进行下拉操作。
/// 前提： 子句中的文字已经排好序
/// \param actCla
/// \return   NOMGU - 没有下拉； SUCCES - 有下拉文字, UNSAT - 

RESULT TriAlgExt::UnitClaReduce(Literal **actLit, Clause * actCla, vector<Literal*> &vNewR) {

    RESULT resReduce = RESULT::NOMGU;
    uint16_t uHoldLitNum = 0;
    //剩余文字列表
    Literal * arrHoldLits[actCla->LitsNumber() - 1 + vNewR.size()];
    memcpy(&arrHoldLits, &vNewR[0], vNewR.size() * sizeof (Literal*));
    (*actLit) = nullptr;

    vector<Literal*>vDelActLit;
    vDelActLit.reserve(4);

    //== 遍历子句中的所有文字，
    for (Literal* actLitPtr = actCla->literals; actLitPtr; actLitPtr = actLitPtr->next) {

        if (!actLitPtr->EqnQueryProp(EqnProp::EPIsHold)) {
            continue;
        }

        //vector<Clause* >&cmpUnitClas = actLitPtr->IsPositive() ? fol->vNegUnitClas :vCandUnitClas fol->vPosUnitClas; //可以考虑优化 用索引树        
        //--- 获取候选被动文字 --- 
        vector<Literal*>&vCandUnitClas = *fol->getPairUnitPredLst(actLitPtr);
        bool isUnified = false; //是否被下拉
        //=== 遍历单元子句 ========================
        for (int ind = 0; ind < vCandUnitClas.size(); ++ind) {
            Lit_p candLit = vCandUnitClas[ind];

            Clause* candUnitCal = candLit->claPtr;
            if (candUnitCal->isDel()) { //被删除的单元子句不处理 [待优化]              
                continue;
            }
            //== 若GivenLitP不是等词，则确保lterm.fcode相等（优化项）
            if (!actLitPtr->EqnIsEquLit() && actLitPtr->lterm->fCode != candLit->lterm->fCode)
                continue;

            //== 当前单元子句没有经过任何替换,因此判断单元子句是否为基项,不需要重新计算
            bool isRN = !candLit->IsGround(false);

            //== 单元子句重用 -- 对非基文字拷贝生成新的单元子句，放入临时单元子句列表,不加入子句集. 完成三角形后 该临时单元子句被删除.
            if (isRN) {
                candUnitCal = new Clause();
                candLit = candLit->RenameCopy(candUnitCal, DerefType::DEREF_NEVER); //单元子句拷贝
            }
            assert(actLitPtr->isComplementProps(candLit));
            uint32_t bpBeforeUnify = subst->Size();
            bool res = unify.literalMgu(actLitPtr, candLit, subst);

            //=== 合一成功 ===

            //=== 规则检查！ PS算法：1.若有变元替换发生检查 检查剩余文字之间是否由于替换导致相同/恒真，
            //                    2.Forward Subsumed 放到最后检查                                
            if (res) {//&& bpBeforeUnify != subst->Size()) {
                int16_t iHoldLitIdx = vNewR.size();

                //=== 若有变元替换发生 - 检查-[恒真&相同文字]
                vDelActLit.clear();
                for (Literal* checkLitA = actCla->literals; checkLitA; checkLitA = checkLitA->next) {
                    if (checkLitA == actLitPtr || !checkLitA->EqnQueryProp(EqnProp::EPIsHold))
                        continue;
                    bool isHold = true;

                    if (bpBeforeUnify != subst->Size()) { //只有发生了变元替换才进行规则检查
                        if (!vNewR.empty()) {
                            ResRule resRule = CheckNewR(checkLitA, vNewR);
                            if (ResRule::TAUTOLOGY == resRule) {
                                res = false;
                                break;
                            } else if (ResRule::DelHoldLit == resRule) {
                                vDelActLit.push_back(checkLitA);
                                continue;
                            }
                        }
                        for (Literal* checkLitB = checkLitA->next; checkLitB; checkLitB = checkLitB->next) {

                            //检查是否恒真&相同
                            if (checkLitA->EqualsStuct(checkLitB)) {
                                if (checkLitA->isSameProps(checkLitB)) { //相同文字，删除
                                    isHold = false;
                                    vDelActLit.push_back(checkLitA); //checkLitA->EqnDelProp(EqnProp::EPIsHold);                                    
                                } else { //互补 则为恒真删除 ->换下一个单元子句检查下拉
                                    res = false;
                                }
                                break;
                            }
                        }
                        if (!res) {
                            break;
                        }
                    }
                    if (isHold) {
                        arrHoldLits[iHoldLitIdx++] = checkLitA;
                    }
                }

                //=== 检查-[FS 归入冗余]                    
                if (res && fol->HoldLitsIsRundacy(arrHoldLits, iHoldLitIdx, nullptr, nullptr)) {
                    res = false; //子句冗余
                    //9.8add  发生冗余对 单元子句 优先级-1
                    vCandUnitClas[ind]->claPtr->priority -= 1;
                }
                if (res) {
                    //开始标注主动子句中被删除的剩余文字                   
                    for (Literal* delLit : vDelActLit) {
                        delLit->EqnDelProp(EqnProp::EPIsHold);
                    }

                }
            }
            //--- Rule check End

            if (res) {
                actLitPtr->EqnDelProp(EqnProp::EPIsHold);
                //=== 输出变名的单元子句：1.重命名的；2没有输出过 THEN 输出到结果文件
                if (isRN) {

                    candUnitCal->bindingLits(candLit); //生成新单元子句                    
                    Clause* tmpOriCandCla = vCandUnitClas[ind]->claPtr;
                    candUnitCal->parentIds.insert(tmpOriCandCla->ident);
                    //--- 输出新子句到info文件
                    OutNewClaInfo(candUnitCal, InfereType::RN);
                    //--- 输出到.r文件
                    string str = "\n";
                    tmpOriCandCla->literals->GetLitInfoWithSelf(str);
                    str += "\nR[" + to_string(candUnitCal->ident) + "]:";
                    candLit->GetLitInfoWithParent(str);
                    FileOp::getInstance()->outRun(str + "\n");


                    //--- 添加该单元子句副本到删除列表中,完成三角形后删除.
                    delUnitCla.push_back(candUnitCal);
                    //--- 变名后的单元子句都不添加到主界线

                }

                candUnitCal->literals->EqnDelProp(EqnProp::EPIsHold);
                actLitPtr->matchLitPtr = candUnitCal->literals; //记录匹配的文字用于输出
                vUsedClas.push_back(candUnitCal); //记录使用过的单元子句；
                //---记录成功匹配单元子句的位置信息。 索引列表类型；索引列表中的位置,后续处理                
                Literal* tmpUnitLit = vCandUnitClas[ind];
                //单元子句使用次数-1, 降低优先级
                tmpUnitLit->claPtr->priority -= 1;
                //移动匹配成功的 单元子句 -- {两种算法:A.没用一次移到最后,下次使用不用排序;B.每次使用记录weight,下次使用排序.目前使用A算法)
                vCandUnitClas.erase(vCandUnitClas.begin() + ind);
                vCandUnitClas.push_back(tmpUnitLit);
                //actLitP->IsPositive() ? sMoveNegUnitCla.insert(ind) : sMovePosUnitCla.insert(ind);
                //=== 匹配成功换下一个剩余文字   
                isUnified = true;
                resReduce = RESULT::SUCCES;
                break;
            } else {
                if (isRN) {
                    DelPtr(candLit); //删除copy literal
                    DelPtr(candUnitCal); //删除空子句
                    --Env::global_clause_counter;
                }
                subst->SubstBacktrackToPos(bpBeforeUnify);
            }
        }
        //=== 该文字没有找到下拉单元子句 == 存在下拉替换导致原有保留文字被删除，因此最后来添加剩余文字
        //        if (actLitPtr->EqnQueryProp(EqnProp::EPIsHold)) {
        //            vHoldLits.push_back(actLitPtr);
        //        }
        if (!isUnified) {
            assert(actLitPtr->EqnQueryProp(EqnProp::EPIsHold));
            ++uHoldLitNum;
            if (nullptr == (*actLit))
                (*actLit) = actLitPtr;
        }
    }

    /*UNSAT*/
    if (0 == (uHoldLitNum + vNewR.size())) {
        assert(nullptr == (*actLit));
        resReduce = RESULT::UNSAT;
    }
    return resReduce;
}

/// 给定一个子句actCla,对其进行约减1.单元 2.主界线
/// \param actCla
/// \return NOMGU -- No Deduct；UnSat; FAIL -- 子句不放入△（文字，函数层等违反限制条件）
//思考：延拓△是否有最优子句概念的？

RESULT TriAlgExt::DeduceOnceByFullPath(Clause * actCla) {
    vector<Literal*> vNewR;
    vNewR.reserve(StrategyParam::MaxLitNumOfR);
    RESULT resReduce = RESULT::NOMGU;
    //回退信息记录------------------------------------------    
    vector<RBPoint_p> vRollBackPoint; //记录回退点
    vector<Literal*> vDelActLit;
    vDelActLit.reserve(8);
    actCla->SetAllLitsHold();

    Literal* actLitPtr = actCla->literals;
    bool isChFirstLit = false;
    bool isReduce = false;

    uint32_t uInd = actLitPtr->uUnitMatchInd; //单元子句 匹配起始位置
    //=== 遍历子句中的所有文字
    for (; actLitPtr; actLitPtr = actLitPtr->next) {
        if (!actLitPtr->EqnQueryProp(EqnProp::EPIsHold)) {
            continue;
        }
        //=== 单文字下拉 ===
        isReduce = UCTriRed(actLitPtr, vNewR, vDelActLit, vRollBackPoint, uInd);
        //若为第一个文字，则记录它匹配单元子句的下标
        //        if (actLitPtr == actCla->literals) {
        //            if (isReduce) {
        //                actCla->uFirstURInd = uInd ;
        //            } else { //第一个文字找不到匹配
        //                actCla->uFirstURInd = uInd;
        //            }
        //        }
        uInd = 0;
        if (isReduce)
            continue;
        //=== 主界线文字下拉 === 
        isReduce = MainTriRed(actLitPtr, vDelActLit, vNewR);
        if (actLitPtr == actCla->literals && !isReduce) {
            isChFirstLit = true;
        }

    }

    //=== 完成一轮下拉，准备回退开始下一轮下拉

    if (vDelActLit.size() == 0) {
        //表示该子句无文字被下拉，不能放入△中
        return RESULT::FAIL;
    }
    /*算法逻辑：1. 判断是否为UNSAT - 退出;2.判断是否全部下拉 - 退出; 3.判断是否是最优路径-保存；4.判断是否回退路径*/
    uint16_t uActHoldNum = actCla->LitsNumber() - vDelActLit.size();
    uint16_t uHoldLitNum = uActHoldNum + vNewR.size();
    resReduce = RESULT::SUCCES;

    vUsedClas.push_back(actCla);
    /*UNSAT*/
    if (0 == uHoldLitNum) {
        //=== 输出 .r 信息
        OutTriAndR(vNewR);
        //=== 输出 .i 信息
        OutUnsatInfo(InfereType::SCS);
        return RESULT::UNSAT;
    }/*是否可以放入△*/
    else if (uHoldLitNum > StrategyParam::MaxLitNumOfNewCla) {
        return RESULT::FAIL;
    }/*是否全部下拉 -- 意味则该三角形结束*/
    else if (0 == uActHoldNum) {
        resReduce = RESULT::NOLits;
        //处理方法2种：A.生成新子句，并将该子句造成的替换还原，继续△; B. △停止,生成新子句
        //*** 生成新子句
    }




    //--- 判断是否可以生成中间过程的新子句         
    if (uHoldLitNum <= StrategyParam::MaxLitsNumOfTriNewCla) {
        assert((RESULT::SUCCES == resReduce || RESULT::NOLits == resReduce));
        //*** 生成新子句
        vector<Literal*>vHoldLits;
        actCla->GetVecHoldLit(vHoldLits);
        if (!vNewR.empty())
            vHoldLits.insert(vHoldLits.end(), vNewR.begin(), vNewR.end());
        Clause* newCla = new Clause();
        newCla->bindingAndRecopyLits(vHoldLits);

        //*** 检查新子句函数层限制
        if (newCla->MinFuncLayer() > StrategyParam::MaxFuncLayerOfR) {
            DelPtr(newCla);
        } else {
            //*** 做Factor Condense Rule ---                
            Factor2NewCla(newCla, vHoldLits);
        }
    }
    /*如果有下拉且满足条件(子句中剩余文字长度)则放入主界线*/
    if (RESULT::SUCCES == resReduce) {
        vUsedClas.push_back(actCla);
        Literal* firstHoldLit = actCla->GetFirstHoldLit(); //将第一个留下的文字作为△文字
        firstHoldLit->SetNoHold();
        vMainTri.push_back(firstHoldLit);
    }
    //    if (isChFirstLit) {
    //        actCla->PutFirstLitToLast();
    //    }
    return resReduce;
}

/// 
/// \param aLitPtr          需要约减的文字
/// \param vNewR            剩余文字集
/// \param vDelActLit       删除的文字列表
/// \param vRollBackPoint   回退集合
/// \param uInd             单元子句起始下标
/// \return 

bool TriAlgExt::UCTriRed(Literal* aLitPtr, vector<Literal*> &vNewR, vector<Literal*> &vDelActLit, vector<RBPoint_p> &vRollBackPoint, uint32_t &uInd) {
    bool res = false;
    //--- 获取候选被动文字 --- 
    vector<Literal*>&vCandUnitClas = *fol->getPairUnitPredLst(aLitPtr);
    bool isUnified = false; //是否被下拉
    //=== 遍历单元子句 ========================
    for (uInd; uInd < vCandUnitClas.size(); ++uInd) {
        Lit_p candULit = vCandUnitClas[uInd];
        Clause* candUnitCal = candULit->claPtr;
        if (candUnitCal->isDel()) { //被删除的单元子句不处理 [待优化]              
            continue;
        }
        assert(!aLitPtr->EqnIsEquLit() && aLitPtr->lterm->fCode == candULit->lterm->fCode);
        assert(aLitPtr->isComplementProps(candULit));

        //-- 当前单元子句没有经过任何替换,因此判断单元子句是否为基项,不需要重新计算
        bool isRN = !candULit->IsGround(false);

        //-- 单元子句重用 -- 对非基文字拷贝生成新的单元子句，放入临时单元子句列表,不加入子句集. 完成三角形后 该临时单元子句被删除.
        if (isRN) {
            candUnitCal = new Clause();
            candULit = candULit->RenameCopy(candUnitCal, DerefType::DEREF_NEVER); //单元子句拷贝
        }

        uint32_t uSustSizeBU = subst->Size();
        uint32_t uDelLitNumBU = vDelActLit.size();
        //=== 合一 =========       
        res = unify.literalMgu(aLitPtr, candULit, subst);

        //=== 规则检查-[恒真&相同文字] 确保中间演绎不是冗余的   1.检查剩余文字之间是否由于替换导致相同/恒真，2.Forward Subsumed
        if (res && uSustSizeBU != subst->Size()) { //只有发生了变元替换才进行规则检查

            ResRule resRule = CheckUnitRule(aLitPtr, vDelActLit, vNewR);
            if (ResRule::RULEOK == resRule) {
            } else {
                res = false;
                vCandUnitClas[uInd]->claPtr->priority -= 1;
                aLitPtr->aUsedCount += StrategyParam::LitRedunancyWeight;
            }
        }
        //--- Rule check End

        if (res) {
            //--- 输出变名的单元子句：1.重命名的；2没有输出过 THEN 输出到结果文件
            if (isRN) {
                candUnitCal->bindingLits(candULit); //生成新单元子句      
                candUnitCal->literals->matchLitPtr = vCandUnitClas[uInd];
                candUnitCal->parentIds.insert(aLitPtr->claPtr->ident);
                //--- 输出新子句到info文件
                OutNewClaInfo(candUnitCal, InfereType::RN);
                //--- 输出到.r文件
                OutRNUnitCla(candUnitCal);
                //--- 添加该单元子句副本到删除列表中,完成三角形后删除.
                delUnitCla.push_back(candUnitCal);
                //--- 变名后的单元子句都不添加到主界线
            }
            ++aLitPtr->aUsedCount;
            //            //--- 如果是共享变元则需要充分使用--添加回退点
            //            if (aLitPtr->getVarState() == VarState::shareVar) {
            //                RollBackPoint* rbP = new RollBackPoint();
            //                rbP->litP = aLitPtr; //回退检查是否能被下拉的文字
            //                rbP->uMatchPos = uInd + 1; //下次检查的单元文字位置
            //                rbP->uSubstSize = uSustSizeBU; //变元替换位置
            //rbP->uVecLitsPos = uDelLitNumBU; //删除文字列表位置
            ////添加回退点
            //vRollBackPoint.push_back(rbP);
            //            }

            //--- 设置相关属性
            candUnitCal->literals->EqnDelProp(EqnProp::EPIsHold);
            aLitPtr->EqnDelProp(EqnProp::EPIsHold);
            aLitPtr->matchLitPtr = candUnitCal->literals; //记录匹配的文字用于输出
            vDelActLit.push_back(aLitPtr);
            vUsedClas.push_back(candUnitCal); //记录使用过的单元子句                  

            //单元子句使用次数-1, 降低优先级
            Literal* tmpUnitLit = vCandUnitClas[uInd];
            tmpUnitLit->claPtr->priority -= 1;
            //移动匹配成功的 单元子句 -- {两种算法:A.没用一次移到最后,下次使用不用排序;B.每次使用记录weight,下次使用排序.目前使用A算法)
            vCandUnitClas.erase(vCandUnitClas.begin() + uInd);
            vCandUnitClas.push_back(tmpUnitLit);

            isUnified = true;
            break;
        } else {
            if (isRN) {
                DelPtr(candULit); //删除copy literal
                DelPtr(candUnitCal); //删除空子句
                --Env::global_clause_counter;
            }
            subst->SubstBacktrackToPos(uSustSizeBU);
        }
    }
    return res;
}

/// 
/// \param aLitPtr          需要约减的文字
/// \param vDelActLit       被删除的文字
/// \param vNewR            剩余的文字集
/// \return true -- 被约减   false -- 没有约减

bool TriAlgExt::MainTriRed(Literal* aLitPtr, vector<Literal*>&vDelActLit, vector<Literal*>&vNewR) {
    bool resReduce = false;
    for (int i = 0; i < vMainTri.size(); ++i) {
        Literal* mTriLitPtr = vMainTri[i];
        //--- 确保互补 --- 
        if (aLitPtr->isSameProps(mTriLitPtr) || (!aLitPtr->EqnIsEquLit() && aLitPtr->lterm->fCode != mTriLitPtr->lterm->fCode))
            continue;
        //---[9.13] 确保 同一主界线文字 不下拉多个文字        if (setUnifiedLit.find(mTriLitPtr) != setUnifiedLit.end())            continue;
        uint32_t iUsedClaSize = vUsedClas.size(); //使用过的子句
        uint32_t uBPBeforeUnify = subst->Size(); //合一回退点        
        bool res = unify.literalMgu(aLitPtr, mTriLitPtr, subst);
        if (res) {
            res = false;
            //=== 规则检查 === 1.剩余文字是否是 恒真的 ；2.剩余文字是否是冗余的
            ResRule resRule = CheckAndFindUsed(mTriLitPtr->claPtr, vNewR); //RuleOk; TAUTOLOGY；【MoreLit--说明这个子句加入到已有的△路径超过文字数限定，换一个子句中的文字下拉】。
            //--- 检查主动子句剩余文字规则.A.剩余文字添加到vNewR恒真/相同(删除） B.与下拉文字[合一]相同,当alitPtr为基项
            if (ResRule::RULEOK == resRule) {
                //resRule = RuleCheck(aLitPtr, vNewR); //RULEOK，[TAUTOLOGY--表示合一失败换一个合一文字]，
                ResRule resRule = CheckUnitRule(aLitPtr, vDelActLit, vNewR);
                if (ResRule::RULEOK == resRule) {
                    res = true;
                    aLitPtr->matchLitPtr = mTriLitPtr;
                    aLitPtr->EqnDelProp(EqnProp::EPIsHold);
                    vDelActLit.push_back(aLitPtr); //被约减文字添加到删除文字集中
                    break;
                }
            }
        }
        assert(!res);
        while (vUsedClas.size() > iUsedClaSize) {
            vUsedClas.pop_back(); //还原使用过的子句集
        }
        //=== 下拉失败，还原 ===
        subst->SubstBacktrackToPos(uBPBeforeUnify);
    }
    return resReduce;
}

/// 
/// \param actCla
/// \param vHoldLits
/// \param vNewR
/// \return  NOMGU -- actCla没有发生任何下拉， SUCCES -- 成功下拉文字 ， FAIL--下拉失败(可能冗余，可能文字数操过限制)

RESULT TriAlgExt::MainTriReduce(Literal **actLit, Clause* actCla, vector<Literal*> &vNewR) {
    RESULT resReduce = RESULT::NOMGU;
    //剩余文字列表 长度为 MaxLitOfNewR+现有文字长度
    (*actLit) = nullptr;
    Literal * arrHoldLits[actCla->LitsNumber() + StrategyParam::MaxLitNumOfNewCla];
    set<Literal* > setUnifiedLit;
    if (!vMainTri.empty()) {
        //uint8_t vDelLits[vHoldLits.size()];
        //=== 遍历子句中的所有文字 ========================
        for (Literal* alitPtr = actCla->literals; alitPtr; alitPtr = alitPtr->next) {

            if (!alitPtr->EqnQueryProp(EqnProp::EPIsHold))
                continue;
            //=== 遍历主届线文字PS:这里有个问题，不同文字优先被同一个主界线文字下拉？ ========================
            //------ [9.13]解决方案 不允许同一个主界线文字同时下拉同一子句中的不同文字
            //for (int i=vMainTri.size()-1;i>-1;--i){//Literal* mTriLitPtr : vMainTri) { //优先尝试 新加入主界线的文字
            for (int i = 0; i < vMainTri.size(); ++i) {

                Literal* mTriLitPtr = vMainTri[i];
                //--- 确保互补 --- 
                if (alitPtr->isSameProps(mTriLitPtr) || (!alitPtr->EqnIsEquLit() && alitPtr->lterm->fCode != mTriLitPtr->lterm->fCode))
                    continue;
                //---[9.13] 确保 同一主界线文字 不下拉多个文字
                if (setUnifiedLit.find(mTriLitPtr) != setUnifiedLit.end())
                    continue;

                uint32_t uBPBeforeUnify = subst->Size(); //合一回退点
                int iUsedClaSize = vUsedClas.size(); //使用过的子句
                bool res = unify.literalMgu(alitPtr, mTriLitPtr, subst);

                if (res) {
                    //=== 规则检查 === 1.剩余文字是否是 恒真的 ；2.剩余文字是否是冗余的

                    //--- 检查被动子句规则；A.回溯所有相关子句，剩余文字添加到vNewR。B.检查vNewR是否恒真/相同(删除)
                    ResRule resRule = CheckAndFindUsed(mTriLitPtr->claPtr, vNewR); //RuleOk; TAUTOLOGY；【MoreLit--说明这个子句加入到已有的△路径超过文字数限定，换一个子句中的文字下拉】。

                    //--- 检查主动子句剩余文字规则.A.剩余文字添加到vNewR恒真/相同(删除） B.与下拉文字[合一]相同,当alitPtr为基项
                    if (ResRule::RULEOK == resRule) {
                        resRule = RuleCheck(alitPtr, vNewR); //RULEOK，[TAUTOLOGY--表示合一失败换一个合一文字]，
                    }
                    if (ResRule::RULEOK == resRule) {
                        // vUsedClas.push_back(mTriLitPtr->claPtr); //添加使用过的子句
                        alitPtr->matchLitPtr = mTriLitPtr;
                        alitPtr->EqnDelProp(EqnProp::EPIsHold);
                        resReduce = RESULT::SUCCES; //表示有下拉发生
                        setUnifiedLit.insert(mTriLitPtr);
                        break;
                    } else {
                        res = false;
                    }
                }
                assert(!res);
                while (vUsedClas.size() > iUsedClaSize) {
                    vUsedClas.pop_back(); //还原使用过的子句集
                }
                //=== 下拉失败，还原 ===
                subst->SubstBacktrackToPos(uBPBeforeUnify);
            }
        }

        //=== 有下拉发生， 生成新子句
        if (RESULT::SUCCES == resReduce) {
            //=== 对剩余文字进行处理 ===[检查FS，文字个数]
            //--- 拷贝vNewR到arrHoldLits
            memcpy(&arrHoldLits, &vNewR[0], vNewR.size() * sizeof (Literal*));
            int iStartInd = vNewR.size();

            for (Literal* holdLit = actCla->literals; holdLit; holdLit = holdLit->next) {
                if (!holdLit->EqnQueryProp(EqnProp::EPIsHold))
                    continue;

                arrHoldLits[iStartInd++] = holdLit;
                if ((*actLit) == nullptr)
                    (*actLit) = holdLit;
                //vNewR.push_back(holdLit);

                //--- 剩余文字数超过新子句的最大限制，则匹配失败，该子句不放入△中-- zj9.12 修改idea: 判断文字数放到 factor之后
                //                if (iStartInd > StrategyParam::MaxLitNumOfNewCla) {
                //                    resReduce = RESULT::FAIL; //文字数超过最大限制
                //                    break;
                //                }
            }

            if (RESULT::SUCCES == resReduce) {
                //=== UNSAT判断 ===
                if (0 == iStartInd) {
                    assert(vNewR.empty());
                    resReduce = RESULT::UNSAT;
                }//--- 检查剩余文字 是否是冗余的(FS:向前归入冗余)                            
                else {
                    set<Clause*>setTmpUsedCla;
                    for (int i = 0; i < vUsedClas.size(); ++i) {
                        setTmpUsedCla.insert(vUsedClas[i]);
                    }
                    if (fol->HoldLitsIsRundacy(arrHoldLits, iStartInd, &setTmpUsedCla, actCla)) {
                        //return ResRule::RSubsump; 
                        resReduce = RESULT::FAIL; //子句冗余

                        actCla->priority -= StrategyParam::ClaRedundancyWeight;
                    } else {

                    }
                }
            }
        }
    }

    if (RESULT::NOMGU == resReduce) {
        for (Literal* lit = actCla->literals; lit; lit = lit->next) {
            if (lit->EqnQueryProp(EqnProp::EPIsHold)) {
                (*actLit) = lit;
                break;
            }
        }
    }
    setUnifiedLit.clear();
    return resReduce;
}

bool TriAlgExt::Factor2NewCla(Clause* newCla, vector<Literal*>& vNewR) {
    bool isAdd = false;
    Clause* factorCla = Simplification::Factor(newCla);
    if (factorCla) {
        //检查是否可以将factorCla 加入到新子句集中
        if (Add2NewClas(factorCla, InfereType::SCS)) {
            //--- 输出factor .i 信息 --- 
            OutNewClaInfo(factorCla, InfereType::FACTOR);
        }
        //--- 删除原始子句 ---
        DelPtr(newCla);
        isAdd = true;
    } else {
        DelPtr(factorCla);
        //检查是否可以将newCla加入到新子句集中
        OutTriAndR(vNewR);
        if (Add2NewClas(newCla, InfereType::SCS)) {
            //vUsedClas.push_back(newCla);
            for (Clause*cla : vUsedClas) {
                newCla->parentIds.insert(cla->ident);
            }
            //--- 输出.r 信息 --- 
            //OutTriAndR(vNewR);
            //--- 输出.i 信息 --- 
            OutNewClaInfo(newCla, InfereType::SCS);
            isAdd = true;
            //边做加入子句     
        }
    }
    return isAdd;
}

bool TriAlgExt::Add2NewClas(Clause* newClaA, InfereType infereType) {

    bool isAddNewCla = true;
    vector<int>delNCla;
    for (int i = 0; i < vNewClas.size(); ++i) {
        Clause* nCla = vNewClas[i];
        if (Simplification::ClauseSubsumeClause(newClaA, nCla)) {
            FileOp::getInstance()->outLog("[N_FS]:del C" + to_string(newClaA->ident) + " by C" + to_string(nCla->ident) + "\n");
            --Env::global_clause_counter;
            DelPtr(newClaA);
            isAddNewCla = false;
            break;
        }
        if (Simplification::ClauseSubsumeClause(nCla, newClaA)) {
            delNCla.push_back(i);
            break;
        }
    }
    for (int i = delNCla.size() - 1; i<-1; --i) {
        int idx = delNCla[i];
        FileOp::getInstance()->outLog("[N_BS]:del C" + to_string(vNewClas[idx]->ident) + " by C" + to_string(newClaA->ident) + "\n");
        std::swap(vNewClas[idx], vNewClas.back());
        DelPtr(vNewClas.back());
        vNewClas.pop_back();
    }
    if (isAddNewCla) {
        newClaA->infereType = infereType;

        if (this->vUsedClas.empty()) {
            newClaA->parentIds.insert(-1);
        } else {
            if (infereType == InfereType::RN) {
                newClaA->parentIds.insert(newClaA->literals->parentLitPtr->claPtr->ident);
            } else {
                for_each(vUsedClas.begin(), vUsedClas.end(), [&newClaA](Clause * cla) {
                    newClaA->parentIds.insert(cla->ident);
                });
            }
        }
        vNewClas.push_back(newClaA);
    }
    return isAddNewCla;
}




// <editor-fold defaultstate="collapsed" desc="输出">
/// 回溯所有参与的子句，A.边添加vNewR，边检查是否恒真/相同删除；B.若outStr!=null生成主界线输出；C.newCla赋值使用过的子句    
/// \param actCla
/// \param vNewR
/// \param sOutStr
/// \param newCla
/// \return RuleOk; TAUTOLOGY；MoreLit

ResRule TriAlgExt::CheckAndFindUsed(Clause * checkCla, vector<Literal*>&vNewR) {

    ResRule resRule = ResRule::RULEOK;
    if (std::find(vUsedClas.begin(), vUsedClas.end(), checkCla) == vUsedClas.end()) {
        //说明 一个子句中 不同文字被同一个子句下拉。

        vector<Clause*>vMatchClas;
        vMatchClas.reserve(16);
        vMatchClas.push_back(checkCla);
        //=== 分析并生成主界线信息
        while (!vMatchClas.empty()) {
            Clause* claPtr = vMatchClas.back();
            vUsedClas.push_back(claPtr);
            vMatchClas.pop_back();
            for (Literal*lit = claPtr->literals; lit; lit = lit->next) {
                //对应C+
                if (lit->EqnQueryProp(EqnProp::EPIsHold)) {
                    //添加vNewR文字 恒真或相同删除，查找使用过的子句
                    ResRule resRule = CheckNewR(lit, vNewR);
                    if (vNewR.size() > StrategyParam::MaxLitNumOfNewCla) {
                        resRule = ResRule::MoreLit;
                        break;
                    }
                    //--- 相同文字，不加入vNewR。也不设置文字属性。因为同一个子句Ci 有可能参与多个不同R的生成。
                    if (ResRule::RULEOK == resRule) {
                        vNewR.push_back(lit);
                    } else if (ResRule::TAUTOLOGY == resRule) {
                        break;
                    }
                }//对应C-
                else if (lit->matchLitPtr) {
                    //--- 已经添加的子句不再添加
                    if (std::find(vUsedClas.begin(), vUsedClas.end(), lit->matchLitPtr->claPtr) == vUsedClas.end()) {
                        vMatchClas.push_back(lit->matchLitPtr->claPtr);

                    }
                }
            }
            if (ResRule::RULEOK != resRule)
                break;
        }
    }
    return resRule;
}

/// 输出参与演绎子句文字和分离式R
/// \param vNewR 剩余文字集

void TriAlgExt::OutTriAndR(vector<Literal*>& vNewR) {
    string sOutStr;
    OutTri(sOutStr);
    //=== 生成R输出信息
    OutR(vNewR, sOutStr);
    //=== 输出 .r 信息
    FileOp::getInstance()->outRun(sOutStr);
}

void TriAlgExt::OutTri(string & sOutStr) {
    string sTmpOutStr = "\n";
    for (Clause*cla : vUsedClas) {
        for (Literal* lit = cla->literals; lit; lit = lit->next) {
            //lit->matchLitPtr->GetLitInfoWithSelf(sTmpOutStr);
            lit->GetLitInfoWithSelf(sTmpOutStr);
            if (lit->matchLitPtr) {
                sTmpOutStr += " \t\t% <->";
                lit->matchLitPtr->getLitInfo(sTmpOutStr);
            }
            sTmpOutStr += "\n";
        }
    }
    sOutStr += sTmpOutStr;
}

void TriAlgExt::OutR(vector<Literal*>&vNewR, string & outStr) {
    size_t uSizeR = vNewR.size();
    if (0 == uSizeR) {
        outStr += "R:空子句";
        //string cnf(c_0_6,plain,    ( $false ),    inference(sr,[status(thm)],[c_0_4,c_0_5]),    [proof]).        
    } else {
        //输出R  
        outStr += "R[" + to_string(Env::global_clause_counter) + "]";
        vNewR[0]->GetLitInfoWithSelf(outStr);
        for (int i = 1; i < uSizeR; ++i) {
            outStr += "+";
            vNewR[i]->GetLitInfoWithSelf(outStr);

        }
        outStr += "\n";
    }
}

/// 单元子句约减后的规则检查
/// \param actCla
/// \param vDelActLit
/// \param arrHoldLits
/// \return RULEOK RSubsump TAUTOLOGY

ResRule TriAlgExt::CheckUnitRule(Literal* actLitPtr, vector<Literal*>&vDelActLit, vector<Literal*> &vNewR) {

    //剩余文字列表
    Literal * arrHoldLits[actLitPtr->claPtr->LitsNumber() - 1 + vNewR.size()];
    memcpy(&arrHoldLits, &vNewR[0], vNewR.size() * sizeof (Literal*));
    uint16_t uRLitsNum = vNewR.size();

    uint16_t uDelLitsNum = vDelActLit.size();

    ResRule res = ResRule::RULEOK;
    //====== 遍历所有文字
    for (Literal* checkLitA = actLitPtr->claPtr->literals; checkLitA; checkLitA = checkLitA->next) {
        if (checkLitA == actLitPtr || !checkLitA->EqnQueryProp(EqnProp::EPIsHold))
            continue;
        bool isHold = true;
        //--- 检查与R文字互补/相同
        if (!vNewR.empty()) {

            ResRule resRule = CheckNewR(checkLitA, vNewR);
            if (ResRule::TAUTOLOGY == resRule) {
                res = ResRule::TAUTOLOGY;
                break;
            } else if (ResRule::DelHoldLit == resRule) {
                vDelActLit.push_back(checkLitA);
                continue;
            }
        }

        //--- 检查与自己的剩余文字互补/相同
        for (Literal* checkLitB = checkLitA->next; checkLitB; checkLitB = checkLitB->next) {
            //检查是否恒真&相同
            if (checkLitA->EqualsStuct(checkLitB)) {
                if (checkLitA->isSameProps(checkLitB)) { //相同文字，删除
                    isHold = false;
                    vDelActLit.push_back(checkLitA); //checkLitA->EqnDelProp(EqnProp::EPIsHold);                                    
                } else { //互补 则为恒真删除 ->换下一个单元子句检查下拉
                    res = ResRule::TAUTOLOGY;
                }
                break;
            }
        }
        if (ResRule::RULEOK != res) {
            break;
        }
        if (isHold) {
            arrHoldLits[uRLitsNum++] = checkLitA;
        }
    }
    //--- 检查-[FS 归入冗余]                    
    if (ResRule::RULEOK == res && fol->HoldLitsIsRundacy(arrHoldLits, uRLitsNum, nullptr, nullptr)) {
        res = ResRule::RSubsump; //子句冗余        
    }
    uint16_t uDLNum = vDelActLit.size();
    if (ResRule::RULEOK == res) {
        //开始标注主动子句中被删除的剩余文字                         
        for (int i = uDelLitsNum; i < uDLNum; ++i) {
            vDelActLit[i]->EqnDelProp(EqnProp::EPIsHold);
        }
    } else {
        //失败了回退被删除的文字
        for (int i = uDelLitsNum; i < uDLNum; ++i) {
            vDelActLit.pop_back();
            assert(vDelActLit.size() == uDelLitsNum);
        }
    }
    return res;
}

/// 主界线下拉后，主动子句剩余文字规则检查.A.剩余文字添加到vNewR恒真/相同(删除） B.与下拉文字[合一]相同,当alitPtr为基项
/// \param alitPtr
/// \param vHoldLits    主动子句中的剩余文字
/// \param vNewR        生成的R
/// \param vDelLits     存储vHoldLits中的被删除的文字，要么与R相同，要么与alitPtr合一相同
/// \return             RULEOK，TAUTOLOGY

ResRule TriAlgExt::RuleCheck(Literal* alitPtr, vector<Literal*>& vNewR) {//, uint8_t*vDelLits
    Clause* actCla = alitPtr->claPtr;
    vector<Literal*>vDelLits;

    vDelLits.reserve(actCla->LitsNumber() - 1);
    ResRule resRule = ResRule::RULEOK;

    //--- 遍历剩余文字，检查是否可以加入vNewR；1.返回Tautology ；2，返回ALitSameBLits文字相同
    // for (int idx = 0; idx < vHoldLits.size(); ++idx) {
    for (Literal* checklitPtr = actCla->literals; checklitPtr; checklitPtr = checklitPtr->next) {

        if (checklitPtr == alitPtr || !checklitPtr->EqnQueryProp(EqnProp::EPIsHold))
            continue;
        //--- 检查是否可以和下拉文字 alitPtr 合一相同（删除）. 条件：只有下拉文字不含变元项（基项） PS:这里是在做因子归结 factor rule                          
        //---【9.10zj】 感觉这个判断是没有必要的。
        //--- [9.13] 如果没有理解错误的话， 这个相当于2个不同文字被同一个主对角线文字下拉 再次确定 不需要
        //        if (alitPtr->IsGround(true)&& (checklitPtr->isSameProps(alitPtr))) {
        //            uint32_t uBeforeSameCheck = subst->Size();
        //            if (unify.literalMgu(checklitPtr, alitPtr, subst)) {
        //                //--- 与alitPtr合一相同(删除)；添加删除文字，后面统一操作                //vDelLits[idx] = 1;
        //                vDelLits.push_back(checklitPtr);
        //                //resRule = ResRule::DelHoldLit;
        //                continue;
        //            } else {
        //                subst->SubstBacktrackToPos(uBeforeSameCheck); //回退
        //            }
        //        }        

        resRule = CheckNewR(checklitPtr, vNewR);
        if (ResRule::TAUTOLOGY == resRule) {
            break;
        } else if (ResRule::DelHoldLit == resRule) {
            //--- 与vNewR中文字相同(删除) --添加删除文字，后面统一操作            vDelLits[idx] = 1;
            vDelLits.push_back(checklitPtr);
            resRule = ResRule::RULEOK;
            continue;
        }
    }

    if (ResRule::RULEOK == resRule) {//给删除的文字上标记
        for (Literal*lit : vDelLits) {
            lit->EqnDelProp(EqnProp::EPIsHold);
        }
    }
    return resRule;
}


/// 
/// \param checkLit
/// \param vNewR
/// \return ResRule::RULEOK;文字相同--ALitSameBLits;恒真--TAUTOLOGY

ResRule TriAlgExt::CheckNewR(Literal* checkLit, vector<Literal*>& vNewR) {
    ResRule res = ResRule::RULEOK;
    for (auto&litR : vNewR) {
        if (checkLit->EqualsStuct(litR)) {
            if (checkLit->isSameProps(litR)) {
                //与已有的剩余文字相同，添加失败
                res = ResRule::DelHoldLit;
            } else {
                //与已有的剩余文字恒真，生成过程失败
                res = ResRule::TAUTOLOGY;
            }
            break;
        }
    }
    return res;
}

void TriAlgExt::OutNewClaInfo(Clause* newCla, InfereType infereType) {
    assert(newCla);
    string strCla = "";
    //空子句
    if (newCla->ClauseIsEmpty()) {
        strCla = "cnf(c" + to_string(Env::global_clause_counter + 1) + ",plain,($false)";
    } else {
        newCla->getStrOfClause(strCla, false);
    }

    string parentCla = "";
    if (infereType == InfereType::RN) {
        parentCla = "c" + to_string(newCla->literals->parentLitPtr->claPtr->ident);
    } else {
        parentCla = "c" + to_string(*newCla->parentIds.begin());
        for_each(++newCla->parentIds.begin(), newCla->parentIds.end(), [&parentCla](uint32_t pClaId) {
            parentCla += ",c" + to_string(pClaId);
        });
    }

    strCla += (newCla->ClauseIsEmpty()) ?
            ",inference(" + InferenceInfo::getStrInfoType(infereType) + ",[status(thm)],[" + parentCla + "]), ['proof']).\n"
            : ",inference(" + InferenceInfo::getStrInfoType(infereType) + ",[status(thm)],[" + parentCla + "])).\n";

    FileOp::getInstance()->outInfo(strCla);
}

void TriAlgExt::OutUnsatInfo(InfereType infereType) {
    string strCla = "cnf(c" + to_string(Env::global_clause_counter + 1) + ",plain,($false)";
    string parentCla = "c" + to_string((*(vUsedClas.begin()))->ident);
    for_each(++vUsedClas.begin(), vUsedClas.end(), [&parentCla](Clause * cla) {
        parentCla += ",c" + to_string(cla->ident);
    });
    strCla += ",inference(" + InferenceInfo::getStrInfoType(infereType) + ",[status(thm)],[" + parentCla + "]), ['proof']).\n";
    FileOp::getInstance()->outInfo(strCla);
}

// </editor-fold>
