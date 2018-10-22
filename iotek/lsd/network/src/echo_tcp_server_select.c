#include <fcntl.h>
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
#include "vector_fd.h"
/*
 * IO多路复用
 * 编译 需要连接pthread库
 * 使用select函数
 * gcc -o bin/echo_tcp_server_select -Iinclude obj/vector_fd.o src/echo_tcp_server_select.c -lpthread
 *
 * 多线程实现服务器并发的例子
 * 测试：
 * 开启服务器
 * 　1 bin/echo_tcp_server_select 8888
 *   2 在另一个terminal中输入
 *     bin/echo_tcp_client2 127.0.0.1 8888
 *     输出 test1  服务器返回 test1
 *     按Ctrl C后客户端终止
 *
 *   3 在另一个terminal中输入
 *     bin/echo_tcp_client2 127.0.0.1 8888
 *     输出 test2  服务器返回 test2
 *  
 *   ... 可以打开多个客户端
 *  
 */


VectorFD *vfd;
//socket描述符
int sockfd;
void sig_handler(int signo){
    if(signo==SIGINT){
        printf("server close\n");

        //步骤６:关闭socket
        close(sockfd);
        //销毁动态数组
        destroy_vector_fd(vfd);
        exit(1);
    }

}
//处理与客户端的IO交互  非阻塞的方式
void do_service(int fd){
    //和客户端进行读写操作 双向通信
    char buff[512];

    memset(buff, 0, sizeof(buff));
    //因为采用非阻塞的方式,若读不到数据直接返回
    //直接服务与下一个客户端
    //因此不需要判断size小于0的情况
    size_t size = read(fd,buff, sizeof(buff));
    if(size == 0){
        //写端关闭  读端读size为0  客户端已经关闭
        //标准io带有缓存机制  因此最好使用内核提供的api
        //不带缓存 直接输出
        printf("client closed!\n");
        //从动态数组中删除对应的fd  同时关闭对应的socket
        remove_fd(vfd, fd);
        close(fd);
    }else if(size >0){
        printf("%s\n",buff);
        //写时  如果对方已经关闭socket
        if(write(fd,buff,size) < 0){  //写出错,会返回-1
              //读端关闭  再写时会发出EPIPE错误码信号
              if(errno == EPIPE){ //客户端已经挂掉
                  perror("write error");

                  //从动态数组中删除对应的fd  同时关闭对应的socket
                  remove_fd(vfd, fd);
                  close(fd);
              }
          }
     }

}

void out_addr(struct sockaddr_in *clientaddr){
    char ip[16];
    memset(ip, 0, sizeof(ip));
    int port = ntohs(clientaddr->sin_port);
    inet_ntop(AF_INET, &clientaddr->sin_addr.s_addr, ip, sizeof(ip));
    printf("%s(%d)  connected!\n", ip, port);
}

//遍历出动态数组中所有的描述符 
//并加入到描述符集set中
//同时此函数返回最大的描述符
int add_set(fd_set *set){
    FD_ZERO(set);  //清空描述符集
    int max_fd = vfd->fd[0];
    int i = 0;
    for(; i < vfd->counter; i++){
        int fd = get_fd(vfd, i);
        if(fd > max_fd) max_fd = fd;
        FD_SET(fd, set);  //将fd加入到描述符集中
        
    }
    return max_fd;
}
void* th_fn(void *arg){
    
    struct timeval t;
    t.tv_sec = 2;//两秒
    t.tv_usec = 0;
    int n = 0;
    //获得描述符的最大值
    fd_set set;  //描述符集
    int maxfd = add_set(&set);
    //委托内核检查描述符是否准备好   
    //若有则返回准备好的描述符数
    //超时则返回0
    //第一个参数为描述符集中描述符的范围
    //若是返回值<0,则出现异常 退出
    while((n=select(maxfd+1, &set, NULL, NULL, &t)) >= 0){
        if(n>0){
            //检测下哪些描述符可以使用了
            int i=0;
            for(;i<vfd->counter;i++){
                int fd = get_fd(vfd, i);
                //测试描述符给定位是否发生变化
                if(FD_ISSET(fd, &set))
                    do_service(fd);
            }
        }
        //重新设置时间和清空描述符集
        t.tv_sec =2;
        t.tv_usec = 0;
        //重新遍历动态数组中最新的描述符放置到描述符集中
        maxfd = add_set(&set);

    }
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
   
   

    //创建放置套接字描述符fd的动态数组
    vfd = create_vector_fd();

    //设置线程分离属性
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
    //启动子线程
    pthread_t th;
    int err;
    if((err = pthread_create(&th, &attr,th_fn, (void*)0)) != 0){
        perror("pthread create error");
        exit(1);
    }
    //销毁线程属性
    pthread_attr_destroy(&attr);

    //放置连接的子线程信息
    struct sockaddr_in clientaddr;
    //一定要使用socklen_t类型
    socklen_t clientaddr_len = sizeof(clientaddr);
    //主控线程获得客户端的连接,将新的socket描述符放置到动态数组中
    //启动的子线程负责遍历动态数据组中socket描述符,
    // a)调用select函数委托内核去检查传入的描述符是否准备好
    // b)利用FD_ISSET来找出准备好的那些描述符,并和对应的客户端进行双向通信(采用非阻塞的读写)
    // 使用select函数后,内核会自动把socket设置为非阻塞读写的
    while(1){
        //返回的描述符是针对客户端的   与sockfd是不同的  
        //而且和拿到的clientaddr也是不一样的  
        //可以使用这个描述符与客户端进行全双工的通信
        //第二个参数 会填入客户端的信息
        int fd=accept(sockfd,(struct sockaddr*)&clientaddr,&clientaddr_len);
        if(fd<0){
            perror("accept error");
            continue;
        }
        
        out_addr(&clientaddr);
        //将返回的新的socket描述符加入到动态数组中
        add_fd(vfd, fd);
        
    }


    return 0;
}
