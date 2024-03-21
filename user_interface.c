#include "user_interface.h"


char* join(conect_inf*inicial_inf,char* ring,char* id){
    
    //Adicionar ao anel
    char buffer2[700]={0};

    char temp_id[4]={0};
    char temp_IP[30]={0};
    char temp_TCP[10]={0};
    
    struct sockaddr addr; 
    socklen_t addrlen; 
    ssize_t n;
    char buffer[700]={0};
    char invite[129]={0};
    char id_i[4]={0};

    char* buffer2_chopped;

    struct addrinfo hints, *res;
    int fd, errcode, randn=0;
    srand(time(NULL));
    strcpy(id_i,id);
    int connection_attempts=0;

    fd = socket(AF_INET, SOCK_DGRAM, 0); //UDP socket
    if (fd == -1){
        printf("Erro socket na função Join\n");
        exit(1);
    }
        

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;      //IPv4
    hints.ai_socktype = SOCK_DGRAM; //UDP socket

    errcode = getaddrinfo(inicial_inf->reg_IP, inicial_inf->reg_UDP, &hints, &res);
    if (errcode != 0){
        printf("Erro getaddrinfo na função Join\n");
        exit(1);
    }
        
    
    //Pedir lista de nós ao servidor
    sprintf(invite,"NODES %s",ring);
    n = sendto(fd,invite, strlen(invite), 0, res->ai_addr, res->ai_addrlen);
    if (n == -1) /*error*/{
        printf("Erro sendto na função Join\n");
        free(res);
        close(fd);
        exit(1);
    }
    addrlen=sizeof(addr); n=recvfrom(fd,buffer2,700,0,&addr,&addrlen); 
    if(n==-1){
        printf("Erro recvfrom na função Join\n");
        free(res);
        close(fd);
        exit(1);
    }
#ifdef DEBUG
    printf("Received %ld bytes: %s\n",n , buffer2);
#endif

    if (n==14){ //Caso seja o 10 nó no servidor
    #ifdef DEBUG
        printf("DEBUG: Não existem nós no anel\n");
    #endif
        sprintf(invite,"REG %s %s %s %s",ring,id_i,inicial_inf->IP,inicial_inf->TCP);
        n = sendto(fd,invite, strlen(invite), 0, res->ai_addr, res->ai_addrlen);
        if (n == -1){
            printf("Erro sendto 2 na função Join\n");
            free(res);
            close(fd);
            exit(1);
        }
        addrlen=sizeof(addr); 
        n=recvfrom(fd,buffer,700,0,&addr,&addrlen); 
        if(n==-1){
            printf("Erro recvfrom 2 na função Join\n");
            free(res);
            close(fd);
            exit(1);
        }
        buffer[n] = '\0';
        if (strcmp("OKREG",buffer)!=0){
            printf("Não foi possível contactar com o servidor.\n");
            return NULL;
        }
        else{
            printf("Nó registado com o id %s.\n",id_i);
            strcpy(id,id_i);
            strcpy(inicial_inf->id,id_i);
            strcpy(inicial_inf->predecessor.ID,id);
            strcpy(inicial_inf->sucessor.ID,id);
            strcpy(inicial_inf->sucessor.IP,inicial_inf->IP);
            strcpy(inicial_inf->sucessor.PORT,inicial_inf->TCP);
            strcpy(inicial_inf->secsuccessor.ID,id);
            strcpy(inicial_inf->secsuccessor.IP,inicial_inf->IP);
            strcpy(inicial_inf->secsuccessor.PORT,inicial_inf->TCP);
            init_tabs(inicial_inf);
        }
    }else{
    #ifdef DEBUG
        printf("DEBUG: Existem nós no anel\n");
    #endif 
        buffer2_chopped=buffer2+14;

        char* rest;
        char* token;
        char strs[16][40];
        token=strtok_r(buffer2_chopped,"\n",&rest);
        int nlinhas=0;
        do{
#ifdef DEBUG
            printf("token: %s %d\n",token,sscanf(token,"%s %s %s",temp_id,temp_IP,temp_TCP));
#endif      
            strcpy(strs[nlinhas],token);
            nlinhas++;

            token=strtok_r(rest,"\n",&rest);
        }while (token!=NULL);

        srand(time(NULL));
        randn=rand()%nlinhas;
        sscanf(strs[randn],"%s %s %s",temp_id,temp_IP,temp_TCP);
        printf("No escolhido como sucessor: %s\n",temp_id);


        while (1){
            sprintf(invite,"REG %s %s %s %s",ring,id_i,inicial_inf->IP,inicial_inf->TCP);
            n = sendto(fd,invite, strlen(invite), 0, res->ai_addr, res->ai_addrlen);
            if (n == -1){
                printf("Erro sendto 3 na função Join\n");
                free(res);
                close(fd);
                exit(1);
            }
            
            addrlen=sizeof(addr); n=recvfrom(fd,buffer,700,0,&addr,&addrlen); if(n==-1)/*error*/exit(1);
            buffer[n] = '\0';
            if (strcmp("OKREG",buffer)!=0){
                randn=rand()%100;
                if (randn<10) sprintf(id_i,"0%d",randn);
                else sprintf(id_i,"%d",randn);
                connection_attempts++;
                if (connection_attempts==16){
                    printf("Não foi possível contactar com o servidor.\n");
                    return NULL;
                }
            }
            else{
                break;
            }
        }
        if (strcmp(id,id_i)!=0) printf("Já existia um nó registado com o id %s\n",id);
        printf("Nó registado com o id %s.\n",id_i);
        printf("nó Registado em servidor que não está vazio\n");
        strcpy(inicial_inf->id,id_i);
        strcpy(inicial_inf->sucessor.ID,temp_id);
        strcpy(inicial_inf->sucessor.IP,temp_IP);
        strcpy(inicial_inf->sucessor.PORT,temp_TCP);
        inicial_inf->joining=direct_join(inicial_inf);
    }

    freeaddrinfo(res);
    close(fd);
    strcpy(id,id_i);
    return id;
}

int leave(conect_inf*inicial_inf,char* ring,char* id){
    struct sockaddr addr; 
    socklen_t addrlen; 
    ssize_t n;
    char buffer[700]={0};
    char invite[129]={0};
    struct addrinfo hints, *res;
    int fd, errcode;
    fd = socket(AF_INET, SOCK_DGRAM, 0); //UDP socket
    if (fd == -1)                        /*error*/
        exit(1);

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;      //IPv4
    hints.ai_socktype = SOCK_DGRAM; //UDP socket

    errcode = getaddrinfo(inicial_inf->reg_IP, inicial_inf->reg_UDP, &hints, &res);
    if (errcode != 0) /*error*/
        exit(1);

    sprintf(invite,"UNREG %s %s",ring,id);
    n = sendto(fd,invite, strlen(invite), 0, res->ai_addr, res->ai_addrlen);
    if (n == -1) /*error*/
        exit(1);
    freeaddrinfo(res);
    addrlen=sizeof(addr); n=recvfrom(fd,buffer,700,0,&addr,&addrlen); if(n==-1)/*error*/exit(1);
    buffer[n] = '\0';
    if (strcmp("OKUNREG",buffer)==0){
        return 0;
    }
    else{
        return (-1);
    }
    close(fd);

}

void create_TCP_server(conect_inf* data){
    int errcode;
    ssize_t n;


    data->host_info.fd=socket(AF_INET,SOCK_STREAM,0); //TCP socket  //SOCKET ONDE SE RECEBEM PEDIDOS DE CONEXÃO
    if (data->host_info.fd==-1) exit(1); //error


    memset(&data->host_info.hints,0,sizeof(data->host_info.hints));
    data->host_info.hints.ai_family=AF_INET; //IPv4
    data->host_info.hints.ai_socktype=SOCK_STREAM; //TCP socket
    data->host_info.hints.ai_flags=AI_PASSIVE;


    errcode=getaddrinfo(NULL,data->TCP,&data->host_info.hints,&data->host_info.res);
    if((errcode)!=0)/*error*/exit(1);
    
    
    
    
    n=bind(data->host_info.fd,data->host_info.res->ai_addr,data->host_info.res->ai_addrlen);
    if(n==-1) /*error*/ exit(1);


    if(listen(data->host_info.fd,5)==-1)/*error*/exit(1);
    return;
}


int direct_join(conect_inf* data){
    char input[300]={0};
    int errcode;
    ssize_t n;
    
    init_tabs(data);

    //Iniciar ligação TCP como cliente do nó a que me quero juntar através do seu endereço IP e porta TCP que foi dada como input
    data->client_info.fd=socket(AF_INET,SOCK_STREAM,0); //TCP socket    //SOCKET ONDE SE RECEBEM E ENVIAM MENSAGENS NESTE NÓ
    if (data->client_info.fd==-1) exit(1); //error

    memset(&data->client_info.hints,0,sizeof data->client_info.hints);
    data->client_info.hints.ai_family=AF_INET; //IPv4
    data->client_info.hints.ai_socktype=SOCK_STREAM; //TCP socket


    
    errcode=getaddrinfo(data->sucessor.IP,data->sucessor.PORT,&data->client_info.hints,&data->client_info.res);
    if(errcode!=0)/*error*/exit(1);


    n=connect(data->client_info.fd,data->client_info.res->ai_addr,data->client_info.res->ai_addrlen);
    if(n==-1)/*error*/exit(1);

    //Enviar mensagem de protocolo que contém a informação do nó que se quer juntar (ENTRY i i.IP i.TCP\n)
    sprintf(input,"ENTRY %s %s %s\n",data->id,data->IP,data->TCP);    
    n=write(data->client_info.fd,input,strlen(input)+1);
    if(n==-1)/*error*/exit(1);

#ifdef DEBUG
    printf("DEBUG: Enviado para o sucessor %s: %s",data->sucessor.ID,input);
#endif
    return 1;
}



int add_client(conect_inf* data, char* buffer, int futurefd){
    //int errcode;
    ssize_t n;
    char input[300]={0};

    //ADICIONAR VERIFICACAO PARA SABER SE TENHO ID
    char send_buffer[256]={0};

    char tmpid[10]={0};
    char tmpIP[30]={0};
    char tmpTCP[10]={0};
    

    if(strstr(buffer,"ENTRY ")!=NULL){   //se a msg recebida for a esperada

        if(strcmp(data->id,data->predecessor.ID)==0){ //se não existir: 

            sscanf(buffer,"%*s %s %s %s",data->sucessor.ID,data->sucessor.IP,data->sucessor.PORT);   //atualizar informação do sucessor com a informação do novo nó

            sprintf(input,"SUCC %s %s %s\n",data->sucessor.ID,data->sucessor.IP,data->sucessor.PORT);   //confirmar ao novo nó que sou o seu sucessor e enviar msg com info do meu sucessor (SUCC k k.IP k.TCP\n), que é o 2o sucessor do nó a entrar
            n=write(futurefd,input,strlen(input)+1); 
            if(n==-1) exit(1); //error 
#ifdef DEBUG      
            printf("DEBUG: Enviada mensagem de protocolo para futurefd no caso em que só há 1 nó: %s\n",input);   //mostrar msg enviada (SUCC k k.IP k.TCP\n)
#endif
            //Atualizar informação de predecessor
            data->predecessor.TCP.fd=futurefd;
            strcpy(data->predecessor.ID,data->sucessor.ID);
#ifdef DEBUG
            printf("DEBUG: Atualizado socket do predecessor e definido o seu novo ID para %s\n",data->predecessor.ID);
#endif
            init_pred(data);
            
            return 3;



        }else{   //se existir: enviar msg com informação do novo nó ao meu predecessor, para que este o possa adicionar como seu sucessor
            
            sscanf(buffer,"%*s %s %s %s",tmpid,tmpIP,tmpTCP);

            sprintf(input,"ENTRY %s %s %s\n",tmpid,tmpIP,tmpTCP);   //enviar msg com informação do novo nó ao meu predecessor (ENTRY i i.IP i.TCP\n)

            n=write(data->predecessor.TCP.fd,input,strlen(input)+1); 
            if(n==-1) exit(1); //error    
#ifdef DEBUG
            printf("Enviado ao meu predecessor: %s\n",input);   //mostrar msg enviada (SUCC k k.IP k.TCP\n)
#endif
            //atualizar predecessor
            data->predecessor.TCP.fd=futurefd;
            strcpy(data->predecessor.ID,tmpid);
            strcpy(data->predecessor.IP,tmpIP);
            strcpy(data->predecessor.PORT,tmpTCP);


            sprintf(send_buffer,"SUCC %s %s %s\n",data->sucessor.ID,data->sucessor.IP,data->sucessor.PORT);   //enviar msg com informação do novo nó ao meu predecessor (ENTRY i i.IP i.TCP\n)

            n=write(data->predecessor.TCP.fd,send_buffer,strlen(input)+1); 
            if(n==-1) exit(1); //error   
#ifdef DEBUG
            printf("Enviado ao nó %s que quer entrar a mensagem: %s\n",data->predecessor.ID,send_buffer);   //mostrar msg enviada (SUCC k k.IP k.TCP\n)
#endif
            return 1;
        }
    }else if(strstr(buffer,"PRED ")!=NULL){

        sscanf(buffer,"%*s %s",data->predecessor.ID);
        data->predecessor.TCP.fd=futurefd;
#ifdef DEBUG
        printf("DEBUG: Atualizado socket do predecessor e definido o seu novo ID para %s\n",data->predecessor.ID);
#endif  
        
        add_adj(data,1);
        add_adj(data,2);
        
        return 2;

    }else if(strstr(buffer,"CHORD ")!=NULL){
        int i=0;
        for (i = 0; i < 14; i++)
        {
            if(data->rcv_chords[i]->fd==-1){
                break;
            }
        }
        
        sscanf(buffer,"%*s %s",data->rcv_chords[i]->ID);
        data->rcv_chords[i]->fd=futurefd;
//#ifdef DEBUG
        printf("Adicionado nó %s como corda\n",data->rcv_chords[i]->ID);
//#endif    
        //add_adj(data,3);
        return 4;
    }else{   //se não for a msg esperada
        printf("Connection attempt declined\n");
        close(futurefd);
        return -1;
    }
}

void rmv(conect_inf*inicial_inf,char *node_id){
    struct sockaddr addr; 
    socklen_t addrlen; 
    ssize_t n;
    char buffer[700]={0};
    char invite[129]={0};
    struct addrinfo hints, *res;
    int fd, errcode;
    fd = socket(AF_INET, SOCK_DGRAM, 0); //UDP socket
    if (fd == -1)                        /*error*/
        exit(1);

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;      //IPv4
    hints.ai_socktype = SOCK_DGRAM; //UDP socket

    errcode = getaddrinfo(inicial_inf->reg_IP, inicial_inf->reg_UDP, &hints, &res);
    if (errcode != 0) /*error*/
        exit(1);

    sprintf(invite,"UNREG 051 %s",node_id);
    n = sendto(fd,invite, strlen(invite), 0, res->ai_addr, res->ai_addrlen);
    if (n == -1) /*error*/{
        exit(1);
    }
    freeaddrinfo(res);
    addrlen=sizeof(addr); n=recvfrom(fd,buffer,700,0,&addr,&addrlen); if(n==-1)/*error*/exit(1);
    //buffer[n] = '\0';
#ifdef DEBUG
    printf("Received: %s\n", buffer);
#endif
    if (strcmp("OKUNREG\n",buffer)!=0){
        printf("No retirado\n");
    }
    else{
        printf("No n retirado\n");
    }
    close(fd); 
    
    return;

    
}

void add_successor(conect_inf* data,char *buffer){

    ssize_t n;
    //Ler mensagem do meu sucessor que contém a informação do meu futuro sucessor (SUCC k k.IP k.TCP\n)
#ifdef DEBUG
    printf("DEBUG: Recebido do meu sucessor %s a mensagem: %s\n",data->sucessor.ID, buffer);   //mostrar msg recebida (SUCC i i.IP i.TCP\n)
#endif

    char tmpid[10];
    char tmpIP[30];
    char tmpTCP[10];

    //extrair parte da mensagem para comparar ("SUCC") e confirmar que é a mensagem esperada
    char resp_cmp[128]={0};
    sprintf(resp_cmp,"%s",buffer);
    sscanf(buffer,"%s %s %s %s",resp_cmp,tmpid,tmpIP,tmpTCP);
    if(strcmp(resp_cmp,"ENTRY")==0){


            //colocar o novo nó como sucessor e colocar o meu sucessor como segundo sucessor e fechar socket com o meu sucessor
            strcpy(data->secsuccessor.ID,data->sucessor.ID);
            strcpy(data->secsuccessor.IP,data->sucessor.IP);
            strcpy(data->secsuccessor.PORT,data->sucessor.PORT);
            sscanf(buffer,"%*s %s %s %s",data->sucessor.ID,data->sucessor.IP,data->sucessor.PORT);
            
            close(data->client_info.fd);
            data->client_info.fd=-1;

#ifdef DEBUG
            printf("DEBUG: Atualizado o meu sucessor para %s e o meu segundo sucessor para %s e fechado socket com este útimo\n",data->sucessor.ID,data->secsuccessor.ID);
#endif

            
            char input[300]={0};
            sprintf(input,"SUCC %s %s %s\n",data->sucessor.ID,data->sucessor.IP,data->sucessor.PORT);   //enviar msg com informação do novo nó ao meu predecessor (ENTRY i i.IP i.TCP\n)

            n=write(data->predecessor.TCP.fd,input,strlen(input)+1); 
            if(n==-1) exit(1); //error    
#ifdef DEBUG
            printf("DEBUG: Enviado ao meu predecessor com id %s a mensagem: %s",data->predecessor.ID,input);   //mostrar msg enviada (SUCC k k.IP k.TCP\n)
#endif

            //iniciar socket TCP para ser cliente do meu novo sucessor
            int errcode;
            data->client_info.fd=socket(AF_INET,SOCK_STREAM,0); //TCP socket
            if (data->client_info.fd==-1) exit(1); //error

            memset(&data->client_info.hints,0,sizeof data->client_info.hints);
            data->client_info.hints.ai_family=AF_INET; //IPv4
            data->client_info.hints.ai_socktype=SOCK_STREAM; //TCP socket

            
            errcode=getaddrinfo(data->sucessor.IP,data->sucessor.PORT,&data->client_info.hints,&data->client_info.res);
            if(errcode!=0) exit(1); //error
            n=connect(data->client_info.fd,data->client_info.res->ai_addr,data->client_info.res->ai_addrlen);
            if(n==-1) exit(1);  //error

           
            sprintf(input,"PRED %s\n",data->id);
            n=write(data->client_info.fd,input,strlen(input)+1);
            if(n==-1)exit(1);  //error

#ifdef DEBUG
            printf("DEBUG: Aberto socket com o meu novo sucessor, enviada a mensagem '%s' para o id %s\n",input,data->sucessor.ID);    
#endif

            return;
    }else if (strcmp(resp_cmp,"SUCC")==0){  //Caso receba a mensagem SUCC do meu sucessor, tenho de atualizar o meu segundo sucessor
#ifdef DEBUG
        printf("DEBUG: Recebida a mensagem %s e por isso o nó da mensagem foi colocado como segundo sucessor\n", buffer);
#endif
        sscanf(buffer,"%*s %s %s %s",data->secsuccessor.ID,data->secsuccessor.IP,data->secsuccessor.PORT);

    }
    else{
        printf("Connection attempt declined\n");
        return;
    }
    return;

}

int leave_ring(conect_inf* data){
    //quando um nó sai do anel:
    //2. Fechar conexão com o seu sucessor
    close(data->client_info.fd);
    data->client_info.fd=-1;
    //3. Fechar conexão com o seu predecessor
    close(data->predecessor.TCP.fd);
    data->predecessor.TCP.fd=-1;
    //fechar com as cordas
    if (data->chord.TCP.fd!=-1)
    {
        close(data->chord.TCP.fd);
        data->chord.TCP.fd=-1;
        data->chord.ID[0]='\0';
        data->chord.IP[0]='\0';
        data->chord.PORT[0]='\0';
    }

    
    for (int i = 0; i < 14; i++)
    {
        if(data->rcv_chords[i]->fd!=-1){
            close(data->rcv_chords[i]->fd);
            data->rcv_chords[i]->fd=-1;
            data->rcv_chords[i]->ID[0]='\0';
        }
    }
    free(data->client_info.res);
    //4. Resetar a sua informação
    data->secsuccessor.ID[0]='\0';
    data->secsuccessor.IP[0]='\0';
    data->secsuccessor.PORT[0]='\0';
    data->sucessor.ID[0]='\0';
    data->sucessor.IP[0]='\0';
    data->sucessor.PORT[0]='\0';
    data->predecessor.ID[0]='\0';
    data->id[0]='\0';
#ifdef DEBUG
    printf("DEBUG: Fechadas as comunicações com sucessor e predecessor e resetados todos os parâmetros");
#endif
    init_tabs(data);
    return 1;
}


void pred_reconnect(conect_inf* data,char *buffer){

    char send_buffer[256]={0};

    if (strcmp(data->id,data->secsuccessor.ID)==0)
    {
    #ifdef DEBUG
        printf("DEBUG: Saiu um dos 2 nós do anel. A colocar-me como nó único\n");
    #endif
        close(data->client_info.fd);
        data->client_info.fd=-1;
        close(data->predecessor.TCP.fd);
        data->predecessor.TCP.fd=-1;
        strcpy(data->sucessor.ID,data->id);
        strcpy(data->sucessor.IP,data->IP);
        strcpy(data->sucessor.PORT,data->TCP);
        strcpy(data->secsuccessor.ID,data->id);
        strcpy(data->secsuccessor.IP,data->IP);
        strcpy(data->secsuccessor.PORT,data->TCP);
        strcpy(data->predecessor.ID,data->id);
        return;
    }


    ssize_t n;
    int futurefd;

    //esperar por conexão do meu futuro predecessor
    data->host_info.addrlen=sizeof(data->host_info.addr);
    if((futurefd=accept(data->host_info.fd,(struct sockaddr*) &data->host_info.addr,&data->host_info.addrlen))==-1)/*error*/ exit(1);
    
    n=read(futurefd,buffer,128);   //receber msg com informação dele (PRED \n)
    if(n==-1) exit(1);   //error
#ifdef DEBUG
    printf("DEBUG: Pedido de conexão recebido em futurefd do nó %s com a mensagem: %s\n", data->id, buffer);   //mostrar msg recebida (ENTRY i i.IP i.TCP\n)
#endif    


    if(strstr(buffer,"PRED")!=NULL){   //se a msg recebida for a esperada
        //atualizar informação do predecessor
        sscanf(buffer,"%*s %s",data->predecessor.ID);
        data->predecessor.TCP.fd=futurefd;
        //enviar ao meu predecessor o id do meu sucessor
        sprintf(send_buffer,"SUCC %s %s %s\n",data->sucessor.ID,data->sucessor.IP,data->sucessor.PORT);   //enviar msg com informação do novo nó ao meu predecessor (ENTRY i i.IP i.TCP\n)
        n=write(data->predecessor.TCP.fd,send_buffer,strlen(send_buffer)+1);
        if(n==-1) exit(1); //error
    #ifdef DEBUG
        printf("DEBUG: Enviado ao meu predecessor com id %s a mensagem: %s e atualizado o meu predecessor",data->predecessor.ID,send_buffer);   //mostrar msg enviada (SUCC k k.IP k.TCP\n)
    #endif
        return;
    }
    else{   //se não for a msg esperada
        printf("Connection attempt declined\n");
        close(futurefd);
        return ;
    }
    return;
}

void suc_reconnect(conect_inf* data,char *buffer){
    ssize_t n;
    int errcode;  
    char send_buffer[256]={0};
    
    if (strcmp(data->id,data->secsuccessor.ID)==0)
    {
    #ifdef DEBUG
        printf("DEBUG: Saiu um dos 2 nós do anel. A colocar-me como nó único\n");
    #endif
        close(data->client_info.fd);
        data->client_info.fd=-1;
        close(data->predecessor.TCP.fd);
        data->predecessor.TCP.fd=-1;
        strcpy(data->sucessor.ID,data->id);
        strcpy(data->sucessor.IP,data->IP);
        strcpy(data->sucessor.PORT,data->TCP);
        strcpy(data->secsuccessor.ID,data->id);
        strcpy(data->secsuccessor.IP,data->IP);
        strcpy(data->secsuccessor.PORT,data->TCP);
        strcpy(data->predecessor.ID,data->id);
        return;
    }
    


    //colocar o meu segundo sucessor como meu sucessor
    strcpy(data->sucessor.ID,data->secsuccessor.ID);
    strcpy(data->sucessor.IP,data->secsuccessor.IP);
    strcpy(data->sucessor.PORT,data->secsuccessor.PORT);

    //fechar socket com o meu sucessor antigo
    close(data->client_info.fd);
    data->client_info.fd=-1;
    //iniciar socket TCP para ser cliente do meu novo sucessor 
    
    data->client_info.fd=socket(AF_INET,SOCK_STREAM,0); //TCP socket
    if (data->client_info.fd==-1) exit(1); //error

    memset(&data->client_info.hints,0,sizeof data->client_info.hints);
    data->client_info.hints.ai_family=AF_INET; //IPv4
    data->client_info.hints.ai_socktype=SOCK_STREAM; //TCP socket

    
    errcode=getaddrinfo(data->sucessor.IP,data->sucessor.PORT,&data->client_info.hints,&data->client_info.res);
    if(errcode!=0) exit(1); //error
    n=connect(data->client_info.fd,data->client_info.res->ai_addr,data->client_info.res->ai_addrlen);
    if(n==-1) exit(1);  //error

    //enviar mensagem de protocolo ao meu novo sucessor (PRED i\n)
    char input[300];
    sprintf(input,"PRED %s\n",data->id);
    n=write(data->client_info.fd,input,strlen(input)+1);
    if(n==-1)exit(1);  //error
#ifdef DEBUG
    printf("DEBUG: Enviado para o meu novo sucessor: %s",input);
#endif


    //enviar mensagem de protocolo ao meu predecessor (SUCC k k.IP k.TCP\n)
    sprintf(send_buffer,"SUCC %s %s %s\n",data->sucessor.ID,data->sucessor.IP,data->sucessor.PORT);   //enviar msg com informação do novo nó ao meu predecessor (ENTRY i i.IP i.TCP\n)

    n=write(data->predecessor.TCP.fd,send_buffer,strlen(send_buffer)+1); 
    if(n==-1) exit(1); //error    
#ifdef DEBUG
    printf("DEBUG: Enviado ao meu predecessor com id %s a mensagem: %s",data->predecessor.ID,send_buffer);   //mostrar msg enviada (SUCC k k.IP k.TCP\n)
#endif
    return;
    char rdbuffer[256]={0};
    //esperar para receber a informação do meu segundo sucessor
    n=read(data->client_info.fd,rdbuffer,128);
    if(n==-1) exit(1);   //error
    sscanf(rdbuffer,"%*s %s %s %s",data->secsuccessor.ID,data->secsuccessor.IP,data->secsuccessor.PORT);
#ifdef DEBUG
    printf("DEBUG: Recebido do meu novo sucessor %s a mensagem: %s\n",data->sucessor.ID, buffer);   //mostrar msg recebida (SUCC i i.IP i.TCP\n)
#endif
    return;
}
void alloc_tabs(conect_inf* data){
    for (int i=0;i<=99;i++){
        for (int j=0;j<=99;j++) {                         ///alocar tabelas
            data->tb_encaminhamento[i][j]= (char *)calloc(50, sizeof(char));
            if (data->tb_encaminhamento[i][j] == NULL) {
            fprintf(stderr, "Erro na alocação de memória\n");
            exit (1);
            }
        }
    }
    for (int i=0;i<=99;i++){               ///alocar tabelas
            data->tb_caminhos_curtos[i]= (char *)calloc(50, sizeof(char));
            if (data->tb_caminhos_curtos[i] == NULL) {
            fprintf(stderr, "Erro na alocação de memória\n");
            exit (1);
            }
    }
}
void free_tabs(conect_inf* data){
    for (int i=0;i<=99;i++){
        for (int j=0;j<=99;j++) {                         ///alocar tabelas
            free(data->tb_encaminhamento[i][j]);
            }
        }
    
    for (int i=0;i<=99;i++){               ///alocar tabelas
            free(data->tb_caminhos_curtos[i]);    
    }
}

void init_tabs(conect_inf* data){
    for (int i=0;i<=99;i++){
        for (int j=0;j<=99;j++)                          ///inicia as tabelas a - (nulo)
            sprintf(data->tb_encaminhamento[i][j],"-");
    }
    for (int i=0;i<=99;i++){
            sprintf(data->tb_caminhos_curtos[i],"-");
    }
    for (int i=0;i<=99;i++){
            sprintf(data->tb_exped[i],"-");
    }
    sprintf(data->tb_caminhos_curtos[atoi(data->id)],"%s",data->id);   /// deternima-se a si próprio como caminho masis curto
#ifdef DEBUG
    printf("TAMANH0: %ld\n",strlen(data->tb_caminhos_curtos[atoi(data->id)]));
#endif
    return;
}

void add_adj(conect_inf*data,int pos){
    char adj[10];
    int i=0,n,fd;
#ifdef DEBUG
    printf("entrou no add_adj\n");
#endif
    if (strcmp(data->id,data->sucessor.ID)==0)
    {
#ifdef DEBUG
        printf("entrou no return\n");
#endif
        return;
    }

    if (pos==1){ 
        strcpy(adj,data->predecessor.ID);   ///determina se a adajacencia foi de um predecessor sucessor ou corda
        fd=data->predecessor.TCP.fd;
    }
    else if (pos==2){ 
        strcpy(adj,data->sucessor.ID);
        fd=data->client_info.fd;
    }
    else if (pos==3) { 
        strcpy(adj,data->chord.ID);
        fd=data->chord.TCP.fd;
    }
    else{
        int j=0;
        for (j=0;data->rcv_chords[j]->fd!=-1;j++);
        j--;
        printf("corda com id %s\n",data->rcv_chords[j]->ID);
        strcpy(adj,data->rcv_chords[j]->ID);
        fd=data->rcv_chords[j]->fd;
    }
     
    
    for (i=0;i<=99;i++){
        if(data->tb_caminhos_curtos[i][0]!='-'){
            char buffer2[256]={0};
            if (i<10)
            {
                sprintf(buffer2,"ROUTE %s 0%d %s\n",data->id,i, data->tb_caminhos_curtos[i]); 
                n=write(fd,buffer2,strlen(buffer2)); 
                if(n==-1) exit(1); //error    
            }else{
                sprintf(buffer2,"ROUTE %s %d %s\n",data->id,i, data->tb_caminhos_curtos[i]); 
                n=write(fd,buffer2,strlen(buffer2)); 
                if(n==-1) exit(1); //error    
            }
            
            
            #ifdef DEBUG
                printf("DEBUG: Enviado ao meu adjacente %d com id %s a mensagem: %s\n",pos,adj,buffer2);   //mostrar msg enviada (SUCC k k.IP k.TCP\n)

            #endif
        }
    }
    return;
}

void rmv_adj(conect_inf*data,char* adj){
#ifdef DEBUG
    printf("entrou no rmv_adj para tirar o %s\n",adj);
#endif
    char buffer[256]={0};
    sprintf(buffer,"%s-%s",data->id,adj); // verifca se havia algum caminho mais curto a passar pela adjacencia removida
    for(int i=0;i<=99;i++){
        strcpy(data->tb_encaminhamento[i][atoi(adj)],"-");
        if (strstr(data->tb_caminhos_curtos[i],buffer)!=NULL ){                 /// tem um caminho que não ele proprio
#ifdef DEBUG
            printf("entrou no if e tem na tabela de encaminhamento:%s\n",data->tb_encaminhamento[i][atoi(adj)]);
#endif 
            char i_str[10];
            if (i<10){
                sprintf(i_str,"0%d",i);
            }
            else{
                sprintf(i_str,"%d",i);
            }
            refresh_caminho_mais_curto_sem_encaminhamento(data,i_str);
            
        }
    }
}

void disconect_adj(conect_inf*data,char* adj,char*new_adj){
#ifdef DEBUG
    printf("entrou no disconect_adj %s\n",adj);
#endif
    char buffer[256]={0};
    sprintf(buffer,"%s-%s",data->id,adj); // verifca se havia algum caminho mais curto a passar pela adjacencia removida
    for(int i=0;i<=99;i++){
        strcpy(data->tb_encaminhamento[i][atoi(adj)],"-");
        if (strstr(data->tb_caminhos_curtos[i],buffer)!=NULL && atoi(adj)!=i){                 /// tem um caminho que não ele proprio
#ifdef DEBUG
            printf("entrou no if e tem na tabela de encaminhamento:%s\n",data->tb_encaminhamento[i][atoi(adj)]);
#endif
            
            char i_str[10];
            if (i<10){
                sprintf(i_str,"0%d",i);
            }
            else{
                sprintf(i_str,"%d",i);
            }
         
            if(strlen(data->tb_encaminhamento[i][atoi(data->sucessor.ID)])>1 || strlen(data->tb_encaminhamento[i][atoi(data->predecessor.ID)])>1){
                refresh_caminho_mais_curto(data,i_str);
            }
            else if (data->chord.TCP.fd!=-1){
                if(strlen(data->tb_encaminhamento[i][atoi(data->chord.ID)])>1)
                refresh_caminho_mais_curto(data,i_str);
                for(int k=0;data->rcv_chords[k]->fd!=-1;k++){
                    if(strlen(data->tb_encaminhamento[i][atoi(data->rcv_chords[k]->ID)])>1){
                        refresh_caminho_mais_curto(data,i_str);
                        k=12;
                    }
                }
            }
            else{
                for(int k=0;data->rcv_chords[k]->fd!=-1;k++){
                    if(strlen(data->tb_encaminhamento[i][atoi(data->rcv_chords[k]->ID)])>1){
                        refresh_caminho_mais_curto(data,i_str);
                        k=12;
                    }
                }
            }

        }
        if(atoi(adj)==i){
#ifdef DEBUG
            printf("entrou no segundo if/n");
#endif
            for (int j=0;j<=99;j++){
                strcpy(data->tb_encaminhamento[atoi(adj)][j],"-");
            }
            refresh_caminho_mais_curto(data,adj);
        }
    }
}

void refresh_caminho_mais_curto(conect_inf*data,char* linha){
    int tam_caminho,menor=-1,tam_menor=200,n,n_caminhos_menores=0;
    char buffer[256]={0};
    if (strcmp(data->id,linha)==0){
        return;
    }
    for (int i=0;i<=99;i++){
        tam_caminho=contar_nos_no_caminho(data->tb_encaminhamento[atoi(linha)][i]);
        if (tam_caminho>0 && tam_caminho<tam_menor){
            tam_menor=tam_caminho;
            n_caminhos_menores=1;
            menor=i;
        }
        else if(tam_caminho>0 && tam_caminho==tam_menor)
            n_caminhos_menores++;
    }

    if (menor==-1){
        strcpy(data->tb_caminhos_curtos[atoi(linha)],"-");
        strcpy(data->tb_exped[atoi(linha)],"-");
        sprintf(buffer,"ROUTE %s %s\n",data->id,linha);
#ifdef DEBUG
        printf("%s\n",buffer);
#endif
    }
    else if(contar_nos_no_caminho(data->tb_caminhos_curtos[atoi(linha)])==tam_menor && n_caminhos_menores>1){
         return;
    }
    else if (contar_nos_no_caminho(data->tb_caminhos_curtos[atoi(linha)])==tam_menor && strcmp(data->tb_caminhos_curtos[atoi(linha)],data->tb_encaminhamento[atoi(linha)][menor])==0)
        return;
    else{
        strcpy(data->tb_caminhos_curtos[atoi(linha)],data->tb_encaminhamento[atoi(linha)][menor]);
        if (tam_menor==2){
            sscanf(data->tb_caminhos_curtos[atoi(linha)],"%*d-%s",data->tb_exped[atoi(linha)]); 
        }
        else{
            data->tb_exped[atoi(linha)][0]=data->tb_caminhos_curtos[atoi(linha)][3];
            data->tb_exped[atoi(linha)][1]=data->tb_caminhos_curtos[atoi(linha)][4];
            data->tb_exped[atoi(linha)][2]='\0';
        }
        sprintf(buffer,"ROUTE %s %s %s\n",data->id,linha,data->tb_caminhos_curtos[atoi(linha)]);
    #ifdef DEBUG
        printf("DEBUG: Atualizado caminho mais curto\n");
    #endif
    }

    if((strcmp(data->sucessor.ID,data->id)!=0) && (data->client_info.fd!=-1)){
        n=write(data->client_info.fd,buffer,strlen(buffer)); 
        if(n==-1) exit(1); //error    
        #ifdef DEBUG
            printf("DEBUG: Enviado ao meu adjacente (sucessor) com id %s a mensagem: %s\n",data->sucessor.ID,buffer);   //mostrar msg enviada (SUCC k k.IP k.TCP\n)
        #endif
    }

    if((strcmp(data->predecessor.ID,data->id)!=0) && (data->predecessor.TCP.fd!=-1)){
        n=write(data->predecessor.TCP.fd,buffer,strlen(buffer)); 
        if(n==-1) exit(1); //error    
        #ifdef DEBUG
            printf("DEBUG: Enviado ao meu adjacente (predecessor) com id %s a mensagem: %s\n",data->predecessor.ID,buffer);   //mostrar msg enviada (SUCC k k.IP k.TCP\n)
        #endif
    }
    if (data->chord.TCP.fd!=-1){
        n=write(data->chord.TCP.fd,buffer,strlen(buffer)); 
        if(n==-1) exit(1); //error    
        #ifdef DEBUG
            printf("DEBUG: Enviado ao meu adjacente (corda aberta por mim) com id %s a mensagem: %s\n",data->chord.ID,buffer);   //mostrar msg enviada (SUCC k k.IP k.TCP\n)
        #endif
    }
    for (int i=0;data->rcv_chords[i]->fd!=-1;i++){
        n=write(data->rcv_chords[i]->fd,buffer,strlen(buffer)); 
        if(n==-1) exit(1); //error    
        #ifdef DEBUG
            printf("DEBUG: Enviado ao meu adjacente (corda recebida) com id %s a mensagem: %s\n",data->rcv_chords[i]->ID,buffer);   //mostrar msg enviada (SUCC k k.IP k.TCP\n)
        #endif
    }
    return;
}

void refresh_caminho_mais_curto_sem_encaminhamento(conect_inf*data,char* linha){
    int tam_caminho,menor=-1,tam_menor=200;
    if (strcmp(data->id,linha)==0){
        return;
    }
    for (int i=0;i<=99;i++){
        tam_caminho=contar_nos_no_caminho(data->tb_encaminhamento[atoi(linha)][i]);
        if (tam_caminho>0 && tam_caminho<tam_menor){
            tam_menor=tam_caminho;
            menor=i;
        }
    }

    if (menor==-1){
        strcpy(data->tb_caminhos_curtos[atoi(linha)],"-");
        strcpy(data->tb_exped[atoi(linha)],"-");
    }
    else if(contar_nos_no_caminho(data->tb_caminhos_curtos[atoi(linha)])==tam_menor){
        return;
    }
    else{
        strcpy(data->tb_caminhos_curtos[atoi(linha)],data->tb_encaminhamento[atoi(linha)][menor]);
        if (tam_menor==2){
            sscanf(data->tb_caminhos_curtos[atoi(linha)],"%*d-%s",data->tb_exped[atoi(linha)]); 
        }
        else{
            data->tb_exped[atoi(linha)][0]=data->tb_caminhos_curtos[atoi(linha)][3];
            data->tb_exped[atoi(linha)][1]=data->tb_caminhos_curtos[atoi(linha)][4];
            data->tb_exped[atoi(linha)][2]='\0';
        }
    #ifdef DEBUG
        printf("DEBUG: Atualizado caminho mais curto\n");
    #endif
    }
    return;
}

void chamada_route(conect_inf*data,char*mensagem){
    int partida, destino;
    char partida_str[10]={0};
    char destino_str[10]={0};
    char id[11]={0};
    char buffer[256]={0};
    char sequencia_com_tracos[200]={0};

#ifdef DEBUG
    printf("DEBUG: Recebida a mensagem: %s na funcao da chamada route\n",mensagem);
#endif
    
    int num_converted = sscanf(mensagem, "ROUTE %s %s %s\n", partida_str, destino_str, sequencia_com_tracos);
    partida=atoi(partida_str);
    destino=atoi(destino_str);
    sprintf(id,"-%s",data->id);
    //Verificar se trouxe caminho na mensagem ou saiu um nó
    if (num_converted == 3) {   //se trouxer
        //ver se o caminho é válido
        if (strstr(sequencia_com_tracos,id)!=NULL){   
            strcpy(data->tb_encaminhamento[destino][partida],"-");
            refresh_caminho_mais_curto(data,destino_str);   
            return;
        }
        else{   //atualizar tabela de encaminhamento e vai extrair o novo caminho mais curto
            sprintf(buffer,"%s-%s",data->id,sequencia_com_tracos);
            strcpy(data->tb_encaminhamento[destino][partida],buffer);
            refresh_caminho_mais_curto(data,destino_str);
        }
    } else {    //se não trouxer é saída de um nó
        if (strcmp(data->tb_caminhos_curtos[destino],"-")==0){
            return;
        }
        else{
#ifdef DEBUG
            printf("ELIMINADO DE %s a %s\n",partida_str,destino_str);
#endif
            strcpy(data->tb_encaminhamento[destino][partida],"-");
            refresh_caminho_mais_curto(data,destino_str);   //atualizar caminhos mais curtos
        }
    }
    return;
}


int contar_nos_no_caminho(char *str) {
    int numeros = 0;
    char *ponteiro = str;

    // Verificar se a string é apenas "-"
    if (strcmp(str, "-") == 0) {
        return 0;
    }

    while (*ponteiro != '\0') {
        if (isdigit(*ponteiro)) {
            numeros++;

            // Avançar para o próximo caractere não numérico
            while (isdigit(*ponteiro)) {
                ponteiro++;
            }
        } else {
            ponteiro++;
        }
    }
    return numeros;
}

int rcv_pred(conect_inf*data){
    char buffer[128]={0};
    ssize_t n;
    char resp_cmp[128];
    int new_predecessor_fd;  //criar descritor temporário para novo cliente e aceitar a conexão
    data->host_info.addrlen=sizeof(data->host_info.addr);
    if((new_predecessor_fd=accept(data->host_info.fd,(struct sockaddr*) &data->host_info.addr,&data->host_info.addrlen))==-1)/*error*/ exit(1);
    //FD_CLR(data->host_info.fd,data->rfds);
#ifdef DEBUG
    printf("DEBUG: Socket criado. À espera de mensagem do meu futuro predecessor\n");
#endif
    n=read(new_predecessor_fd,buffer,128);   //receber msg com informação dele (PRED i\n)
    if(n==-1) exit(1);   //error
    //extrair parte da mensagem para comparar ("PRED") e confirmar que é a mensagem esperada
    sscanf(buffer,"%s",resp_cmp);
    if (strcmp(resp_cmp,"PRED")!=0){
        printf("Connection attempt declineddd\n");

        freeaddrinfo(data->client_info.res);
        close(data->client_info.fd);
        data->client_info.fd=-1;
        close(new_predecessor_fd);
        return -1;
    }
    else{   //caso seja a mensagem esperada, enviar mensagem de confirmação (OK\n) e atualizar informação do predecessor
    
        sscanf(buffer,"%*s %s",data->predecessor.ID);
        data->predecessor.TCP.fd=new_predecessor_fd;

    #ifdef DEBUG
        printf("DEBUG: A mensagem recebida foi a esperada. Atualizado socket do predecessor e definido o seu novo ID para %s\n",data->predecessor.ID);
    #endif

    }
    add_adj(data,1);
    add_adj(data,2);
    return 2;
    ////init_tabs(data);
}


void init_pred(conect_inf*data){
    int errcode;
    ssize_t n;
    char input[300];
    data->client_info.fd=socket(AF_INET,SOCK_STREAM,0); //TCP socket
    if (data->client_info.fd==-1) exit(1); //error

    memset(&data->client_info.hints,0,sizeof data->client_info.hints);
    data->client_info.hints.ai_family=AF_INET; //IPv4
    data->client_info.hints.ai_socktype=SOCK_STREAM; //TCP socket

    
    errcode=getaddrinfo(data->sucessor.IP,data->sucessor.PORT,&data->client_info.hints,&data->client_info.res);
    if(errcode!=0) exit(1); //error
    n=connect(data->client_info.fd,data->client_info.res->ai_addr,data->client_info.res->ai_addrlen);
    if(n==-1) exit(1);  //error
    
    sprintf(input,"PRED %s\n",data->id);
    n=write(data->client_info.fd,input,strlen(input)+1);
    if(n==-1)exit(1);  //error
#ifdef DEBUG
    printf("DEBUG: Enviado para o meu sucessor (%s) pelo socket client fd no caos em que só existe 1 nó: %s",data->sucessor.ID,input);
#endif
    return;
}

void enviar_mensagem(conect_inf* data, char* dest, char* mensagem, char* origem){
    int d=atoi(dest);

    char no_a_enviar[10]={0};

    strcpy(no_a_enviar,data->tb_exped[d]);

    char buffer_envio[256]={0};

    if (strcmp(origem,data->id)==0)
    {
        sprintf(buffer_envio,"CHAT %s %s %s\n",data->id,dest,mensagem);
    }else{
        sprintf(buffer_envio,"CHAT %s %s %s\n",origem,dest,mensagem);
    }
    if (strcmp(no_a_enviar,data->sucessor.ID)==0)
    {
        ssize_t n;
        n=write(data->client_info.fd,buffer_envio,strlen(buffer_envio)+1);
        if(n==-1) exit(1); //error
        return;
    }
    if (strcmp(no_a_enviar,data->predecessor.ID)==0)
    {
        ssize_t n;
        n=write(data->predecessor.TCP.fd,buffer_envio,strlen(buffer_envio)+1);
        if(n==-1) exit(1); //error
        return;
    }
    if (strcmp(no_a_enviar,data->chord.ID)==0)
    {
        ssize_t n;
        n=write(data->chord.TCP.fd,buffer_envio,strlen(buffer_envio)+1);
        if(n==-1) exit(1); //error
        return;
    }
    for (int i=0;data->rcv_chords[i]->fd!=-1;i++){
        if (strcmp(no_a_enviar,data->rcv_chords[i]->ID)==0)
        {
            ssize_t n;
            n=write(data->rcv_chords[i]->fd,buffer_envio,strlen(buffer_envio)+1);
            if(n==-1) exit(1); //error
            return;
        }
    }
    
    
#ifdef DEBUG
    printf("DEBUG: Enviado para o nó %s a mensagem: %s",no_a_enviar,buffer_envio);
#endif
    return;
}

void rcv_mensagem(conect_inf* data, char* mensagem){
    char origem[10]={0};
    char destino[10]={0};
    char msg[256]={0};
    sscanf(mensagem,"CHAT %s %s %s\n",origem,destino,msg);
    if (strcmp(destino,data->id)==0)
    {
        printf("Mensagem recebida de %s:%s\n",origem,&mensagem[11]);
    }else{
        enviar_mensagem(data,destino,&mensagem[11],origem);
    }
    return;
}



void send_chord(conect_inf* data){
    char buffer2[700]={0};

    char temp_id[4]={0};
    char temp_IP[30]={0};
    char temp_TCP[10]={0};
    
    struct sockaddr addr; 
    socklen_t addrlen; 
    ssize_t n;
    char buffer[700]={0};
    char invite[129]={0};

    char* buffer2_chopped;

    struct addrinfo hints, *res;
    int fd, errcode, randn=0;
    srand(time(NULL));

    fd = socket(AF_INET, SOCK_DGRAM, 0); //UDP socket
    if (fd == -1){
        printf("Erro socket na função send chord\n");
        exit(1);
    }
        

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;      //IPv4
    hints.ai_socktype = SOCK_DGRAM; //UDP socket

    errcode = getaddrinfo(data->reg_IP, data->reg_UDP, &hints, &res);
    if (errcode != 0){
        printf("Erro getaddrinfo na função send chord\n");
        exit(1);
    }
        
    
    //Pedir lista de nós ao servidor
    sprintf(invite,"NODES %s",data->ring);
    n = sendto(fd,invite, strlen(invite), 0, res->ai_addr, res->ai_addrlen);
    if (n == -1) /*error*/{
        printf("Erro sendto na função send chord\n");
        free(res);
        close(fd);
        exit(1);
    }
    addrlen=sizeof(addr); n=recvfrom(fd,buffer2,700,0,&addr,&addrlen); 
    if(n==-1){
        printf("Erro recvfrom na função Join\n");
        free(res);
        close(fd);
        exit(1);
    }
#ifdef DEBUG
    printf("Received %ld bytes: %s\n",n , buffer2);
#endif
    //nodeslist 051\n--->14 carateres

    buffer2_chopped=buffer2+14;
    
    char* rest;
    char* token;
    char strs[16][40];
    token=strtok_r(buffer2_chopped,"\n",&rest);
    int nlinhas=0,i=0,k=0;
    strcpy(temp_id,data->id);
    do{
        sscanf(token,"%s %s %s",temp_id,temp_IP,temp_TCP);
        nlinhas++;
        if (strcmp(temp_id,data->sucessor.ID)!=0 && strcmp(temp_id,data->predecessor.ID)!=0 && strcmp(temp_id,data->id)!=0)
        {   
            k=0;
            for (int j = 0; data->rcv_chords[j]->fd!=-1; j++)
            {
                if (strcmp(data->rcv_chords[j]->ID,temp_id)==0) k=1;
            }
            if (k==0){
                printf("No %s adicionado à lista de cordas\n",temp_id);
                strcpy(strs[i],token);
                i++;
            }


        }
        token=strtok_r(rest,"\n",&rest);
    }while (token!=NULL);
    if (i==0){
        printf("Não é possivel estabelecer corda com nenhum dos nós do anel\n");
        return;
    }

    srand(time(NULL));
    randn=rand()%i;
    sscanf(strs[randn],"%s %s %s",temp_id,temp_IP,temp_TCP);

    printf("No escolhido para corda: %s\n",temp_id);
    strcpy(data->chord.ID,temp_id);
    strcpy(data->chord.IP,temp_IP);
    strcpy(data->chord.PORT,temp_TCP);
    
    data->chord.TCP.fd=socket(AF_INET,SOCK_STREAM,0); //TCP socket
    if (data->chord.TCP.fd==-1) exit(1); //error

    memset(&data->chord.TCP.hints,0,sizeof data->chord.TCP.hints);
    data->chord.TCP.hints.ai_family=AF_INET; //IPv4
    data->chord.TCP.hints.ai_socktype=SOCK_STREAM; //TCP socket

    
    errcode=getaddrinfo(data->chord.IP,data->chord.PORT,&data->chord.TCP.hints,&data->chord.TCP.res);
    if(errcode!=0) exit(1); //error
    n=connect(data->chord.TCP.fd,data->chord.TCP.res->ai_addr,data->chord.TCP.res->ai_addrlen);
    if(n==-1) exit(1);  //error
    
    sprintf(buffer,"CHORD %s\n",data->id);
    n=write(data->chord.TCP.fd,buffer,strlen(buffer)+1);
    if(n==-1)exit(1);  //error
#ifdef DEBUG
    printf("DEBUG: Enviado para a corda (%s) pelo socket chords fd: %s",data->chord.ID,buffer);
#endif
    
    freeaddrinfo(res);
    close(fd);
    return;
}

void rmv_established_chord(conect_inf* data){
    close(data->chord.TCP.fd);
    data->chord.TCP.fd=-1;
    strcpy(data->chord.ID,"\0");
    strcpy(data->chord.IP,"\0");
    strcpy(data->chord.PORT,"\0");
    return;
}

void chord_disconnected(conect_inf* data,int pos){
    int j=0;
    _chord* aux;
    for(j=pos;strlen(data->rcv_chords[j]->ID)>0;j++);
    j--;
    aux=data->rcv_chords[pos];
    data->rcv_chords[pos]=data->rcv_chords[j];
    data->rcv_chords[j]=aux;
    data->rcv_chords[j]->fd=-1;
    data->rcv_chords[j]->ID[0]='\0';
    return;
}

void chord_disconnect_adj(conect_inf*data,char* adj){
#ifdef DEBUG
    printf("entrou no disconect_adj %s\n",adj);
#endif
    char buffer[256]={0};
    sprintf(buffer,"%s-%s",data->id,adj); // verifca se havia algum caminho mais curto a passar pela adjacencia removida
    for(int i=0;i<=99;i++){
        strcpy(data->tb_encaminhamento[i][atoi(adj)],"-");
        if (strstr(data->tb_caminhos_curtos[i],buffer)!=NULL && atoi(adj)!=i){                 /// tem um caminho que não ele proprio
#ifdef DEBUG
            printf("entrou no if e tem na tabela de encaminhamento:%s\n",data->tb_encaminhamento[i][atoi(adj)]);
#endif
            
            char i_str[10];
            if (i<10){
                sprintf(i_str,"0%d",i);
            }
            else{
                sprintf(i_str,"%d",i);
            }
         
            if(strlen(data->tb_encaminhamento[i][atoi(data->sucessor.ID)])>1 || strlen(data->tb_encaminhamento[i][atoi(data->predecessor.ID)])>1){
                refresh_caminho_mais_curto(data,i_str);
            }
            else if (data->chord.TCP.fd!=-1){
                if(strlen(data->tb_encaminhamento[i][atoi(data->chord.ID)])>1)
                refresh_caminho_mais_curto(data,i_str);
                for(int k=0;data->rcv_chords[k]->fd!=-1;k++){
                    if(strlen(data->tb_encaminhamento[i][atoi(data->rcv_chords[k]->ID)])>1){
                        refresh_caminho_mais_curto(data,i_str);
                        k=12;
                    }
                }
            }
            else{
                for(int k=0;data->rcv_chords[k]->fd!=-1;k++){
                    if(strlen(data->tb_encaminhamento[i][atoi(data->rcv_chords[k]->ID)])>1){
                        refresh_caminho_mais_curto(data,i_str);
                        k=12;
                    }
                }
            }

        }
        if(atoi(adj)==i){
#ifdef DEBUG
            printf("entrou no segundo if/n");
#endif
            refresh_caminho_mais_curto(data,adj);
        }
    }
}