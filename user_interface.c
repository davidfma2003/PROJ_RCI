#include "user_interface.h"


char* join(conect_inf*inicial_inf,char* ring,char* id){
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
        if (n == -1) /*error*/
            exit(1);
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

    data->client_info.fd=socket(AF_INET,SOCK_STREAM,0); //TCP socket
    if (data->client_info.fd==-1) exit(1); //error

    memset(&data->client_info.hints,0,sizeof data->client_info.hints);
    data->client_info.hints.ai_family=AF_INET; //IPv4
    data->client_info.hints.ai_socktype=SOCK_STREAM; //TCP socket
    
    
    errcode=getaddrinfo(data->sucessor.IP,data->sucessor.PORT,&data->client_info.hints,&data->client_info.res);
    if(errcode!=0)/*error*/exit(1);
    
    n=connect(data->client_info.fd,data->client_info.res->ai_addr,data->client_info.res->ai_addrlen);
    if(n==-1)/*error*/exit(1);
    
    sscanf(input,"ENTRY %s %s %s\n",data->sucessor.ID,data->sucessor.IP,data->sucessor.PORT);
    n=write(data->sucessor.TCP.fd,input,strlen(input));
    if(n==-1)/*error*/exit(1);
    
    
    //n=read(fd,buffer,128);
   /// if(n==-1)/*error*/exit(1);
    freeaddrinfo(data->client_info.res);
    close(data->client_info.fd);
    return 0;
    
}

int add_client(conect_inf* data){


    
    //int errcode;
    ssize_t n;
    char buffer[128];
    

    //criar descritor para novo cliente
    int newfd;
    data->host_info.addrlen=sizeof(data->host_info.addr);
    if((newfd=accept(data->host_info.fd,(struct sockaddr*) &data->host_info.addr,&data->host_info.addrlen))==-1)/*error*/ exit(1);
    
    n=read(newfd,buffer,128);   //receber msg com informação dele
    if(n==-1)/*error*/exit(1);
    buffer[n]='\0';
    
    //reencaminhar mensagem para o meu predecessor
    printf("%s\n",buffer);
    







/*
    char send_buffer[300];

    sscanf(send_buffer,"ENTRY %s %s %s\n",data->predecessor.ID,data->predecessor)

    write(1,"ENTRY i i.IP i.TCP\n ",10);
    write(1,buffer,n);
    
    n=write(data->predecessor.TCP.fd,buffer,n);
    if(n==-1)error*///exit(1);
    
    return 0;
}