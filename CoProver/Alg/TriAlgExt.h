/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   TriAlgExt.h
 * Author: zj
 *
 * Created on 2019年8月15日, 下午8:19
 */

#ifndef TRIALGEXT_H
#define TRIALGEXT_H
#include "Inferences/InferenceInfo.h"
#include "Inferences/Unify.h"
#include "Global/IncDefine.h"
#include "Formula/Formula.h"
using namespace std;

class TriAlgExt {
public:
    TriAlgExt(Formula* _fol);
    TriAlgExt(const TriAlgExt& orig);
    virtual ~TriAlgExt();
private:
    Unify unify;
    Subst* subst; //一次三角形过程中合一
    Formula* fol;

    vector<Literal*> vMainTri; //主界线
    vector<Clause*> vNewClas; //存储新的子句集
    vector<Clause*> delUnitCla; //存储需要最后清理的单元子句副本

   // set<Cla_p> setUsedCla; //使用过的子句
    vector<Clause*> vUsedClas;//使用过的子句
    map<Clause*, vector<Literal*> > mClaReduceLit; //存储所有参与子句的下拉文字--用于构建主界线输出

    inline void DestroyRNUnitCla();
public:
    // <editor-fold defaultstate="collapsed" desc="扩展△算法">

    /// 扩展△核心算法
    // 算法描述：从非单元子句中选择一个子句放入△中
    /// \param 
    /// \return 
    RESULT ExtendTri();

    RESULT UnitClaReduce(Clause * actCla, vector<Literal*>&vHoldLits);

    /// 主界线下拉约减
    /// \param actCla
    /// \param vHoldLits
    /// \return 
    RESULT MainTriReduce(Literal **actLit, vector<Literal*>&vHoldLits, vector<Literal*> &vNewR);
    bool function(vector<Literal*>& vHoldLits, vector<Literal*>& vNewR, set<Literal*>& vDelLits, Literal* alitPtr, bool& res);


    /// 主界线下拉后的规则检查
    /// \return 

    ResRule RuleCheck(Literal* alitPtr, vector<Literal*>& vHoldLits, vector<Literal*>& vNewR); //, uint8_t* vDelLits

    //添加到新子句集中
    bool Add2NewClas(Clause* newClaA, InfereType infereType);

    // </editor-fold>



    // <editor-fold defaultstate="collapsed" desc="输出">

    /// 回溯所有参与的子句，A.边添加vNewR，边检查是否恒真/相同删除；B.若outStr!=null生成主界线输出；C.newCla赋值使用过的子句
    /// \param actCla
    /// \param vNewR
    /// \param outStr
    /// \param newCla
    /// \return RuleOk; TAUTOLOGY；MoreLit
    ResRule CheckAndFindUsed(Clause * actCla, vector<Literal*>&vNewR );

    void OutTriAndR(vector<Literal*>&vNewR);
    void OutTri(string & sOutStr);
    void OutR(vector<Literal*>&vNewR, string & outStr);

    ResRule CheckNewR(Literal* checkLit, vector<Literal*>&vNewR);
    /// 输出新子句信息到 .info 文件
    /// \param newCla
    /// \param infereType
    void OutNewClaInfo(Clause* newCla, InfereType infereType);

    void OutUnsatInfo(InfereType infereType);
    // </editor-fold>
    /// 初始化变量
    void IniVect();
    /// 释放内存空间    
    void DestroyVect();
};

#endif /* TRIALGEXT_H */

