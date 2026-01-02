/************************************************************************************
Il s'agit du programme qui va permettre aux équipements d'accéder aux services des autres équipements.
L'équipement établit plusieurs connexion afin d'accéder aux différents services proposés par chaque équipement.
La connexion consiste en l'ouverture d'un socket qui permettra à l'équipement de recevoir les informations
des différents services du réseau.
******************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include <pthread.h>
#include "files.h"
#include <time.h>
#include <sys/stat.h>
#include <signal.h>
#include <errno.h>
#include <vector>
#include "service.h"
#define CHECK_FILE_TIME 60
#define SERVICES_FILE "tmp.txt"

using namespace std;

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

char fileName[1024];


pthread_t listen_thread;
vector <Service> services;
void *listen_handler(void *param);
void *serv_handler(void *param);

//Fonction permettant de rechercher un service en fonction dans le vecteur contenant l'ensemble des services
bool find_service(char *serviceName, char *hostname, char *port)
{
	bool found = false;
	unsigned int i = 0;
	while((i<services.size()) and !found)
	{
		if ((services[i].getServiceName().compare(serviceName) == 0) and
			(services[i].getHostname().compare(hostname) == 0) and
			(services[i].getPort().compare(port) == 0))
		{
			found = true;
		}
		i++;
	}
	return found;
}

//Fonction permettant le remplissage du vecteur de services en fonction des services disponibles sur le réseau
//Ces services sont dispo dans le fichier SERVICES_FILE obtenu au préalable par l'exécution du script avahi.py
void detect_services()
{
	FILE *fd = fopen(SERVICES_FILE, "r");
	char * line = NULL;
	size_t len = 0;
	ssize_t read;
	if (fd == NULL)
		exit(EXIT_FAILURE);
	
        char ch[3][1024];
	while((read = getline(&line, &len, fd)) != -1)
	{
		unsigned int i = 0, j = 0, k = 0;
		while(i <strlen(line))
		{
			if ((line[i] != ';') and (line[i] != '\n'))
			{
				ch[j][k++] = line[i];
			}
			else
			{
				j++;
				k = 0;
			}
			i++;
		}
		//A chaque service on associe un thread bien-sûr on vérifie si le service n'est pas déjà pris en compte
		if (!find_service(ch[1], ch[0], ch[2]))
		{
			Service serv(ch[1], ch[0], ch[2]);
			services.push_back(serv);
			pthread_t serv_thread;
			cout<<"service: "<<ch[1]<<" host: "<<ch[0]<<" port: "<<ch[2]<<endl;
			int *pos = new int();
			*pos = services.size() - 1;
			if(pthread_create(&serv_thread, NULL, serv_handler, (void *)pos)<0)
				perror("Impossible to create a service thread");
		}
		bzero(ch[0], strlen(ch[0]));
		bzero(ch[1], strlen(ch[1]));
		bzero(ch[2], strlen(ch[2]));
	}
	fclose(fd);
}

int main(void)
{
	struct stat sb;
	char *init_time = new char[1024];

	strcpy(init_time, "");
	detect_services();
	
	//La boucle permet de vérifier tous les CHECK_FILE_TIME si le fichier contenant les services a été modifié
	//Si c'est le cas on relance la fonction detect_services()
	while(stat(SERVICES_FILE, &sb) != -1)
	{
		if(strlen(init_time) == 0)
			strcpy(init_time, ctime(&sb.st_mtime));
		else
		{
			if(strcmp(init_time, ctime(&sb.st_mtime)) != 0)
			{
				strcpy(init_time, ctime(&sb.st_mtime));
				detect_services();
			}
		}

		sleep(CHECK_FILE_TIME);
	}
    
	return 0;
}

//Ce handler permet l'ouverture d'un socket sur le service associé au thread
void *serv_handler(void *param)
{
	int *i = (int *)param;
	int sockfd;
	string service(services[*i].getServiceName());
	string hostname(services[*i].getHostname());
	int portno = atoi(services[*i].getPort().c_str());
	struct sockaddr_in serv_addr;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		error("ERROR opening socket");
	struct hostent *server = gethostbyname(hostname.c_str());
	if (server == NULL)
	{
		cout<<"ERROR, no such host: "<<hostname<<endl;
		pthread_exit(0);
	}
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
	serv_addr.sin_port = htons(portno);
	int ttl = 0;

	//On fait 3 tentatives de connexion de 60 secondes
	//Si au bout des tentatives on arrive pas à se connecter le thread est stoppé
	while ((connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) and (ttl<3))
	{
		cout<<"ERROR connecting to service: "<<service<<" host: "<<hostname<<" port: " <<portno<<endl;
		ttl++;
		sleep(60);
	}
	if(ttl == 3)
		pthread_exit(0);
	
	//On remplit le champ socket du service
	services[*i].socket = sockfd;

	char server_message[2000];
	int server_sock = services[*i].socket;
	int r;
	while((r=read(sockfd,server_message,8))>0)
	{
		cout<<server_message<<endl;
		if (strcmp (server_message, "ecbstate") == 0)
		{
			if (send(sockfd,server_message,strlen(server_message), 0) >= 0)
			{
				if (receive_file(server_sock, (char*)"xmlFiles/myecb.xml") > 0)
					cout<<"reception succeeded-------"<<endl;
				else
					cout<<"reception failed------"<<endl;
			}
			else
				cout<<"Server unreachabled"<<endl;
		}
		bzero(server_message, strlen(server_message));
	}

	if(r==0)
		puts("Server Disconnected");
	else
		perror("recv failed");

	close(sockfd);

	pthread_exit(NULL);
}
