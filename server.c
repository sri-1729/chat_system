#include<stdio.h>
#include<sys/socket.h>
#include<sys/socket.h>
#include <netinet/in.h> 
#include<string.h>
#include<time.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<stdlib.h>

const char localhost[] = "127.0.0.1";
#define PORT 6678
#define backlog 5
#define MSG_SIZE 50

void DieWithError(char *errorMessage) {
  perror(errorMessage);
  exit(1);
}

void calendar(char curr[], int sz){
    struct timespec t;
    timespec_get(&t, TIME_UTC);
    strftime(curr, sz, "%D %T", localtime(&t.tv_sec));
} 

void calculator(int s_msg_sock, int cno){
    struct timeval t;
    t.tv_sec = 15;
    
    do{
    char msg[MSG_SIZE];
    // printf("hi\n");
    // fflush(stdout);
    setsockopt(s_msg_sock, SOL_SOCKET, SO_RCVTIMEO, &t, sizeof(t));
    if(recv(s_msg_sock, msg, MSG_SIZE,0) <= 0){
        return;
    }
    printf("[Client%d]%s", cno, msg);
    int a, b, res;
    char op;
    sscanf(msg, "%d %c %d", &a, &op, &b);
    switch (op)
    {
    case '+':
        /* code */
        res = a + b;
        break;
    case '-':
        res = a - b;
        break;
    case '*':
        res = a * b;
        break;
    case '/':
        if(b != 0)
            res = a / b;
        break;
    default:
        break;
    }
    char snd[MSG_SIZE];
    printf("[Server]%d\n", res);
    memcpy(snd, &res, sizeof(int));
    send(s_msg_sock, snd, MSG_SIZE, 0);
    }while(1);
}

int main(){
    int s_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in s_add;
    s_add.sin_family = AF_INET;
    s_add.sin_addr.s_addr = inet_addr(localhost);
    s_add.sin_port = htons(PORT);
    int opt = 1;
    setsockopt(s_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                                                  &opt, sizeof(opt));
    if(bind(s_fd, (struct sockaddr *)&s_add, sizeof(s_add)) == -1){
        DieWithError("Binding Failed");
    }
    if(listen(s_fd, backlog) == -1){
        DieWithError("Listen Failed");
    }
    printf("listening on %s:%d\n", localhost, PORT);
    fflush(stdout);
    struct sockaddr_in c_addr;
    int cnt = 0;
    for(;;){
        int s_msg_fd;
        socklen_t sz;
        if((s_msg_fd = accept(s_fd, (struct sockaddr *)&c_addr, &sz)) == -1){
            DieWithError("Accept Failed");
        }
        printf("-------------------------------------\n");
        char time[100];
        calendar(time, 100);
        printf("Connection started at %s for client no %d:\n", time, ++cnt);
        printf("-------------------------------------\n");
        pid_t x;
        if((x = fork()) == 0){
            // printf("%d\n", x);
            calculator(s_msg_fd, cnt);
            close(s_msg_fd);
           // printf("I'm back\n");
        
        }
        
    }

}