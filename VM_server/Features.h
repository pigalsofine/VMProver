	///////////////////////////////////////////////////////////////////////////////
// Name:        Features.h
// Purpose:     Head file for Features.cpp. 
// Author:      OS Group
// Modified by: Haoyu Yaobei
// Created:     2017-06-15
// Copyright:   (C) Copyright 2017, OS Group, HFUT, All Rights Reserved.
// Licence:     OS group Licence, Version 1.0
///////////////////////////////////////////////////////////////////////////////

#ifndef __FEATURES_H__
#define __FEATURES_H__

#include <iostream>
#include <vector>
#include <string>
using namespace std;

#define MAX_DATA_NUM 50000
#define MAX_LINE_LEN 55000
#define MAX_FEATURES_LEN 1000
#define MAX_CODE_LEN 1000
#define MAX_FEATURES_NUM 5

// 恶意软件代码特征
typedef struct _MalSoftwareFeature {
	int offset;         // 偏移量
	int len;            // 长度
	uint8_t f[MAX_FEATURES_LEN];      //特征代码
} MalSoftwareFeature ;

// 恶意软件
typedef struct _MalSoftware {
	char *name;
	char *description;
	int num;    //实际特征数
	MalSoftwareFeature feats[MAX_FEATURES_NUM];//特征数组
} MalSoftware;

class VM_process;
vector<VM_process> cmp_virus_process(VM_process &root,char* virus_filename);
class VM_module;
vector<VM_module> cmp_virus_module(VM_module& module,char* virus_filename);

//病毒库的维护函数
bool insert_virus(MalSoftware& virus,const char * const filename);
vector<MalSoftware> read_virus(const char * const filename);

#endif
