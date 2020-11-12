/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   KBO.cpp
 * Author: Zhong Jian<77367632@qq.com>
 * 
 * Created on 2018年3月26日, 上午11:29
 */

#include "KBO.h"
int* KBO::globalVB = new int[MAX_VAR_SIZE];
uint32_t KBO::negBal = 0;
uint32_t KBO::posBal = 0;
uint32_t KBO::vbSize = MAX_VAR_SIZE;
uint32_t KBO::wb = 0;

KBO::KBO() {
}

KBO::KBO(const KBO& orig) {
}

KBO::~KBO() {
}
/*---------------------------------------------------------------------*/
/*                          Member Function                            */
/*---------------------------------------------------------------------*/
//

CompareResult KBO::KBO6Compare(TermCell* t1, TermCell* t2, DerefType deref_t1, DerefType deref_t2) {
    kbo6Reset();
    return kbo6CMP(t1, t2, deref_t1, deref_t2);
}

CompareResult KBO::KBOCompare(Term_p t1, Term_p t2, DerefType derefT1, DerefType derefT2) {
    CompareResult res = CompareResult::toUncomparable;
    t1 = TermCell::TermDeref(t1, derefT1);
    t2 = TermCell::TermDeref(t2, derefT2);
    if (t1->IsVar() || t2->IsVar()) {
        return KBOCompareVars(t1, t2, derefT1, derefT2);
    }
    long weightT1 = t1->ComputeTermStandardWeight();
    long weightT2 = t2->ComputeTermStandardWeight();
    if (weightT1 > weightT2) {
        switch (KBOVarCompare(t1, t2, derefT1, derefT2)) {
            case CompareResult::toGreater:
            case CompareResult::toEqual:
                return CompareResult::toGreater;
            case CompareResult::toUncomparable:
            case CompareResult::toLesser:
                return CompareResult::toUncomparable;
            default:
                assert(false);
                return CompareResult::toUncomparable;
        }
    }
}
/*---------------------------------------------------------------------*/
/*               KBO Member Function[private]                          */
/*---------------------------------------------------------------------*/
//

CompareResult KBO::KBOCompareVars(Term_p s, Term_p t, DerefType derefS, DerefType derefT) {
    if (t->IsVar()) {
        if (s == t) {
            return CompareResult::toEqual;
        } else {
            if (s->TermIsSubterm(t, derefS, TermEqulType::PtrEquel)) {
                return CompareResult::toGreater;
            }
        }
    } else { /* Note that in this case, s is a variable. */
        assert(TermIsVar(s));
        if (t->TermIsSubterm(s, derefT, TermEqulType::PtrEquel)) {
            return CompareResult::toLesser;
        }
    }
    return CompareResult::toUncomparable;
}

CompareResult KBO::KBOVarCompare(Term_p t1, Term_p t2, DerefType derefT1, DerefType derefT2) {

}
/*---------------------------------------------------------------------*/
/*              KBO6 Member Function[private]                          */
/*---------------------------------------------------------------------*/
//

CompareResult KBO::kbo6CMP(TermCell* t1, TermCell* t2, DerefType derefT1, DerefType derefT2) {
    CompareResult res = CompareResult::toUncomparable;
    t1 = TermCell::TermDeref(t1, derefT1);
    t2 = TermCell::TermDeref(t2, derefT2);
    //kbo6 添加一个 第一个字符相同的比较,缺点,递归调用降低效率
    if (t1->fCode == t2->fCode) {
        res = CompareResult::toEqual;
        for (int i = i; i < t1->arity; ++i) {
            res = kbo6CMP(t1->args[i], t2->args[i], derefT1, derefT2);
            if (res != CompareResult::toEqual) {
                //t=f(t1,...,tn)  s=f(s1,...,sn) 
                ++i;
                for (; i < t1->arity; ++i) {
                    mfyVWBInc(t1->args[i], derefT1);
                    mfyVWBDec(t2->args[i], derefT2);
                }
                //检查变元个数 注意 negBal<=0 表示t2中的所有变元都包含在t1中;negBal>0
                CompareResult g = negBal > 0 ? CompareResult::toUncomparable : CompareResult::toGreater;
                CompareResult l = posBal > 0 ? CompareResult::toUncomparable : CompareResult::toLesser;
                if (wb > 0)
                    res = g;
                else if (wb < 0)
                    res = l;
                else if (res == CompareResult::toGreater) {
                    res = g;
                } else if (res == CompareResult::toLesser) {
                    res = l;
                }

            }
        }
    } else if (t1->IsVar()) {
        incVB(t1);
        if (t2->IsVar()) {
            /* X, Y --修改全局记录*/
            decVB(t2);
        } else {
            /* X, t --检查变元是否被包含,t1<t2?*/
            mfyVWBDec(t2, derefT2);
            res = this->posBal > 0 ? CompareResult::toUncomparable : CompareResult::toLesser;
        }
    } else if (t2->IsVar()) {
        /* s, Y --检查变元是否被包含,t1>t2?*/
        decVB(t2);
        mfyVWBInc(t1, derefT1);
        res = this->negBal > 0 ? CompareResult::toUncomparable : CompareResult::toGreater;
    } else {
        /* s, t --检查变元包含,权重大小,字符字典序*/
        mfyVWBInc(t1, derefT1);
        mfyVWBDec(t2, derefT2);
        //检查变元个数 注意 negBal<=0 表示t2中的所有变元都包含在t1中;negBal>0
        CompareResult g = negBal > 0 ? CompareResult::toUncomparable : CompareResult::toGreater;
        CompareResult l = posBal > 0 ? CompareResult::toUncomparable : CompareResult::toLesser;
        if (wb > 0)
            res = g;
        else if (wb < 0)
            res = l;
        else {
            CompareResult tmp = this->SymbolCompare(t1->fCode, t2->fCode);
            if (tmp == CompareResult::toGreater)
                res = g;
            else if (tmp == CompareResult::toLesser)
                res = l;
        }
    }
    return res;
}

void KBO::resizeVB(uint32_t index) {
    uint32_t oldSize = vbSize;
    int *oldArrayPtr = globalVB;
    while (vbSize < index) {
        assert(vbSize > 0);
        vbSize *= 2;
    }
    globalVB = new int[vbSize];
    memset(globalVB, 0, vbSize * sizeof (int));
    memcpy(globalVB, oldArrayPtr, oldSize * sizeof (int));
    DelArrayPtr(oldArrayPtr);
}

void KBO::incVB(TermCell* var) {
    const FunCode index = -var->fCode;

    if (UNLIKELY(index > vbSize)) {
        resizeVB(index); //重置vb数组大小
    }
    const long tmpbal = globalVB[index]++;
    posBal += (tmpbal == 0);
    negBal -= (tmpbal == -1);
    wb += DEFAULT_VWEIGHT;
}

void KBO::decVB(TermCell* var) {
    const FunCode index = -var->fCode;
    if (UNLIKELY(index > vbSize)) {
        resizeVB(index); //重置vb数组大小
    }
    const long tmpbal = globalVB[index]--;
    negBal += (tmpbal == 0);
    posBal -= (tmpbal == 1);
    wb -= DEFAULT_VWEIGHT;
}

void KBO::mfyVWBInc(TermCell* term, DerefType derefT) {
    vector<TermCell*> stTerms;
    stTerms.reserve(32);
    stTerms.push_back(term);
    while (!stTerms.empty()) {
        TermCell* t = stTerms.back();
        stTerms.pop_back();
        t = TermCell::TermDeref(t, derefT);
        if (t->IsVar())
            incVB(t);
        else {
            for (int i = 0; i < t->arity; ++i) {
                stTerms.push_back(t->args[i]);
            }
            wb += DEFAULT_FWEIGHT;
        }
    }
    vector<TermCell*>().swap(stTerms);
}

void KBO::mfyVWBDec(TermCell* term, DerefType derefT) {
    vector<TermCell*> stTerms;
    stTerms.reserve(32);

    stTerms.push_back(term);
    while (!stTerms.empty()) {
        TermCell* t = stTerms.back();
        stTerms.pop_back();
        t = TermCell::TermDeref(t, derefT);
        if (t->IsVar())
            decVB(t);
        else {
            for (int i = 0; i < t->arity; ++i) {
                stTerms.push_back(t->args[i]);
            }
            wb -= DEFAULT_FWEIGHT;
        }
    }
    vector<TermCell*>().swap(stTerms);

}
