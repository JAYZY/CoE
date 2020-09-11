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
#include "Global/Environment.h"
using namespace std;




//对应的命令行枚举型

enum class OptCodes {
    NoOpt = 0,
    Help, // 帮助
    Version, //版本
    Format,
    MaxLit //文字长度  

};

/* 命令行参数类型 */
enum class OptArgType {
    NoArg, //无参数
    NumArg, //数字参数,包括整数,浮点数
    StrArg //字符串参数
};

typedef struct optcell {
    OptCodes option_code;
    string shortopt; /* Single Character options */
    string longopt; /* Double dash, GNU-Style */
    OptArgType type; /* What about Arguments? */
    const char* arg_default; /* Default for optional argument (long style only */
    const char* desc; /* Put the documentation in immediately! */
} OptCell, *Opt_p;

class Options {
public:
    
    static uint32_t step_limit; //运行总次数
    //命令行提取和分析
    static bool AnalyseOpt(int argc,char** argv);

    Options();

    Options(const Options& orig);
    virtual ~Options();

private:
    static void ModifyOpt(const OptCell* optcell,string& value);
    const static OptCell Opts[];
    static int GetOptSize();
public:
    //返回指令

    static const OptCell* GetOption(string& sOpt, bool isShort) {

        for (int i = 0; i < GetOptSize(); i++) {
            const OptCell* opt = &Opts[i];
            if (isShort && (opt->shortopt == sOpt)) {
                return opt;
            }
            else if (opt->longopt == sOpt) {
                return opt;
            }

        }
        return nullptr;
    }
   
};

#endif /* OPTIONS_H */

