#ifndef USER_INERFACE
#define USER_INERFACE

#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/socket.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>
#include <errno.h>


#define CONNECTION_TIMEOUT_SECONDS 6





//estrutura para guardar informação do servidor TCP
typedef struct _TCP_server{
    int fd;
    struct sockaddr_in addr;    
    struct addrinfo hints;  
    struct addrinfo *res;
    socklen_t addrlen;
}TCP_server;


//estrutura para guardar informação das ligações
typedef struct _node{
    char ID[10];
    char IP[30];
    char PORT[10];
    TCP_server TCP;
}node;



typedef struct IP_s{
    
    char* IP;   //IP servidor da máquina a correr aplicação
    char* TCP;  //porto servidor da máquina a correr aplicação
    
    
    char reg_IP[30];    //IP servidor de nós do anel
    char reg_UDP[30];   //porto servidor de nós do anel


    char ring[10];
    char id[10];
    bool server_join;

    TCP_server host_info;   //informação do servidor TCP
    TCP_server client_info;    //informação do nó como cliente do seu predecessor

    node sucessor;    //informação das ligações
    node predecessor; //informação das ligações
    node secsuccessor;    //informação das ligações com 2o sucessor


}conect_inf;



char* join(conect_inf*inicial_inf,char* ring,char* id);
int leave(conect_inf*inicial_inf,char* ring,char* id);
void user_input( conect_inf* data);

void create_TCP_server(conect_inf* data);

int direct_join(conect_inf* data);

int add_client(conect_inf* data);

int decode_msg(int fd);

char* get_nodes(conect_inf*inicial_inf,char* ring,char* buffer);

#endif // MAIN_H
