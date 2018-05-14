/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "SysDate.h"

void SysDatePrint(FILE* out, SysDate date) {
    fprintf(out, "%5lu", date);
}