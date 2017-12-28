/*
 * 文件操作类
 * File:   FileOp.cpp
 * Author: zj 
 * 
 * Created on 2017年3月13日, 上午10:15
 */
#include "FileOp.h"
#include <fstream>


            
string FileOp::TPTP_dir = "";
//IOFormat FileOp::OutputFormat = IOFormat::AutoFormat;
long FileOp::OutputLevel = 1;
int FileOp::TmpErrno = 0;
FILE* FileOp::GlobalOut = stdout;

map<string, FileOp*> FileOp::lsOut;

/*---------------------------------------------------------------------*/
/*                    Constructed Function                             */
/*---------------------------------------------------------------------*/



/*---------------------------------------------------------------------*/
/*                  Member Function-[public]                           */
/*---------------------------------------------------------------------*/
//

/*---------------------------------------------------------------------*/
/*                          Static Function                            */
/*---------------------------------------------------------------------*/

/* 创建一个输出文件 */
void FileOp::CreateFile(const string&name, string fileName) {
    if (lsOut.find(name) == lsOut.end()) {
        //没有找到该文件
        lsOut[name] = new FileOp(name, fileName);
    }
}

FileOp* FileOp::GetFile(const string&name) {
    if (lsOut.find(name) == lsOut.end()) {
        //没有找到该文件
        return nullptr;
    }
    return lsOut[name];
}

/* 向指定文件写入内容 */
void FileOp::Write(const string&name, const string&msg) {
    FileOp* file = GetFile(name);
    if (file == nullptr) {
        cout << "File:" << name << " for Write,No Exist!" << endl;
        return;
    }
    file->Write(msg);
}

void FileOp::Append(const string&name, const string&msg) {
    FileOp* file = GetFile(name);
    if (file == nullptr) {
        cout << "File:" << name << " for Append,No Exist!" << endl;
        return;
    }
    file->Append(msg);
}

/* 关闭指定文件 */
void FileOp::Close(const string&name) {
    FileOp* file = GetFile(name);
    if (file != nullptr && file->IsOpen()) {
        DelPtr(file);
        lsOut.erase(name);
    }
}

/* 关闭所有文件 */
void FileOp::CloseAll() {
    for (auto&f : lsOut) {
        if (f.second->outType == FileOut)
            //f.second->Close();
            DelPtr(f.second);
    }
    lsOut.clear();
}