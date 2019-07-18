
/* 
 * File:   Resolution.h
 * Author: Zhong Jian<77367632@qq.com>
 *
 * Created on 2018年11月23日, 下午3:52
 */

#ifndef RESOLUTION_H
#define RESOLUTION_H
#include "Formula/Formula.h"
#include "HEURISTICS/StrategyParam.h"

class Resolution {
public:
    Resolution();
    Resolution(const Resolution& orig);
    virtual ~Resolution();
    /*****************************************************************************
     * 基础三角形算法
     ****************************************************************************/
    RESULT BaseAlg(Formula *fol);
    RESULT BaseAlgByOnlyBinaryCla(Formula *fol);
private:

};

#endif /* RESOLUTION_H */

