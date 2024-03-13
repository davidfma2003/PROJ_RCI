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
#include <ctype.h>

#define DEBUG 1


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
    node chords;

    char *tb_encaminhamento[100][100];
    char tb_exped[100][3];
    char *tb_caminhos_curtos[100];


}conect_inf;

void user_input(conect_inf* data);

char* join(conect_inf*inicial_inf,char* ring,char* id);
int leave(conect_inf*inicial_inf,char* ring,char* id);


void create_TCP_server(conect_inf* data);

int direct_join(conect_inf* data);

int add_client(conect_inf* data);

void rmv(conect_inf*inicial_inf,char *node_id);

void add_successor(conect_inf* data, char buffer[128]);

int leave_ring(conect_inf* data);

void pred_reconnect(conect_inf* data, char buffer[128]);
void suc_reconnect(conect_inf* data, char buffer[128]);

void alloc_tabs(conect_inf* data);

void init_tabs(conect_inf* data);
void free_tabs(conect_inf* data);
void add_adj(conect_inf*data,int pos);
void rmv_adj(conect_inf*data,int pos);
void chamada_route(conect_inf*data,char*mensagem);
    void refresh_caminho_mais_curto(conect_inf*data,int linha);
    int contar_nos_no_caminho(char *str);

#endif // MAIN_H
