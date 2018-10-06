#include "io.h"
#include <unistd.h>
#include <errno.h>  //错误编码
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>

#define BUFFER_LEN 1024
//文件拷贝
void copy(int fdin,int fdout){
    char buffer[BUFFER_LEN];
    ssize_t size;
    while((size=read(fdin,buffer,BUFFER_LEN))>0){
        //查看当前偏移量

       // printf("read size: %ld\n",size);

       // printf("current: %ld\n",lseek(fdin,0L,SEEK_CUR));

        if(write(fdout,buffer,size)!=size){
            
            fprintf(stderr,"read error:%s\n",strerror(errno));
            exit(1);
        }
    }
    if(size<0){
        fprintf(stderr,"read error:%s\n",strerror(errno));
        exit(1);
    }


}

void set_fl(int fd,int flag){
    //获得原来的文件状态标志
    int val=fcntl(fd,F_GETFL);
    //增加新的文件状态标志
    val |= flag;
    //重新设置一下
    if(fcntl(fd,F_SETFL,val)<0){
        perror("fcntl error");
    }
}
void clr_fl(int fd,int flag){
    int val = fcntl(fd, F_GETFL);
    //清除指定的文件标志
    val &= ~flag;
    
    if(fcntl(fd,F_SETFL,val)<0){
        perror("fcntl error");
    }

}
