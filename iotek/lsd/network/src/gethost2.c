#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <arpa/inet.h>

void out_addr(struct hostent *h){
    printf("host name: %s\n", h->h_name);
    printf("addrtype: %s\n",
                h->h_addrtype == AF_INET ? "IPV4":"IPV6");
    char ip[16];
    memset(ip, 0, sizeof(ip));
    //网络字节序转化为主机字节序
    inet_ntop(h->h_addrtype, h->h_addr_list[0], ip, sizeof(ip));
    printf("ip address: %s\n", ip);
    
    //输出别名
    int i=0;
    while(h->h_aliases[i] != NULL){
        printf("aliase: %s\n", h->h_aliases[i++]);
    }
}
int main(int argc, char *argv[]){
    if(argc < 2){
        fprintf(stderr, "usage: %s host\n",argv[0]);
        exit(1);

    }

    struct hostent *h;
    //gethost获取所有的 再遍历判断是否是输入的hostname
    //遍历所有组
    while((h = gethostent())!=NULL){
        if(!strcmp(argv[1], h->h_name)){

            out_addr(h);
            exit(0);
        }else{
            int i=0;
            while(h->h_aliases[i] != NULL){
                if(!strcmp(argv[1], h->h_aliases[i])){
                    out_addr(h);
                    exit(0);
                }
                i++;
            }
        }
    }

    //释放一下
    endhostent();
    printf("no %s exist\n", argv[1]);
    return 0;

}
