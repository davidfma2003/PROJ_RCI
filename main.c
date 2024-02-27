#include "main.h"




int main(int argc, char *argv[]){
    conect_inf inicial_inf;
    strcpy(inicial_inf.reg_IP,"193.136.138.142");
    strcpy(inicial_inf.reg_UDP,"59000");
    char input[300] ;
    char ring[10],id[10];
    int ret=-1;
    if (argc!=3 && argc!=5){
        printf("Número de argumentos de inicialição do programa incoerentes\n");
        exit(0);
    }
    else if (argc==5){
        strcpy(inicial_inf.reg_UDP,argv[4]);
        strcpy(inicial_inf.reg_IP,argv[3]);
    }
    inicial_inf.IP=argv[1];
    inicial_inf.TCP=argv[2];
    if (atoi(inicial_inf.reg_UDP)<=0 || atoi(inicial_inf.TCP)<=0) {
        printf("Valores de portos TCP ou UDP incoerentes\n");
        exit(0);
    }

    while (1){
        printf("Digite:");
        fgets(input, 299, stdin);
        if(input[0]=='x'){
            printf("Fecho da aplicação\n");
            exit(0);
        }
        else if(input[0]=='j'){
            sscanf(input,"%*s %s %s",ring,id);
            if (atoi(ring)<0 || atoi(ring)>999 || strlen(ring)!=3){
                printf("Valor de ring inválido (necessaŕio 3 digitos)\nPor favor tente novamente\n");
            }
            else if (atoi(id)<0 || atoi(id)>99 || strlen(id)!=2){
                printf("Valor de id inválido (necessário 2 digitos)\nPor favor tente novamente\n");
            }
            else{
                ret=join(&inicial_inf,ring,id);
                if (ret!=-1)
                printf("Valor de id indisponivél no servidor\n Entrou com o valor de id %d\n",ret);
            }
        }
        else{
            printf("Input iválido\nPor favor tente novamente\n");
        }
    }
    return 0;
}

void limparBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}