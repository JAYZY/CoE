/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Formula.h
 * Author: Zhong Jian<77367632@qq.com>
 *
 * Created on 2017年12月22日, 下午6:03
 */

#ifndef FORMULA_H
#define FORMULA_H

#include "Formula/ClauseSet.h"
#include "Indexing/TermIndexing.h"
#include "CLAUSE/LiteralCompare.h"
#include "INOUT/FileOp.h"

class Formula {
private:
    //谓词索引------
    map<int32_t, vector<Literal*>> g_PostPred; //正谓词
    map<int32_t, vector<Literal*>> g_NegPred; //负谓词
    map<TermCell*, set<TermCell*>> g_PostEqn; //正等词列表 a=b a=c a=d;
    map<TermCell*, set<TermCell*>> g_NegEqn; //负等词 a!=b a!=c

    //单文字子句
    // TermIndexing *unitTermIndex = nullptr; //单文字子句索引
    /*Statistics(统计信息)--    */



    //list<Clause*> axioms;
    ClauseSet* origalClaSet; //原始子句集合
    ClauseSet* workClaSet; //处理后的子句集,(或工作子句集) --不包含单子子句和等词公理
    Scanner* in;


public:
    //公式集的相关信息
    uint32_t uEquLitNum; //等词个数
    uint32_t uNonHornClaNum; //非Horn子句个数

    TermIndexing* unitClaIndex; //正单元子句索引
    //TermIndexing* posUnitClaIndex; //负单元子句索引
    TermIndexing *allTermIndex; //Discrimation Indexing 主要用于forward subsume

    vector<Clause*> goalClaset; //目标子句集--注意,包括单元子句,也包括其他子句
    vector<Clause*> vNegUnitClas; //负单元子句,只有一个 负文字
    vector<Clause*> vPosUnitClas; //正单元子句

    vector<Clause*> vEqulityAxiom; //等词公理
    Formula();
    Formula(const Formula& orig);
    virtual ~Formula();
private:
    //读取tptp公式集
    void WFormulaTPTPParse();
public:
    /*---------------------------------------------------------------------*/
    /*                       Inline  Function                              */

    /*---------------------------------------------------------------------*/
    inline ClauseSet* getAxioms() {
        return origalClaSet;
    }

    inline list<Clause*>* getWorkClas() {
        return workClaSet->getClaSet();
    }
    //初始化所有的索引

    inline void initIndex() {
        if (this->allTermIndex) {
            DelPtr(allTermIndex);
        }
        allTermIndex = new DiscrimationIndexing();

        if (this->unitClaIndex) {
            DelPtr(unitClaIndex);
        }
        unitClaIndex = new DiscrimationIndexing();
    }

    inline void addGoalClas(Clause* goalCla) {
        goalClaset.push_back(goalCla);
    }

    //    inline void ClauseSetInsert(Clause* cla) {
    //        assert(cla);
    //        claSet->InsertCla(cla);
    //        //暂时没有给定子句的评估函数
    //    }

    inline void printOrigalClaSet() {
        FILE* out = FileOp::getInstance()->getInfoFile();
        string sinfo = "#------ input clause ------\n";
        FileOp::getInstance()->outInfo(sinfo);
        origalClaSet->Print(out);
        sinfo = "\n#------ new clause ------\n";
        FileOp::getInstance()->outInfo(sinfo);
    }

    inline void printProcessedClaSet(FILE* out) {
        workClaSet->Print(out);
    }

    inline void printEqulityAxioms(FILE* out) {
        for (Clause* cla : vEqulityAxiom) {
            cla->ClausePrint(out, true);
            cout << endl;
        }
    }

    /**
     * 对单元子句列表进行排序
     */
    inline void unitClasSort() {
        //对单元子句集合进行排序 
        sort(vNegUnitClas.begin(), vNegUnitClas.end(), UnitClaCompareWithWeight());
        sort(vPosUnitClas.begin(), vPosUnitClas.end(), UnitClaCompareWithWeight());
    }

    inline void GenerateEqulitAxiom() {
        //前3条固定加入：
        //(1) Reflexivity: X1=X1
        //(2) Symmetry: X1~=X2 | X2 =X1
        //(3) Transitivity: X1~=X2 | X2 ~=X3 | X1 =X3

        //======================================================================/
        //(1) Reflexivity: X1=X1
        Clause* claReflex = new Clause();
        claReflex->ClauseSetProp(ClauseProp::CPTypeAxiom);
        TermCell* t = claReflex->claTB->shareVars->Insert("A", claReflex->ident);
        Lit_p reflexLit = new Literal(t, t, true); //Reflexivity: X1=X1
        reflexLit->EqnSetProp(EqnProp::EPIsEquLiteral);
        reflexLit->EqnSetProp(EqnProp::EPIsPositive);
        claReflex->bindingLits(reflexLit);

        //claReflex->info->name = "reflexivity";
        vEqulityAxiom.push_back(claReflex);
        //(2) Symmetry: X1~=X2 | X2 =X1
        Clause* claSymmetry = new Clause();
        TermCell* lt = claSymmetry->claTB->shareVars->Insert("A", claSymmetry->ident);
        TermCell* rt = claSymmetry->claTB->shareVars->Insert("B", claSymmetry->ident);
        Lit_p symLitA = new Literal(lt, rt, false);
        Lit_p symLitB = new Literal(rt, lt, true);
        symLitA->EqnSetProp(EqnProp::EPIsEquLiteral);
        symLitB->EqnSetProp(EqnProp::EPIsEquLiteral);
        symLitB->EqnSetProp(EqnProp::EPIsPositive);
        symLitA->next = symLitB;
        claSymmetry->bindingLits(symLitA);
        claSymmetry->ClauseSetProp(ClauseProp::CPTypeAxiom);
        //claSymmetry->info->name = "symmetry";
        vEqulityAxiom.push_back(claSymmetry);

        //(3) Transitivity: X1~=X2 | X2 ~=X3 | X1 =X3
        Clause* claTrans = new Clause();
        TermCell* tA = claTrans->claTB->shareVars->Insert("A", claTrans->ident);
        TermCell* tB = claTrans->claTB->shareVars->Insert("B", claTrans->ident);
        TermCell* tC = claTrans->claTB->shareVars->Insert("C", claTrans->ident);
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
        //claTrans->info->name = "transitivity";
        vEqulityAxiom.push_back(claTrans);

        //(4) add function-substitution and predicate-substitution
        GenerateEqulitAxiomByFunction();
    }

    inline void GenerateEqulitAxiomByFunction() {
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
                    TermCell* lt = c1->claTB->shareVars->Insert("X", c1->ident);
                    lt->TermCellSetProp(TermProp::TPIsShared);
                    TermCell* rt = c1->claTB->shareVars->Insert("Y", c1->ident);
                    rt->TermCellSetProp(TermProp::TPIsShared);
                    Lit_p litA = new Literal(lt, rt, false); //X~=Y                 
                    litA->EqnSetProp(EqnProp::EPIsEquLiteral);

                    //创建n-1个项
                    TermCell** arrTerm = new TermCell*[arity];
                    for (int j = 1; j <= arity; j++) {
                        arrTerm[j] = c1->claTB->shareVars->Insert("Z" + to_string(j), c1->ident);
                        arrTerm[j]->TermCellSetProp(TermProp::TPIsShared);
                    }
                    //create left term  e.g f3(A,C,D)
                    TermCell* leftSubT = new TermCell(fCode, arity);
                    for (int j = 1; j <= arity; j++) {
                        if (j == i) {
                            leftSubT->args[j - 1] = lt;
                        } else {
                            leftSubT->args[j - 1] = arrTerm[j];
                        }
                    }
                    leftSubT->uVarCount = arity;
                    leftSubT = c1->claTB->TBTermTopInsert(leftSubT);
                    //create right term  e.g = f3(B,C,D)
                    TermCell* rightSubT = new TermCell(fCode, arity);
                    for (int j = 1; j <= arity; j++) {
                        if (j == i) {
                            rightSubT->args[j - 1] = rt;
                        } else {
                            rightSubT->args[j - 1] = arrTerm[j];
                        }
                    }
                    rightSubT->uVarCount = arity;
                    rightSubT = c1->claTB->TBTermTopInsert(rightSubT);
                    litA->next = new Literal(leftSubT, rightSubT, true); //f30(A,C,D) = f30(B,C,D)
                    litA->next->EqnSetProp(EqnProp::EPIsPositive);
                    litA->next->EqnSetProp(EqnProp::EPIsEquLiteral);
                    c1->bindingLits(litA);

                    DelArrayPtr(arrTerm);
                    this->vEqulityAxiom.push_back(c1);
                }
            }//读取谓词符号
            else if (sig_p->SigIsPredicate(fCode)&& !sig_p->SigIsSpecial(fCode)) {
                //create arity-number clauses;
                for (int i = 1; i <= arity; i++) {
                    Clause* c1 = new Clause();
                    c1->ClauseSetProp(ClauseProp::CPTypeAxiom);
                    //c1->info->name = (claName + to_string(i));

                    TermCell* lt = c1->claTB->shareVars->Insert("X", c1->ident);
                    TermCell* rt = c1->claTB->shareVars->Insert("Y", c1->ident);
                    lt->TermCellSetProp(TermProp::TPIsShared);
                    rt->TermCellSetProp(TermProp::TPIsShared);
                    Lit_p litA = new Literal(lt, rt, false); //X~=Y                 
                    litA->EqnSetProp(EqnProp::EPIsEquLiteral);
                    Lit_p litPtr = litA;
                    //创建n-1个项
                    TermCell** arrTerm = new TermCell*[arity];
                    for (int j = 1; j <= arity; j++) {
                        arrTerm[j] = c1->claTB->shareVars->Insert("Z" + to_string(j), c1->ident);
                        arrTerm[j]->TermCellSetProp(TermProp::TPIsShared);
                    }
                    /*====== Create first negative literal ======*/
                    //create left term  e.g ~ p2(A,C)
                    TermCell* leftSubT = new TermCell(fCode, arity);
                    for (int j = 1; j <= arity; j++) {
                        if (j == i) {
                            leftSubT->args[j - 1] = lt;
                        } else {
                            leftSubT->args[j - 1] = arrTerm[j];
                        }
                    }
                    leftSubT = c1->claTB->TBTermTopInsert(leftSubT);
                    litPtr->next = new Literal(leftSubT, Env::getGTbank()->trueTerm, false);
                    litPtr = litPtr->next;
                    /*====== Create right positive literal ======*/
                    //create right term  e.g p2(B,C)
                    TermCell* rightSubT = new TermCell(fCode, arity);
                    for (int j = 1; j <= arity; j++) {
                        if (j == i) {
                            rightSubT->args[j - 1] = rt;
                        } else {
                            rightSubT->args[j - 1] = arrTerm[j];
                        }
                    }
                    rightSubT = c1->claTB->TBTermTopInsert(rightSubT);
                    litPtr->next = new Literal(rightSubT, Env::getGTbank()->trueTerm, true);
                    litPtr->next->EqnSetProp(EqnProp::EPIsPositive);
                    c1->bindingLits(litA);
                    DelArrayPtr(arrTerm);
                    this->vEqulityAxiom.push_back(c1);
                }
            }
        }
    }



    /*---------------------------------------------------------------------*/
    /*                          Static Function                            */
    /*---------------------------------------------------------------------*/
    //读取公式
    void generateFormula(Scanner * in);
    //公式预处理
    RESULT preProcess();

    bool leftLitsIsRundacy(Literal** pasClaLeftLits, uint16_t uPosLeftLitInd, Literal* actLits, uint16_t uActLeftLitInd, vector<Literal*>&vNewR);
    bool leftLitsIsRundacy(Literal* pasClaHoldLits, uint16_t uPasHoldLitInd, vector<Literal*>&vNewR, set<Cla_p>&setUsedCla);

    bool holdLitsIsRundacy(Literal** arrayHoldLits, uint16_t arraySize, set<Cla_p>&setUsedCla, Clause* pasCla);




    //插入新子句到
    void insertNewCla(Cla_p cla);
    //删除子句
    void removeWorkCla(Cla_p cal);

    //添加谓词符号到全局列表中  
    void AddPredLst(Clause * cla);
    vector<Literal*>* getPredLst(Literal * lit);
    /*得到互补谓词候选文字集合*/
    vector<Literal*>* getPairPredLst(Literal * lit);
    //输出公式集的信息
    void printOrigalClasInfo(FILE * out);


    //根据策略，得到下一次演绎起步子句
    list<Clause*>::iterator getNextStartClause();
};

#endif /* FORMULA_H */

