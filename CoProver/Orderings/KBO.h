/* 
 * File:   KBO.h
 * Author: Zhong Jian<77367632@qq.com>
 * Created on 2018年3月26日, 上午11:29
 * 仿照E的Kbolin 以及文献[Loechner:JAR-2006] (Bernd Loechner, "Things to Know
  when Implementing KBO", JAR 36(4):289-310, 2006.
 * 注意KBO 序主要用途
 * 1.给等词文字的左右项排序.实现重写的重要技术
 * 2.给统一个子句中的文字进行排序.为有序归结提供序条件.
 */

#ifndef KBO_H
#define KBO_H 
#include "Terms/TermCell.h"

class KBO {
private:
    const uint32_t VAR_COMPARE_LEN = 16;
    uint32_t VAR_HASH_MASK = VAR_COMPARE_LEN - 1;
public:

    //定义一个全局的数组用来记录变元个数比较结果
    static int* globalVB;
    static uint32_t vbSize;
    static uint32_t wb; //权重大小       
    static uint32_t posBal; //计算有多少个正数
    static uint32_t negBal; //计算有多少个负数

public:

    KBO();
    KBO(const KBO& orig);
    virtual ~KBO();
public:
    CompareResult KBO6Compare(TermCell* t1, TermCell*t2, DerefType deref_t1, DerefType deref_t2);
    CompareResult KBOCompare(Term_p t1, Term_p t2, DerefType deref_t1, DerefType deref_t2);
private:
    /*---------------------------------------------------------------------*/
    /*                       Inline  Function                              */
    /*---------------------------------------------------------------------*/
    //

    inline void kbo6Reset() {
        memset(globalVB, 0, sizeof (uint32_t)*(vbSize)); //将全局数组VB清零
        wb = 0;
        posBal = 0;
        negBal = 0;
    }
    /// 符号比较
    /// \param f1
    /// \param f2
    /// \return 

    inline CompareResult SymbolCompare(FunCode f1, FunCode f2) {
        assert((f1 > 0)&&(f2 > 0));
        if (f1 == f2) {
            return CompareResult::toEqual;
        }
        if (f1 == (int) DerefType::TRUECODE) {
            return CompareResult::toLesser;
        }
        if (f2 == (int) DerefType::TRUECODE) {
            return CompareResult::toGreater;
        }
        CompareResult res = (f1 - f2) > 0 ? CompareResult::toGreater : CompareResult::toLesser;

        return res;
    }


    /*---------------------------------------------------------------------*/
    /*                          Member Function                            */
    /*---------------------------------------------------------------------*/
    CompareResult kbo6CMP(TermCell* t1, TermCell*t2, DerefType derefT1, DerefType derefT2);
    void resizeVB(uint32_t index);
    void incVB(TermCell* var);
    void decVB(TermCell* var);
    void mfyVWBInc(TermCell* term, DerefType derefT);
    void mfyVWBDec(TermCell* term, DerefType derefT);



    CompareResult KBOCompareVars(Term_p t1, Term_p t2, DerefType derefT1, DerefType derefT2);
    CompareResult KBOVarCompare(Term_p t1, Term_p t2, DerefType derefT1, DerefType derefT2);
};

#endif /* KBO_H */

