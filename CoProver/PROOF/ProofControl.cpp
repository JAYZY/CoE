/* 
 * File:   ProofControl.cpp
 * Author: Zhong Jian<77367632@qq.com>
 * 证明过程控制 
 * Created on 2018年1月8日, 上午10:30
 */

#include <random>

#include "ProofControl.h"
#include "Inferences/Simplification.h"
#include "INOUT/FileOp.h"
#include "Formula/Formula.h"

//TermIndexing* ProofControl::negUnitClaIndex; //单元子句索引
//TermIndexing* ProofControl::posUnitClaIndex; //单元子句索引
//
//list<Clause*> ProofControl::negUnitClas; //负单元子句,只有一个 负文字
//
//list<Clause*> ProofControl::posUnitClas; //正单元子句

ProofControl::ProofControl(ClauseSet* _claSet) : axiom(_claSet) {
    this->procedSet = new Processed();
    this->unprocSet = new UnProcessed(_claSet);
}

ProofControl::ProofControl(const ProofControl& orig) {
}

ProofControl::~ProofControl() {
}

/*---------------------------------------------------------------------*/
/*                  Member Function-[public]                           */
/*---------------------------------------------------------------------*/
//

void Processed::Proc(Clause* selCla) {
    //检查 forword-subsumption
    // ti->ForwordSubsumption(selCla);
    //检查 back-subsumption
#ifdef OUTINFO

    cout << "选中的子句:";
    selCla->ClausePrint(stdout, true);
    if (selCla->GetClaId() == 30)
        cout << endl;
    cout << endl;
#endif    
    //加入indexing 
    //    Literal* lit = ;
    //    Literal* p = lit;
    //对选择的子句进行simplify
    //检查选择的子句是否有效
    //    if (!Simplification::ForwordSubsumption(selCla, termIndex)) {
    //        Insert(selCla);
    //    }

    //simpliy 子句集中的子句(根据selCla,检查冗余的子句)

    Simplification::BackWardSubsumption(selCla, termIndex);
    InsertInd(selCla);

    termIndex->ClearVarLst();
}

/*-----------------------------------------------------------------------
 * 1.检查 forword-subsumption
 * 2.生成单文字子句索引
/*---------------------------------------------------------------------*/

void Processed::PreProc(Clause* selCla) {
    assert(termIndex);

    if (Simplification::ForwardSubsumption(selCla, termIndex)) {
        //记录冗余

        return;
    }
    InsertInd(selCla);


}

uint32_t Processed::InsertInd(Clause* cla) {
    Literal* lit = cla->Lits();
    while (lit) {
        termIndex->Insert(lit);
        lit = lit->next;
    }
    if (cla->IsUnitPos()) {
        PosRules.push_back(cla);
    }//处理过的正文字 单元子句 (有序的,要么不是等词,要么已经左项>右项) 
    else if (cla->isUnitNeg()) {
        NegUnits.push_back(cla);

    } else {
        NonUnits.push_back(cla);
    }

    //PosEqns; //处理过的正文字单元子句(无序的等词文字)
    //    list<Clause*>  //单元子句,只有一个 负文字
    //    list<Clause*> NonUnits; //其他非单元子句
}

//预处理    

void ProofControl::Preprocess(Formula* fol) {
    /*Statistics--    */
    uint32_t uFSNum = 0; //向前归入冗余子句个数
    fol->uEquLitNum = 0; //等词文字个数
    fol->uNonHornClaNum = 0; //非Horn 子句个数(最多一个非负文字)
    ClauseSet* pClaSet = new ClauseSet();
    double initial_time = CPUTime();
    ClauseSet* oriClaset = fol->getAxioms(); //原始子句集 

    TermIndexing *allTermIndex = new DiscrimationIndexing();
    fol->posUnitClaIndex = new DiscrimationIndexing();
    fol->negUnitClaIndex = new DiscrimationIndexing();
    oriClaset->SortByLitNumAsc();

    list<Clause*>*claLst = oriClaset->getClaSet();

    for (auto claIt = claLst->cbegin(); claIt != claLst->cend();) {

        // oriClaset->ClauseSetExtractFirst();
        Clause* selCla = *claIt;
        if (Simplification::ForwardSubsumption(selCla, allTermIndex)) {
            ++uFSNum;
            claIt = claLst->erase(claIt);
            continue;
        }
        //插入到索引中    1.单元子句索引    2.全局索引
        Literal * lit = selCla->Lits();
        if (selCla->IsUnitPos()) {
            fol->posUnitClaIndex->Insert(lit);
            fol->vPosUnitClas.push_back(selCla);

        }
        if (selCla->isUnitNeg()) {
            fol->negUnitClaIndex->Insert(lit);
            fol->vNegUnitClas.push_back(selCla);
        }
        uint32_t posLitNum = 0;

        while (lit) {
            if (lit->EqnIsEquLit()) {
                ++(fol->uEquLitNum);
            }
            if (lit->IsPositive()) {
                ++posLitNum;
            }
            allTermIndex->Insert(lit);
            lit = lit->next;
        }
        if (posLitNum > 1) ++(fol->uNonHornClaNum);
        else if (posLitNum == 0) {//目标子句
            fol->addGoalClas(selCla);
        }
        //添加到公式集 谓词全局列表中(注意单文字子句不加入谓词列表) 
        if (selCla->LitsNumber() > 1)
            fol->AddPredLst(selCla);

        ++claIt;
    }

    //信息输出
    PaseTime("Preprocess_", initial_time);
    fprintf(stdout, "%12s", "# Preprocess Information===============#\n");
    fprintf(stdout, "# ForwardSubsump          %12u #\n", uFSNum);
    fol->printInfo(stdout);
}

void ProofControl::parseInput() {

}
//
CPUTIME_DEFINE(FindBack);

Clause * ProofControl::Saturate() {
    uint32_t count = 0;
    Options::step_limit = this->axiom->Members();
    //对子句集合排序

    double initial_time = CPUTime();

    while (Options::step_limit>++count
            && unprocSet->GetClaNum() > 0) {

        Clause * selCla = this->unprocSet->GetBestClause();

        this->procedSet->Proc(selCla);
    }
    CPUTIME_EXIT(FindBack)
    PaseTime("FindBack", initial_time);

    FILE *fp;
#ifdef New
    fp = fopen("rectNew.txt", "a"); //参数a表示追加写入 

#else
    fp = fopen("rectOld.txt", "a"); //参数a表示追加写入 
#endif


    string fileName = FileOp::FileNameBaseName(Env::getIn()->source->source);
    // printf("%s,%lu,%12.2f s",fileName.c_str(),Env::backword_CMP_counter, cpuTime() - initial_time);
    fprintf(fp, "%s,%lu,%lu,%12.2f\n", fileName.c_str(), Env::backword_CMP_counter, Env::backword_Finded_counter, CPUTime() - initial_time);

    printf("%s,%lu,%lu,%5.2f\n", fileName.c_str(), Env::backword_CMP_counter, Env::backword_Finded_counter, CPUTime() - initial_time);

    // this->procedSet->PrintIndex();
}