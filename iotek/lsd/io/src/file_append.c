#include "io.h"
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

int main(int argc,char* argv[]){
    if(argc<3){
        printf("usage: %s content destfile\n",argv[0]);
        exit(1);
    }
    //O_APPEND原子操作  防止多线程写文件时被覆盖掉(?)
    //int fd=open(argv[2],O_WRONLY | O_APPEND);
    
    int fd=open(argv[2],O_WRONLY);//使用fcntl添加上O_APPEND属性
    //设置追加文件标志
    set_fl(fd,O_APPEND);
    //清除追加文件标志
    //clr_fl(fd,O_APPEND);

    if(fd<0){
        perror("open error");
    }

    sleep(10);
    //在文件的尾部追加写入
    size_t size=strlen(argv[1]) * sizeof(char);


    if(write(fd,argv[1],size) !=size){
        perror("write error");
        exit(1);
    }
    close(fd);
    return 0;
}
