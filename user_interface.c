#include "user_interface.h"


int join(conect_inf*inicial_inf,char* ring,char* id){
    struct sockaddr addr; 
    socklen_t addrlen; 
    ssize_t n;
    char buffer[700];
    char invite[129];
    struct addrinfo hints, *res;
    int fd, errcode, randn=0;
    srand(time(NULL));

    fd = socket(AF_INET, SOCK_DGRAM, 0); //UDP socket
    if (fd == -1)                        /*error*/
        exit(1);

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;      //IPv4
    hints.ai_socktype = SOCK_DGRAM; //UDP socket

    errcode = getaddrinfo(inicial_inf->reg_IP, inicial_inf->reg_UDP, &hints, &res);
    if (errcode != 0) /*error*/
        exit(1);
    while (1){
        sprintf(invite,"REG %s %s %s %s",ring,id,inicial_inf->IP,inicial_inf->TCP);
        n = sendto(fd,invite, strlen(invite), 0, res->ai_addr, res->ai_addrlen);
        if (n == -1) /*error*/
            exit(1);
        freeaddrinfo(res);
        addrlen=sizeof(addr); n=recvfrom(fd,buffer,700,0,&addr,&addrlen); if(n==-1)/*error*/exit(1);
        buffer[n] = '\0';
        printf("%s\n", buffer);
        if (strcmp("OKREG",buffer)!=0){
            randn=rand()%100;
            if (randn<10) sprintf(id,"0%d",randn);
            else sprintf(id,"%d",randn);
        }
        else{
            break;
        }
    }
    printf("Nó registado com o id %s.\n",id);
    close(fd); 
    
    return -1;
}