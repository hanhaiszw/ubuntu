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
/*
 * 测试：
 * 开启服务器
 * 　1 bin/time_tcp_server 8888
 * 一
 *   2 打开另一个terminal输入 
 *     telnet 127.0.0.1 8888
 *     连接已开启的服务器
 *     telnet是在应用层  tcp是在传输层
 * 二
 *   2 浏览器中输入
 *     http://127.0.0.1:8888
 *     也可以连接到服务器
 *     http协议位于应用层
 * 说明上层的客户端是可以与处于传输层的TCP通信的
 * 三 
 *   2 在另一个terminal中输入
 *     bin/time_tcp_client 127.0.0.1 8888
 *   现在服务器端和客户端都在传输层进行通信
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

//输出连接上来的客户端相关信息
void out_addr(struct sockaddr_in *clientaddr){
    //网络字节序转化为主机字节序
    int port=ntohs(clientaddr->sin_port);
    char ip[16];
    memset(ip,0,sizeof(ip));
    //将ip地址从网络字节序转换为点分十进制
    inet_ntop(AF_INET,&clientaddr->sin_addr.s_addr,ip,sizeof(ip));
    printf("client: %s(%d) connected\n",ip,port);

}

//给客户端返回当前时间
void do_service(int fd){
    //获取当前时间
    long t=time(0);
    char* s=ctime(&t);
    size_t size=strlen(s)* sizeof(char);
    //将服务器获得的系统时间写回到客户端
    if(write(fd,s,size)!=size){
        perror("write error");
    }

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
    sockfd=socket( AF_INET, SOCK_STREAM, 0);
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
    struct sockaddr_in clientaddr;
    //一定要使用socklen_t类型
    socklen_t clientaddr_len = sizeof(clientaddr);
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

        //步骤5: 调用IO函数(read/write)和连接的客户端进行全双工的双向通信
        out_addr(&clientaddr);//输出客户端地址信息
        do_service(fd);       //给客户端返回信息
        //步骤６:关闭socket
        close(fd);
    }


    return 0;
}
