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

bool find_service(char *serviceName, char *hostname, char *port)
{
	bool found = false;
	int i = 0;
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
		int i = 0, j = 0, k = 0;
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

int main(int argc, char *argv[])
{
	struct stat sb;
	char *init_time = new char[1024];

	strcpy(init_time, "");

	detect_services();
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
	while ((connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) and (ttl<3))
	{
		cout<<"ERROR connecting to service: "<<service<<" host: "<<hostname<<" port: " <<portno<<endl;
		ttl++;
		sleep(60);
	}
	if(ttl == 3)
		pthread_exit(0);
	
	services[*i].socket = sockfd;

	if(pthread_create(&listen_thread, NULL, listen_handler, i)<0)
	{
		perror("could not create listening thread");
	}

	pthread_join(listen_thread, NULL);

}

void *listen_handler(void *param)
{
	int r;
	int *i = (int *)param;
	char server_message[2000], *sending_buffer = new char[2048];
	int server_sock = services[*i].socket;

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
