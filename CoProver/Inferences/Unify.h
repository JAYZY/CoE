/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Unify.h
 * Author: Zhong Jian<77367632@qq.com>
 *
 * Created on 2018年3月1日, 下午1:16
 */

#ifndef UNIFY_H
#define UNIFY_H
#include "CLAUSE/Literal.h"
#include "Subst.h"



class TermCell;

class Unify {
private:

public:
    /*---------------------------------------------------------------------*/
    /*                    Constructed Function                             */
    /*---------------------------------------------------------------------*/
    //
    Unify();
    Unify(const Unify& orig);
    virtual ~Unify();
    /*---------------------------------------------------------------------*/
    /*                          Static Function                            */
    /*---------------------------------------------------------------------*/
    //
    bool OccurCheck(Term_p term, Term_p var);
    bool SubstComputeMatch(TermCell* matcher, TermCell* to_match, Subst*subst);
    bool SubstComputeMgu(Term_p t1, Term_p t2, Subst_p subst);
    bool literalMgu(Literal* litA,Literal* litB,Subst_p subst);
    //等词的合一   E(f(x),f(a))  -E(x,f(b))
    
    bool literalEMgu(Literal* litA, Literal* litB, Subst_p subst);
};

#endif /* UNIFY_H */

