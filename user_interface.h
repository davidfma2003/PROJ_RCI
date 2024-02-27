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

typedef struct IP_s{
    char reg_IP[30];
    char reg_UDP[30];
    char* TCP;
    char*IP;
}conect_inf;

int join(conect_inf*inicial_inf,char* ring,char* id);

#endif // MAIN_H
