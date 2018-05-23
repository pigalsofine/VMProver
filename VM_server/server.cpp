//#include <iostream>
//#include <unistd.h>
//#include <sys/types.h>
//#include <sys/socket.h>
//#include <strings.h>
//#include <string.h>
//#include <ctype.h>
//#include <arpa/inet.h>
//#include <queue>
//#include <map>
//#include <zconf.h>
//#include ""
//#include "definition.h"
//using namespace std;
//#define SERV_PORT 9528
//
//bool RecvAll(int &sock, char*buffer, int size)
//{
//    while (size>0)//剩余部分大于0
//    {
//        int RecvSize= recv(sock, buffer, size, 0);
//        if(-1==RecvSize)
//            return false;
//        size = size - RecvSize;
//        buffer+=RecvSize;
//    }
//    return true;
//}
//
//
//bool ShowTree(VM_process &process,int b){
//    VM_list_head *temp = NULL;
//    VM_process *temp_process=NULL;
//
//    for (int i = 0; i < b; ++i) {
//        cout<<"---";
//    }cout<<"|";
//    cout<<process.pid<<" "<<process.comm;
//
//    cout<<"\n";
//    if (NULL!=process.children.next)
//    {
//        for (temp = process.children.next;temp!=&process.children ;temp = temp->next)
//        {
//            temp_process = VM_list_entry(temp,struct VM_process,sibling);
//            ShowTree(*temp_process,b+1);
//        }
//    }
//    return 1;
//}
//
//
//void VM_find_process_tree(VM_vmi &vmi_os,int cfd){
//    vector<VM_process> vec_VMPro; //保存接收到的process
//    VM_process temp_process;
//    map<int,VM_process> map_pid_process; //将pid和进程做map
//    map<int,VM_process>::iterator itea_process;
//
//    while (1) {
//        /*读取客户端发送数据*/
//        RecvAll(cfd,(char *)&temp_process, sizeof(temp_process));
//        if (-1 == temp_process.pid){
//            break;
//        }
//
//        vec_VMPro.push_back(temp_process);
//        map_pid_process.insert(pair<int,VM_process>(temp_process.pid,temp_process));
//    }
//
//    queue<VM_process*> que_process;
//    VM_process* root = new VM_process(vec_VMPro[0]);
//    root->children.next = NULL;
//    root->children.pre = NULL;
//    que_process.push(root);
//    VM_process *tail_process = NULL;
//    while(!que_process.empty()){
//
//        VM_process* temp_process = que_process.front();
//        que_process.pop();
//
//        for (int i = 0; i < temp_process->num_children_pid; ++i) {
//
//            itea_process = map_pid_process.find( temp_process->children_pid[i] );
//
//            VM_process* add_process = new VM_process(itea_process->second);
//            add_process->children.pre = NULL;
//            add_process->children.next = NULL;
//
//            if (0 == i){
//                temp_process->children.next = &add_process->sibling;
//                temp_process->children.pre =  &add_process->sibling;
//                add_process->sibling.pre = &temp_process->children;
//                add_process->sibling.next = &temp_process->children;
//                add_process->sibling_pid = process_tree_no_pid;
//
//                tail_process = add_process;
//            }else{
//
//                tail_process->sibling.next = &add_process->sibling;
//                add_process->sibling.pre = &tail_process->sibling;
//                add_process->sibling.next = &temp_process->children;
//                temp_process->children.pre = &add_process->sibling;
//                add_process->sibling_pid = tail_process->pid;
//
//                tail_process = add_process;
//            }
//            que_process.push(add_process);
//        }
//
//    }
//
//    vmi_os.root = root;
//}
//
//void VM_find_process_list(VM_vmi &vmi_os,int cfd){
//
//    VM_process temp_process;
//    VM_process* add_process=NULL;
//    VM_process* head_process=NULL;
//    VM_process* tail_process=NULL;
//
//    while (1) {
//        /*读取客户端发送数据*/
//        RecvAll(cfd,(char *)&temp_process, sizeof(VM_process));
//        if (-1 == temp_process.pid){
//            break;
//        }
//        //cout<<temp_process.pid<<" "<<temp_process.comm<<"\n";
//        add_process = new VM_process(temp_process);
//        if (NULL == head_process) {
//            head_process = add_process;
//            head_process->tasks.next  = &head_process->tasks;
//            head_process->tasks.pre = &head_process->tasks;
//
//            tail_process = head_process;
//        }else {
//            add_process->tasks.pre = &tail_process->tasks;
//            add_process->tasks.next = tail_process->tasks.next;
//            tail_process->tasks.next->pre = &add_process->tasks;
//            tail_process->tasks.next = &add_process->tasks;
//
//            tail_process = add_process;
//        }
//    }
//    vmi_os.process = *head_process;
//}
//
//void print_process_list(VM_vmi &vmi){
//    vector<VM_process> vec_VMPro;
//    cout<<"in print_process_list\n";
//    VM_list_head *post = &vmi.process.tasks;
//
//    do{
//        VM_process *temp_process = VM_list_entry(post,struct VM_process,tasks);
//        vec_VMPro.push_back(*temp_process);
//        cout<<temp_process->pid<<" "<<temp_process->comm<<" "<<post<<"\n";
//        post = post->next;
//    }while(post != vmi.process.tasks.next->pre);
//
//}
//
//int start_socket(){
//    int sfd, cfd;
//    int len, i;
//    char buf[BUFSIZ], clie_IP[BUFSIZ];
//
//    struct sockaddr_in serv_addr, clie_addr;
//    socklen_t clie_addr_len;
//
//    /*创建一个socket 指定IPv4协议族 TCP协议*/
//    sfd = socket(AF_INET, SOCK_STREAM, 0);
//
//    /*初始化一个地址结构 man 7 ip 查看对应信息*/
//    bzero(&serv_addr, sizeof(serv_addr));           //将整个结构体清零
//    serv_addr.sin_family = AF_INET;                 //选择协议族为IPv4
//    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);  //监听本地所有IP地址
//    serv_addr.sin_port = htons(SERV_PORT);          //绑定端口号
//
//    /*绑定服务器地址结构*/
//    bind(sfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
//
//    /*设定链接上限,注意此处不阻塞*/
//    listen(sfd, 64);                                //同一时刻允许向服务器发起链接请求的数量
//
//    cout<<"wait for client connect ...\n";
//
//    /*获取客户端地址结构大小*/
//    clie_addr_len = sizeof(clie_addr_len);
//    /*参数1是sfd; 参2传出参数, 参3传入传入参数, 全部是client端的参数*/
//    cfd = accept(sfd, (struct sockaddr *)&clie_addr, &clie_addr_len);           /*监听客户端链接, 会阻塞*/
//
//
//    return cfd;
//}
//
////
////int main(void)
////{
////    VM_vmi vmi;
////    int sfd = start_socket();
////    int a;
////    while(cin>>a){
////        if (1 == a){
////            char buffer[128] = "get_process_list";
////            int SendSize= send(sfd, buffer, 128, 0);
////            VM_find_process_list(vmi,sfd);
////            print_process_list(vmi);
////        } else if (2 == a){
////            char buffer2[128] = "get_process_tree";
////            send(sfd, buffer2, 128, 0);
////            VM_find_process_tree(vmi,sfd);
////            ShowTree(*vmi.root,0);
////        }
////    }
////
////
////
////
////
////
////   // close(sfd);
////
////    return 0;
////}