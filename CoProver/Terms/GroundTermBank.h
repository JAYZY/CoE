/* 
 * File:   GroundTermBank.h
 * Author: Zhong Jian<77367632@qq.com>
 * Contents: 全局基项存储, 该全局项是不会被删除的
 * Created on 2018年11月1日, 上午10:37
 */

#ifndef GROUNDTERMBANK_H
#define GROUNDTERMBANK_H
#include <stdint.h>
#include "VarBank.h"
#include "TermCellStore.h"
#include "INOUT/Scanner.h"
#include "TermCell.h"
class GroundTermBank {
private:
   
public:
    uint64_t inCount; 
    TermCell* trueTerm; /* 特殊项$true -- Pointer to the special term with the $true constant. */
    TermCell* falseTerm; /* 特殊项$false -- Pointer to the special term with the $false constant. */
    TermCell* minTerm; /* A small (ideally the minimal possible) term, to be used for RHS instantiation. */
    
    vector<TermCell*> extIndex; /* 用来存储项的缩写形式 */
    TermCellStore termStore; /*存放Term的hash结构 -- Here are the terms */
public:
    /*---------------------------------------------------------------------*/
    /*                    Constructed Function                             */
    /*---------------------------------------------------------------------*/
    GroundTermBank();
    GroundTermBank(const GroundTermBank& orig);
    virtual ~GroundTermBank();
    /*---------------------------------------------------------------------*/
    /*                  Member Function-[public]                           */
    /*---------------------------------------------------------------------*/
    //
    TermCell* GBInsert(TermCell* term);
    TermCell* GTermTopInsert(TermCell* t);

    void GTPrintAllTerm(FILE *out) ;
};
typedef GroundTermBank *GTermBank_p;
#endif /* GROUNDTERMBANK_H */

