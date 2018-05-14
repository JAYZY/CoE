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
#include "TERMS/TermBank.h"

class Env {
private:

    static Scanner* in;
    static IOFormat parseFormat;
    static TermBank* termBank; // 全局唯一一个项bank 存储共享 term
    static Sigcell* sig; // 全局唯一一个sig 存储项相关的符号
public:
    static uint64_t global_formula_counter;
    static uint64_t global_clause_counter;
    static uint64_t backword_CMP_counter; //backword 比较次数
    static uint64_t backword_Finded_counter; //backword 找到冗余次数
    
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

    static void iniScanner(StreamType type, char *name, bool ignore_comments, char *default_dir) {
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

    static TermBank* getTb() {
        if (!termBank)
            termBank = new TermBank();
        assert(termBank);
        return termBank;
    }

    static Sigcell* getSig() {
        //初始化TermBank的时候,插入两个特殊的项 $True $False
        if (!sig)
            sig = new Sigcell();
        return sig;
    }

private:

};
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
static inline double cpuTime(void);

static inline double cpuTime(void) {
    struct rusage ru;
    getrusage(RUSAGE_SELF, &ru);
    return (double) ru.ru_utime.tv_sec + (double) ru.ru_utime.tv_usec / 1000000;
}

static inline void paseTime(const char* tip, double initial_time) {
    printf("|  %stime:           %12.2f s                 |\n", tip, cpuTime() - initial_time);

}


#endif /* ENVIRONMENT_H */

