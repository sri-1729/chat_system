#include<stdio.h>
#include<sys/socket.h>
#include<sys/socket.h>
#include <netinet/in.h> 
#include<string.h>
#include<time.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<stdlib.h>
#include<signal.h>
#include<pthread.h>

const char localhost[] = "127.0.0.1";
#define PORT 6678
#define backlog 5
#define MSG_SIZE 80

void DieWithError(char *errorMessage) {
  perror(errorMessage);
  exit(1);
}

void getmsg(char *expr){
    int i = 0;
    while(read(0, (expr + i), 1) > 0 && expr[i++] != '\n');
}

struct chat{
    char cli[20];
    char msg[50];
};

int completed;
void* sendTo(void *c_fd){
    int c = *(int *)c_fd;
    do{
        char msg[50];
        printf("[you] ");
        fflush(stdout);
        getmsg(msg);
        send(c, msg, 50, 0);
    }while(1);
    
}

void* recvFrom(void *c_fd){
    int c = *(int *)c_fd;
    do{
    struct chat ch;
    int cnt = recv(c, &ch, sizeof(ch), 0);
    if(cnt > 0){
        printf("\33[2K\r");
        printf("[%s] %s", ch.cli, ch.msg);
        printf("[you] ");
        fflush(stdout);
    }
    else
        DieWithError("Server Closed");
    }while(1);
}

void bye(int sig){
    completed = 1;
}
int main(){
    //signal(SIGINT, handler);
    completed = 0;
    int c_fd = socket(AF_INET, SOCK_STREAM, 0);
    signal(SIGINT, bye);
    struct sockaddr_in s_add;
    s_add.sin_family = AF_INET;
    s_add.sin_addr.s_addr = inet_addr(localhost);
    s_add.sin_port = htons(PORT);
    pthread_t s, r;
    if(connect(c_fd, (struct sockaddr *) &s_add, sizeof(s_add)) == -1)
        DieWithError("Connection Refused!!");
    char name[MSG_SIZE];
    char msg[50];
    scanf("%s", name);
    send(c_fd, name, MSG_SIZE, 0);
    pthread_create(&s, NULL, sendTo, (void *)&c_fd);
    pthread_create(&r, NULL, recvFrom, (void *)&c_fd);
    while(!completed){
        sleep(2);
    }
    close(c_fd);
}