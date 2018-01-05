#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "helpers.h"
#define MAX_CLIENTS	100
int ocupat[BUFLEN];
int pinIncorect[BUFLEN];
//Alocare client
client aloc_client()
{
	client newClient;
	newClient.nume = malloc(12 * sizeof(char));
	newClient.prenume = malloc(12 * sizeof(char));
	newClient.numar_card = malloc(6 * sizeof(char));
	newClient.pin = malloc(4 * sizeof(char));
	newClient.parola = malloc(16 * sizeof(char));
	return newClient;
}
//Functie care intoarce tipul de mesaj
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

	if(strcmp(buffer, "logout") == 0)
		return 2;
    if(strcmp(buffer,"quit\n") == 0)
        return 7;
    if(strcmp(buffer,"logout\n") == 0)
        return 2;
    if(strcmp(buffer,"listsold\n") == 0)
        return 3;

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

//Functie care copiaza o linie citita din fisier intr-o structura de tipul client
client copy_data(char *token)
{
	client newClient = aloc_client();
	
	strcpy(newClient.nume,token);
	token = strtok(NULL," ");
	strcpy(newClient.prenume,token);
	token = strtok(NULL," ");
	strcpy(newClient.numar_card,token);
	token = strtok(NULL," ");
	strcpy(newClient.pin,token);
	token = strtok(NULL," ");
	strcpy(newClient.parola,token);
	token = strtok(NULL," ");
	newClient.sold = atof(token);
	return newClient;

}
//Functie care se uita dupa numarul de card primit ca parametru si returneaza indicele daca a fost gasit, sau BUFLEN +1 daca nu gaseste
int look_for_name(client *listOfClients, char *numarCard, int numberOfClients)
{
	int i;
	for(i = 0 ; i < numberOfClients; i++)
	{
		
		if(strcmp(listOfClients[i].numar_card, numarCard) == 0)
		{
			return i; //Returnez indicele
		}
	
	}

return BUFLEN + 1; //Daca nu il gaseste
}
//Functie intoarce codul de eroare sau 0 daca a gasit numele 
int preluc_login(char *buffer, int sockfd, client *listOfClients, int numberOfClients)
{
	
	int i;
	char *tokens;
	tokens = strtok(buffer," "); //Deja stiu ca e login, nu mai am nevoie de asta
	tokens = strtok(NULL," "); //Obtin numarul de card
	char buff[BUFLEN], buff2[BUFLEN];
	strcpy(buff, tokens);
	tokens = strtok(NULL," "); //Obtin pin-ul
	strcpy(buff2, tokens);
	buff2[strlen(buff2) - 1] = '\0'; // scap de \n de la sfarsit
	int indice = look_for_name(listOfClients, buff, numberOfClients);
	if(pinIncorect[indice] >= 3)
		return -5;
	if( indice == BUFLEN +1) { //Trebuie sa intoarcem -4
		return -4;
	}
	else if(strcmp(listOfClients[indice].pin, buff2) != 0) { //Daca pinul nu este corect, intoarcem -3 sau -5
			pinIncorect[indice] ++;
			if(pinIncorect[indice] == 3)
				return -5;
			return -3;
		}
	else if(ocupat[indice] == 1) {
		return -2;
	}
	
	else  {//Intoarcem 0
		listOfClients[indice].sockfd = sockfd;
		pinIncorect[indice] = 0;
		ocupat[indice] = 1;
		return 0;
	}
}
//Functie de parsare input
client* read_from_file(FILE *f, int numberOfClients)
{
	int i;
	client *listOfClients = malloc(numberOfClients * sizeof(client));
	char *token;
	for(i = 0 ; i< numberOfClients; i++)
	{
		char buff[255];
		fgets(buff, 255, f);
		token = strtok(buff, " ");
		listOfClients[i] = copy_data(token);
	}	
return listOfClients;
}
void error(char *msg)
{
	perror(msg);
	exit(1);
}
void show_info(client *listOfClients, int numberOfClients)
{
	int i;
	for(i = 0 ; i < numberOfClients; i++)
	{
		printf("%s %s %s %s %s %f \n", listOfClients[i].nume, listOfClients[i].prenume,
									  listOfClients[i].numar_card, listOfClients[i].pin,
									  listOfClients[i].parola, listOfClients[i].sold);
	}
}
//Functie care porneste un socket de tipul TCP
int open_TCP()
{
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0)
		printf("-10");
	return sockfd;
}
//Functie care porneste un socket de tipul UDP
int open_UDP()
{
	int sockfd = socket(PF_INET, SOCK_DGRAM, 0 );
	if(sockfd < 0)
		printf("-10");
	return sockfd;
}
//Functie care completeaza structura
struct sockaddr_in completeStruct(int portno)
{
	struct sockaddr_in serv_addr;
	memset((char *) &serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY; //Foloseste adresa IP a masinii
	serv_addr.sin_port = htons(portno);
	return serv_addr;
}
//Functie care cauta in vectorul de structuri de tipul client, clientul cu care se comunica pe socketul sockfd
int look_for_sock(int sockfd, client *listOfClients, int numberOfClients)
{
	int i;
	for(i = 0; i < numberOfClients; i++)
	{
		if(listOfClients[i].sockfd == sockfd) //Inseamna ca sockfd-ul cautat exista
		{
			listOfClients[i].sockfd = BUFLEN +1;
			ocupat[i] = 0; //Nu mai e ocupat
			return 1;
		}
	}
	return 0;
}
//Functie care returneaza sold-ul
double get_sold(int sockfd, client *listOfClients, int numberOfClients)
{
	int i;
	double rez;
	for(i = 0; i < numberOfClients; i++)
	{
		if(listOfClients[i].sockfd == sockfd)
		{
			return listOfClients[i].sold;
		}
	}
	return -1;

}
//Functie care face operatia pentru extragere de bani
int preluc_getmoney(char *buffer, client *listOfClients, int numberOfClients, int sockfd)
{
	char * token;
	token = strtok(buffer," ");
	token = strtok(NULL," "); //Obtinem suma
	char buffer2[BUFLEN];
	strcpy(buffer2, token);
	int sum = atoi(buffer2);
	if(sum % 10 != 0)
		return -9;
	int i;

	for(i = 0 ; i < numberOfClients; i++)
	{
		if(listOfClients[i].sockfd == sockfd)
		{
			if(listOfClients[i].sold < sum) // Daca nu sunt bani suficient
				return -8;
			listOfClients[i].sold -= sum;
			return BUFLEN + 1;
		}
	}
	return BUFLEN + 1;
}
//Functie care face operatia pentru adaugare de bani
void put_money(char *buffer, client *listOfClients, int numberOfClients, int sockfd)
{
	char *token;
	token = strtok(buffer," ");
	token = strtok(NULL," ");
	char buffer2[BUFLEN];
	strcpy(buffer2, token);
	double sum = atof(buffer2);
	int i;
	for(i = 0; i< numberOfClients; i++)
	{
		if(listOfClients[i].sockfd == sockfd)
		{
			listOfClients[i].sold += sum;
		}
	}
}
int main(int argc, char *argv[])
{
	FILE *f;
	int sockTCPfd, sockUDPfd, newsockfd, portno, i, clilen, n;
	struct sockaddr_in serv_addrTCP;
	struct sockaddr_in serv_addrUDP;
	struct sockaddr_in clientAdress;
	struct sockaddr_in cli_addr;
	char buffer[BUFLEN];
	char bufferCoppy[BUFLEN];
	char *tokens;
	char recieve_data[BUFLEN];
	int ok, numberOfBytes;
	int address_length = sizeof(struct sockaddr);
	fd_set read_fds, tmp_fds, udp;
	//Citire din fisier
	f = fopen(argv[2], "r");
	char buff[255];
	fgets(buff, 255, f);
	int numberOfClients = atoi(buff);
	client *listOfClients = malloc(numberOfClients * sizeof(client));
	listOfClients = read_from_file(f, numberOfClients);
	//Pornesc socket-ul TCP
	sockTCPfd = open_TCP();
	//Pornesc socket-ul UDP
	sockUDPfd = open_UDP();
	//Numarul portului
	portno =  atoi(argv[1]);
	//golim multimea de descriptori de citire(read_fs) si multimea tmp_fds
	FD_ZERO(&read_fds);
	FD_ZERO(&tmp_fds);
	FD_ZERO(&udp);
	//Obtinere bind pt TCP
	serv_addrTCP = completeStruct(portno);
	if (bind(sockTCPfd, (struct sockaddr *) &serv_addrTCP, sizeof(struct sockaddr)) < 0) 
              printf("-10");
	//Obtinere bind pt UDP
  	serv_addrUDP = completeStruct(portno);
    	if (bind(sockUDPfd, (struct sockaddr *) &serv_addrUDP, sizeof(struct sockaddr)) < 0) 
              printf("-10");
	//Asculta pe TCP
   	listen(sockTCPfd, numberOfClients);
    	//Adaugam noul file descriptor
    	FD_SET(sockTCPfd, &read_fds); //TCP
   		FD_SET(sockUDPfd, &read_fds); //UDP
   		FD_SET(sockUDPfd, &udp); //adaug socket-ul udp si in multimea cu socket-uri udp
 	int fdmax = sockTCPfd;	
 	if(sockUDPfd > sockTCPfd) {
 		fdmax = sockUDPfd;
 	}
 	else
 		fdmax = sockTCPfd;
 	printf("%d", fdmax);
 	while (1) {
		tmp_fds = read_fds; 
		if (select(fdmax + 1, &tmp_fds, NULL, NULL, NULL) == -1) 
			error("ERROR in select");
		for(i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &tmp_fds)){

			if(FD_ISSET(i, &udp)) { //Daca e UDP
					
					//Daca e socket UDP
					numberOfBytes = recvfrom(sockUDPfd, recieve_data, BUFLEN, 0, 
						   					 (struct sockaddr *)&clientAdress, &address_length);	
					//printf("am primit%s", recieve_data);
					tokens = strtok(recieve_data," ");
					tokens = strtok(NULL," ");
					strcpy(buffer, tokens); //Obtinem numarul de card
					strcpy(buff, "UNLOCK> Trimite parola secreta");
					sendto(sockUDPfd, buff, strlen(buff), 0, (struct sockaddr *)&clientAdress, sizeof(struct sockaddr));
					int n =recvfrom(sockUDPfd, recieve_data, BUFLEN, 0, (struct sockaddr *)&clientAdress, &address_length);
					int k = look_for_name(listOfClients, buffer, numberOfClients);
					recieve_data[n - 1] = '\0';
					
					if(strcmp(listOfClients[k].parola, recieve_data) == 0)
					{
						strcpy(buff, "UNLOCK> Client deblocat");
						pinIncorect[k] = 0;
					}
					else
					{
						strcpy(buff,"UNLOCK -7x : Deblocare esuata");
					}
					sendto(sockUDPfd, buff, strlen(buff), 0, (struct sockaddr *)&clientAdress, sizeof(struct sockaddr));
	
			} else { //Inseamna ca e TCP
				
				
				if (i == sockTCPfd) {
					
					// a venit ceva pe socketul inactiv(cel cu listen) = o noua conexiune
					// actiunea serverului: accept()
					clilen = sizeof(cli_addr);
					if ((newsockfd = accept(sockTCPfd, (struct sockaddr *)&cli_addr, &clilen)) == -1) {
						error("ERROR in accept");
					} 
					else {
						//adaug noul socket intors de accept() la multimea descriptorilor de citire
						FD_SET(newsockfd, &read_fds);
						if (newsockfd > fdmax) { 
							fdmax = newsockfd;
						}
					}
					printf("Noua conexiune de la %s, port %d, socket_client %d\n ", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port), newsockfd);
				}
					
				else {
					// am primit date pe unul din socketii cu care vorbesc cu clientii
					//actiunea serverului: recv()
					memset(buffer, 0, BUFLEN);
					if ((n = recv(i, buffer, sizeof(buffer), 0)) <= 0) {
				
						if (n == 0) {
							//conexiunea s-a inchis
							printf("selectserver: socket %d hung up\n", i);
						} else {
							error("ERROR in recv");
						}
						close(i); 
						FD_CLR(i, &read_fds); // scoatem din multimea de citire socketul
					} 
					
					else { 
						//recv intoarce >0

						
						strcpy(bufferCoppy, buffer); //Fac copie pt ca in factory fac strtok si stric string-ul
						if(factory(bufferCoppy) == 1) { //Primesc login
							//printf("alalbala%s", listOfClients[0].pin);
							strcpy(bufferCoppy, buffer); //Fac iar copie
							ok = preluc_login(bufferCoppy, i, listOfClients, numberOfClients); //Functie care prelucreaza un login
							//Acum trimit inapoi mesaj
							if(ok == -3) {
								strcpy(bufferCoppy,"-3 : Pin gresit");
							} else if(ok == -4) {
								strcpy(bufferCoppy,"-4 : Numar card inexistent");
							} 
							else if(ok == -2) {
								strcpy(bufferCoppy, "-2 : Sesiune deja deschisa");
							}
							else if(ok == -5) {
								strcpy(bufferCoppy, "-5 : Card blocat");
							}
							else {
								tokens = strtok(buffer," "); //Obtin numarul de card
								tokens = strtok(NULL," ");
								strcpy(bufferCoppy, tokens); //Aici e numarul de crad
								int indice = look_for_name(listOfClients, bufferCoppy, numberOfClients);
								strcpy(buffer, "Welcome ");
								strcat(buffer, listOfClients[indice].nume);
								strcat(buffer, " ");
								strcat(buffer, listOfClients[indice].prenume);
								strcpy(bufferCoppy, buffer);
							}
						//Acum trebuie sa trimit mesajul		
							n = send(i, bufferCoppy, strlen(bufferCoppy), 0);
							if(n < 0)
								printf("ERROR");
						}
						if(factory(bufferCoppy) == 2) {//Primesc logout
							int ok = look_for_sock(i,listOfClients, numberOfClients);
							if(ok == 0){
								strcpy(buffer, "-1 : Clientul nu este autentificat");
							} else{
								strcpy(buffer, "Deconectare de la bancomat");
							}
							n = send(i, buffer, strlen(buffer), 0);
							if(n < 0)
								printf("ERROR");

						}
						if(factory(bufferCoppy) == 3) //Daca e listSold
						{
							double rez = get_sold(i, listOfClients, numberOfClients);
							strcpy(bufferCoppy, "ATM> ");
							sprintf(buffer, "%f", rez);
							strcat(bufferCoppy, buffer);
							n = send(i, bufferCoppy, strlen(bufferCoppy), 0);
							if(n < 0)
								printf("ERROR");

						}
						if(factory(bufferCoppy) == 4) //Daca e getMonney
						{
							strcpy(bufferCoppy, buffer);
							int ok = preluc_getmoney(bufferCoppy, listOfClients, numberOfClients, i);
							if(ok == -9)
							{
								strcpy(bufferCoppy, "-9 : Suma nu este multiplu de 10");
							} else if(ok == -8)
							{
								strcpy(bufferCoppy, "-8 : Fonduri insuficiente");		
							}
							else {
								tokens = strtok(buffer, " ");
								tokens = strtok(NULL," ");
								strcpy(bufferCoppy, tokens);
								bufferCoppy[strlen(bufferCoppy) -1] = '\0';
								strcpy(buffer, "Suma ");
								strcat(buffer, bufferCoppy);
								strcat(buffer," ");
								strcat(buffer, "retrasa cu succes");
								strcpy(bufferCoppy, buffer);
							}
							n = send(i, bufferCoppy, strlen(bufferCoppy), 0);
							if(n < 0)
								printf("ERROR");
						}
						if(factory(bufferCoppy) == 5) //Daca e putmoney
						{
							strcpy(bufferCoppy, buffer);
							put_money(bufferCoppy, listOfClients, numberOfClients, i);
							strcpy(buffer, "Suma depusa cu succes");
							n = send(i, buffer, strlen(buffer), 0);
							if(n < 0)
								printf("ERROR");
						}
						if(factory(bufferCoppy) == 7) //Daca e quit
						{	
							close(i); //Inchidem conexiunea 
							FD_CLR(i, &read_fds); 
							strcpy(buffer, "OK");
							n = send(i, buffer, strlen(buffer), 0);
							if(n < 0)
								printf("ERROR");

						}
					}
				} 
			}
			}
		}
     }
    fclose(f);
}
