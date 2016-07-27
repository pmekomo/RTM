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

using namespace std;

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

void *message_handler(void *socket_desc);
void *listen_handler(void *socket_desc);
int main(int argc, char *argv[])
{

//--------------INITIALISATION DU SOCKET ET CONNECTION------------------
	int sockfd, portno, *tmp_sock;
	struct sockaddr_in serv_addr;
	struct hostent *server;

	if (argc < 3) {
		fprintf(stderr,"usage %s hostname port [connection_type]\n", argv[0]);
		exit(0);
	}
	portno = atoi(argv[2]);
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) 
		error("ERROR opening socket");
	server = gethostbyname(argv[1]);
	if (server == NULL) {
		fprintf(stderr,"ERROR, no such host\n");
		exit(0);
	}
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
	serv_addr.sin_port = htons(portno);
	if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
		error("ERROR connecting");
//**********************************************************************

	pthread_t msg_thread;
	
	tmp_sock = (int*)malloc(sizeof(int));
	*tmp_sock = sockfd;
	if (pthread_create(&msg_thread, NULL, message_handler, (void*)tmp_sock)<0)
	{
		perror("could not create messages thread");
	}
    
	pthread_t listen_thread;
	if(pthread_create(&listen_thread, NULL, listen_handler, (void*)tmp_sock)<0)
	{
		perror("could not create listening thread");
	}
        
	pthread_join(msg_thread, NULL);
	pthread_join(listen_thread, NULL); 
    
	return 0;
}

void *message_handler(void *socket_desc)
{
	
	//Get the socket descriptor
	int sock = *(int*)socket_desc;
	char buffer[256];
	scanf("%s",buffer);
	int n = write(sock,buffer,strlen(buffer));
	bzero(buffer,256);
	n = read(sock,buffer,255);
	cout<<buffer<<endl;
	
	if (n < 0) error("ERROR reading from socket");
	close(sock);
}

void *listen_handler(void *socket_desc)
{
	//Get the socket descriptor
	int sock = *(int*)socket_desc;
	int n;

	char server_message[2000];

	while((n=recv(sock,server_message,2000,0))>0)
	{
		cout<<server_message<<endl;
		bzero(server_message, strlen(server_message));
	}

	if(n==0)
	{
		puts("Server Disconnected");
	}
	else
	{
		perror("recv failed");
	}
	close(sock);
	return 0;
}
