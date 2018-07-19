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

#include "TERMS/TermCell.h"
#include "TERMS/TermBank.h"
#include "Global/Environment.h"
/*---------------------------------------------------------------------*/
/*                      【Literal】文字相关枚举                          */
/*---------------------------------------------------------------------*/

/* 枚举 -- 文字属性类型 */
enum class EqnProp : uint32_t {
    EPNoProps = 0, /* No properties set or selected */
    EPIsPositive = 1, /* s=t (as opposed to s!=t) */
    EPIsMaximal = 2, /* Eqn is maximal in a clause */
    EPIsStrictlyMaximal = 4, /* Eqn is strictly maximal */
    EPIsEquLiteral = 8, /* s=t, not s=$true */
    EPIsOriented = 16, /* s=>t  or s=t ? 该文字是有方向性的 */
    EPMaxIsUpToDate = 32, /* Orientation status is up to date */
    EPHasEquiv = 64, /* Literal has been used in	 multiset-comparison (and found an equivalent partner) */
    EPIsDominated = 128, /* Literal is dominated by another one */
    EPDominates = EqnProp::EPIsDominated, /* Double use of this property in potentially maximal or minimal clauses */
    EPIsUsed = 256, /* For non-injective subsumption and  pattern-generation */
    EPGONatural = 512, /* Set if left side is bigger in the special (total) ground ordering treating variables as small constants */
    EPIsSelected = 1024, /* For selective superpostion */
    EPIsPMIntoLit = 2048, /* For inheriting selection */
    EPFromClauseLit = 4096, /* This comes from the from clause in a paramod step */
    EPPseudoLit = 8192, /* This is a pseudo-literal that does not contribute to the semantic evaluation of the clause. */
    EPLPatMinimal = 16384, /* Eqn l=r is Pattern-Minimal */
    EPRPatMinimal = 32768, /* Eqn r=l is Pattern-Minimal */
    EPIsSplitLit = 65636 /* This literal has been introduced by splitting */
};

class Literal {
public:
    EqnProp properties; /*prositive ,maximal,equational */
    int pos;
    TermCell* lterm; /*左文字*/
    TermCell* rterm; /*等号右边文字,若非等词,则为$True;
    //TermBank ss;
    //TermBank* bank; /* Terms are from this bank */
    Literal* next; /*下一个文字*/

    /*所在子句信息*/
    Clause* claPtr; //所在子句
    Literal* parentLitPtr; //父子句文字
    float xyW;
    float zjlitWight;
public:
    Literal();
    Literal(Term_p lt, Term_p rt, bool positive);
    Literal(const Literal& orig);
    //替代 EqnAlloc

    virtual ~Literal();
    /*---------------------------------------------------------------------*/
    /*                       Inline  Function                              */
    /*---------------------------------------------------------------------*/
    //

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

    inline bool EqnIsPositive() {
        return EqnQueryProp(EqnProp::EPIsPositive);
    }

    inline bool EqnIsNegative() {
        return !(EqnQueryProp(EqnProp::EPIsPositive));
    }

    inline bool EqnIsMaximal() {
        return EqnQueryProp(EqnProp::EPIsMaximal);
    }

    inline bool EqnIsOriented() {
        return EqnQueryProp(EqnProp::EPIsOriented);
    }

    inline bool EqnIsEquLit() {
        return EqnQueryProp(EqnProp::EPIsEquLiteral);
    }

    inline bool EqnIsGround() {
        return this->lterm->TBTermIsGround() && (this->rterm->TBTermIsGround());
    }

    inline bool EqnIsPropFalse() {
        return ((lterm == rterm) && EqnIsNegative());
    }

    inline bool EqnIsTrivial() {
        return this->lterm == this->rterm;
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
        float w=0.0f;        
        map<int, int>varGroup;
        if (this->lterm->IsConst() || this->lterm->TBTermIsGround()) {
            w = 2.0f;
        } else {
            if (this->EqnIsEquLit()) {
                w += ClacDepthFunc(this->lterm, varGroup, 1);
                if (this->rterm->IsConst() || this->rterm->TBTermIsGround()) {
                    if (w == 2) return 2.0f;
                    w += 2;
                } else {
                    w += ClacDepthFunc(this->rterm, varGroup, 1);
                    //w += (subVarW * 1.0f) / (subVarW + 1.0f);
                    //w += subVarW;
                }
                 w = 1 + w / (w + 1);
                 
            } else {
                w += ClacDepthFunc(this->lterm, varGroup, 0);
             }
        }
        if (varGroup[0] == 0)return 2.0f;
        float sameVarW = (varGroup[0] - varGroup.size() + 1) / (1.0f * varGroup[0]);
        w = WEI * sameVarW + (1 - WEI) * w;
        return w;
    }
    //f(x)=1.5  f(f(x))= 1+(1+1.5)/(2+1.5) 
    inline float ClacDepthFunc(TermCell*t, map<int, int>&varGroup, int level = 0) {
        float w = 0.0f;
        if (t->IsVar()) {
            w = level;
            ++varGroup[0];
            ++varGroup[t->fCode];
        } else if (t->IsConst() || t->TBTermIsGround()) {
            w = 2.0f; //常元+2
        } else if (t->IsFunc()) {            
            ++level;
            for (int i = 0; i < t->arity; ++i) {
                w += ClacDepthFunc(t->args[i], varGroup, level);
            }
              
            //if (w == 2 * t->arity)return 2.0f;
            // w += (subVarW * 1.0f) / (subVarW + 1.0f);
            //w += subVarW;
             w = 1.0f + w / (1 + w);
         }
        return w;
    }
    /* inline float ClacDepthFunc(TermCell*t, map<int, int>&varGroup, int &varW, int level = 0) {
            float w = 0.0f;
            if (t->IsVar()) {
                varW += level;
                ++varGroup[0];
                ++varGroup[t->fCode];
            } else if (t->IsConst() || t->TBTermIsGround()) {
                w = 2.0f; //常元+2
            } else if (t->IsFunc()) {
                int subVarW = 0;
                ++level;
                for (int i = 0; i < t->arity; ++i) {
                    w += ClacDepthFunc(t->args[i], varGroup, subVarW, level);
                }
                if (w == 2 * t->arity)return 2.0f;
               // w += (subVarW * 1.0f) / (subVarW + 1.0f);
                w+=subVarW;
                w = 1.0f + w / (1 + w);
            }
            return w;
        }*/

    //w=depX+|C|     w/(w+1) 

    inline float DepV() {
        int v = 0, numC = 0;
        if (this->lterm->IsConst() || this->lterm->TBTermIsGround()) {
            numC = 2;
        } else {
            if (this->EqnIsEquLit()) {
                CalcDepV(this->lterm, v, numC, 1);
                if (this->rterm->IsConst() || this->rterm->TBTermIsGround()) {
                    numC += 2;
                } else
                    CalcDepV(this->rterm, v, numC, 1);

            } else {
                CalcDepV(this->lterm, v, numC, 0);
            }
        }
        if (v == 0) {
            assert(numC > 0);
            return 2.0f;
        }
        // float w = v*1.0f / (v + 1) + numC;        
        //return 1 + w / (w + 1);
        float w = (v * 1.0f + numC);
        return 1 + w / (w + 1);
    }

    inline void CalcDepV(TermCell* t, int &v, int&numC, int level = 0) {

        if (t->IsVar())
            v += level;
        else if (t->IsConst() || t->TBTermIsGround()) {
            numC += 2; //常元+2
        } else if (t->IsFunc()) {
            ++level;
            for (int i = 0; i < t->arity; ++i) {
                CalcDepV(t->args[i], v, numC, level);
            }
        }
    }

    //w=v+|C|+f     f=w/(w+1) 

    inline float NewW() {
        float w=0.0f;
        map<int, int>varGroup;
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
        if (varGroup[0] == 0)return 2.0f;
        float sameVarW = (varGroup[0] - varGroup.size() + 1) / (1.0f * varGroup[0]);
        w = WEI * sameVarW + (1 - WEI) * w;
        return w;
    }
    // w=v+|C|+f     f=w/(w+1) 

    inline float ClacNewW(TermCell* t, map<int, int>&varGroup) {
        float w = 0.0f;
        if (t->IsVar()) {
            varGroup[t->fCode] += 1;
            ++varGroup[0];
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

    inline float zxmWeight(TermCell* t) {
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

        if (this->EqnIsGround()) return 0;
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
        if (this->EqnIsGround()) return 0;
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
        this->zjlitWight = sumKinds; //最大变元数
        return sumKinds; //返回不同变元个数(变元分组数)
    }

    inline long StandardWeight() {
        return lterm->TermStandardWeight() + rterm->TermStandardWeight();
    }

    inline long EqnDepth() {
        return lterm->TermDepth() + rterm->TermDepth();
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

    /***************************************************************************** 
     * 解析文字,生成文字列表，EqnListParse(Scanner_p in, TB_p bank, TokenType sep)
     ****************************************************************************/
    inline void EqnListParse(TokenType sep) {

        Scanner* in = Env::getIn();
        TB_p bank = Env::getTb();

        TokenType testTok = (TokenType) ((uint64_t) TokenCell::TermStartToken() | (uint64_t) TokenType::TildeSign);

        if (((in->format == IOFormat::TPTPFormat) && in->TestInpTok(TokenType::SymbToken)) ||
                ((in->format == IOFormat::LOPFormat) && in->TestInpTok(testTok)) ||
                ((in->format == IOFormat::TSTPFormat) && in->TestInpTok(testTok))) {

            //单个文字 解析&生成
            this->EqnParse(in, bank);

            Literal* handle = this;
            while (in->TestInpTok(sep)) {
                in->NextToken();
                //此处过滤项list = EqnParse(in, bank);
                handle->next = new Literal();
                handle->next->EqnParse(in, bank);
                handle = handle->next;
            }
        }
        //return list;
    }

    /*---------------------------------------------------------------------*/
    /*                  Member Function-[private]                           */
    /*---------------------------------------------------------------------*/
private:

    /*Parse an equation with optional external sign and depending on wether FOF or CNF is being parsed.*/
    bool eqn_parse_real(Scanner* in, TermBank* bank, TermCell * *lref, TermCell * *rref, bool fof);

    /*Parse a literal without external sign assuming that _all_equational literals are prefix. 
     * Return sign. This is for TPTP*/
    bool eqn_parse_prefix(TermCell * *lref, TermCell * *rref);

    /*Parse a literal without external sign, allowing both infix and prefix notations (this is for mixed LOP)*/
    bool eqn_parse_mixfix(TermCell * *lref, TermCell * *rref);

    /* Parse a literal without external sign assuming that _all_equational literals are infix. Return sign. 
     * This is for TSTP syntax and E-LOP style.*/
    bool eqn_parse_infix(TermCell* *lref, TermCell* *rref);
public:
    /*---------------------------------------------------------------------*/
    /*                  Member Function-[public]                           */
    /*---------------------------------------------------------------------*/
    //    

    void EqnParse(Scanner* in, TermBank* bank) {

        Term_p lt = nullptr, rt = nullptr;
        bool positive = eqn_parse_real(in, bank, &lt, &rt, false);
        EqnAlloc(lt, rt, bank, positive);

        //计算徐杨稳定度
        xyW = xyWeight(lt);
        if (this->EqnIsEquLit()) {
            xyW = (xyW + xyWeight(rt)) / 2.0f;
        }
        //计算钟小梅稳定度
        //        map<int, int>varGroup;
        //        xyW = zxmWeight(lt);
        //        if (this->EqnIsEquLit()) {
        //            xyW = 0.5 + 0.5 * (xyW + zxmWeight(rt)) / 2.0f;
        //        }
        //改进算法2
        //        {
        //
        //            map<int, int>varGroup;
        //            varGroup[0] = 0;
        //            int level = this->EqnIsEquLit() ? 1 : 0;
        //            int subVarW = 0;
        //            float funcW = 0.0f;
        //            if (lt->IsConst() || lt->TBTermIsGround()) {
        //                funcW = 2.0f;
        //            } else {
        //                funcW = newDepth(lt, varGroup, subVarW, level);
        //            }
        //
        //            if (this->EqnIsEquLit()) {
        //                if (rt->IsConst() || rt->TBTermIsGround()) {
        //                    funcW += 2;
        //                    if (4 == funcW)
        //                        funcW = 2;
        //                } else {
        //                    funcW += newDepth(rt, varGroup, subVarW, level);
        //                    funcW += (subVarW*1.0f) / (subVarW + 1.0f);
        //                    funcW = 1.0f+funcW / (funcW + 1.0f);                     
        //                }
        //            }
        //            float sameVarW = (varGroup[0] == 0) ? 1 : (varGroup[0] - varGroup.size() + 1) / (1.0f * varGroup[0]);
        //            zjlitWight = WEI * sameVarW + (1 - WEI)*(funcW);
        //        }
        //改进变元函数嵌套算法        

        //zjlitWight = DepV();
       //  zjlitWight = DepFunc();
         zjlitWight = NewW();
        // zjlitWight = NewW2();
        //改进的稳定度算法1
        /*{
            map<int, int>varGroup;
            varGroup[0] = 0;
            //float weight = ((lt->IsConst() || lt->TBTermIsGround())) ? 2.0f : newW(lt, varGroup);

            if (this->EqnIsEquLit()) {
                map<int, int>subVarGroup;
                subVarGroup[0] = 0;
               // weight += newW(rt, subVarGroup);
                weight = 1.0f + (weight / (1 + weight));

                if (subVarGroup[0] > 0) {
                    for (auto&ele : subVarGroup) {
                        if (varGroup.find(ele.first) == varGroup.end()) {
                            varGroup[ele.first] = ele.second;
                        } else {
                            varGroup[ele.first] += ele.second;
                        }
                    }
                }
            }
            if (varGroup[0] == 0)
                zjlitWight = 2.0f;
            else {
                float sameVarW = (varGroup[0] - varGroup.size() + 1) / (1.0f * varGroup[0]);
                zjlitWight = WEI * sameVarW + (1 - WEI) * weight;
            }

        }*/

        //(zjlitWight + newW(rt)) / 2.0f;// 

        //改进的算法 不考虑相同变元
        //        {
        //            this->zjlitWight = this->newWNOSameVar(lt);
        //            if (this->EqnIsEquLit()) {
        //                float rweight = newWNOSameVar(rt);
        //                zjlitWight = 0.5 + 0.5 * (zjlitWight + rweight) / 2.0f; //(zjlitWight + newW(rt)) / 2.0f;// 
        //            }
        //        }
    }

    void EqnAlloc(Term_p lt, Term_p rt, TermBank* bank, bool positive) {

        this->pos = 0;
        this->properties = EqnProp::EPNoProps;

        if (positive) { //设置正文字属性
            EqnSetProp(EqnProp::EPIsPositive);
        }
        if (rt != bank->trueTerm) {//设置等词属性
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
    /*****************************************************************************
     * Print a literal in TSTP format.
     ****************************************************************************/
    void EqnTSTPPrint(FILE* out, bool fullterms);

    Literal * EqnListCopyDisjoint();
    Literal * EqnListFlatCopy();
    Literal * EqnCopyDisjoint();

    Literal * EqnCopy();
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

    void EqnFOFParse(Scanner* in, TB_p bank);
    /*---------------------------------------------------------------------*/
    /*                          Static Function                            */
    /*---------------------------------------------------------------------*/
    static void EqnListFree(Literal * lst);


};

#endif /* LITERAL_H */

