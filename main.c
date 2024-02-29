#include "main.h"

///grupo 51


int main(int argc, char *argv[]){

    //Verificacao do input
    conect_inf data;
    strcpy(data.reg_IP,"193.136.138.142");
    strcpy(data.reg_UDP,"59000");
    
    data.server_join=false;
    data.ring[0]='\0';
    data.id[0]='\0';

    if (argc!=3 && argc!=5){
        printf("Número de argumentos de inicialição do programa incoerentes\n");
        exit(0);
    }
    else if (argc==5){
        strcpy(data.reg_UDP,argv[4]);
        strcpy(data.reg_IP,argv[3]);
    }
    data.IP=argv[1];
    data.TCP=argv[2];
    if (atoi(data.reg_UDP)<=0 || atoi(data.TCP)<=0) {
        printf("Valores de portos TCP ou UDP incoerentes\n");
        exit(0);
    }




    create_TCP_server(&data);
    printf("Servidor TCP criado\n");


    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(0,&rfds); // adicionar o descritor 0 (stdin) ao conjunto 
    FD_SET(data.host_info.fd,&rfds); // adicionar o descritor fd (socket UDP) ao conjunto     
    
    int maxfd=data.host_info.fd;
 
    while (1)
    {
        printf("Digite:");
        int counter=select(maxfd+1,&rfds,(fd_set*)NULL,(fd_set*)NULL,(struct timeval*)NULL);
        if (counter==-1){
            printf("Erro no select\n");
            exit(0);
        }
        else if (counter==0){
            printf("Timeout\n");
            exit(0);
        }
        else{
            if (FD_ISSET(0,&rfds)){
                printf("User Input\n");
                user_input(&data);
                
            }
            if (FD_ISSET(data.host_info.fd,&rfds)){
                printf("Message\n");
            }
        }

    }

/*
    while (1)
    {
        counter=select(maxfd+1,&rfds,(fd_set*)NULL,(fd_set*)NULL,(struct timeval*)NULL);
        if (counter==-1){
            printf("Erro no select\n");
            exit(0);
        }
        else if (counter==0){
            printf("Timeout\n");
            exit(0);
        }
        else{
            if (FD_ISSET(0,&rfds)){
                printf("User Input\n");
            }
            if (FD_ISSET(data.host_info.fd,&rfds)){
                printf("Message\n");
            }
        }

    }*/
    return 0;
}

void user_input( conect_inf* data){
    char input[300] ;
    char* id_i;
    
    printf("Digite:");
    fgets(input, 299, stdin);
    
    if(input[0]=='x'){
        printf("Fecho da aplicação\n");
        freeaddrinfo(data->host_info.res);
        close(data->host_info.fd);
        exit(0);
    }

    else if(input[0]=='j' && input[1]==' '){
        sscanf(input,"%*s %s %s",data->ring,data->id);
        if (atoi(data->ring)<0 || atoi(data->ring)>999 || strlen(data->ring)!=3){
            printf("Valor de ring inválido (necessaŕio 3 digitos)\nPor favor tente novamente\n");
        }
        else if (atoi(data->id)<0 || atoi(data->id)>99 || strlen(data->id)!=2){
            printf("Valor de id inválido (necessário 2 digitos)\nPor favor tente novamente\n");
        }
        else{
            id_i=join(data,data->ring,data->id);
            if (id_i==NULL)
            {
                printf("Não foi possível juntar ao servidor\n");
                return;
            }
            data->server_join=true;

        }
    }
    else if (input[0]=='l'){
        if(data->server_join){
            if (leave(data,data->ring,data->id)==0){
                data->server_join=false;
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

