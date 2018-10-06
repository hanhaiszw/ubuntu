#include "io.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
int main(int argc,char* argv[]){
    int fd_in,fd_out;
    int flag=0;
    
    int i=0;
    for(int i=1;i<argc;i++){
        //+改为输入重定向<   -改为输出重定向>
        if(!strcmp("+",argv[i])){
            fd_in=open(argv[++i],O_RDONLY);
            if(fd_in<0){
                perror("open error");
                exit(1);
            }
            //将标准输入重定向到文件 复制的实际是指针
            if(dup2(fd_in,STDIN_FILENO)!=STDIN_FILENO){
                perror("dup2 error");
                exit(1);
            }
            //关闭不再使用的fd_in
            close(fd_in);
        }else if(!strcmp("-",argv[i])){

            fd_out=open(argv[++i],O_WRONLY|O_CREAT|O_TRUNC,0777);
            if(fd_out<0){
                perror("open error");
                exit(1);

            }
            //将输出重定向到文件
            if(dup2(fd_out,STDOUT_FILENO)!=STDOUT_FILENO){
                perror("dup2 error");
                exit(1);
            }
            close(fd_out);

        }else{
            //其他情况
            flag=1;
            
            fd_in=open(argv[i],O_RDONLY);
            if(fd_in<0){
                perror("open error");
                exit(1);
            }
            //将标准输入重定向到文件
            if(dup2(fd_in,STDIN_FILENO)!=STDIN_FILENO){
                perror("dup2 error");
                exit(1);
            }
            copy(STDIN_FILENO,STDOUT_FILENO);
            close(fd_in);

        }

    }
    if(!flag){
        copy(STDIN_FILENO,STDOUT_FILENO);
    }
    return 0;
}
