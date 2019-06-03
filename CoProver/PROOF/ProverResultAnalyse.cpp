/* 
 * File:   ProverResultAanl.cpp
 * Author: zj
 * 
 * Created on 2019年6月2日, 下午8:19
 */

#include "ProverResultAnalyse.h"
#include "Global/GlobalStrOp.h"
#include <regex>
using namespace std;

ProverResultAanl::ProverResultAanl() {
}

ProverResultAanl::ProverResultAanl(const ProverResultAanl& orig) {
}

ProverResultAanl::~ProverResultAanl() {
}

/**分析路径 
 **  readFileN -- 文件路径名称
 ** out  sbReadContext  -- 该路径下的详细路径
 ** out  sbValidPath --该路径下的有效路径
 */
bool analyDeducdPath(string readFileN, string& sbReadContext, string& sbValidPath) {
    bool isEmpty = false;
    bool isClearBuf = true;
    sbReadContext = ""; //该路径下的详细路径
    sbValidPath = ""; //该路径下的有效路径
    map<int, string> sbBackPath; //该路径下的备选路径

    string tmpSbReadContext = "", sbTmp = ""; //该路径下的有效演绎过程
    string triContext = ""; //三角形内容
    vector<Stu_Lit> lsPred; //互补文字列表  
    vector<int> lstValidPathIndex; //有效路径序号 
    vector<Stu_OptVaildPath> lstOptPaths; //一次完整的演绎过程
    string Rdata = "";
    int triTimes = 1;

    ifstream in(readFileN);
    if (in == nullptr) {
        fprintf(stderr, "结果文件读取错误！");
        return false;
    }
    string strDataLine = "";
    while (getline(in, strDataLine)) {
        TrimStr(strDataLine);
        if (strDataLine == "" || strDataLine[0] == '#') //不读空行       /读取备注信息    
            continue;
        tmpSbReadContext += strDataLine; //完整路径
        //sbValidPath.Append(strFirstLine); //有效路径                 


        Stu_OptVaildPath optPath;
        switch (strDataLine[0]) {
            case '['://归结路径      
                // <editor-fold defaultstate="collapsed" desc="读取归结路径">
                //if (isClearBuf)
                // sbBackPath = new Dictionary<int, string>();
                isClearBuf = false;
                triContext += strDataLine; //读取路径内容
                regex reg1("C+\\d+"), reg2("_+\\d+");
                smatch row, col;
                regex_search(strDataLine, row, reg1);
                regex_search(strDataLine, col, reg1);
                Stu_Lit lit = {.Row = stoi(row.str().substr(1)),
                    .Col = stoi(col.str().substr(1)),
                    .context = strDataLine.substr(FindSubStr(strDataLine, "]") + 1)};

                lsPred.push_back(lit);
                // </editor-fold>

                break;
            case 'R'://新R
                // <editor-fold defaultstate="collapsed" desc="读取R信息">
                triContext += strDataLine; //读取R内容
                optPath.triTimes = triTimes++; //路径序号 
                optPath.lsPred = lsPred; //互补文字列表
                // <editor-fold defaultstate="collapsed" desc="空子句处理">
                if (0 == strDataLine.compare("R:空子句")) {

                    optPath.RState = 1; //有效
                    optPath.rowNewR = -1; //归结式R行号                                                        
                    lstValidPathIndex.push_back(triTimes);
                    sbValidPath += ("第" + to_string(optPath.triTimes) + "个△-----\n");
                    sbValidPath += (triContext);
                    isEmpty = true;
                } else {
                    regex reg1("\\[+\\d+");
                    smatch row;
                    regex_search(strDataLine, row, reg1);

                    optPath.rowNewR = stoi(row.str().substr(1)); //归结式R行号
                    
                    int newRPos = Rdata.IndexOf(':') + 1;
                    string[] words = Rdata.Substring(newRPos, isVaildPos - newRPos).Split(Properties.Settings.Default.SplitChar);
                    List<Stru_Lit> newR = new List<Stru_Lit>();
                    List<string> lstNFol = new List<string>();
                    for (int i = 0; i < words.Length; i++) {
                        string tmpStr = words[i].Substring(words[i].IndexOf(']') + 1);
                        lstNFol.Add(tmpStr);
                        newR.Add(new Stru_Lit(){
                            row = int.Parse(Regex.Match(words[i], @"\[+\d+").Value.Substring(1)),
                            col = int.Parse(Regex.Match(words[i], @"_+\d+").Value.Substring(1)),
                            word = tmpStr
                        });
                    }

                }

                // </editor-fold>

                // </editor-fold>

                break;
        }





        while ((Rdata = sr.ReadLine()) != null) {
            if (Rdata.Trim() == "") continue; //不读空行                   
            if (Rdata[0] == '#')
                Rdata = Rdata.Substring(2);
            //sbTmp.Append(Rdata + "\n");
            Stu_OptVaildPath optPath = new Stu_OptVaildPath();
            string validInfo = ""; //有效或无效信息
            if (Rdata[0] == '=')//读取附加信息[注：附加信息中 == 表示要显示的信息，++表示需要隐藏的信息]
            {
                sbValidPath.AppendLine(Rdata);
            }
            int sJudgePos = 0;
            if (Rdata[0] == '{')
                sJudgePos = Rdata.IndexOf('}') + 1;
            switch (Rdata[sJudgePos]) {

                case 'R'://新R
#region 读取R信息
                    triContext.AppendLine(Rdata); //读取R内容
                    optPath.triTimes = triTimes++; //路径序号 
                    optPath.lsPred = lsPred; //互补文字列表
                    if (Rdata.Equals("R:空子句")) {
#region 空子句处理
                        optPath.RState = 1; //有效
                        optPath.rowNewR = -1; //归结式R行号                                    
                        optPath.newR = null; //归结式R                                 
                        lstValidPathIndex.Add(triTimes);
                        sbValidPath.Append(string.Format("第{0}个△-----\n", optPath.triTimes));
                        sbValidPath.Append(triContext);
                        isEmpty = true;
#endregion
                    } else {
                        int isVaildPos = Rdata.LastIndexOf('|');
                        if (Rdata.Substring(isVaildPos + 2) == "Y]") {
                            optPath.RState = 1;
                            validInfo = "[有效]";
                        } else if (Rdata.Substring(isVaildPos + 2) == "B]") //备选
                        {
                            optPath.RState = -1;
                            validInfo = "[备选]";
                        } else {
                            optPath.RState = 0; //无效路径
                            validInfo = Rdata.Substring(isVaildPos + 1);
                        }
#region 提取R信息
                        optPath.rowNewR = int.Parse(Regex.Match(Rdata, @"\[+\d+").Value.Substring(1)); //归结式R行号
                        int newRPos = Rdata.IndexOf(':') + 1;
                        string[] words = Rdata.Substring(newRPos, isVaildPos - newRPos).Split(Properties.Settings.Default.SplitChar);
                        List<Stru_Lit> newR = new List<Stru_Lit>();
                        List<string> lstNFol = new List<string>();
                        for (int i = 0; i < words.Length; i++) {
                            string tmpStr = words[i].Substring(words[i].IndexOf(']') + 1);
                            lstNFol.Add(tmpStr);
                            newR.Add(new Stru_Lit(){
                                row = int.Parse(Regex.Match(words[i], @"\[+\d+").Value.Substring(1)),
                                col = int.Parse(Regex.Match(words[i], @"_+\d+").Value.Substring(1)),
                                word = tmpStr
                            });
                        }
#endregion
                        optPath.newR = newR; //归结式R 

                        if (optPath.RState == 1) //有效路径
                        {
#region 有效路径处理
                            lstValidPathIndex.Add(triTimes - 1); //添加有效路径序号
                            sbValidPath.Append(string.Format("第{0}个△-----\n", optPath.triTimes));
                            sbValidPath.Append(triContext);
                            currentFOL.Add(lstNFol.ToArray());
#endregion
                        } else if (optPath.RState == -1)//备选路径
                        {
                            currentFOL.Add(lstNFol.ToArray());
                            sbBackPath.Add(optPath.rowNewR, string.Format("第{0}个△-----\n{1}", optPath.triTimes, triContext));
                        }
                    }
                    lstOptPaths.Add(optPath); //该路径的演绎过程  
                    tmpSbReadContext.Append(sbTmp);
                    tmpSbReadContext.Append(string.Format("第{0}个△{1}-----\n", optPath.triTimes, validInfo));
                    tmpSbReadContext.Append(triContext);
                    lsPred = new List<Stu_LitCont>();
                    sbTmp = new StringBuilder();
                    triContext = new StringBuilder();
#endregion
                    break;
                case 'S'://对备选R进行处理
#region  对备选R进行处理-确定有效信息
                    string tmpS = Rdata.Substring(Rdata.IndexOf('[') + 1);
                    int selIndex = int.Parse(tmpS.Substring(0, tmpS.Length - 1));
                    sbValidPath.Append(sbBackPath[selIndex]);
                    for (int i = lstOptPaths.Count - 1; i >= 0; i--) {
                        Stu_OptVaildPath sp = lstOptPaths[i];
                        //List<string> lstNFol = new List<string>();
                        //sp.newR.ForEach(r => lstNFol.Add(r.word));
                        //currentFOL.Add(lstNFol.ToArray());

                        if (sp.rowNewR == selIndex) {
                            sp.RState = 1;
                            lstOptPaths[i] = sp;
                            lstValidPathIndex.Add(i); //添加有效路径序号                                     

                            break;
                        }
                    }
                    isClearBuf = true;
                    sbTmp.AppendLine(Rdata);
#endregion
                    break;
                default://读取文本其余信息
                    sbTmp.AppendLine(Rdata);
                    break;
            }
        }
        tmpSbReadContext.Append(sbTmp);
        sbReadContext.Append(string.Format("该路径总共迭代{0}次，有效迭代{1}次\n---------------------\n{2}",
                triTimes - 1, lstValidPathIndex.Count, tmpSbReadContext));
    }
    return isEmpty;
}
