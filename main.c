#include "main.h"

///grupo 51


int main(int argc, char *argv[]){

    fd_set* descriptors;

    FD_SET(0,descriptors);
    



















    conect_inf inicial_inf;
    strcpy(inicial_inf.reg_IP,"193.136.138.142");
    strcpy(inicial_inf.reg_UDP,"59000");
    char input[300] ;
    char ring[10],id[10];
    char* id_i;
    bool server_join=false;
    ring[0]='\0';
    id[0]='\0';

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

        select
        printf("Digite:");
        fgets(input, 299, stdin);
        if(input[0]=='x'){
            printf("Fecho da aplicação\n");
            exit(0);
        }
        else if(input[0]=='j' && input[1]==' '){
            sscanf(input,"%*s %s %s",ring,id);
            if (atoi(ring)<0 || atoi(ring)>999 || strlen(ring)!=3){
                printf("Valor de ring inválido (necessaŕio 3 digitos)\nPor favor tente novamente\n");
            }
            else if (atoi(id)<0 || atoi(id)>99 || strlen(id)!=2){
                printf("Valor de id inválido (necessário 2 digitos)\nPor favor tente novamente\n");
            }
            else{
                id_i=join(&inicial_inf,ring,id);
                if (id_i==NULL)
                {
                    printf("Não foi possível juntar ao servidor\n");
                    continue;
                }
                server_join=true;

            }
        }
        else if (input[0]=='l'){
            if(server_join){
                if (leave(&inicial_inf,ring,id)==0){
                    server_join=false;
                    printf("Nó retirado do servidor com sucesso\n");
                }
                else
                printf("Não é possível conectar com o servidor\n");
            }
            else{
                printf("Não é possível sair do servidor sem lá estar\n");
            }
        }
        else{
            printf("Input iválido\nPor favor tente novamente\n");
        }
    }
    return 0;
}

