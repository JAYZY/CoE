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
    list<Clause*>*lstCla = fol->getWorkClas();
    list<Clause*>::iterator itSelCla = lstCla->begin();
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

    vector<Literal*>vHoldLits;
    vector<Literal*> vNewR;
    int iIter = 0;
    //cout << "=============" << endl;
    /*--- 按照文字数 从少到多选择子句----*/
    for (; itSelCla != lstCla->end(); ++itSelCla) {

        ++iIter;
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
        resTri = UnitClaReduce(givenCla, vHoldLits);

        if (RESULT::UNSAT == resTri) {
            vUsedClas.push_back(givenCla);
            goto UnsatOut;
        } else if (RESULT::SUCCES == resTri) {
            isDeduct = true;
        }

        /*--- 与主界线文字归结[下拉] ----*/
        Literal* triLit = nullptr;
        resTri = MainTriReduce(&triLit, vHoldLits, vNewR);
        if (triLit == nullptr) {//所有文字均下拉 -- 该子句不放入△
            subst->SubstBacktrackToPos(iIniSubStSize);
            continue;
        }
        if (RESULT::UNSAT == resTri) {
            vUsedClas.push_back(givenCla);
            goto UnsatOut;
        } else if (RESULT::SUCCES == resTri) {
            isDeduct = true;
        } else if (RESULT::FAIL == resTri) {
            isDeduct = false;
        }
        //--- 无论是 fail 还是 nomug 说明该子句不应该加入 △；
        if (isDeduct) {
            vector<Literal*> *vRLits = (RESULT::NOMGU == resTri) ? &vHoldLits : &vNewR;
            /*--- 判断能否生成新子句加入S----*/
            if (vRLits->size() > StrategyParam::MaxLitNumOfNewCla) {
                isDeduct = false;
            } else {
                //--- 判断&生成新子句         
                Clause* newCla = new Clause();
                newCla->bindingAndRecopyLits(*vRLits);
                if (newCla->MinFuncLayer() > StrategyParam::R_MAX_FUNCLAYER) {
                    DelPtr(newCla);
                    isDeduct = false;
                } else if (Add2NewClas(newCla, InfereType::SCS)) {
                    //加入新子句失败 ， 路径是冗余的因此不能添加到三角形中  vUsedClas.push_back(givenCla);
                    for (Clause*cla : vUsedClas) {
                        newCla->parentIds.insert(cla->ident);
                    }
                    vUsedClas.push_back(givenCla);
                    newCla->parentIds.insert(givenCla->ident);
                    //--- 输出.r 信息 --- 
                    OutTriAndR(*vRLits);
                    //--- 输出.i 信息 --- 
                    OutNewClaInfo(newCla, InfereType::SCS);

                    if (vRLits->size() < StrategyParam::MaxLitNumOfR) { //超过限制，不放入主界线
                        //--- 目前放入主界线文字的策略：按照子句中文字顺序第一个保留的文字放入主界线
                        assert(triLit);
                        triLit->EqnDelProp(EqnProp::EPIsHold);
                        ++triLit->usedCount;
                        vMainTri.push_back(triLit);
                    } else {
                        isDeduct = false;
                    }
                    fol->insertNewCla(newCla);
                } else {
                    isDeduct = false;
                }
            }
        } else if (vMainTri.empty()) {

            assert(triLit);
            triLit->EqnDelProp(EqnProp::EPIsHold);
            ++triLit->usedCount;
            vMainTri.push_back(triLit);
        }
        //        //--- 下拉不成功回退使用过的子句
        //        if (!isDeduct) {
        //
        //            while (vUsedClas.size() > iUsedClaSize) {
        //                vUsedClas.pop_back();
        //            }
        //        }
    }
    //--- 完成一轮△，输出新生成的子句 ---
    //    for (int i = 0; i < vNewClas.size(); ++i) {
    //        //=== 修改新子句权重-------------
    //        //        int pri = 0;
    //        //        Literal* tmpLit = newCla->literals;
    //        //        for (; tmpLit; tmpLit = tmpLit->next) {
    //        //            pri += tmpLit->parentLitPtr->claPtr->priority;
    //        //        }
    //        //        /*改变新子句的权重R的权重为文字权重的平均--遍历第一个△路径除外 取整*/
    //        //        //注意:由于目标子句初始化权重为100 因此 平均值后 若新子句中有目标子句参与自然权重会较高
    //        //        newCla->priority = pri / (int) (newCla->LitsNumber());
    //        fol->insertNewCla(vNewClas[i]);
    //    }
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

/// 单元子句对actCla中的文字进行下拉操作。
/// 前提： 子句中的文字已经排好序
/// \param actCla
/// \return   NOMGU - 没有下拉； SUCCES - 有下拉文字, UNSAT - 

RESULT TriAlgExt::UnitClaReduce(Clause * actCla, vector<Literal*>&vHoldLits) {
    RESULT resReduce = RESULT::NOMGU;
    Literal* actLitPtr = actCla->literals;
    //剩余文字列表
    Literal * arrHoldLits[actCla->LitsNumber() - 1];

    vector<Literal*>vDelActLit;
    vDelActLit.reserve(4);


    //== 遍历子句中的所有文字，
    for (; actLitPtr; actLitPtr = actLitPtr->next) {
        if (!actLitPtr->EqnQueryProp(EqnProp::EPIsHold))
            continue;
        vector<Clause* >&cmpUnitClas = actLitPtr->IsPositive() ? fol->vNegUnitClas : fol->vPosUnitClas; //可以考虑优化 用索引树
        //=== 遍历单元子句 ========================
        for (int ind = 0; ind < cmpUnitClas.size(); ++ind) {

            Clause* candUnitCal = cmpUnitClas[ind];
            if (candUnitCal->isDel()) { //被删除的单元子句不处理 [待优化]              
                continue;
            }
            Lit_p candLit = candUnitCal->Lits();
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

            //=== 规则检查！ PS算法：每次下拉文字后，若文字有共享变元，则检查1.是否存在恒真&相同文字；2.Forward Subsumed 放到最后检查                                
            if (res) {//&& actLitPtr->getVarState() == VarState::shareVar
                int16_t holdLitIdx = 0;
                Literal* checkLitA = actCla->literals;
                //=== 检查-[恒真&相同文字]
                vDelActLit.clear();
                for (; checkLitA; checkLitA = checkLitA->next) {
                    if (checkLitA == actLitPtr || !checkLitA->EqnQueryProp(EqnProp::EPIsHold))
                        continue;
                    bool isHold = true;
                    for (Literal* checkLitB = checkLitA->next; checkLitB; checkLitB = checkLitB->next) {
                        //检查是否恒真&相同
                        if (checkLitA->equalsStuct(checkLitB)) {
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
                    if (isHold) {
                        arrHoldLits[holdLitIdx++] = checkLitA;
                    }
                }

                //=== 检查-[FS 归入冗余]                    
                if (res && fol->HoldLitsIsRundacy(arrHoldLits, holdLitIdx, nullptr, nullptr)) {
                    res = false; //子句冗余
                }
                if (res) {
                    //开始标注主动子句中的被删除的剩余文字                   
                    for (Literal* delLit : vDelActLit) {
                        delLit->EqnDelProp(EqnProp::EPIsHold);
                    }
                }
            }

            if (res) {
                actLitPtr->EqnDelProp(EqnProp::EPIsHold);
                //=== 输出变名的单元子句：1.重命名的；2没有输出过 THEN 输出到结果文件
                if (isRN) {

                    candUnitCal->bindingLits(candLit); //生成新单元子句                    
                    Clause* tmpOriCandCla = cmpUnitClas[ind];
                    candUnitCal->parentIds.insert(tmpOriCandCla->ident);
                    //--- 输出新子句到info文件
                    OutNewClaInfo(candUnitCal, InfereType::RN);
                    //--- 输出到.r文件
                    string str = "\n";
                    tmpOriCandCla->literals->GetLitInfoWithSelf(str);
                    str += "\nR[" + to_string(candUnitCal->ident) + "]:";
                    candLit->GetLitInfoWithParent(str);
                    FileOp::getInstance()->outRun(str + "\n");

                    candUnitCal->literals->EqnDelProp(EqnProp::EPIsHold);
                    //--- 添加该单元子句副本到删除列表中,完成三角形后删除.
                    delUnitCla.push_back(candUnitCal);
                    //--- 变名后的单元子句都不添加到主界线

                }

                actLitPtr->matchLitPtr = candUnitCal->literals; //记录匹配的文字用于输出
                vUsedClas.push_back(candUnitCal); //记录使用过的单元子句；
                //---记录成功匹配单元子句的位置信息。 索引列表类型；索引列表中的位置,后续处理                
                Clause* tmpCla = cmpUnitClas[ind];
                //单元子句使用次数-1, 降低优先级
                tmpCla->priority -= 1;
                //移动匹配成功的 单元子句 -- {两种算法:A.没用一次移到最后,下次使用不用排序;B.每次使用记录weight,下次使用排序.目前使用A算法)
                cmpUnitClas.erase(cmpUnitClas.begin() + ind);
                cmpUnitClas.push_back(tmpCla);
                //actLitP->IsPositive() ? sMoveNegUnitCla.insert(ind) : sMovePosUnitCla.insert(ind);
                //=== 匹配成功换下一个剩余文字   
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
    }
    for (Literal* ckholdLit = actCla->literals; ckholdLit; ckholdLit = ckholdLit->next) {
        if (ckholdLit->EqnQueryProp(EqnProp::EPIsHold)) {
            vHoldLits.push_back(ckholdLit);
        }
    }
    /*UNSAT*/
    if (vHoldLits.empty()) {
        resReduce = RESULT::UNSAT;
    }
    return resReduce;
}

/// 
/// \param actCla
/// \param vHoldLits
/// \param vNewR
/// \return  NOMGU -- actCla没有发生任何下拉， SUCCES -- 成功下拉文字 ， FAIL--下拉失败(可能冗余，可能文字数操过限制)

RESULT TriAlgExt::MainTriReduce(Literal **actLit, vector<Literal*>&vHoldLits, vector<Literal*> &vNewR) {
    RESULT resReduce = RESULT::NOMGU;
    //剩余文字列表 长度为 MaxLitOfNewR+现有文字长度
    Literal * arrHoldLits[vHoldLits.size() + StrategyParam::MaxLitNumOfNewCla];

    if (!vMainTri.empty()) {
        //uint8_t vDelLits[vHoldLits.size()];
        //=== 遍历子句中的所有文字 ========================
        for (Literal* alitPtr : vHoldLits) {
            if (!alitPtr->EqnQueryProp(EqnProp::EPIsHold))
                continue;
            //=== 遍历主届线文字 ========================
            for (Literal* mTriLitPtr : vMainTri) {
                //--- 确保互补 --- 
                if (alitPtr->isSameProps(mTriLitPtr) || (!alitPtr->EqnIsEquLit() && alitPtr->lterm->fCode != mTriLitPtr->lterm->fCode))
                    continue;

                uint32_t bpBeforeUnify = subst->Size();
                bool res = unify.literalMgu(alitPtr, mTriLitPtr, subst);
                int iUsedClaSize = vUsedClas.size();
                if (res) {
                    //=== 规则检查 === 1.剩余文字是否是 恒真的 ；2.剩余文字是否是冗余的


                    //--- 检查被动子句规则；A.回溯所有相关子句，剩余文字添加到vNewR。B.检查vNewR是否恒真/相同(删除)
                    ResRule resRule = CheckAndFindUsed(mTriLitPtr->claPtr, vNewR, vUsedClas); //RuleOk; TAUTOLOGY；【MoreLit--说明这个子句加入到已有的△路径超过文字数限定，换一个子句中的文字下拉】。

                    //--- 检查主动子句剩余文字规则.A.剩余文字添加到vNewR恒真/相同(删除） B.与下拉文字[合一]相同,当alitPtr为基项
                    if (ResRule::RULEOK == resRule) {
                        resRule = RuleCheck(alitPtr, vHoldLits, vNewR); //RULEOK，[TAUTOLOGY--表示合一失败换一个合一文字]，
                    }
                    if (ResRule::TAUTOLOGY == resRule || ResRule::MoreLit == resRule) {
                        res = false;
                    } else {
                        // vUsedClas.push_back(mTriLitPtr->claPtr); //添加使用过的子句
                        alitPtr->matchLitPtr = mTriLitPtr;
                        alitPtr->EqnDelProp(EqnProp::EPIsHold);
                        resReduce = RESULT::SUCCES; //表示有下拉发生
                        break;
                    }
                }
                assert(!res);
                while (vUsedClas.size() > iUsedClaSize) {
                    vUsedClas.pop_back(); //还原使用过的子句集
                }
                //=== 下拉失败，还原 ===
                subst->SubstBacktrackToPos(bpBeforeUnify);
            }
        }

        //=== 有下拉发生， 生成新子句
        if (RESULT::SUCCES == resReduce) {
            //=== 对剩余文字进行处理 ===[检查FS，文字个数]
            //--- 拷贝vNewR到arrHoldLits
            memcpy(&arrHoldLits, &vNewR[0], vNewR.size() * sizeof (Literal*));
            int iStartInd = vNewR.size();
            for (Literal* holdLit : vHoldLits) {
                if (!holdLit->EqnQueryProp(EqnProp::EPIsHold))
                    continue;

                //--- 剩余文字数超过最大限制
                if (iStartInd > StrategyParam::MaxLitNumOfNewCla) {
                    resReduce = RESULT::FAIL;
                    break;
                }

                arrHoldLits[iStartInd++] = holdLit;
                if ((*actLit) == nullptr)
                    (*actLit) = holdLit;
                vNewR.push_back(holdLit);
                if (iStartInd > StrategyParam::MaxLitNumOfNewCla) {
                    resReduce = RESULT::FAIL; //文字数超过最大限制
                    break;
                }
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
                    if (fol->HoldLitsIsRundacy(arrHoldLits, iStartInd, &setTmpUsedCla, vHoldLits[0]->claPtr)) {
                        //return ResRule::RSubsump; 
                        resReduce = RESULT::FAIL; //子句冗余
                    } else {

                    }
                }
            }
        }
    }
    if (RESULT::NOMGU == resReduce) {
        (*actLit) = vHoldLits[0];
    }
    return resReduce;
}

/// 主界线下拉后，主动子句剩余文字规则检查.A.剩余文字添加到vNewR恒真/相同(删除） B.与下拉文字[合一]相同,当alitPtr为基项
/// \param alitPtr
/// \param vHoldLits    主动子句中的剩余文字
/// \param vNewR        生成的R
/// \param vDelLits     存储vHoldLits中的被删除的文字，要么与R相同，要么与alitPtr合一相同
/// \return             RULEOK，TAUTOLOGY

ResRule TriAlgExt::RuleCheck(Literal* alitPtr, vector<Literal*>& vHoldLits, vector<Literal*>& vNewR) {//, uint8_t*vDelLits
    vector<Literal*>vDelLits;
    vDelLits.reserve(vHoldLits.size());
    ResRule resRule = ResRule::RULEOK;
    //--- 遍历剩余文字，检查是否可以加入vNewR；1.返回Tautology ；2，返回ALitSameBLits文字相同
    // for (int idx = 0; idx < vHoldLits.size(); ++idx) {
    for (Literal* checklitPtr : vHoldLits) {
        if (!checklitPtr->EqnQueryProp(EqnProp::EPIsHold))
            continue;

        //--- 检查是否可以和下拉文字 alitPtr 合一相同（删除）. 条件：只有下拉文字不含变元项（基项）                           
        if (alitPtr->IsGround(true)) {
            uint32_t uBeforeSameCheck = subst->Size();
            if (unify.literalMgu(checklitPtr, alitPtr, subst)) {
                //--- 与alitPtr合一相同(删除)；添加删除文字，后面统一操作                //vDelLits[idx] = 1;
                vDelLits.push_back(checklitPtr);
                //resRule = ResRule::DelHoldLit;
                continue;
            } else {
                subst->SubstBacktrackToPos(uBeforeSameCheck); //回退
            }
        }

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
    if (ResRule::RULEOK == resRule) {//给删除的文字给上标记
        for (Literal*lit : vDelLits) {
            lit->EqnDelProp(EqnProp::EPIsHold);
        }
    }
    return resRule;
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

ResRule TriAlgExt::CheckAndFindUsed(Clause * checkCla, vector<Literal*>&vNewR, vector<Clause*> &vTmpUsedCla) {

    vector<Clause*>vMatchClas;
    vMatchClas.reserve(16);


    vMatchClas.push_back(checkCla);
    ResRule resRule = ResRule::RULEOK;

    //=== 分析并生成主界线信息
    while (!vMatchClas.empty()) {
        Clause* claPtr = vMatchClas.back();
        vMatchClas.pop_back();
        for (Literal*lit = claPtr->literals; lit; lit = lit->next) {
            if (lit->EqnQueryProp(EqnProp::EPIsHold)) {
                //添加vNewR文字
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
            } else if (lit->matchLitPtr) {
                //--- 已经添加的子句不再添加
                if (std::find(vTmpUsedCla.begin(), vTmpUsedCla.end(), lit->matchLitPtr->claPtr) == vTmpUsedCla.end()) {
                    vMatchClas.push_back(lit->matchLitPtr->claPtr);
                }
            }
        }
        vTmpUsedCla.push_back(claPtr);
        if (ResRule::RULEOK != resRule)
            break;
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
        vNewR[0]->getLitInfo(outStr);
        vNewR[0]->getStrOfEqnTSTP(outStr);
        for (int i = 1; i < uSizeR; ++i) {

            outStr += "+";
            vNewR[i]->getLitInfo(outStr);
            vNewR[i]->getStrOfEqnTSTP(outStr);
        }
        outStr += "\n";
    }
}
/// 
/// \param checkLit
/// \param vNewR
/// \return ResRule::RULEOK;文字相同--ALitSameBLits;恒真--TAUTOLOGY

ResRule TriAlgExt::CheckNewR(Literal* checkLit, vector<Literal*>& vNewR) {
    ResRule res = ResRule::RULEOK;
    for (auto&litR : vNewR) {
        if (checkLit->equalsStuct(litR)) {
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