#ifndef __IO_H
#define __IO_H

extern void copy(int fdin,int fdout);
//设置文件标志  清除文件标志
extern void set_fl(int fd,int flag);
extern void clr_fl(int fd,int flag);


#endif
