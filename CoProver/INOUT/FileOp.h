/*
 * 文件操作类
 * File:   FileOp.h
 * Author: zj 
 *
 * Created on 2017年3月13日, 上午10:15
 */

#ifndef FILEOP_H
#define FILEOP_H
#include "Global/IncDefine.h" 
#include <fstream>
#include <iostream>
#include <map>
using namespace std;

typedef enum {
    StdOut, //控制台正常输出
    StdErr, //控制台错误输出
    FileOut, //正常文件输出
    BinaryFile //二进制文件输出      
} OutType;

class FileOp {
private:
    string outName; //输出名称
    string fileFullName; //文件完整路径
    OutType outType; //输出类型
    ofstream* OutFile;
    static map<string, FileOp*>lsOut;
public:
    /*---------------------------------------------------------------------*/
    /*                    Constructed Function                             */
    /*---------------------------------------------------------------------*/
    //

    FileOp(const string name, const string fileName) : outName(name) {
        if (fileName.empty()) {
            OutFile = nullptr;
            cout << "create \"{out}\" error!" << endl;
        } else if (fileName == "stdout") {
            OutFile = nullptr;
            outType = StdOut;
        } else if (fileName == "stderr") {
            OutFile = nullptr;
            outType = StdErr;
        } else {
            OutFile = new ofstream(fileName);
            fileFullName = fileName;
            outType = FileOut;
        }
    };

    FileOp(const FileOp& orig) {
    };

    virtual ~FileOp() {
        Close();
    };

public:

    inline bool IsOpen() {
        return OutFile && OutFile->is_open();
    }

    inline void get_exe_path(char* path,int size) {
        char link[1024];
        sprintf(link, "/proc/%d/exe", getpid()); /////////////
        readlink(link, path, size); //////////////
        //printf("%s/n", path);
        
    }

    /* 写入信息 */
    inline void Write(const string&msg) {
        switch (outType) {
            case StdOut:
                cout << msg << endl;
                break;
            case StdErr:
                cerr << msg << endl;
                break;
            case FileOut:
                //重新打开
                if (IsOpen()) {
                    OutFile->close();
                }
                OutFile->open(fileFullName, ios::out);
                *OutFile << msg;
                Flush();
                break;
        }
    }

    /* 追加信息 */
    inline void Append(const string&msg) {
        switch (outType) {
            case StdOut:
                cout << msg << endl;
                break;
            case StdErr:
                cerr << msg << endl;
                break;
            case FileOut:
                if (!IsOpen())
                    OutFile->open(fileFullName, ios::app);
                *OutFile << msg;
                Flush();
                break;
        }
    }

    /* 写入缓冲区中的内容 */
    inline void Flush() {
        OutFile->flush();
    }

    /* 关闭文件流 */
    inline void Close() {
        if (OutFile && OutFile->is_open()) {
            assert(outType == FileOut);
            OutFile->clear();
            OutFile->close();
            OutFile = nullptr;
            //DelPtr(OutFile);
        }
    }
public:
    /*---------------------------------------------------------------------*/
    /*                   Static Function[out op]                           */
    /*---------------------------------------------------------------------*/
    //

    static FILE* SecureFOpen(char* name, char* mode) {
        FILE* res;
        res = fopen(name, mode);
        if (!res) {
            TmpErrno = errno;
            Out::SysError("Cannot open file %s", ErrorCodes::FILE_ERROR, name);
        }
        return res;
    }

    static void SecureFClose(FILE* fp) {
        if (fclose(fp)) {
            TmpErrno = errno;
            Out::SysWarning("Problem closing file");
        }
    }

    /* 创建一个输出文件 */
    static void CreateFile(const string&name, const string fileName);

    /* 打开一个输出文件 以创建方式 */
    static FileOp* GetFile(const string&name);

    /* 向指定文件写入内容 */
    static void Write(const string&name, const string&msg);

    /* 追加内容 */
    static void Append(const string&name, const string&msg);

    /* 关闭指定文件 */
    static void Close(const string&name);

    /* 关闭所有文件 */
    static void CloseAll();

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
    /// 提取文件的基本名称
    /// \param fileFullName
    /// \param baseName

    inline static string FileNameBaseName(const string& fileFullName) {
        assert(!fileFullName.empty());
        int pos = fileFullName.find_last_of('/');
        return fileFullName.substr(pos + 1);

    }




private:

};

#endif /* FILEOP_H */

