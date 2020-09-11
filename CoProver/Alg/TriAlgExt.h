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
#include "TriAlg.h"
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
    vector<Clause*> vUsedClas; //使用过的子句
    map<Clause*, vector<Literal*> > mClaHoldLits; //记录所有参与归结子句的剩余文字 存储子句的 C+部分

    inline void DestroyRNUnitCla();
public:
    
    // <editor-fold defaultstate="collapsed" desc="扩展△算法">

    /// 扩展△核心算法
    // 算法描述：从非单元子句中选择一个子句放入△中
    /// \param 
    /// \return 
    RESULT ExtendTri();

    /// 新的扩展方式--一个文字先完成单元，再完成主界线；尽可能充分
    /// \return 
    RESULT ExtendTriByFull();

    /// 单元子句下拉
    /// \param actCla
    /// \param vNewR
    /// \return 
    RESULT UnitClaReduce(Literal **actLit, Clause * actCla, vector<Literal*> &vNewR);

    /// 对子句进行一次完整的约减 --1.单元，2.主界线 尽可能充分下拉
    /// \param actLit
    /// \param ind
    /// \return 
    RESULT DeduceOnceByFullPath(Clause * actCla);

    bool UCTriRed(Literal *actLit, vector<Literal*>&newR, vector<Literal*>& vDelActLit, vector<RBPoint_p>&vRollBackPoint/*记录回退点*/, uint32_t&uInd);
    bool MainTriRed(Literal *actLit, vector<Literal*>&vDelActLit, vector<Literal*>&newR);

    /// 主界线下拉约减
    /// \param actCla
    /// \param vHoldLits
    /// \return 
    RESULT MainTriReduce(Literal **actLit, Clause* actCla, vector<Literal*> &vNewR);

    /// 对产生的新子句进行 factor rule 处理。
    /// 1. 规则成功，删除newCla ,并检查约减后的子句是否可以添加到新子句集中； 2.检查是否可以将新子句添加到新子句集中
    /// \param newCla
    /// \param vNewR
    /// \return 
    bool Factor2NewCla(Clause* newCla, vector<Literal*>& vNewR);

    /// 主界线下拉后的规则检查
    /// \return 

    ResRule RuleCheck(Literal* alitPtr, vector<Literal*>& vNewR); //, uint8_t* vDelLits

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
    ResRule CheckAndFindUsed(Clause * actCla, vector<Literal*>&vNewR);

    void OutTriAndR(vector<Literal*>&vNewR);
    void OutTri(string & sOutStr);
    void OutR(vector<Literal*>&vNewR, string & outStr);



    ResRule CheckUnitRule(Literal* actLitPtr, vector<Literal*>&vDelActLit, vector<Literal*> &vNewR);
    ResRule CheckNewR(Literal* checkLit, vector<Literal*>&vNewR);


    //重命名单元子句输出

    inline void OutRNUnitCla(Clause* unitCla) {
        //输出到.r文件
        string str = "\n";
        unitCla->literals->matchLitPtr->GetLitInfoWithSelf(str);
        str += "\nR[" + to_string(unitCla->ident) + "]:";
        unitCla->literals->GetLitInfoWithParent(str);
        FileOp::getInstance()->outRun(str + "\n");
    }




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

