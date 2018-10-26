#include <stdio.h>
#define MY_INT 8
//加法函数
float add(int a,int b,float c){
    return a+b+c;
}
int main(){
    int a = 5;
    int b = 9;
    float c = 1.0;
    printf("the result is %.2f\n",add(a,b,c)+MY_INT);
    return 0;
}
