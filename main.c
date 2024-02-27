#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"

int main(int argc, char *argv[]){
    conect_inf inicial_inf;
    strcpy(inicial_inf.reg_IP,"193.136.138.142");
    inicial_inf.reg_UDP=59000;
    inicial_inf.TCP=0;
    if (argc!=3 && argc!=5){
        printf("Número de argumentos de inicialição do programa incoerentes\n");
        exit(0);
    }
    else if (argc==5){
        inicial_inf.reg_UDP=atoi(argv[4]);
        strcpy(inicial_inf.reg_IP,argv[3]);
    }
    inicial_inf.IP=argv[1];
    inicial_inf.TCP=atoi(argv[2]);
    if (inicial_inf.reg_UDP<=0 || inicial_inf.TCP<=0) {
        printf("Valores de portos TCP ou UDP incoerentes\n");
        exit(0);
    }
    return 0;
}