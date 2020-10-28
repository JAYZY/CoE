/* 
 * File:   Simplification.cpp
 * Author: Zhong Jian<77367632@qq.com>
 * 
 * Created on 2018年2月23日, 下午1:59
 */

#include <cstdlib>

#include "Simplification.h"
#include "CLAUSE/Clause.h"
#include "Indexing/TermIndexing.h"
#include "Unify.h"
#include "CLAUSE/LiteralCompare.h"
#include "INOUT/FileOp.h"
#include "InferenceInfo.h"
//map<TermCell*, int> Simplification::termcmp;

Simplification::Simplification() {
}

Simplification::Simplification(const Simplification& orig) {
}

Simplification::~Simplification() {
}

bool Simplification::isTautology(Clause* cla) {

    for (Literal* litA = cla->literals; litA; litA = litA->next) {
        for (Literal* litB = litA->next; litB; litB = litB->next) {
            if (litA->isComplementProps(litB) && litA->EqualsStuct(litB)) {
                return true;
            }
        }
    }
    return false;
}

 



/// 检查子句genCla 是否存在替换 使得候选子句 C 归入到查询子句 G, 目的检查查询子句G是否冗余
/*  算法流程:  C {L1,L2..Ln} 
 * 1.从C中按照规则选择文字Li 查找indexTree.找到候选文字集{candLits}
 * 2.排除 A.已经比较过的子句;B.候选子句candClas的文字数 > C 的文字数的子句;
 * 
 */
/// \param genCla 查询子句
/// \param indexing 项索引
/// \return 子句genCla是否冗余

bool Simplification::ForwardSubsumption(Clause* genCla, TermIndexing* indexing) {

    Literal* selConLit = genCla->Lits();
    //Literal* selConLit = genCla->FileMaxLit<ImprovementCMP, TermIndexing>(ImprovementCMP(), indexing);
    //Literal* selConLit = genCla->FileMaxLit<OnlyVarLenCMP, TermIndexing>(OnlyVarLenCMP(), indexing);
    //Literal* selConLit = genCla->FileMaxLit<OnlyMaxDepthCMP, TermIndexing>(OnlyMaxDepthCMP(), indexing);
    // Literal* selConLit = genCla->FileMaxLit<StandardWCMP, TermIndexing>(StandardWCMP(), indexing);

    set<Clause*> checkedClas; //存储已经检查过的子句
    while (selConLit) {

#ifdef OUTINFO       //debug print        
        cout << "test-genCla:";
        genCla->ClausePrint(stdout, true);
        cout << endl;
        cout << "选择文字:";
        selConLit->EqnTSTPPrint(stdout, true);
        cout << endl;
#endif

        //等词不做索引树判断
        if (selConLit->EqnIsEquLit()) {
            selConLit = selConLit->next;
            continue;
        }

        //从索引树上获取,候选节点(项)
        TermIndNode* termIndNode = indexing->Subsumption(selConLit, SubsumpType::Forword);

        if (termIndNode == nullptr) {
            indexing->ClearVarLst();
            indexing->subst->SubstBacktrack(); //清除替换
            return false;
        }
        //候选文字
        vector<Literal*>*candVarLits = &((termIndNode)->leafs);
        Clause* candVarCla = nullptr; //找到可能存在归入冗余的候选子句
        Literal* candVarLit = nullptr;
        const uint16_t uLitNum = genCla->LitsNumber();
        //确保所有的匹配节点都能找到.
        while (true) {
            //遍历候选文字集合.查找满足向前归入的文字
            for (int ind = 0; ind < candVarLits->size(); ++ind) {

                candVarLit = candVarLits->at(ind);
                int substPos = indexing->subst->Size();

                candVarCla = candVarLit->claPtr; //找到可能存在归入冗余的候选子句               
                assert(candVarCla);

                //条件 候选子句文字个数 <=查询子句文字个数 && 不能是检查过的子句
                if (uLitNum < candVarCla->LitsNumber() || checkedClas.find(candVarCla) != checkedClas.end()) {
                    continue;
                }
                ++Env::backword_CMP_counter;
                //------ 得到候选子句canVarCla  检查是否存在替换r 使得 canVarCla*r=pasCla + vNewR (排除消除的文字)
                // if (LitListSubsume(candVarCla->Lits(), candVarLit, genCla->Lits(), indexing->subst, nullptr)) {
                if (ClauseSubsumeClause(genCla, candVarCla)) {
                    //找到匹配的冗余子句--说明候选子句中的所有文字均可以通过替换与 剩余文字 匹配.
                    //fprintf(stdout, "\n# [FS]C%u invalid by c%d\n", selConLit->claPtr->ident, candVarCla->GetClaId());
                    string tmpstr = "\n# [FS]R invalid by c" + to_string(candVarCla->GetClaId()) + "\n";
                    //FileOp::getInstance()->outRun(tmpstr);
                    FileOp::getInstance()->outLog("[FS]c" + to_string(genCla->GetClaId()) + " by c" + to_string(candVarCla->GetClaId()) + "\n");
                    ++Env::backword_Finded_counter;
                    indexing->ClearVarLst(); //清除替换
                    return true;
                }


                //没有找到1.添加已经检查过的子句;2.查找下一个候选子句
                checkedClas.insert(candVarCla); //添加已经检查过的子句
            }
            termIndNode = indexing->NextForwordSubsump(); //查找下一个
            if (termIndNode == nullptr) {
                indexing->ClearVarLst();
                return false;
            }
            candVarLits = &((termIndNode)->leafs);
        }
        selConLit = selConLit->next;
    }
    return false;
}

/// 对三角形过程中的剩余文字进行向前归入冗余检查
/// \param pasClaLeftLits 被动剩余文字数组
/// \param uPosLeftLitInd 被动剩余文字个数
/// \param actClaLeftLits 主动剩余文字数组
/// \param uActLeftLitInd 主动剩余文字个数
/// \return 

bool Simplification::ForwardSubsumption(Literal** pasClaLeftLits, uint16_t uPosLeftLitInd, Literal** actClaLeftLits, uint16_t uActLeftLitInd, TermIndexing* indexing) {



    set<Clause*> checkedClas; //存储已经检查过的子句
    uint16_t uLitNum = uPosLeftLitInd + uActLeftLitInd; //总文字个数
    //只检查从被动文字出发的 匹配文字
    for (int pLeftIndA = 0; pLeftIndA < uPosLeftLitInd; ++pLeftIndA) {

        Literal* selConLit = pasClaLeftLits[pLeftIndA];
#ifdef OUTINFO       //debug print        
        cout << "选择文字:";
        selConLit->EqnTSTPPrint(stdout, true);
        cout << endl;
#endif

        //从索引树上获取,候选节点(项)
        TermIndNode* termIndNode = indexing->Subsumption(selConLit, SubsumpType::Forword);
        if (termIndNode == nullptr)
            return false;

        //候选文字
        vector<Literal*>*candVarLits = &((termIndNode)->leafs);
        Clause* candVarCla = nullptr; //找到可能存在归入冗余的候选子句
        Literal* candVarLit = nullptr;

        //确保所有的匹配节点都能找到.
        while (true) {
            //遍历候选文字集合.查找满足向前归入的文字
            for (int ind = 0; ind < candVarLits->size(); ++ind) {

                candVarLit = candVarLits->at(ind);
                candVarCla = candVarLit->claPtr; //找到可能存在归入冗余的候选子句               
                assert(candVarCla);
                //要求满足条件 文字个数 less than 候选子句的文字个数
                if (uLitNum < candVarCla->LitsNumber() || checkedClas.find(candVarCla) == checkedClas.end()) {
                    continue;
                }

                ++Env::backword_CMP_counter;
                Unify unify;

                bool isMatch = false;
                uint32_t iniSubstPos = indexing->subst->Size();
                //遍历候选子句的文字集合.排除已经匹配的文字candVarLit,保证所有文字均可以有匹配的文字
                for (Literal* varEqn = candVarCla->Lits(); varEqn; varEqn = varEqn->next) {

                    if (varEqn == candVarLit) {
                        continue;
                    }

                    isMatch = false;
                    //遍历被动子句剩余文字,检查是否可以匹配(match)
                    for (int pLeftIndB = 0; pLeftIndB < uPosLeftLitInd; ++pLeftIndB) {
                        if (pLeftIndA == pLeftIndB) {
                            continue;
                        }

                        Literal* conEqn = pasClaLeftLits[pLeftIndB];

                        if (!conEqn->isSameProps(varEqn) || conEqn->StandardWeight() < varEqn->StandardWeight()) ////相同正,负属性 || 被归入文字的权重 > 归入文字的权重 
                            continue;

                        //debug print 
                        // cout << "eqn:";      varEqn->EqnTSTPPrint(stdout, true);  cout << endl;
                        // cout << "eqnCand:";  conEqn->EqnTSTPPrint(stdout, true); cout << endl;  

                        uint32_t substPos = indexing->subst->Size();
                        if (unify.SubstComputeMatch(varEqn->lterm, conEqn->lterm, indexing->subst)) {
                            if (unify.SubstComputeMatch(varEqn->rterm, conEqn->rterm, indexing->subst)) {
                                isMatch = true;
                                break;
                            }
                        }
                        indexing->subst->SubstBacktrackToPos(substPos);
                        /*如果为等词,检查如下情况   l1=E(a,b)  l2=E(b,a)  是否为包含关系? */
                        if (!conEqn->EqnIsEquLit()) {
                            continue;
                        }
                        if (unify.SubstComputeMatch(varEqn->rterm, conEqn->lterm, indexing->subst) &&
                                unify.SubstComputeMatch(varEqn->lterm, conEqn->rterm, indexing->subst)) {
                            isMatch = true;
                            break;
                        }
                        indexing->subst->SubstBacktrackToPos(substPos);
                    }

                    //如找到了,则进行下一次文字匹配查找
                    if (isMatch) {
                        continue;
                    }
                    assert(iniSubstPos == indexing->subst->Size());

                    //  subst->SubstBacktrackToPos(iniSubstPos);
                    //遍历主动子句剩余文字,检查是否可以匹配(match)
                    for (int aLeftInd = 0; aLeftInd < uPosLeftLitInd; ++aLeftInd) {

                        Literal* conEqn = actClaLeftLits[aLeftInd]; //获取主动剩余文字

                        if (!conEqn->isSameProps(varEqn) || conEqn->StandardWeight() < varEqn->StandardWeight()) ////相同正,负属性 || 被归入文字的权重 > 归入文字的权重 
                            continue;

                        //debug print 
                        // cout << "eqn:";      varEqn->EqnTSTPPrint(stdout, true);  cout << endl;
                        // cout << "eqnCand:";  conEqn->EqnTSTPPrint(stdout, true); cout << endl;  

                        uint32_t substPos = indexing->subst->Size();

                        if (unify.SubstComputeMatch(varEqn->lterm, conEqn->lterm, indexing->subst)) {
                            if (unify.SubstComputeMatch(varEqn->rterm, conEqn->rterm, indexing->subst)) {
                                isMatch = true;
                                break;
                            }
                        }
                        indexing->subst->SubstBacktrackToPos(substPos);
                        /*如果为等词,检查如下情况   l1=E(a,b)  l2=E(b,a)  是否为包含关系? */
                        if (!conEqn->EqnIsEquLit()) {
                            continue;
                        }

                        if (unify.SubstComputeMatch(varEqn->rterm, conEqn->lterm, indexing->subst) &&
                                unify.SubstComputeMatch(varEqn->lterm, conEqn->rterm, indexing->subst)) {
                            isMatch = true;
                            break;
                        }
                        indexing->subst->SubstBacktrackToPos(substPos);
                    }
                    if (isMatch) {
                        continue;
                    }
                    assert(iniSubstPos == indexing->subst->Size());
                }

                if (isMatch) {//说明候选子句中的所有文字均可以通过替换与 剩余文字 匹配.
                    indexing->subst->SubstBacktrackSingle(); //清除替换
                    //记录冗余
                    fprintf(stdout, "[FS]R invalid by c%d\n", candVarCla->GetClaId());
                    ++Env::backword_Finded_counter;
                    indexing->ClearVarLst(); //清除替换
                    return true;
                }

                //没有找到 indexing->chgVars->SubstBacktrackToPos(substPos);
                assert(iniSubstPos == indexing->subst->Size());
                checkedClas.insert(candVarCla); //添加已经检查过的子句
            }

            termIndNode = indexing->NextForwordSubsump(); //查找下一个
            if (termIndNode == nullptr)
                return false;
            candVarLits = &((termIndNode)->leafs);
        }
    }
    assert(false); //不会执行到这个语句
    return true;
}

bool Simplification::ForwardSubsumUnitCla(Literal* unitLit, TermIndexing* indexing) {

    //从索引树上获取,候选节点(项)
    TermIndNode* termIndNode = indexing->Subsumption(unitLit, SubsumpType::Forword);
    indexing->ClearVarLst(); //清除中间过程的变元以及替换
    if (termIndNode == nullptr)
        return false;
    else
        return true;
}


/// 根据子句genCla查找(indexing tree中)冗余子句,有任意替换r使得 c * r = g ?
/// \param genCla 被检查的子句(生成的子句)
/// \param indexing 已有的索引
/// \return genCla是否归入某个子句(是否被某个子句包含)

bool Simplification::BackWardSubsumption(Clause* genCla, TermIndexing* indexing, set<Clause*>&outDelClas) {


#ifdef New
    //Literal* selLit = genCla->FileMaxLit<XYSteadyCMP,TermIndexing>(XYSteadyCMP(),indexing);
    // Literal* selLit = genCla->FileMaxLit<StandardWCMP, TermIndexing>(StandardWCMP(), indexing);
    // Literal* selLit = genCla->FileMaxLit<ImprovementCMP, TermIndexing>(ImprovementCMP(), indexing);
    Literal* selLit = genCla->FileMaxLit<EqnCompareKBO6, TermIndexing>(EqnCompareKBO6(), indexing);

#else
    Literal* selLit = genCla->Lits();
#endif
    //    if (genCla->GetClaId() == 30) {
    //        cout << "test-genCla:";
    //        genCla->ClausePrint(stdout, true);
    //        cout << endl;
    //        cout << "选择文字:";
    //        selLit->EqnTSTPPrint(stdout, true);
    //        cout << endl;
    //    }
#ifdef OUTINFO

    cout << "选择文字:";
    selLit->EqnTSTPPrint(stdout, true);
    // cout << " zjw:" << selLit->zjlitWight << " EW:" << selLit->StandardWeight() << "  ";
    cout << "termcmp[selLit->lterm]:" << TermIndexing::constTermNum[selLit->lterm] << endl;

#endif
    //#ifdef OUTINFO
    int tmpTest = 0;
    //#endif

    Clause* candCla = nullptr;
    TermIndNode* candTermNode = indexing->Subsumption(selLit, SubsumpType::Backword);
    if (candTermNode == nullptr) {
        indexing->ClearVarLst();

#ifdef OUTINFO
        cout << "比较次数:" << tmpTest << endl;
#endif
        return false;
    }
    vector<Literal*>*candLits = &(candTermNode->leafs);

    Literal* candLit = nullptr;
    Subst* subst = new Subst();
    set<int>cmpedCla;
    while (true) {
        for (int ind = 0; ind < candLits->size(); ++ind) {
            int substPos = subst->Size();
            candLit = candLits->at(ind);
            candCla = candLit->claPtr; //找到第一个文字所匹配的子句 
            if (cmpedCla.find(candCla->GetClaId()) != cmpedCla.end())
                continue;
            cmpedCla.insert(candCla->GetClaId());
            if (outDelClas.find(candCla) == outDelClas.end() && candCla->LitsNumber() >= genCla->LitsNumber()) {
                ++Env::backword_CMP_counter;

                //#ifdef OUTINFO
                ++tmpTest;
                //#endif
                if (ClauseSubsumeClause(candCla, genCla)) {
                    // if (LitListSubsume(genCla->Lits(), nullptr, candCla->Lits(), subst, nullptr)) {
                    //记录找到的冗余子句
                    ++Env::backword_Finded_counter;
                    outDelClas.insert(candCla);

                }
            }
            subst->SubstBacktrackToPos(substPos);
        }
        candTermNode = indexing->NextBackSubsump();
        if (candTermNode == nullptr) {
            break;
        }
        candLits = &(candTermNode->leafs);
        DelPtr(subst);
        subst = new Subst();
    }
    DelPtr(subst);
    //对找到的冗余子句进行处理
    TermIndexing::constTermNum[selLit->lterm] += tmpTest;
    indexing->ClearVarLst();
#ifdef OUTINFO   
    if (outDelClas.empty()) {
        // cout << "No found backsubsumption!" << endl;
    } else {

        cout << "\nfound backsubsumption!,size:" << outDelClas.size() << " " << endl;
        genCla->ClausePrint(stdout, true);
        cout << " ==> ";
        (*outDelClas.begin())->ClausePrint(stdout, true);

    }


    cout << "比较次数:" << tmpTest << endl;
#endif
    return true;
}


/// 检查查询文字列表是否归入候选文字列表(PS:存在替换r,使得subsumVarLst(r) 归入 subsumConLst)
/* sumsumLst 改变,subsumCandLst 变元不改变,*/
/// \param subsumVarLst     查询的文字列表(可变文字)
/// \param subsumConLst     候选的文字列表(不可变文字)
/// \param subst    替换对象
/// \return    subsumVarLst 是否归入 subsumConLst;

bool Simplification::LitListSubsume(Literal* subsumVarLst, Literal* exceptLit, Literal* subsumConLst, Subst_p subst, int8_t* pickLst) {

    if (!subsumVarLst) return true; //查询文字列表没有文字,返回true    

    uint32_t iniSubstPos = subst->Size();
    Unify unify;
    for (Literal* varEqn = subsumVarLst; varEqn; varEqn = varEqn->next) {

        if (varEqn == exceptLit)continue;

        bool res = false;
        for (Literal* conEqn = subsumConLst; conEqn; conEqn = conEqn->next) {
            if (!conEqn->isSameProps(varEqn))
                continue;
            uint32_t substPos = subst->Size();
            if (conEqn->StandardWeight() < varEqn->StandardWeight()) //被归入文字的变元数 > 归入文字的变元数 
                continue;
            if (unify.SubstComputeMatch(varEqn->lterm, conEqn->lterm, subst)) {
                if (unify.SubstComputeMatch(varEqn->rterm, conEqn->rterm, subst)) {
                    res = true;
                    break;
                }
            }
            subst->SubstBacktrackToPos(substPos);
            /*如果为等词,检查如下情况   l1=E(a,b)  l2=E(b,a)  是否为包含关系? */
            if (unify.SubstComputeMatch(varEqn->rterm, conEqn->lterm, subst) &&
                    unify.SubstComputeMatch(varEqn->lterm, conEqn->rterm, subst)) {
                res = true;
                break;
            }
            subst->SubstBacktrackToPos(substPos);
        }
        if (!res) {
            //失败还原所有的替换改变
            subst->SubstBacktrackToPos(iniSubstPos);
            return false;
        }
    }

    return true;
}


/// 检查 subsumerVarcla子句归入 subsumConCla子句
/// \param subsumConCla 被检查子句（冗余？）
/// \param subsumerVarcla
/// \return 

bool Simplification::ClauseSubsumeClause(Clause* subsumConCla, Clause* subsumerVarcla) {
    assert(subsumConCla);
    assert(subsumerVarcla);

    if (subsumConCla->LitsNumber() < subsumerVarcla->LitsNumber())
        return false;
    Unify unify;
    Subst subst;
    vector<Lit_p> stRollBack; //定义rollback堆栈
    vector<uint32_t> stSubstPos;
    Literal* varEqn = subsumerVarcla->literals;


    Literal* conEqn = subsumConCla->literals;
    bool isMatch = false;
    while (true) {

        isMatch = false;
        while (conEqn) {
            isMatch = true;
            //debug               cout << "\nvarEqn ";     varEqn->EqnTSTPPrint(stdout, true);       cout << endl;        

            //debug              cout << "conEqn ";            conEqn->EqnTSTPPrint(stdout, true);            cout << endl;
            if (!conEqn->isSameProps(varEqn) || conEqn->StandardWeight() < varEqn->StandardWeight()) //被归入文字的变元数 > 归入文字的变元数 
                isMatch = false;

            uint32_t substPos = subst.Size();
            if (isMatch) {
                isMatch = false;
                if (unify.SubstComputeMatch(varEqn->lterm, conEqn->lterm, &subst) && unify.SubstComputeMatch(varEqn->rterm, conEqn->rterm, &subst)) {
                    isMatch = true;
                }
            }
            if (!isMatch) {
                //匹配失败
                subst.SubstBacktrackToPos(substPos); //还原替换
                /*如果为等词,检查如下情况   l1=E(a,b)  l2=E(b,a)  是否为包含关系? */
                if (varEqn->EqnIsEquLit() && unify.SubstComputeMatch(varEqn->rterm, conEqn->lterm, &subst)
                        && unify.SubstComputeMatch(varEqn->lterm, conEqn->rterm, &subst)) {
                    isMatch = true;
                }
            }
            //varEqn匹配成功!
            if (isMatch) {
                stRollBack.push_back(varEqn);
                stRollBack.push_back(conEqn->next);
                stSubstPos.push_back(substPos);
                //匹配成功,匹配下一个varEqn | 若没有下一个VarEqn说明全部匹配成功
                varEqn = varEqn->next;
                if (varEqn == nullptr) {
                    break;
                }
                conEqn = subsumConCla->literals;
            }//匹配失败
            else {
                subst.SubstBacktrackToPos(substPos); //还原替换
                conEqn = conEqn->next; //varEqn匹配下一个 conEqn 文字
            }
        }
        //全部匹配成功
        if (isMatch) {
            break;
        }
        //当前VarEqn没有查找到匹配conEqn回退
        if (stRollBack.empty()) {
            break; //没有回退项
        }
        conEqn = stRollBack.back();
        stRollBack.pop_back();
        varEqn = stRollBack.back();
        stRollBack.pop_back();
        subst.SubstBacktrackToPos(stSubstPos.back());
        stSubstPos.pop_back();
    }
    subst.Clear(); //还原所有变元替换项
    return isMatch;
}

bool Simplification::ClauseSubsumeArrayLit(Literal** arrayConLit, uint16_t conLitsSize, Clause* varCal) {
    assert(arrayConLit);
    if (conLitsSize < varCal->LitsNumber()) //子句文字数不能大于文字集合数
        return false;
    Unify unify;
    Subst subst;
    vector<Lit_p> stRollBack; //定义rollback堆栈
    vector<uint32_t> stSubstPos;
    Literal* varEqn = varCal->literals;

    uint32_t ind = 0;

    bool isMatch = false;
    while (true) {

        isMatch = false;
        while (ind < conLitsSize) {
            Literal* conEqn = arrayConLit[ind];
            isMatch = true;
            //debug               cout << "varEqn ";            varEqn->EqnTSTPPrint(stdout, true);            cout << endl;

            //debug                cout << "conEqn ";            conEqn->EqnTSTPPrint(stdout, true);            cout << endl;
            if (!conEqn->isSameProps(varEqn) || conEqn->StandardWeight() < varEqn->StandardWeight()) //被归入文字的变元数 > 归入文字的变元数 
                isMatch = false;

            uint32_t substPos = subst.Size();
            if (isMatch) {

                isMatch = false;
                if (unify.SubstComputeMatch(varEqn->lterm, conEqn->lterm, &subst) && unify.SubstComputeMatch(varEqn->rterm, conEqn->rterm, &subst)) {
                    isMatch = true;
                }
                if (!isMatch && varEqn->EqnIsEquLit()) {
                    //匹配失败
                    subst.SubstBacktrackToPos(substPos); //还原替换
                    /*如果为等词,检查如下情况   l1=E(a,b)  l2=E(b,a)  是否为包含关系? */
                    if (unify.SubstComputeMatch(varEqn->rterm, conEqn->lterm, &subst) && unify.SubstComputeMatch(varEqn->lterm, conEqn->rterm, &subst)) {
                        isMatch = true;
                    }
                }
            }
            //varEqn匹配成功!
            if (isMatch) {
                stRollBack.push_back(varEqn);
                stSubstPos.push_back(++ind);
                stSubstPos.push_back(substPos);
                //匹配成功,匹配下一个varEqn | 若 没有下一个VarEqn说明全部匹配成功
                varEqn = varEqn->next;
                if (varEqn == nullptr) {
                    break;
                }
                ind = 0;
            }//匹配失败
            else {
                subst.SubstBacktrackToPos(substPos); //还原替换++ind
                ++ind;
                //Literal* conEqn = arrayConLit[++ind]; //varEqn匹配下一个 conEqn 文字
            }
        }
        //全部匹配成功
        if (isMatch) {
            break;
        }
        //当前VarEqn没有查找到匹配conEqn回退
        if (stRollBack.empty()) {
            break; //没有回退项
        }

        varEqn = stRollBack.back();
        stRollBack.pop_back();
        subst.SubstBacktrackToPos(stSubstPos.back());
        stSubstPos.pop_back();
        ind = stSubstPos.back();
        stSubstPos.pop_back();
    }
    subst.Clear(); //还原所有变元替换项
    return isMatch;

}

//Implementation of the condensation rule:
//
//  C
//  == if C' is a factor of C, C' subsumes C
//  C'
//

Clause* Simplification::FactorOnce(Clause* cla) {
    bool res = false;
    Unify unify;
    Subst factorSubSt;
    Clause* newCla = nullptr;
    for (Lit_p litA = cla->literals; litA; litA = litA->next) {
        res = false;
        for (Lit_p litB = litA->next; litB; litB = litB->next) {

            if (!litA->isSameProps(litB))
                continue;
            if (unify.literalMgu(litA, litB, &factorSubSt)) {

                //约减成功
                newCla = cla->RenameCopy(litB);


                string sOut = "";
                //输出 .r
                for (Literal*tmpLit = cla->literals; tmpLit; tmpLit = tmpLit->next) {
                    tmpLit->GetLitInfoWithSelf(sOut);
                    sOut += "\n";
                }
                sOut += "R[" + to_string(newCla->ident) + "]";
                Literal*tmpLit = newCla->literals;
                tmpLit->GetLitInfoWithParent(sOut);
                tmpLit = tmpLit->next;
                for (; tmpLit; tmpLit = tmpLit->next) {
                    sOut += "+";
                    tmpLit->GetLitInfoWithParent(sOut);
                }
                factorSubSt.SubstBacktrack();
                //检查包含冗余                if(cla->ident==1257)                    cout<<"DEbug";
                if (ClauseSubsumeClause(cla, newCla)) {

                    //删除相同文字                
                    int iRemoveNum = Literal::EqnListRemoveDuplicates(newCla->literals);
                    newCla->RecomputeLitCounts();
                    if (iRemoveNum > 0)
                        FileOp::getInstance()->outLog("cla_" + to_string(cla->ident) + " remove same literals:" + to_string(iRemoveNum) + "\n");

                    res = true;
                    //添加推理类型
                    newCla->infereType = InfereType::FACTOR;
                    newCla->parentIds.insert(cla->ident);
                    FileOp::getInstance()->outLog("[S_FAT]:Factor C" + to_string(newCla->ident) + " From C" + to_string(cla->ident) + "\n");

                    //输出 .r  
                    FileOp::getInstance()->outRun(sOut);
                    //输出 .i
                    sOut = "";
                    newCla->getStrOfClause(sOut, false);
                    //输出 .tp
                    //FileOp::getInstance()->OutTPTP(sOut+" ).\n");
                    sOut += ",inference(" + InferenceInfo::getStrInfoType(newCla->infereType) + ",[status(thm)],[c" + to_string(cla->ident) + "]) ).\n";
                    FileOp::getInstance()->outInfo(sOut);



                    //暂时不删除原始子句 -- 由调用来控制是否删除
                    //DelPtr(cla);
                    break;

                } else {
                    res = false;
                    //删除新子句
                    DelPtr(newCla);
                    --Env::global_clause_counter;
                }


            } else
                factorSubSt.SubstBacktrack();
            assert(false == res);


        }
        if (res)
            break;
    }
    return newCla;
}

Clause* Simplification::Factor(Clause* cla) {

    Clause* rtnCla = nullptr;
    //debug        if (cla->ident == 10575)        cout << endl;

    //CondensationAttempts++;
    if ((cla->negLitNo > 1) || (cla->posLitNo > 1)) {
        // clause->weight = ClauseStandardWeight(clause);

        while (cla = FactorOnce(cla)) {
            rtnCla = cla;
        }
        //        if (res) {
        //            CondensationSuccesses++;
        //            DocClauseModificationDefault(clause, inf_condense, NULL);
        //            ClausePushDerivation(clause, DCCondense, NULL, NULL);
        //        }
    }
    return rtnCla;
}

Clause* Simplification::FactorOnce(Clause* actCla, vector<Literal*> vR) {
    bool res = false;
    Unify unify;
    Subst factorSubSt;
    Clause* newCla = nullptr;
    for (Literal* givenLitP = actCla->literals; givenLitP; givenLitP = givenLitP->next) {
        if (!givenLitP->IsHold())continue;
        res = false;
        for (Literal*litR : vR) {
            if ((!givenLitP->isSameProps(litR)) || (!givenLitP->EqnIsEquLit() && givenLitP->lterm->fCode != litR->lterm->fCode))
                continue;
            if (unify.literalMgu(givenLitP, litR, &factorSubSt)) {
                //约减成功
                // newCla = cla->RenameCopy(litR);
            }
        }
    }
    return newCla;

}