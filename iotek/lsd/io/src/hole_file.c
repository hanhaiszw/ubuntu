#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
//生成空洞文件
//测试
//在io目录下
// bin/hole_file hole.txt
// more hole.txt   只能查看到0123456789
// od  -c hole.txt 可以查看到所有
char* buffer="0123456789";
int main(int argc,char* argv[]){
    if(argc<2){
        fprintf(stderr,"-usage: %s [file]\n",argv[0]);
        exit(1);
    }

    int fd =open(argv[1],O_WRONLY|O_CREAT|O_TRUNC,0777);
    if(fd<0){
        perror("open error");
        exit(1);

    }

    size_t size=strlen(buffer)*sizeof(char);
    //将字符串写入到空洞文件
    if(write(fd,buffer,size) !=size){
        perror("write error");
        exit(1);
    }
    if(lseek(fd,10L,SEEK_END)<0){
        perror("lseek error");
        exit(1);
    }
    //从文件尾部的10个字节处再写入字符串
    if(write(fd,buffer,size) !=size){
        perror("write error");
        exit(1);
    }
    close(fd);
    return 0;

}
