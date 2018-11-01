/*-----------------------------------------------------------------------
File  : GlobalStrOpe_h__.c
2016-3-8 [zj]字符操作类文件
Contents:字符操作类
Changes
<1>
-----------------------------------------------------------------------*/
#pragma once
#include "IncDefine.h"

using namespace std;
//去除字符前后空格

inline void TrimStr(string& s) {
    if (s.empty())
        return;
    s.erase(0, s.find_first_not_of(" "));
    s.erase(s.find_last_not_of(" ") + 1);

}

//查找子字符串

inline int FindSubStr(string& str, string& subStr) {
    int pos = -1;
    size_t strLen = str.size(), subStrLen = subStr.size();
    if (strLen < subStrLen)
        return -1;
    while (pos++<strLen) {
        bool flag = false;
        if (str[pos] == subStr[0]) {
            flag = true;
            for (size_t k = 1; k < subStrLen; k++) {
                if (str[pos + k] != subStr[k]) {
                    flag = false;
                    break;
                }
            }
        }
        if (flag && (str[pos + subStrLen] == ')' || str[pos + subStrLen] == ',')) {
            return pos;
        }
    }
    return -1;
}
//分割字符串

inline vector<string> Split_c(const string& s, const char delim) {
    size_t last = 0;
    size_t index = s.find_first_of(delim, last);
    vector<string> ret;
    while (index != string::npos) {
        ret.push_back(s.substr(last, index - last));
        last = index + 1;
        index = s.find_first_of(delim, last);
    }
    if (index - last > 0)
        ret.push_back(s.substr(last, index - last));
    return ret;
}
//分割字符串 

inline vector<string> Split_s(const string& s, const string& delim) {
    size_t last = 0;
    size_t index = s.find_first_of(delim, last);
    vector<string> ret;
    while (index != string::npos) {
        ret.push_back(s.substr(last, index - last));
        last = index + 1;
        index = s.find_first_of(delim, last);
    }
    if (index - last > 0)
        ret.push_back(s.substr(last, index - last));
    return ret;
}

//返回谓词符号以及第一个括号所在位置

inline string GetPred(const string& pred, size_t& i) {
    size_t predSize = pred.size();
    assert(predSize != 0);
    //if(predSize==0)return nullptr;		
    i = pred.find_first_of('(');
    assert(i != 0);
    assert(i != string::npos);
    return pred.substr(0, i);
}

//分离谓词符号和括号中内容
inline void SplitSymbol(const string &pred, string &symbol, string &strContent) {
    size_t pos = 0;
    symbol = GetPred(pred, pos);
    strContent = pred.substr(pos + 1, pred.length() - pos - 2); //抽取内容
}

//判定是否为基文字-是否包含'x'
inline bool IsGroundLits(vector<string>& vectStrLits) {
    bool isBase = true;
    for (string &str : vectStrLits) {
        if (str.find('x') != string::npos) {
            isBase = false;
            break;
        }
    }
    return isBase;
}
