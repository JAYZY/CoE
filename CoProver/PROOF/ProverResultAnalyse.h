
/* 
 * File:   ProverResultAanl.h
 * Author: zj
 * Content:  对结果进行分析提取得到空路径
 * Created on 2019年6月2日, 下午8:19
 */
#include"Global/IncDefine.h"
#ifndef PROVERRESULTAANL_H
#define PROVERRESULTAANL_H

struct Stu_OptVaildPath {
    int triTimes; //迭代次数
    vector<Stu_Lit> lsPred; //互补文字列表
    vector<Stu_Lit> newR; //新归结式
    int rowNewR; //新归结式行号        
    int RState; //归结式状态 0--无效 1--有效 -1--备选 
};

struct Stu_Lit {
    int Row;
    int Col; //对应原始的行列号
    string context; //文字 
};

class ProverResultAanl {
public:
    ProverResultAanl();
    ProverResultAanl(const ProverResultAanl& orig);
    virtual ~ProverResultAanl();
private:

};

#endif /* PROVERRESULTAANL_H */

