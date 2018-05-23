#include <iostream>
#include <vector>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "wrap.h"
#include "process_list.h"
#include "definition.h"
#include "getTxt.h"

#define SERV_IP "127.0.0.1"
#define SERV_PORT 9527
	

bool SendAll(int &sock, char*buffer, int size)
{
    while (size>0)
    {
        int SendSize= send(sock, buffer, size, 0);
        if(-1==SendSize)
            return false;
        size = size - SendSize;//用于循环发送且退出功能
        buffer+=SendSize;//用于计算已发buffer的偏移量
    }
    return true;
}

void send_process_list(int sfd,VM_vmi &vmi){
	VM_find_process_list(vmi,vmi.process);//链式
	vector<VM_process> vec_VMPro;
	VM_list_head *post = &vmi.process.tasks;
	do{
		VM_process *temp_process = VM_list_entry(post,struct VM_process,tasks);
		vec_VMPro.push_back(*temp_process);
		post = post->next;
	}while(post != &vmi.process.tasks);



   	for (int i = 0; i < vec_VMPro.size(); ++i)
   	{
   		int ret = SendAll(sfd, (char *)&vec_VMPro[i], sizeof(VM_process));       //写个服务器
        cout<<"aa  "<<vec_VMPro[i].comm<<"\n";
   	}

    //服务器终止接收条件
    VM_process process;
    process.pid = -1;
    SendAll(sfd, (char *)&process, sizeof(process));
}

bool ShowTree(VM_process &process,int b,int sfd){
    VM_list_head *temp = NULL;
    VM_process *temp_process=NULL;

    for (int i = 0; i < b; ++i) {
        cout<<"---";
    }cout<<"|";
    SendAll(sfd, (char *)&process, sizeof(process));
    cout<<process.pid<<" "<<process.comm;

    for (int i = 0; i < process.num_children_pid; ++i) {
        cout<<process.children_pid[i]<<" ";
    }
    cout<<"\n";
    if (NULL!=process.children.next)
    {
        for (temp = process.children.next;temp!=&process.children ;temp = temp->next)
        {
            temp_process = VM_list_entry(temp,struct VM_process,sibling);
            ShowTree(*temp_process,b+1,sfd);
        }
    }
    return 1;

}

void send_process_tree(int sfd,VM_vmi &vmi){
	VM_find_process_tree(vmi,vmi.process);//tree
    ShowTree( vmi.process,0,sfd);
    //服务器终止接收条件
    VM_process process;
    process.pid = -1;
    SendAll(sfd, (char *)&process, sizeof(process));
}

void send_module_list(int sfd,VM_vmi &vmi){
    VM_find_modules_list(vmi,vmi.module);
    VM_list_head *post = &vmi.module.list;
    do{
        VM_module *temp_module = VM_list_entry(post,struct VM_module,list);
        SendAll(sfd, (char *)temp_module, sizeof(VM_module));
        post = post->next;
    }while(post != &vmi.module.list);

    VM_module end_module;
    strcpy(end_module.name,"last_module");
    SendAll(sfd, (char *)&end_module, sizeof(VM_module));
}


void show_file_tree(VM_file* fileNode,int deep,int sfd)
{
    VM_list_head* p;

    for(p=fileNode->subdirs.next;p!=&(fileNode->subdirs);p=p->next){
        VM_file* tempNode=VM_list_entry(p,struct VM_file,bro);

        SendAll(sfd, (char *)tempNode, sizeof(*tempNode));

        for (int i = 0; i < deep; ++i) {
            cout<<"---";
        }cout<<"|";
		cout<<tempNode->id<<" "<<tempNode->name<<" "<<" ";
        for (int i = 0; i < tempNode->num_children_id; ++i) {
            cout<<tempNode->children_id[i]<<" ";
        }cout<<"\n";
        if(tempNode->subdirs.next)
        {
            show_file_tree(tempNode,deep+1,sfd);
        }
    }

}

void send_file_tree(int sfd,VM_vmi &vmi){
    init_file_tree(vmi,"/home/");//tree
    show_file_tree(&vmi.file_root,0,sfd);
    //服务器终止接收条件
    VM_file file;
    file.id = -1;
    SendAll(sfd, (char *)&file, sizeof(file));
}



int  start_socket(){
	int sfd, len;
    struct sockaddr_in serv_addr;
    char buf[BUFSIZ]; 

    /*创建一个socket 指定IPv4 TCP*/
    sfd = socket(AF_INET, SOCK_STREAM, 0);

    /*初始化一个地址结构:*/
    bzero(&serv_addr, sizeof(serv_addr));                       //清零
    serv_addr.sin_family = AF_INET;                             //IPv4协议族
    inet_pton(AF_INET, SERV_IP, &serv_addr.sin_addr.s_addr);    //指定IP 字符串类型转换为网络字节序 参3:传出参数
    serv_addr.sin_port = htons(SERV_PORT);                      //指定端口 本地转网络字节序

    /*根据地址结构链接指定服务器进程*/
    connect(sfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    return sfd;
}

void send_ps_file(int sfd,VM_vmi &vmi){
    //pclose(getTxt("ps", ""));

    char buffer[1024];
    string file_name;
    file_name = "ps.txt";
    FILE *fp = fopen(file_name.data(), "r");
    if (fp == NULL)
    {
        printf("File:\t%s Not Found!\n", file_name.data());
    }
    else
    {
        bzero(buffer, BUFFER_SIZE);
       // while( (file_block_length = fread(buffer, sizeof(char), (size_t)1024, fp)) > 0)
        while (fgets(buffer,BUFFER_SIZE,fp) != NULL)
        {
            //cout<<buffer<<"\n";
            // 发送buffer中的字符串到new_server_socket,实际上就是发送给客户端
            if (send(sfd, buffer, BUFFER_SIZE, 0) < 0)
            {
                printf("Send File:\t%s Failed!\n", file_name);
                break;
            }

            bzero(buffer, BUFFER_SIZE);
        }
        fclose(fp);

        if (send(sfd, "send_ps_file_end", BUFFER_SIZE, 0) < 0)
        {
            printf("Send File:\t%s Failed!\n", file_name);

        }

        printf("File:\t%s Transfer Finished!\n", file_name.data());
    }

}

void send_lsmod_file(int sfd,VM_vmi &vmi){
    pclose(getTxt("lsmod",""));

    char buffer[1024];
    string file_name;
    file_name = "lsmod.txt";
    FILE *fp = fopen(file_name.data(), "r");
    if (fp == NULL)
    {
        printf("File:\t%s Not Found!\n", file_name.data());
    }
    else
    {
        bzero(buffer, BUFFER_SIZE);
        int file_block_length = 0;
        //while( (file_block_length = fread(buffer, sizeof(char), 1024, fp)) > 0)
        while (fgets(buffer,BUFFER_SIZE,fp) != NULL)
        {

            // 发送buffer中的字符串到new_server_socket,实际上就是发送给客户端
            if (send(sfd, buffer, BUFFER_SIZE, 0) < 0)
            {
                printf("Send File:\t%s Failed!\n", file_name);
                break;
            }

            bzero(buffer, BUFFER_SIZE);
        }
        fclose(fp);
        if (send(sfd, "send_lsmod_file_end", BUFFER_SIZE, 0) < 0)
        {
            printf("Send File:\t%s Failed!\n", file_name);
        }

        printf("File:\t%s Transfer Finished!\n", file_name.data());
    }

}

void send_tree_file(int sfd,VM_vmi &vmi){
    pclose(getTxt("tree", "/home/"));

    char buffer[1024];
    string file_name;
    file_name = "tree.txt";
    FILE *fp = fopen(file_name.data(), "r");
    if (fp == NULL)
    {
        printf("File:\t%s Not Found!\n", file_name.data());
    }
    else
    {
        bzero(buffer, BUFFER_SIZE);
        int file_block_length = 0;
       // while( (file_block_length = fread(buffer, sizeof(char), 1024, fp)) > 0)
        while (fgets(buffer,BUFFER_SIZE,fp) != NULL)
        {
         //   cout<<buffer;
            // 发送buffer中的字符串到new_server_socket,实际上就是发送给客户端
            if (send(sfd, buffer, BUFFER_SIZE, 0) < 0)
            {
                printf("Send File:\t%s Failed!\n", file_name);
                break;
            }

            bzero(buffer, BUFFER_SIZE);
        }
        fclose(fp);

        if (send(sfd, "send_tree_file_end", BUFFER_SIZE, 0) < 0)
        {
            printf("Send File:\t%s Failed!\n", file_name);
        }

        printf("File:\t%s Transfer Finished!\n", file_name.data());
    }

}

int main(void)
{
	VM_vmi vmi;
	VM_vmi_init("ubuntu14.04",vmi);//初始化虚拟机
    int sfd = start_socket();
    char buffer[128];

    while (1) {
        //Write(sockfd, buf, strlen(buf));
        int n = Read(sfd, buffer, 128);
        //printf("a\n");
        if (n == 0){
            printf("the other side has been closed.\n");
            break;
        }
        else if (n > 0){
            if ( 0 == strcmp(buffer,"get_VM_name") ){
                Write(sfd, "11", 3);
                sleep(1);
                Write(sfd,"ubuntu14.04",128);
                printf("get_VM_name\n");
            } else if ( 0 == strcmp(buffer,"get_process_tree") ){
                Write(sfd, "12", 3);
                //sleep(1);
                send_process_tree(sfd,vmi);
                printf("get_process_tree\n");
            } else if ( 0 == strcmp(buffer,"get_module_list") ){
                cout<<"get_module_list\n";
                Write(sfd, "13", 3);
                send_module_list(sfd,vmi);
            } else if ( 0 == strcmp(buffer,"get_file_tree") ){
                cout<<"get_file_tree\n";
                Write(sfd, "14", 3);
                send_file_tree(sfd,vmi);
            } else if (0 == strcmp(buffer,"get_process_code")){
                uint64_t start;
                uint64_t end;
                int pid;
                uint8_t a[1000];
                Read(sfd, &pid, sizeof(int));
                Read(sfd, &start, sizeof(uint64_t));
                Read(sfd, &end, sizeof(uint64_t));

                VM_find_process_code_byaddress(a, start, end,vmi, pid);

                Write(sfd, "15", 3);
                SendAll(sfd, (char *)a, sizeof(a));
            } else if (0 == strcmp(buffer,"get_ps_file")) {
                Write(sfd, "16", 3);

                send_ps_file(sfd,vmi);
            } else if (0 == strcmp(buffer,"get_lsmod_file")) {
                Write(sfd, "17", 3);

                send_lsmod_file(sfd,vmi);
            } else if (0 == strcmp(buffer,"get_tree_file")) {
                Write(sfd, "18", 3);

                send_tree_file(sfd,vmi);
            }
        }
    }

    cout<<"end main\n";
    /*关闭链接*/
    close(sfd);

    return 0;
}
//
//int main (){
//
//    VM_vmi vmi;
//    VM_vmi_init("ubuntu14.04",vmi);//初始化虚拟机
//
    //    send_file_tree(0,vmi,sfd);
//    return  0;
//}