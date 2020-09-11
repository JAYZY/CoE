/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   FormulaSet.cpp
 * Author: Zhong Jian<77367632@qq.com>
 * 
 * Created on 2018年4月23日, 下午7:23
 */

#include "FormulaSet.h"
#include "LIB/Out.h"
const float TFORMULA_GC_LIMIT = 1.5f;

FormulaSet::FormulaSet() {
    WFormula* anchor;
    long members;
    string identifier;
    this->members = 0;
    this->anchor = new WFormula();
    this->anchor->succ = this->anchor;
    this->anchor->pred = this->anchor;
    this->identifier = "";
    this->claSet = nullptr; //子句集为空
}

FormulaSet::FormulaSet(const FormulaSet& orig) {
}

FormulaSet::~FormulaSet() {
}
/*---------------------------------------------------------------------*/
/*                  Member Function-[public]                           */
/*---------------------------------------------------------------------*/
/// 将一个公式插入公式集合
/// \param newform

void FormulaSet::FormulaSetInsert(WFormula* newform) {

    assert(newform);
    assert(!newform->set);
    newform->succ = this->anchor;
    newform->pred = this->anchor->pred;
    this->anchor->pred->succ = newform;
    this->anchor->pred = newform;
    newform->set = this;
    this->members++;


}

///  Move all formulas from fromset into set (leaving from empty, but not deleted).
/// \param fromset 要合并的公式集
/// \return 合并的公式数量

long FormulaSet::FormulaSetInsertSet(FormulaSet* fromset) {
    WFormula* handle;
    long res = 0;
    while (!fromset->FormulaSetEmpty()) {
        handle = fromset->FormulaSetExtractFirst();
        FormulaSetInsert(handle);
        res++;
    }
    return res;
}

long FormulaSet::FormulaAndClauseSetParse(Scanner* in, ClauseSet* cset,
        SplayTree<StrTreeCell>&name_selector, SplayTree<StrTreeCell>&skip_includes) {


    long res = 0;
    WFormula* form = nullptr;
    WFormula* nextform = nullptr;
    //Clause* clause, nextclause;
    // StrTree_p stand_in = NULL;

    //    if (name_selector.IsEmpty()) {
    //        name_selector. = &stand_in;
    //    }

    switch (in->format) {
        case IOFormat::LOPFormat:
            /* LOP does not at the moment support full FOF */
            // res = ClauseSetParseList(in, cset, terms);
            Out::Error("No Support lop format!", ErrorCodes::INPUT_SEMANTIC_ERROR);
            break;
        default:
            while (in->TestInpId("input_formula|input_clause|fof|cnf|include")) {
                if (in->TestInpId("include")) {//读取include 文件
                    SplayTree<StrTreeCell> new_limit;
                    Scanner* new_in;
                    ClauseSet* ncset = new ClauseSet();
                    FormulaSet* nfset = new FormulaSet();
                    new_in = in->ScannerParseInclude(new_limit, skip_includes);

                    if (new_in) {
                        res += nfset->FormulaAndClauseSetParse(new_in, ncset, new_limit, skip_includes);
                        DelPtr(new_in);
                    }
                    new_limit.Destroy();

                    cset->InsertSet(ncset);
                    this->FormulaSetInsertSet(nfset); //FormulaSetInsertSet(fset, nfset);

                    assert(ncset->ClauseSetEmpty());
                    assert(nfset->FormulaSetEmpty());
                    ncset->FreeAllClas();
                    DelPtr(nfset); //ClauseSetFree();
                    //FormulaSetFree(nfset);
                } else {
                    if (in->TestInpId("input_formula|fof")) {
                        Out::Error("现在暂时不能识别 FOF公式集!", ErrorCodes::FILE_ERROR);

                        //form = new WFormula();
                       // form->WFormulaParse(in, terms);

                        // fprintf(stdout, "Parsed: ");
                        // WFormulaPrint(stdout, form, true);
                        // fprintf(stdout, "\n");
                        this->FormulaSetInsert(form);
                    } else {

                        assert(in->TestInpId("input_clause|cnf"));
                        Clause* clause = new Clause();
                        clause->ClauseParse(in);
                        cset->InsertCla(clause);
                    }
                    res++;
                }
            }
            break;
    }
    if (!name_selector.IsEmpty()) {
        Out::Error("name_select is not Empty~", ErrorCodes::FILE_ERROR);
        //            form = this->anchor->succ;
        //            while (form != this->anchor) {
        //                nextform = form->succ;
        //                if (!verify_name(name_selector, form->info)) {
        //                    FormulaSetDeleteEntry(form);
        //                }
        //                form = nextform;
        //            }
        //            clause = cset->anchor->succ;
        //            while (clause != cset->anchor) {
        //                nextclause = clause->succ;
        //                if (!verify_name(name_selector, clause->info)) {
        //                    ClauseSetDeleteEntry(clause);
        //                }
        //                clause = nextclause;
        //            }
        //            check_all_found(in, *name_selector);
    }
    return res;
}

long FormulaSet::FormulaSetCNF(FormulaSet* set, FormulaSet* archive, ClauseSet* clauseset, TermBank_p tbs, VarBank* fresh_vars) {

    WFormula* form, handle;
    long res = 0;
    long old_nodes = tbs->TBNonVarTermNodes();
    long gc_threshold = old_nodes*TFORMULA_GC_LIMIT;

    /* FormulaSetSimplify(tbs);
     // printf("FormulaSetSimplify done\n");
     TFormulaSetIntroduceDefs(set, archive, tbs);
     // printf("Definitions introduced\n");

     while (!FormulaSetEmpty(set)) {
         handle = FormulaSetExtractFirst(set);
         // WFormulaPrint(stdout, handle, true);
         // fprintf(stdout, "\n");
         if (BuildProofObject) {
             form = WFormulaFlatCopy(handle);
             FormulaSetInsert(archive, handle);
             WFormulaPushDerivation(form, DCFofQuote, handle, NULL);
             handle = form;
         }
         res += WFormulaCNF(handle, clauseset, tbs, fresh_vars);
         if (BuildProofObject) {
             FormulaSetInsert(archive, handle);
         }
         if (handle->tformula &&
                 (tbs->TBNonVarTermNodes() > gc_threshold)) {
             assert(terms == handle->terms);
             GCCollect(gc);
             old_nodes = tbs->TBNonVarTermNodes();
             gc_threshold = old_nodes*TFORMULA_GC_LIMIT;
         }
         if (!BuildProofObject) {
             DelPtr(handle);//WFormulaFree(handle);
         }
     }
     if (tbs->TBNonVarTermNodes() != old_nodes) {
         GCCollect(gc);
     }*/
    return res;
}


/// Apply standard FOF simplification rules to all formulae in the set. Returns number of changed formulas.
/// \param tbs
/// \return 

long FormulaSet::FormulaSetSimplify(TermBank_p tbs) {
    WFormula* handle;
    long res = 0;
    long old_nodes = tbs->TBNonVarTermNodes();
    long gc_threshold = old_nodes*TFORMULA_GC_LIMIT;
    handle = this->anchor->succ;
    /* while (handle != this->anchor) {
         // printf("Simplifying: \n");
         // WFormulaPrint(stdout, handle, true);
         // printf("\n");
         bool changed = handle->WFormulaSimplify(tbs);
         // printf("Simplified %d\n", changed);
         // WFormulaPrint(stdout, handle, true);
         // printf("\n");
         if (changed) {
             res++;
             if (tbs->TBNonVarTermNodes() > gc_threshold) {
                 assert(tbs == handle->terms);
                 GCCollect(tbs->gc);
                 old_nodes = tbs->TBNonVarTermNodes();
                 gc_threshold = old_nodes*TFORMULA_GC_LIMIT;
             }
         }
         handle = handle->succ;
     }
     // printf("All simplified\n");
     if (tbs->TBNonVarTermNodes() != old_nodes) {
         GCCollect(tbs->gc);
     }
     // printf("Garbage collected\n");*/
    return res;

}
