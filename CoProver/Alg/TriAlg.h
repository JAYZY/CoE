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
//#include <bits/stdint-uintn.h>
#include <stdbool.h>
#include "Inferences/InferenceInfo.h"
#include "Inferences/Unify.h"
#include "Formula/Formula.h"

//枚举规则检查的返回结果

enum class BackType : uint8_t {
    NoBack, ChgLit, ChgNext
};

enum class ResRule : uint8_t {
    /*主界线文字相同(A文字集中有相同文字)*/
    ALitSameALits,
    ALitSameBLits, /*与前面剩余文字相同(A文字与B文字相同)*/
    BLitSameBLits, /*剩余文字与前面剩余文字相同(B文字与B文字相同)*/
    ChgPasLit, //换被归结文字
    ChgActLit, //换主界线文字(主动归结文字)
    MoreFunclayer/*函数复合层过多*/, MoreLit/*剩余文字过多*/, NoLeftLit/*没有剩余文字*/, SingleLit, TAUTOLOGY/*R为恒真*/,
    EqnTautology, RSubsump/*R包含冗余*/, RULEOK/*规则检查通过*/
};

//主界线文字

typedef struct alit {
    uint16_t reduceNum; //记录,后续的主界线下拉次数
    int16_t unitLitInd; //单文字列表中的下标 默认为 -1; 
    Literal* alit; //主动文字
    Literal* blit; //只记录第一次配对的延拓文字
} ALit, *ALit_p;

//记录被下拉的文字 

typedef struct reduceLit {
    Literal* cLit; //被下拉的文字
    ALit_p aLit; // 下拉作用的主界线文字
} RLit, *RLit_p;

/**
 * 定义一个回退点结构体
 */
typedef struct RollBackPoint {
    Literal* litP; //对应的项 

    uint16_t substSize; //变元替换位置
    uint16_t delLitPos; //被删除文字的位置
    uint16_t uTriPos; //主界线的位置
    uint matchPos; //匹配项的位置
    //vector<Literal*> vHoldLits; //该回退点被保留的文字
    // RollBackPoint(TermCell* t,mPos,sSize):term(t),matchPos(mPos),substSize(sSize){}
} RollBackPiont, *RBPoint_p;

class TriAlg {
public:
    Unify unify;
    Subst* subst; //一次三角形过程中合一

    vector<ALit_p> vALitTri; //主界线文字(A 文字)

    vector<RLit_p> vReduceLit; //被下拉的文字集合

    vector<Literal*>vNewR; //剩余文字(B 文字)

    vector<Clause*> newClas; //产生的新子句 -- 要确保这些子句不是冗余的(无恒真,无向前归入)

    set<uint32_t> setRedundClaId; //记录导致冗余的子句，在路径回退的时候不在与之归结
    set<Cla_p> setUsedCla; //已经归结过的子句，不再比较；

    vector<Clause*> delUnitCla; //存储需要最后清理的单元子句副本
    Formula* fol;

    bool unitResolutionrReduct(Literal* *actLit, uint16_t&uPasHoldLitNum);
    bool unitResolutionBySet(Literal* lit, int ind = 0);
        
    int UnitClasReduct(Lit_p *actLit, uint16_t & uActHoldLitNum);
public:
    TriAlg(Formula* _fol);
    TriAlg(const TriAlg& orig);
    virtual ~TriAlg();
    /*---------------------------------------------------------------------*/
    /*                       Inline  Function                              */
    /*---------------------------------------------------------------------*/
    //

    inline void iniVect() {
        for (int i = 0; i < vALitTri.size(); ++i) {
            DelPtr(vALitTri[i]);
        }
        this->vALitTri.clear();
        this->vALitTri.reserve(16);

        while (!vReduceLit.empty()) {
            DelPtr(vReduceLit.back());            
        }
        this->vReduceLit.clear();
        this->vReduceLit.reserve(16);

        this->vNewR.clear();
        this->vNewR.reserve(16);

        this->setRedundClaId.clear();
        this->setUsedCla.clear();

        this->newClas.clear();
        this->newClas.reserve(2);

        assert(this->delUnitCla.empty());
        this->delUnitCla.reserve(4);

    }

    //销毁三角形过程中所有生成的单元子句副本(有变元的)

    inline void disposeRNUnitCla() {
        for (Clause* cla : delUnitCla) {
            DelPtr(cla);
        }
        delUnitCla.clear();
    }

    inline uint32_t getRNum() {
        return vNewR.size();
    }
    /*---------------------------------------------------------------------*/
    /*                          Member Function                            */
    /*---------------------------------------------------------------------*/
    RESULT GenreateTriLastHope(Clause* givenCla);
    
    /// 添加新子句到新子句集
    /// \param newClaA 需要添加的新子句
    /// \return 是否成功添加到新子句集中。 false -- 没有添加到新子句集&该子句被删除
    bool Add2NewClas(Clause* newClaA);

    //二元子句与单元子句生成新子句
    RESULT GenByBinaryCla(Clause* givenCla);

    ResRule RuleCheck(Literal*actLit, Literal* candLit, Lit_p *leftLit, uint16_t& uLeftLitInd);

    /**
     * 原始的规则检查.不涉及过多的 合一替换
     * @param actLit  主动归结文字
     * @param candLit 被动归结文字
     * @param uPasClaHoldLitSize    返回被动归结子句中剩余文字数
     * @param isVarChg  归结过程中是否有变元变化
     * @return 
     */
    ResRule RuleCheckOri(Literal*actLit, Literal* candLit, uint16_t& uPasClaHoldLitSize, bool isVarChg);

    //单元子句约减后的规则检查
    ResRule RuleCheckUnitReduct(Clause*actCla, Literal* *arrayHoldLits, vector<Literal*>&vDelLit);

    //原始的规则检查.不涉及过多的合一替换,只做规则检查的事情
    ResRule RuleCheckLastHope(Literal*actLit);

    //生成一个新的子句时,遍历主界线对剩余文字进行合一下拉
    int TriMguReduct();

    //清楚三角形的所有变元绑定
    void ClearResVTBinding();


    //对主动归结子句进行处理 并添加到剩余子句集
    ResRule actClaProcAddNewR(Lit_p actLit);

    //对被动归结子句进行处理 
    ResRule pasClaProc(Lit_p candLit, uint16_t& uPasHoldLitNum);

    //生成新子句
    Clause* getNewCluase(Clause* pasCla);

    // <editor-fold defaultstate="collapsed" desc="输出相关">
    void printTri(FILE* out);

    void printR(FILE* out, Literal* lit);

    void outTri();
    void outTri(vector<ALit_p>& vTri, string&outStr);
    void outR(Clause * actCla);
    void outNewClaInfo(Clause* newCla, InfereType infereType, set<Cla_p>*setUCla = nullptr);

    // </editor-fold>


private:

};

#endif /* TRIALG_H */

