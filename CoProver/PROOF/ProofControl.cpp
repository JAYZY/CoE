/* 
 * File:   ProofControl.cpp
 * Author: Zhong Jian<77367632@qq.com>
 * 证明过程控制 
 * Created on 2018年1月8日, 上午10:30
 */

#include "ProofControl.h"

void Processed::Proc(Clause* selCla) {
    //检查 forword-subsumption
    // ti->ForwordSubsumption(selCla);
    //检查 back-subsumption

    cout<<"选中的子句:";
    selCla->ClausePrint(stdout,true);cout<<endl;
    //加入indexing 
    Literal* lit = selCla->Lits();
    Literal* p = lit;
    while (p) {
        Literal* isSubsump = ti->ForwordSubsumption(p->lterm);
        cout <<( (isSubsump==nullptr)? "nill":"issub") << endl;
        p = p->next;
    }

    while (lit) {
        ti->Insert(lit);
        lit = lit->next;
    }

}

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

Clause* ProofControl::Saturate() {
    uint32_t count = 0;
    Options::step_limit = 10;
    while (Options::step_limit>++count
            && unprocSet->getClaNum() > 0) {

        Clause * selCla = this->unprocSet->GetBestClause();

        this->procedSet->Proc(selCla);

        this->unprocSet->RemoveCla(selCla);

    }
    this->procedSet->PrintTi();
}