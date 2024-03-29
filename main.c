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
    data.chord.TCP.fd=-1;
    data.chordadd=false;
    create_TCP_server(&data);
    alloc_tabs(&data);
    for (int i=0;i<14;i++){                           //guarda as estruturas de cordas num vetor de ponteiros
        data.rcv_chords[i]=&data.mem_chords[i];
        data.rcv_chords[i]->fd=-1;
        data.rcv_chords[i]->ID[0]='\0';
    }

    

#ifdef DEBUG
    printf("DEBUG: Servidor TCP criado e memória alocada\n");
#endif

    fd_set rfds;
    int maxfd;
    char bufferhold[256]={0};
    ssize_t n;
    int cladd=0;
    int sucadd=0;
    char aux[10]={0};
    printf("Digite:\n");
    while (1)
    {
        FD_ZERO(&rfds); // inicializar o conjunto de descritores a 0
        FD_SET(0,&rfds); // adicionar o descritor 0 (stdin) ao conjunto 
        if (data.host_info.fd!=-1) FD_SET(data.host_info.fd,&rfds); // adicionar o descritor fd (socket TCP) ao conjunto
        if (data.client_info.fd!=-1) FD_SET(data.client_info.fd,&rfds); // adicionar o descritor fd (socket TCP) ao conjunto
        if (data.predecessor.TCP.fd!=-1) FD_SET(data.predecessor.TCP.fd,&rfds); // adicionar o descritor fd (socket TCP) ao conjunto
        if (data.chord.TCP.fd!=-1) FD_SET(data.chord.TCP.fd,&rfds); // adicionar o descritor fd (socket TCP) ao conjunto
        for (int i = 0; data.rcv_chords[i]->fd!=-1; i++) FD_SET(data.rcv_chords[i]->fd,&rfds); // adicionar o descritor fd (socket TCP) ao conjunto
        
        maxfd=retmax(&data);
#ifdef DEBUG
        printf("Digite:\n");
#endif
        select(maxfd+1,&rfds,(fd_set*)NULL,(fd_set*)NULL,(struct timeval*)NULL);    // espera por um descritor pronto
        if (FD_ISSET(0,&rfds)){
#ifdef DEBUG
            printf("User Input\n");
#endif
            user_input(&data);
            printf("Digite:\n");
            continue;
        }
        else if (FD_ISSET(data.host_info.fd,&rfds)){
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
            char aux2[10]={0};
            strcpy(bufferhold,rdbuffer);
            token=strtok_r(bufferhold,"\n",&rest);
            int ret=0;
            while (token!=NULL)
            {
                strcpy(aux2,data.predecessor.ID);
                ret=add_client(&data,token,futurefd);
                token=strtok_r(rest,"\n", &rest);
            }
            if (strcmp(aux2,data.sucessor.ID)!=0 && ret!=4)
            {
                rmv_adj(&data,aux2);
            }
            if (ret!=4) cladd=1;
            else add_adj(&data,4);
            
            
        }
        else if (FD_ISSET(data.predecessor.TCP.fd,&rfds))
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

                strcpy(aux,data.predecessor.ID);
                
                pred_reconnect(&data,rdbuffer);
                disconect_adj(&data,aux,data.predecessor.ID); 
                cladd=2;
                
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
                        if (cladd==2){
                            cladd=0;
                            add_adj(&data,1);
                        }
                        chamada_route(&data,token);
                    }else if (strstr(token,"CHAT"))
                    {
                        rcv_mensagem(&data,token);
                    }
                    token=strtok_r(rest,"\n", &rest);
                }
            }           
        }
        else if (FD_ISSET(data.client_info.fd,&rfds))
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

                strcpy(aux,data.sucessor.ID);
                suc_reconnect(&data,rdbuffer);
                sucadd=2;
                
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
                        rcv_mensagem(&data,token);
                    }else if(strstr(token,"ROUTE")){
                        if(sucadd==1)
                        {
                            sucadd=0;
                            add_adj(&data,2);
                        }
                        else if(sucadd==2){
                            sucadd=0;
                            disconect_adj(&data,aux,data.sucessor.ID);
                            add_adj(&data,2);
                        }
                        chamada_route(&data,token);

                    }
                    token=strtok_r(rest,"\n", &rest);
                }

            }

        }else if (FD_ISSET(data.chord.TCP.fd,&rfds)){
#ifdef DEBUG
            printf("Message received from established chord\n");
#endif          
            char rdbuffer[256]={0};
            n=read(data.chord.TCP.fd,rdbuffer,256);
            FD_CLR(data.chord.TCP.fd,&rfds);
            if (n==-1) printf("ERRO a receber mensagem\n");
            else if(n==0){
                printf("Chord desconectado\n");
                chord_disconnect_adj(&data,data.chord.ID);
                rmv_established_chord(&data);
            }
            else{
                strcpy(bufferhold,rdbuffer);

                char* rest;
                char* token; 
                
                token=strtok_r(bufferhold,"\n",&rest);
                while (token!=NULL)
                {
                    if (strstr(token,"ROUTE ")){
                        if (data.chordadd){
                            data.chordadd=false;
                            add_adj(&data,3);
                        }
                        chamada_route(&data,token);
                    }else if(strstr(token,"CHAT ")){
                        rcv_mensagem(&data,token);
                    }
                    token=strtok_r(rest,"\n", &rest);
                }
            }
            
        }else{
            for (int i = 0; i < 14; i++)
            {
                if (FD_ISSET(data.rcv_chords[i]->fd,&rfds)){
#ifdef DEBUG
                    printf("Message received from chord with id %s\n",data.rcv_chords[i]->ID);
#endif          
                    char rdbuffer[256]={0};
                    n=read(data.rcv_chords[i]->fd,rdbuffer,256);
                    FD_CLR(data.rcv_chords[i]->fd,&rfds);
                    if (n==-1) printf("ERRO a receber mensagem\n");
                    else if(n==0){
                        printf("Chord desconectado\n");
                        strcpy(aux,data.rcv_chords[i]->ID);
                        chord_disconnected(&data,i);
                        chord_disconnect_adj(&data,aux);
                    }
                    else{
                        strcpy(bufferhold,rdbuffer);

                        char* rest;
                        char* token; 
                        
                        token=strtok_r(bufferhold,"\n",&rest);
                        while (token!=NULL)
                        {
                            if (strstr(token,"ROUTE ")){
                               
                                chamada_route(&data,token);
                            }else if(strstr(token,"CHAT ")){
                                rcv_mensagem(&data,token);
                            }
                            token=strtok_r(rest,"\n", &rest);
                        }
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
    
    if(input[0]=='x'){
        printf("Fecho da aplicação\n");
        freeaddrinfo(data->host_info.res);
        close(data->host_info.fd);
        free_tabs(data);
        exit(0);
    }

    else if(input[0]=='j' && input[1]==' '){
        sscanf(input,"%*s %s %s",data->ring,id_buff);
        char *tb=NULL;
        strtol(data->ring,&tb,10);
        if (tb==data->ring)
        {
            printf("Input inválido\nPor favor tente novamente\n");
            return;
        }
        tb=NULL;
        strtol(id_buff,&tb,10);
        if (tb==id_buff)
        {
            printf("Input inválido\nPor favor tente novamente\n");
            return;
        }
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

        char *tb=NULL;
        strtol(data->id,&tb,10);
        if (tb==data->id)
        {
            printf("Input inválido\nPor favor tente novamente\n");
            return;
        }
        tb=NULL;
        strtol(data->sucessor.ID,&tb,10);
        if (tb==data->sucessor.ID)
        {
            printf("Input inválido\nPor favor tente novamente\n");
            return;
        }


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
            init_tabs(data);
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
        printf("Corda estabelecida:\n\tid: %s\n\tIP: %s\n\tPorta: %s\n",data->chord.ID,data->chord.IP,data->chord.PORT);
        printf("Cordas recebidas: ");
        for (int i = 0; i < 14; i++)
        {
            if (data->rcv_chords[i]->ID[0]!='\0') printf("%s ",data->rcv_chords[i]->ID);
        }
        printf("\n");
    }
    else if(input[0]=='s' && input[1]=='r' && input[2]==' '){
        char dest[10]={0};
        sscanf(input,"%*s %s",dest);
        char *tb=NULL;
        strtol(dest,&tb,10);
        if (tb==dest)
        {
            printf("Input inválido\nPor favor tente novamente\n");
            return;
        }
        
        printf("Tabela de encaminhamento para %s:\n",dest);



        for (int i = 0; i < 100; i++)
        {
            if (strcmp(data->tb_encaminhamento[atoi(dest)][i],"-")!=0)
            {
                printf("%d: %s\n",i,data->tb_encaminhamento[atoi(dest)][i]);
            }
        }
    }
    else if (input[0]=='s' && input[1]=='p' && input[2]==' '){
        char dest[10]={0};
        sscanf(input,"%*s %s",dest);
        char *tb=NULL;
        strtol(dest,&tb,10);
        if (tb==dest)
        {
            printf("Input inválido\nPor favor tente novamente\n");
            return;
        }
        printf("Caminho mais pequeno para %s:\n\t%s",dest,data->tb_caminhos_curtos[atoi(dest)]);
    }
    else if(input[0]=='s' && input[1]=='f'){
        printf("Tabela de expedição:\n");
        for (int i = 0; i < 100; i++)
        {
            if (strcmp(data->tb_exped[i],"-")!=0)
            {
                printf("%d: %s\n",i,data->tb_exped[i]);
            }   
        }
    }else if (input[0]=='m' && input[1]==' ')   
    {
        char dest[10]={0};
        char msg[100]={0};
        sscanf(input,"%*s %s %s\n",dest,msg);
        if (strlen(dest)!=2){
            printf("Valor de id de destino inválido\n");
            return;
        }
        char *tb=NULL;
        strtol(dest,&tb,10);
        if (tb==dest)
        {
            printf("Input inválido\nPor favor tente novamente\n");
            return;
        }
        if (strcmp(data->tb_caminhos_curtos[atoi(dest)],"-")==0)
        {
            printf("Não é possível enviar mensagem para %s\n",dest);
        }
        else{
            enviar_mensagem(data,dest,&input[5],data->id);
            printf("Mensagem enviada para %s\n",dest);
        }
    }
    else if (input[0]=='c')    
    {
        if(data->chord.TCP.fd==-1 && strcmp(data->secsuccessor.ID,data->predecessor.ID)!=0 && strcmp(data->sucessor.ID,data->predecessor.ID)!=0 && strcmp(data->id,data->sucessor.ID)!=0){
            send_chord(data); 
            data->chordadd=true;
        }
        else{
            printf("Não é possivel estabelecer corda com nenhum dos nós do anel_input\n\n\n");
            return;
        }
    }
    else if (input[0]=='r' && input[1]=='c')    
    {
        if(data->chord.TCP.fd!=-1){
            char aux[10]={0};
            strcpy(aux,data->chord.ID);
            rmv_established_chord(data);
            chord_disconnect_adj(data,aux);
        } 
    }
    
    else if(input[0]=='r'){
        char temp[10];
        sscanf(input,"%*s %s",temp);
        rmv(data,temp);
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

int retmax(conect_inf*data){
    int maxfd=-1;
    if(data->predecessor.TCP.fd>maxfd)
        maxfd=data->predecessor.TCP.fd;
    if(data->client_info.fd>maxfd)
        maxfd=data->client_info.fd;
    if(data->host_info.fd>maxfd)
        maxfd=data->host_info.fd;
    if(data->chord.TCP.fd>maxfd)
        maxfd=data->chord.TCP.fd;
    for (int i=0;data->rcv_chords[i]->fd!=-1;i++){
        if(data->rcv_chords[i]->fd>maxfd)
            maxfd=data->rcv_chords[i]->fd;
    }
    return maxfd;
}