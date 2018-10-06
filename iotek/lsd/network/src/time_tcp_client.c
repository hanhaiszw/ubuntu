#include <netdb.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <unistd.h>
#include <arpa/inet.h>
//需要传入服务器的IP和port
int main(int argc,char* argv[]){
    if(argc<3){
        printf("usage: %s ip port\n",argv[0]);
        exit(1);
    }
    //步骤1:创建socket
    int sockfd=socket(AF_INET,SOCK_STREAM,0);
    if(sockfd<0){
        perror("socket error");
        exit(1);
    }
    
    //往serveraddr中填入ip,port和地址族类型ipv4
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(atoi(argv[2]));//转化为网络字节序
    //将ip地址转换成网络字节序填入serveraddr中
    //填入的是需要连接的服务IP和端口
    inet_pton(AF_INET, argv[1], &serveraddr.sin_addr.s_addr);
    
    /*
     * 步骤2:客户端调用connect函数连接到服务器端
     */
     if(connect(sockfd,(struct sockaddr*)&serveraddr,sizeof(serveraddr)) < 0){
        perror("connect error");
        exit(1);
     }

     //步骤3: 调用IO函数(read/write)和服务器端进行双向通信
     char buffer[1024];
     memset(buffer, 0, sizeof(buffer));
     size_t size;
     if((size=read(sockfd,buffer,sizeof(buffer)))<0){
        perror("write error");
          
     }

     //显示
     if(write(STDOUT_FILENO, buffer, size)!=size){
        perror("write error");
       
     }

     //步骤4: 关闭socket
     close(sockfd);
        


    return 0;
}
