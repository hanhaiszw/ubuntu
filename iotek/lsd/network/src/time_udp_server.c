#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <stdio.h>
#include <arpa/inet.h>

/*
  测试:
  1.开启服务器   bin/time_udp_server 8888
  2.另一个terminal中启动客户端连接
    bin/time_udp_client 127.0.0.1 8888

*/
int sockfd;

void sig_handler(int signo){

    if(signo == SIGINT){
        printf("server close\n");
        close(sockfd);
        exit(1);
    }
    
}

void out_addr(struct sockaddr_in *clientaddr){
    char ip[16];
    int port;
    memset(ip, 0, sizeof(ip));
    inet_ntop(AF_INET, &clientaddr->sin_addr.s_addr, ip, sizeof(ip));
    port = ntohs(clientaddr -> sin_port);
    printf("client: %s(%d)\n",ip, port);
}

void do_service(){
    struct sockaddr_in clientaddr;
    socklen_t len = sizeof(clientaddr);
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    //接受客户端的数据报文
    //如果要考虑并发,在此处,如果有客户端报文发送过来,那就启动一个子线程来专业执行与此客户端的交互
    if(recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&clientaddr, &len)<0){
        perror("recvfrom error");
    }else{
        //输出客户端的信息
        out_addr(&clientaddr);
        printf("client send info: %s\n",buffer);

        //向客户端发送数据报文
        long int t = time(0);
        char *ptr = ctime(&t);
        size_t size = strlen(ptr) * sizeof(char);
        //发送报文信息给指定的客户端
        if(sendto(sockfd, ptr, size, 0, (struct sockaddr*)&clientaddr, len) < 0){
            perror("sendto error");
        }
    }
}

//一个整形值  和 一个指针数据
int main(int argc, char* argv[]){
    if(argc<2){
        printf("usage: %s port\n", argv[0]);
        exit(1);
    }

    if(signal(SIGINT, sig_handler) == SIG_ERR){
        perror("signal sigint error");
    }
    /*
        步骤一: 创建socket  SOCK_DGRAM
    */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd < 0){
        perror("socket error");
        exit(1);
    }
    
    //如果服务器断掉  监听的端口一般不能马上可以再次使用
    //这里可以设置参数项   使端口立马可以再次使用
    //设置套接字选项
    //补充说明: 因为recv是阻塞的, udp不可靠的,有可能造成数据的丢失
    //这就会造成程序一直阻塞再recv那里得不到执行  
    //再这里可以设置超时参数   
    //也可以自己使用signal设置等待超时的时间来进行 
    //因为设置了SO_REUSEADDR选项,再次绑定一个已经绑定的端口时,不会报端口绑定失败的错误
    //而是相当与重新绑定一下这个端口, 之前的绑定就会失效
    int ret;
    int opt = 1;
    if((ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))<0){
        perror("setsockopt error");
        exit(1);
    }

    // 步骤2 调用bind函数对socket和地址进行绑定
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;  //ipv4
    serveraddr.sin_port = htons(atoi(argv[1]));  //port
    serveraddr.sin_addr.s_addr = INADDR_ANY;  //绑定所有网卡ip
    if(bind(sockfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr))<0){
        perror("bind error");
        exit(1);
    }

    //步骤3: 和客户端进行双向的数据通信
    while(1){
        do_service();
    }



    return 0;
}
