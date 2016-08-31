#ifndef FILE_H
#define FILE_H
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>
#include <fstream>

int send_file(int socket, char *fileName);

int receive_file(int socket, char *fileName);

#endif
