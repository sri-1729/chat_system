#include<stdio.h>
#include<sys/socket.h>
#include<sys/socket.h>
#include <netinet/in.h> 
#include<string.h>
#include<time.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<stdlib.h>
#include<pthread.h>

const char localhost[] = "127.0.0.1";
#define PORT 6678
#define backlog 5
#define MSG_SIZE 80
#define MAX_CLIENTS 50

struct client{
    int id;
    int fd; 
    char name[20];
    //char msg[MSG_SIZE];
};

void DieWithError(char *errorMessage) {
  perror(errorMessage);
  exit(1);
}

void calendar(char curr[], int sz){
    struct timespec t;
    timespec_get(&t, TIME_UTC);
    strftime(curr, sz, "%D %T", localtime(&t.tv_sec));
} 
//mutex
pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;
char status[50];
struct client cls[50];

struct chat{
    char cli[20];
    char msg[50];
};

void send_msgs(int sender, char *msg){
    pthread_mutex_lock(&mut);
    for(int i = 0; i < MAX_CLIENTS; i++){
        if(status[i] == 'C' && i != sender){
            struct chat ch;
            memcpy(&ch.cli, cls[sender].name, 20);
            memcpy(&ch.msg, msg, 50);
            send(cls[i].fd, &ch, sizeof(ch), 0);
        }
    }
    pthread_mutex_unlock(&mut);
}

int add_client(int fd){
    pthread_mutex_lock(&mut);
    for(int i = 0; i < MAX_CLIENTS; i++){
        if(status[i] == 'N'){
            status[i] = 'C';
            cls[i].id = i;
            cls[i].fd = fd;
            //printf("Sucessfully Added\n");
            pthread_mutex_unlock(&mut);
            return i;
        }
    }
    pthread_mutex_unlock(&mut);
    return -1;
}

void remove_client(int client_in){
    pthread_mutex_lock(&mut);
    status[client_in] = 'N';
    pthread_mutex_unlock(&mut);
}
void* client_handler(void *fd){
    //struct client *cli = (struct client *)client;
    int fd_i = *(int *)fd;
    int ind = add_client(fd_i);
    char msg[MSG_SIZE];
    recv(cls[ind].fd, msg, MSG_SIZE, 0);
    strcpy(cls[ind].name, msg);
    printf("%s joined\n", cls[ind].name);
    sprintf(msg, "%s joined\n", cls[ind].name);
    send_msgs(0, msg);
    fflush(stdout);
    int left = 0;
    do{
        int cnt = recv(cls[ind].fd, msg, 50, 0);
        if(cnt > 0){
            printf("Successfully received!!\n");
            send_msgs(ind, msg);
        }
        else if(cnt <= 0){
            printf("%s has left\n", cls[ind].name);
            fflush(stdout);
            sprintf(msg, "%s has left\n", cls[ind].name);
            send_msgs(0, msg);
            left = 1;
            remove_client(ind);
        }
    }while(!left);

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
    pthread_t thread;
    struct sockaddr_in c_addr;
    for(int i = 0; i < MAX_CLIENTS; i++)
        status[i] = 'N';
    status[0] = 'C';
    strcpy(cls[0].name, "server");
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
        printf("Connection started at %s:\n", time);
        printf("-------------------------------------\n");

        pthread_create(&thread, NULL, client_handler, (void *)&s_msg_fd);
        sleep(1);
        
    }

}