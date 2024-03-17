#include "main.h"

///grupo 51
conect_inf data;
void close_sockets(){
    close(data.client_info.fd);
    close(data.host_info.fd);
    close(data.predecessor.TCP.fd);
    exit(EXIT_SUCCESS);
}


int main(int argc, char *argv[]){

    //Verificacao do input

    strcpy(data.reg_IP,"193.136.138.142");
    strcpy(data.reg_UDP,"59000");
    data.sucessor.ID[0]='\0';
    data.predecessor.ID[0]='\0';

    data.server_join=false;
    data.ring[0]='\0';
    data.id[0]='\0';

    signal(SIGINT, close_sockets);
    
   

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

    data.host_info.fd=-1;
    data.client_info.fd=-1;
    data.predecessor.TCP.fd=-1;

    create_TCP_server(&data);
    alloc_tabs(&data);


    

#ifdef DEBUG
    printf("DEBUG: Servidor TCP criado e memória alocada\n");
#endif

    fd_set rfds;
    int maxfd;
    char bufferhold[256]={0};
    ssize_t n;
    int cladd=0;
    int sucadd=0;
    while (1)
    {
        FD_ZERO(&rfds); // inicializar o conjunto de descritores a 0
        FD_SET(0,&rfds); // adicionar o descritor 0 (stdin) ao conjunto 
        if (data.host_info.fd!=-1) FD_SET(data.host_info.fd,&rfds); // adicionar o descritor fd (socket TCP) ao conjunto
        if (data.client_info.fd!=-1) FD_SET(data.client_info.fd,&rfds); // adicionar o descritor fd (socket TCP) ao conjunto
        if (data.predecessor.TCP.fd!=-1) FD_SET(data.predecessor.TCP.fd,&rfds); // adicionar o descritor fd (socket TCP) ao conjunto
        maxfd=retmax(data.host_info.fd,data.client_info.fd,data.predecessor.TCP.fd);
#ifdef DEBUG
        printf("Digite:\n");
#endif
        select(maxfd+1,&rfds,(fd_set*)NULL,(fd_set*)NULL,(struct timeval*)NULL);    // espera por um descritor pronto
        if (FD_ISSET(0,&rfds)){
#ifdef DEBUG
            printf("User Input\n");
#endif
            user_input(&data);
            continue;
        }
        if (FD_ISSET(data.host_info.fd,&rfds)){
#ifdef DEBUG
            printf("New connection\n");
#endif      
            char rdbuffer[256]={0};
            int futurefd;
            data.host_info.addrlen=sizeof(data.host_info.addr);
            if((futurefd=accept(data.host_info.fd,(struct sockaddr*) &data.host_info.addr,&data.host_info.addrlen))==-1)/*error*/ exit(1);
            FD_CLR(data.host_info.fd,&rfds);

            n=read(futurefd,rdbuffer,256);
            FD_CLR(futurefd,&rfds);
#ifdef DEBUG
            printf("BUFFER: %s",rdbuffer);
#endif
            char* rest;
            char* token; 
            int aux=0;
            char aux2[10]={0};
            strcpy(bufferhold,rdbuffer);
            token=strtok_r(bufferhold,"\n",&rest);
            while (token!=NULL)
            {
                strcpy(aux2,data.predecessor.ID);
                add_client(&data,token,futurefd);
                token=strtok_r(rest,"\n", &rest);
            }
            if (strcmp(aux2,data.sucessor.ID)!=0)
            {
                rmv_adj(&data,aux2);
            }
            cladd=1;
            
            
        }
        if (FD_ISSET(data.predecessor.TCP.fd,&rfds))
        {
#ifdef DEBUG
            printf("Message received from predecessor\n");
#endif      
            char rdbuffer[256]={0};
            n=read(data.predecessor.TCP.fd,rdbuffer,256);
            FD_CLR(data.predecessor.TCP.fd,&rfds);
#ifdef DEBUG
            printf("BUFFER: %s",rdbuffer);
#endif
            if (n==-1)
            {
                printf("ERRO\n");
            }else if (n==0){  
#ifdef DEBUG
                printf("DEBUG: predecessor disconnected\n");
#endif              
                pred_reconnect(&data,rdbuffer);
            }
            else{
                strcpy(bufferhold,rdbuffer);

                char* rest;
                char* token; 

                token=strtok_r(bufferhold,"\n",&rest);
                while (token!=NULL)
                {
                    if(strstr(token,"ROUTE")){
                        if (cladd==1)
                        {
                            cladd=0;
                            add_adj(&data,1);
                        }
                        chamada_route(&data,token);
                    }else if(strstr(rdbuffer,"CHORD")){
                        
                    }
                    token=strtok_r(rest,"\n", &rest);
                }
            }           
        }
        if (FD_ISSET(data.client_info.fd,&rfds))
        {
#ifdef DEBUG
            printf("Message received from sucessor\n");
#endif
            char rdbuffer[256]={0};

            n=read(data.client_info.fd,rdbuffer,256);
            FD_CLR(data.client_info.fd,&rfds);
#ifdef DEBUG
            printf("BUFFER: %s",rdbuffer);
#endif
            if (n==-1)
            {
                printf("ERRO\n");
            }else if (n==0){  
#ifdef DEBUG
                printf("DEBUG: Sucessor disconnected\n");
#endif              
                suc_reconnect(&data,rdbuffer);
            }
            else{
                strcpy(bufferhold,rdbuffer);

                char* rest;
                char* token; 
                
                token=strtok_r(bufferhold,"\n",&rest);
                while (token!=NULL)
                {
                    if (strstr(token,"ENTRY")){
                        char aux2[10]={0};
                        strcpy(aux2,data.sucessor.ID);
                        add_successor(&data,token);
                        if (strcmp(aux2,data.predecessor.ID)!=0){
                            rmv_adj(&data,aux2);
                        }
                        sucadd=1;
                    }else if (strstr(token,"SUCC")){
                        add_successor(&data,token);
                    }else if (strstr(token,"CHAT")){
                    }else if(strstr(token,"ROUTE")){
                        if(sucadd==1)
                        {
                            sucadd=0;
                            add_adj(&data,2);
                        }
                        chamada_route(&data,token);

                    }else if(strstr(rdbuffer,"CHORD")){
                    }
                    token=strtok_r(rest,"\n", &rest);
                }

            }

        }
    }
    return 0;
}

void user_input( conect_inf* data){
    char input[300] ;
    char id_buff[10];
    char* id_i;
    
    fgets(input, 299, stdin);
    //FD_CLR(0,data->rfds);
    
    if(input[0]=='x'){
        printf("Fecho da aplicação\n");
        freeaddrinfo(data->host_info.res);
        close(data->host_info.fd);
        free_tabs(data);
        exit(0);
    }

    else if(input[0]=='j' && input[1]==' '){
        sscanf(input,"%*s %s %s",data->ring,id_buff);
        if (data->id[0]!='\0' && strcmp(id_buff,data->id)!=0){
            printf("Está se a tentar resgistar num anel com um id diferente do que se registou na ligação com outro nó\n");
            return;
        }
        else{
            strcpy(data->id,id_buff);
        }
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
        if(leave_ring(data)==1){
            printf("Nó saiu do anel com sucesso\n");
        }
        else{
            printf("Nó nao retirado do anel\n");
        }
        
    }
    else if (input[0]=='d' && input[1]=='j'){
        sscanf(input,"%*s %s",id_buff);
        if (data->id[0]!='\0' && strcmp(data->id,id_buff)!=0){
            printf("Não é possível registar um nó com id difrente do registado no servidor\n");
            
        }
        sscanf(input,"%*s %s %s %s %s",data->id,data->sucessor.ID,data->sucessor.IP,data->sucessor.PORT);
        if (atoi(data->id)<0 || atoi(data->id)>99 || strlen(data->id)!=2){
            printf("Valor de id inválido (necessário 2 digitos)\nPor favor tente novamente\n");
            
        }
        else if (atoi(data->sucessor.ID)<0 || atoi(data->sucessor.ID)>99 || strlen(data->sucessor.ID)!=2){
            printf("Valor de id do sucessor inválido (necessário 2 digitos)\nPor favor tente novamente\n");
            
        }
        else if (atoi(data->sucessor.PORT)<=0){
            printf("Valor de porto do sucessor inválido\nPor favor tente novamente\n");
        }
        else if (strcmp(data->sucessor.ID,data->id)==0){    //inicio de anel
            printf("Início de um novo anel\n");
            strcpy(data->predecessor.ID,data->id);
            strcpy(data->predecessor.IP,data->IP);
            strcpy(data->predecessor.PORT,data->TCP);
            strcpy(data->sucessor.IP,data->IP);
            strcpy(data->sucessor.PORT,data->TCP);
            strcpy(data->secsuccessor.ID,data->id);
            strcpy(data->secsuccessor.IP,data->IP);
            strcpy(data->secsuccessor.PORT,data->TCP);
        }
        else{
            data->joining=direct_join(data);
            return;
        }
    }
    else if(input[0]=='s' && input[1]=='t'){
        printf("Nó atual:\n\tid: %s\n\tIP: %s\n\tPorta: %s\n",data->id,data->IP,data->TCP);
        printf("Predecessor:\n\tid: %s\n",data->predecessor.ID);
        printf("Sucessor:\n\tid: %s\n\tIP: %s\n\tPorta: %s\n",data->sucessor.ID,data->sucessor.IP,data->sucessor.PORT);
        printf("Segundo sucessor:\n\tid: %s\n\tIP: %s\n\tPorta: %s\n",data->secsuccessor.ID,data->secsuccessor.IP,data->secsuccessor.PORT);
    }
    else if(input[0]=='r'){
        char temop[10];
        sscanf(input,"%*s %s",temop);
        rmv(data,temop);
    }
    else if(input[0]=='e'){
        printf("Tabela de caminhos mais curtos:\n");
        for (int i = 0; i < 100; i++)
        {
            if (data->tb_caminhos_curtos[i][0]!='-')
            {
                printf("%d: %s\n",i,data->tb_caminhos_curtos[i]);
            }
            
        }
        
    }else{
        printf("Input inválido\nPor favor tente novamente\n");
    }
    return;
}

int retmax(int a, int b, int c){
    if (a>b && a>c) return a;
    else if (b>a && b>c) return b;
    else return c;
}