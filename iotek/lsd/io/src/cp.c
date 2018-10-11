#include "io.h"
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <stdio.h>
//文件复制  示例程序
int main(int argc,char *argv[]){
    if(argc!=3){
        fprintf(stderr,"usage: %s srcfile dextfile\n",argv[0]);
        exit(1);
    }
    int fdin,fdout;
    //打开一个带读取的文件
    fdin=open(argv[1],O_RDONLY);
    printf("file length: %ld\n",lseek(fdin,0L,SEEK_END)); //定位到文件尾部，来实现查看文件长度
    //重新定位到文件的开头
    lseek(fdin,0L,SEEK_SET);
    if(fdin<0){

        fprintf(stderr,"usage: %s srcfile dextfile\n",argv[0]);
        exit(1);
    }else{
        printf("open file: %d\n",fdin);
    }
    //打开一个待写入的文件    0为黏着位
    fdout=open(argv[2],O_WRONLY|O_CREAT|O_TRUNC,0777);
    
    if(fdout<0){

        fprintf(stderr,"open error:%s\n",strerror(errno));
        exit(1);
    }else{
        printf("open file: %d\n",fdout);
    }

    //文件复制
    copy(fdin,fdout);
    //关闭文件
    close(fdin);
    close(fdout);

    return 0;
}
