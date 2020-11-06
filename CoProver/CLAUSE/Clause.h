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
enum class ClauseProp : uint32_t {
    CPIgnoreProps = 0, /* For masking propertiesout */
    CPInitial = 1, /* Initial clause */
    CPInputFormula = 2 * ClauseProp::CPInitial, /* _Really_ initial clause in TSTP sense */
    CPIsProcessed = 2 * ClauseProp::CPInputFormula, /* Clause has been processed previously (用这个属性表示子句中处理过恒真文字和相同文字)*/
    CPIsOriented = 2 * ClauseProp::CPIsProcessed, /* Term and literal comparisons are up to date(子句中文字完成排序 ) */
    CPGroundCla = 2 * ClauseProp::CPIsOriented, /* 基子句 */
    CPHasSharedVarCla = 2 * ClauseProp::CPGroundCla, /* 含共享变元子句 */
    CPCopyCla = 2 * ClauseProp::CPHasSharedVarCla,
    CPDeleteClause = 2 * CPCopyCla, /* Clause should be deleted for some reason */
    /*子句读取时候的类型*/
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


};
//子句的信息

class ClauseInfo {
public:
    string name; /* In the input file, if any */
    //const char* source; /* File name, if any */
    long line;
    long column;
    /*---------------------------------------------------------------------*/
    /*                    Constructed Function                             */
    /*---------------------------------------------------------------------*/
    //

    ClauseInfo() {
        name = "";
        line = -1;
        column = -1;
    }

    ClauseInfo(string _name, const char* _src, long _line, long _col)
    : name(_name), /*source(_src),*/ line(_line), column(_col) {
    }

    ~ClauseInfo() {
        //  DelPtr(name);
        //  DelPtr(source);
    }
    /*---------------------------------------------------------------------*/
    /*                       Inline  Function                              */
    /*---------------------------------------------------------------------*/
    //

    inline void ClauseSourceInfoPrint(FILE* out, const string&inf_lit, const string& delim) {
        string src = "unknown";
        //        if (source) {
        //            src = delim + source + delim;
        //        }
        string _name = name;
        if (name.empty()) {
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
    /*同一子句中相同变元共享同一个内存地址--而且是有序的*/
    TermBank_p claTB;
public:
    //记录变元与文字的对应关系,变元x1 有文字L1，L2，  变元索引,文字pos  [待优化] 后续可以考虑用 二进制位来简化 x|=(1<<litPos)
    map<TermCell*, set<Literal*>> mapVarTermToLitpos;
    //记录文字位置与文字的对应关系,变元x1 有文字L1，L2，  变元索引,文字pos
    map<Literal*, set<TermCell*>> mapLitposToVarTerm;
    InfereType infereType; //子句演绎类型; uint8_t

    //------句相关属性-------    
    uint16_t gapWithGoal; //记录与目标子句之间的距离 初始化为0 表示与目标子句无关 与目标子句直接互补+1 子目标+1(一层目标子句 +1)
    uint16_t maxFuncLayer, minFuncLayer; //文字中最大的函数嵌套层数  
    uint16_t negLitNo; //负文字个数
    uint16_t posLitNo; //正文字个数
    uint16_t claMinWeight, claMaxWeight; //子句中最小文字权重,最大文字权重


    uint32_t userTimes; //子句使用次数  < 2^32   
    uint32_t claWeight; //子句权重 固定不变 体现子句的稳定度(算法1.字符的简单和;2.稳定度算法),例:f(x) = 2+1=3  f(a)=4; 越大越复杂     

    float priority; //优先级 --(越大越好,目标子句为最大,与目标子句无关则为0 )

    uint32_t ident; //子句创建时确定的唯一识别子句的id   PS:一般为子句编号     
    ClauseProp properties; //子句属性 uint32_t

    //    //------ 记录第一个文字的单元下拉位置 放入文字
    uint32_t uFirstURInd;

    ClauseInfo* info; //子句信息    
    set<uint32_t> parentIds; //父子句编号集;

    Literal* literals; //文字列表   

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
    // <editor-fold defaultstate="collapsed" desc="inline 相关属性">

    inline TermBank_p GetClaTB() {
        if (claTB == nullptr)
            claTB = new TermBank(this);
        return claTB;
    }

    inline void SetClaTB(TermBank_p _tb) {

        claTB = _tb;
    }

    inline void ClearClaTB() {
        DelPtr(claTB);
    }

    inline Literal* Lits() {
        return literals;
    }

    inline uint32_t GetClaId() {
        return this->ident;
    }

    inline void GetWeight(uint64_t value) {
        assert(value>-1);
        claWeight = value;
    }

    inline uint32_t GetWeight() {
        return claWeight;
    }

    inline uint16_t LitsNumber() const {
        return posLitNo + negLitNo;
    }

    inline int GetMaxVarId() {
        int maxVarId = 0;
        if (claTB == nullptr || claTB->GetShareVar() == nullptr)
            maxVarId = 0;
        else {
            maxVarId = claTB->GetShareVar()->VarBankGetVCount();
        }
        return maxVarId;
    }


    /// 空子句
    /// \return 

    inline bool ClauseIsEmpty() {
        return 0 == LitsNumber();
    }
    /// 是否为单元子句
    /// \return 

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

    /// 是否为基子句
    /// \param isCompute 是否重新计算(默认=false)
    /// \return 

    inline bool isGroundCla(bool isCompute = false) {
        bool isGroundCla = false;
        if (isCompute) {
            if (claTB == nullptr || claTB->GetShareVar() == nullptr || claTB->GetShareVar()->VarBankGetVCount() == 0) {
                isGroundCla = true;
                this->ClauseSetProp(ClauseProp::CPGroundCla);
                this->ClauseDelProp(ClauseProp::CPHasSharedVarCla);
            } else {
                isGroundCla = false;
                this->ClauseDelProp(ClauseProp::CPGroundCla);
            }
        } else {
            isGroundCla = QueryProp(properties, ClauseProp::CPGroundCla);
        }
        return isGroundCla;
    }
    /// 是否在子句中有共享变元
    /// \return 

    inline bool HasShareVarInCla() {
        return QueryProp(properties, ClauseProp::CPHasSharedVarCla);
    }
    /// 通过变元共享列表来判断是否有共享变元存在
    /// \return 

    inline bool HasShareVarInClaByVarLst() {
        return (this->mapVarTermToLitpos.size() < this->GetMaxVarId());
    }

    inline void SetShareVarInCla() {
        DelProp(properties, ClauseProp::CPGroundCla);
        SetProp(properties, ClauseProp::CPHasSharedVarCla);
    }


    ///子句中所有变元均为独立变元

    inline bool IsAllAloneVarInCla() {
        return !HasShareVarInCla()&&!isGroundCla(true);
    }

    /// 是否为删除子句
    /// \return 

    inline bool isDel() {
        return QueryProp(properties, ClauseProp::CPDeleteClause);
    }

    /// 是否为目标子句
    /// \return 

    inline bool isGoal() { //目标子句的判断 由输入的子句标注
        return QueryProp(properties, ClauseProp::CPTypeNegConjecture);
    }
    //是否为拷贝的子句

    inline bool isCopeyCla() {
        return QueryProp(properties, ClauseProp::CPCopyCla);
    }

    inline bool isLemmaCla() {
        return QueryProp(properties, ClauseProp::CPTypeLemma);
    }
    ///设置子句拷贝属性    

    inline void SetCopyClaProp() {
        this->ClauseSetProp(ClauseProp::CPCopyCla);
    }
    ///设置子句推出的引理属性 -- 这种子句一般是在演绎过程中最后一个演绎子句所有文字被下拉nolits

    inline void SetLemmaClaProp() {
        this->ClauseSetProp(ClauseProp::CPTypeLemma);
    }
    /// 无正文字全为负文字
    /// \return 

    inline bool isNoPos() {
        return (0 == posLitNo);
    }
    /// 是否为原始子句
    /// \return 

    inline bool isOrial() {
        return this->ClauseQueryProp(ClauseProp::CPInitial);
    }

    inline bool isAxiom() {
        return this->ClauseQueryProp(ClauseProp::CPTypeAxiom);
    }

    inline void SetOrigin() {
        this->ClauseSetProp(ClauseProp::CPInitial);
    }

    inline void RemoveDuplicates() {
        if (this->LitsNumber() > 1) {
            if (Literal::EqnListRemoveDuplicates(this->literals) > 0) {
                this->RecomputeLitCounts();
            }
        }
    }

    inline void SetAllLitsHold() {
        Literal* litPtr = this->literals;
        while (litPtr) {
            litPtr->EqnSetProp(EqnProp::EPIsHold);
            litPtr->matchLitPtr = nullptr;
            litPtr = litPtr->next;
        }
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
        return 0; //((int) properties & (int) ClauseProp::CP_CSSCPA_Mask) / (int) ClauseProp::CP_CSSCPA_1;
    }
    // </editor-fold>
    /*---------------------------------------------------------------------*/
    /*                  Member Function-[public]                           */
    /*---------------------------------------------------------------------*/
    //

    inline void PutFirstLitToLast() {
        assert(literals);
        if (1 == this->LitsNumber())
            return;
        Literal* p, *q;
        q = this->literals;
        this->literals = literals->next;
        p = this->literals;
        while (p->next) {
            p = p->next;
        }
        p->next = q;
        q->next = nullptr;
    }
    /// 得到子句的最小函数嵌套层
    /// \return 

    inline uint16_t MinFuncLayer() {

        Literal* lit = this->literals;
        uint16_t minFuncLayer = lit->MaxFuncLayer();
        while (true) {
            lit = lit->next;
            if (lit == nullptr)
                break;
            minFuncLayer = MIN(minFuncLayer, lit->MaxFuncLayer());
        }
        return minFuncLayer;
    }
    /// 检查子句中是否有文字满足函数嵌套要求
    /// \return 第一个满足要求的文字， nullptr 均不符合要求

    inline Literal* CheckDepthLimit() {
        Literal* litP = this->literals;
        while (litP) {
            if (litP->CheckDepthLimit()) {
                return litP;
            }
            litP = litP->next;
        }
        return nullptr;
    }




    void RecomputeLitCounts();
    //重新绑定文字列表,并重新计算
    void bindingLits(Literal * lit);
    void bindingAndRecopyLits(const vector<Literal*>&vNewR);


    void ClausePrint(FILE* out, bool fullterms);
    void getStrOfClause(string&outStr, bool complete = true);

    void ClausePrintTPTPFormat(FILE * out);
    void ClauseTSTPPrint(FILE* out, bool fullterms, bool complete);
    void ClauseTSTPCorePrint(FILE* out, bool fullterms);
    ///重新计算子句的权重--字符权重
    void ClaRecomputStdWeight();

    void EqnListTSTPPrint(FILE* out, Literal* lst, string sep, bool fullterms);
    //得到字符串
    void getEqnListTSTP(string&outStr, string sep, bool colInfo);
    void SortLits(); //对子句中 单文字进行排序

    // void ClauseNormalizeVars(VarBank_p fresh_vars);
    //得到一个,给定一个freevar变元列表上的rename拷贝(更名所有的变元项)
    Clause * RenameCopy(Literal * except);


    //设置文字的变元共享状态--新的方法,查找对应表
    void SetEqnListVarState();
    //设置文字的变元共享状态--旧方法遍历文字和遍历子项计算
    void SetEqnListVarStateByCompute();

    uint16_t calcMaxFuncLayer() const;

    Literal * GetFirstHoldLit()const;
    Literal * FindMaxLit();
    void GetVecHoldLit(vector<Literal*>&vHoldLits)const;


    //用模板+仿函数来实现 根据制定比较规则查找最大的Literal

    template<typename FunObj, typename T >
    Literal * FileMaxLit(FunObj cmp_fun, T * index) {

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

    bool operator<(Clause * cla)const {
        return (posLitNo + negLitNo) < cla->LitsNumber();
    }

    bool operator<(Clause & cla)const {
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

