//实现基于TCP一对多模型的客户端
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<pthread.h>
#include<time.h>
int sockfd;
char name[100];
void gettime(){
    time_t t = time(NULL);
    struct tm *curr_time = localtime(&t);
    printf("%02d:%02d:%02d\n",curr_time->tm_hour,curr_time->tm_min,curr_time->tm_sec);
}
char *readstr(void){
    char c;
    if (scanf(" %c", &c) != 1){
        return NULL;
    }
    int maxlen = strlen(name)+1;
    char *str = malloc(maxlen * sizeof(char));
    int len = maxlen-1;
    for(int i = 0; i < maxlen;i++){
        str[i] = name[i];
    }
    str[len] =':';
    len++;
    do{
        str[len] = c;
        ++len;
        if(len == maxlen){
            maxlen *= 2;
            str = realloc(str, maxlen *sizeof(char));
        }
        if(scanf("%c" , &c) != 1) {
            break;
        }
    } while (c != '\n');
    str[len] = '\0';
    if(str[len-1] == 'e' && str[len-2] == 'y'&& str[len-3] == 'b' && len == strlen(name)+4){
        free(str);
        return "bye";
    } else {
        return str;
    }
}
void *tasksend(void *p){
    printf("talk begin\n");
    while(1){
        char *buf = readstr();
        int res = write(sockfd,buf, strlen(buf));
        if(-1 == res) {
            perror("write"),exit(-1);
        }
        //判断客户端是否下线
        if(!strcmp(buf,"bye")){
            exit(0);
        }
        if(buf!=NULL){
            free(buf);
        }
    }
}
void *taskrev(void *p){
    while(1){
        char buf2[10000] = {};
        int res = read(sockfd,buf2,sizeof(buf2));
        if(-1 == res) {
            perror("read"),exit(-1);
        }
        gettime();
        printf("%s\n",buf2);
    }
}
int main() {
    sockfd = socket(AF_INET,SOCK_STREAM,0);
    if(-1 == sockfd) {
        perror("socket"),exit(-1);
    }
    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8888);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    int res = connect(sockfd,(struct sockaddr*)&addr,sizeof(addr));
    if(res == -1) {
        perror("connect"),exit(-1);
    }
    //不断地和服务器进行通信，使用read/write
    printf("Please Enter the nick name\n");
    scanf("%s", name);
    char enter[107]={};
    strcat(enter, name);
    strcat(enter, " enters");
    res = write(sockfd,enter,strlen(enter));
    pthread_t tid1,tid2;
    pthread_create(&tid1,NULL,taskrev,NULL);
    pthread_create(&tid2,NULL,tasksend,NULL);
    pthread_join(tid1,NULL);
    pthread_detach(tid2);
    return 0;
}
