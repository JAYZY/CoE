/* 
 * File:   Environment.h
 * Author: Zhong Jian<77367632@qq.com>
 *
 * Created on 2017年12月23日, 下午2:24
 */

#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H
#include "IncDefine.h"


#include "INOUT/Scanner.h"

#include "Terms/GroundTermBank.h" 

extern int TmpErrno;
//class GroundTermBank;
class Env {
private:

    static Scanner* in;
    
    static GroundTermBank* GTBank; // 全局唯一一个基项bank 存储共享基项term 只存储不删除.
    static Sig_p sig; // 全局唯一一个sig 存储项相关的符号 只存储不删除
public:
    static IOFormat parseFormat;  //文件的输入类型
    static string tptpFileName; //文件名
    static string ExePath;//程序执行目录绝对地址
    //static string ProgName; //程序名
    static uint32_t global_formula_counter;
    static uint32_t global_clause_counter;
    static uint32_t backword_CMP_counter; //backword 比较次数
    static uint32_t backword_Finded_counter; //backword 找到冗余次数
    static uint32_t forward_Finded_counter; //backword 找到冗余次数

    static uint16_t S_OverMaxLitLimit_Num;
    static uint16_t S_ASame2R_Num;
    static uint16_t S_ASame2A_Num; //主界线文字相同次数

public:
    /*---------------------------------------------------------------------*/
    /*                    Constructed Function                             */
    /*---------------------------------------------------------------------*/
    //
    Env();
    virtual ~Env();
public:

    /*---------------------------------------------------------------------*/
    /*                          Static Function                            */
    /*---------------------------------------------------------------------*/
    //
    /// 初始化全局扫描器,读取文件
    /// \param type 扫描器类型
    /// \param name 读取文件名称
    /// \param ignore_comments 是否忽略注释,往往是/*开头
    /// \param default_dir 默认路径(一般为nullptr)

    static void IniScanner(StreamType type, char *name, bool ignore_comments, char *default_dir) {
        in = new Scanner(type, name, ignore_comments, default_dir);
        in->SetFormat(parseFormat);
        //if (parseFormat == IOFormat::AutoFormat && Env::in->format == IOFormat::TSTPFormat) {
        // Options::OutputFormat = IOFormat::TSTPFormat;
        //}
    }

    static Scanner* getIn() {
        assert(in);
        return in;
    }

    static GroundTermBank* getGTbank() {
        if (!GTBank)
            GTBank = new GroundTermBank();
        assert(GTBank);
        return GTBank;
    }

    static Sigcell* getSig() {
        //初始化TermBank的时候,插入两个特殊的项 $True $False
        if (!sig)
            sig = new Sigcell();
        return sig;
    }


    /*--------------------------------------------------------------------------
      /* Print resource usage to given stream.
      /-------------------------------------------------------------------------*/
    static double GetTotalTime();

    static void PrintRusage(char* sRusage);

    static inline void PrintRunInfo(FILE* out) {
        fprintf(out,
                "# R与主界线相同次数            : %u \n", S_ASame2R_Num);
        fprintf(out,
                "# 主界线文字相同次数           : %u \n", S_ASame2A_Num);
        fprintf(out,
                "# R超过最大文字数限制          : %u \n", S_OverMaxLitLimit_Num);

    }
private:

};



#endif /* ENVIRONMENT_H */

