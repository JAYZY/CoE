/* 
 * File:   ProofControl.cpp
 * Author: Zhong Jian<77367632@qq.com>
 * 证明过程控制 
 * Created on 2018年1月8日, 上午10:30
 */

#include "ProofControl.h"
#include "Inferences/Simplification.h"
#include "INOUT/FileOp.h"

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

    Simplification::BackWordSubsumption(selCla, termIndex);
    Insert(selCla);

    termIndex->ClearVarLst();
}

uint32_t Processed::Insert(Clause* cla) {
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

void ProofControl::parseInput() {

}
//

Clause* ProofControl::Saturate() {
    uint32_t count = 0;
    Options::step_limit = this->axiom->Members();
    //对子句集合排序

    double initial_time = cpuTime();

    while (Options::step_limit>++count
            && unprocSet->getClaNum() > 0) {

        Clause * selCla = this->unprocSet->GetBestClause();

        this->procedSet->Proc(selCla);
    }
    paseTime("FindBack", initial_time);

    FILE *fp;
#ifdef New
    fp = fopen("rectNew.txt", "a"); //参数a表示追加写入 

#else
    fp = fopen("rectOld.txt", "a"); //参数a表示追加写入 
#endif


    string fileName = FileOp::FileNameBaseName(Env::getIn()->source->source);
    // printf("%s,%lu,%12.2f s",fileName.c_str(),Env::backword_CMP_counter, cpuTime() - initial_time);
    fprintf(fp, "%s,%lu,%lu,%12.2f\n", fileName.c_str(), Env::backword_CMP_counter, Env::backword_Finded_counter, cpuTime() - initial_time);

    printf("%s,%lu,%lu,%5.2f\n", fileName.c_str(), Env::backword_CMP_counter, Env::backword_Finded_counter, cpuTime() - initial_time);

    // this->procedSet->PrintIndex();
}