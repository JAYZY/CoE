/* 
 * File:   ProverResultAanl.cpp
 * Author: zj
 * 
 * Created on 2019年6月2日, 下午8:19
 */

#include "ProverResultAnalyse.h"
#include "Global/GlobalFunc.h"
#include <regex>
using namespace std;

ProverResultAnalyse::ProverResultAnalyse(Formula *_fol) : fol(_fol) {

}

ProverResultAnalyse::ProverResultAnalyse(const ProverResultAnalyse& orig) {
}

ProverResultAnalyse::~ProverResultAnalyse() {
}

/**分析路径 
 **  readFileN -- 文件路径名称
 ** out  sbReadContext  -- 该路径下的详细路径
 ** out  sbValidPath --该路径下的有效路径
 */
bool ProverResultAnalyse::AnalyDeducdPath(string readFileN, string& sbReadContext, string& sbValidPath) {
    bool isEmpty = false;
    bool isClearBuf = true;
    map<int, string> sbBackPath; //该路径下的备选路径


    sbReadContext = ""; //该路径下的详细路径
    sbValidPath = ""; //该路径下的有效路径

    int triTimes = 1;

    string tmpSbReadContext = "", sbTmp = ""; /*该路径下的有效演绎过程*/



    ifstream in(readFileN); //读取文件.r
    if (!in) {
        fprintf(stderr, "结果文件读取错误！");
        return false;
    }

    string strDataLine = "";
    while (getline(in, strDataLine)) {

        string triContext = ""; //三角形内容
        TrimStr(strDataLine);
        if (strDataLine == "" || strDataLine[0] == '#') //不读空行       /读取备注信息    
            continue;
        tmpSbReadContext += strDataLine; //完整路径
        //sbValidPath.Append(strFirstLine); //有效路径                 

        vector<StuLitP> *lsPred = new vector<StuLitP>; //互补文字列表      
        //一次
        Stu_OptVaildPath* optPath = new Stu_OptVaildPath(++triTimes);

        regex regRow("C+\\d+"), regCol("_+\\d+"), regR("\\[+\\d+");
        smatch row, col, r;
        switch (strDataLine[0]) {
            case '['://归结路径      
                // <editor-fold defaultstate="collapsed" desc="读取归结路径">                

                isClearBuf = false;
                triContext += strDataLine; //读取路径内容

                regex_search(strDataLine, row, regRow);
                regex_search(strDataLine, col, regCol);
                //添加互补文字              
                lsPred->push_back(new Stu_Lit(atoi(row.str().substr(1).c_str()), atoi(col.str().substr(1).c_str()),
                        strDataLine.substr(FindSubStr(strDataLine, ']') + 1).c_str()));

                // </editor-fold>
                break;
            case 'R'://新R
                // <editor-fold defaultstate="collapsed" desc="读取R信息">
                triContext += strDataLine; //读取R内容

                optPath->lsPred = lsPred; //互补文字列表
                lsPred = new vector<StuLitP>; //互补文字列表



                if (0 == strDataLine.compare("R:空子句")) {
                    // <editor-fold defaultstate="collapsed" desc="空子句处理">  

                    optPath->rowNewR = -1; //归结式R行号         
                    optPath->newR = nullptr; //归结式R     
                    lstValidPathIndex.push_back(triTimes);
                    sbValidPath += ("第" + to_string(optPath->triTimes) + "个△-----\n");
                    sbValidPath += (triContext);
                    isEmpty = true;
                    // </editor-fold>

                } else { //新R字句

                    regex_search(strDataLine, r, regR);

                    optPath->rowNewR = stoi(r.str().substr(1)); //归结式R行号
                    int newRPos = FindSubStr(strDataLine, ':') + 1; //r内容位置

                    vector<string>words = Split_c(strDataLine.substr(newRPos), '+');
                    for (int i = 0; i < words.size(); ++i) {
                        string tmpStr = words[i];
                        regex_search(tmpStr, row, regRow);
                        regex_search(tmpStr, col, regCol);
                        optPath->newR->push_back(new Stu_Lit(stoi(row.str().substr(1).c_str()), stoi(col.str().substr(1).c_str()),
                                strDataLine.substr(FindSubStr(tmpStr, ']') + 1).c_str())); //归结式R 
                    }
                }
                lstValidPaths.push_back(optPath); //该路径的演绎过程  

                // tmpSbReadContext+=sbTmp;
                tmpSbReadContext += "第" + to_string(optPath->triTimes) + "个△-----\n";
                tmpSbReadContext += (triContext);
                // </editor-fold>
                break;
        }
    }
    return isEmpty;
}

/*
 **  查找产生新子句的迭代编号
 */

int ProverResultAnalyse::FindPath(int newRow) {

    for (int i = lstValidPaths.size() - 2; i >= 0; i--) {
        Stu_OptVaildPath* validPath = lstValidPaths[i];
        if (validPath->rState != 1) continue; //路径无效
        if (validPath->rowNewR > 0) {
            if (validPath->rowNewR == newRow)
                return i;
        }
    }
    return -1;
}
//输出最终结果路径

void ProverResultAnalyse::OutputResultP(int newR, vector<int>& P) {

    Stu_OptVaildPath* validPath = lstValidPaths[newR];
    Stu_Lit* lsP = validPath->lsPred->at(0);
    int originalLen = fol->getOrigalSet()->Size() - 1;
    if (lsP->Row > originalLen) {

        //找到产生该编号的新子句的 迭代
        int rIndex = FindPath(lsP->Row);
        if (rIndex > -1 && find(P.begin(), P.end(), rIndex) == P.end()) {
            P.push_back(rIndex);
            OutputResultP(rIndex, P);
        }
    }

    for (int i = 1; i < validPath->lsPred->size(); ++i) {
        lsP = validPath->lsPred->at(i);
        if (lsP->Row > originalLen) {
            //找到产生该编号的新子句的 迭代
            int rIndex = FindPath(lsP->Row);
            if (rIndex > -1 && find(P.begin(), P.end(), rIndex) == P.end()) {
                P.push_back(rIndex);
                OutputResultP(rIndex, P);
            }
        }
    }
}

//显示unsat路径

void ProverResultAnalyse::ShowUNSATPath(string fileFullName) {
    //vector<int>_lstIndex = new vector<int>();
    string sbReadContext, sbValidPath;
    //对.r文件进行分析提取
   // FileOp fileop;
    
   // AnalyDeducdPath(fileFullName,sbReadContext,sbValidPath);
   // OutputResultP(lstValidPaths.size() - 1, _lstIndex);
}