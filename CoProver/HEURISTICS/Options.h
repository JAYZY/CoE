/* 
 * File:   Options.h
 * Contents: 配置运行参数
 * Author: Zhong Jian<77367632@qq.com>
 * 
 * Created on 2017年12月22日, 下午4:13
 */

#ifndef OPTIONS_H
#define OPTIONS_H
//输入文件的格式
#include <string>

enum class IOFormat {
    LOPFormat = 0,
    TPTPFormat = 2,
    TSTPFormat,
    AutoFormat,
};

class Options {
public:
    static IOFormat OutputFormat; //文件输出格式
    static std::string ProgName;
    static uint32_t step_limit; //运行总次数
    Options();
    Options(const Options& orig);
    virtual ~Options();
private:

};

#endif /* OPTIONS_H */

