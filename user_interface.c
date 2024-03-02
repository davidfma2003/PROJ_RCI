#include "user_interface.h"


char* join(conect_inf*inicial_inf,char* ring,char* id){
    
    //Adicionar ao anel

    char buffer2[700];

    char temp_id[4];
    char temp_IP[30];
    char temp_TCP[10];

    sscanf(get_nodes(inicial_inf,ring,buffer2),"%*s %*s\n%s %s %s",temp_id,temp_IP,temp_TCP);
    printf("%s\n",buffer2);
    printf("\nID: %s\n",temp_id);
    
    struct sockaddr addr; 
    socklen_t addrlen; 
    ssize_t n;
    char buffer[700];
    char invite[129];
    char id_i[4];
    struct addrinfo hints, *res;
    int fd, errcode, randn=0;
    srand(time(NULL));
    strcpy(id_i,id);
    int connection_attempts=0;
    while (1){
        fd = socket(AF_INET, SOCK_DGRAM, 0); //UDP socket
        if (fd == -1)                        /*error*/
            exit(1);

        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_INET;      //IPv4
        hints.ai_socktype = SOCK_DGRAM; //UDP socket

        errcode = getaddrinfo(inicial_inf->reg_IP, inicial_inf->reg_UDP, &hints, &res);
        if (errcode != 0) /*error*/
            exit(1);
   
        sprintf(invite,"REG %s %s %s %s",ring,id_i,inicial_inf->IP,inicial_inf->TCP);
        n = sendto(fd,invite, strlen(invite), 0, res->ai_addr, res->ai_addrlen);
        if (n == -1) /*error*/{
            exit(1);
        }
        freeaddrinfo(res);
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

    strcpy(id,id_i);

    close(fd); 
    if (strcmp(temp_id,id)==0){
        printf("1o nó registado no servidor\n");
        strcpy(inicial_inf->predecessor.ID,id);
        strcpy(inicial_inf->sucessor.ID,id);
        strcpy(inicial_inf->sucessor.IP,inicial_inf->IP);
        strcpy(inicial_inf->sucessor.PORT,inicial_inf->TCP);
        strcpy(inicial_inf->secsuccessor.ID,id);
        strcpy(inicial_inf->secsuccessor.IP,inicial_inf->IP);
        strcpy(inicial_inf->secsuccessor.PORT,inicial_inf->TCP);
    }
    else{
        printf("nó Registado em servidor que não está vazio\n");
        strcpy(inicial_inf->sucessor.ID,temp_id);
        strcpy(inicial_inf->sucessor.IP,temp_IP);
        strcpy(inicial_inf->sucessor.PORT,temp_TCP);
        direct_join(inicial_inf);   
    }
    

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


    data->host_info.fd=socket(AF_INET,SOCK_STREAM,0); //TCP socket
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


    //Iniciar ligação TCP como cliente do nó a que me quero juntar através do seu endereço IP e porta TCP que foi dada como input
    data->client_info.fd=socket(AF_INET,SOCK_STREAM,0); //TCP socket
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
    printf("Enviado: %s",input);
    n=write(data->client_info.fd,input,strlen(input)+1);
    if(n==-1)/*error*/exit(1);


    char resp[300]; //Variavel para guardar resposta do nó a que me quero juntar

    //Esperar por resposta do nó a que me quero juntar que irá conter a informação do meu segundo sucessor (SUCC k k.IP k.TCP\n)
    printf("Waiting for confirmation from sucessor\n");

    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(data->client_info.fd, &read_fds);

    int ready = select(data->client_info.fd + 1, &read_fds, NULL, NULL, NULL);  
    if (ready < 0) {
        perror("Error in select");
        exit(EXIT_FAILURE);
    }
    else if (FD_ISSET(data->client_info.fd, &read_fds)) {    //quando chegar a resposta, i.e. o descritor foi alterado
        n=read(data->client_info.fd, resp, sizeof(resp));
        printf("Connection established. Received: %s\n", resp);
        
    }

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
        printf("Connection confirmed. 2nd sucessor is %s\n",data->secsuccessor.ID);
    }
    

    //A conexão com o sucessor correu bem, agora é necessário esperar que o meu futuro predecessor me envie a sua informação
    printf("Waiting for client request of predecessor\n");
    
    int newfd;  //criar descritor temporário para novo cliente e aceitar a conexão
    data->host_info.addrlen=sizeof(data->host_info.addr);
    if((newfd=accept(data->host_info.fd,(struct sockaddr*) &data->host_info.addr,&data->host_info.addrlen))==-1)/*error*/ exit(1);


    //Esperar por mensagem de protocolo do meu futuro predecessor que contém a sua informação (PRED i\n)
    FD_ZERO(&read_fds);
    FD_SET(newfd, &read_fds);

    char buffer[128];
    ready = select(newfd + 1, &read_fds, NULL, NULL, NULL);
    if (ready < 0) {
        perror("Error in select");
        exit(EXIT_FAILURE);
    }

    if (FD_ISSET(newfd, &read_fds)) {
        n=read(newfd,buffer,128);   //receber msg com informação dele (PRED i\n)
        if(n==-1) exit(1);   //error
        printf("Received: %s\n", buffer);
    }
    //extrair parte da mensagem para comparar ("PRED") e confirmar que é a mensagem esperada
    sscanf(buffer,"%s",resp_cmp);
    if (strcmp(resp_cmp,"PRED")!=0){
        printf("Connection attempt declined\n");

        freeaddrinfo(data->client_info.res);
        close(data->client_info.fd);
        close(newfd);
        return -1;
    }
    else{   //caso seja a mensagem esperada, enviar mensagem de confirmação (OK\n) e atualizar informação do predecessor
        printf("Sent: Predecessor established.\n");
        sscanf(buffer,"%*s %s",data->predecessor.ID);
        n=write(newfd,"OK",3);
        data->predecessor.TCP.fd=newfd;
    }


    //n=read(fd,buffer,128);
    //if(n==-1)/*error*/exit(1);
    freeaddrinfo(data->client_info.res);
    close(data->client_info.fd);
    return 0;
    
}


int add_client(conect_inf* data){
    //int errcode;
    ssize_t n;
    char buffer[128];
    char input[300];

    //criar descritor temporário para novo cliente e aceitar a conexão
    int newfd;
    data->host_info.addrlen=sizeof(data->host_info.addr);
    if((newfd=accept(data->host_info.fd,(struct sockaddr*) &data->host_info.addr,&data->host_info.addrlen))==-1)/*error*/ exit(1);
    
    n=read(newfd,buffer,128);   //receber msg com informação dele (ENTRY i i.IP i.TCP\n)
    if(n==-1) exit(1);   //error
    printf("Connection request. Received: %s\n", buffer);   //mostrar msg recebida (ENTRY i i.IP i.TCP\n)
    

    if(strstr(buffer,"ENTRY")!=NULL){   //se a msg recebida for a esperada
        sscanf(buffer,"%*s %s %s %s",data->sucessor.ID,data->sucessor.IP,data->sucessor.PORT);   //atualizar informação do sucessor com a informação do novo nó
    }
    else{   //se não for a msg esperada
        printf("Connection attempt declined\n");
        close(newfd);
        return -1;
    }

    
    //Não existe anel: dar erro e fechar conexão (ainda não foi definido o id do nó a que se estão a tentar conectar)
    if (data->id[0]=='\0'){
        printf("No ring with this node created yet, create ring first.\n");

        sprintf(input,"No ring created with target node yet\n");   
        n=write(newfd,input,strlen(input)+1); 
        if(n==-1) exit(1); //error    
        
               

        close(newfd);
        return -1;
    }

    //pedido de conexão tem id correto: aceitar conexão e enviar iniciar comunicação de protocolo
    //enviar msg com informação ao predecessor
    //verificar se existe predecessor i.e. se existe mais que 1 nó no anel
    if(strcmp(data->id,data->predecessor.ID)==0){ //se não existir: 

        sscanf(buffer,"%*s %s %s %s",data->sucessor.ID,data->sucessor.IP,data->sucessor.PORT);   //atualizar informação do sucessor com a informação do novo nó

        sprintf(input,"SUCC %s %s %s\n",data->sucessor.ID,data->sucessor.IP,data->sucessor.PORT);   //confirmar ao novo nó que sou o seu sucessor e enviar msg com info do meu sucessor (SUCC k k.IP k.TCP\n), que é o 2o sucessor do nó a entrar
        n=write(newfd,input,strlen(input)+1); 
        if(n==-1) exit(1); //error    
        printf("Connection established. Sent: %s",input);   //mostrar msg enviada (SUCC k k.IP k.TCP\n)

    

        int errcode;
        data->client_info.fd=socket(AF_INET,SOCK_STREAM,0); //TCP socket
        if (data->client_info.fd==-1) exit(1); //error

        memset(&data->client_info.hints,0,sizeof data->client_info.hints);
        data->client_info.hints.ai_family=AF_INET; //IPv4
        data->client_info.hints.ai_socktype=SOCK_STREAM; //TCP socket
        
        sscanf(buffer,"%*s %s %s %s",data->sucessor.ID,data->sucessor.IP,data->sucessor.PORT);
        
        errcode=getaddrinfo(data->sucessor.IP,data->sucessor.PORT,&data->client_info.hints,&data->client_info.res);
        if(errcode!=0) exit(1); //error
        n=connect(data->client_info.fd,data->client_info.res->ai_addr,data->client_info.res->ai_addrlen);
        if(n==-1) exit(1);  //error
        
        sprintf(input,"PRED %s\n",data->id);
        printf("Enviado: %s",input);
        n=write(data->client_info.fd,input,strlen(input)+1);
        if(n==-1)exit(1);  //error


        printf("À espera de confirmação do envio de predecessor\n");
        fd_set read_fdsa;
        FD_ZERO(&read_fdsa);
        FD_SET(data->client_info.fd, &read_fdsa);
        int ready = select(data->client_info.fd + 1, &read_fdsa, NULL, NULL, NULL);
        if (ready < 0) {
            perror("Error in select");
            exit(EXIT_FAILURE);
        }

        if (FD_ISSET(data->client_info.fd, &read_fdsa)) {
            n=read(data->client_info.fd,buffer,128);   //confirmar que enviou a informação (OK\n)
            if(n==-1) exit(1);   //error
            printf("Received: %s\n", buffer);
        }
        
        //Atualizar informação de predecessor
        data->predecessor.TCP.fd=newfd;
        strcpy(data->predecessor.ID,data->sucessor.ID);
    }
    else{   //se existir: enviar msg com informação do novo nó ao meu predecessor, para que este o possa adicionar como seu sucessor
        

         



        sprintf(input,"SUCC %s %s %s\n",data->sucessor.ID,data->sucessor.IP,data->sucessor.PORT);   //confirmar ao novo nó que sou o seu sucessor e enviar msg com info do meu sucessor (SUCC k k.IP k.TCP\n), que é o 2o sucessor do nó a entrar
        n=write(newfd,input,strlen(input)+1); 
        if(n==-1) exit(1); //error    
        printf("Connection established. Sent: %s",input);   //mostrar msg enviada (SUCC k k.IP k.TCP\n)

        int errcode;
        data->client_info.fd=socket(AF_INET,SOCK_STREAM,0); //TCP socket
        if (data->client_info.fd==-1) exit(1); //error

        memset(&data->client_info.hints,0,sizeof data->client_info.hints);
        data->client_info.hints.ai_family=AF_INET; //IPv4
        data->client_info.hints.ai_socktype=SOCK_STREAM; //TCP socket
        
        sscanf(buffer,"%*s %s %s %s",data->sucessor.ID,data->sucessor.IP,data->sucessor.PORT);
        
        errcode=getaddrinfo(data->sucessor.IP,data->sucessor.PORT,&data->client_info.hints,&data->client_info.res);
        if(errcode!=0) exit(1); //error
        n=connect(data->client_info.fd,data->client_info.res->ai_addr,data->client_info.res->ai_addrlen);
        if(n==-1) exit(1);  //error
        
        sprintf(input,"PRED %s\n",data->predecessor.ID);
        printf("Enviado: %s",input);
        n=write(data->client_info.fd,input,strlen(input)+1);
        if(n==-1)exit(1);  //error


        printf("À espera de confirmação do envio de predecessor\n");
        fd_set read_fdsa;
        FD_ZERO(&read_fdsa);
        FD_SET(data->client_info.fd, &read_fdsa);
        int ready = select(data->client_info.fd + 1, &read_fdsa, NULL, NULL, NULL);
        if (ready < 0) {
            perror("Error in select");
            exit(EXIT_FAILURE);
        }

        if (FD_ISSET(data->client_info.fd, &read_fdsa)) {
            n=read(data->client_info.fd,buffer,128);   //confirmar que enviou a informação (OK\n)
            if(n==-1) exit(1);   //error
            printf("Received: %s\n", buffer);
        }
        
        //Atualizar informação de predecessor
        data->predecessor.TCP.fd=newfd;
        strcpy(data->predecessor.ID,data->sucessor.ID);
        
    }
    close(newfd);

    return 0;
}


int decode_msg(int fd){
    ssize_t n;
    char buffer[128];
    n=read(fd,buffer,128);
    if(n==-1){printf("entrou no exit");exit(1);}  //error    
    char resp_cmp[128];
    sprintf(buffer,"%s",resp_cmp);

    printf("Received in resp comp: %s\n", resp_cmp);
    if(strcmp(buffer,"SUCC")==0){
        printf("Received: %s\n", buffer);
    }
    else if(strcmp(buffer,"PRED")==0){
        printf("Received: %s\n", buffer);
    }
    else if(strcmp(buffer,"ENTRY")==0){
        printf("Received: %s\n", buffer);
    }
    n=read(fd,buffer,128);
    printf("Received: %s\n", buffer);
    
    
    return 0;
}


char* get_nodes(conect_inf*inicial_inf,char* ring,char* buffer){
    struct sockaddr addr; 
    socklen_t addrlen; 
    ssize_t n;
    //char buffer[700];
    struct addrinfo hints, *res;
    int fd, errcode;
    srand(time(NULL));

    char invite[129];


    fd = socket(AF_INET, SOCK_DGRAM, 0); //UDP socket
    if (fd == -1)                        /*error*/
        exit(1);

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;      //IPv4
    hints.ai_socktype = SOCK_DGRAM; //UDP socket

    errcode = getaddrinfo(inicial_inf->reg_IP, inicial_inf->reg_UDP, &hints, &res);
    if (errcode != 0) /*error*/
        exit(1);

    sprintf(invite,"NODES %s",ring);
    n = sendto(fd,invite, strlen(invite), 0, res->ai_addr, res->ai_addrlen);
    if (n == -1) /*error*/{
        exit(1);
    }
    freeaddrinfo(res);
    addrlen=sizeof(addr); n=recvfrom(fd,buffer,700,0,&addr,&addrlen); if(n==-1)/*error*/exit(1);
    buffer[n] = '\0';

    close(fd);
    return buffer;
}