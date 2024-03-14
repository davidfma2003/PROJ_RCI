#include "main.h"

///grupo 51


int main(int argc, char *argv[]){

    //Verificacao do input
    conect_inf data;
    strcpy(data.reg_IP,"193.136.138.142");
    strcpy(data.reg_UDP,"59000");
    data.sucessor.ID[0]='\0';
    data.predecessor.ID[0]='\0';

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
    alloc_tabs(&data);

#ifdef DEBUG
    printf("DEBUG: Servidor TCP criado e memória alocada\n");
#endif

    fd_set rfds;
    int maxfd;
    int counter;
    while (1)
    {
        if ((data.id[0]=='\0') || (strcmp(data.id,data.sucessor.ID)==0))
        {
            maxfd=data.host_info.fd;    // valor do descritor mais alto
            printf("Digite:\n");
            
            FD_ZERO(&rfds); // inicializar o conjunto de descritores a 0
            FD_SET(0,&rfds); // adicionar o descritor 0 (stdin) ao conjunto 
            FD_SET(data.host_info.fd,&rfds); // adicionar o descritor fd (socket UDP) ao conjunto                 
            data.rfds=&rfds;

            counter=select(maxfd+1,&rfds,(fd_set*)NULL,(fd_set*)NULL,(struct timeval*)NULL);    // espera por um descritor pronto
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
                #ifdef DEBUG
                    printf("User Input\n");
                #endif  
                    user_input(&data);
                    
                }
                if (FD_ISSET(data.host_info.fd,&rfds)){
                #ifdef DEBUG
                    printf("New connection\n");
                #endif
                    add_client(&data);

                    //printf("\n1\n");
                   // usleep(250000);
                    add_adj(&data,1);
                   /// add_adj(&data,2);
                }
            }
        }else{
            printf("Digite:\n");
            
            FD_ZERO(&rfds); // inicializar o conjunto de descritores a 0
            FD_SET(0,&rfds); // adicionar o descritor 0 (stdin) ao conjunto 
            FD_SET(data.host_info.fd,&rfds); // adicionar o descritor fd (socket TCP) ao conjunto     
            FD_SET(data.client_info.fd,&rfds); // adicionar o descritor fd (socket TCP) ao conjunto
            FD_SET(data.predecessor.TCP.fd,&rfds); // adicionar o descritor fd (socket TCP) ao conjunto
            if (data.host_info.fd>data.client_info.fd && data.host_info.fd>data.predecessor.TCP.fd){
                maxfd=data.host_info.fd;
            }else if(data.client_info.fd>data.host_info.fd && data.client_info.fd>data.predecessor.TCP.fd){
                maxfd=data.client_info.fd;
            }
            else{
                maxfd=data.predecessor.TCP.fd;
            }

            int counter=select(maxfd+1,&rfds,(fd_set*)NULL,(fd_set*)NULL,(struct timeval*)NULL);    // espera por um descritor pronto
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
                    int aux=atoi(data.predecessor.ID);
                    add_client(&data);
                    //usleep(250000);
                    if (aux!=atoi(data.sucessor.ID))rmv_adj(&data,aux);
                    add_adj(&data,1);
                    continue;
                }
                if (FD_ISSET(data.client_info.fd,&rfds)){
                #ifdef DEBUG
                    printf("Message received from sucessor\n");
                #endif
                    char buffer[128];
                    char buffer2[128];
                    ssize_t n;
                    n=read(data.client_info.fd,buffer,128);
                    FD_CLR(data.client_info.fd,&rfds);
                    if(n==-1) exit(1);   
                    else if (n==0)   
                    {  
                    #ifdef DEBUG
                        printf("DEBUG: Sucessor disconnected\n");
                    #endif
                        int aux=atoi(data.sucessor.ID);
                        suc_reconnect(&data,buffer);
                        rmv_adj(&data,aux);
                        add_adj(&data,2);
                        //continue;
                    }else if (strstr(buffer,"ENTRY")){
                        int aux=atoi(data.sucessor.ID);
                        add_successor(&data,buffer);
                        //usleep(250000);
                        if (aux!=atoi(data.predecessor.ID))rmv_adj(&data,aux);
                        add_adj(&data,2);
                        //continue;
                    }else if (strstr(buffer,"SUCC")){
                    #ifdef DEBUG
                        printf("DEBUG: Buffer do main: %s\n",buffer);
                    #endif
                        add_successor(&data,buffer);
                        ///usleep(250000);
                        //continue;
                    }else if (strstr(buffer,"CHAT")){
                        //continue;
                    }else if(strstr(buffer,"ROUTE")){
                        chamada_route(&data,buffer);
                        //sprintf(buffer2,"ROUTE %d %d %d",atoi(data.sucessor.ID),atoi(data.sucessor.ID),atoi(data.sucessor.ID));
                        //if (strcmp(buffer2,buffer)==0)
                          //  add_adj(&data,2);
                        //continue;
                    }else if(strstr(buffer,"CHORD")){
                        //continue;
                    }
                                
                }
                if (FD_ISSET(data.predecessor.TCP.fd,&rfds)){
                #ifdef DEBUG
                    printf("Message received from predecessor\n");
                #endif
                    char buffer[128];
                    ssize_t n;
                    n=read(data.predecessor.TCP.fd,buffer,128);
                    FD_CLR(data.predecessor.TCP.fd,&rfds);
                    if(n==-1) exit(1);   
                    
                    else if (n==0)   
                    {  
                    #ifdef DEBUG
                        printf("DEBUG: predecessor disconnected\n");
                    #endif
                        int aux=atoi(data.predecessor.ID);
                        pred_reconnect(&data,buffer);
                        rmv_adj(&data,aux);
                        add_adj(&data,1);
                        //continue;
                    }else if (strstr(buffer,"ENTRY")){
                        int aux=atoi(data.predecessor.ID);
                        add_successor(&data,buffer);
                        //usleep(250000);
                        if (aux !=atoi(data.sucessor.ID))rmv_adj(&data,aux);
                        add_adj(&data,1);
                       // continue;
                    }else if (strstr(buffer,"SUCC")){
                        add_successor(&data,buffer);
                        //usleep(250000);
                       // continue;
                    }else if (strstr(buffer,"CHAT")){
                        //continue;
                    }else if(strstr(buffer,"ROUTE")){
                        chamada_route(&data,buffer);
        
                        //continue;
                    }else if(strstr(buffer,"CHORD")){
                        //continue;
                    }
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
    FD_CLR(0,data->rfds);
    
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

            add_adj(data,1);
            add_adj(data,2);

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
            direct_join(data);

            add_adj(data,1);
            add_adj(data,2);

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
        
    }
    else{
        printf("Input inválido\nPor favor tente novamente\n");
    }
    return;
}