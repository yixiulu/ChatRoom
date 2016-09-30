//实现基于TCP模型一对多模型的服务器
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<signal.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<pthread.h>
int sockfd;
int fd[100] = {0};
int pos = 0;
int amount = 0;
int total_enter = 0;
int acc[100];
void fa(int signo){
    printf("Closing the server，please wait...\n");
    sleep(2);
    close(sockfd);
    printf("successfully close the server\n");
    exit(0);//终止进程
}
void *task(void *p){
    //针对每一个客户端进行不断通信，使用read/write函数
    int num = *(int*)p;
    while(1) {
        char buf[10000] = {};
        int res = read(fd[num],buf,sizeof(buf));
        if (-1 == res) {
            perror("read"),exit(-1);
        }
        printf("Msg received from client：%s\n，size of msg：%d\n", buf, res);
        //判断客户端是否下线
        if (!strcmp(buf,"bye")) {
            printf("the server out\n");
            close(fd[num]);
            fd[num] = -1;
            amount--;
            pthread_exit(0);
        }
        for(int i = 0; i < pos; i++){
            if(fd[i] != fd[num]){
                res = write(fd[i],buf,strlen(buf));
            }
            if (-1 == res) {
                perror("write"),exit(-1);
            }
        }
    }
}
int main() {
    //创建socket函数，使用socket函数
    sockfd = socket(AF_INET,SOCK_STREAM,0);
    if (-1 == sockfd) {
        perror("sockfd"),exit(-1);
    }
    //准备通信地址，使用结构体类型
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8888);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    //解决重新启动程序时地址被占用的问题
    int reuseaddr = 1;
    setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&reuseaddr,sizeof(reuseaddr));
    //绑定socket和通信地址，使用bind函数
    int res = bind(sockfd,(struct sockaddr*)&addr,sizeof(addr));
    if (-1 == res) {
        perror("bind"), exit(-1);
    }
    printf("bind successful\n");
    res = listen(sockfd,100);
    if (res == -1) {
        perror("listen"),exit(-1);
    }
    printf("listen successful\n");
    //采用信号2来关闭服务器
    printf("To shut down the server，press ctrl+c...\n");
    signal(2,fa);
    //不断地响应客户端的连接请求，accept函数
    while(1){
        amount++;
        total_enter++;
        if(amount >= 100){
            printf("Full\n");
        } else {
            if(total_enter >= 100){
                for (int i = 0; i < 99; i++) {
                    if(fd[i] == -1){
                        pos = i;
                        break;
                    }
                }
            }
            struct sockaddr_in recv_addr;
            socklen_t len = sizeof(recv_addr);
            fd[pos] = accept(sockfd,(struct sockaddr*)&recv_addr,&len);
            if (-1 == fd[pos]) {
                perror("accept"),exit(-1);
            }
            char *ip= inet_ntoa(recv_addr.sin_addr);
            printf("client %s connect successful\n", ip);
            acc[pos] = pos;
            pthread_t tid;
            pthread_create(&tid,NULL,task,&acc[pos]);
            pos++;
            pthread_detach(tid);
        }
    }
    return 0;
}

