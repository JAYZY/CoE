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

#include "Formula.h"
#include "Global/Environment.h"
#include "Indexing/TermIndexing.h"

//谓词索引------

Formula::Formula() {
    in = Env::getIn();
    claSet = new ClauseSet();
    this->uEquLitNum = 0;
    this->uNonHornClaNum = 0;

}

Formula::Formula(const Formula& orig) {

}

Formula::~Formula() {

}

void Formula::GenerateFormula(Scanner* in) {
    Clause* clause;
    TermBank_p termBank = Env::getTb();
    assert(claSet);
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
                        clause = new Clause();
                        clause->ClauseParse(in, termBank);
                        claSet->InsertCla(clause);
                    }

                }
            }
            break;
    }

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

void Formula::printInfo(FILE* out) {
    fprintf(out, "# Total_Number_Formual    %12ld #\n", this->claSet->Members());
    fprintf(out, "# Horn_Clase              %12s #\n", uNonHornClaNum > 0 ? "FALSE" : "TRUE");
    fprintf(out, "# Horn_Clase_Number       %12u #\n", uNonHornClaNum);
    fprintf(out, "# Has_Equality            %12s #\n", 0 == this->uEquLitNum ? "FALSE" : "TRUE");
    fprintf(out, "# Goal_Clause_Number      %12ld #\n", this->goalClaset.size());
}