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

using namespace std;

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int server_sock;
char fileName[1024];


pthread_t msg_thread;
pthread_t listen_thread;
pthread_t checkfile_thread;

void *message_handler(void *socket_desc);
void *listen_handler(void *socket_desc);
void *checkfile_handler(void *arg);
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
	
	tmp_sock = (int*)malloc(2*sizeof(int));
	tmp_sock[0] = sockfd;
	if (argc == 4)
	{
		if (strcmp(argv[3], "nocast") == 0)
			tmp_sock[1] = 1;
	}
	else
		tmp_sock[1] = 0;

	if (pthread_create(&msg_thread, NULL, message_handler, (void*)tmp_sock)<0)
	{
		perror("could not create messages thread");
	}
    
	if(pthread_create(&listen_thread, NULL, listen_handler, (void*)tmp_sock)<0)
	{
		perror("could not create listening thread");
	}

	if (pthread_create(&checkfile_thread, NULL, checkfile_handler, NULL)<0)
		perror("could not create checkfile thread");
        
	pthread_join(msg_thread, NULL);
	pthread_join(listen_thread, NULL);
	pthread_join(checkfile_thread, NULL);
    
	return 0;
}

void *message_handler(void *socket_desc)
{
	
	//Get the socket descriptor
	int *sock = (int*)socket_desc;
	char buffer[256];
	int n;

	//Envoi du type de connection au serveur
	if (sock[1] == 1)
		strcpy(buffer, "nocast");
	else
		strcpy(buffer, "normal");
	n = write(sock[0],buffer,strlen(buffer));
	bzero(buffer, strlen(buffer));

	//Envoi de messages au serveur
	while(n>=0)
	{
		cin>>buffer;
		n = write(sock[0],buffer,strlen(buffer));
		bzero(buffer, strlen(buffer));
	}
	
	if (n < 0) error("ERROR reading from socket");
	close(sock[0]);
	pthread_exit(NULL);
}


void socket_send(int sig)
{
	cout<<"--------states file modified-------"<<endl;
	if (send(server_sock,"eqstate",7, 0) >= 0)
	{
		if (send_file (server_sock, fileName))
			cout<<"send succeeded-------"<<endl;
		else
			cout<<"send failed--------"<<endl;
	}
	else
		cout<<"Server unreachabled"<<endl;

}

void *listen_handler(void *socket_desc)
{
	//Get the socket descriptor
	server_sock = *(int*)socket_desc;
	int r;

	char server_message[2000], *sending_buffer = new char[2048];

	/*struct sigaction sa;

	sa.sa_handler = socket_send;
	sigemptyset(&sa.sa_mask); 
	sa.sa_flags=0;
	sigaction(SIGURG, &sa, 0);*/

	if ((signal(SIGURG, socket_send)) == SIG_ERR)
	{
		perror("signal");
		exit(EXIT_FAILURE);
	}

	//File name
	strcpy(fileName, "xmlFiles/file.xml");

	//Sending Message
	strcpy (sending_buffer, "ok");

	while((r=read(server_sock,server_message,2000))>0)
	{
		cout<<server_message<<endl;
		if (strcmp (server_message, "eqstate") == 0)
		{
			if (send(server_sock,server_message,strlen(server_message), 0) >= 0)
			{
				if (send_file (server_sock, fileName))
					cout<<"send succeeded-------"<<endl;
				else
					cout<<"send failed--------"<<endl;
			}
			else
				cout<<"Server unreachabled"<<endl;
		}
		if (strcmp (server_message, "ecbstate") == 0)
		{
			if (send(server_sock,server_message,strlen(server_message), 0) >= 0)
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

	close(server_sock);

	pthread_exit(NULL);
}

void *checkfile_handler(void* arg)
{
	struct stat sb;
	char *init_time = new char[1024];

	strcpy (init_time, "");
	//to remove warning
	(void) arg;
	
	while(stat("xmlFiles/file.xml", &sb) != -1) 
	{
		if (strlen(init_time) == 0)
			strcpy(init_time, ctime(&sb.st_mtime));
		else
		{
			if (strcmp(init_time, ctime(&sb.st_mtime)) != 0)
			{
				strcpy(init_time, ctime(&sb.st_mtime));
				pthread_kill(listen_thread, SIGURG);
			}
		}

		sleep(120);
	}

	perror("stat");
	pthread_exit(NULL);
}
