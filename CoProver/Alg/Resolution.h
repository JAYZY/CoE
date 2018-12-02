
/* 
 * File:   Resolution.h
 * Author: Zhong Jian<77367632@qq.com>
 *
 * Created on 2018年11月23日, 下午3:52
 */

#ifndef RESOLUTION_H
#define RESOLUTION_H
#include "Formula/Formula.h"

class Resolution {
public:
    Resolution();
    Resolution(const Resolution& orig);
    virtual ~Resolution();


    RESULT BaseAlgByRecodePath(Formula *fol);
private:

};

#endif /* RESOLUTION_H */

