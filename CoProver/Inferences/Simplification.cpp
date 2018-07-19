/* 
 * File:   Simplification.cpp
 * Author: Zhong Jian<77367632@qq.com>
 * 
 * Created on 2018年2月23日, 下午1:59
 */

#include "Simplification.h"
#include "CLAUSE/Clause.h"
#include "Indexing/TermIndexing.h"
#include "Unify.h"
#include "CLAUSE/LiteralCompare.h"

Simplification::Simplification() {
}

Simplification::Simplification(const Simplification& orig) {
}

Simplification::~Simplification() {
}
/// 检查子句genCla 是否存在替换 使得候选子句 C 归入到查询子句 G, 目的检查查询子句G是否冗余
/*PS:G的文字数> C 的文字数;*/
/// \param genCla 查询子句
/// \param indexing 项索引
/// \return 子句genCla是否冗余

bool Simplification::ForwordSubsumption(Clause* genCla, TermIndexing* indexing) {

    //Literal* selConLit = genCla->Lits();
    Literal* selConLit = genCla->FileMaxLit<StandardWCMP, TermIndexing>(StandardWCMP(), indexing);
    if (genCla->GetClaId() == 4469) {
        cout << "test-genCla:";
        genCla->ClausePrint(stdout, true);
        cout << endl;
        cout << "选择文字:";
        selConLit->EqnTSTPPrint(stdout, true);
        cout << endl;
    }


    //Literal* selConLit = genCla->FileMaxLit<ImprovementCMP, TermIndexing>(ImprovementCMP(), indexing);

    //Literal* selConLit = genCla->FileMaxLit<OnlyVarLenCMP, TermIndexing>(OnlyVarLenCMP(), indexing);

    // Literal* selConLit = genCla->FileMaxLit<OnlyMaxDepthCMP, TermIndexing>(OnlyMaxDepthCMP(), indexing);
#ifdef OUTINFO
    cout << "选择文字:";
    selConLit->EqnTSTPPrint(stdout, true);
    cout << endl;
#endif

    TermIndNode* termIndNode = indexing->Subsumption(selConLit, SubsumpType::Forword);

    if (termIndNode == nullptr)
        return false;
    vector<Literal*>*candVarLits = &((termIndNode)->leafs);

    set<Clause*> checkedClas; //存储已经检查过得子句
    Clause* candVarCla = nullptr; //找到可能存在归入冗余的候选子句
    Literal* candVarLit = nullptr;

    while (true) {
        for (int ind = 0; ind < candVarLits->size(); ++ind) {

            candVarLit = candVarLits->at(ind);

            int substPos = indexing->chgVars->Size();
            candVarCla = candVarLit->claPtr; //找到可能存在归入冗余的候选子句
            if (candVarCla->GetClaId() == 3479) {
                cout << "test-candCla:";
                candVarCla->ClausePrint(stdout, true);
                cout << endl;
                cout << "匹配的文字:";
                candVarLit->EqnTSTPPrint(stdout, true);
                cout << endl;

            }
            assert(candVarCla);

            if (genCla->LitsNumber() >= candVarCla->LitsNumber() && checkedClas.find(candVarCla) == checkedClas.end()) {
                ++Env::backword_CMP_counter;
                if (LitListSubsume(candVarCla->Lits(), candVarLit, genCla->Lits(), indexing->chgVars, nullptr)) {
                    //测试:输出找到的冗余子句
                    genCla->ClausePrint(stdout, true);

                    cout << "\n<--:";
                    candVarCla->ClausePrint(stdout, true);
                    cout << endl;
                    ++Env::backword_Finded_counter;
                    return true;
                }
            }
            //没有找到 indexing->chgVars->SubstBacktrackToPos(substPos);
            assert(substPos == indexing->chgVars->Size());
            checkedClas.insert(candVarCla); //添加已经检查过的子句
        }
        termIndNode = indexing->NextForwordSubsump(); //查找下一个
        if (termIndNode == nullptr)
            return false;
        candVarLits = &((termIndNode)->leafs);
    }

}

/// 根据子句genCla查找(indexing tree中)冗余子句,有任意替换r使得 c * r = g ?
/// \param genCla 被检查的子句(生成的子句)
/// \param indexing 已有的索引
/// \return genCla是否归入某个子句(是否被某个子句包含)

bool Simplification::BackWordSubsumption(Clause* genCla, TermIndexing* indexing) {


#ifdef New
    // Literal* selLit = genCla->FileMaxLit<XYSteadyCMP,TermIndexing>(XYSteadyCMP(),indexing);
    // Literal* selLit = genCla->FileMaxLit<StandardWCMP, TermIndexing>(StandardWCMP(), indexing);
    //Literal* selLit = genCla->FileMaxLit<ConstLenCMP, TermIndexing>(ConstLenCMP(), indexing);
    // Literal* selLit = genCla->FileMaxLit<CountVarT>(CountVarT()); 
       Literal* selLit = genCla->FileMaxLit<ZXMSteadyCMP, TermIndexing>(ZXMSteadyCMP(), indexing);
   // Literal* selLit = genCla->FileMaxLit<ImprovementCMP, TermIndexing>(ImprovementCMP(), indexing);
    // Literal* selLit =genCla->FileMaxLit<OnlyMaxDepthCMP, TermIndexing>(OnlyMaxDepthCMP(), indexing);

#else
    Literal* selLit = genCla->Lits();
#endif
    //    if (genCla->GetClaId() == 30) {
    //        cout << "test-genCla:";
    //        genCla->ClausePrint(stdout, true);
    //        cout << endl;
    //        cout << "选择文字:";
    //        selLit->EqnTSTPPrint(stdout, true);
    //        cout << endl;
    //    }
#ifdef OUTINFO

    cout << "选择文字:";
    selLit->EqnTSTPPrint(stdout, true);
    cout << endl;
#endif
#ifdef OUTINFO
    int tmpTest = 0;
#endif
    set<Clause*> subsumedCla;
    Clause* candCla = nullptr;
    TermIndNode* candTermNode = indexing->Subsumption(selLit, SubsumpType::Backword);
    if (candTermNode == nullptr) {
#ifdef OUTINFO
        cout << "比较次数:" << tmpTest << endl;
#endif
        return false;
    }
    vector<Literal*>*candLits = &(candTermNode->leafs);

    Literal* candLit = nullptr;
    Subst* subst = new Subst();

    while (true) {

        for (int ind = 0; ind < candLits->size(); ++ind) {
            int substPos = subst->Size();
            candLit = candLits->at(ind);
            candCla = candLit->claPtr; //找到第一个文字所匹配的子句 

            //            if (candCla->GetClaId() == 6) {
            //                cout << "test-candCla:";
            //                candCla->ClausePrint(stdout, true);
            //                cout << endl;
            //                cout << "匹配的文字:";
            //                candLit->EqnTSTPPrint(stdout, true);
            //                cout << endl;
            //            }


            //  cout<<"candClaId"<<candCla->GetClaId()<<endl; 

            if (subsumedCla.find(candCla) == subsumedCla.end() && candCla->LitsNumber() >= genCla->LitsNumber()) {
                ++Env::backword_CMP_counter;
#ifdef OUTINFO
                ++tmpTest;
#endif
                if (LitListSubsume(genCla->Lits(), nullptr, candCla->Lits(), subst, nullptr)) {
                    //记录找到的冗余子句
                    ++Env::backword_Finded_counter;
                    subsumedCla.insert(candCla);
                }
            }
            subst->SubstBacktrackToPos(substPos);
        }
        candTermNode = indexing->NextBackSubsump();

        if (candTermNode == nullptr) {//no backsump,rollback;
            break;
        }
        candLits = &(candTermNode->leafs);
        DelPtr(subst);
        subst = new Subst();
    }
    DelPtr(subst);
    //对找到的冗余子句进行处理
#ifdef OUTINFO   
    if (subsumedCla.empty()) {
        // cout << "No found backsubsumption!" << endl;
    } else {

        cout << "\nfound backsubsumption!,size:" << subsumedCla.size() << " " << endl;
        genCla->ClausePrint(stdout, true);
        cout << " ==> ";
        (*subsumedCla.begin())->ClausePrint(stdout, true);

    }
    cout << "比较次数:" << tmpTest << endl;
#endif
    return true;
}



/// 检查查询文字列表是否归入候选文字列表(PS:存在替换r,使得subsumVarLst(r) 归入 subsumConLst)
/* sumsumLst 改变,subsumCandLst 变元不改变,*/
/// \param subsumVarLst     查询的文字列表(可变文字)
/// \param subsumConLst     候选的文字列表(不可变文字)
/// \param subst    替换对象
/// \return    subsumVarLst 是否归入 subsumConLst;

bool Simplification::LitListSubsume(Literal* subsumVarLst, Literal* exceptLit, Literal* subsumConLst, Subst*subst, int8_t* pickLst) {

    if (!subsumVarLst) return true; //查询文字列表没有文字,返回true
    uint32_t iniSubstPos = subst->Size();
    for (Literal* varEqn = subsumVarLst; varEqn; varEqn = varEqn->next) {

        if (varEqn == exceptLit)continue;

        bool res = false;
        for (Literal* conEqn = subsumConLst; conEqn; conEqn = conEqn->next) {
            if (conEqn->EqnIsPositive() != varEqn->EqnIsPositive())
                continue;

            if (conEqn->EqnIsEquLit() != varEqn->EqnIsEquLit())
                continue;
            uint32_t substPos = subst->Size();


            if (conEqn->StandardWeight() < varEqn->StandardWeight()) //被归入文字的变元数 > 归入文字的变元数 
                continue;
            //test
            //                        cout << "eqn:";     varEqn->EqnTSTPPrint(stdout, true);  cout << endl;
            //                         cout << "eqnCand:";  conEqn->EqnTSTPPrint(stdout, true); cout << endl;

            if (Unify::SubstComputeMatch(varEqn->lterm, conEqn->lterm, subst)) {
                if (Unify::SubstComputeMatch(varEqn->rterm, conEqn->rterm, subst)) {
                    res = true;

                    break;
                }
            }
            subst->SubstBacktrackToPos(substPos);
            /*如果为等词,检查如下情况   l1=E(a,b)  l2=E(b,a)  是否为包含关系? */
            if (Unify::SubstComputeMatch(varEqn->rterm, conEqn->lterm, subst) &&
                    Unify::SubstComputeMatch(varEqn->lterm, conEqn->rterm, subst)) {
                res = true;
                break;
            }
            subst->SubstBacktrackToPos(substPos);
        }
        if (!res) {
            //失败还原所有的替换改变
            subst->SubstBacktrackToPos(iniSubstPos);
            return false;
        }
    }

    return true;
}

/*-----------------------------------------------------------------------
//   Test wether an equation subsumes another one. If yes, return true
//   and extend subst to give the substitution, otherwise just return
//   false and let subst unmodified. Don't deal with commutativity of equality. 
//----------------------------------------------------------------------*/

bool Simplification::LitSubsume(Literal* subsumer, Literal* subsumed, Subst * subst) {
    //PStackPointer backtrack = PStackGetSP(subst);
    int backtrack = subst->Size();
    bool res;
    Unify unify;
    res = unify.SubstComputeMatch(subsumer->lterm, subsumed->lterm, subst);
    if (res) {
        res = unify.SubstComputeMatch(subsumer->rterm, subsumed->rterm, subst);
    }
    if (!res) {
        subst->SubstBacktrackToPos(backtrack);
    }
    return res;
}