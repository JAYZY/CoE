/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   SysDate.h
 * Author: Zhong Jian<77367632@qq.com>
 *
 * Created on 2018年4月23日, 下午8:15
 */

#ifndef SYSDATE_H
#define SYSDATE_H
#include "IncDefine.h"
typedef long SysDate;

/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define SysDateCreationTime() ((SysDate)0)
#define SysDateInvalidTime() ((SysDate)-1)
#define SysDateIsInvalid(date) ((date) == SysDateInvalidTime())
#define SysDateInc(sd) ((*(sd))++);assert(*(sd));
#define SysDateIsEarlier(date1, date2) ((date1)<(date2))
#define SysDateEqual(date1, date2) ((date1)==(date2))

#define SysDateMaximum(date1, date2) MAX(date1, date2)
#define SysDateIsCreationDate(date) ((date) == SysDateCreationTime())

void SysDatePrint(FILE* out, SysDate date);

#endif /* SYSDATE_H */

