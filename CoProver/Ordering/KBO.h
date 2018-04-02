/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   KBO.h
 * Author: Zhong Jian<77367632@qq.com>
 *
 * Created on 2018年3月26日, 上午11:29
 */

#ifndef KBO_H
#define KBO_H
#include "TERMS/TermCell.h"

 
class KBO {
private:
    const uint32_t VAR_COMPARE_LEN = 16;
    uint32_t VAR_HASH_MASK = VAR_COMPARE_LEN - 1;

public:
    KBO();
    KBO(const KBO& orig);
    virtual ~KBO();
private:

    uint32_t getVarIndex(TermCell*var) {
        assert(var->fCode < 0);
        return (-(var->fCode) & VAR_HASH_MASK);
    }

    void setVarIndex(TermCell* t, uint32_t *varIndex, uint32_t value) {

        vector<TermCell*> vecTerm;
        vecTerm.reserve(16);
        vecTerm.push_back(t);
        TermCell* term = nullptr;
        while (!vecTerm.empty()) {
            term = vecTerm.back();
            vecTerm.pop_back();
            if (term->IsVar()) {
                uint32_t ind = getVarIndex(term);
                varIndex[ind] += value;
            } else {
                for (uint32_t i = 0; i < term->arity; ++i) {
                    vecTerm.push_back(term->args[i]);
                }
            }
        }
    }

    CompareResult varCheck(TermCell* s, TermCell* t, uint16_t index) {
        uint32_t *varIndex = new uint32_t[VAR_COMPARE_LEN]();

        setVarIndex(s, varIndex, 1);
        setVarIndex(t, varIndex, -1);
        CompareResult res = CompareResult::toUnknown;
        for (uint16_t i = 1; i < VAR_COMPARE_LEN; ++i) {
            if (varIndex[i] > 0) {
                res = CompareResult::toGreater;
                break;
            } else if (varIndex[i] < 0) {
                res = CompareResult::toLesser;
                break;
            }
        }
         DelArrayPtr(varIndex);
        return res;
    }

};

#endif /* KBO_H */

