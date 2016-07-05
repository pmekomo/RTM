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

using namespace std;

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

vector <int> clients_tab;
void erase_client(int elem)
{
	int i = 0;
	while ((clients_tab[i] != elem) && (i<clients_tab.size()))
		i++;
	if(i<clients_tab.size())
		clients_tab.erase(clients_tab.begin() + i);
}
void *listen_handler(void *socket_desc);
void *connection_handler(void *socket_desc);
void *message_handler(void *param);
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
	if (pthread_create (&client_thread, NULL, listen_handler, (void*) tmp_sock) < 0)
	{
		perror("could not create clients thread");
		return 1;
	}
	
	pthread_t msg_thread;
	if (pthread_create (&msg_thread, NULL, message_handler, NULL) <0)
	{
		perror("could not create messages thread");
		return 2;
	}
	
	pthread_join(client_thread, NULL);
	pthread_join(msg_thread, NULL);
		
	return 0; 
}

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
            exit;
        }
        puts("Handler assigned");
        clients_tab.push_back(newsockfd);
	}
	if (newsockfd < 0)
		error("ERROR on accept");
}

void *connection_handler(void *socket_desc)
{
	//Get the socket descriptor
	int sock = *(int*)socket_desc;
	int n;

	char sendBuff[100], client_message[2000];

	while((n=recv(sock,client_message,2000,0))>0)
    {
		send(sock,client_message,n,0);
	}
	close(sock);
	erase_client(sock);

	if(n==0)
	{
		puts("Client Disconnected");
	}
	else
	{
		perror("recv failed");
	}
    return 0;
}

void *message_handler(void *param)
{
	char buffer[255];
	do
	{
		cin>>buffer;
		for (int i = 0; i<clients_tab.size(); i++)
		{
			if (send(clients_tab[i],buffer,strlen(buffer), 0)<0)
				perror("error of broadcasting message");
		}
	}while(1);
	
}
