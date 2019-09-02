/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Literal.h
 * Author: Zhong Jian<77367632@qq.com>
 *
 * Created on 2017年12月23日, 下午1:58
 */

#ifndef LITERAL_H 
#define LITERAL_H
#include <map>
//#include <bits/stdint-uintn.h>
#include "Terms/TermCell.h"
#include "Terms/TermBank.h"
#include "Global/Environment.h"

/*---------------------------------------------------------------------*/
/*                      【Literal】文字相关枚举                          */
/*---------------------------------------------------------------------*/

/* 枚举 -- 文字属性类型 */
enum class EqnProp : uint32_t {
    EPNoProps = 0, /* No properties set or selected */
    EPIsPositive = 1, /* 正文字 s=t (as opposed to s!=t) */
    EPIsMaximal = 2, /* Eqn is maximal in a clause */
    EPIsStrictlyMaximal = 4, /* Eqn is strictly maximal */
    EPIsEquLiteral = 8, /* 等词 s=t, not s=$true */
    EPIsOriented = 16, /* 有序的 s=>t  or s=t ? 该文字是有方向性的 */
    EPMaxIsUpToDate = 32, /* Orientation status is up to date */
    EPHasEquiv = 64, /* Literal has been used in multiset-comparison (and found an equivalent partner) */
    EPIsDominated = 128, /* Literal is dominated by another one */
    EPDominates = EqnProp::EPIsDominated, /* Double use of this property in potentially maximal or minimal clauses */
    EPIsUsed = 256, /* For non-injective subsumption and  pattern-generation */
    EPIsHold = 512, /* 在三角形过程中该文字是否被剩余 */
    EPIsSelected = 1024, /* For selective superpostion */
    EPIsPMIntoLit = 2048, /* For inheriting selection */
    EPFromClauseLit = 4096, /* This comes from the from clause in a paramod step */
    EPPseudoLit = 8192, /* This is a pseudo-literal that does not contribute to the semantic evaluation of the clause. */
    EPLPatMinimal = 16384, /* Eqn l=r is Pattern-Minimal */
    EPRPatMinimal = 32768, /* Eqn r=l is Pattern-Minimal */
    EPIsSplitLit = 65636 /* This literal has been introduced by splitting */

};

//文字中变元状态

enum class VarState : uint8_t {
    unknown,
    noVar,
    freeVar,
    shareVar
};

class Clause;

class Literal {
public:
    uint8_t usedCount; // 该文字在演绎中使用的次数;使用一次+1 若使用后发生冗余 则+5; 到达255 则翻转
    uint8_t pos; //在子句中的位置 一个子句中最大文字数 < 2^8=256

    VarState varState; //文字中变元状态

    uint16_t reduceTime; //在归结中消除其他文字的次数  < 2^16=65536
    EqnProp properties; /*prositive ,maximal,equational */
    TermCell* lterm; /*左文字*/
    TermCell* rterm; /*等号右边文字,若非等词,则为$True;*/
    Literal* next; /*下一个文字*/
    /*所在子句信息*/
    Clause* claPtr; //所在子句
    Literal* parentLitPtr; //父子句文字
    Literal* matchLitPtr; //主界线上配对文字
    //long weight;
    //float zjlitWight;

public:
    /*---------------------------------------------------------------------*/
    /*                    Constructed Function                             */
    /*---------------------------------------------------------------------*/
    //
    Literal();
    Literal(Scanner* in, Clause* cla);
    Literal(Term_p lt, Term_p rt, bool positive);
    Literal(const Literal& orig);


    virtual ~Literal();
    /*---------------------------------------------------------------------*/
    /*                       Inline  Function                              */
    /*---------------------------------------------------------------------*/
    //
    // <editor-fold defaultstate="collapsed" desc="Inline  Function">

    inline void EqnSetProp(EqnProp prop) {
        SetProp(this->properties, prop);
    }

    inline void EqnDelProp(EqnProp prop) {
        DelProp(this->properties, prop);
    }

    inline void EqnFlipProp(EqnProp prop) {
        FlipProp(this->properties, prop);
    }

    inline bool EqnQueryProp(EqnProp prop) {
        return QueryProp(this->properties, prop);
    }

    inline FunCode EqnGetPredCode() {
        return EqnIsEquLit() ? 0 : this->lterm->fCode;
    }

    inline uint16_t MaxFuncLayer() {
        assert(lterm->uMaxFuncLayer == lterm->TermDepth());
        assert(rterm->uMaxFuncLayer == rterm->TermDepth());
        return MAX(lterm->uMaxFuncLayer, rterm->uMaxFuncLayer) + 1;

    }
    //比较两个文字对某个属性的拥有情况一致.要么都有,要么都没有.

    inline bool EqnAreEquivProps(Literal* lit, EqnProp prop) {
        return PropsAreEquiv(this->properties, lit->properties, prop);
    }

    //比较两个文字是否是互补谓词文字.

    inline bool isComplementProps(Literal* lit) {
        return (this->IsPositive() != lit->IsPositive());
    }
    /// 两个文字是否同时为正文字或同时为负文字
    /// \param lit
    /// \return 

    inline bool isSameProps(Literal* lit) {

        return (this->IsPositive() == lit->IsPositive())&&(this->EqnIsEquLit() == lit->EqnIsEquLit());
    }

    //是否文本被强行指定(选择)

    inline bool IsSelected() {
        return EqnQueryProp(EqnProp::EPIsSelected);
    }

    inline bool IsPositive() {
        return EqnQueryProp(EqnProp::EPIsPositive);
    }

    inline bool IsNegative() {
        return !(EqnQueryProp(EqnProp::EPIsPositive));
    }

    inline bool IsMaximal() {
        return EqnQueryProp(EqnProp::EPIsMaximal);
    }

    inline bool IsOriented() {
        return EqnQueryProp(EqnProp::EPIsOriented);
    }

    //该文字是否为剩余文字

    inline bool IsHold() {
        return EqnQueryProp(EqnProp::EPIsHold);
    }

    inline bool EqnIsEquLit() {
        return EqnQueryProp(EqnProp::EPIsEquLiteral);
    }

    inline bool IsGround(bool isReCla = false) {
        //return this->lterm->TBTermIsGround() && (this->rterm->TBTermIsGround());
        if (isReCla)
            return this->lterm->IsGround() && (this->rterm->IsGround());
        return this->lterm->uVarCount + this->rterm->uVarCount == 0;
    }

    inline bool IsPropFalse() {
        return ((lterm == rterm) && IsNegative());
    }

    inline bool IsTrivial() {
        return this->lterm == this->rterm;
    }

    inline void swapSides() {
        Term_p term = this->lterm;
        this->lterm = this->rterm;
        this->rterm = term;
    }
    //交换左右项(针对等词)

    inline void swapSidesDelProp() {
        EqnDelProp(EqnProp::EPIsOriented);
        EqnDelProp(EqnProp::EPMaxIsUpToDate);
        Term_p term = this->lterm;
        this->lterm = this->rterm;
        this->rterm = term;
    }

    inline float xyWeight(TermCell* t) {
        float w = 0.0f;
        if (t->IsConst() || t->TBTermIsGround()) {
            w = 1.0f; //常元+1
        } else if (t->IsFunc()) {
            float funcW = 0.0f;
            for (int i = 0; i < t->arity; ++i) {
                funcW += xyWeight(t->args[i]);
            }
            // w=0.5f+0.5f*(funcW/(funcW+1));
            w = 0.5f + 0.5f * (funcW / t->arity);
        }
        return w;
    }

    inline float xyWeight1(TermCell* t) {
        float w = 0.0f;
        if (t->IsConst()) {
            w = 1; //常元+1
        } else if (t->IsFunc()) {
            float funcW = 0.0f;
            for (int i = 0; i < t->arity; ++i) {
                funcW += xyWeight1(t->args[i]);
            }
            if ((funcW - 0.0f) < 0.00000001 && (funcW - 0.0f) >-0.00000001)
                funcW = 0.5f; //全是变元
            w = funcW / t->arity;
        }
        return w;
    }

    inline float newWNOSameVar(TermCell* t) {
        float w = 0.0f;
        float varNum = 0.0f; //变元个数
        if (t->IsFunc()) {
            float funcW = 0.0f;
            for (int i = 0; i < t->arity; ++i) {
                TermCell* subT = t->args[i];
                if (subT->IsVar()) {
                    ++funcW;
                    ++varNum;
                } else if (subT->IsConst()) {
                    funcW += 2.0f; //常元+1
                } else {
                    funcW += newWNOSameVar(subT);
                }
            }
            w = 0.5f + 0.5f * (varNum / (1 + varNum) + funcW) / t->arity;
        } else if (t->IsVar()) {
            w = 1.0f;
        } else {
            assert(t->IsConst());
            w = 2.0f;
        }
        return w;
    }

    //w=depX+|C|+s(f)     s(f)=1+w/(w+1) 

    inline float DepFunc() {
        float w = 0.0f;
        map<int, int>varGroup;
        int varN = 0;
        if (this->lterm->IsConst() || this->lterm->TBTermIsGround()) {
            w = 2.0f;
        } else {
            if (this->EqnIsEquLit()) {
                w += ClacDepthFunc(this->lterm, varGroup, varN);
                if (this->rterm->IsConst() || this->rterm->TBTermIsGround()) {
                    if (w == 2) return 2.0f;
                    w += 2;
                } else {
                    w += ClacDepthFunc(this->rterm, varGroup, varN);
                    //w += (subVarW * 1.0f) / (subVarW + 1.0f);
                    //w += subVarW;
                }
                w += varN;
                w = 1 + w / (w + 1);

            } else {
                w += ClacDepthFunc(this->lterm, varGroup, varN);
            }
        }
        // if (varGroup[0] == 0)return 2.0f;
        float sameVarW = (varGroup[0]*1.0f) / (1.0f + varGroup[0]);
        w = WEI * sameVarW + (1 - WEI) * w;
        return w;
    }
    //f(x)=1.5  f(f(x))= 1+(1+1.5)/(2+1.5)=1.7143  f(f(f(x)))= 1+(1+1.7143)/(2+1.7143)=1.73077
    //w=depX+|C|+s(f)     s(f)=1+w/(w+1) 

    inline float ClacDepthFunc(TermCell*t, map<int, int>&varGroup, int&varN) {
        float w = 0.0f;
        if (t->IsVar()) {
            ++varN;
            if (++varGroup[t->fCode] > 1) {
                ++varGroup[0];
            }
        } else if (t->IsConst() || t->TBTermIsGround()) {
            w = 2.0f; //常元+2
        } else if (t->IsFunc()) {
            int subVarN = 0;

            for (int i = 0; i < t->arity; ++i) {
                w += ClacDepthFunc(t->args[i], varGroup, subVarN);
            }
            varN += subVarN;
            w += subVarN;
            w = 1.0f + w / (1 + w);
        }
        return w;
    }
    //w=X+depX/(depX+1)+|C|+s(f)     s(f)=1+w/(w+1) 

    inline float DepToOneFunc() {
        float w = 0.0f;
        map<int, int>varGroup;
        float varN = 0;
        if (this->lterm->IsConst() || this->lterm->TBTermIsGround()) {
            w = 2.0f;
        } else {
            if (this->EqnIsEquLit()) {
                w += ClacDepthToOneFunc(this->lterm, varGroup, varN);
                if (this->rterm->IsConst() || this->rterm->TBTermIsGround()) {
                    if (w == 2) return 2.0f;
                    w += 2;
                } else {
                    w += ClacDepthToOneFunc(this->rterm, varGroup, varN);
                    //w += (subVarW * 1.0f) / (subVarW + 1.0f);
                    //w += subVarW;
                }
                w += varN;
                w = 1 + w / (w + 1);

            } else {
                w += ClacDepthToOneFunc(this->lterm, varGroup, varN);
            }
        }
        // if (varGroup[0] == 0)return 2.0f;
        float sameVarW = (varGroup[0]*1.0f) / (1.0f + varGroup[0]);
        w = WEI * sameVarW + (1 - WEI) * w;
        return w;
    }
    //f(x)=1.5  f(f(x))= 1+(1+1.5)/(2+1.5)=1.7143  f(f(f(x)))= 1+(1+1.7143)/(2+1.7143)=1.73077
    //w=depX+|C|+s(f)     s(f)=1+w/(w+1) 

    inline float ClacDepthToOneFunc(TermCell*t, map<int, int>&varGroup, float&varN) {
        float w = 0.0f;
        if (t->IsVar()) {
            ++varN;
            if (++varGroup[t->fCode] > 1) {
                ++varGroup[0];
            }
        } else if (t->IsConst() || t->TBTermIsGround()) {
            w = 2.0f; //常元+2
        } else if (t->IsFunc()) {
            float subVarN = 0;

            for (int i = 0; i < t->arity; ++i) {
                w += ClacDepthToOneFunc(t->args[i], varGroup, subVarN);
            }
            varN += subVarN / (subVarN + 1.0f);
            w += subVarN;
            w = 1.0f + w / (1 + w);
        }
        return w;
    }

    inline float DepV() {
        map<int, int> varGroup;
        varGroup[0] = 0;
        int v = 0, numC = 0;
        if (this->lterm->IsConst() || this->lterm->TBTermIsGround()) {
            numC = 2;
        } else {
            if (this->EqnIsEquLit()) {
                CalcDepV(this->lterm, v, numC, varGroup, 1);
                if (this->rterm->IsConst() || this->rterm->TBTermIsGround()) {
                    numC += 2;
                } else
                    CalcDepV(this->rterm, v, numC, varGroup, 1);

            } else {
                CalcDepV(this->lterm, v, numC, varGroup, 0);
            }
        }
        if (v == 0) {
            assert(numC > 0);
            return 2.0f;
        }
        // float w = v*1.0f / (v + 1) + numC;        
        //return 1 + w / (w + 1);
        float w = (v + numC);
        w = 1 + w / (w + 1.0f);
        float x = (varGroup[0]*1.0f) / (varGroup[0] + 1);
        return x * WEI + (1 - WEI) * w;
    }

    inline void CalcDepV(TermCell* t, int &v, int&numC, map<int, int>&varGroup, int level = 0) {

        if (t->IsVar()) {
            v += level;
            ++varGroup[t->fCode];
            if (varGroup[t->fCode] > 1)
                ++varGroup[0];
        } else if (t->IsConst() || t->TBTermIsGround()) {
            numC += 2; //常元+2
        } else if (t->IsFunc()) {
            ++level;
            for (int i = 0; i < t->arity; ++i) {
                CalcDepV(t->args[i], v, numC, varGroup, level);
            }
        }
    }

    //w=v+|C|+f     f=w/(w+1) 

    inline float NewW() {
        float w = 0.0f;
        map<int, int>varGroup;
        varGroup[0] = 0;
        if (this->lterm->IsConst() || this->lterm->TBTermIsGround()) {
            w = 2.0f;
        } else {
            w = ClacNewW(this->lterm, varGroup);
        }
        if (this->EqnIsEquLit()) {
            if (this->rterm->IsConst() || this->rterm->TBTermIsGround()) {
                if (w == 2) return 2.0f;
                w += 2;
            } else {
                w += ClacNewW(this->rterm, varGroup);
            }
            w = 1 + w / (w + 1);
        }
        // if (varGroup[0] == 0)return 2.0f;
        //cout<<"varGroup[0]"<<varGroup[0]<<endl;
        float x = (varGroup[0]*1.0f) / (varGroup[0] + 1);
        //
        //float sameVarW = (x - varGroup.size() + 1) / (1.0f * varGroup[0]);
        w = WEI * x + (1 - WEI) * w;
        return w;
    }
    // w=v+|C|+f     f=w/(w+1) 

    inline float ClacNewW(TermCell* t, map<int, int>&varGroup) {
        float w = 0.0f;
        if (t->IsVar()) {
            varGroup[t->fCode] += 1;
            if (varGroup[t->fCode] > 1) {
                //记录相同变元的个数
                ++varGroup[0];
            }
            ++w;
        } else if (t->IsConst() || t->TBTermIsGround()) {
            w += 2.0f; //常元+2
        } else if (t->IsFunc()) {
            for (int i = 0; i < t->arity; ++i) {
                w += ClacNewW(t->args[i], varGroup);
            }
            if (w == 2 * t->arity)
                w = 2;
            else
                w = 1 + w / (w + 1);
        }
        return w;
    }

    // w=v/(v+1)+|C|+f     f=w/(w+1) 

    inline float NewW2() {
        float w;
        map<int, int>varGroup;
        int varN = 0;
        if (this->lterm->IsConst() || this->lterm->TBTermIsGround()) {
            w = 2.0f;
        } else {
            w = ClacNewW2(this->lterm, varN, varGroup);
        }
        if (this->EqnIsEquLit()) {
            if (this->rterm->IsConst() || this->rterm->TBTermIsGround()) {
                if (w == 2) return 2.0f;
                w += 2;
            } else {
                w += ClacNewW2(this->rterm, varN, varGroup);
            }
            w += (varN * 1.0f) / (varN + 1);
            w = 1 + w / (w + 1);
        }
        if (varGroup[0] == 0)return 2.0f;
        float sameVarW = (varGroup[0] - varGroup.size() + 1) / (1.0f * varGroup[0]);
        w = WEI * sameVarW + (1 - WEI) * w;
        return w;
    }
    // w=v/(v+1)+|C|+f     f=w/(w+1)

    inline float ClacNewW2(TermCell* t, int&varNum, map<int, int>&varGroup) {
        float w = 0.0f;
        if (t->IsVar()) {
            varGroup[t->fCode] += 1;
            ++varGroup[0];
            ++varNum;
            return 0.0f;
        } else if (t->IsConst() || t->TBTermIsGround()) {
            return 2.0f; //常元+2
        } else if (t->IsFunc()) {

            map<int, int>subVarGroup;
            int varN = 0;
            for (int i = 0; i < t->arity; ++i) {
                w += ClacNewW2(t->args[i], varN, subVarGroup);
            }

            if (subVarGroup[0] > 0) {
                for (auto&ele : subVarGroup) {
                    if (varGroup.find(ele.first) == varGroup.end()) {
                        varGroup[ele.first] = ele.second;
                    } else {
                        varGroup[ele.first] += ele.second;
                    }
                }
            }

            if (w == 2 * t->arity) {
                assert(varN == 0);
                return 2.0f;
            } else {
                w += varN * 1.0f / (varN + 1);
                w = 1 + w / (w + 1);
            }


        }
        return w;
    }
    //只计算相同的变元个数

    inline float SameX() {
        map<int, int>varGroup;
        CalcSameX(this->lterm, varGroup);

        if (this->EqnIsEquLit()) {
            CalcSameX(this->rterm, varGroup);
        }
        //if (varGroup[0] == 0) return this->StandardWeight();
        float sameVarW = (varGroup[0] *1.0f) / (1.0f + varGroup[0]);
        return WEI * sameVarW + (1 - WEI) * this->StandardWeight();
    }

    inline void CalcSameX(TermCell* term, map<int, int>&varGroup) {
        vector<TermCell*>vecT;
        vecT.reserve(16);
        vecT.push_back(term);
        while (!vecT.empty()) {
            TermCell* t = vecT.back();
            vecT.pop_back();
            if (t->IsVar()) {

                ++varGroup[t->fCode];
                if (varGroup[t->fCode] > 1)
                    ++varGroup[0];
            }
            for (int i = 0; i < t->arity; ++i) {
                vecT.push_back(t->args[i]);
            }
        }
    }

    inline float zxmWeight(TermCell * t) {
        float w = 0.0f;
        map<int, int> varSubGroup;
        varSubGroup.clear();
        if (t->IsFunc()) {
            float funcW = 0.0f;
            for (int i = 0; i < t->arity; ++i) {
                TermCell* subT = t->args[i];
                if (subT->IsVar()) {
                    ++funcW; //变元个数+1
                    if (varSubGroup.find(subT->fCode) == varSubGroup.end())
                        varSubGroup[subT->fCode] = 1;
                    else {
                        ++varSubGroup[subT->fCode];
                    }
                } else if (subT->IsConst()) {
                    ++funcW; //常元+1
                } else {
                    funcW += zxmWeight(t->args[i]);
                }
            }
            w = 0.5f + 0.5f * (funcW - varSubGroup.size()) / t->arity;
        } else if (t->IsVar()) {
            w = 0;
        } else {
            assert(t->IsConst());
            w = 1;
        }
        return w;
    }

    inline float zxmWeight1(TermCell* t, map<int, int>&varGroup) {
        float w = 0.0f;

        map<int, int> varSubGroup;
        varSubGroup[0] = 0;
        if (t->IsFunc()) {
            float funcW = 0.0f;
            for (int i = 0; i < t->arity; ++i) {

                funcW += zxmWeight1(t->args[i], varSubGroup);
            }
            w = 0.5f + 0.5f * (funcW + (varSubGroup[0] - varSubGroup.size() + 1)) / t->arity;
            for (auto&ele : varSubGroup) {
                if (varGroup.find(ele.first) == varGroup.end())
                    varGroup[ele.first] = ele.second;
                else
                    varGroup[ele.first] += ele.second;
            }


        } else if (t->IsVar()) {
            ++varGroup[0]; //变元个数+1
            if (varGroup.find(t->fCode) == varGroup.end())
                varGroup[t->fCode] = 1;
            else {
                ++varGroup[t->fCode];
            }
        } else {
            assert(t->IsConst());
            ++w; //常元+1
        }
        return w;
    }

    inline int computJZD() {
        map<int, int> vars;
        int w = zjJZD(this->lterm, vars);
        if (this->EqnIsEquLit()) {
            w = MAX(w, zjJZD(this->rterm, vars));
        }
        return w;
    }
    //紧致度度量 相同变元根据嵌套层数+权重  f(x1) wx1=1  f(f(x1)) wx1=2
    //找权重最大的变元

    inline int zjJZD(TermCell* t, map<int, int>&vars, int level = 1) {
        int w = 0;

        for (int i = 0; i < t->arity; ++i) {
            TermCell* subT = t->args[i];
            if (subT->IsVar()) {
                FunCode vId = subT->fCode;
                if (vars.find(vId) == vars.end()) {
                    vars[vId] = level;
                } else {
                    vars[vId] += level;
                }
                if (vars[vId] > w) w = vars[vId];
            } else if (subT->IsFunc()) {
                int subW = zjJZD(subT, vars, ++level);
                if (subW > w)w = subW;
            }

        }
        return w;
    }

    inline int ConstNum() {

        if (this->IsGround()) return 0;
        vector<TermCell*>terms;
        terms.reserve(MAX_SUBTERM_SIZE);
        terms.push_back(lterm);
        int count = 0;
        while (!terms.empty()) {
            TermCell* t = terms.back();
            terms.pop_back();
            if (t->IsConst()) {
                ++count;
            }
            for (int i = 0; i < t->arity; ++i) {
                if (t->args[i]->TBTermIsGround()) {
                    count += t->arity;
                    continue;
                }
                terms.push_back(t->args[i]);
            }
        }
        if (this->EqnIsEquLit()) {
            terms.push_back(rterm);
            while (!terms.empty()) {
                TermCell* t = terms.back();
                terms.pop_back();
                if (t->IsConst()) {
                    ++count;
                }
                for (int i = 0; i < t->arity; ++i) {
                    if (t->args[i]->TBTermIsGround()) {
                        count += t->arity;
                        continue;
                    }
                    terms.push_back(t->args[i]);
                }
            }
        }
        return count;
    }
    //简单统计变元个数

    inline int VarNum() {
        if (this->IsGround()) return 0;
        vector<TermCell*>terms;
        terms.reserve(MAX_SUBTERM_SIZE);
        terms.push_back(lterm);
        int count = 0;
        while (!terms.empty()) {
            TermCell* t = terms.back();
            terms.pop_back();
            if (t->IsVar()) {
                ++count;
            }
            for (int i = 0; i < t->arity; ++i) {
                if (t->args[i]->TBTermIsGround()) continue;
                terms.push_back(t->args[i]);
            }
        }
        if (this->EqnIsEquLit()) {
            terms.push_back(rterm);
            while (!terms.empty()) {
                TermCell* t = terms.back();
                terms.pop_back();
                if (t->IsVar()) {
                    ++count;
                }
                for (int i = 0; i < t->arity; ++i) {
                    if (t->args[i]->TBTermIsGround()) continue;
                    terms.push_back(t->args[i]);
                }
            }
        }
        return count;
    }

    inline int zjLitWeight() {
        int sumKinds = 0;
        int x[1000]{};
        int max = 0; //最大的相同次数
        vector<TermCell*> terms;
        terms.reserve(MAX_SUBTERM_SIZE);
        terms.push_back(lterm);
        while (!terms.empty()) {
            TermCell* t = terms.back();
            terms.pop_back();
            if (t->IsVar()) {
                if (x[-t->fCode] > 0) {
                    ++sumKinds;
                }
                ++x[-t->fCode];
                if (x[-t->fCode] > max)
                    max = x[-t->fCode];
            }
            for (int i = 0; i < t->arity; ++i) {
                terms.push_back(t->args[i]);
            }
        }

        if (this->EqnIsEquLit()) {
            terms.push_back(rterm);
            while (!terms.empty()) {
                TermCell* t = terms.back();
                terms.pop_back();
                if (t->IsVar()) {
                    if (x[-t->fCode] > 0) {
                        ++sumKinds;
                    }
                    ++x[-t->fCode];
                    if (x[-t->fCode] > max)max = x[-t->fCode];
                }
                for (int i = 0; i < t->arity; ++i) {
                    terms.push_back(t->args[i]);
                }
            }
        }
        // this->zjlitWight = sumKinds; //最大变元数
        return sumKinds; //返回不同变元个数(变元分组数)
    }

    inline long StandardWeight(bool isReCla = true) {
        if (isReCla)
            return (lterm->TermStandardWeight() + rterm->TermStandardWeight());
        else
            return lterm->weight + rterm->weight;
    }

    inline uint16_t TermDepth() {

        uint16_t termDepth = MAX(lterm->TermDepth(), rterm->TermDepth());
        return (this->EqnIsEquLit()) ? termDepth + 1 : termDepth;

    }

    /***************************************************************************** 
     * 将文字element 插入到文字pos的后面。Insert the element at the position defined by pos.
     ****************************************************************************/
    inline void EqnListInsertElement(Literal** pos, Literal * element) {
        element->next = *pos;
        *pos = element;
    }

    /***************************************************************************** 
     * Delete the given element from the list.
     ***************************************************************************/
    inline void EqnListDeleteElement(Literal** element) {
        Literal* handle = nullptr;
        handle = EqnListExtractElement(element);
        //释放文字对象，考虑需要重新把子句文字列表连接上
        DelPtr(handle);
        //EqnFree(handle);
    }

    /*****************************************************************************
     * 返回文字列表中的文字。Take the given element out of the list and return a pointer to it. 
     ****************************************************************************/
    inline Literal * EqnListExtractElement(Literal** element) {
        Literal* handle = *element;
        assert(handle);
        *element = handle->next;
        handle->next = nullptr;
        return handle;
    }

    /// 得到文字的输出字符串，文字位置信息+文字内容字符串
    /// \param sOut
    inline void GetLitInfoWithSelf(string&sOut,DerefType deref = DerefType::DEREF_ALWAYS) {
        this->getLitInfo(sOut);
        this->getStrOfEqnTSTP(sOut,deref);
     
    }
      /// 得到文字的输出字符串，父文字位置信息+文字内容字符串
    /// \param sOut
    inline void GetLitInfoWithParent(string&sOut,DerefType deref = DerefType::DEREF_ALWAYS) {
        this->getParentLitInfo(sOut);
        this->getStrOfEqnTSTP(sOut,deref);
        
    }
    // </editor-fold>


    /*---------------------------------------------------------------------*/
    /*                  Member Function-[private]                           */
    /*---------------------------------------------------------------------*/

private:
    // <editor-fold defaultstate="collapsed" desc="Member Function-[private]">
    /*Parse an equation with optional external sign and depending on wether FOF or CNF is being parsed.*/
    bool eqn_parse_real(Scanner* in, TermCell * *lref, TermCell * *rref, bool fof);

    /*Parse a literal without external sign assuming that _all_equational literals are prefix. 
     * Return sign. This is for TPTP*/
    bool eqn_parse_prefix(TermCell * *lref, TermCell * *rref);

    /*Parse a literal without external sign, allowing both infix and prefix notations (this is for mixed LOP)*/
    bool eqn_parse_mixfix(TermCell * *lref, TermCell * *rref);

    /* Parse a literal without external sign assuming that _all_equational literals are infix. Return sign. 
     * This is for TSTP syntax and E-LOP style.*/
    bool eqn_parse_infix(TermCell* *lref, TermCell* *rref);

    //正文字--正文字 负文字--负文字 比较
    CompareResult ComparePosToPos(Literal* posEqn);
    //正文字--负文字 比较
    CompareResult ComparePosToNeg(Literal* negEqn);
    //负文字--正文字 比较   CompareResult CompareNegToPos(Literal* posEqn);
    // </editor-fold>

public:
    /*---------------------------------------------------------------------*/
    /*                  Member Function-[public]                           */
    /*---------------------------------------------------------------------*/
    //    

    /**
     * 读取In生成文字
     * @param in
     */
    void EqnParse(Scanner* in) {
        Term_p lt = nullptr, rt = nullptr;
        //生成左右项
        bool positive = eqn_parse_real(in, &lt, &rt, false);
        EqnAlloc(lt, rt, positive);

        //改进的算法 不考虑相同变元
        //        {
        //            this->zjlitWight = this->newWNOSameVar(lt);
        //            if (this->EqnIsEquLit()) {
        //                float rweight = newWNOSameVar(rt);
        //                zjlitWight = 0.5 + 0.5 * (zjlitWight + rweight) / 2.0f; //(zjlitWight + newW(rt)) / 2.0f;// 
        //            }
        //        }
    }

    /**
     * 根据2个项生成文字
     * @param lt
     * @param rt
     * @param positive
     */
    void EqnAlloc(Term_p lt, Term_p rt, bool positive) {

        this->pos = 0;
        this->properties = EqnProp::EPIsHold;
        if (positive) { //设置正文字属性
            EqnSetProp(EqnProp::EPIsPositive);
        }
        if (rt != Env::getGTbank()->trueTerm) {//设置等词属性
            assert(rt->fCode != (FunCode) DerefType::TRUECODE);
            EqnSetProp(EqnProp::EPIsEquLiteral);
        } else {//非等词文字

            assert(rt->TermCellQueryProp(TermProp::TPPredPos));
            /*printf("# lterm->f_code: %ld <%s>\n", lterm->f_code,
              SigFindName(bank->sig,lterm->f_code));
              SigPrint(stdout,bank->sig);
              fflush(stdout); */

            assert(!lt->IsVar());
            /* TermPrint(stdout, lterm, bank->sig, DEREF_NEVER);
            printf("===");
            TermPrint(stdout, rterm, bank->sig, DEREF_NEVER);
            printf("\n"); */

            assert(Env::getSig()->SigQueryFuncProp(lt->fCode, FPPredSymbol));

            lt->TermCellSetProp(TermProp::TPPredPos);
            if (Env::getSig()->SigQueryFuncProp(lt->fCode, FPPseudoPred)) {
                EqnSetProp(EqnProp::EPPseudoLit);
            }
        }
        this->next = nullptr;
        this->lterm = lt;
        this->rterm = rt;

#ifndef NDEBUG
        //   EqnPrint(stdout, false, true);
        //  printf("\n");
#endif
    }

    VarState getVarState();
    bool IsShareVar(Literal* litA);
    TermBank_p getClaTermBank();


    /*****************************************************************************
     * Print a literal in TSTP format.
     ****************************************************************************/
    void EqnTSTPPrint(FILE* out, bool fullterms, DerefType deref = DerefType::DEREF_ALWAYS);
    void getStrOfEqnTSTP(string&outStr, DerefType deref = DerefType::DEREF_ALWAYS);

    //返回父文字的信息
    void getParentLitInfo(string& parentLitInfo);
    const char* getLitInfo(string& strLitInfo);

    bool EqnOrient();

    Literal * EqnListCopyDisjoint();
    Literal * EqnListFlatCopy();

    Literal * RenameCopy(Clause * newCla, DerefType deref = DerefType::DEREF_ALWAYS);

    Literal * EqnCopyDisjoint();
    //  Literal * EqnCopy(TermBank_p termbank);
    Literal * EqnFlatCopy();
    Literal * EqnCopyOpt();




    /// Return a reasonably initialized term cell for shared terms.
    /// \param EqnDirIsReverse 文字读取方向是否为反向读取 /* Which way to read an equation */   
    /// \return 
    Term_p EqnTermsTBTermEncode(bool EqnDirIsReverse = false);

    ///  Parse a literal in FOF format (changes syntax for TPTP literals).
    /// \param in
    /// \param bank
    /// \return 

    void EqnFOFParse(Scanner* in, TermBank_p bank);

    bool equalsStuct(Literal * lit);
    //根据选定的启发式策略来判断两个文字的比较结果
    CompareResult Compare(Literal * lit);

    /*---------------------------------------------------------------------*/
    /*                          Static Function                            */
    /*---------------------------------------------------------------------*/
    //
    static void EqnListFree(Literal * lst);


};
typedef Literal *Lit_p;
#endif /* LITERAL_H */

