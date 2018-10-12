#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

/*
   发送者向广播地址发送数据  相当于客户端
   测试:
      1  bin/receive 8888
      2  bin/broadcast 192.168.61.255 8888   
      广播地址查看 ipconfig  最后一段为255

 */
int main(int argc, char* argv[]){
    if(argc < 3){
        fprintf(stderr,"usage: %s ip port\n", argv[0]);
        exit(1);
    }

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd < 0){
        perror("socket error");
        exit(1);
    }

    int opt = 1;
    //采用广播方式发送   设置广播参数
    setsockopt(sockfd,SOL_SOCKET, SO_BROADCAST, &opt, sizeof(opt));

    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET; //ipv4
    serveraddr.sin_port = htons(atoi(argv[2]));//port
    inet_pton(AF_INET, argv[1], &serveraddr.sin_addr.s_addr);//ip
    printf("I will broadcast...\n");
    char* info = "hello iotek";
    size_t size = strlen(info) * sizeof(char);
    //发送数据向广播地址
    if(sendto(sockfd, info,size, 0, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) < 0){
        perror("sendto error");
        exit(1);
    }else{
        printf("broadcast success\n");
    }
    close(sockfd);
    return 0;

}
