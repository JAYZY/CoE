/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   TriAlg.cpp
 * Author: Zhong Jian<77367632@qq.com>
 * 
 * Created on 2018年10月25日, 上午10:46
 */

#include "TriAlg.h"
#include "CLAUSE/LiteralCompare.h"
#include "HEURISTICS/SortRule.h"
#include "Inferences/Simplification.h"
#include <set>
#include <algorithm>
#include <bits/stl_set.h>

TriAlg::TriAlg(Formula* _fol) : fol(_fol), subst(nullptr) {

}

TriAlg::TriAlg(const TriAlg& orig) {
}

TriAlg::~TriAlg() {
}
/// 前提条件:同一子句中完成 factor / duplicate delete /tautology 处理]
//  1. 子句中没有(合一)互补文字,2.子句中文字经过了 factor处理.
/// \param givenCla
/// \return 

RESULT TriAlg::GenerateTriByRecodePath(Clause* givenCla) {
    /*
     * 1.传入目标子句 --  所有文字与单元子句进行合一  
     * 2.选择自动文字 优先选择负文字
     * 3.与全局谓词符号进行比较
     */
    // int triNum = 0;
    cout << "# 起步子句:" << givenCla->ident << "===========" << endl;

    clearVect();
    RESULT resTri = RESULT::NOMGU;


    subst = new Subst();


    fol->unitClasSort(); //遵循 稳定度高的 VS 稳定度高的
    givenCla->SortLits(); //子句中文字的排序策略 :建议 先负后正,先稳定低后稳定高  
    Lit_p gLit = givenCla->Lits(); //选择一个文字 


    //经过多次合一，构建三角形
    uint16_t uActLeftLitNum = 0; //记录主动归结子句的剩余文字个数
    uint16_t uPasLeftLitNum = 0; //记录被动归结子句的剩余文字个数
    Lit_p actLit = nullptr;

    setUsedCla.insert(givenCla); //记录起步子句已经使用.
    //对选择的子句 进行 单文字匹配
    while (gLit) {
        gLit->EqnDelProp(EqnProp::EPIsLeft);

        if (!unitResolutionBySet(gLit)) {
            if (actLit == nullptr) {
                actLit = gLit;
            } else {
                ++uActLeftLitNum; //剩余文字个数
                gLit->EqnSetProp(EqnProp::EPIsLeft); //设置该文字left 属性
            }
        }
        gLit = gLit->next;
    }


    if (0 == uActLeftLitNum && actLit == nullptr)
        return RESULT::UNSAT;
    /************************************************************************/
    /*主动文字选择原则 ,1 尽量选择 负文字 2 尽量选择稳定度低的文字	
    /************************************************************************/

    //回退相关
    vector<uint32_t> vRecodeBackPoint; //替换的回退点,每次成功一个配对 就记录一次 
    vRecodeBackPoint.reserve(32);

    vector<uint32_t> vPasCandBackPoint; //候选文字集序号的回退点
    vPasCandBackPoint.reserve(32);



    Lit_p pasLit = nullptr; //被动归结文字
    vector<Literal*>* vCandLit = nullptr; //候选文字集合
    Clause* actCla = givenCla; //记录主动子句

    int backpoint = 0;
    uint32_t pasLitInd = 0;
    // Lit_p rollBackAvoidLit = nullptr; //回退要避免的候选文字
    while (1) {


        //=======遍历主动子句中剩余文字 -----------------------------------------------------
        while (actLit) {

            //根据文字互补谓词项对应的互补文字得到候选被动文字
            vCandLit = fol->getPairPredLst(actLit);

            /* 对候选被动文字进行排序 A.被动文字所在子句文字数从少到多;B.相同文字数情况下考虑稳定度,主动被动相近度 等启发式策略[u]如何高效的排序?*/
            if (resTri != RESULT::RollBack)
                stable_sort(vCandLit->begin(), vCandLit->end(), SortRule::PoslitCmp);

            resTri = RESULT::NOMGU;

            //遍历候选被动文字子句顺序进行匹配查找 -----------------------------------------------------
            for (; pasLitInd < vCandLit->size(); ++pasLitInd) {


                // resTri = RESULT::NOMGU;

                pasLit = vCandLit->at(pasLitInd); //候选被动归结文字
                //                if (pasLit == rollBackAvoidLit)
                //                    continue;
                //==========候选文字的条件限制==================
                {
                    /*同一子句中文字不进行比较;归结过的子句不在归结;文字条件限制*/
                    if (pasLit->claPtr == actLit->claPtr || setUsedCla.find(pasLit->claPtr) != setUsedCla.end())
                        continue;
                    /*限制子句中文字数个数
                    if (vNewR.size() + givenCla->uLitNum + candClaP->uLitNum - 2 > StrategyParam::R_MAX_NUM)
                        continue;*/
                    //不允许跟上一轮母式进行归结  
                    if ((pasLit->parentLitPtr && pasLit->parentLitPtr->claPtr == actLit->claPtr)
                            || (actLit->parentLitPtr && actLit->parentLitPtr->claPtr == pasLit->claPtr))
                        continue;
                }

                //是否找到合一文字
                backpoint = subst->Size();
                bool res = unify.literalMgu(actLit, pasLit, subst);
                if (!res) {
                    subst->SubstBacktrackToPos(backpoint);
                }
                uPasLeftLitNum = 0;
                //规则检查
                //int backALitPoint = vALitTri.size();


                uint32_t litSize = pasLit->claPtr->LitsNumber() - 1;
                Lit_p *pasClaLeftLits = new Lit_p[litSize];
                memset(pasClaLeftLits, 0, sizeof (Lit_p) * litSize);
                ResRule resRule = RuleCheck(actLit, pasLit, pasClaLeftLits, uPasLeftLitNum);
                if (resRule == ResRule::ChgActLit) {//换主界线文字
                    DelArrayPtr(pasClaLeftLits);
                    subst->SubstBacktrackToPos(backpoint);
                    break;
                } else if (resRule == ResRule::ChgPosLit) {//换被归结文字
                    DelArrayPtr(pasClaLeftLits);
                    subst->SubstBacktrackToPos(backpoint);
                    continue;
                }//检查剩余文字 是否是无效的(FS:向前归入冗余/恒真) 无效返回 true 则改变被归结文字                
                if (fol->leftLitsIsRundacy(pasClaLeftLits, uPasLeftLitNum, vNewR)) {
                    DelArrayPtr(pasClaLeftLits);
                    subst->SubstBacktrackToPos(backpoint);
                    continue;
                }
                //将剩余的单文字 添加到 子句集中.---------
                if (1 == uPasLeftLitNum && vNewR.empty()) {
                    Clause* newCla = new Clause();
                    Literal* newLit = pasClaLeftLits[0]->renameCopy(newCla);

                    newCla->bindingLits(newLit);
                    fol->insertNewCla(newCla);

                    // cout << "out fol test:" << endl;
                    //  fol->printProcessedClaSet(stdout);
                }

                //-------------------------------------
                DelArrayPtr(pasClaLeftLits);
                //一次三角形构建成功
                assert(resRule == ResRule::RULEOK);
                {
                    //1.添加主界线文字
                    actLit->EqnDelProp(EqnProp::EPIsLeft);
                    vALitTri.push_back(new ALit{actLit, pasLit, 0});

                    //2.添加回退点--替换栈
                    vRecodeBackPoint.push_back(backpoint);
                    //3.添加回退点--候选文字序号
                    vPasCandBackPoint.push_back(pasLitInd);

                    //4.将主动文字的剩余文字添加到 vNewR中        
                    Lit_p aLeftLit = actLit->claPtr->Lits();

                    while (aLeftLit) {/*注:检查恒真冗余,这种情况只发生在起步子句,因为后续子句 均已经检查了 整个剩余文字是否冗余;而对于起步子句,对起步子句的检查应该在三角形开始前完成*/
                        if (aLeftLit->EqnQueryProp(EqnProp::EPIsLeft)) {
                            vNewR.push_back(aLeftLit);
                        }
                        aLeftLit = aLeftLit->next;
                    }
                    setUsedCla.insert(pasLit->claPtr); //记录 被动子句已经使用. 
                }
                resTri = RESULT::SUCCES;
                break;

            }

            if (resTri == RESULT::SUCCES)
                break;

            actLit = actLit->next;
            while (actLit != nullptr&&!actLit->EqnQueryProp(EqnProp::EPIsLeft)) { //没有被left 继续下一个
                actLit = actLit->next;
            }
            pasLitInd = 0;
        }


        //最后一个子句没有剩余文字开始执行ME的truncation操作--------------

        /*  -
         * 注意这种回退是不会回退变元合一情况的
         * 生成新的单元子句,并且检查是否是冗余子句.(主要检查是否可以被其他单元子句归入)
     
        if (0 == uPosLeftLitInd) {
            ALit_p lit = (vALit.back());
            vALit.pop_back();

            if (0 == uReduceNum) {
                //可以得到单文字子句并检查是否是冗余
                if (!Simplification::ForwardSubsumUnitCla(lit->alit, fol->unitClaIndex)) {
                    Clause* newCla = new Clause();
                    Lit_p newLit = lit->alit->EqnCopy(newCla->claTB);
                    newLit->parentLitPtr = lit->alit;
                    newCla->bindingLits(newLit);
                    fol->insertNewCla(newCla); //插入单元子句(会改变单元子句集,索引 ,全局索引---相当于下一次三角形延拓就可以使用了)
                }
            } else {
                for (int i = vALit.size() - 1; i >-1; --i) {
                    lit = vALit[i];
                    if (lit->reduceNum > 0) {
                        --(lit->reduceNum);
                        break;
                    }
                }
                --uReduceNum;
            }
            //回退 newR
        
            actClaLeftLits=new Lit_p[];
        }*/

        //======== △无法延拓时候进行回退====================

        if (resTri == RESULT::NOMGU) {
            //说明遍历所有主动文字均没有找到可以延拓的文字.三角形开始回退
            this->printTri(actCla);
            /*
             * 回退的类型:1.主对角线重新查找下一个互补文字(改变被归结文字);2.重新选择主界线文字(改变主动归结文字);3.重新选择被下拉的主界线文字(改变下拉替换)
             回退分析: 在正常△构建中,从主动子句出发,剩余文字均无法找到Linked文字,此时需要回退.
             * 若某个主动文字均找不到可以延拓的文字,则说明,类型1(改变被归结文字)不适用. 
             * 此时考虑两种情况 检查是否有下拉,若有下拉,则重新下拉并且还是重该主界线文字出发
             */
            //1.主对角线重新查找下一个互补文字(改变被归结文字);

            if (vALitTri.empty() || (actCla == givenCla)) { //已经没有回退点了

                return RESULT::SUCCES;
            }
            //-----------------  回退操作 --------------------------------//
            actLit = vALitTri.back()->alit; //1.设置主动文字==主界线最后一个文字,并且主界线pop(回退)   
            vALitTri.pop_back();
            //2.回退单元子句下拉
            while ((!vALitTri.empty()) && vALitTri.back()->blit->claPtr == actCla) {

                setUsedCla.erase(vALitTri.back()->alit->claPtr); //删除使用的子句 
                vALitTri.pop_back();
            }

            //3.修改其他主界线下拉次数 

            while ((!vReduceLit.empty()) && vReduceLit.back()->cLit->claPtr == actCla) {
                vReduceLit.back()->aLit->reduceNum--;
                vReduceLit.pop_back();
            }


            if (actCla->ident == 15)
                cout << endl;
            cout << "回退" << actCla->ident << endl;
            setUsedCla.erase(actCla); //删除使用的子句 
            actCla = actLit->claPtr;

            //回退vNewR
            while ((!vNewR.empty()) && (vNewR.back()->claPtr == actCla)) {
                vNewR.pop_back();
            }

            //回退 合一替换
            if (!vRecodeBackPoint.empty()) {
                uint32_t backPoint = vRecodeBackPoint.back();
                vRecodeBackPoint.pop_back();
                subst->SubstBacktrackToPos(backPoint);
            }
            //设置候选文字下标
            pasLitInd = 0;
            if (!vPasCandBackPoint.empty()) {
                pasLitInd = vPasCandBackPoint.back() + 1;
                vPasCandBackPoint.pop_back();
                resTri = RESULT::RollBack;
            }
            continue;

        }

        assert(resTri == RESULT::SUCCES);

        //判断△结果,并继续△延拓.


        if (vNewR.empty()) {

            // 1.R为空,被动子句剩余文字为0,则为UNSAT;
            if (0 == uPasLeftLitNum) {
                fprintf(stdout, "[R]:空子句");
                return RESULT::UNSAT;
            }// 2.被动子句 剩余一个文字,生成单文字子句,加入原始子句集
            //            else if (1 == uPasLeftLitNum) {
            //                   //检查是否冗余
            //                fol->LeftUnitLitsIsRundacy()
            //            }

        }


        //输出三角形
        {
            //
            //            fprintf(stdout, "[C%u_%d]", actLit->claPtr->ident, actLit->pos);
            //            actLit->EqnTSTPPrint(stdout, true);
            //            cout << endl;
            //            if (pasLit != nullptr) {
            //                fprintf(stdout, "[C%u_%d]", pasLit->claPtr->ident, pasLit->pos);
            //                pasLit->EqnTSTPPrint(stdout, true);
            //                fprintf(stdout, "\n--------------------\n");
            //            }
            //
            //判断是否UNSAT

            //
            //            //输出R(剩余文字)
            //            // for (int i = 0; i < uActLeftLitInd; ++i) {
            //            cout << "[R]:";
            //            for (Literal* rLit : vNewR) {
            //                fprintf(stdout, "{C%u_%d}", rLit->claPtr->ident, rLit->pos);
            //                rLit->EqnTSTPPrint(stdout, true);
            //                cout << " + ";
            //            }
            //            Lit_p lit = pasLit->claPtr->Lits();
            //            while (lit) {
            //                if (lit->EqnQueryProp(EqnProp::EPIsLeft)) {
            //                    fprintf(stdout, " + {C%u_%d}", lit->claPtr->ident, lit->pos);
            //                    lit->EqnTSTPPrint(stdout, true);
            //                }
            //                lit = lit->next;
            //            }
            //
            //            fprintf(stdout, "\n--------------------\n");
        }

        //5.交换主动被动子句,进行△延拓
        uActLeftLitNum = uPasLeftLitNum;
        uPasLeftLitNum = 0;
        pasLitInd = 0;
        //得到被动子句中,下一个剩余文字
        actCla = pasLit->claPtr;
        actLit = actCla->Lits();
        pasLit = nullptr;
        while (actLit != nullptr&&!actLit->EqnQueryProp(EqnProp::EPIsLeft)) {
            //没有被left 继续下一个
            actLit = actLit->next;
        }
        resTri = RESULT::NOMGU;
    }
    return RESULT::SUCCES;
}

/// 规则检查
/// \param actLit
/// \param candLit
/// \return 

ResRule TriAlg::RuleCheck(Literal*actLit, Literal* candLit, Lit_p *leftLit, uint16_t& uLeftLitNum) {
    /*规则检查 ，
     * 总的原则:  主界线上文字不能相同(互补);剩余文字不能相同(恒真);
     * 一.主界线(A)文字与前面剩余(B)文字不能相同 [不做合一].
     * 1.检查主动文字与前面剩余文字相同 [ 直接换主动文字 ]
     * 2.其它主界线文字与前面B文字相同 [ 说明由于替换导致相同,换下一个被动归结文字 ]
     * 3.主动文字与前面主界线文字相同 [直接换主动文字]
     * 二.对被动归结子句中剩余文字
     * 4.检查与对角线文字互补 [删除(下拉)] -- * 注意需要合一,优先做检查 * 
     * 
     * 5.检查被动文字与对角线文字相同[ 换下一个主动文字 ] -- 不允许主界线文字相同(不考虑合一)
     * 
     * 三.检查剩余文字
     * 1.剩余文字之间是否恒真； [ 换下一个被动归结文字 ]
     * 2.检查与前面剩余文字恒真 [ 换下一个被动归结文字 ]
     * 3.检查与前面剩余文字相同 [ 删除(合并) ]
     *  
     * 四.对剩余文字用单元文字合一(下拉)
     */
    Unify unify;
    Cla_p candCla = candLit->claPtr;

    assert(uLeftLitNum == 0);
    bool isLeftLit = true;
    //优先完成合一操作 -- 
    //==== 遍历被动归结子句的剩余文字 1.单元子句[合一下拉];2.主界线文字[合一下拉] =====================//  
    Lit_p bLit = candCla->Lits();
    for (; bLit != nullptr; bLit = bLit->next) {
        bLit->EqnDelProp(EqnProp::EPIsLeft);
        if (bLit == candLit) {
            continue;
        }

        /* 用单元集子句下拉剩余文字 -- 注意失败后需要 还原*/
        if (unitResolutionBySet(bLit)) {
            cout << "单文字下拉" << endl;
            continue;
        }

        isLeftLit = true;
        /* 检查与对角线文字互补下拉,注意:不检查与actLit的合一互补情况,这个涉及factor内容 */
        for (ALit* elem : vALitTri) {
            Lit_p aLit = elem->alit;

            int backpointA = subst->Size();
            if (bLit->isComplementProps(aLit) && unify.literalMgu(bLit, aLit, subst)) {
                /* 主界线文字下拉(合一)剩余文字.并记录下拉次数[要保留合一替换的内容]  */
                ++(elem->reduceNum);

                vReduceLit.push_back(new RLit{bLit, elem}); //记录下拉文字
                //++uReduceNum;
                isLeftLit = false;
                // cout << "主界线下拉" << endl;
                break;
            } else {
                subst->SubstBacktrackToPos(backpointA);
            }
        }

        if (isLeftLit) {
            //添加到剩余文字标志
            bLit->EqnSetProp(EqnProp::EPIsLeft);
            // leftLit[uLeftLitNum++] = bLit;
        }

    }

    //====遍历主界线(A文字) 确保主界线文字不能相同=====================//   
    vector<ALit_p>::iterator iterALit = vALitTri.begin();
    /*"输出测试 -主界线:" */
    
    {

        //        cout << "输出测试 -主界线:" << endl;
        //        fprintf(stdout, "actLit[C%u_%d]", actLit->claPtr->ident, actLit->pos);
        //        actLit->EqnTSTPPrint(stdout, true);
        //        cout << endl;
        //        while (iterALit != vALitTri.end()) {
        //            Lit_p aLit = (*iterALit)->alit;
        //            fprintf(stdout, "测试[C%u_%d]", aLit->claPtr->ident, aLit->pos);
        //            aLit->EqnTSTPPrint(stdout, true);
        //            cout << endl;
        //            Lit_p bLit = (*iterALit)->blit;
        //            fprintf(stdout, "测试[C%u_%d]", bLit->claPtr->ident, bLit->pos);
        //            bLit->EqnTSTPPrint(stdout, true);
        //            cout << endl;
        //            ++iterALit;
        //        }
        //        cout << "\n------主界线------" << endl;
        //        iterALit = vALitTri.begin();
    }
    
    while (iterALit != vALitTri.end()) {

        Lit_p aLitPtr = (*iterALit)->alit;
        if (actLit->equalsStuct(aLitPtr)) { //当前选择的主动文字与主界线相同时候
            //输出测试--------------
            {
                //                fprintf(stdout, "\n[C%u_%d]", actLit->claPtr->ident, actLit->pos);
                //                actLit->EqnTSTPPrint(stdout, true);
                //                cout << endl;
                //
                //                fprintf(stdout, "\n[C%u_%d]", aLitPtr->claPtr->ident, aLitPtr->pos);
                //                aLitPtr->EqnTSTPPrint(stdout, true);
                //                cout << endl;
            }

            assert(actLit->isSameProps(aLitPtr)); //主界线文字不存在互补情况

            return ResRule::ChgActLit;
        }
        vector<ALit_p>::iterator iterALitB = iterALit + 1;
        while (iterALitB != vALitTri.end()) {
            Lit_p aLitB = (*iterALitB)->alit;
            if (aLitPtr->equalsStuct(aLitB)) {
                assert(aLitPtr->isSameProps(aLitB)); //如果互补上一个检查应该下拉
                return ResRule::ChgActLit;
            }
            ++iterALitB;
        }
        ++iterALit;
    }


    //====遍历剩余文字(恒真,合并) ============================//    
    vector<Lit_p>::iterator iterA = vNewR.begin();
    for (; iterA != vNewR.end(); ++iterA) {
        Lit_p bElem = *iterA;

        //检查主动文字与前面剩余文字相同(直接换主动文字) -- 注意这种相同往往是由于 合一替换导致的
        if (candLit->isComplementProps(bElem) && candLit->equalsStuct(bElem)) {
            return ResRule::ChgActLit;
        }
        /* 主界线文字与前面剩余文字 [不能相同(不做合一)]. [U]这里暂时不考虑factor情况.  */
        //[U]这里可以考虑采用索引方式 进行优化        
        for (ALit* aElem : vALitTri) {
            if (bElem->isComplementProps(aElem->alit) && bElem->equalsStuct(aElem->alit)) {
                //其他主界线文字与前面B文字相同(说明相同是由于替换产生的,换下一个被动归结文字)
                return ResRule::ChgPosLit;
            }
        }
        vector<Lit_p>::iterator iterB = iterA + 1;
        /* 前面剩余文字之间 [恒真(换下一个被动归结文字)].[相同(删除后面的剩余文字)]*/
        while (iterB != vNewR.end()) {
            Lit_p bElemB = *iterB;
            if (bElem->equalsStuct(bElemB)) {
                if (bElem->isComplementProps(bElemB)) {//剩余文字恒真,回退下一个被动归结文字
                    return ResRule::ChgPosLit;
                } else if (bElem->isComplementProps(bElemB)) { //剩余文字相同,删除后面的剩余文字
                    iterB = vNewR.erase(iterB);
                    continue;
                }
            }
            ++iterB;
        }
    }



    bool isDelLit = false;

    //==== 再一次遍历被动归结子句的剩余文字 ======================//
    uLeftLitNum = 0;
    bLit = candCla->Lits();
    for (; bLit != nullptr; bLit = bLit->next) {
        if (!bLit->EqnQueryProp(EqnProp::EPIsLeft)) {
            continue;
        }
        isDelLit = false;
        bLit->EqnDelProp(EqnProp::EPIsLeft);
        assert(bLit != candLit);

        //====检查剩余文字与被归结文字相同(删除)======================//
        if (bLit->isComplementProps(actLit) && bLit->equalsStuct(actLit)) {
            isDelLit = true;
            continue;
        }

        //====检查剩余文字(B文字)与前面剩余文字是否(恒真,相同)======================//
        for (Lit_p bElem : vNewR) {
            //[U]这里暂时不考虑  factor情况.
            // 检查被动子句中的文字 与剩余文字是否 相同(合并),采用完全相同,若采用合一相同,则是不完备的(factor),可以考虑不同策略
            if (bLit->equalsStuct(bElem)) {
                if (bLit->isComplementProps(bElem)) {//谓词符号互补,恒真(换下一个被动归结文字)

                    return ResRule::ChgPosLit;
                }
                assert(bLit->isSameProps(bElem)); //谓词符号相同,合并 
                isDelLit = true;
                break;
            }
        }
        if (isDelLit) continue;
        assert(!isDelLit);
        bLit->EqnSetProp(EqnProp::EPIsLeft);
        leftLit[uLeftLitNum++] = bLit;

    }

    return ResRule::RULEOK;


}

//三角形结果输出

void TriAlg::printTri(Cla_p actCla) {
    //输出主界线文字
    for (ALit_p elem : vALitTri) {
        Lit_p actLit = elem->alit;
        fprintf(stdout, "\n[C%u_%d]", actLit->claPtr->ident, actLit->pos);
        actLit->EqnTSTPPrint(stdout, true);
        Lit_p pasLit = elem->blit;

        fprintf(stdout, "\n[C%u_%d]", pasLit->claPtr->ident, pasLit->pos);
        pasLit->EqnTSTPPrint(stdout, true);

    }

    //输出R    
    uint32_t uSizeR = vNewR.size();
    cout << "\n[R]:";
    if (uSizeR > 0) {
        Lit_p r = vNewR[0];
        fprintf(stdout, "[C%u_%d]", r->claPtr->ident, r->pos);
        r->EqnTSTPPrint(stdout, true);
        for (int i = 1; i < uSizeR; ++i) {
            r = vNewR[i];
            fprintf(stdout, "+[C%u_%d]", r->claPtr->ident, r->pos);
            r->EqnTSTPPrint(stdout, true);
        }
    }
    Lit_p lit = actCla->Lits();
    while (lit) {
        if (lit->EqnQueryProp(EqnProp::EPIsLeft)) {
            fprintf(stdout, " + [C%u_%d]", lit->claPtr->ident, lit->pos);
            lit->EqnTSTPPrint(stdout, true);
        }
        lit = lit->next;
    }

    fprintf(stdout, "\n--------------------\n");

}


/// 给定一个文字gLit 检查单元子句集合中是否有可以与之否合一的子句.
/// \param gLit
/// \return 是否找到可以合一的单元子句

bool TriAlg::unitResolutionBySet(Literal* gLit, int ind) {

    vector<Clause* >&cmpUnitClas = gLit->IsPositive() ? fol->vNegUnitClas : fol->vPosUnitClas;

    for (; ind < cmpUnitClas.size(); ++ind) {
        Clause* candUnitCal = cmpUnitClas[ind];
        if (setUsedCla.find(candUnitCal) != setUsedCla.end()) {
            continue;
        }

        Lit_p candLit = candUnitCal->Lits();
        assert(gLit->isComplementProps(candLit));
        int backpoint = subst->Size();
        bool res = unify.literalMgu(gLit, candLit, subst);

        if (res) {
            //找到可以合一的单文字子句,1.添加单元子句文字到A文字列表中;2.添加到被使用文字中
            setUsedCla.insert(candUnitCal);
            vALitTri.push_back(new ALit{candLit, gLit, 0});



            //输出该对角线文字-------------------------
            {
                //                fprintf(stdout, "单元子句下拉-[C%u_%d]", candLit->claPtr->ident, candLit->pos);
                //                candLit->EqnTSTPPrint(stdout, true);
                //                cout << endl;
                //
                //                fprintf(stdout, "[C%u_%d]", gLit->claPtr->ident, gLit->pos);
                //                gLit->EqnTSTPPrint(stdout, true);
                //                cout << endl;
            }
            //------------------------------------------

            return true;
        }
        subst->SubstBacktrackToPos(backpoint);
    }

    return false;
}
/// 给定一个文字(单元子句),用索引的方式检查是否是冗余的
/// \param gLit

bool TriAlg::unitResolutionByIndex(Literal * gLit) {
    //[U]后面实现
    return false;
}