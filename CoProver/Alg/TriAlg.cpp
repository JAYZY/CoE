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
#include "Inferences/Simplification.h"
#include "Inferences/Unify.h"
#include "CLAUSE/LiteralCompare.h"
#include<algorithm>

TriAlg::TriAlg(Formula* _fol) : fol(_fol) {

}

TriAlg::TriAlg(const TriAlg& orig) {
}

TriAlg::~TriAlg() {
}

bool TriAlg::GenerateTriByRecodePath(Clause* givenCla) {
    /*
     * 1.传入目标子句 --  所有文字与单元子句进行合一  
     * 2.选择自动文字 优先选择负文字
     * 3.与全局谓词符号进行比较
     */


    givenCla->SortLits();
    Unify unify;
    Subst* subst = new Subst();

    fol->unitClasSort();
    VarBank_p triFreeVarBank=new VarBank();//三角形期间的所有自由变量
    
    Literal* gLit = givenCla->Lits(); //选择一个文字  //遵循 稳定度高的 VS 稳定度高的
    
    while (gLit) {
        Literal* gLitCopy = gLit->EqnCopyDisjoint();
        vector<Clause* >*cmpUnitClas = gLit->IsPositive() ? &fol->vPosUnitClas : &fol->vNegUnitClas;
        for (auto&candUnitCal : *cmpUnitClas) {
            int backpoint = subst->Size();
            bool res = unify.literalMgu(gLitCopy, candUnitCal->Lits(), subst);
            if (res) {
                //找到可以互补的单文字子句,将该单文字子句添加到A文字列表中
               // vALit.push_back(candUnitCal->Lits());
                break;
            }
            subst->SubstBacktrackToPos(backpoint);
        }

    }

    return true;
}
//单元归结

void TriAlg::unitResolutionBySet(Literal* gLit) {
    //vector<Literal* >*cmpUnitClas = gLit->IsPositive() ? &fol->vPosUnitClas : &fol->vNegUnitClas;


}

void TriAlg::unitResolutionByIndex(Literal* gLit) {

    TermIndNode* termIndNode = fol->posUnitClaIndex->Subsumption(gLit, SubsumpType::Forword);
    if (termIndNode == nullptr)
        return ;
    vector<Literal*>*candVarLits = &((termIndNode)->leafs);
}