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

//#include <bits/stdint-uintn.h>
#include <vector>
#include "Formula.h"
#include "Global/Environment.h"
#include "Indexing/TermIndexing.h"
#include "Inferences/Unify.h"
#include "Inferences/Simplification.h"
#include "HEURISTICS/SortRule.h"
//谓词索引------

Formula::Formula() {
    this->in = Env::getIn();
    this->origalClaSet = new ClauseSet();
    this->workClaSet = new ClauseSet();
    this->allTermIndex = nullptr;
    this->unitClaIndex = nullptr;

    iniFolInfo();


}

Formula::Formula(const Formula& orig) {

}

Formula::~Formula() {
    DelPtr(this->allTermIndex);

    DelPtr(this->workClaSet);

    DelPtr(unitClaIndex);


    //DelPtr(this->origalClaSet);
}
// <editor-fold defaultstate="collapsed" desc="等词公理添加">

//前3条固定加入：
//(1) Reflexivity: X1=X1
//(2) Symmetry: X1~=X2 | X2 =X1
//(3) Transitivity: X1~=X2 | X2 ~=X3 | X1 =X3

void Formula::GenerateEqulitAxiom() {
    //前3条固定加入：
    //(1) Reflexivity: X1=X1
    //(2) Symmetry: X1~=X2 | X2 =X1
    //(3) Transitivity: X1~=X2 | X2 ~=X3 | X1 =X3

    //======================================================================/
    //(1) Reflexivity: X1=X1
    Clause* claReflex = new Clause();
    claReflex->ClauseSetProp(ClauseProp::CPTypeAxiom);
    TermCell* t = claReflex->GetClaTB()->VarInert("A", claReflex->ident);
    Lit_p reflexLit = new Literal(t, t, true); //Reflexivity: X1=X1
    reflexLit->EqnSetProp(EqnProp::EPIsEquLiteral);
    reflexLit->EqnSetProp(EqnProp::EPIsPositive);

    claReflex->bindingLits(reflexLit);
    claReflex->ClauseSetProp(ClauseProp::CPTypeAxiom);
    claReflex->ClauseSetProp(ClauseProp::CPType1);
    //claReflex->info->name = "reflexivity";
    this->insertNewCla(claReflex, true);

    //(2) Symmetry: X1~=X2 | X2 =X1
    Clause* claSymmetry = new Clause();
    TermCell* lt = claSymmetry->GetClaTB()->VarInert("A", claSymmetry->ident);
    TermCell* rt = claSymmetry->GetClaTB()->VarInert("B", claSymmetry->ident);
    Lit_p symLitA = new Literal(lt, rt, false);
    Lit_p symLitB = new Literal(rt, lt, true);
    symLitA->EqnSetProp(EqnProp::EPIsEquLiteral);
    symLitB->EqnSetProp(EqnProp::EPIsEquLiteral);
    symLitB->EqnSetProp(EqnProp::EPIsPositive);
    symLitA->next = symLitB;
    claSymmetry->bindingLits(symLitA);
    claSymmetry->ClauseSetProp(ClauseProp::CPTypeAxiom);
    claSymmetry->ClauseSetProp(ClauseProp::CPType1);
    //claSymmetry->info->name = "symmetry";    vEqulityAxiom.push_back(claSymmetry);
    this->insertNewCla(claSymmetry, true);

    //(3) Transitivity: X1~=X2 | X2 ~=X3 | X1 =X3
    Clause* claTrans = new Clause();
    TermCell* tA = claTrans->GetClaTB()->VarInert("A", claTrans->ident);
    TermCell* tB = claTrans->GetClaTB()->VarInert("B", claTrans->ident);
    TermCell* tC = claTrans->GetClaTB()->VarInert("C", claTrans->ident);
    Lit_p transA = new Literal(tA, tB, false);
    transA->EqnSetProp(EqnProp::EPIsEquLiteral);
    Lit_p transB = new Literal(tB, tC, false);
    transB->EqnSetProp(EqnProp::EPIsEquLiteral);
    Lit_p transC = new Literal(tA, tC, true);
    transC->EqnSetProp(EqnProp::EPIsEquLiteral);
    transC->EqnSetProp(EqnProp::EPIsPositive);
    transA->next = transB;
    transB->next = transC;
    claTrans->bindingLits(transA);
    claTrans->ClauseSetProp(ClauseProp::CPTypeAxiom);
    claTrans->ClauseSetProp(ClauseProp::CPType1);

    //claTrans->info->name = "transitivity";
    // vEqulityAxiom.push_back(claTrans);
    this->insertNewCla(claTrans, true);
    //(4) add function-substitution and predicate-substitution
    GenerateEqulitAxiomByFunction();
}

//添加等词函数

void Formula::GenerateEqulitAxiomByFunction() {
    Sigcell* sig_p = Env::getSig();

    for (int fCode = 1; fCode <= sig_p->fCount(); ++fCode) {
        int arity = sig_p->SigFindArity(fCode); //读取函数项个数
        string claName = "";
        sig_p->SigFindName(fCode, claName);
        claName += "_substitution_";
        //读取函数符号
        if (sig_p->SigIsFunction(fCode)) {

            //create arity-number clauses;
            for (int i = 1; i <= arity; i++) {
                Clause* c1 = new Clause();
                //c1->info->name = (claName + to_string(i)).c_st();
                c1->ClauseSetProp(ClauseProp::CPTypeAxiom);
                c1->ClauseSetProp(ClauseProp::CPType1);
                TermCell* lt = c1->GetClaTB()->VarInert("X", c1->ident);
                lt->uVarCount = 1;
                lt->TermCellSetProp(TermProp::TPIsShared);
                TermCell* rt = c1->GetClaTB()->VarInert("Y", c1->ident);
                lt->uVarCount = 1;
                rt->TermCellSetProp(TermProp::TPIsShared);
                Lit_p litA = new Literal(lt, rt, false); //X~=Y                 
                litA->EqnSetProp(EqnProp::EPIsEquLiteral);

                //                //创建n-1个项
                //                TermCell** arrTerm = new TermCell*[arity];
                //                for (int j = 1; j < arity; j++) {
                //                    arrTerm[j] = c1->GetClaTB()->VarInert("Z" + to_string(j), c1->ident);
                //                    arrTerm[j]->uVarCount = 1;
                //                    arrTerm[j]->TermCellSetProp(TermProp::TPIsShared);
                //                }


                //create left term  e.g f3(A,C,D)
                TermCell* leftSubT = new TermCell(fCode, arity);
                int varId = 0;
                for (int j = 1; j <= arity; j++) {
                    if (j == i) {
                        leftSubT->args[j - 1] = lt;
                    } else {
                        TermCell* t = c1->GetClaTB()->VarInert("Z" + to_string(++varId), c1->ident);
                        t->uVarCount = 1;
                        t->TermCellSetProp(TermProp::TPIsShared);
                        leftSubT->args[j - 1] = t;
                    }
                }
                leftSubT->uVarCount = arity;
                leftSubT = c1->GetClaTB()->TBTermTopInsert(leftSubT);
                //create right term  e.g = f3(B,C,D)                 
                TermCell* rightSubT = new TermCell(fCode, arity);
                varId = 0;
                for (int j = 1; j <= arity; j++) {
                    if (j == i) {
                        rightSubT->args[j - 1] = rt;
                    } else {
                        TermCell* t = c1->GetClaTB()->VarInert("Z" + to_string(++varId), c1->ident);
                        t->uVarCount = 1;
                        t->TermCellSetProp(TermProp::TPIsShared);
                        rightSubT->args[j - 1] = t;
                    }
                }
                rightSubT->uVarCount = arity;
                rightSubT = c1->GetClaTB()->TBTermTopInsert(rightSubT);
                litA->next = new Literal(leftSubT, rightSubT, true); //f30(A,C,D) = f30(B,C,D)
                litA->next->EqnSetProp(EqnProp::EPIsPositive);
                litA->next->EqnSetProp(EqnProp::EPIsEquLiteral);
                c1->bindingLits(litA);


                this->insertNewCla(c1, true);
                // this->vEqulityAxiom.push_back(c1);
            }
        }//读取谓词符号
        else if (sig_p->SigIsPredicate(fCode)&& !sig_p->SigIsSpecial(fCode)) {
            //create arity-number clauses;
            for (int i = 1; i <= arity; i++) {
                Clause* c1 = new Clause();

                c1->ClauseSetProp(ClauseProp::CPTypeAxiom);
                c1->ClauseSetProp(ClauseProp::CPType1);
                //c1->info->name = (claName + to_string(i));

                TermCell* lt = c1->GetClaTB()->VarInert("X", c1->ident);
                lt->uVarCount = 1;
                TermCell* rt = c1->GetClaTB()->VarInert("Y", c1->ident);
                rt->uVarCount = 1;
                lt->TermCellSetProp(TermProp::TPIsShared);
                rt->TermCellSetProp(TermProp::TPIsShared);
                Lit_p litA = new Literal(lt, rt, false); //X~=Y                 

                litA->EqnSetProp(EqnProp::EPIsEquLiteral);
                Lit_p litPtr = litA;
                //                //创建n-1个项
                //                TermCell** arrTerm = new TermCell*[arity];
                //                for (int j = 1; j <= arity; j++) {
                //                    arrTerm[j] = c1->GetClaTB()->VarInert("Z" + to_string(j), c1->ident);
                //                    arrTerm[j]->uVarCount = 1;
                //                    arrTerm[j]->TermCellSetProp(TermProp::TPIsShared);
                //                }
                /*====== Create first negative literal ======*/
                //create left term  e.g ~ p2(A,C)                
                TermCell* leftSubT = new TermCell(fCode, arity);
                int iVarId = 0;
                for (int j = 1; j <= arity; j++) {
                    if (j == i) {
                        leftSubT->args[j - 1] = lt;
                    } else {
                        TermCell* t = c1->GetClaTB()->VarInert("Z" + to_string(++iVarId), c1->ident);
                        t->uVarCount = 1;
                        t->TermCellSetProp(TermProp::TPIsShared);
                        leftSubT->args[j - 1] = t;
                    }
                }
                leftSubT->uVarCount = arity;
                leftSubT = c1->GetClaTB()->TBTermTopInsert(leftSubT);

                litPtr->next = new Literal(leftSubT, Env::getGTbank()->trueTerm, false);
                litPtr = litPtr->next;
                /*====== Create right positive literal ======*/
                //create right term  e.g p2(B,C)
                TermCell* rightSubT = new TermCell(fCode, arity);
                iVarId = 0;
                for (int j = 1; j <= arity; j++) {
                    if (j == i) {
                        rightSubT->args[j - 1] = rt;
                    } else {
                        TermCell* t = c1->GetClaTB()->VarInert("Z" + to_string(++iVarId), c1->ident);
                        t->uVarCount = 1;
                        t->TermCellSetProp(TermProp::TPIsShared);
                        rightSubT->args[j - 1] = t;
                    }
                }
                rightSubT->uVarCount = arity;
                rightSubT = c1->GetClaTB()->TBTermTopInsert(rightSubT);

                litPtr->next = new Literal(rightSubT, Env::getGTbank()->trueTerm, true);
                litPtr->next->EqnSetProp(EqnProp::EPIsPositive);
                c1->bindingLits(litA);

                //debug c1->ClausePrint(stdout,true);
                this->insertNewCla(c1, true);
                //this->vEqulityAxiom.push_back(c1);
            }
        }
    }
}



// </editor-fold>

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
                        //debug                        clause->GetClaTB()->TBPrintBankInOrder(stdout);
                        origalClaSet->InsertCla(clause);
                        //                      printf("number of clause:%u\n", Env::global_clause_counter);
                        //
                        //                      cout << "exIndexSize:" << clause->GetClaTB()->extIndex.size() << endl;
                        //                      cout << "inCount:" << clause->GetClaTB()->inCount << endl;
                        //
                        //                      //插入原始子句集
                        //                      if (Env::global_clause_counter % 1000 == 0) {
                        //                          printf("number of clause:%u\n", Env::global_clause_counter);
                        //                          printf("number of GTBankSize:%lu\n", Env::getGTbank()->inCount);
                        //                          printf("number of extIndex:%ld\n", Env::getGTbank()->extIndex.size());
                        //                          printf("number of Sig:%ld\n", Env::getSig()->fInfo.size());
                        //                             
                        //                       // Env::getGTbank()->GTPrintAllTerm(stdout);
                        //                          Env::PrintRusage(stdout);
                        //                          printf("\n");
                        //
                        //                       }
                    }
                }
            }
            break;
    }
}

/*-----------------------------------------------------------------------
 *预处理模块:
 * 1.根据策略 对原始子句集进行排序 (目前是文字个数排序)
 * 2.完成向前归入冗余检查
 * 3.将有效子句加入到子句集中,并且插入索引
 * ---------还未实现 ------------------
 * A. 恒真子句  P+ ~P   或  a=a
 * B. 假文字删除  a~=a 
/*---------------------------------------------------------------------*/
RESULT Formula::preProcess() {
    /*Statistics--    */
    uint16_t uFSNum = 0; //向前归入冗余子句个数
    uint16_t uTautologyNum = 0; //恒真子句个数
    uint16_t uFactorNum = 0;
    iniFolInfo();

    double startTime = CPUTime();
    /*1. 对原始子句集排序 */
    this->initIndex();
    this->origalClaSet->SortByLitNumAsc(); //这里确保了所有的原始子句按照 文字个数进行升序排序从少到多

    list<Clause*>*claLst = this->origalClaSet->getClaSet();

    /*2. 对原始子句集做约减删除 */
    for (auto claIt = claLst->cbegin(); claIt != claLst->cend(); ++claIt) {

        //debug        if((*claIt)->ident==26)            cout<<"asdf"<<endl;
        //------ 检查子句是否为恒真 ------
        if (Simplification::isTautology(*claIt)) {
            ++uTautologyNum;
            (*claIt)->ClauseSetProp(ClauseProp::CPDeleteClause); //标注子句被删除
            (*claIt)->priority = INT_MIN; //修改优先级为最小值 排序永远最后
            continue;
        }

        //------ 检查子句是否为前归入冗余 ------
        TermIndexing* indexing = ((*claIt)->LitsNumber() == 1) ? this->unitClaIndex : this->allTermIndex;
        if (Simplification::ForwardSubsumption((*claIt), indexing)) {
            ++uFSNum;
            //fprintf(stdout, "C%ud is deled \n", (*claIt)->ident);
            (*claIt)->ClauseSetProp(ClauseProp::CPDeleteClause); //标注子句被删除
            (*claIt)->priority = INT_MIN; //修改优先级为最小值 排序永远最后
            continue;
        }
        //------ 对子句进行factor rule 约减 ------
        Clause* factorCla = Simplification::FactorOnce((*claIt));

        if (factorCla) {
            ++uFactorNum;
        } else {
            factorCla = (*claIt);
        }
        this->uMaxLitNum = MAX(this->uMaxLitNum, factorCla->LitsNumber());
        //  this->uMaxFuncLayer=MAX(this->uMaxFuncLayer,(*claIt)->)
        //插入到索引中    1.单元子句索引    2.全局索引
        insertNewCla(factorCla);

        //若为单元子句,检查是否有其他单元子句 合一
        if (factorCla->isUnit()&&(isUnsat(factorCla))) {
            return RESULT::UNSAT;
        }
    }


    StrategyParam::MaxLitNumOfR = 1; //剩余子句集中最大文字数限制-- 决定了△的继续延拓（思考：与扩展▲的区别在于此）   
    StrategyParam::HoldLits_NUM_LIMIT = 2;
    StrategyParam::MaxLitNumOfNewCla = 6; //限制新子句添加到子句集中  -- 决定了搜索空间的膨胀
    StrategyParam::R_MAX_FUNCLAYER = 6;
    //输出子句集预处理的信息---------------------------------------------------
    PaseTime("Preprocess_", startTime);
    fprintf(stdout, "%18s", "# =====Preprocess Information===========#\n");
    this->printOrigalClasInfo(stdout);
    fprintf(stdout, "# Number Of Subsumption Clause:%8u #\n", uFSNum);
    fprintf(stdout, "# Number Of Tautology Clause  :%8u #\n", uTautologyNum);
    fprintf(stdout, "# Number Of Factor  Clause    :%8u #\n", uFactorNum);
    fprintf(stdout, "%12s", "# ======================================#\n");
    return RESULT::SUCCES;
}


//检查单元子句是否存在互补合一 -- unsat

bool Formula::isUnsat(Clause* unitCla) {
    assert(unitCla->isUnit());
    Lit_p checkLit = unitCla->literals;
    if (!checkLit->EqnIsEquLit()) {
        vector<Clause*>&vUnits = checkLit->IsPositive() ? this->vNegUnitClas : this->vPosUnitClas;
        Unify unify;
        Subst* subst = new Subst();
        for (Clause* candCla : vUnits) {
            Lit_p candLit = candCla->literals;
            if (unify.literalMgu(checkLit, candLit, subst)) //找到unsat
            {
                string litInfo = "";
                checkLit->GetLitInfoWithSelf(litInfo);
                string outStr = litInfo + "\n";
                litInfo = "";
                candLit->GetLitInfoWithSelf(litInfo);
                outStr += litInfo + "\n";
                outStr += "[R]:空子句";
                FileOp::getInstance()->outRun(outStr);
                string strCla = "cnf(c" + to_string(Env::global_clause_counter + 1) + ",plain,($false)";
                string parentCla = "";
                parentCla = "c" + to_string(unitCla->ident);
                parentCla += ",c" + to_string(candCla->ident);
                strCla += ",inference( BR,[status(thm)],[" + parentCla + "]) ).\n";
                FileOp::getInstance()->outInfo(strCla);

                subst->Clear();
                return true;
            }
            subst->Clear();
        }
    }
    return false;
}











/// 给定两个剩余文字集合,检查它们组成子句后是否是冗余
/// \param pasClaLeftLits
/// \param uPosLeftLitInd
/// \param actClaLeftLits
/// \param uActLeftLitInd
/// \return 冗余(无效) 返回 true;  有效返回 false;

bool Formula::leftLitsIsRundacy(Literal** pasClaHoldLits, uint16_t uPasHoldLitInd, Literal* actLits, uint16_t uActLeftLitInd, vector<Literal*>&vNewR) {

    if (uPasHoldLitInd == 0) return false; //不是冗余的
    uint16_t uLitNum = uPasHoldLitInd + uActLeftLitInd + vNewR.size(); //总文字个数
    set<Clause*> checkedClas; //存储已经检查过的子句

    //只检查从被动文字出发的 匹配文字
    for (int pLeftIndA = 0; pLeftIndA < uPasHoldLitInd; ++pLeftIndA) {

        Literal* selConLit = pasClaHoldLits[pLeftIndA];

#ifdef OUTINFO       //debug print        
        cout << "选择文字:" << selConLit->claPtr->ident << endl;
        selConLit->EqnTSTPPrint(stdout, true);
        cout << endl;
#endif

        TermIndexing* indexing = (uLitNum == 1) ? this->unitClaIndex : this->allTermIndex;
        //从索引树上获取,候选节点(项)
        TermIndNode* termIndNode = indexing->Subsumption(selConLit, SubsumpType::Forword);
        if (termIndNode == nullptr) {
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
                if (uLitNum < candVarCla->LitsNumber() || checkedClas.find(candVarCla) != checkedClas.end()) {
                    continue;
                }

                ++Env::backword_CMP_counter;
                Unify unify;

                bool isMatch = false;
                uint32_t iniSubstPos = indexing->subst->Size();
                //遍历候选子句的文字集合.排除已经匹配的文字candVarLit,保证所有文字均可以有匹配的文字
                for (Literal* varEqn = candVarCla->Lits(); varEqn; varEqn = varEqn->next) {
                    isMatch = false;
                    //遍历被动子句剩余文字,检查是否可以匹配(match)
                    for (int pLeftIndB = 0; pLeftIndB < uPasHoldLitInd; ++pLeftIndB) {
                        if (pLeftIndA == pLeftIndB) {
                            continue;
                        }

                        Literal* conEqn = pasClaHoldLits[pLeftIndB];
                        //相同正,负属性 || 被归入文字的权重 > 归入文字的权重 
                        if (!conEqn->isComplementProps(varEqn) || conEqn->StandardWeight() < varEqn->StandardWeight())
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
                    Literal* conEqn = actLits;
                    while (conEqn) {
                        if (conEqn->EqnQueryProp(EqnProp::EPIsHold)) {
                            conEqn = conEqn->next;
                            continue;
                        }

                        if (!conEqn->isComplementProps(varEqn) || conEqn->StandardWeight() < varEqn->StandardWeight()) ////相同正,负属性 || 被归入文字的权重 > 归入文字的权重 
                            continue;
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
                        conEqn = conEqn->next;
                    }
                    if (isMatch) {
                        continue;
                    }
                    assert(iniSubstPos == indexing->subst->Size());


                    //遍历现有的剩余文字,检查是否可以匹配(match)
                    for (Literal* conEqn : vNewR) {
                        if (!conEqn->isComplementProps(varEqn) || conEqn->StandardWeight() < varEqn->StandardWeight()) ////相同正,负属性 || 被归入文字的权重 > 归入文字的权重 
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
                    //Print-level   fprintf(stdout, "[FS]R invalid by c%d\n", candVarCla->GetClaId());
                    ++Env::backword_Finded_counter;
                    indexing->ClearVarLst(); //清除替换                    
                    return true;
                }
                //没有找到 indexing->chgVars->SubstBacktrackToPos(substPos);
                assert(iniSubstPos == indexing->subst->Size());
                checkedClas.insert(candVarCla); //添加已经检查过的子句
            }
            termIndNode = indexing->NextForwordSubsump(); //查找下一个
            if (termIndNode == nullptr) {
                indexing->ClearVarLst(); //清除替换
                return false;
            }
            candVarLits = &((termIndNode)->leafs);
        }
    }

    assert(false); //不会执行到这个语句

    return true;
}

bool Formula::leftLitsIsRundacy(Literal* pasClaHoldLits, uint16_t uPasHoldLitInd, vector<Literal*>&vNewR, set<Cla_p>&setUsedCla) {

    const uint16_t uLitNum = uPasHoldLitInd + vNewR.size(); //总文字个数
    if (uLitNum == 0) return false; //不是冗余的
    set<Clause*> checkedClas; //存储已经检查过的子句
    Literal * vCmpLits[uLitNum]; //创建数组用来存储 数据

    { //拷贝要检查的数据到数组 vCmpLits中
        uint32_t ind = 0;
        Lit_p tmpLit = pasClaHoldLits->claPtr->literals;
        while (tmpLit) {
            if (tmpLit->EqnQueryProp(EqnProp::EPIsHold))
                vCmpLits[ind++] = tmpLit;
            tmpLit = tmpLit->next;
        }
        //assert(ind == uPasHoldLitInd);
        if (vNewR.size() > 0)
            memcpy(vCmpLits + ind, &vNewR[0], vNewR.size() * sizeof (Literal*));
    }
    //只检查从 匹配文字    

    //-----------  遍历被动子句文字集中保留的文字 ------
    //for (size_t i = 0; i < uPasHoldLitInd; ++i) {
    for (size_t i = 0; i < uLitNum; ++i) {
        Lit_p selConLit = vCmpLits[i];
        //debug        cout << "选择文字:C" << selConLit->claPtr->ident << endl;        selConLit->EqnTSTPPrint(stdout, true);        cout << endl;

        //------- 查找索引树上的 备选文字节点 ------
        TermIndexing* indexing = (uLitNum == 1) ? this->unitClaIndex : this->allTermIndex;

        // 从索引树上获取,候选节点(项)
        TermIndNode* termIndNode = indexing->Subsumption(selConLit, SubsumpType::Forword);
        if (termIndNode == nullptr) {

            continue; //没有找到 重新查找下一个文字           
        }
        vector<Literal*>*candVarLits = &((termIndNode)->leafs); //可以匹配的文字集
        Clause* candVarCla = nullptr; //待检查的归入冗余的候选子句
        Literal* candVarLit = nullptr;

        //确保候选子句中所有文字均可以匹配到 查询子句中的文字.
        while (true) {
            //------ 遍历候选文字集合.查找满足向前归入的文字
            for (int ind = 0; ind < candVarLits->size(); ++ind) {
                candVarLit = candVarLits->at(ind);
                candVarCla = candVarLit->claPtr; //候选子句                                               
                if (candVarCla == selConLit->claPtr) { //找到查询子句中的文字.包括自己
                    continue;
                }
                if (setUsedCla.find(candVarCla) != setUsedCla.end()) {
                    continue;
                }
                assert(candVarCla);
                uint16_t candLitNum = candVarCla->LitsNumber();
                //单元子句,查询子句为冗余子句     //记录冗余
                if (1 == candLitNum) {
                    //Print-level fprintf(stdout, "\n# [FS]R invalid by c%d\n", candVarCla->GetClaId());
                    string tmpstr = "\n# [FS]R invalid by c" + to_string(candVarCla->GetClaId()) + "\n";
                    FileOp::getInstance()->outLog(tmpstr);
                    ++Env::forward_Finded_counter;

                    indexing->ClearVarLst(); //清除替换
                    return true;
                }
                //条件 候选子句文字个数 <=查询子句文字个数 && 不能是检查过的子句
                if (uLitNum < candLitNum || checkedClas.find(candVarCla) != checkedClas.end()) {
                    continue;
                }
                //++Env::backword_CMP_counter;

                //------ 得到候选子句canVarCla  检查是否存在替换r 使得 canVarCla*r=pasCla + vNewR (排除消除的文字)
                if (Simplification::ClauseSubsumeArrayLit(vCmpLits, uLitNum, candVarCla)) {
                    //找到匹配的冗余子句--说明候选子句中的所有文字均可以通过替换与 剩余文字 匹配.
                    //Print-level fprintf(stdout, "\n# [FS]R invalid by c%d\n", candVarCla->GetClaId());
                    string tmpstr = "\n# [FS]R invalid by c" + to_string(candVarCla->GetClaId()) + "\n";
                    FileOp::getInstance()->outLog(tmpstr);
                    ++Env::forward_Finded_counter;

                    indexing->ClearVarLst(); //清除替换
                    return true;
                }
                //没有找到1.添加已经检查过的子句;2.查找下一个候选子句
                checkedClas.insert(candVarCla);
            }
            termIndNode = indexing->NextForwordSubsump(); //查找下一个
            if (termIndNode == nullptr) {
                break;
            }
            candVarLits = &((termIndNode)->leafs);
        }
        indexing->ClearVarLst();
    }

    //assert(false); //不会执行到这个语句
    return false;
}

bool Formula::HoldLitsIsRundacy(Literal** arrayHoldLits, uint16_t arraySize, set<Cla_p>*setUsedCla, Clause* pasCla) {

    set<Clause*> checkedClas; //存储已经检查过的子句
    bool isRundacy = false; //0 NORundacy 1-- LitRundacy 2 -- ClaRundacy
    //------- 查找索引树上的 备选文字节点 ------
    TermIndexing* indexing = (arraySize == 1) ? this->unitClaIndex : this->allTermIndex;
    //-----------  遍历被动子句文字集中保留的文字 ------
    for (size_t i = 0; i < arraySize; ++i) {
        indexing->ClearVarLst();
        Lit_p selConLit = arrayHoldLits[i];
        //debug          cout << "unit 选择文字:C" << selConLit->claPtr->ident << endl;   selConLit->EqnTSTPPrint(stdout, true);        cout << endl;

        //=== 如果是等词则且非单元子句则不参与索引树操作
        if (selConLit->EqnIsEquLit() && arraySize > 1) {
            continue;
        }

        //=== 从索引树上获取,候选节点(项)
        TermIndNode* termIndNode = indexing->Subsumption(selConLit, SubsumpType::Forword);
        if (termIndNode == nullptr) {
            continue; //没有找到,重新查找下一个文字           
        }
        //== 可以匹配的文字集
        vector<Literal*>*candVarLits = &((termIndNode)->leafs);
        Clause* candVarCla = nullptr; //待检查的归入冗余的候选子句
        //Literal* candVarLit = nullptr;

        //确保候选子句中所有文字均可以匹配到 查询子句中的文字.
        while (true) {
            //------ 遍历候选文字集合.查找满足向前归入的文字
            for (int ind = 0; ind < candVarLits->size(); ++ind) {
                candVarCla = candVarLits->at(ind)->claPtr; //候选子句                                               
                if (candVarCla == selConLit->claPtr || candVarCla == pasCla || (setUsedCla && setUsedCla->find(candVarCla) != setUsedCla->end())) { //找到查询子句中的文字.包括自己
                    continue;
                }

                assert(candVarCla);
                uint16_t candLitNum = candVarCla->LitsNumber();
                //=== 归入子句为单元子句,则查询子句为冗余子句     //记录冗余
                if (1 == candLitNum) {
                    string tmpstr = "\n# [FS]R invalid by UC" + to_string(candVarCla->GetClaId()) + "\n";
                    FileOp::getInstance()->outLog(tmpstr);
                    ++Env::forward_Finded_counter;
                    isRundacy = true;
                    break;
                }
                //=== 条件 候选子句文字个数 <=查询子句文字个数 && 不能是检查过的子句
                if (arraySize < candLitNum || checkedClas.find(candVarCla) != checkedClas.end()) {
                    continue;
                }
                //=== 得到候选子句canVarCla  检查是否存在替换r 使得 canVarCla*r=pasCla + vNewR (排除消除的文字)
                if (Simplification::ClauseSubsumeArrayLit(arrayHoldLits, arraySize, candVarCla)) {
                    //找到匹配的冗余子句--说明候选子句中的所有文字均可以通过替换与 剩余文字 匹配.      
                    string tmpstr = "\n# [FS]R invalid by MC" + to_string(candVarCla->GetClaId()) + "[" + to_string(candVarCla->LitsNumber()) + "]\n";
                    FileOp::getInstance()->outLog(tmpstr);
                    ++Env::forward_Finded_counter;
                    isRundacy = true;
                    break;
                }
                //==没有找到1.添加已经检查过的子句;2.查找下一个候选子句
                checkedClas.insert(candVarCla);
            }
            if (isRundacy)
                break;
            termIndNode = indexing->NextForwordSubsump(); //查找下一个
            if (termIndNode == nullptr) {
                break;
            }
            //debug             for(auto&tt:termIndNode->leafs){            cout<<tt->claPtr->ident<<endl;}
            candVarLits = &((termIndNode)->leafs);
        }
        if (isRundacy)
            break;
    }

    indexing->ClearVarLst();
    return isRundacy;
}

bool Formula::unitLitIsRundacy(Literal* unitLit) {
    bool isRundacy = true;
    // 从索引树上获取,候选节点(项)
    TermIndNode* termIndNode = this->unitClaIndex->Subsumption(unitLit, SubsumpType::Forword);
    if (termIndNode == nullptr) {
        isRundacy = false;
    } else {
        vector<Literal*>*candVarLits = &((termIndNode)->leafs); //可以匹配的文字集
        if (1 == candVarLits->size() && candVarLits->at(0)->claPtr == unitLit->claPtr) {
            //相同子句查找下一个
            termIndNode = this->unitClaIndex->NextForwordSubsump(); //查找下一个
            if (termIndNode == nullptr) {
                isRundacy = false;
            }
        }
    }
    this->unitClaIndex->ClearVarLst();
    return isRundacy;
}



//将子句添加到公式集中

void Formula::insertNewCla(Cla_p cla, bool isEquAxiom) {
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
        if (!isEquAxiom && lit->EqnIsEquLit()) {
            ++(this->uEquLitNum);
        }
        if (!isEquAxiom && lit->IsPositive()) {
            ++posLitNum;
        }
        allTermIndex->Insert(lit);
        lit = lit->next;
    }
    if (isEquAxiom) {//如果是等词公理则不做处理，只加入等词公理集合，不加入workClaSet

        vEqulityAxiom.push_back(cla);
        if (cla->isUnit()) {
            this->AddUnitPredLst(cla);
        } else {
            assert(cla->LitsNumber() > 1);
            this->AddPredLst(cla);
        }

        return;
    }
    if (posLitNum > 1) {
        ++(this->uNonHornClaNum);
    } else if (posLitNum == 0) {//目标子句
        cla->priority = 100;
        this->addGoalClas(cla);
        //添加目标子句
        // this->workClaSet->InsertCla(cla);
        //目标子句优先,将目标子句的优先级改为一个较高的值(也可以试试 maxint)

    }
    //添加到公式集 谓词全局列表中[注意单元子句和非单元子句分开加入谓词列表] 
    if (cla->isUnit()) {
        this->AddUnitPredLst(cla);
    } else {
        assert(cla->LitsNumber() > 1);
        this->AddPredLst(cla);
        //添加到处理后的子句集中
        this->workClaSet->InsertCla(cla);
    }
}

/**
 * 删除work子句集中的子句
 * @param cal
 */
void Formula::removeWorkCla(Cla_p cal) {
    this->workClaSet->RemoveClause(cal); //暂时没有删除谓词链表
}

void Formula::AddUnitPredLst(Clause* unitCla) {
    assert(unitCla->isUnit());
    Literal* lit = unitCla->literals;
    if (unitCla->IsUnitPos()) {
        if (lit->EqnIsEquLit()) { //等词
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
            g_UnitPostPred[0].push_back(lit); //将等词 f(x)=a 当做 E(f(x),a) 存储到谓词列表中 注意令E的fCode=0;
        }
        g_UnitPostPred[lit->lterm->fCode].push_back(lit);;
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
            g_UnitNegPred[0].push_back(lit); //将等词 f(x)=a 当做 E(f(x),a) 存储到谓词列表中 注意令E的fCode=0;
        } else {
            g_UnitNegPred[lit->lterm->fCode].push_back(lit);
        }
    }
}
//添加谓词符号到全局列表中
//注意: 等词也加入pred列表.谓词符号为 0

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
                g_PostPred[0].push_back(lit); //将等词 f(x)=a 当做 E(f(x),a) 存储到谓词列表中 注意令E的fCode=0;
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
                g_NegPred[0].push_back(lit); //将等词 f(x)=a 当做 E(f(x),a) 存储到谓词列表中 注意令E的fCode=0;
            } else {
                g_NegPred[lit->lterm->fCode].push_back(lit);
            }
        }
        lit = lit->next;
    }
}

/*得到互补谓词候选文字集合*/
vector<Literal*>* Formula::getPairUnitPredLst(Literal* lit) {
    //debug    if (lit->EqnIsEquLit())        cout << "eqlit" << endl;
    if (lit->IsPositive())
        return (lit->EqnIsEquLit()) ? &g_UnitNegPred[0] : &g_UnitNegPred[lit->lterm->fCode];
    else
        return (lit->EqnIsEquLit()) ? &g_UnitPostPred[0] : &g_UnitPostPred[lit->lterm->fCode];
}

/*得到互补谓词候选文字集合*/
vector<Literal*>* Formula::getPairPredLst(Literal* lit) {
    //debug    if (lit->EqnIsEquLit())        cout << "eqlit" << endl;
    if (lit->IsPositive())
        return (lit->EqnIsEquLit()) ? &g_NegPred[0] : &g_NegPred[lit->lterm->fCode];
    else
        return (lit->EqnIsEquLit()) ? &g_PostPred[0] : &g_PostPred[lit->lterm->fCode];
}

vector<Literal*>* Formula::getPredLst(Literal* lit) {
    assert(!lit->EqnIsEquLit()); //不能是等词文字
    if (lit->IsPositive())
        return &g_PostPred[lit->lterm->fCode];
    else
        return &g_NegPred[lit->lterm->fCode];
}

list<Clause*>::iterator Formula::getNextStartClause() {
    return min_element(this->workClaSet->getClaSet()->begin(), this->workClaSet->getClaSet()->end(), SortRule::ClaCmp);

}

void Formula::printOrigalClasInfo(FILE* out) {
    fprintf(out, "# Total Number Of Clauses   %11ld #\n", this->origalClaSet->Size());
    fprintf(out, "# Maxium Len Of Clauses     %11u #\n", this->uMaxLitNum);
    fprintf(out, "# Maxium Layer Of Function  %11u #\n", this->uMaxFuncLayer);
    fprintf(out, "# Number Of Goal Clauses    %11zu #\n", this->goalClaset.size());
    fprintf(out, "# Number Of Goal Horn       %11u #\n", uNonHornClaNum);
    fprintf(out, "# IS Equlity                %11s #\n", 0 == this->uEquLitNum ? "FALSE" : "TRUE");
}


