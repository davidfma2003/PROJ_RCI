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

typedef struct _node{
    char reg_IP[30];
    char reg_UDP[30];
    char* TCP;
    char* IP;
}node;

typedef struct IP_s{
    char reg_IP[30];
    char reg_UDP[30];
    char* TCP;
    char* IP;
    
    node connection;
}conect_inf;



char* join(conect_inf*inicial_inf,char* ring,char* id);
int leave(conect_inf*inicial_inf,char* ring,char* id);
#endif // MAIN_H
