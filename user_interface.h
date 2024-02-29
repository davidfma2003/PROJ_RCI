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



//estrutura para guardar informação das ligações
typedef struct _node{
    char ID;
    char IP;
    char* PORT;
}node;

//estrutura para guardar informação do servidor TCP
typedef struct _TCP_server{
    int fd;
    struct sockaddr_in addr;    
    struct addrinfo hints;  
    struct addrinfo *res;
    socklen_t addrlen;
}TCP_server;



typedef struct IP_s{
    
    char* IP;   //IP servidor da máquina a correr aplicação
    char* TCP;  //porto servidor da máquina a correr aplicação
    
    
    char reg_IP[30];    //IP servidor de nós do anel
    char reg_UDP[30];   //porto servidor de nós do anel

    char ring[10];
    char id[10];
    bool server_join;


    

    TCP_server host_info;   //informação do servidor TCP

    node sucessor;    //informação das ligações
    node predecessor; //informação das ligações


}conect_inf;



char* join(conect_inf*inicial_inf,char* ring,char* id);
int leave(conect_inf*inicial_inf,char* ring,char* id);
void user_input( conect_inf* data);

void create_TCP_server(conect_inf* data);


#endif // MAIN_H
