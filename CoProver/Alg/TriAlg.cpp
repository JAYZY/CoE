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
    int triNum = 0;
    clearVect();
    RESULT resTri = RESULT::NOMGU;

    usedCla.insert(givenCla); //记录 子句是否被使用    
    subst = new Subst();
    uReduceNum = 0;

    fol->unitClasSort(); //遵循 稳定度高的 VS 稳定度高的

    givenCla->SortLits(); //子句中文字的排序策略 :建议 先负后正,先稳定低后稳定高  
    Lit_p gLit = givenCla->Lits(); //选择一个文字 


    //经过多次合一，构建三角形

    Lit_p *pasClaLeftLits = nullptr;
    Lit_p *actClaLeftLits = new Lit_p[givenCla->LitsNumber()]; //主动子句的剩余文字
    memset(actClaLeftLits, 0, sizeof (Lit_p) * (givenCla->LitsNumber() - 1));

    uint16_t uActLeftLitNum = 0; //记录主动归结子句的剩余文字个数
    uint16_t uPasLeftLitNum = 0; //记录被动归结子句的剩余文字个数

    Lit_p actLit = nullptr;

    //对选择的子句 进行 单文字匹配
    while (gLit) {
        gLit->EqnDelProp(EqnProp::EPIsDel);
        if (!unitResolutionBySet(gLit)) {
            if (actLit == nullptr) {
                actLit = gLit;
            } else {

                actClaLeftLits[uActLeftLitNum++] = gLit; //添加到剩余文字集中
            }
        }
        gLit = gLit->next;
    }

    if (0 == uActLeftLitNum && actLit == nullptr)
        return RESULT::UNSAT;
    /************************************************************************/
    /*主动文字选择原则 ,1 尽量选择 负文字 2 尽量选择稳定度低的文字	
    /************************************************************************/


    Lit_p pasLit = nullptr; //被动归结文字
    vector<Literal*>* vCandLit; //候选文字集合
    vector<uint32_t> vRecodeBackPoint; //每次成功一个配对 就记录一次 替换的回退点

    vRecodeBackPoint.reserve(32);
    vRecodeBackPoint.push_back(0);
    vector<uint32_t> vPasCandBackPoint;
    vPasCandBackPoint.reserve(32);

    int32_t actLitsInd = -1;

    uint32_t pasLitInd = 0;
    Lit_p rollBackAvoidLit = nullptr; //回退要避免的候选文字
    while (1) {
        //=======遍历主动子句中剩余文字 -----------------------------------------------------
        while (true) {

            //根据文字互补谓词项对应的互补文字得到候选被动文字
            vCandLit = fol->getPairPredLst(actLit);

            /* 对候选被动文字进行排序 A.被动文字所在子句文字数从少到多;B.相同文字数情况下考虑稳定度,主动被动相近度 等启发式策略[u]如何高效的排序?*/
            if (resTri != RESULT::RollBack)
                stable_sort(vCandLit->begin(), vCandLit->end(), SortRule::PoslitCmp);


            //           测试,输出候选子句集
            //            for (int i = 0; i < vCandLit->size(); i++) {
            //                Literal* elem = vCandLit->at(i);
            //                cout << elem->claPtr->ident << " ";
            //                elem->EqnTSTPPrint(stdout, true);
            //                cout << endl;
            //            }


            //遍历候选被动文字子句顺序进行匹配查找 -----------------------------------------------------
            for (; pasLitInd < vCandLit->size(); ++pasLitInd) {

                resTri = RESULT::NOMGU;

                pasLit = vCandLit->at(pasLitInd); //候选被动归结文字
                if (pasLit == rollBackAvoidLit)
                    continue;
                //==========候选文字的条件限制==================
                {
                    /*同一子句中文字不进行比较;归结过的子句不在归结;文字条件限制*/
                    if (pasLit->claPtr == actLit->claPtr || usedCla.find(pasLit->claPtr) != usedCla.end())
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
                int backpoint = subst->Size();
                bool res = unify.literalMgu(actLit, pasLit, subst);
                if (!res) {
                    subst->SubstBacktrackToPos(backpoint);
                }


                if (pasClaLeftLits)
                    DelArrayPtr(pasClaLeftLits);
                pasClaLeftLits = new Lit_p[pasLit->claPtr->LitsNumber() - 1]; //被动子句的剩余文字

                memset(pasClaLeftLits, 0, sizeof (Lit_p) * (pasLit->claPtr->LitsNumber() - 1));
                uPasLeftLitNum = 0;

                //规则检查
                ResRule resRule = RuleCheck(pasLit, pasClaLeftLits, uPasLeftLitNum);

                if (resRule == ResRule::ChgActLit) {//换主界线文字
                    subst->SubstBacktrackToPos(backpoint);
                    break;
                } else if (resRule == ResRule::ChgPosLit) {//换被归结文字
                    subst->SubstBacktrackToPos(backpoint);
                    continue;
                }
                if (++triNum == 5)
                    cout << "Test" << endl;
                //检查剩余文字 是否是无效的(FS:向前归入冗余/恒真) 无效返回 true 则改变被归结文字                
                //                if (fol->leftLitsIsRundacy(pasClaLeftLits, uPasLeftLitInd, vNewR)) {
                //                    //actClaLeftLits, uActLeftLitInd)) {
                //                    subst->SubstBacktrackToPos(backpoint);
                //                    continue;
                //                }
                //[U] 可以考虑 将剩余的单文字 添加到 子句集中.---------

                //-------------------------------------

                //一次三角形构建成功
                resTri = RESULT::SUCCES;
                break;

            }

            if (resTri == RESULT::SUCCES)
                break;

            if (++actLitsInd >= uActLeftLitNum) {
                break;
            }
            actLit = actClaLeftLits[actLitsInd ];
            //actLit = actClaLeftLits[actLitsInd + 1];
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

        if (resTri == RESULT::NOMGU) { //说明遍历所有主动文字均没有找到可以延拓的文字.三角形开始回退

            /*回退的类型:1.主对角线重新查找下一个互补文字(改变被归结文字);2.重新选择主界线文字(改变主动归结文字);3.重新选择被下拉的主界线文字(改变下拉替换)
             回退分析: 在正常△构建中,若某个主动文字均找不到可以延拓的文字,则说明,类型1(改变被归结文字)不适用. 
             * 此时考虑两种情况 检查是否有下拉,若有下拉,则重新下拉并且还是重该主界线文字出发
             */
            //                if (vReduceLitPtr.back().rLit->claPtr == actLit->claPtr) { //表示有下拉 --有点复杂,后面优化
            //                }
            //第二种回退类型:改变主动归结文字
            Literal* backALit = vALitTri.back()->alit;
            vALitTri.pop_back();
            Clause* backCla = backALit->claPtr;
            DelArrayPtr(actClaLeftLits);
            //2.回退剩余文字 并产生 actLit和actLefts
            uint32_t uArraySize = backCla->LitsNumber() - 1;
            actClaLeftLits = new Lit_p[uArraySize];
            memset(actClaLeftLits, 0, sizeof (Lit_p) * (uArraySize));
            uPasLeftLitNum = 0;
            int actLitsNewInd = -1;

            Literal* backRLit = nullptr;
            actLit = nullptr;
            while ((!vNewR.empty()) &&(backRLit = vNewR.back())->claPtr == backCla) {
                vNewR.pop_back();
                if (backRLit->pos > backALit->pos) {
                    actClaLeftLits[++actLitsNewInd] = backALit;
                    actLit = backRLit;
                }
                actClaLeftLits[++actLitsNewInd] = backRLit;
            }

            //回退 
            uint32_t backPoint = vRecodeBackPoint.back();
            vRecodeBackPoint.pop_back();
            subst->SubstBacktrackToPos(backPoint);

            if (nullptr != actLit) {//回退成功
                actLitsInd = actLitsNewInd;
                resTri = RESULT::RollBack;

            } else {//开始第三种回退 -- 改变被动归结文字(重新选择候选文字)
                actLit = backALit;
                pasLitInd = vPasCandBackPoint.back() + 1;
                vPasCandBackPoint.pop_back();
                continue;
            }
            //return RESULT::SUCCES;

        }
        if (RESULT::RollBack == resTri) {
            continue;
        }

        assert(resTri == RESULT::SUCCES);

        //判断△结果,并继续△延拓.

        //1.添加主界线文字
        vALitTri.push_back(new ALit{actLit, 0});

        //2.添加替换栈回退点        
        vRecodeBackPoint.push_back(subst->Size());
        vPasCandBackPoint.push_back(pasLitInd);
        //3.将主动文字的剩余文字添加到 vNewR中
        for (int i = 0; i < uActLeftLitNum; ++i) {/*注:检查恒真冗余,这种情况只发生在起步子句,因为后续子句 均已经检查了 整个剩余文字是否冗余;而对于起步子句,对起步子句的检查应该在三角形开始前完成*/
            Literal* lit = actClaLeftLits[i];
            if (actLit == lit) {
                continue;
            }
            vNewR.push_back(lit);
        }

        //输出三角形
        {

            fprintf(stdout, "[C%u_%d]", actLit->claPtr->ident, actLit->pos);
            actLit->EqnTSTPPrint(stdout, true);
            cout << endl;
            if (pasLit != nullptr) {
                fprintf(stdout, "[C%u_%d]", pasLit->claPtr->ident, pasLit->pos);
                pasLit->EqnTSTPPrint(stdout, true);
                fprintf(stdout, "\n--------------------\n");
            }
            //判断是否UNSAT
            if (vNewR.empty()&&0 == uPasLeftLitNum) {
                fprintf(stdout, "[R]:空子句");
                return RESULT::UNSAT;
            }

            //输出R(剩余文字)
            // for (int i = 0; i < uActLeftLitInd; ++i) {
            for (Literal* rLit : vNewR) {
                fprintf(stdout, "[R]:{C%u_%d}", rLit->claPtr->ident, rLit->pos);
                rLit->EqnTSTPPrint(stdout, true);
                cout << " + ";
            }
            for (int i = 0; i < uPasLeftLitNum; ++i) {
                fprintf(stdout, " {C%u_%d}", pasClaLeftLits[i]->claPtr->ident, pasClaLeftLits[i]->pos);
                pasClaLeftLits[i]->EqnTSTPPrint(stdout, true);
                cout << " + ";
            }
            fprintf(stdout, "\n--------------------\n");

        }


        //3.交换主动被动子句,进行△延拓
        DelArrayPtr(actClaLeftLits);
        actClaLeftLits = pasClaLeftLits;
        pasClaLeftLits = nullptr;
        uActLeftLitNum = uPasLeftLitNum;
        uPasLeftLitNum = 0;
        actLitsInd = -1;
        pasLitInd = 0;
        actLit = actClaLeftLits[++actLitsInd];

    }
    return RESULT::SUCCES;
}

/// 规则检查
/// \param actLit
/// \param candLit
/// \return 

ResRule TriAlg::RuleCheck(Literal* candLit, Literal* *leftLitCand, uint16_t& uLeftLitInd) {
    /*规则检查 ，
     * 1.主界线文字不能合一互补(下拉)
     * 2.主界线文字不能与剩余文字相同[不]
     * 对被动归结子句中剩余文字1、对角线互补；2、第一条对角线元素是否和该轮前面保留的R相等
     * C、对角线文字不能完全相同（可以是合一后相同，如 P(x)和P(a)）
     * D、对角线文字不能互补（包括合一后互补如 P(x)和~P(a),注：这一条实际是下拉文字）
     * E、对角线文字不能与前面剩余文字相同。
     * F、对角线文字不能被单元子句合一后互补。 单元子句放右边                 * 
     */
    Unify unify;
    Cla_p candCla = candLit->claPtr;
    assert(leftLitCand != nullptr);
    //Lit_p leftLitCand = new Literal(); //候选文字的剩余文字链


    /* 主界线文字与前面剩余文字不能相同(没有合一). [U]这里暂时不考虑factor情况.  */
    uint32_t uRSize = vNewR.size();
    for (int i = 0; i < uRSize; i++) {
        Lit_p bElem = vNewR[i];

        //检查主动文字与前面剩余文字相同(直接换主动文字)
        if (candLit->isComplementProps(bElem) && candLit->equalsStuct(bElem)) {
            return ResRule::ChgActLit;
        }

        //[U]这里可以考虑采用索引方式 进行优化        
        for (ALit* aElem : vALitTri) {
            if (bElem->isComplementProps(aElem->alit) && bElem->equalsStuct(aElem->alit)) {
                //其他主界线文字与前面B文字相同(说明相同是由于替换产生的,换下一个被动归结文字)
                return ResRule::ChgPosLit;
            }
        }
    }

    //====遍历被动子句的所有剩余文字======================//
    for (Lit_p bLit = candCla->Lits(); bLit != nullptr; bLit = bLit->next) {
        bool isLeftLit = true;

        //====检查剩余文字(B文字)======================//
        if (bLit != candLit) {
            for (Lit_p bElem : vNewR) {
                /*
                 * [U]这里暂时不考虑  factor情况.
                 * 检查被动子句中的文字 与剩余文字是否 相同(合并),采用完全相同,若采用合一相同,则是不完备的(factor),可以考虑不同策略
                 */
                if (bLit->equalsStuct(bElem)) {
                    if (bLit->isComplementProps(bElem)) {//谓词符号互补,恒真(换下一个被动归结文字)
                        return ResRule::ChgPosLit;
                    }
                    assert(bLit->isSameProps(bElem)); //谓词符号相同,合并 
                    isLeftLit = false;
                    break;
                }
            }
        }
        if (!isLeftLit) continue;

        //====检查主界线(A文字) 1.确保主界线文字不能合一相同;2.主界线文字下拉(合一)剩余文字;3.用单元集子句下拉剩余文字=====================//
        //====[11.28]修改:检查主界线(A文字) 1.允许主界线文字合一相同,但不能完全相同;2.主界线文字下拉(合一)剩余文字;3.用单元集子句下拉剩余文字=====================//
        for (ALit* elem : vALitTri) {
            Lit_p aLit = elem->alit;

            if (bLit == candLit && StrategyParam::ALIT_LIMIT != ALimit::NoUnifySameA) {//[11.28]1.不检查主界线文字合一相同情况,
                continue;
            }
            int backpointA = subst->Size();
            if (bLit->isComplementProps(aLit) && unify.literalMgu(bLit, aLit, subst)) {
                if (bLit == candLit) {

                    //1.确保主界线文字不能合一相同 [U]注意这里是互补相同,后续可以测试下不是互补相同会怎样?
                    //若为被动归结文字,则说明选择作为主动子句中选择作为主界线的文字与前面主界线文字相同. 
                    subst->SubstBacktrackToPos(backpointA);
                    return ResRule::ChgActLit; //主界线文字互补相同(换下一个主界线文字)
                }
                /* 2.主界线文字下拉(合一)剩余文字.并记录下拉次数[要保留合一替换的内容]  */
                ++(elem->reduceNum);
                ++uReduceNum;
                isLeftLit = false;
                break;
            } else {
                subst->SubstBacktrackToPos(backpointA);
            }
        }
        if (!isLeftLit) continue;
        /* 3.用单元集子句下拉剩余文字*/
        if (unitResolutionBySet(bLit))continue;
        assert(isLeftLit);
        if (bLit == candLit)

            continue;
        //添加到剩余文字集中
        leftLitCand[uLeftLitInd++] = bLit;


    }
    return ResRule::RULEOK;


    //处理被动归结子句中的剩余文字
    //1、检查与queryLit合一相同（换下一个可归结文字）[factor]
    //2、与vOutTir中对角线文字是否与归结文字合一相同（换下一个可归结文字）[确保主界线文字不相同]
    //3、检查与vnewR 
    //		3.1 是否 合一互补（恒真换下一个文字）
    //		3.2 是否 合一相等（合并）
    //4、函数嵌套是否超过限制(换下一个可归结文字)			注：起步子句剩余文字也需要检查
    //5、基文字检查是否与单文子包含冗余(换下一个可归结文字)	注：起步子句剩余文字也需要检查，P(x)->P(b)
}

void TriAlg::printTri() {

}
/// 给定一个文字gLit 检查单元子句集合中是否有可以与之否合一的子句.
/// \param gLit
/// \return 是否找到可以合一的单元子句

bool TriAlg::unitResolutionBySet(Literal* gLit, int ind) {

    vector<Clause* >&cmpUnitClas = gLit->IsPositive() ? fol->vNegUnitClas : fol->vPosUnitClas;

    for (; ind < cmpUnitClas.size(); ++ind) {
        Clause* candUnitCal = cmpUnitClas[ind];
        if (usedCla.find(candUnitCal) != usedCla.end()) {
            continue;
        }

        Lit_p candLit = candUnitCal->Lits();
        assert(gLit->isComplementProps(candLit));
        int backpoint = subst->Size();
        bool res = unify.literalMgu(gLit, candLit, subst);

        if (res) {
            //找到可以合一的单文字子句,1.添加单元子句文字到A文字列表中;2.添加到被使用文字中
            usedCla.insert(candUnitCal);
            vALitTri.push_back(new ALit{candLit, 0});
            vReduceLitPtr.push_back(new RLit{gLit, ind});

            //输出该对角线文字-------------------------
            {
                fprintf(stdout, "[C%u_%d]", candLit->claPtr->ident, candLit->pos);
                candLit->EqnTSTPPrint(stdout, true);
                cout << endl;

                fprintf(stdout, "[C%u_%d]", gLit->claPtr->ident, gLit->pos);
                gLit->EqnTSTPPrint(stdout, true);
                cout << endl;
            }
            //------------------------------------------

            gLit->EqnSetProp(EqnProp::EPIsDel);

            return true;
        }
        subst->SubstBacktrackToPos(backpoint);
    }
    return false;
}
/// 给定一个文字(单元子句),用索引的方式检查是否是冗余的
/// \param gLit

bool TriAlg::unitResolutionByIndex(Literal* gLit) {
    //[U]后面实现
    return false;
}