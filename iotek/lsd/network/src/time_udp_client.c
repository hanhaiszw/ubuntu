#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <unistd.h>
#include <arpa/inet.h>
/*
  测试:
  1.开启服务器   bin/time_udp_server 8888
  2.另一个terminal中启动客户端连接
    bin/time_udp_client 127.0.0.1 8888

*/

int is_host(struct hostent *host, char *name){
    if(!strcmp(host->h_name, name)) return 1;
    int i=0;
    while(host->h_aliases[i] != NULL){
        if(!strcmp(host->h_aliases[i], name)) return 1;
        i++;
    }
    return 0;
}
unsigned int get_ip_by_name(char* name){
    unsigned int ip=0;
    struct hostent *host;
    while((host = gethostent()) != NULL){
        if(is_host(host, name)){
            //取出IP地址
            memcpy(&ip, host->h_addr_list[0], 4);
            break;
        }    
    }
    endhostent();
    return ip;
}
int main(int argc, char *argv[]){
    
    if(argc < 3){
        printf("usage: %s ip port\n",argv[0]);
        exit(0);
    }

    //步骤一:创建socket
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd < 0){
        perror("socket error");
        exit(1);
    }

    //步骤二: 调用recvfrom和sendto等函数和服务器端进行双向通信
    
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET; //ipv4
    serveraddr.sin_port = htons(atoi(argv[2]));//port;
    //1
    //inet_pton(AF_INET, argv[1], &serveraddr.sin_addr.s_addr); //ip
    //1



    //2   返回的就是网络字节序ip
    //根据域名获取IP地址
    //如果不自己解析  则还是可以处理localhost的  其他的不行??? 
    //经测试在/etc/hosts中其他的域名 也可以自动解析
    unsigned int ip = get_ip_by_name(argv[1]);
    if(ip !=0){
        serveraddr.sin_addr.s_addr = ip;
    }else{
        inet_pton(AF_INET, argv[1], &serveraddr.sin_addr.s_addr); //ip
    }
    //2

    //udp中同样可以调用connect函数,和TCP中不同的是,udp的connect函数不会进行三次握手
    //udp中的connect函数只是在内核中记录了服务器的ip和port信息
    //调用connect函数后,recv就会只接受到服务器端的数据信息,如果没有调用,客户端就有可能接受到服务器以外其他地方的数据
    //一般应该调用connect来做
    /*
    if(connect(sockfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr))<0){
        perror("connect error");
        exit(1);
    }*/
    //当调用connect函数后,下面就可以使用send函数来发送数据 不需要再使用sendto来指定服务器的地址信息
    //send(sockfd, buffer, sizeof(buffer),0)


    char buffer[1024] = "hello iotek";
    //向服务器端发送数据报文
    if(sendto(sockfd, buffer, sizeof(buffer),0,
            (struct sockaddr*)&serveraddr,sizeof(serveraddr)) < 0){
        perror("sendto error");
        exit(1);        
    }else{
        //发送成功则接收
        //接受服务器端发送的数据报文
        memset(buffer, 0, sizeof(buffer));
        //udp是面向无连接  无需判断是否连接已经断开 
        //tcp是相面连接 readsize==0时,对方已经关闭掉了
        size_t size;
        if((size = recv(sockfd, buffer, sizeof(buffer), 0))<0){
            perror("recv error");
            exit(1);
        }else{
            printf("%s",buffer);
        }
    }
    close(sockfd);

    
    return 0;
}
