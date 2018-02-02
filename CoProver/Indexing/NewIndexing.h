/* 
 * File:   NewIndexing.h
 * Author: Zhong Jian<77367632@qq.com>
 * Content: 新的项索引结构 
 * Created on 2018年2月2日, 上午10:34
 */

#ifndef NEWINDEXING_H
#define NEWINDEXING_H
#include "TermIndexing.h"

class NewIndexing : public TermIndexing {
public:
    NewIndexing();
    NewIndexing(const NewIndexing& orig);
    virtual ~NewIndexing();
public:
    //插入文字(项)到indexTree 中
    Insert(Literal *lit);
};

#endif /* NEWINDEXING_H */

