#ifndef __ACCOUNT_H__
#define __ACCOUNT_H__
#include <pthread.h>

typedef struct
{
    int code; //账号号码
    double balance; //账号余额
    //定义一把互斥锁
    //保护银行账户  
    //互斥锁用来锁定一个账户,则和这个账户绑定在一起,尽量不去使用全局变量,否则可能出现同一个锁去锁
    pthread_mutex_t mutex;
}Account;

//创建账户
extern Account* create_account(int code, double balance); 

//销毁账户
extern void destroy_account(Account* a);

//取款
extern double withdraw(Account *a, double amt);

//存款
extern double deposit(Account *a, double amt);

//查看账户余额
extern double get_balance(Account *a);


#endif

