#include "msg.h"
#include <unistd.h>
#include <string.h>
#include <memory.h>
#include <sys/types.h>


//计算校验码
static unsigned char msg_check(Msg *message){
    unsigned char s = 0;
    int i;
    for(i=0; i<sizeof(message->head);i++){
        s += message->head[i];
    }

    for(i=0; i<sizeof(message->buff); i++){
        s += message->buff[i];
    }
    return s;
}
/*
 * 发送一个基于自定义协议的message
 * 发送的数据存放在buff中
 */
int write_msg(int sockfd, char *buff, size_t len){
    Msg message;
    memset(&message, 0, sizeof(message));
    strcpy(message.head, "iotek2012");
    memcpy(message.buff, buff,len);
    message.checknum = msg_check(&message);
    if(write(sockfd, &message, sizeof(message)) != sizeof(message)){
        return -1;
    }
}

/*
 * 读取一个基于自定义协议的message
 * 读取的数据存放在buff中
 */
int read_msg(int sockfd,char *buff,size_t len){
    Msg message;
    memset(&message, 0, sizeof(message));
    size_t size;
    if((size =read(sockfd, &message, sizeof(message))) < 0){
        return -1;
    }else if(size==0){
        //数据已经读取完全
        //或者一端已经关闭 或网线断开
        return 0;
    }
    //进行校验码验证,判断接受都达到message是否完整
    unsigned char s = msg_check(&message);
    if((s == (unsigned char)message.checknum)
        &&(!strcmp("iotek2012",message.head))){
        //取出msg体部的信息
        memcpy(buff, message.buff, len);
        return sizeof(message);
     }
     return -1;
}

