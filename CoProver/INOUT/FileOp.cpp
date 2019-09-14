/*
 * 文件操作类
 * File:   FileOp.cpp
 * Author: zj 
 * 
 * Created on 2017年3月13日, 上午10:15
 */
#include "FileOp.h"
#include <fstream>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdlib.h>
#include "HEURISTICS/StrategyParam.h"
#include "CLAUSE/Clause.h"
#include<regex>
using namespace std;

string FileOp::homePath = getenv("HOME");

string FileOp::TPTP_dir = "";
//IOFormat FileOp::OutputFormat = IOFormat::AutoFormat;
long FileOp::OutputLevel = 1;
int FileOp::TmpErrno = 0;
FILE* FileOp::GlobalOut = stdout;

map<string, FileOp*> FileOp::lsOut;

/*---------------------------------------------------------------------*/
/*                    Constructed Function                             */

/*---------------------------------------------------------------------*/
FileOp::FileOp() {
    tptpFileNameNoExt = this->getFileNameNoExt(Env::tptpFileName);
    tptpFileName = this->getFileName(Env::tptpFileName);
    setWorkDirAndCreateFile(homePath + "/Desktop/");

}


/*---------------------------------------------------------------------*/
/*                  Member Function-[public]                           */
/*---------------------------------------------------------------------*/
//

/*---------------------------------------------------------------------*/
/*                          Static Function                            */

/*---------------------------------------------------------------------*/

/* 关闭所有文件 */
void FileOp::CloseAll() {
    if (fInfo)
        fclose(fInfo);
    if (fRun)
        fclose(fRun);
    if (fLog)
        fclose(fLog);
    if (fUNSAT)
        fclose(fUNSAT);
    if(fTri)
        fclose(fTri);
    
    
}

/**
 * 设置并创建目录以及 .r .i 文件
 * @param strDir
 * @return 
 */
bool FileOp::setWorkDirAndCreateFile(string strDir) {
    workDir = strDir + "output/";
    if (tptpFileNameNoExt == "") {
        Out::SysError("Not get tptpfile %s", ErrorCodes::FILE_ERROR, "No File Error!");
        return false;
    }
    outDir = workDir + tptpFileNameNoExt + "/";
    DIR *mydir = nullptr;
    if ((mydir = opendir(outDir.c_str())) == nullptr) {//目录不存在
        if (mkMultiDir(outDir) != 0) {
            Out::SysError("Create dir error %s", ErrorCodes::FILE_ERROR, outDir);
            DelPtr(mydir);
            return false;
        }
    }
    DelPtr(mydir);
    CloseAll();

    string tmpStr = outDir + tptpFileNameNoExt;
    cnfFileName = tmpStr + ".cnf";
    fInfoFileName = tmpStr + ".i";
    fUnsatFileName = tmpStr + ".unsat";
    if ((fInfo = fopen(fInfoFileName.c_str(), "wb")) == nullptr) { //第一次以读的方式新建一个文件
        Out::SysError("Create file: %s  error", ErrorCodes::FILE_ERROR, ".info");
        return false;
    }
    string sFileName = tmpStr + ".r";
    if ((fRun = fopen(sFileName.c_str(), "wb")) == nullptr) { //第一次以读的方式新建一个文件
        Out::SysError("Create file: %s  error", ErrorCodes::FILE_ERROR, ".run");
        return false;
    }

    sFileName = tmpStr + ".log";
    if ((fLog = fopen(sFileName.c_str(), "wb")) == nullptr) { //第一次以读的方式新建一个文件
        Out::SysError("Create file: %s  error", ErrorCodes::FILE_ERROR, ".log");
        return false;
    }

    sFileName=tmpStr+".tri";
    if ((fTri = fopen(sFileName.c_str(), "wb")) == nullptr) { //第一次以读的方式新建一个文件
        Out::SysError("Create file: %s  error", ErrorCodes::FILE_ERROR, ".tri");
        return false;
    }
    
    if (fGlobalInfo)
        fclose(fGlobalInfo);
    sFileName = workDir + "proofRes.g";
    if ((fGlobalInfo = fopen(sFileName.c_str(), "ab")) == nullptr) { //第一次以追加的方式新建一个文件,并写入该判定文件的名称
        Out::SysError("Create file: %s  error", ErrorCodes::FILE_ERROR, ".g");
        return false;
    }
    outGlobalInfo("\n" + getFileName(Env::tptpFileName) + " ");
    return true;
}



// <editor-fold defaultstate="collapsed" desc="系统文件操作(文件夹,文件)">

/**
 * 创建多级目录
 * @param dirPath
 * @return 
 */
int FileOp::mkMultiDir(string& dirPath) {
    int mdret;
    size_t pre = 0, pos;
    string dir;
    if (dirPath[dirPath.size() - 1] != '/')
        dirPath += '/';

    while ((pos = dirPath.find_first_of('/', pre)) != string::npos) {
        dir = dirPath.substr(0, pos++);
        pre = pos;
        if (dir.size() == 0) continue; // if leading / first time is 0 length
        if ((mdret = mkdir(dir.c_str(), 0755)) && errno != EEXIST) {
            return mdret;
        }
    }
    return mdret;
}

//recursively delete all the file in the directory.

int FileOp::rmDir(string dirPath) {
    DIR* dirp = opendir(dirPath.c_str());
    if (!dirp) {
        return -1;
    }
    struct dirent *dir;
    struct stat st;
    while ((dir = readdir(dirp)) != NULL) {
        if (strcmp(dir->d_name, ".") == 0
                || strcmp(dir->d_name, "..") == 0) {
            continue;
        }
        std::string sub_path = dirPath + '/' + dir->d_name;
        if (lstat(sub_path.c_str(), &st) == -1) {
            printf("rm_dir:lstat %s%s ", sub_path.c_str(), " error");
            continue;
        }
        if (S_ISDIR(st.st_mode)) {
            if (rmDir(sub_path) == -1) { // 如果是目录文件，递归删除
                closedir(dirp);
                return -1;
            }
            rmdir(sub_path.c_str());
        } else if (S_ISREG(st.st_mode)) {
            unlink(sub_path.c_str()); // 如果是普通文件，则unlink
        } else {
            printf("rm_dir:st_mode %s%s", sub_path.c_str(), " error");
            continue;
        }
    }
    if (rmdir(dirPath.c_str()) == -1)//delete dir itself.
    {
        closedir(dirp);
        return -1;
    }
    closedir(dirp);
    return 0;
}
//获取带后缀的文件名

string FileOp::getFileName(string &fileFullName) {
    size_t pos = fileFullName.find_last_of('/');
    if (pos == -1)
        return fileFullName;
    else
        return fileFullName.substr(pos + 1);
}
//获取不带扩展名的文件名

string FileOp::getFileNameNoExt(string fileFullName) {
    size_t posBeg = fileFullName.find_last_of('/');
    if (posBeg == string::npos)
        posBeg = -1;
    size_t posEnd = fileFullName.find_last_of('.');
    if (posEnd == string::npos)
        return fileFullName.substr(posBeg + 1);
    else
        return fileFullName.substr(posBeg + 1, posEnd - posBeg - 1);
}
// </editor-fold>

/**
 * 读取.i文件生成最小路径文件.out
 */
void FileOp::GenerateEmptyPath() {
    ifstream fin(fInfoFileName, ios::in);
    ofstream fout(fUnsatFileName, ios::out); //输出
    map<int, vector<string> > allFOL;
    std::string line;

    regex patternClaNo("c\\d+,", regex::icase);
    regex patternUseNo("c\\d+", regex::icase);
    smatch result;
    bool newCla = false;
    if (fin) {// 有该文件
        while (getline(fin, line)) {
            if (line == "") //空行
                continue;
            //   cout<<line<<endl;
            if ('#' == line[0]) {//注释语句
                int x = line.find("New Clauses");
                if (x != string::npos)
                    newCla = true;
                //if (!newCla)
                //     fout<<line<<endl;
                continue;
            }
            //if (!newCla)
            //       fout<< line<<endl;
            string::const_iterator iter = line.begin();
            string::const_iterator iterEnd = line.end();
            int iRowNo;
            if (regex_search(iter, iterEnd, result, patternClaNo)) {
                string keyRowNo = result[0];
                keyRowNo = keyRowNo.substr(1, keyRowNo.length() - 2);
                iRowNo = stoi(keyRowNo);
                allFOL[iRowNo].push_back(line);
                iter = result[0].second;

                if (newCla) { //若为新子句，提取新子句
                    while (regex_search(iter, iterEnd, result, patternUseNo)) {
                        string str = result[0];
                        allFOL[iRowNo].push_back(str);
                        iter = result[0].second;
                    }
                    if (line.find("$false") != string::npos) {
                        allFOL[0].push_back(keyRowNo);
                    }
                }
            }
        }

        //输出最短路径
        set<int>outClaID;
        int lastClaId = stoi(allFOL[0][0]);
        outClaID.insert(lastClaId);
        vector<string> st = allFOL[lastClaId];
        swap(st[0], st[st.size() - 1]);
        st.pop_back();

        while (!st.empty()) {
            int iClaId = stoi(st.back().substr(1));
            outClaID.insert(iClaId);
            st.pop_back();
            for (int i = 1; i < allFOL[iClaId].size(); ++i) {//说明不是原始子句
                st.push_back(allFOL[iClaId][i]);
            }
        }
        string resPoof = "\n%------ Proof Process ------\n";
        GetProblemInfo(resPoof);        
        for (auto id : outClaID) {
            resPoof += allFOL[id][0] + "\n";
        }
        char sRusage[10000];
        Env::PrintRusage(sRusage);
        resPoof += sRusage;
        fout << resPoof;
        //outInfo(resPoof);
        cout << resPoof;


    } else { // 没有该文件
        cout << "no such file" << endl;
    }

    fin.clear();
    fin.close();
}
