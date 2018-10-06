#include <stdio.h>
#include <unistd.h>
int main(void){
    //是一个行缓存      行缓存满才会输出  或者碰到换行符自动输出
    printf("hello iotek");
    while(1){
      sleep(1);
    }
    return 0;
}
