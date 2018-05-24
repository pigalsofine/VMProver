#ifndef _PROCESS_LIST_H
#define _PROCESS_LIST_H

#include "definition.h"
#include <algorithm>
#include <iostream>
using namespace std;

int start_socket();

void print_process_list(VM_vmi &vmi);

vector<VM_module*> cmp_lsmod(VM_vmi &vmi_os);

vector<VM_file*> cmp_file(VM_vmi& vmi_os,char* path);

void VM_find_process_list(VM_vmi &vmi_os,int cfd);

void VM_find_process_tree(VM_vmi &vmi_os,int cfd);

bool ShowTree(VM_process &process,int b);

bool RecvAll(int &sock, char*buffer, int size);

void VM_find_module_list(VM_vmi &vmi_os,int cfd);

void print_module_list(VM_vmi &vmi);

void VM_find_module_asm(VM_module &module);

vector<VM_process*> cmp_process(VM_vmi &vmi_os);

int VM_find_process_hide(VM_vmi &vmi_os);

int test_socket();
#endif
