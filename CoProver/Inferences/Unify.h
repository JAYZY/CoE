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
#include "Global/IncDefine.h"
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
    static bool SubstComputeMatch(TermCell* matcher, TermCell* to_match, Subst*subst);
    //bool SubstComputeMgu(Term_p t1, Term_p t2,  vector<TermCell*>&subst);

    //    /* 回滚替换变元,直到给定的位置pos -- Backtrack variable bindings up to (down to?) a given stack pointer position. */
    //    int SubstBacktrackToPos(int pos, vector<TermCell*> &vecSubst);
    //private:
    //    
    //    int SubstAddBinding(TermCell* var, TermCell* bind, vector<TermCell*> &vecSubst);
    //
    //    /* Backtrack a single binding and remove it from the substitution (if possible). 
    //     * Return true if successful, false if the substitutuion is empty. */
    //    bool SubstBacktrackSingle( vector<TermCell*> &vecSubst);
    //
    //   
    //
    //    /* Undo all stored variable binding in subst. */
    //    int SubstBacktrack( vector<TermCell*> &vecSubst);

};

#endif /* UNIFY_H */

