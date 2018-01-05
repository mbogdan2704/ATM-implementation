#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <unistd.h>
#define BUFLEN 256
char currentNumber[BUFLEN];
void error(char *msg)
{
    perror(msg);
    exit(0);
}
//Functie care intoarce un numar specific pentru fiecare operatie in aprte
// 1 - LOGIN
// 2 - LOGOUT
// 3 - LISTSOLD
// 4 - GETMONEY
// 5 - PUTMONEY
// 6 - UNLOCK
// 7 - QUIT
// 8 - NONE
int factory(char *buffer)
{
    if(strcmp(buffer,"quit\n") == 0)
        return 7;
    if(strcmp(buffer,"logout\n") == 0)
        return 2;
    if(strcmp(buffer,"listsold\n") == 0)
        return 3;
    if(strcmp(buffer, "unlock\n") == 0)
        return 6;

    char * token;
    token = strtok(buffer, " ");
    char buff[BUFLEN];
    strcpy(buff, token);
    if(strcmp(buff, "login") == 0) { 
        return 1;
    }
    if(strcmp(buff, "logout") == 0) {
        return 2;
    }
    if(strcmp(buff, "listsold") == 0) {
        return 3;
    }
    if(strcmp(buff, "getmoney") == 0) {
        return 4;
    }
    if(strcmp(buff, "putmoney") == 0) {
        return 5;
    }
    if(strcmp(buff, "unlock") == 0) {
        return 6;
    }
    if(strcmp(buff, "quit") == 0) { 
        return 7;
    }
    return 8;
}

int main(int argc, char *argv[])
{
    int sesiunePornita = 0;
    int trimiteMesaj = 0;
    int sockfdTCP, n, sockfdUDP;
    struct sockaddr_in serv_addrTCP;
    struct sockaddr_in serv_addrUDP;
    struct hostent *server;
    FILE *f;
    char buffer[BUFLEN];
    char bufferCoppy[BUFLEN];
    char *token, *token2;
    if (argc < 3) {
       printf("-10");
       exit(0);
    }  
    //Deschid socketul TCP
	sockfdTCP = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfdTCP < 0) 
        printf("-10");

    sockfdUDP = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfdUDP < 0)
        printf("ERROR");
    
    serv_addrTCP.sin_family = AF_INET;
    serv_addrTCP.sin_port = htons(atoi(argv[2]));
    inet_aton(argv[1], &serv_addrTCP.sin_addr);
    
    serv_addrUDP.sin_family = AF_INET;
    serv_addrUDP.sin_port = htons(atoi(argv[2]));
    inet_aton(argv[1], &serv_addrUDP.sin_addr);
    

    
    if (connect(sockfdTCP,(struct sockaddr*) &serv_addrTCP,sizeof(serv_addrTCP)) < 0) 
        printf("-10: Error");    
    
    //Deschidere fisier log
    char buffer2[BUFLEN];
    strcpy(buffer, "client-");
    int pid = getpid();
    sprintf(buffer2, "%d", pid);
    strcat(buffer, buffer2);
    strcat(buffer, ".log");
    f = fopen(buffer, "w");
    int  address_length = sizeof(struct sockaddr);
    while(1) {
  		//citesc de la tastatura
    	memset(bufferCoppy, 0 , BUFLEN);
        memset(buffer, 0 , BUFLEN);
        fgets(buffer, BUFLEN-1, stdin);
        fprintf(f, "%s", buffer);
        strcpy(bufferCoppy, buffer);
          
        

        if(factory(bufferCoppy) == 7) { //daca e quit
              strcpy(buffer,"quit");
              n = send(sockfdTCP,buffer,strlen(buffer), 0);
              if (n < 0) 
                    printf("ERROR writing to socket");
              recv(sockfdTCP, buffer, sizeof(buffer), 0);
            close(sockfdTCP);
            break;
        }
        //Pt login
        if(factory(bufferCoppy) == 1) {
            if(sesiunePornita == 1) {
                trimiteMesaj = 0;
                printf("-2: Sesiune deja deschisa\n");
                fprintf(f, "-2: Sesiune deja deschisa\n");
            } else {
                
                trimiteMesaj = 1;
                sesiunePornita = 1;
                char buffer3[BUFLEN];
                strcpy(buffer3, buffer);
                token = strtok(buffer3, " ");
                token = strtok(NULL," ");
                strcpy(currentNumber, token);
              
            }
        }
        strcpy(bufferCoppy, buffer);
        //Pt logout
        if(factory(bufferCoppy) == 2) {
            if(sesiunePornita == 0) {
                printf("-1: Clientul nu este autentificat\n");
                fprintf(f, "-1: Clientul nu este autentificat\n\n");
                trimiteMesaj = 0;
            }
            else {
            sesiunePornita = 0;
            trimiteMesaj = 1;
            }
        }
        strcpy(bufferCoppy, buffer);
        if(factory(bufferCoppy) == 3) //Daca e listsold
        {
            if(sesiunePornita == 0) //In cazul in care dam ListSold si nu e niciun utilizator autentificat
            {
                printf("-1: Clientul nu este autentificat\n");
                fprintf(f, "-1: Clientul nu este autentificat\n\n");
                trimiteMesaj = 0;

            }   
            else {
                trimiteMesaj = 1;
            }
        }
        strcpy(bufferCoppy, buffer);
        if(factory(bufferCoppy) == 4) //Daca e getMoney
        {
            if(sesiunePornita == 0) //In cazul in care dam ListSold si nu e niciun utilizator autentificat
            {
                printf("-1: Clientul nu este autentificat\n");
                fprintf(f, "-1: Clientul nu este autentificat\n\n");
                trimiteMesaj = 0;

            }   
            else {
                trimiteMesaj = 1;
            }
        }
        strcpy(bufferCoppy, buffer);
        if(factory(bufferCoppy) == 5) //Daca e putMoney
        {
            if(sesiunePornita == 0) 
            {
                printf("-1: Clientul nu este autentificat\n");
                fprintf(f, "-1: Clientul nu este autentificat\n\n");
                trimiteMesaj = 0;

            }   
            else {
                trimiteMesaj = 1;
            }
        }
        if(factory(bufferCoppy) == 6) // Daca e unlock, trebuie sa trimitem pe UDP
        {
            
                trimiteMesaj = 0;
                buffer[strlen(buffer) -1] = '\0';
                strcat(buffer, " ");
                strcat(buffer, currentNumber);
                sendto(sockfdUDP, buffer, strlen(buffer), 0, (struct sockaddr *)&serv_addrUDP, sizeof(struct sockaddr));
                recvfrom(sockfdUDP, buffer, sizeof(buffer), 0, (struct sockadd *)&serv_addrUDP, address_length);
                printf("%s\n", buffer);
                fprintf(f, "%s\n", buffer);
                fgets(buffer, BUFLEN-1, stdin);
                fprintf(f,"%s", buffer);
                sendto(sockfdUDP, buffer, strlen(buffer), 0, (struct sockaddr *)&serv_addrUDP, sizeof(struct sockaddr));
                memset(buffer, 0 , BUFLEN);
                n = recvfrom(sockfdUDP, buffer, BUFLEN, 0, (struct sockadd *)&serv_addrUDP, address_length);
                buffer[n-1] = '\0';
                printf("%s\n", buffer);
                fprintf(f, "%s\n\n\n", buffer);
            

        }
        if(trimiteMesaj == 1) { 
 
    	   //trimit mesaj la server
              n = send(sockfdTCP,buffer,strlen(buffer), 0);
    	      if (n < 0) 
        	        printf("ERROR writing to socket");
            //Primesc mesajul inapoi
            recv(sockfdTCP, buffer, sizeof(buffer), 0);
            strcpy(bufferCoppy, buffer);
            token = strtok(buffer, " ");
            if(strcmp(token,"ATM>") != 0) {
                fprintf(f, "ATM> %s\n\n\n", bufferCoppy);
                printf("ATM> %s\n", bufferCoppy);
            }
            else
            {
                fprintf(f, "%s\n\n\n", bufferCoppy);
                printf("%s\n", bufferCoppy);
            }

          
            //Daca e -3,-4,-5 sau Deconectare inseamna ca sesiunea nu s-a deschis
            strcpy(buffer, token);
            if(strcmp(buffer, "-2") == 0 ||strcmp(buffer, "-3") == 0 || strcmp(buffer, "-4") == 0 || strcmp(buffer, "-5") == 0)
            {
                sesiunePornita = 0;
            }
            token = strtok(NULL, " ");
            strcpy(buffer, token);
            //Daca am primti Deconectare
            if(strcmp(buffer, "Deconectare") == 0)
            {
                sesiunePornita = 0;
            }
        }
    }
    fclose(f);
    return 0;
}


