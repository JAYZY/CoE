
/* 
 * File:   ProverResultAanl.h
 * Author: zj
 * Content:  对结果进行分析提取得到空路径
 * Created on 2019年6月2日, 下午8:19
 */

#ifndef PROVERRESULTAANL_H
#define PROVERRESULTAANL_H
#include"Global/IncDefine.h"
#include "Formula/Formula.h"

typedef struct Stu_Lit {
    int Row; //对应原始的行列号
    int Col;
    const char* context; //文字 

    Stu_Lit(int r, int c,const char* lit) : Row(r), Col(c), context(lit) {
    }
} *StuLitP;

//有效路径集合

typedef struct Stu_OptVaildPath {
    char rState; //归结式状态 0--无效 1--有效 -1--备选 
    int triTimes; //迭代次数
    int rowNewR; //新归结式行号            
    vector<StuLitP>* lsPred; //互补文字列表
    vector<StuLitP>* newR; //新归结式

    Stu_OptVaildPath(int times) : triTimes(times) {
        rState = 1;
        rowNewR = 0;
        lsPred = nullptr;
        newR = nullptr;
    }
} *StuVaildPathP;

class ProverResultAnalyse {
    Formula *fol;
    vector<int> lstValidPathIndex; //有效路径序号 

    vector<StuVaildPathP> lstValidPaths; //一次完整的演绎过程    

public:
    ProverResultAnalyse(Formula *_fol);
    ProverResultAnalyse(const ProverResultAnalyse& orig);
    virtual ~ProverResultAnalyse();
public:

    bool AnalyDeducdPath(string readFileN, string& sbReadContext, string& sbValidPath);
    void OutputResultP(int newR, vector<int>& P);
    int FindPath(int newRow);
    void ShowUNSATPath(string fileFullName);
};

#endif /* PROVERRESULTAANL_H */

