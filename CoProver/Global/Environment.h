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

class Env {
private:

    static Scanner* in;
    static IOFormat parseFormat;
    static GTermBank_p GTBank; // 全局唯一一个基项bank 存储共享基项term 只存储不删除.
    static Sig_p sig; // 全局唯一一个sig 存储项相关的符号 只存储不删除
public:
    static string tptpFileName; //文件名

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
        if (parseFormat == IOFormat::AutoFormat && Env::in->format == IOFormat::TSTPFormat) {
            Options::OutputFormat = IOFormat::TSTPFormat;
        }
    }

    static Scanner* getIn() {
        assert(in);
        return in;
    }

    static GTermBank_p getGTbank() {
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
    static inline double GetTotalTime() {
        struct rusage usage, cusage;

        if (getrusage(RUSAGE_SELF, &usage)) {
            TmpErrno = errno;
            Out::SysError("Unable to get resource usage information", ErrorCodes::SYS_ERROR);
        }
        if (getrusage(RUSAGE_CHILDREN, &cusage)) {
            TmpErrno = errno;
            Out::SysError("Unable to get resource usage information", ErrorCodes::SYS_ERROR);
        }
        usage.ru_utime.tv_sec += cusage.ru_utime.tv_sec;
        usage.ru_utime.tv_usec += cusage.ru_utime.tv_usec;
        usage.ru_stime.tv_sec += cusage.ru_stime.tv_sec;
        usage.ru_stime.tv_usec += cusage.ru_stime.tv_usec;

        double spanTime = (usage.ru_utime.tv_sec + usage.ru_stime.tv_sec)+
                ((usage.ru_utime.tv_usec + usage.ru_stime.tv_usec) / 1000000.0);
        return spanTime;
    }

    static inline void PrintRusage(char* sRusage) {
        struct rusage usage, cusage;

        if (getrusage(RUSAGE_SELF, &usage)) {
            TmpErrno = errno;
            Out::SysError("Unable to get resource usage information", ErrorCodes::SYS_ERROR);
        }
        if (getrusage(RUSAGE_CHILDREN, &cusage)) {
            TmpErrno = errno;
            Out::SysError("Unable to get resource usage information", ErrorCodes::SYS_ERROR);
        }
        usage.ru_utime.tv_sec += cusage.ru_utime.tv_sec;
        usage.ru_utime.tv_usec += cusage.ru_utime.tv_usec;
        usage.ru_stime.tv_sec += cusage.ru_stime.tv_sec;
        usage.ru_stime.tv_usec += cusage.ru_stime.tv_usec;

        // outStr = "\n# -------------------------------------------------\n";
        //        outStr+="# Maximum ClauseID         : "+to_string(Env::global_clause_counter)+"\n";
        //        outStr+="# User time                : "+to_string((usage.ru_utime.tv_sec)+(usage.ru_utime.tv_usec) / 1000000.0)+" s\n";
        //        outStr+="# System time              : "+to_string((usage.ru_stime.tv_sec)+(usage.ru_stime.tv_usec) / 1000000.0)+" s\n"
        string sOut =
                "\n%% -------------------------------------------------\n%% SZS output end Proof.\n";
        sOut += "%% User time                   : %.3f s\n";
        sOut += "%% System time                 : %.3f s\n";
        sOut += "%% Total time                  : %.3f s\n";
        sOut += "%% Maximum resident set size   : %ld pages\n";
        sOut += "%% Maximum ClauseID            : %d \n";
        float userTime = (usage.ru_utime.tv_sec)+(usage.ru_utime.tv_usec) / 1000000.0f;
        float sysTime = (usage.ru_stime.tv_sec)+(usage.ru_stime.tv_usec) / 1000000.0f;
        float totalTime = (usage.ru_utime.tv_sec + usage.ru_stime.tv_sec)+((usage.ru_utime.tv_usec + usage.ru_stime.tv_usec) / 1000000.0f);

        sprintf(sRusage, sOut.c_str(), userTime, sysTime, totalTime, usage.ru_maxrss, Env::global_clause_counter);
        //        sprintf(sRusage,
        //                "%% User time                : %.3f s\n", sOut,
        //                (usage.ru_utime.tv_sec)+(usage.ru_utime.tv_usec) / 1000000.0);
        //        sprintf(sRusage,
        //                "%% System time              : %.3f s\n",
        //                (usage.ru_stime.tv_sec)+(usage.ru_stime.tv_usec) / 1000000.0);
        //        sprintf(sRusage,
        //                "%% Total time               : %.3f s\n",
        //                (usage.ru_utime.tv_sec + usage.ru_stime.tv_sec)+
        //                ((usage.ru_utime.tv_usec + usage.ru_stime.tv_usec) / 1000000.0));
        //        sprintf(sRusage,
        //                "%% Maximum resident set size: %ld pages\n", usage.ru_maxrss);
        //        sprintf(sRusage,
        //                "%% Maximum ClauseID         : %d \n", Env::global_clause_counter);
    }

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

