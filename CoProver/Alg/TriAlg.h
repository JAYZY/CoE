/*-----------------------------------------------------------------------
 File:   TriAlg.h
 Author: Zhong Jian<77367632@qq.com>
 Contents
    该模块实现核心演绎算法 -- 三角形算法
 Created on 2018年10月25日, 上午10:46
 * Changes
<1>
-----------------------------------------------------------------------*/

#ifndef TRIALG_H
#define TRIALG_H
#include <stdint.h>

#include "Formula/Formula.h"
typedef struct alit{
    Literal* alit;
    uint32_t reduceNum;
}ALit,*ALit_p;
class TriAlg {
private:
    vector<ALit_p> vALit;//主界线文字(A 文字)
    vector<Literal*>vNewR;//剩余文字(B 文字)
    Formula* fol;
    
    void unitResolutionByIndex(Literal* lit);
     void unitResolutionBySet(Literal* lit);
public:
    TriAlg(Formula* _fol);
    TriAlg(const TriAlg& orig);
    virtual ~TriAlg();
    /*---------------------------------------------------------------------*/
    /*                          Member Function                            */
    /*---------------------------------------------------------------------*/
    bool GenerateTriByRecodePath(Clause* givenCla);
    
    
    
private:

};

#endif /* TRIALG_H */

