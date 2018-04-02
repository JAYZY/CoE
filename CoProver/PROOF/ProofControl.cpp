/* 
 * File:   ProofControl.cpp
 * Author: Zhong Jian<77367632@qq.com>
 * 证明过程控制 
 * Created on 2018年1月8日, 上午10:30
 */

#include "ProofControl.h"
#include "Inferences/Simplification.h"
#include "INOUT/FileOp.h"

ProofControl::ProofControl(list<Clause*>& _axiom) : axiom(_axiom) {
    this->procedSet = new Processed();
    this->unprocSet = new UnProcessed(_axiom);
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

    //    cout << "选中的子句:";
    //    selCla->ClausePrint(stdout, true);
    //    if (selCla->GetClaId() == 31)
    //        cout << endl;
    // cout << endl;
    //加入indexing 
    Literal* lit = selCla->Lits();
    Literal* p = lit;
    //对选择的子句进行simplify
    //检查选择的子句是否有效
    //Simplification::ForwordSubsumption(selCla, termIndex);
    //simpliy 子句集中的子句(根据selCla,检查冗余的子句)

    Simplification::BackWordSubsumption(selCla, termIndex); //删除
    Insert(lit);

    termIndex->ClearVarLst();
}

uint32_t Processed::Insert(Literal* lit) {
    while (lit) {
        termIndex->Insert(lit);
        lit = lit->next;
    }

}


//

Clause* ProofControl::Saturate() {
    uint32_t count = 0;
    Options::step_limit = this->axiom.size();
    double initial_time = cpuTime();

    while (Options::step_limit>++count
            && unprocSet->getClaNum() > 0) {

        Clause * selCla = this->unprocSet->GetBestClause();

        this->procedSet->Proc(selCla);

        this->unprocSet->RemoveCla(selCla);

    }
    paseTime("FindBack", initial_time);
    
    FILE *fp;
    
    fp = fopen("rect.txt", "a"); //参数a表示追加写入 
    
    string fileName=FileOp::FileNameBaseName(Env::getIn()->source->source);
    // printf("%s,%lu,%12.2f s",fileName.c_str(),Env::backword_CMP_counter, cpuTime() - initial_time);
    fprintf(fp, "%s,%lu,%lu,%12.2f\n",fileName.c_str(),Env::backword_CMP_counter, Env::backword_Finded_counter,cpuTime() - initial_time);
    // this->procedSet->PrintIndex();
}