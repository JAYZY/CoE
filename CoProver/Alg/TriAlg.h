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
#include <bits/stdint-uintn.h>
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
    ChgPosLit, //换被归结文字
    ChgActLit, //换主界线文字(主动归结文字)
    MoreFunclayer/*函数复合层过多*/, MoreLit/*剩余文字过多*/, NoLeftLit/*没有剩余文字*/, SingleLit, TAUTOLOGY/*R为恒真*/,
    EqnTautology, RSubsump/*R包含冗余*/, RULEOK/*规则检查通过*/
};

//主界线文字

typedef struct alit {
    Literal* alit; //主动文字
    Literal* blit; //只记录第一次配对的延拓文字
    uint32_t reduceNum; //记录,后续的主界线下拉次数
} ALit, *ALit_p;

//记录被下拉的文字 

typedef struct reduceLit {
    Literal* cLit; //被下拉的文字
    ALit_p aLit; // 下拉作用的主界线文字
} RLit, *RLit_p;

class TriAlg {
private:
    Unify unify;
    Subst* subst; //一次三角形过程中合一

    vector<ALit_p> vALitTri; //主界线文字(A 文字)

    vector<RLit_p> vReduceLit; //被下拉的文字集合

    vector<Literal*>vNewR; //剩余文字(B 文字)
    set<uint32_t> setRedundClaId; //记录导致冗余的子句，在路径回退的时候不在与之归结
    set<Cla_p> setUsedCla; //已经归结过的子句，不再比较；

    //uint32_t uReduceNum;
    Formula* fol;

    bool unitResolutionByIndex(Literal* lit);
    bool unitResolutionBySet(Literal* lit, int ind = 0);
public:
    TriAlg(Formula* _fol);
    TriAlg(const TriAlg& orig);
    virtual ~TriAlg();
    /*---------------------------------------------------------------------*/
    /*                       Inline  Function                              */
    /*---------------------------------------------------------------------*/
    //

    inline void clearVect() {
        vector<ALit_p>().swap(vALitTri);
        vALitTri.reserve(32);
        vector<RLit_p>().swap(vReduceLit);
        vReduceLit.reserve(64);
        vector<Literal*>().swap(vNewR);
        vNewR.reserve(32);
        setRedundClaId.clear();
        setUsedCla.clear();


    }

    inline uint32_t getRNum() {
        return vNewR.size();
    }
    /*---------------------------------------------------------------------*/
    /*                          Member Function                            */
    /*---------------------------------------------------------------------*/
    RESULT GenerateTriByRecodePath(Clause* givenCla);
    ResRule RuleCheck(Literal*actLit, Literal* candLit, Lit_p *leftLit, uint16_t& uLeftLitInd);

    void printTri(Clause* pasLit);
    


private:

};

#endif /* TRIALG_H */

