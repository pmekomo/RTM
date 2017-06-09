/******************************************************************************************************************************
Dans ma logique de programmation chaque service est en fait une exécution du binaire server.
Dans ce script on ouvre un port socket sur lequel devront se connecter tous les autres équipements du réseau.
A tous les équipements connectés au socket les informations leurs sont transmises(à partir d'un fichier *.xml assocé au service)

*******************************************************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <fstream>
#include <pthread.h>
#include <vector>
#include "equipement.h"
#include <sys/stat.h>
#include "files.h"
#include "utile.h"
#define CHECK_FILE_TIME 10
#define SERVER_FILE "xmlFiles/final.xml"

using namespace std;

void error(const char *msg)
{
    perror(msg);
    exit(1);
}	

vector <Equipement> clients_tab;
void erase_client(int elem)
{
	unsigned int i = 0;
	while ((clients_tab[i].getNumSock() != elem) && (i<clients_tab.size()))
		i++;
	if(i<clients_tab.size())
		clients_tab.erase(clients_tab.begin() + i);
}
void *listen_handler(void *socket_desc);
void *connection_handler(void *socket_desc);
void *checkfile_handler(void *param);
int main(int argc, char *argv[])
{

//---------------INITIALISATION DES SOCKETS ET CONNNECTION-------------
	int sockfd, portno, *tmp_sock;
	struct sockaddr_in serv_addr;
	if (argc < 2) {
		fprintf(stderr,"ERROR, no port provided\n");
		exit(1);
	}
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) 
		error("ERROR opening socket");
	bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = atoi(argv[1]);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) 
		error("ERROR on binding");
	listen(sockfd,5);
     
	pthread_t client_thread;
	
	tmp_sock = (int *)malloc(sizeof(int));
	*tmp_sock = sockfd;
	//On crée le thread qui va gérer la connexion des clients
	if (pthread_create (&client_thread, NULL, listen_handler, (void*) tmp_sock) < 0)
	{
		perror("could not create clients thread");
		return 1;
	}
	
	//On crée un thread qui va vérifié l'état du fichiers contenant les informations du service
	pthread_t checkfile_thread;
	if (pthread_create (&checkfile_thread, NULL, checkfile_handler, NULL) <0)
	{
		perror("could not create messages thread");
		return 2;
	}
	
	pthread_join(client_thread, NULL);
	pthread_join(checkfile_thread, NULL);
		
	return 0; 
}

//Pour gérer les clients qui se connectent au serveur un client = un thread
void *listen_handler(void *socket_desc)
{
	
	struct sockaddr_in serv_addr, cli_addr;
	int newsockfd, socklh = *(int*)socket_desc, *client_sock;
	
	socklen_t clilen = sizeof(cli_addr);
	
	
	while(newsockfd = accept(socklh, 
                 (struct sockaddr *) &cli_addr, 
                 &clilen))
	{
		pthread_t new_thread;
		
		client_sock = (int *)malloc(sizeof(int));
		*client_sock = newsockfd;
		if (pthread_create (&new_thread, NULL, connection_handler, (void*) client_sock) < 0)
		{
			perror("could not create thread");
			exit(1);
		}
        	puts("Handler assigned");
		if (newsockfd >= 0)
		{
			Equipement eqtmp(newsockfd);
        		clients_tab.push_back(eqtmp);
		}
	}
	if (newsockfd < 0)
		error("ERROR on accept");
}

//pour envoyer un message à un client
void *connection_handler(void *socket_desc)
{
	//Get the socket descriptor
	int sock = *(int*)socket_desc;
	int n;
	char client_message[2000], fileName[1024];

	bzero(client_message, strlen(client_message));

	while((n = read(sock,client_message,2000)) > 0)
	{
		cout<<client_message<<endl;
		if (mystrcmp(client_message, "ecbstate") == 0)
		{
			cout<<"Sending of server file"<<endl;
			if (send_file(sock, SERVER_FILE) > 0)
				cout<<"sending succeeded------"<<endl;
			else
				cout<<"sending failed------"<<endl;
		}
		else
			cout<<"???"<<endl;
		bzero(client_message, strlen(client_message));
	}
	close(sock);
	erase_client(sock);

	if(n == 0)
		puts("Client Disconnected");
	else
		perror("recv failed");

    return 0;
}

void broadcast_data(void)
{
	for (unsigned int i = 0; i<clients_tab.size(); i++)
	{
		write(clients_tab[i].getNumSock(), "ecbstate", 8);
	}
}

//Handler pour vérifier l'état du fichier du service
void *checkfile_handler(void*)
{
	struct stat sb;
	char *init_time = new char[1024];

	strcpy(init_time, "");

while(stat(SERVER_FILE, &sb) != -1)
	{
		if(strlen(init_time) == 0)
			strcpy(init_time, ctime(&sb.st_mtime));
		else
		{
			if(strcmp(init_time, ctime(&sb.st_mtime)) != 0)
			{
				strcpy(init_time, ctime(&sb.st_mtime));
				broadcast_data();
			}
		}

		sleep(CHECK_FILE_TIME);
	}

	perror("stat");
	pthread_exit(NULL);
}
