/*
 *Contents:
 *  Definitions for term banks - i.e. shared representations of terms as defined in cte_terms.h. 
 *  Uses the same struct, but adds administrative stuff and functionality for sharing.
 *  
 *  There are two sets of funktions for the manangment of term trees:
 *  Funktions operating only on the top cell, and functions descending
 *  the term structure. Top level functions implement a conventional splay 
 *  tree with key f_code.masked_properties.entry_nos_of_args and are implemented in cte_termtrees.[ch]
 * 
 * File:   TermBank.h
 * Author: zj 
 *
 * Created on 2017年2月24日, 下午4:46
 */

#ifndef TERMBANK_H
#define TERMBANK_H
#include "VarBank.h"
#include "TermCellStore.h"
#include "INOUT/Scanner.h"
#include "TermCell.h"

class TermBank {
private:
    static bool TBPrintInternalInfo;
    static bool TBPrintDetails;
public:
    unsigned long inCount; /* TermBank中项个数统计 不需要!!! -- How many terms have been inserted? */
    //Sig_p sig; /* Store sig info */
    VarBank* vars; /* 共享变元存储对象 -- Information about (shared) variables */
    TermCell* trueTerm; /* 特殊项$true -- Pointer to the special term with the $true constant. */
    TermCell* falseTerm; /* 特殊项$false -- Pointer to the special term with the $false constant. */
    TermCell* minTerm; /* A small (ideally the minimal possible) term, to be used for RHS instantiation. */
    unsigned long rewriteSteps; /* 统计TBTermReplace 调用次数;How many calls to TBTermReplace? */
    SplayTree<PTreeCell>freeVarSets; /*项中的自由变元,不能共享的. Associates a term (or Tformula) with the set of its free variables.
                                        * Only initalized for specific operations and then reset again */
    TermProp garbageState; /* For the mark-and sweep garbage collection.
                            * This is flipped at each sweep,and all new term cell get the new value,
                            * so that marking can be done by flipping in the term cell. */

    /*struct gc_admin_cell *gc;  Higher level code can register garbage collection information here. 
     * This is only a convenience link, memory needs to be managed elsewhere. */
    vector<TermCell*> extIndex; /* 用来存储项的缩写形式, 可以考虑删除 -- Associate _external_ abbreviations (= entry_no's with term nodes, necessary
		  	    for parsing of term bank terms. For critical cases (full protocolls) this
  			    is bound to be densly poulated -> we use an array. Please note that term
			    replacing does not invalidate entries in ext_index(it would be pretty expensive in terms of time and memory), so higher
			   layers have to take care of this if they want to both access terms via references and do replacing! */

    TermCellStore termStore; /*存放Term的hash结构 -- Here are the terms */

public:
    /*---------------------------------------------------------------------*/
    /*                    Constructed Function                             */
    /*---------------------------------------------------------------------*/
    TermBank(); //Sig_p sig);
    TermBank(const TermBank& orig);
    virtual ~TermBank();

    /*---------------------------------------------------------------------*/
    /*                       Inline Function                               */
    /*---------------------------------------------------------------------*/

    /* 返回非变元项个数 */
    inline long TBNonVarTermNodes() {
        return termStore.entries;
    }

    /* 返回神仙数字 项个数+函数项的元个数 */
    inline long TBStorage() {
        return ( sizeof (TermCell) + 4 * sizeof (void*))* termStore.entries + termStore.argCount * sizeof (TermCell*);
        //return TERMCELL_DYN_MEM * termStore.entries + termStore.argCount*sizeof(TermCell*);
    }

    /* 返回项是否为被标注- marked */
    inline bool TBTermCellIsMarked(TermCell* t) {
        return GiveProps(t->properties, TermProp::TPGarbageFlag) != garbageState;
    }

    inline void TBPrintTermFull(FILE* out, TermCell* term) {
        term->TermPrint(out,DerefType::DEREF_NEVER);
    }

    static inline bool TBTermEqual(TermCell* t1, TermCell* t2) {
        return (t1 == t2);
    }

private:
    /*---------------------------------------------------------------------*/
    /*                    Member Function[private]                         */
    /*---------------------------------------------------------------------*/
    void tb_print_dag(FILE *out, NumTree_p spNode );

    /* 转换一个子项  i.e. a term which cannot start with a predicate symbol. */
    TermCell* tb_subterm_parse(Scanner* in);

    /*转换一个项列表 Parse a list of terms  */
    int tb_term_parse_arglist(Scanner* in, TermCell*** arg_anchor, bool isCheckSymbProp);

    /* Parse a LOP list into an (shared) internal $cons list. */
    TermCell* tb_parse_cons_list(Scanner* in, bool isCheckSymbProp);

    TermCell* tb_termtop_insert(TermCell* t);

public:
    /*---------------------------------------------------------------------*/
    /*                          inline Function                            */
    /*---------------------------------------------------------------------*/
    //

    inline long TBStorageMEM() {
        return sizeof (TermBank) + sizeof (TermCell) * termStore.entries +
                termStore.argCount * sizeof (Term_p);
    }
    /* 插入一个项到termbank中.但该项的子项已经存在,重用或删除 top cell! */
    TermCell* TBTermTopInsert(TermCell* t);

    /* Return a reasonably initialized term cell for shared terms. */
    TermCell* DefaultSharedTermCellAlloc(void);

    /* 解析scanner对象为一个Term,并存储到termbank中. */
    TermCell* TBTermParseReal(Scanner* in, bool isCheckSymbProp);

    /* Make ref point to a term of the same structure as *ref, but with properties prop set. 
     * Properties do not work for variables! */
    void TBRefSetProp(TermCell** ref, TermProp prop);

    /* Return the number of term nodes (variables and non-variables) in the term bank. */
    long TBTermNodes();

    //  TermCell* TermEquivCellAlloc(TermCell* source, VarBank_p vars);

    /* 插入一个Term　项到　termbank中 */
    TermCell* TBInsert(TermCell* term, DerefType deref);
    /* 插入项　t 到　TermBank 中．新项t的属性 0; 与TBInsert比较多了一个属性初始化为0; */
    TermCell* TBInsertNoProps(TermCell* term, DerefType deref);

    TermCell* TBInsertOpt(TermCell* term, DerefType deref);

    TermCell* TBInsertRepl(TermCell* term, DerefType deref, TermCell* old, TermCell* repl);
    /* 插入项t到TermBank 中,判断是否为基项或存在绑定,该方法用于rewrite  */
    TermCell* TBInsertInstantiated(TermCell* term);

    /* 实际的插入实现. Insert a term cell into the store-[termStore].*/
    TermCell* TermTopInsert(TermCell* t);

    /* Find a term in the term cell bank and return it. */
    TermCell* TBFind(TermCell* term);

    /* If bank->min_term exists, return it. Otherwise create and return it. */
    Term_p TBCreateMinTerm(FunCode min_const);

    /*--输出--*/
    void TBPrintBankInOrder(FILE* out);
    void TBPrintTerm(FILE* out, TermCell* term, bool fullterms);
    void TBPrintTermCompact(FILE* out, TermCell* term);
    TermCell* TBInsertDisjoint(TermCell* term);
};
typedef TermBank *TB_p;
#endif /* TERMBANK_H */

