/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.cpp
 * Author: Zhong Jian<77367632@qq.com>
 *
 * Created on 2019年3月10日, 下午5:43
 */

#include <cstdlib>
#include <iostream>
#include "fstream"  
#include <dirent.h>
#include <stdio.h>
#include <cstring>
using namespace std;
void convertDir();
/*
 * covertTPTP 
 */
int main(int argc, char** argv) {
    convertDir();
    return 0;
}
void convertDir(){
     DIR * dir;
    struct dirent * ptr;
    char file_list[100][40];
    int i=0;
    char srcFile1[1][100];
    string rootdirPath = "/home/icdc/Problems/myprob/一阶基本测试例子/";
    string x,dirPath;
    dir = opendir((char *)rootdirPath.c_str()); //打开一个目录
    while((ptr = readdir(dir)) != NULL) //循环读取目录数据
    {
        printf("d_name : %s\n", ptr->d_name); //输出文件名
        x=ptr->d_name;
        dirPath = rootdirPath + x;
        printf("d_name : %s\n", dirPath.c_str()); //输出文件绝对路径

        strcpy(srcFile1[i],dirPath.c_str()); //存储到数组

        if ( ++i>=100 ) break;
    }
    closedir(dir);//关闭目录指针

}

