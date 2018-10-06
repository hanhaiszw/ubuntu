#include "io.h"
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int main(void){
    //初始化
    char buffer[4096]={'\0'};
    ssize_t size=0;
    //1设置非阻塞IO  则不会阻塞等待输入
    set_fl(STDIN_FILENO,O_NONBLOCK);
    sleep(5);  //睡眠5秒
    

    //从键盘读取  scanf   这里会阻塞  等待键盘输入
    //进程挂起 等待键盘输入
    size=read(STDIN_FILENO,buffer,sizeof(buffer));
    if(size<0){
        perror("read errod");
        exit(1);
    }else if(size==0){
        //睡眠时 按下CTRL + D  读到文件末尾
        printf("read finished!\n");

    }else{
        //显示到屏幕
        if(write(STDOUT_FILENO,buffer,size)!=size){
            perror("write error");
            exit(1);

        }

    }
    return 0;
    


}
