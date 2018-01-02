/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Clause.h
 * Author: Zhong Jian<77367632@qq.com>
 *
 * Created on 2017年12月22日, 下午8:41
 */

#ifndef CLAUSE_H
#define CLAUSE_H
#include "Global/IncDefine.h"
#include "Literal.h"
#include "INOUT/Scanner.h" 
using namespace std;
/*---------------------------------------------------------------------*/
/*                      【clause】子句相关枚举                           */
/*---------------------------------------------------------------------*/

/* 枚举 -- 子句属性类型 */
enum class ClauseProp {
    CPIgnoreProps = 0, /* For masking propertiesout */
    CPInitial = 1, /* Initial clause */
    CPInputClause = 2 * ClauseProp::CPInitial, /* _Really_ initial clause in TSTP sense */
    CPIsProcessed = 2 * ClauseProp::CPInputClause, /* Clause has been processed previously */
    CPIsOriented = 2 * ClauseProp::CPIsProcessed, /* Term and literal comparisons are up to date */
    CPIsDIndexed = 2 * ClauseProp::CPIsOriented, /* Clause is in the demod_index of its set */
    CPIsSIndexed = 2 * ClauseProp::CPIsDIndexed, /* Clause is in the fvindex of its set */
    CPIsGlobalIndexed = 2 * ClauseProp::CPIsSIndexed, /* Clause is in the Subterm FPIndex  */
    CPRWDetected = 2 * ClauseProp::CPIsGlobalIndexed, /* Rewritability of the clause has been established. Temporary property. */
    CPDeleteClause = 2 * ClauseProp::CPRWDetected, /* Clause should be deleted for some reason */
    CPType1 = 2 * ClauseProp::CPDeleteClause, /* Three bits used to encode the Clause type, taken from TPTP or  TSTP input format or assumed */
    CPType2 = 2 * ClauseProp::CPType1,
    CPType3 = 2 * ClauseProp::CPType2,
    CPTypeMask = ClauseProp::CPType1 | ClauseProp::CPType2 | ClauseProp::CPType3,
    CPTypeUnknown = 0, /* Also used as wildcard */
    CPTypeAxiom = ClauseProp::CPType1, /* Clause is Axiom */
    CPTypeHypothesis = ClauseProp::CPType2, /* Clause is Hypothesis */
    CPTypeConjecture = ClauseProp::CPType1 | ClauseProp::CPType2, /* Clause is Conjecture */
    CPTypeLemma = ClauseProp::CPType3, /* Clause is Lemma */
    CPTypeNegConjecture = ClauseProp::CPType1 | ClauseProp::CPType3, /* Clause is an negated conjecture (used for refutation) */
    CPTypeQuestion = ClauseProp::CPType2 | ClauseProp::CPType3, /* `Clause is a question only used for FOF, really. */
    CPTypeWatchClause = ClauseProp::CPType1 | ClauseProp::CPType2 | ClauseProp::CPType3, /* Clause is intended as a watch list clause */
    CPIsIRVictim = 2 * ClauseProp::CPType3, /* Clause has just been simplified in interreduction */
    CPOpFlag = 2 * ClauseProp::CPIsIRVictim, /* Temporary marker */
    CPIsSelected = 2 * ClauseProp::CPOpFlag, /* For analysis of selected clauses only */
    CPIsFinal = 2 * ClauseProp::CPIsSelected, /* Clause is a final clause, i.e. a clause that might be used by a postprocessor. */
    CPIsProofClause = 2 * ClauseProp::CPIsFinal, /* Clause is part of a successful proof. */
    CPIsSOS = 2 * ClauseProp::CPIsProofClause, /* Clause is in the set of support.*/
    CPNoGeneration = 2 * ClauseProp::CPIsSOS, /* No generating inferences with this clause are necessary */
    CP_CSSCPA_1 = 2 * ClauseProp::CPNoGeneration, /* CSSCPA clause sources */
    CP_CSSCPA_2 = 2 * ClauseProp::CP_CSSCPA_1,
    CP_CSSCPA_4 = 2 * ClauseProp::CP_CSSCPA_2,
    CP_CSSCPA_8 = 2 * ClauseProp::CP_CSSCPA_4,
    CP_CSSCPA_Mask = ClauseProp::CP_CSSCPA_1 | ClauseProp::CP_CSSCPA_2 | ClauseProp::CP_CSSCPA_4 | ClauseProp::CP_CSSCPA_8,
    CP_CSSCPA_Unkown = 0,
    CPIsProtected = 2 * ClauseProp::CP_CSSCPA_8, /* Unprocessed clause has been used in simplification and cannot be deleted even if parents die. */
    CPWatchOnly = 2 * ClauseProp::CPIsProtected,
    CPSubsumesWatch = 2 * ClauseProp::CPWatchOnly,
    CPLimitedRW = 2 * ClauseProp::CPSubsumesWatch, /* Clause has been processed and hence can only be rewritten in limited ways. */
    CPIsRelevant = 2 * ClauseProp::CPLimitedRW /* Clause is selected as relevant for a proof attempt (used by SInE). */
};
//子句的信息

class ClauseInfo {
public:
    const char* name; /* In the input file, if any */
    const char* source; /* File name, if any */
    long line;
    long column;
    /*---------------------------------------------------------------------*/
    /*                    Constructed Function                             */
    /*---------------------------------------------------------------------*/
    //

    ClauseInfo() {
        name = nullptr;
        source = nullptr;
        line = -1;
        column = -1;
    }

    ClauseInfo(const char* _name, const char* _src, long _line, long _col)
    : name(_name), source(_src), line(_line), column(_col) {
    }

    ~ClauseInfo() {
    }
    /*---------------------------------------------------------------------*/
    /*                       Inline  Function                              */
    /*---------------------------------------------------------------------*/
    //

    inline void ClauseSourceInfoPrint(FILE* out, const string&inf_lit, const string& delim) {
        string src = "unknown";
        if (source) {
            src = delim + source + delim;
        }
        string _name = name;
        if (!name) {
            if (line < 0) {
                _name = "unknown";
            } else {
                assert(column >= 0);
                char* buffer = new char();
                snprintf(buffer, 50, "at_line_%ld_column_%ld", line, column);
                _name = buffer; //= "at_line_" + to_string(line) + "_colum_" + to_string(column);
            }
        }
        fprintf(out, "%s(%s, %s)", inf_lit.c_str(), src.c_str(), _name.c_str());
    }

    inline void ClauseSourceInfoPrintTSTP(FILE* out) {
        ClauseSourceInfoPrint(out, "file", "'");
    }

    inline void ClauseSourceInfoPrintPCL(FILE* out) {
        ClauseSourceInfoPrint(out, "initial", "\"");
    }
};

class Clause {
private:
    uint64_t ident; //子句创建时确定的唯一识别子句的id    
    ClauseProp properties; //子句属性
    ClauseInfo* info; //子句信息
    Literal* literals; //文字列表
    uint16_t negLitNo; //负文字个数
    uint16_t posLitNo; //正文字个数
    uint64_t weight; //子句权重

    Clause* parent1; //父子句1;
    Clause* parent2; //父子句2;

public:

    Clause();
    Clause(Literal* literal_s);
    //Clause(const Clause& orig);
    virtual ~Clause();
    /*---------------------------------------------------------------------*/
    /*                       Inline  Function                              */
    /*---------------------------------------------------------------------*/
    //

    inline void getWeight(uint64_t value) {
        assert(value>-1);
        weight = value;
    }

    uint64_t getWeight() {
        return weight;
    }
    /*---------------------------------------------------------------------*/
    /*                  Member Function-[public]                           */
    /*---------------------------------------------------------------------*/
    //
   static ClauseProp ClauseTypeParse(Scanner* in, string &legal_types) ;
    
    //
    /// 解析成子句
    /// \param in 
    /// \param bank
    /// \return 
    static Clause* ClauseParse( );
    
    
    
private:

};

#endif /* CLAUSE_H */

