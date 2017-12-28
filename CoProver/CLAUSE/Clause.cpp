/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Clause.cpp
 * Author: Zhong Jian<77367632@qq.com>
 * 
 * Created on 2017年12月22日, 下午8:41
 */

#include "Clause.h"
#include "Global/Environment.h"
/*---------------------------------------------------------------------*/
/*                    Constructed Function                             */
/*---------------------------------------------------------------------*/

//

Clause::Clause()
: ident(-1), properties(ClauseProp::CPIgnoreProps), info(nullptr), literals(nullptr)
, negLitNo(0), posLitNo(0), weight(123), parent1(nullptr), parent2(nullptr) {
    printf("asdf\n");
}

Clause::Clause(Literal* literal_s) : Clause() {
    //Clause_p handle = EmptyClauseAlloc();

    Literal* next;

    ident = ++Env::global_clause_counter;

    //    while (literal_s) {
    //        next = literal_s->next;
    //        if (literal_s->EqnIsPositive()) {
    //            posLitNo++;
    //            *pos_append = literal_s;
    //            pos_append = &((*pos_append)->next);
    //        } else {
    //            negLitNo++;
    //            *neg_append = literal_s;
    //            neg_append = &((*neg_append)->next);
    //        }
    //        literal_s = next;
    //    }
    //    *pos_append = neg_lits;
    //    *neg_append = nullptr;
    //    literals = pos_lits;
    //return handle;
}

Clause::~Clause() {
}
/*---------------------------------------------------------------------*/
/*                  Member Function-[public]                           */
/*---------------------------------------------------------------------*/
/// 解析子句
/// \param in 扫描器scanner
void Clause::ClauseParse(Scanner* in) {
     
}
