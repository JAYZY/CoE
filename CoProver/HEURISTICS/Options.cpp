/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Options.cpp
 * Author: Zhong Jian<77367632@qq.com>
 * 
 * Created on 2017年12月22日, 下午4:13
 */



#include "Options.h"
IOFormat Options::OutputFormat = IOFormat::AutoFormat;
std::string Options::ProgName = "CoProver";

Options::Options() {
}

Options::Options(const Options& orig) {
}

Options::~Options() {
}

