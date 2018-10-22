#include <sys/stat.h>
#include <syslog.h>
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
 * 守护进程
 * 注意: 1. 守护进程没有控制终端
         2. 不可以使用CTRL + C 终止守护进程

 * 编译 需要连接pthread库
 * 使用select函数
 * gcc -o bin/echo_tcp_server_daemon -Iinclude obj/vector_fd.o src/echo_tcp_server_daemon.c -lpthread
 *
 * 多线程实现服务器并发的例子
 * 测试：
 * 开启服务器
 * 　1 bin/echo_tcp_server_daemon 8888
       
       查看进程是否启动
       ps -ef | grep echo_tcp
       杀死守护进程
       kill -9 3039  //3039为上一步查到的进程id

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
 *   查看系统日志    cat /var/log/syslog   或者 查看后20行   tail -20 /var/log/syslog
 *                                              默认查看10行 tail /var/log/syslog
 */


VectorFD *vfd;
//socket描述符
int sockfd;
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
        syslog(LOG_DEBUG, "client closed");
        //从动态数组中删除对应的fd  同时关闭对应的socket
        remove_fd(vfd, fd);
        close(fd);
    }else if(size >0){
        syslog(LOG_DEBUG,"%s\n", buff);
        //写时  如果对方已经关闭socket
        if(write(fd,buff,size) < 0){  //写出错,会返回-1
              //读端关闭  再写时会发出EPIPE错误码信号
              if(errno == EPIPE){ //客户端已经挂掉
                  syslog(LOG_DEBUG, "write:%s\n", strerror(errno));
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
    syslog(LOG_DEBUG, "%s(%d) connected!\n", ip, port);
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

    //守护进程编程的5个步骤
    //1 创建屏蔽字0
    umask(0);
    //2 调用fork函数创建子进程,然后父进程退出
    pid_t pid = fork();
    if(pid > 0) exit(0);
    //3 调用setsid函数创建一个新会话
    setsid();
    //4 将当前工作目录更改为根目录
    chdir("/");
    //5 关闭不需要的文件描述符
    //关闭的只是当前进程的拷贝,不影响其他进程使用这3个描述符
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    //打开系统日志服务的一个连接
    openlog(argv[0],LOG_PID, LOG_SYSLOG);



    //步骤1:创建socket
    //注:socket创建再内核中,是一个结构体
    //AF_INET:IPV4
    //SOCK_STREAM:tcp协议
    sockfd = socket( AF_INET, SOCK_STREAM, 0);
    if(sockfd<0){
        //守护进程没有控制台,不能将错误信息输出到控制台,
        //此处将错误信息输出到系统文件 
        syslog(LOG_DEBUG, "socket:%s\n", strerror(errno));
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
        //将日志信息写入到系统日志文件中 /var/log/syslog
        syslog(LOG_DEBUG, "bind:%s\n", strerror(errno));
        exit(1);
    }

    /*
     * 步骤3:调用listen函数启动监听
     * 通知系统去接收来自客户端的连接请求
     * 将客户端的连接请求放置到对应的队列中
     * 第二个参数10来制定客户端连接队列的长度
     */
    if(listen(sockfd,10)<0){
        syslog(LOG_DEBUG, "listen:%s\n", strerror(errno));
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
        syslog(LOG_DEBUG, "pthread_create:%s\n", strerror(errno));
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
            syslog(LOG_DEBUG, "accept:%s\n", strerror(errno));
            continue;
        }
        
        out_addr(&clientaddr);
        //将返回的新的socket描述符加入到动态数组中
        add_fd(vfd, fd);
        
    }


    return 0;
}
