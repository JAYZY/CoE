/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Formula.cpp
 * Author: Zhong Jian<77367632@qq.com>
 * 
 * Created on 2017年12月22日, 下午6:03
 */

#include <bits/stdint-uintn.h>
#include <vector>

#include "Formula.h"
#include "Global/Environment.h"
#include "Indexing/TermIndexing.h"
#include "Inferences/Unify.h"
#include "Inferences/Simplification.h"
//谓词索引------

Formula::Formula() {
    this->in = Env::getIn();
    this->origalClaSet = new ClauseSet();
    this->processedClaSet = new ClauseSet();
    this->uEquLitNum = 0;
    this->uNonHornClaNum = 0;
    this->allTermIndex = nullptr;

}

Formula::Formula(const Formula& orig) {

}

Formula::~Formula() {
    DelPtr(this->allTermIndex);
    DelPtr(this->origalClaSet);
    DelPtr(this->processedClaSet);

}

void Formula::generateFormula(Scanner* in) {
    assert(origalClaSet);
    switch (in->format) {
        case IOFormat::LOPFormat:
            /* LOP does not at the moment support full FOF */
            // res = ClauseSetParseList(in, cset, terms);
            Out::Error("No Support lop format!", ErrorCodes::INPUT_SEMANTIC_ERROR);
            break;
        default:
            while (in->TestInpId("input_formula|input_clause|fof|cnf|include")) {
                if (in->TestInpId("include")) {//读取include 文件
                    Out::Error("暂时不识别include 文件的读取", ErrorCodes::INPUT_SEMANTIC_ERROR);
                } else {
                    if (in->TestInpId("input_formula|fof")) {
                        Out::Error("现在暂时不能识别 FOF公式集!\n请先用响应工具进行转换;", ErrorCodes::INPUT_SEMANTIC_ERROR);
                    } else {
                        assert(in->TestInpId("input_clause|cnf"));
                        Clause* clause = new Clause();
                        clause->ClauseParse(in);
                        origalClaSet->InsertCla(clause); //插入原始子句集
                    }
                }
            }
            break;
    }
}

void Formula::preProcess() {
    /*Statistics--    */
    uint32_t uFSNum = 0; //向前归入冗余子句个数
    this->uEquLitNum = 0; //等词文字个数
    this->uNonHornClaNum = 0; //非Horn 子句个数(最多一个非负文字)
    // ClauseSet* pClaSet = new ClauseSet();

    double initial_time = CPUTime();
    this->initIndex();
    this->origalClaSet->SortByLitNumAsc(); //这里确保了所有的原始子句按照 文字个数进行排序

    list<Clause*>*claLst = this->origalClaSet->getClaSet();
    for (auto claIt = claLst->cbegin(); claIt != claLst->cend(); ++claIt) {

        Clause* selCla = *claIt;

        if (Simplification::ForwardSubsumption(selCla, allTermIndex)) {

            ++uFSNum;
            (*claIt)->ClauseSetProp(ClauseProp::CPDeleteClause); //标注子句被删除
            continue;

        }

        //插入到索引中    1.单元子句索引    2.全局索引

        insertNewCla(selCla);
    }
    //输出子句集预处理的信息---------------------------------------------------
    PaseTime("Preprocess_", initial_time);
    fprintf(stdout, "%12s", "# =====Preprocess Information===========#\n");
    fprintf(stdout, "# ForwardSubsump          %12u #\n", uFSNum);
    this->printOrigalClasInfo(stdout);
    fprintf(stdout, "%12s", "# ======================================#\n");
}

/// 给定两个剩余文字集合,检查它们组成子句后是否是冗余
/// \param pasClaLeftLits
/// \param uPosLeftLitInd
/// \param actClaLeftLits
/// \param uActLeftLitInd
/// \return 冗余(无效) 返回 true;  有效返回 false;

bool Formula::leftLitsIsRundacy(Literal** pasClaLeftLits, uint16_t uPasLeftLitInd, vector<Literal*>&vNewR) {

    uint16_t uLitNum = uPasLeftLitInd + vNewR.size(); //总文字个数
    if (uLitNum == 0) return false; //不是冗余的


    set<Clause*> checkedClas; //存储已经检查过的子句

    //只检查从被动文字出发的 匹配文字
    for (int pLeftIndA = 0; pLeftIndA < uPasLeftLitInd; ++pLeftIndA) {

        Literal* selConLit = pasClaLeftLits[pLeftIndA];
#ifdef OUTINFO       //debug print        
        cout << "选择文字:" << selConLit->claPtr->ident << endl;
        selConLit->EqnTSTPPrint(stdout, true);
        cout << endl;
#endif
        TermIndexing* indexing = (uLitNum == 1) ? this->unitClaIndex : this->allTermIndex;
        //从索引树上获取,候选节点(项)
        TermIndNode* termIndNode = indexing->Subsumption(selConLit, SubsumpType::Forword);
        if (termIndNode == nullptr){
            indexing->ClearVarLst();
            return false;
        }

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
                if (uLitNum > candVarCla->LitsNumber() || checkedClas.find(candVarCla) == checkedClas.end()) {
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
                    for (int pLeftIndB = 0; pLeftIndB < uPasLeftLitInd; ++pLeftIndB) {
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
                    //遍历现有的剩余文字,检查是否可以匹配(match)
                    // for (int aLeftInd = 0; aLeftInd < uPosLeftLitInd; ++aLeftInd) {
                    for (Literal* conEqn : vNewR) {
                        //  Literal* conEqn = actClaLeftLits[aLeftInd]; //获取主动剩余文字
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

                if (isMatch) {
                    //说明候选子句中的所有文字均可以通过替换与 剩余文字 匹配.
                   
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
            {
                indexing->ClearVarLst(); //清除替换
                return false;
                
            }
            candVarLits = &((termIndNode)->leafs);
        }
    }

    assert(false); //不会执行到这个语句
    return true;
}
/// 检查单元剩余文字是否为向前归入冗余
/// \param unitConLit  单元文字,注意没做变元更名,确保1,该文字来自于非单元文字,2只在单元索引上做冗余检查.
/// \return 冗余 --true  无冗余 -- false;

bool Formula::LeftUnitLitsIsRundacy(Literal* unitConLit) {
    if (unitConLit == nullptr) {
        return false;
    }
    //从索引树上获取,候选节点(项)
    TermIndNode* termIndNode = this->unitClaIndex->Subsumption(unitConLit, SubsumpType::Forword);
    if (termIndNode == nullptr)
        return false;
    assert( ( (termIndNode)->leafs).size()> 0);
    return true;
}

//将子句添加到公式集中

void Formula::insertNewCla(Cla_p cla) {
    //插入到索引中    1.单元子句索引    2.全局索引
    Literal * lit = cla->Lits();
    //添加到单元子句列表中
    if (cla->IsUnitPos()) {
        this->unitClaIndex->Insert(lit);
        this->vPosUnitClas.push_back(cla);
    }
    if (cla->isUnitNeg()) {
        this->unitClaIndex->Insert(lit);
        this->vNegUnitClas.push_back(cla);
    }
    uint32_t posLitNum = 0;
    while (lit) {
        if (lit->EqnIsEquLit()) {
            ++(this->uEquLitNum);
        }
        if (lit->IsPositive()) {
            ++posLitNum;
        }
        allTermIndex->Insert(lit);
        lit = lit->next;
    }
    if (posLitNum > 1) {
        ++(this->uNonHornClaNum);
    } else if (posLitNum == 0) {//目标子句
        this->addGoalClas(cla);
    }
    //添加到公式集 谓词全局列表中(注意单文字子句不加入谓词列表) 
    if (cla->LitsNumber() > 1)
        this->AddPredLst(cla);
    //添加到处理后的子句集中
    this->processedClaSet->InsertCla(cla);
}


//添加谓词符号到全局列表中

void Formula::AddPredLst(Clause* cla) {
    Literal* lit = cla->Lits();
    while (lit) {
        if (lit->IsPositive()) {//正文字
            if (lit->EqnIsEquLit()) {
                if (!lit->IsOriented()) {
                    lit->EqnOrient();
                }//确保左边项>右边项
                //解决 a==b  b==c  ==> a==c
                if (g_PostEqn[lit->lterm].find(lit->rterm) == g_PostEqn[lit->lterm].end()) {
                    g_PostEqn[lit->lterm].insert(lit->rterm);
                    if (g_PostEqn.find(lit->rterm) != g_PostEqn.end()) {
                        g_PostEqn[lit->lterm].insert(g_PostEqn[lit->rterm].begin(), g_PostEqn[lit->rterm].end());
                    }
                }
            } else {
                g_PostPred[lit->lterm->fCode].push_back(lit);
            }
        } else {
            if (lit->EqnIsEquLit()) {
                if (!lit->IsOriented()) {
                    lit->EqnOrient();
                }
                //确保左边项>右边项
                //解决 a==b  b==c  ==> a==c
                if (g_NegEqn[lit->lterm].find(lit->rterm) == g_NegEqn[lit->lterm].end()) {
                    g_NegEqn[lit->lterm].insert(lit->rterm);
                    if (g_NegEqn.find(lit->rterm) != g_NegEqn.end()) {
                        g_NegEqn[lit->lterm].insert(g_NegEqn[lit->rterm].begin(), g_NegEqn[lit->rterm].end());
                    }
                }
            } else {
                g_NegPred[lit->lterm->fCode].push_back(lit);
            }
        }
        lit = lit->next;
    }
}

/*得到互补谓词候选文字集合*/
vector<Literal*>* Formula::getPairPredLst(Literal* lit) {
    assert(!lit->EqnIsEquLit()); //不能是等词文字
    if (lit->IsPositive())
        return &g_NegPred[lit->lterm->fCode];
    else
        return &g_PostPred[lit->lterm->fCode];
}

vector<Literal*>* Formula::getPredLst(Literal* lit) {
    assert(!lit->EqnIsEquLit()); //不能是等词文字
    if (lit->IsPositive())
        return &g_PostPred[lit->lterm->fCode];
    else
        return &g_NegPred[lit->lterm->fCode];
}

void Formula::printOrigalClasInfo(FILE* out) {
    fprintf(out, "# Total_Number_Formual    %12ld #\n", this->origalClaSet->Members());
    fprintf(out, "# Goal_Clause_Number      %12ld #\n", this->goalClaset.size());
    fprintf(out, "# Horn_Clase_Number       %12u #\n", uNonHornClaNum);
    fprintf(out, "# Horn_Clase              %12s #\n", uNonHornClaNum > 0 ? "FALSE" : "TRUE");
    fprintf(out, "# Has_Equality            %12s #\n", 0 == this->uEquLitNum ? "FALSE" : "TRUE");
}