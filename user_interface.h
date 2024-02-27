#ifndef USER_INERFACE
#define USER_INERFACE

#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct IP_s{
    char reg_IP[30];
    int reg_UDP;
    int TCP;
    char*IP;
}conect_inf;

int interface(conect_inf *input_inf);


#endif // MAIN_H
