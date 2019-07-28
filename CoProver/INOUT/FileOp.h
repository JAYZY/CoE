/*
 * 文件操作类 -- 采用单例模式
 * File:   FileOp.h
 * Author: zj 
 *
 * Created on 2017年3月13日, 上午10:15
 * modify    2019-3-18  改为单例模式
 */

#ifndef FILEOP_H
#define FILEOP_H
#include "Global/IncDefine.h" 
#include <fstream>
#include <iostream>
#include <map>
using namespace std;

enum class OutType {
    StdOut, //控制台正常输出
    StdErr, //控制台错误输出
    Info, //info文件输出
    Run, //run 运行记录删除        
    BinaryFile //二进制文件输出      
};

class FileOp {
private:

    //1.构造函数私有

    FileOp();
    FileOp(const FileOp&); //拷贝构造函数不实现，防止拷贝产生多个实例
    FileOp & operator=(const FileOp&); //复制构造函数不实现，防止赋值产生多个实例

private:

    /*--------------------------------------------------------------------------
    /* 输出目录说明:
     * 1.工作目录为 /output 
     * 2.信息输出目录为: /output/判断文件的名称(不带后缀名) 
     * 3.*.info 为整个子句集以及演绎过程中生成的新子句信息.
     * 4.*.r为演绎过程中的过程信息.
     * 5.*.g为全局信息-- A.测试使用,B.所有测试例特征.C.测试结果
    /-------------------------------------------------------------------------*/
    static string homePath; //固定的Home 路径

    string workDir; //工作目录(后台输出目录)
    string outDir; //输出目录
    string tptpFileNameNoExt; //判断文件名称

    string tptpFileName; //文件名称


    FILE* fInfo; //.i 文件,包括原始子句集,采用策略,以及新增加子句
    string fInfoFileName; //.i文件完整路径
    string fUnsatFileName;
    FILE* fRun; //.r 文件,记录整个演绎过程.
    FILE* fLog; //.log 文件,记录演绎过程中的 删除信息日志信息
    FILE* fUNSAT; //.unsat 文件， 记录得到empty路径

    FILE* fGlobalInfo; //.g 文件,全局信息输出
    string outName; //输出名称
    string fileFullName; //文件完整路径  
    ofstream* OutFile;

    static map<string, FileOp*>lsOut;

public:
    string cnfFileName; //CNF文件名称
    /*---------------------------------------------------------------------*/
    /*                    Constructed Function                             */
    /*---------------------------------------------------------------------*/
    //Singleton

    static FileOp * getInstance() { //2.提供全局访问点    
        static FileOp m_singletonConfig; //3.c++11保证了多线程安全，程序退出时，释放资源
        return &m_singletonConfig;
    }

    virtual ~FileOp() {
        CloseAll();
    };

public:

    inline FILE* getInfoFile() {
        assert(fInfo);
        return fInfo;
    }

    inline FILE* getRunFile() {
        assert(fRun);
        return fRun;
    }
    static OutType outType; //输出类型

    inline bool IsOpen() {
        return OutFile && OutFile->is_open();
    }

    inline void get_exe_path(char* path, int size) {
        char link[1024];
        sprintf(link, "/proc/%d/exe", getpid()); /////////////
        readlink(link, path, size); //////////////
        //printf("%s/n", path);
    }

    /*
     ** 写入 info 文件
     */
    inline void outInfo(const string&msg) {
        fwrite(msg.c_str(), 1, msg.length(), fInfo);
        fflush(fInfo);
    }

    inline void outLog(const string&msg) {
        fwrite(msg.c_str(), 1, msg.length(), fLog);
        fflush(fLog);
    }

    inline void outRun(const string&msg) {
        fwrite(msg.c_str(), 1, msg.length(), fRun);
        fflush(fRun);
    }

    inline FILE* GetfRun() {
        return fRun;
    }

    inline void outGlobalInfo(const string&msg) {
        fwrite(msg.c_str(), 1, msg.length(), fGlobalInfo);
        fflush(fGlobalInfo);
    }

    /* 写入缓冲区中的内容 */
    inline void Flush() {
        OutFile->flush();
    }


public:
    /* 关闭所有文件 */
    void CloseAll();
    /**
     * 设置并创建目录以及 .r .i 文件*/
    bool setWorkDirAndCreateFile(string strWorkDir);

    // <editor-fold defaultstate="collapsed" desc="系统文件操作(文件夹,文件)">
    /**
     * 创建目录(包括多级目录)
     * @param dirPath
     * @return 
     */
    int mkMultiDir(string& dirPath);
    int rmDir(std::string dir_full_path);

    //获取带后缀的文件名
    string getFileName(string &fileFullName);

    //获取不带扩展名的文件名
    string getFileNameNoExt(string fileFullName);
    // </editor-fold>



    //读取.i文件生成最小路径文件.out
    void GenerateEmptyPath();

    void GetProblemInfo(string &strInfo) {
        strInfo = "start to proof:" + this->tptpFileName;
        strInfo += "\n% Version  : CoProver---0.1\n% Problem  : " + this->tptpFileName;
        strInfo += "\n% Proof found!\n% SZS status Theorem for " + this->tptpFileName;
        strInfo += "\n% SZS output start Proof\n";
    }
public:

    /*---------------------------------------------------------------------*/
    /*                   Static Function[out op]                           */
    /*---------------------------------------------------------------------*/
    //

    static FILE* InputOpen(const char* name, bool fail) {
        FILE* in;
        if (name && strcmp(name, "-") != 0) {
            Out::Err("Trying file ", name);
            in = fopen(name, "r");
            if (fail&& !in) {
                TmpErrno = errno;
                Out::SysError("Cannot open file %s for reading", ErrorCodes::FILE_ERROR, name);
            }
            if (fail) {
                Out::Err("Input file is ", name);
            }
        } else {
            Out::Err("Input is coming from <stdin>\n");
            in = stdin;
        }
        return in;
    }


    /*---------------------------------------------------------------------*/
    /*                 Static Function[fileName op]                        */
    /*---------------------------------------------------------------------*/

    // <editor-fold defaultstate="collapsed" desc="Static Function for FileName">


    static string TPTP_dir;
    static long OutputLevel;
    static int TmpErrno;
    //  static IOFormat OutputFormat;
    static FILE* GlobalOut;

    inline static bool FileNameIsAbsolute(const char* fileName) {
        return fileName[0] == '/';
    }

    inline static char StrLastChar(const char* str) {

        return strlen(str) == 0 ? str[strlen(str) - 1] : '\0';
    }

    /* 提取目录地址串 Given a path name, return the directory portion */
    inline static const char* FileNameDirName(const string& name) {
        assert(!name.empty());
        return (name.substr(0, name.find_last_of('/'))).c_str();

    }
    /// 提取文件的基本名称,不包括后缀名
    /// \param fileFullName
    /// \param baseName

    inline static string FileNameBaseName(const string& fileFullName) {
        assert(!fileFullName.empty());
        int pos = fileFullName.find_last_of('/');
        return fileFullName.substr(pos + 1);

    }

//    string static GetProgramDir() {
//        char exeFullPath[1024]; // Full path
//        string strPath = "";
//
//        GetModuleFileName(nullptr, exeFullPath, 1024);
//        strPath = (string) exeFullPath; // Get full path of the file
//        int pos = strPath.find_last_of('\\', strPath.length());
//        return strPath.substr(0, pos); // Return the directory without the file name
//    }
    // </editor-fold>






};

#endif /* FILEOP_H */

