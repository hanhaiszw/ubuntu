#include "io.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
//cat  指令实现
//示例
//
int main(int argc,char* argv[]){
    //标准输入 键盘
    int fd_in=STDIN_FILENO;//0
    //标准输出  屏幕
    int fd_out=STDOUT_FILENO;//1

    for(int i=1;i<argc;i++){
        fd_in=open(argv[i],O_RDONLY);
        if(fd_in<0){
            //可以自动解析出标准错误的信息   
            //比如文件不能存在错误就会输出  open error:No such file or dictionary
            perror("open error");
            continue;
        }
        copy(fd_in,fd_out);
        close(fd_in);
    }
    if(argc==1) copy(fd_in,fd_out);


}
