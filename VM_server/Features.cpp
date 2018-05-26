//
// Created by hengliy on 5/19/18.
//
#include <algorithm>
#include <sstream>
#include <vector>
#include <iostream>
using namespace std;
#include "definition.h"

//static MalSoftware virus[] = {
//        { "systemd-udevd", "Only fot test1", 1, { { 100, 2, {0x41,0x57,0x41,0x56,0x41,0x55,0x49,0x89,0xf5,0x41,0x54,0x55,0x89,0xfd,0x53,0x48,0x81,0xec,0x88,0x6,0,0,0x64,0x48,0x8b,0x4,0x25,0x28,0,0}},{},{},{},{}} },
//        { "RootKit2", "Only fot test2", 1, { { 37, 23,{ 0xf, 0x1f, 0x44, 0x0, 0x0,
//                                                               0x48, 0x83, 0x3d, 0xdb, 0x21, 0x0, 0x0, 0x0, 0x55, 0x48, 0x89,
//                                                               0xe5,0x53, 0x75, 0x26, 0x48, 0xc7, 0xc1, 0x54 } },{},{},{} } },
//        { "RootKit3", "Only fot test2", 1, { { 37, 23,{ 0xf, 0x1f, 0x44, 0x0, 0x0,
//                                                               0x48, 0x83, 0x3d, 0xfb, 0x21, 0x0, 0x0, 0x0, 0x55, 0x48, 0x89,
//                                                               0xe5, 0x53, 0x75, 0x26, 0x48, 0xc7, 0xc1, 0x51 }  },{},{},{} } },
//        { "RootKit4", "Only fot test2", 1, { { 37, 23,{ 0xf, 0x1f, 0x44, 0x0, 0x0,
//                                                               0x48, 0x83, 0x3d, 0xa3, 0x30, 0x0, 0x0, 0x0, 0x55, 0x48, 0x89,
//                                                               0xe5, 0x53, 0x75, 0x34, 0x48, 0x83, 0x3d, 0x84 }  },{},{},{} } }
//};

//文件读取
int read_file(char ** const buff, const unsigned int spec, const char * const filename)
{
    FILE *fp = fopen(filename, "r");
    if (fp == NULL)
    {
        printf("Fail to open file %s\n", filename);
        return 0;
    }
    printf("Open file %s OK.\n", filename);

    char line[MAX_LINE_LEN + 2];
    unsigned int cnt = 0;
    while ((cnt < spec) && !feof(fp))//是否超过最大长度，且没有结束
    {
        line[0] = 0;
        if (fgets(line, MAX_LINE_LEN + 2, fp) == NULL)  continue;
        if (line[0] == 0)   continue;

        buff[cnt] = (char *)malloc(MAX_LINE_LEN + 2);//给一行申请内存
        memcpy(buff[cnt], line,MAX_LINE_LEN + 2);//拷贝这一样的字符串
        buff[cnt][MAX_LINE_LEN + 1] = 0;//末尾补零
        cnt++;//行数+1
    }
    fclose(fp);
    printf("There are %d lines in file %s.\n", cnt, filename);
    return cnt;
}

vector<MalSoftware> read_virus(const char * const filename)
{
    vector<MalSoftware> viruss;
    char *data[MAX_DATA_NUM];//最大数据
    int len=read_file(data,MAX_DATA_NUM,filename);
    int seek=0;
    while(seek<len)//遍历整个文件
    {
        MalSoftware temp;
        temp.name=data[seek];
        temp.description=data[seek+1];
        temp.num=atoi(data[seek+2]);
        seek+=3;
        for(int j=0;j<temp.num;j++)//遍历所有病毒的所有特征
        {
            stringstream sstream;
            sstream<<data[seek];
            sstream>>temp.feats[j].offset>>temp.feats[j].len;
            sstream.clear();
            seek++;

            for(int k=0;k<temp.feats[j].len;k++)//遍历病毒特征代码
            {
                temp.feats[j].f[k]=data[seek][k];
            }

            sstream.clear();
            seek++;
        }

        viruss.push_back(temp);
    }

    for(int i=0;i<viruss.size();i++)
    {
        MalSoftware temp=viruss[i];
        cout<<temp.name;
        cout<<temp.description;
        cout<<temp.num<<endl;
        for(int j=0;j<temp.num;j++)
        {
            cout<<temp.feats[j].offset<<" "<<temp.feats[j].len<<endl;
            for(int k=0;k<temp.feats[j].len;k++)
            {
                cout<<temp.feats[j].f[k]<<" ";
            }
            cout<<endl;
        }
    }
    return viruss;
}

bool cmp_code(uint8_t code[],const vector<MalSoftware>& viruss)
{
    vector<uint8_t> codev(code,code+MAX_CODE_LEN);//拿出代码段
	for(int i=0;i<viruss.size();i++)//遍历病毒库
	{
		for(int j=0;j<viruss[i].num;j++)//遍历特征
		{
            vector<uint8_t> virus_code(viruss[i].feats[j].f,viruss[i].feats[j].f+viruss[i].feats[j].len);
			vector<uint8_t>::iterator it=search(codev.begin(),codev.end(),virus_code.begin(),virus_code.end());

            if(it!=codev.end())
			{
				return true;
			}
		}
	}
	return false;
}

void rec_process(VM_process &process,int b,vector<VM_process>& vec_process,const vector<MalSoftware>& viruss)
{
    VM_list_head *temp = NULL;
	VM_process *temp_process=NULL;

	for (int i = 0; i < b; ++i) {
		cout<<"---";
	}cout<<"|";
	cout<<process.pid<<" "<<process.comm;

    //对比病毒
	if(cmp_code(process.code,viruss))
	{
		vec_process.push_back(process);
        cout<<" find virus in "<<process.comm<<endl;
	}else
    {
        cout<<" ok!!!"<<endl;
    }
	cout<<"\n";
	if (NULL!=process.children.next)
	{
		for (temp = process.children.next;temp!=&process.children ;temp = temp->next)
		{
			temp_process = VM_list_entry(temp,struct VM_process,sibling);
			rec_process(*temp_process,b+1,vec_process,viruss);
		}
	}
}

void rec_module(VM_module &module,int b,vector<VM_module>& vec_module,const vector<MalSoftware>& viruss)
{
    //  cout<<"in print_process_list\n";
    VM_list_head *post = &module.list;
    do{
        VM_module *temp_module = VM_list_entry(post,struct VM_module,list);
        if(cmp_code(temp_module->code,viruss))
        {
            vec_module.push_back(*temp_module);
            cout<<" find virus in "<<temp_module->name<<endl;
        }else
        {
            cout<<" ok!!!"<<endl;
        }
        post = post->next;
    }while(post != module.list.next->pre);
}


//插入病毒
//参数1：病毒类型
//参数2：插入文件路径
bool insert_virus(MalSoftware& virus,const char * const filename)
{
    FILE *fp = fopen(filename, "a");
    if (fp == NULL)
    {
        printf("Fail to open file %s\n", filename);
        return false;
    }
    printf("Open file %s OK.\n", filename);
    string buff_str=virus.name;//插入病毒名字
    buff_str+="\n";
    buff_str+=virus.description;
    buff_str+="\n";
    buff_str+=to_string(virus.num);
    buff_str+="\n";

    for(int i=0;i<virus.num;i++)//分别插入病毒的特征
    {
        MalSoftwareFeature temp=virus.feats[i];
        buff_str+=to_string(temp.offset);
        buff_str+=" ";
        buff_str+=to_string(temp.len);
        buff_str+="\n";

        for(int j=0;j<temp.len;j++)
        {
            buff_str+=(char)temp.f[j];
        }
        buff_str+="\n";

        char buff[buff_str.length()];
        for(int j=0;j<buff_str.length();j++)
        {
            buff[j]=buff_str[j];
        }

        fwrite(buff, sizeof(char), buff_str.size(), fp);
        buff_str="";
    }
    fclose(fp);
    return true;
}

vector<VM_process> cmp_virus_process(VM_process &root,char* virus_filename)
{
	vector<MalSoftware> viruss=read_virus(virus_filename);
    vector<VM_process> vec_process;
    if(viruss.size()==0)
    {
        cout<<"read virus error"<<endl;
        return vec_process;
    }
	rec_process(root,0,vec_process,viruss);
    return vec_process;
}

vector<VM_module> cmp_virus_module(VM_module& module,char* virus_filename)
{
    vector<MalSoftware> viruss=read_virus(virus_filename);

    vector<VM_module> vec_module;
    if(viruss.size()==0)
    {
        cout<<"read virus error"<<endl;
        return vec_module;
    }
    rec_module(module,0,vec_module,viruss);
    return vec_module;
}