#ifndef __MSG_H
#define __MSG_H
/*
 * 自定义应用层协议
 */

#include <sys/types.h>
typedef struct{
    //协议头部
    char head[10]; //logo什么的
    char checknum; //校验码  验证数据是否一致

    //协议体部
    char buff[512];//数据
}Msg;

/*
 * 发送一个基于自定义协议的message
 * 发送的数据存放在buff中
 */
extern int write_msg(int sockfd, char *buff, size_t len);

/*
 * 读取一个基于自定义协议的message
 * 读取的数据存放在buff中
 */
extern int read_msg(int sockfd,char *buff,size_t len);



#endif
