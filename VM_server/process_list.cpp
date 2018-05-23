#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <strings.h>
#include <string.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <queue>
#include <map>
#include <sys/epoll.h>
#include <bits/socket.h>
#include <asm/socket.h>
#include "definition.h"
#include "process_list.h"
#include "wrap.h"
using namespace std;
#define SERV_PORT 9527
#define MAXLINE 8192
#define OPEN_MAX 5000

struct VM_info{
    char name[128];
    string ip;
    int connfd;
};

vector<VM_info> vec_VM_info;
VM_vmi vmi_os;
VM_info current_VM_info;


struct connect{
    int	efd;
    int listenfd;
    struct epoll_event tep;
};


int VM_get_ps_file(int sockfd){
    cout<<"in VM_get_ps_file\n";
    char file_name[FILE_NAME_MAX_SIZE + 1];

    char buffer[BUFFER_SIZE];
    bzero(buffer, sizeof(buffer));

    FILE *fp = fopen("ps.txt", "w");
    if (fp == NULL)
    {
        printf("File:\t%s Can Not Open To Write!\n", file_name);
        exit(1);
    }

    // 从服务器端接收数据到buffer中
    bzero(buffer, sizeof(buffer));
    int length = 0;
    while(length = recv(sockfd, buffer, BUFFER_SIZE, 0))
    {
        if ( 0 == strcmp(buffer,"send_ps_file_end") )
            break;
        if (length < 0)
        {

            break;
        }
        cout<<buffer;
        int write_length = fwrite(buffer, sizeof(char), (size_t)strlen(buffer), fp);
        if (write_length < strlen(buffer))
        {
            printf("File:\t%s Write Failed!\n", file_name);
            break;
        }
        bzero(buffer, BUFFER_SIZE);
    }

    // 传输完毕，关闭socket
    fclose(fp);

}

void VM_get_lsmod_file(int sockfd){
    cout<<"in VM_get_lsmod_file\n";
    char file_name[FILE_NAME_MAX_SIZE + 1];

    char buffer[BUFFER_SIZE];
    bzero(buffer, sizeof(buffer));

    FILE *fp = fopen("lsmod.txt", "w");
    if (fp == NULL)
    {
        printf("File:\t%s Can Not Open To Write!\n", file_name);
        exit(1);
    }

    // 从服务器端接收数据到buffer中
    bzero(buffer, sizeof(buffer));
    int length = 0;
    while(length = recv(sockfd, buffer, BUFFER_SIZE, 0))
    {

        if ( 0 == strcmp(buffer,"send_lsmod_file_end") )
            break;

        if (length < 0)
        {

            break;
        }

        int write_length = fwrite(buffer, sizeof(char), strlen(buffer), fp);
        if (write_length < strlen(buffer))
        {
            printf("File:\t%s Write Failed!\n", file_name);
            break;
        }
        bzero(buffer, BUFFER_SIZE);
    }

    // 传输完毕，关闭socket
    fclose(fp);

}

void VM_get_tree_file(int sockfd){
    cout<<"in VM_get_tree_file\n";
    char file_name[FILE_NAME_MAX_SIZE + 1];

    char buffer[BUFFER_SIZE];
    bzero(buffer, sizeof(buffer));

    FILE *fp = fopen("tree.txt", "w");
    if (fp == NULL)
    {
        printf("File:\t%s Can Not Open To Write!\n", file_name);
        exit(1);
    }

    // 从服务器端接收数据到buffer中
    bzero(buffer, sizeof(buffer));
    int length = 0;
    while(length = recv(sockfd, buffer, BUFFER_SIZE, 0))
    {

        if ( 0 == strcmp(buffer,"send_tree_file_end") )
            break;

        if (length < 0)
        {

            break;
        }

        int write_length = fwrite(buffer, sizeof(char), strlen(buffer), fp);
        if (write_length < strlen(buffer))
        {
            printf("File:\t %s Write Failed!\n", file_name);
            break;
        }
        bzero(buffer, BUFFER_SIZE);
    }

    // 传输完毕，关闭socket
    fclose(fp);

}
//
//void VM_find_process_asm(VM_vmi &vmi_os,VM_process &process)
//{
//    string asm_code;
//    for (unsigned long i = process.start_code_address+start_offset; i < process.start_code_address+start_offset+300; i+=1){
//        uint8_t temp=0;
//        vmi_read_8_va(vmi_os.vmi, (addr_t)i, process.pid, &temp);
//        process.code[process.code_long++] = temp;
//        //if(1000<process.code_long)
//        asm_code+=temp;
//    }
//    if (cs_open(CS_ARCH_X86, CS_MODE_64, &process.handle) != CS_ERR_OK)
//        return ;
//    process.asm_count = cs_disasm(process.handle, (uint8_t*)asm_code.c_str(), asm_code.size()-1, process.start_code_address+start_offset, 0, 			&process.insn);
//}


bool RecvAll(int &sock, char*buffer, int size)
{
    while (size>0)//剩余部分大于0
    {
        int RecvSize= recv(sock, buffer, size, 0);
        if(-1==RecvSize)
            return false;
        size = size - RecvSize;
        buffer+=RecvSize;
    }
    return true;
}


void VM_find_file_tree(VM_vmi &vmi_os,int cfd){
    vector<VM_file> vec_VMfile; //保存接收到的process
    VM_file temp_file;
    map<int,VM_file> map_pid_file; //将pid和进程做map
    map<int,VM_file>::iterator itea_file;

    while (1) {
        /*读取客户端发送数据*/
        RecvAll(cfd,(char *)&temp_file, sizeof(temp_file));
        if ( temp_file.id == -1 ){
            break;
        }
        cout<<temp_file.name<<" ";
        for (int i = 0; i < temp_file.num_children_id; ++i) {
            cout<<temp_file.children_id[i]<<" ";
        }cout<<"\n";
        vec_VMfile.push_back(temp_file);
        map_pid_file.insert(pair<int,VM_file>(temp_file.id,temp_file));
    }

    queue<VM_file*> que_file;
    VM_file* root = new VM_file(vec_VMfile[0]);
    root->subdirs.next = NULL;
    root->subdirs.pre = NULL;
    que_file.push(root);
    VM_file *tail_file = NULL;

    while(!que_file.empty()){

        VM_file* temp_file = que_file.front();
        que_file.pop();

        for (int i = 0; i < temp_file->num_children_id; ++i) {

            itea_file = map_pid_file.find( temp_file->children_id[i] );

            VM_file* add_file = new VM_file(itea_file->second);
            add_file->subdirs.pre = NULL;
            add_file->subdirs.next = NULL;

            if (0 == i){
                temp_file->subdirs.next = &add_file->bro;
                temp_file->subdirs.pre =  &add_file->bro;
                add_file->bro.pre = &temp_file->subdirs;
                add_file->bro.next = &temp_file->subdirs;

               // add_file->sibling_pid = process_tree_no_pid;

                tail_file = add_file;
            }else{
                tail_file->bro.next = &add_file->bro;
                add_file->bro.pre = &tail_file->bro;
                add_file->bro.next = &temp_file->subdirs;
                temp_file->subdirs.pre = &add_file->bro;
               // add_process->sibling_pid = tail_process->pid;

                tail_file = add_file;
            }
            que_file.push(add_file);
        }

    }

    vmi_os.file_root = root;
}

bool ShowTree(VM_process &process,int b){
    VM_list_head *temp = NULL;
    VM_process *temp_process=NULL;

    for (int i = 0; i < b; ++i) {
        cout<<"---";
    }cout<<"|";
    cout<<process.pid<<" "<<process.comm;

    cout<<"\n";
    if (NULL!=process.children.next)
    {
        for (temp = process.children.next;temp!=&process.children ;temp = temp->next)
        {
            temp_process = VM_list_entry(temp,struct VM_process,sibling);
            ShowTree(*temp_process,b+1);
        }
    }
    return 1;
}

bool Deleate_Tree(VM_process &process,int b){
    VM_list_head *temp = NULL;
    VM_process *temp_process=NULL;
    if (NULL!=process.children.next)
    {
        for (temp = process.children.next;temp!=&process.children ;temp = temp->next)
        {
            temp_process = VM_list_entry(temp,struct VM_process,sibling);
            Deleate_Tree(*temp_process,b+1);
            delete temp_process;
        }
    }
    return 1;
}

void VM_find_process_asm(VM_process &process)
{
    string asm_code;
    for (unsigned long i = 0; i < 100; i+=1){
        asm_code+=process.code[i];
    }

    if (cs_open(CS_ARCH_X86, CS_MODE_64, &process.handle) != CS_ERR_OK)
        return ;
    int count = process.asm_count = cs_disasm(process.handle, (uint8_t*)asm_code.c_str(), asm_code.size()-1, 0x400829, 0,&process.insn);
    if (count > 0) {
        size_t j;
       // if (0 == strcmp("a.out",process.comm)){
            for (j = 0; j < count; j++) {

                printf("0x%"PRIx64":\t%s\t\t%s\n", process.insn[j].address, process.insn[j].mnemonic, process.insn[j].op_str);
            }
       // }

      //  cs_free(process.insn, count);
    } else
        printf("ERROR: Failed to disassemble given code!\n");
    //cs_close(&process.handle);
}

void VM_find_process_tree(VM_vmi &vmi_os,int cfd){
    vector<VM_process> vec_VMPro; //保存接收到的process
    VM_process temp_process;
    map<int,VM_process> map_pid_process; //将pid和进程做map
    map<int,VM_process>::iterator itea_process;

    while (1) {
        /*读取客户端发送数据*/
        RecvAll(cfd,(char *)&temp_process, sizeof(temp_process));
        if (-1 == temp_process.pid){
            break;
        }

        vec_VMPro.push_back(temp_process);
        map_pid_process.insert(pair<int,VM_process>(temp_process.pid,temp_process));
    }

    queue<VM_process*> que_process;
    VM_process* root = new VM_process(vec_VMPro[0]);
    root->children.next = NULL;
    root->children.pre = NULL;
    que_process.push(root);
    VM_process *tail_process = NULL;
    while(!que_process.empty()){

        VM_process* temp_process = que_process.front();
        cout<<"pid = "<<temp_process->pid<<"name = "<<temp_process->comm<<"\n";
        VM_find_process_asm(*temp_process);
        que_process.pop();



        for (int i = 0; i < temp_process->num_children_pid; ++i) {

            itea_process = map_pid_process.find( temp_process->children_pid[i] );

            VM_process* add_process = new VM_process(itea_process->second);
            add_process->children.pre = NULL;
            add_process->children.next = NULL;

            if (0 == i){
                temp_process->children.next = &add_process->sibling;
                temp_process->children.pre =  &add_process->sibling;
                add_process->sibling.pre = &temp_process->children;
                add_process->sibling.next = &temp_process->children;
                add_process->sibling_pid = process_tree_no_pid;

                tail_process = add_process;
            }else{

                tail_process->sibling.next = &add_process->sibling;
                add_process->sibling.pre = &tail_process->sibling;
                add_process->sibling.next = &temp_process->children;
                temp_process->children.pre = &add_process->sibling;
                add_process->sibling_pid = tail_process->pid;

                tail_process = add_process;
            }
            que_process.push(add_process);
        }

    }

    vmi_os.root = root;
}

void VM_find_process_list(VM_vmi &vmi_os,int cfd){

    VM_process temp_process;
    VM_process* add_process=NULL;
    VM_process* head_process=NULL;
    VM_process* tail_process=NULL;

    while (1) {
        /*读取客户端发送数据*/
        RecvAll(cfd,(char *)&temp_process, sizeof(VM_process));
        if (-1 == temp_process.pid){
            break;
        }
        //cout<<temp_process.pid<<" "<<temp_process.comm<<"\n";
        add_process = new VM_process(temp_process);
        if (NULL == head_process) {
            head_process = add_process;
            head_process->tasks.next  = &head_process->tasks;
            head_process->tasks.pre = &head_process->tasks;

            tail_process = head_process;
        }else {
            add_process->tasks.pre = &tail_process->tasks;
            add_process->tasks.next = tail_process->tasks.next;
            tail_process->tasks.next->pre = &add_process->tasks;
            tail_process->tasks.next = &add_process->tasks;

            tail_process = add_process;
        }
    }
    vmi_os.process = *head_process;
}

void VM_find_module_list(VM_vmi &vmi_os,int cfd){

    VM_module temp_module;
    VM_module* add_module=NULL;
    VM_module* head_module=NULL;
    VM_module* tail_module=NULL;

    while (1) {
        /*读取客户端发送数据*/
        RecvAll(cfd,(char *)&temp_module, sizeof(VM_module));
        if (0 == strcmp(temp_module.name,"last_module")){
            break;
        }
        cout<<temp_module.name<<"\n";
        add_module = new VM_module(temp_module);
        if (NULL == head_module) {
            head_module = add_module;
            head_module->list.next  = &head_module->list;
            head_module->list.pre = &head_module->list;

            tail_module = head_module;
        }else {
            add_module->list.pre = &tail_module->list;
            add_module->list.next = tail_module->list.next;
            tail_module->list.next->pre = &add_module->list;
            tail_module->list.next = &add_module->list;

            tail_module = add_module;
        }
    }
    vmi_os.module = *head_module;
}

void print_module_list(VM_vmi &vmi){
    vector<VM_module> vec_VMModu;
    cout<<"in print_process_list\n";
    VM_list_head *post = &vmi.module.list;
    do{
        VM_module *temp_module = VM_list_entry(post,struct VM_module,list);
        vec_VMModu.push_back(*temp_module);
        cout<<temp_module->name<<" "<<"\n";
        post = post->next;
    }while(post != vmi.module.list.next->pre);

}

void print_process_list(VM_vmi &vmi){
    vector<VM_process> vec_VMPro;
    cout<<"in print_process_list\n";
    VM_list_head *post = &vmi.process.tasks;

    do{
        VM_process *temp_process = VM_list_entry(post,struct VM_process,tasks);
        vec_VMPro.push_back(*temp_process);
        cout<<temp_process->pid<<" "<<temp_process->comm<<" "<<post<<"\n";
        post = post->next;
    }while(post != vmi.process.tasks.next->pre);

}

void show_file_tree(VM_file* fileNode,int deep,int sfd)
{
    VM_list_head* p;

    for(p=fileNode->subdirs.next;p!=&(fileNode->subdirs);p=p->next){
        VM_file* tempNode=VM_list_entry(p,struct VM_file,bro);

        for (int i = 0; i < deep; ++i) {
            cout<<"---";
        }cout<<"|";
        cout<<tempNode->id<<" "<<tempNode->name<<"\n";
        if(tempNode->subdirs.next)
        {
            show_file_tree(tempNode,deep+1,sfd);
        }
    }

}

void VM_get_process_code(VM_vmi &vmi_os,int cfd) {

    RecvAll(cfd,(char *)vmi_os.code, sizeof(vmi_os.code));
    for (int i = 0; i < 1000; ++i) {
        printf("%lx ",vmi_os.code[i]);
    }
}

void* do_work(void* args){
    struct connect cn = *((struct connect*) args);
    int	efd = cn.efd;
    int listenfd = cn.listenfd;
    struct epoll_event tep = cn.tep;

    struct epoll_event ep[OPEN_MAX];
    socklen_t clilen;
    struct sockaddr_in cliaddr;
    char  str[INET_ADDRSTRLEN];
    ssize_t res;
    int num = 0;
    for ( ; ; ) {
        /*epoll为server阻塞监听事件, ep为struct epoll_event类型数组, OPEN_MAX为数组容量, -1表永久阻塞*/
        int nready = epoll_wait(efd, ep, OPEN_MAX, -1);
        if (nready == -1)
            perr_exit("epoll_wait error");

        for (int i = 0; i < nready; i++) {
            if (!(ep[i].events & EPOLLIN))      //如果不是"读"事件, 继续循环
                continue;

            if (ep[i].data.fd == listenfd) {    //判断满足事件的fd是不是lfd
                clilen = sizeof(cliaddr);
                int connfd = Accept(listenfd, (struct sockaddr *)&cliaddr, &clilen);    //接受链接

                printf("received from %s at PORT %d\n",
                       inet_ntop(AF_INET, &cliaddr.sin_addr, str, sizeof(str)),
                       ntohs(cliaddr.sin_port));
                //获取客户机的ip
                VM_info vm_info;
                vm_info.ip = inet_ntop(AF_INET, &cliaddr.sin_addr, str, sizeof(str));
                vm_info.connfd = connfd;
                vec_VM_info.push_back(vm_info);


                printf("cfd %d---client %d\n", connfd, ++num);

                tep.events = EPOLLIN; tep.data.fd = connfd;
                res = epoll_ctl(efd, EPOLL_CTL_ADD, connfd, &tep);
                if (res == -1)
                    perr_exit("epoll_ctl error");
                char buf[128] = "get_VM_name";
                Writen(connfd, buf, 128);

            } else {                                //不是lfd,
                int sockfd = ep[i].data.fd;
                char buf[3];
                printf("size = %d\n",sizeof(buf));
                int n = Read(sockfd, buf, 3);

                if (n == 0) {                       //读到0,说明客户端关闭链接
                    res = epoll_ctl(efd, EPOLL_CTL_DEL, sockfd, NULL);  //将该文件描述符从红黑树摘除
                    if (res == -1)
                        perr_exit("epoll_ctl error");
                    Close(sockfd);                  //关闭与该客户端的链接
                    printf("client[%d] closed connection\n", sockfd);
                    for(int i = 0 ;i < vec_VM_info.size(); i++){
                        if (vec_VM_info[i].connfd == sockfd){
                            vec_VM_info.erase(vec_VM_info.begin()+i);
                            break;
                        }
                    }

                } else if (n < 0) {                 //出错
                    perror("read n < 0 error: ");
                    res = epoll_ctl(efd, EPOLL_CTL_DEL, sockfd, NULL);
                    Close(sockfd);

                } else {                            //实际读到了字节数
                    if (0==strcmp("11",buf)){cout<<"a\n";
                        char name[128];
                        int n = Read(sockfd, name, 128);
                        if (n == 0) {                       //读到0,说明客户端关闭链接
                            res = epoll_ctl(efd, EPOLL_CTL_DEL, sockfd, NULL);  //将该文件描述符从红黑树摘除
                            if (res == -1)
                                perr_exit("epoll_ctl error");
                            Close(sockfd);                  //关闭与该客户端的链接
                            printf("client[%d] closed connection\n", sockfd);

                        } else if (n < 0) {                 //出错
                            perror("read n < 0 error: ");
                            res = epoll_ctl(efd, EPOLL_CTL_DEL, sockfd, NULL);
                            Close(sockfd);

                        } else {
                            //将名字加入info
                            VM_info vm_info;
                            vm_info.connfd = sockfd;
                            for (int j = 0; j < vec_VM_info.size() ; ++j) {
                                if (vec_VM_info[i].connfd == sockfd) {
                                    strcpy( vec_VM_info[i].name, name );
                                }
                            }
                        }

                    } else if (0==strcmp("12",buf)){cout<<"aa\n";
                        char name[128];
                        VM_find_process_tree(vmi_os,sockfd);
//                        int n = Read(sockfd, name, 128);
//                        if (n == 0) {                       //读到0,说明客户端关闭链接
//                            res = epoll_ctl(efd, EPOLL_CTL_DEL, sockfd, NULL);  //将该文件描述符从红黑树摘除
//                            if (res == -1)
//                                perr_exit("epoll_ctl error");
//                            Close(sockfd);                  //关闭与该客户端的链接
//                            printf("client[%d] closed connection\n", sockfd);
//
//                        } else if (n < 0) {                 //出错
//                            perror("read n < 0 error: ");
//                            res = epoll_ctl(efd, EPOLL_CTL_DEL, sockfd, NULL);
//                            Close(sockfd);
//
//                        } else {                            //实际读到了字节数
//                            cout<<"get infomation "<<name;
//                        }
                    } else if ( 0 == strcmp("13",buf)){
                        VM_find_module_list(vmi_os,sockfd);
                        print_module_list(vmi_os);

                    } else if ( 0 == strcmp("14",buf)){
                        VM_find_file_tree(vmi_os,sockfd);
                        show_file_tree(vmi_os.file_root,0,sockfd);

                    } else if ( 0 == strcmp("15",buf)){
                        VM_get_process_code(vmi_os,sockfd);
                    } else if (0 == strcmp("16",buf)){
                        VM_get_ps_file(sockfd);
                        cout<<"VM_get_ps_file succsee\n";
                    } else if (0 == strcmp("17",buf)){
                        VM_get_lsmod_file(sockfd);
                        cout<<"VM_get_lsmod_file succsee\n";
                    } else if (0 == strcmp("18",buf)){
                        VM_get_tree_file(sockfd);
                        cout<<"VM_get_tree_file succsee\n";
                    }
                }
            }
        }
    }
}

int start_socket(){
    int i, listenfd;
    ssize_t nready, efd, res;

    struct sockaddr_in  servaddr;
    struct epoll_event tep, ep[OPEN_MAX];       //tep: epoll_ctl参数  ep[] : epoll_wait参数

    listenfd = Socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));      //端口复用

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);

    Bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

    Listen(listenfd, 20);

    efd = epoll_create(OPEN_MAX);               //创建epoll模型, efd指向红黑树根节点
    if (efd == -1)
        perr_exit("epoll_create error");

    tep.events = EPOLLIN; tep.data.fd = listenfd;           //指定lfd的监听时间为"读"
    res = epoll_ctl(efd, EPOLL_CTL_ADD, listenfd, &tep);    //将lfd及对应的结构体设置到树上,efd可找到该树
    if (res == -1)
        perr_exit("epoll_ctl error");

    struct connect cn;
    cn.efd = efd;
    cn.listenfd = listenfd;
    cn.tep = tep;
    pthread_t tid;
    pthread_create(&tid, NULL, do_work, (void*)&cn);

    return listenfd;
}

int test_socket(){
    int sfd, cfd;
    int len, i;
    char buf[BUFSIZ], clie_IP[BUFSIZ];

    struct sockaddr_in serv_addr, clie_addr;
    socklen_t clie_addr_len;

    /*创建一个socket 指定IPv4协议族 TCP协议*/
    sfd = socket(AF_INET, SOCK_STREAM, 0);

    /*初始化一个地址结构 man 7 ip 查看对应信息*/
    bzero(&serv_addr, sizeof(serv_addr));           //将整个结构体清零
    serv_addr.sin_family = AF_INET;                 //选择协议族为IPv4
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);  //监听本地所有IP地址
    serv_addr.sin_port = htons(SERV_PORT);          //绑定端口号

    /*绑定服务器地址结构*/
    bind(sfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    /*设定链接上限,注意此处不阻塞*/
    listen(sfd, 64);                                //同一时刻允许向服务器发起链接请求的数量

    printf("wait for client connect ...\n");

    /*获取客户端地址结构大小*/
    clie_addr_len = sizeof(clie_addr_len);
    /*参数1是sfd; 参2传出参数, 参3传入传入参数, 全部是client端的参数*/
    cfd = accept(sfd, (struct sockaddr *)&clie_addr, &clie_addr_len);           /*监听客户端链接, 会阻塞*/

    printf("client IP:%s\tport:%d\n",
           inet_ntop(AF_INET, &clie_addr.sin_addr.s_addr, clie_IP, sizeof(clie_IP)),
           ntohs(clie_addr.sin_port));
return cfd;

//    /*读取客户端发送数据*/
//
//    char get_buf[128] = "get_module_list";
//    /*处理完数据回写给客户端*/
//    write(cfd, get_buf, 128);
//    VM_vmi vmi;
//    VM_find_module_list(vmi,cfd);
//    print_module_list(vmi);
//
//    /*关闭链接*/
//    close(sfd);
//    close(cfd);
}

//int main(){
//
//
//    return 0;
//}