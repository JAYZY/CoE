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
#include <stdint.h>
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
    CPInputFormula = 2 * ClauseProp::CPInitial, /* _Really_ initial clause in TSTP sense */
    CPIsProcessed = 2 * ClauseProp::CPInputFormula, /* Clause has been processed previously (用这个属性表示子句中处理过恒真文字和相同文字)*/
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
public:
    uint16_t negLitNo; //负文字个数
    uint16_t posLitNo; //正文字个数    
    uint32_t ident; //子句创建时确定的唯一识别子句的id   PS:一般为子句编号     
    uint32_t weight; //子句权重
    int priority;  //优先级 -- 越大越好, 若为目标子句优先,则目标子句的优先级 一直保持最大.当然起步的时候需要策略控制避免永远都是由目标子句起步
    ClauseProp properties; //子句属性
    ClauseInfo* info; //子句信息    
    Clause* parent1; //父子句1;
    Clause* parent2; //父子句2;
    Literal* literals; //文字列表
    
    /*同一子句中相同变元共享同一个内存地址--而且是有序的*/
    TermBank_p claTB;
public:
    /*---------------------------------------------------------------------*/
    /*                    Constructed Function                             */
    /*---------------------------------------------------------------------*/
    //
    Clause();
    Clause(const Clause* orig);
    Clause(Literal* literal_s);   

    //Clause(const Clause& orig);
    //
    /// 解析成子句
    /// \param in 
    /// \param bank
    /// \return 
    void ClauseParse(Scanner* in);

    virtual ~Clause();

    //传入的文字加入到文字链中
    //void addLits(Lit_p lit)
    /*---------------------------------------------------------------------*/
    /*                       Inline  Function                              */
    /*---------------------------------------------------------------------*/
    //

    inline Literal* Lits() {
        return literals;
    }

    inline uint32_t GetClaId() {
        return this->ident;
    }

    inline void GetWeight(uint64_t value) {
        assert(value>-1);
        weight = value;
    }

    inline uint32_t GetWeight() {
        return weight;
    }

    inline uint16_t LitsNumber() const{
        return posLitNo + negLitNo;
    }

    inline bool ClauseIsEmpty() {
        return 0 == LitsNumber();
    }

    inline bool isUnit() {
        return 1 == (posLitNo + negLitNo);
    }
    //正单元子句

    inline bool IsUnitPos() {
        return (1 == posLitNo && 0 == negLitNo);
    }
    //负单元子句

    inline bool isUnitNeg() {
        return (0 == posLitNo && 1 == negLitNo);
    }
    inline bool isDel(){
        return this->ClauseQueryProp(ClauseProp::CPDeleteClause);
    }
    //modify ClauseProperties to int

    inline void ClauseDelProp(ClauseProp prop) {
        DelProp(this->properties, prop);
    }

    inline void ClauseSetProp(ClauseProp prop) {
        SetProp(this->properties, prop);
    }

    inline void ClauseSetTPTPType(ClauseProp type) {
        ClauseDelProp(ClauseProp::CPTypeMask);
        ClauseSetProp(type);
    }

    inline int ClauseQueryTPTPType() {
        return (int) properties & (int) ClauseProp::CPTypeMask;
    }

    inline bool ClauseQueryProp(ClauseProp prop) {
        return QueryProp(properties, prop);
    }

    inline int ClauseQueryCSSCPASource() {
        return ((int) properties & (int) ClauseProp::CP_CSSCPA_Mask) / (int) ClauseProp::CP_CSSCPA_1;
    }

    /*---------------------------------------------------------------------*/
    /*                  Member Function-[public]                           */
    /*---------------------------------------------------------------------*/
    //
    //重新绑定文字列表,并重新计算
    void bindingLits(Literal* lit);
    void bindingAndRecopyLits(const vector<Literal*>&vNewR);

    void ClausePrint(FILE* out, bool fullterms);
    void getStrOfClause(string&outStr, bool complete=true);
    
    void ClausePrintTPTPFormat(FILE* out);
    void ClauseTSTPPrint(FILE* out, bool fullterms, bool complete);
    void ClauseTSTPCorePrint(FILE* out, bool fullterms);
    
    void ClauseStandardWeight();    
    
    void EqnListTSTPPrint(FILE* out, Literal* lst, string sep, bool fullterms);
    //得到字符串
    void getEqnListTSTP(string&outStr, string sep, bool colInfo);
    void SortLits(); //对子句中 单文字进行排序

    void ClauseNormalizeVars(VarBank_p fresh_vars);
    //得到一个,给定一个freevar变元列表上的rename拷贝(更名所有的变元项)
    Clause* renameCopy(VarBank_p renameVarbank);

    //设置文字的变元共享状态
    void SetEqnListVarState();

    Literal* FindMaxLit();

    //用模板+仿函数来实现 根据制定比较规则查找最大的Literal

    template<typename FunObj, typename T>
    Literal* FileMaxLit(FunObj cmp_fun, T* index) {

        Lit_p handle = this->literals;
        Lit_p maxLit = handle;
        if (this->LitsNumber() > 1) {
            handle = handle->next;
            while (handle) {
                float res = cmp_fun(handle, maxLit);
                if (res > 0) {
                    maxLit = handle;
                }
                handle = handle->next;
            }
        }
        return maxLit;

    }
    /*---------------------------------------------------------------------*/
    /*                          Static Function                            */
    /*---------------------------------------------------------------------*/
    static ClauseProp ClauseTypeParse(Scanner* in, string legal_types);


    /*---------------------------------------------------------------------*/
    /*                        operator Function                            */
    /*---------------------------------------------------------------------*/
    //

    bool operator<(Clause* cla)const {
        return (posLitNo + negLitNo) < cla->LitsNumber();
    }

    bool operator<(Clause& cla)const {
        return (posLitNo + negLitNo) < cla.LitsNumber();
    }


private:

    /***************************************************************************** 
     * 解析文字,生成文字列表，EqnListParse(Scanner_p in, TB_p bank, TokenType sep)
     ****************************************************************************/
    void EqnListParse(TokenType sep);

};
typedef Clause* Cla_p;
#endif /* CLAUSE_H */

