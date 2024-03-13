#include "user_interface.h"


char* join(conect_inf*inicial_inf,char* ring,char* id){
    
    //Adicionar ao anel
    char buffer2[700];

    char temp_id[4];
    char temp_IP[30];
    char temp_TCP[10];
    
    struct sockaddr addr; 
    socklen_t addrlen; 
    ssize_t n;
    char buffer[700];
    char invite[129];
    char id_i[4];

    char* buffer2_chopped;

    struct addrinfo hints, *res;
    int fd, errcode, randn=0;
    srand(time(NULL));
    strcpy(id_i,id);
    int connection_attempts=0;

    fd = socket(AF_INET, SOCK_DGRAM, 0); //UDP socket
    if (fd == -1)                        /*error*/
        exit(1);

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;      //IPv4
    hints.ai_socktype = SOCK_DGRAM; //UDP socket

    errcode = getaddrinfo(inicial_inf->reg_IP, inicial_inf->reg_UDP, &hints, &res);
    if (errcode != 0) /*error*/
        exit(1);
    
    //Pedir lista de nós ao servidor
    sprintf(invite,"NODES %s",ring);
    n = sendto(fd,invite, strlen(invite), 0, res->ai_addr, res->ai_addrlen);
    if (n == -1) /*error*/{
        exit(1);
    }
    addrlen=sizeof(addr); n=recvfrom(fd,buffer2,700,0,&addr,&addrlen); if(n==-1)/*error*/exit(1);
    printf("Received %ld bytes: %s\n",n , buffer2);


    if (n==14){ //Caso seja o 10 nó no servidor
    #ifdef DEBUG
        printf("DEBUG: Não existem nós no anel\n");
    #endif
        sprintf(invite,"REG %s %s %s %s",ring,id_i,inicial_inf->IP,inicial_inf->TCP);
        n = sendto(fd,invite, strlen(invite), 0, res->ai_addr, res->ai_addrlen);
        if (n == -1) /*error*/{
            exit(1);
        }
        addrlen=sizeof(addr); n=recvfrom(fd,buffer,700,0,&addr,&addrlen); if(n==-1)/*error*/exit(1);
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
        }
    }else{
    #ifdef DEBUG
        printf("DEBUG: Existem nós no anel\n");
    #endif 
        buffer2_chopped=buffer2+14;
        sscanf(buffer2_chopped,"%s %s %s",temp_id,temp_IP,temp_TCP);
        while (1){
            sprintf(invite,"REG %s %s %s %s",ring,id_i,inicial_inf->IP,inicial_inf->TCP);
            n = sendto(fd,invite, strlen(invite), 0, res->ai_addr, res->ai_addrlen);
            if (n == -1) /*error*/{
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
        direct_join(inicial_inf);
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
    char buffer[700];
    char invite[129];
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
    char input[300];
    int errcode;
    ssize_t n;
    char buffer[128];

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


    char resp[300]; //Variavel para guardar resposta do nó a que me quero juntar

    //Esperar por resposta do nó a que me quero juntar que irá conter a informação do meu segundo sucessor (SUCC k k.IP k.TCP\n)
#ifdef DEBUG    
    printf("DEBUG: À espera que o sucessor confirme que aceitou e da mensagem com a informação do segundo sucessor\n");
#endif
    n=read(data->client_info.fd, resp, sizeof(resp));
#ifdef DEBUG  
    printf("DEBUG: Conexão estabelecida. recebido do sucessor a seguinte mensagem: %s\n", resp);
#endif

    //Verificar se a resposta é a esperada e atualizar informaçaõ do segundo sucessor caso seja, anular tudo caso não seja e enviar mensagem de erro
    
    char resp_cmp[300]; //variavel para extrair parte da resposta para comparar

    sscanf(resp,"%s",resp_cmp);
    if (strcmp(resp_cmp,"SUCC")!=0){
        printf("Connection attempt declined\n");

        freeaddrinfo(data->client_info.res);
        close(data->client_info.fd);
        return -1;
    }
    else{
        sscanf(resp,"%*s %s %s %s",data->secsuccessor.ID,data->secsuccessor.IP,data->secsuccessor.PORT);
        printf("DEBUG: Conexão com sucessor %s confirmada. O 2o sucessor foi atualizado para %s\n",data->sucessor.ID,data->secsuccessor.ID);
    }
    

    //A conexão com o sucessor correu bem, agora é necessário esperar que o meu futuro predecessor me envie a sua informação
#ifdef DEBUG
    printf("DEBUG: A iniciar socket para receber mensagem do meu futuro predecessor\n");
#endif


    int new_predecessor_fd;  //criar descritor temporário para novo cliente e aceitar a conexão
    data->host_info.addrlen=sizeof(data->host_info.addr);
    if((new_predecessor_fd=accept(data->host_info.fd,(struct sockaddr*) &data->host_info.addr,&data->host_info.addrlen))==-1)/*error*/ exit(1);
#ifdef DEBUG
    printf("DEBUG: Socket criado. À espera de mensagem do meu futuro predecessor\n");
#endif
    n=read(new_predecessor_fd,buffer,128);   //receber msg com informação dele (PRED i\n)
    if(n==-1) exit(1);   //error
    //extrair parte da mensagem para comparar ("PRED") e confirmar que é a mensagem esperada
    sscanf(buffer,"%s",resp_cmp);
    if (strcmp(resp_cmp,"PRED")!=0){
        printf("Connection attempt declined\n");

        freeaddrinfo(data->client_info.res);
        close(data->client_info.fd);
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
    ////init_tabs(data);
    return 0;
}



int add_client(conect_inf* data){
    //int errcode;
    ssize_t n;
    char buffer[128];
    char input[300];


    //criar descritor temporário para novo cliente e aceitar a conexão
    int futurefd;
    data->host_info.addrlen=sizeof(data->host_info.addr);
    if((futurefd=accept(data->host_info.fd,(struct sockaddr*) &data->host_info.addr,&data->host_info.addrlen))==-1)/*error*/ exit(1);
    
    n=read(futurefd,buffer,128);   //receber msg com informação dele (ENTRY i i.IP i.TCP\n)
    if(n==-1) exit(1);   //error
#ifdef DEBUG
    printf("Pedido de conexão recebido em futurefd do nó %s com a mensagem: %s\n", data->id, buffer);   //mostrar msg recebida (ENTRY i i.IP i.TCP\n)
#endif    

    //ADICIONAR VERIFICACAO PARA SABER SE TENHO ID
    
    char tmpid[10];
    char tmpIP[30];
    char tmpTCP[10];


    if(strstr(buffer,"ENTRY")!=NULL){   //se a msg recebida for a esperada
        //sscanf(buffer,"%*s %s %s %s",data->sucessor.ID,data->sucessor.IP,data->sucessor.PORT);   //atualizar informação do sucessor com a informação do novo nó
        sscanf(buffer,"%*s %s %s %s",tmpid,tmpIP,tmpTCP);
    }
    else{   //se não for a msg esperada
        printf("Connection attempt declined\n");
        close(futurefd);
        return -1;
    }


    //Não existe anel: dar erro e fechar conexão (ainda não foi definido o id do nó a que se estão a tentar conectar)
    if (data->id[0]=='\0'){
        printf("No ring with this node created yet, create ring first.\n");

        sprintf(input,"No ring created with target node yet\n");   
        n=write(futurefd,input,strlen(input)+1); 
        if(n==-1) exit(1); //error    
        
        close(futurefd);
        return -1;
    }

    //pedido de conexão tem id correto: aceitar conexão e enviar iniciar comunicação de protocolo
    //enviar msg com informação ao predecessor

    //verificar se existe predecessor i.e. se existe mais que 1 nó no anel
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

        //iniciar socket TCP como cliente para enviar msg com informação do novo nó ao meu predecessor, que é também o meu sucessor
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
        printf("DEBUG: Enviado para o meu sucessor (%s) pelo socket client fd no caos em que só existe 1 nó: %s",data->sucessor.ID,input);
    #endif
        return 0;
    }
    else{   //se existir: enviar msg com informação do novo nó ao meu predecessor, para que este o possa adicionar como seu sucessor

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


        sprintf(buffer,"SUCC %s %s %s\n",data->sucessor.ID,data->sucessor.IP,data->sucessor.PORT);   //enviar msg com informação do novo nó ao meu predecessor (ENTRY i i.IP i.TCP\n)

        n=write(data->predecessor.TCP.fd,buffer,strlen(input)+1); 
        if(n==-1) exit(1); //error   
    #ifdef DEBUG
        printf("Enviado ao nó %s que quer entrar a mensagem: %s\n",data->sucessor.ID,buffer);   //mostrar msg enviada (SUCC k k.IP k.TCP\n)
    #endif
    }
    return 0;
}

void rmv(conect_inf*inicial_inf,char *node_id){
    struct sockaddr addr; 
    socklen_t addrlen; 
    ssize_t n;
    char buffer[700];
    char invite[129];
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
    printf("Received: %s\n", buffer);
    if (strcmp("OKUNREG\n",buffer)!=0){
        printf("No retirado\n");
    }
    else{
        printf("No n retirado\n");
    }
    close(fd); 
    
    return;

    
}

void add_successor(conect_inf* data,char buffer[128]){

    ssize_t n;
    //Ler mensagem do meu sucessor que contém a informação do meu futuro sucessor (SUCC k k.IP k.TCP\n)
#ifdef DEBUG
    printf("DEBUG: Recebido do meu sucessor %s a mensagem: %s\n",data->sucessor.ID, buffer);   //mostrar msg recebida (SUCC i i.IP i.TCP\n)
#endif

    char tmpid[10];
    char tmpIP[30];
    char tmpTCP[10];

    //extrair parte da mensagem para comparar ("SUCC") e confirmar que é a mensagem esperada
    char resp_cmp[128];
    sprintf(resp_cmp,"%s",buffer);
    sscanf(buffer,"%s %s %s %s",resp_cmp,tmpid,tmpIP,tmpTCP);
    if(strcmp(resp_cmp,"ENTRY")==0){
            //colocar o novo nó como sucessor e colocar o meu sucessor como segundo sucessor e fechar socket com o meu sucessor
            strcpy(data->secsuccessor.ID,data->sucessor.ID);
            strcpy(data->secsuccessor.IP,data->sucessor.IP);
            strcpy(data->secsuccessor.PORT,data->sucessor.PORT);
            sscanf(buffer,"%*s %s %s %s",data->sucessor.ID,data->sucessor.IP,data->sucessor.PORT);
            
            close(data->client_info.fd);
        #ifdef DEBUG
            printf("DEBUG: Atualizado o meu sucessor para %s e o meu segundo sucessor para %s e fechado socket com este útimo\n",data->sucessor.ID,data->secsuccessor.ID);
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

            char input[300];
            sprintf(input,"PRED %s\n",data->id);
            n=write(data->client_info.fd,input,strlen(input)+1);
            if(n==-1)exit(1);  //error

        #ifdef DEBUG
            printf("DEBUG: Aberto socket com o meu novo sucessor, enviada a mensagem '%s' para o id %s\n",input,data->sucessor.ID);    
        #endif


            sprintf(input,"SUCC %s %s %s\n",data->sucessor.ID,data->sucessor.IP,data->sucessor.PORT);   //enviar msg com informação do novo nó ao meu predecessor (ENTRY i i.IP i.TCP\n)

            n=write(data->predecessor.TCP.fd,input,strlen(input)+1); 
            if(n==-1) exit(1); //error    
        #ifdef DEBUG
            printf("DEBUG: Enviado ao meu predecessor com id %s a mensagem: %s",data->predecessor.ID,input);   //mostrar msg enviada (SUCC k k.IP k.TCP\n)
        #endif
            
            return;
    }else if (strcmp(resp_cmp,"SUCC")==0){  //Caso receba a mensagem SUCC tenho de atualizar o meu segundo sucessor
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
    //3. Fechar conexão com o seu predecessor
    close(data->predecessor.TCP.fd);
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
    return 1;
}


void pred_reconnect(conect_inf* data,char buffer[128]){

    if (strcmp(data->id,data->secsuccessor.ID)==0)
    {
    #ifdef DEBUG
        printf("DEBUG: Saiu um dos 2 nós do anel. A colocar-me como nó único\n");
    #endif
        close(data->client_info.fd);
        close(data->predecessor.TCP.fd);
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
        sprintf(buffer,"SUCC %s %s %s\n",data->sucessor.ID,data->sucessor.IP,data->sucessor.PORT);   //enviar msg com informação do novo nó ao meu predecessor (ENTRY i i.IP i.TCP\n)
        n=write(data->predecessor.TCP.fd,buffer,strlen(buffer)+1);
        if(n==-1) exit(1); //error
    #ifdef DEBUG
        printf("DEBUG: Enviado ao meu predecessor com id %s a mensagem: %s e atualizado o meu predecessor",data->predecessor.ID,buffer);   //mostrar msg enviada (SUCC k k.IP k.TCP\n)
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

void suc_reconnect(conect_inf* data,char buffer[128]){
    ssize_t n;
    int errcode;  
    
    if (strcmp(data->id,data->secsuccessor.ID)==0)
    {
    #ifdef DEBUG
        printf("DEBUG: Saiu um dos 2 nós do anel. A colocar-me como nó único\n");
    #endif
        close(data->client_info.fd);
        close(data->predecessor.TCP.fd);
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
    sprintf(buffer,"SUCC %s %s %s\n",data->sucessor.ID,data->sucessor.IP,data->sucessor.PORT);   //enviar msg com informação do novo nó ao meu predecessor (ENTRY i i.IP i.TCP\n)

    n=write(data->predecessor.TCP.fd,buffer,strlen(input)+1); 
    if(n==-1) exit(1); //error    
#ifdef DEBUG
    printf("DEBUG: Enviado ao meu predecessor com id %s a mensagem: %s",data->predecessor.ID,buffer);   //mostrar msg enviada (SUCC k k.IP k.TCP\n)
#endif

    //esperar para receber a informação do meu segundo sucessor
    n=read(data->client_info.fd,buffer,128);
    if(n==-1) exit(1);   //error
    sscanf(buffer,"%*s %s %s %s",data->secsuccessor.ID,data->secsuccessor.IP,data->secsuccessor.PORT);
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
    return;
}

void add_adj(conect_inf*data,int pos){
    char adj[10];
    char buffer[200];
    int i=0,n,fd;
    if (pos==1){ 
        strcpy(adj,data->predecessor.ID);   ///determina se a adajacencia foi de um predecessor sucessor ou corda
        fd=data->predecessor.TCP.fd;
    }
    else if (pos==2){ 
        strcpy(adj,data->sucessor.ID);
        fd=data->client_info.fd;
    }
    else { 
        strcpy(adj,data->chords.ID);
        fd=data->chords.TCP.fd;
    }
    for (i=0;i<=99;i++){
        if(data->tb_caminhos_curtos[i][0]!='-'){
            sprintf(buffer,"ROUTE %d %d %s\n",atoi(data->id),i,data->tb_caminhos_curtos[i]);   
            n=write(fd,buffer,strlen(buffer)+1); 
            if(n==-1) exit(1); //error    
            #ifdef DEBUG
                printf("DEBUG: Enviado ao meu adjacente com id %s a mensagem: %s",adj,buffer);   //mostrar msg enviada (SUCC k k.IP k.TCP\n)
            #endif
        }
    }
    return;
}

void rmv_adj(conect_inf*data,int adj){
    char buffer[30];
    sprintf(buffer,"%d-%d",atoi(data->id),adj); // verifca se havia algum caminho mais curto a passar pela adjacencia removida
    for(int i=0;i<=99;i++){
        strcpy(data->tb_encaminhamento[i][adj],"-");
        if (strlen(data->tb_caminhos_curtos[i])>2 && strstr(buffer,data->tb_caminhos_curtos[i])!=NULL ){                 /// tem um caminho que não ele proprio
            refresh_caminho_mais_curto(data,i);
        }
    }
}

void refresh_caminho_mais_curto(conect_inf*data,int linha){
    int tam_caminho,menor=-1,tam_menor=200,n;
    char buffer[200];
    for (int i=0;i<=99;i++){
        tam_caminho=contar_nos_no_caminho(data->tb_encaminhamento[linha][i]);
        if (tam_caminho>0 && tam_caminho<tam_menor){
            tam_menor=tam_caminho;
            menor=i;
        }
    }

    if (menor==-1){
        strcpy(data->tb_caminhos_curtos[linha],"-");
        strcpy(data->tb_exped[linha],"-");
        sprintf(buffer,"ROUTE %d %d",atoi(data->id),linha);
    }
    else if(contar_nos_no_caminho(data->tb_encaminhamento[linha][menor])==contar_nos_no_caminho(data->tb_caminhos_curtos[linha])){
        return;
    }
    else {
        strcpy(data->tb_caminhos_curtos[linha],data->tb_encaminhamento[linha][menor]);
        if (tam_menor==2)
        sscanf(data->tb_caminhos_curtos[linha],"%*d-%s",data->tb_exped[linha]); 
        else
        sscanf(data->tb_caminhos_curtos[linha],"%*d-%s-%*s",data->tb_exped[linha]); 
        sprintf(buffer,"ROUTE %d %d %s",atoi(data->id),linha,data->tb_caminhos_curtos[linha]);
    }
    if(strlen(data->sucessor.ID)!=0){
        n=write(data->client_info.fd,buffer,strlen(buffer)+1); 
            if(n==-1) exit(1); //error    
            #ifdef DEBUG
                printf("DEBUG: Enviado ao meu adjacente com id %s a mensagem: %s",data->sucessor.ID,buffer);   //mostrar msg enviada (SUCC k k.IP k.TCP\n)
            #endif
    }
    if(strlen(data->predecessor.ID)!=0){
        n=write(data->predecessor.TCP.fd,buffer,strlen(buffer)+1); 
            if(n==-1) exit(1); //error    
            #ifdef DEBUG
                printf("DEBUG: Enviado ao meu adjacente com id %s a mensagem: %s",data->predecessor.ID,buffer);   //mostrar msg enviada (SUCC k k.IP k.TCP\n)
            #endif
    }
    /////
    ////FALTA PARA AS CORDAS
    /////
    return;
}

void chamada_route(conect_inf*data,char*mensagem){
    int partida, destino;
    char id[8];
    char buffer[120];
    char sequencia_com_tracos[100];


    int num_converted = sscanf(mensagem, "ROUTE %d %d %s", &partida, &destino, sequencia_com_tracos);
    sprintf(id,"-%d",atoi(data->id));
    if (num_converted == 3) {
        if (strstr(sequencia_com_tracos,id)){
            return;
        }
        else{
            sprintf(buffer,"%d-%s",atoi(data->id),sequencia_com_tracos);
            strcpy(data->tb_encaminhamento[destino][partida],buffer);
            refresh_caminho_mais_curto(data,destino);
        }
    } else {
        strcpy(data->tb_encaminhamento[destino][partida],"-");
        refresh_caminho_mais_curto(data,destino);
    }
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