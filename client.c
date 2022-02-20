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

const char localhost[] = "127.0.0.1";
#define PORT 6678
#define backlog 5
#define MSG_SIZE 50

void DieWithError(char *errorMessage) {
  perror(errorMessage);
  exit(1);
}

void getexpr(char *expr){
    int i = 0;
    while(read(0, (expr + i), 1) > 0 && expr[i++] != '\n');
}


int main(){
    //signal(SIGINT, handler);
    int c_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in s_add;
    s_add.sin_family = AF_INET;
    s_add.sin_addr.s_addr = inet_addr(localhost);
    s_add.sin_port = htons(PORT);
    if(connect(c_fd, (struct sockaddr *) &s_add, sizeof(s_add)) == -1)
        DieWithError("Connection Refused!!");
    char expr[MSG_SIZE];
    char rec[MSG_SIZE];
    int res;
    do{
        printf("Enter query: ");
        fflush(stdout);
        getexpr(expr);
        send(c_fd, expr, MSG_SIZE, 0);
        //printf("msg sent\n");
        recv(c_fd, rec, MSG_SIZE, 0);
        memcpy(&res, rec, sizeof(int));
        printf("%d\n", res);
    }while(1);

}