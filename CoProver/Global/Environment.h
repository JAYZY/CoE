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
    static uint32_t global_formula_counter;
    static uint32_t global_clause_counter;
    static uint32_t backword_CMP_counter; //backword 比较次数
    static uint32_t backword_Finded_counter; //backword 找到冗余次数
 static uint32_t forward_Finded_counter; //backword 找到冗余次数
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

private:

};



#endif /* ENVIRONMENT_H */

