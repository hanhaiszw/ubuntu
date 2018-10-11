#include "msg.h"
#include <netdb.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <memory.h>
#include <signal.h>
#include <time.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/wait.h>  //wait(0)
#include <pthread.h>

/*
 * 编译 需要连接pthread库
 * gcc -o bin/echo_tcp_server_th -Iinclude obj/msg.o src/echo_tcp_server_th.c -lpthread
 *
 * 多线程实现服务器并发的例子
 * 测试：
 * 开启服务器
 * 　1 bin/echo_tcp_server_th 8888
 *   2 在另一个terminal中输入
 *     bin/echo_tcp_client 127.0.0.1 8888
 *     输出 test1  服务器返回 test1
 *     按Ctrl C后客户端终止
 *
 *   3 在另一个terminal中输入
 *     bin/echo_tcp_client 127.0.0.1 8888
 *     输出 test2  服务器返回 test2
 *  
 *   ... 可以打开多个客户端
 *  
 */
//socket描述符
int sockfd;
void sig_handler(int signo){
    if(signo==SIGINT){
        printf("server close\n");

        //步骤６:关闭socket
        close(sockfd);
        exit(1);
    }

}
//处理与客户端的IO交互
void do_service(int fd){
    //和客户端进行读写操作 双向通信
    char buff[512];
    while(1){
        memset(buff, 0, sizeof(buff));
        // printf("start read and write...\n");
        size_t size;
        if((size=read_msg(fd,buff, sizeof(buff)))<0){
            perror("protocal error");
            break;
        }else if(size==0){
            //写端关闭  读端读size为0
            break;  //一端关闭或者已经断开
        }else{
            printf("%s\n",buff);
            //写时  如果对方已经关闭socket
            if(write_msg(fd,buff,sizeof(buff))<0){
                
                //读端关闭  再写时会发出EPIPE错误码信号
                if(errno == EPIPE){ //客户端已经挂掉
                    break;
                }
                perror("protocal error");
            }
        }

    }
}

//获取客户端信息
void out_fd(int fd){
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    //从fd中获得连接的客户端相关信息
    if(getpeername(fd,(struct sockaddr*)&addr,&len)<0){
        perror("getpeername error");
        return;
    }
    char ip[16];
    memset(ip, 0, sizeof(ip));
    int port = ntohs(addr.sin_port);
    //网络字节序转化为主机字节序  转化为点分十进制ip地址
    inet_ntop(AF_INET,&addr.sin_addr.s_addr, ip, sizeof(ip));
    printf("%16s(%5d) closed!\n",ip,port);
}



void* th_fn(void *arg){
    int fd=*((int*)arg);
    do_service(fd);
    out_fd(fd);
    close(fd);
    
    return (void*)0;
}

//需要传入监听的端口号 比如8888
int main(int argc,char* argv[]){
    if(argc<2){
        printf("usage: %s #port\n",argv[0]);
        exit(1);
    }
    //信号处理函数  ctrl c结束服务器端运行
    //Ctrl C 产生SIGINT信号
    if(signal(SIGINT,sig_handler)==SIG_ERR){
        perror("signal sigint error");
        exit(1);
    }

    //步骤1:创建socket
    //注:socket创建再内核中,是一个结构体
    //AF_INET:IPV4
    //SOCK_STREAM:tcp协议
    sockfd = socket( AF_INET, SOCK_STREAM, 0);
    if(sockfd<0){
        perror("socket error");
        exit(1);
    }
    /*
    步骤2:调用bind函数将socket和地址(包括IP,port)进行绑定
    */
    //添加专用网络地址  再转化为通用网络地址
    struct sockaddr_in serveraddr;
    memset(&serveraddr,0,sizeof(serveraddr));
    //往地址中填入ip,port,internet地址族类型
    serveraddr.sin_family=AF_INET;//ipv4  主机字节序
    serveraddr.sin_port=htons(atoi(argv[1])); //填入端口   要求是网络字节序  端口号是通过控制台传入的
    serveraddr.sin_addr.s_addr = INADDR_ANY;  //要求是网络字节序  特殊的bind  响应所有网卡地址的请求 不是特定的网卡
    //需要把专用地址强转为通用地址
    if(bind(sockfd, (struct sockaddr*)&serveraddr,sizeof(serveraddr))<0){
        perror("bind error");
        exit(1);
    }

    /*
     * 步骤3:调用listen函数启动监听
     * 通知系统去接收来自客户端的连接请求
     * 将客户端的连接请求放置到对应的队列中
     * 第二个参数10来制定客户端连接队列的长度
     */
    if(listen(sockfd,10)<0){
        perror("listen error");
        exit(1);
    }

    /*
     * 步骤4: 调用accept函数从队列中获得一个客户端的请求连接,
     * 并返回新的socket描述符
     * 若没有客户端连接,则accept则会阻塞,等待连接
     */
    /*
    struct sockaddr_in clientaddr;
    //一定要使用socklen_t类型
    socklen_t clientaddr_len = sizeof(clientaddr);
    */
    //设置线程分离属性
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);

    while(1){
        //返回的描述符是针对客户端的   与sockfd是不同的  
        //而且和拿到的clientaddr也是不一样的  
        //可以使用这个描述符与客户端进行全双工的通信
        //第二个参数 会填入客户端的信息
        //int fd=accept(sockfd,(struct sockaddr*)&clientaddr,&clientaddr_len);
        //也可以通过返回的fd来获取客户端socket信息
        int fd=accept(sockfd, NULL, NULL);
        if(fd<0){
            perror("accept error");
            continue;
        }

        //步骤5: 调用IO函数(read/write)和连接的客户端进行全双工的双向通信
        //在此启动子线程来执行io
        pthread_t th;
        int err;
        //以分离状态创建子线程   th_fn为线程函数
        if((err=pthread_create(&th,&attr,th_fn,(void*)&fd))!=0){
            perror("pthread create error");
        }
        //销毁线程属性  为什么?
        pthread_attr_destroy(&attr);
        //主控线程继续调用accept函数   分离的子线程用来执行IO
    }


    return 0;
}
