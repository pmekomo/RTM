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

using namespace std;

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int server_sock;
char fileName[1024];


pthread_t listen_thread;
vector <Service> services;
void detect_services()
{
	FILE *fd = fopen("tmp.txt", "r");
	char * line = NULL;
	size_t len = 0;
	ssize_t read;
	if (fd == NULL)
		exit(EXIT_FAILURE);
	
        char ch[1024][1024];
	while((read = getline(&line, &len, fd)) != -1)
	{
		int i = 0, j = 0, k = 0;
		while(i <strlen(line))
		{
			if (line[i] != ';')
			{
				if (line[i] != '\n')
					if (j == 0)
						ch[j][k++] = line[i];
			}
			else
			{
				j++;
				k = 0;
			}
			i++;
		}

		for (int i = 0; i<services.size(); i++)
		{
			if ((services[i].getServiceName().compare(ch[1]) != 0) and 
				(services[i].getHostname().compare(ch[0]) != 0) and
				(services[i].getPort().compare(ch[2]) != 0))
			{
				Service serv(ch[1], ch[0], ch[2]);
				services.push_back(serv);
			}
		}
	}
	fclose(fd);
}
void *listen_handler(void *socket_desc);
int main(int argc, char *argv[])
{

//--------------INITIALISATION DU SOCKET ET CONNECTION------------------
	int sockfd, portno, tmp_sock;
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
	
	server_sock = sockfd;
    
	if(pthread_create(&listen_thread, NULL, listen_handler, NULL)<0)
	{
		perror("could not create listening thread");
	}
        
	pthread_join(listen_thread, NULL);
    
	return 0;
}

void *listen_handler(void *socket_desc)
{
	int r;
	(void) socket_desc;
	char server_message[2000], *sending_buffer = new char[2048];


	while((r=read(server_sock,server_message,8))>0)
	{
		cout<<server_message<<endl;
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
