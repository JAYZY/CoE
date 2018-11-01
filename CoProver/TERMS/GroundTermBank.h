/* 
 * File:   GroundTermBank.h
 * Author: Zhong Jian<77367632@qq.com>
 * Contents: 全局基项存储
 * Created on 2018年11月1日, 上午10:37
 */

#ifndef GROUNDTERMBANK_H
#define GROUNDTERMBANK_H
#include <stdint.h>

#include "Global/IncDefine.h"
class GroundTermBank {
private:
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
private:

};
typedef GroundTermBank *GTermBank_p;
#endif /* GROUNDTERMBANK_H */

