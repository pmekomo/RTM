#include "files.h"


int send_file(int socket, char *fileName){

	FILE *file;
	int size, read_size, stat;
	char send_buffer[10240], read_buffer[256];

	file = fopen(fileName, "rb");
	
	//Getting file Size   
	if(file == NULL) {
		printf("Error Opening file");
		return 0;
	} 

	//Obtention de la taille du fichier
	fseek(file, 0, SEEK_END); //on met le pointeur à la fin du fichier
	size = ftell(file); //On obtient la valeur qui est la taille en octet
	fseek(file, 0, SEEK_SET); //on remet le pointeur à la position initiale
	printf("Total file size: %i\n",size);

	//Send file Size
	printf("Sending file Size\n");
	write(socket, (void *)&size, sizeof(int));

	//Send file as Byte Array
	printf("Sending file as Byte Array\n");

	do { //Read while we get errors that are due to signals.
		stat=read(socket, &read_buffer , 255);
		printf("Bytes read: %i\n",stat);
	} while (stat < 0);

	printf("Received data in socket\n");
	printf("Socket data: %c\n", read_buffer);

	while(!feof(file)) {
	//Read from the file into our send buffer
		read_size = fread(send_buffer, 1, sizeof(send_buffer)-1, file);

		//Send data through our socket 
		do{
			stat = write(socket, send_buffer, read_size);  
		}while (stat < 0);

		//Zero out our send buffer
		bzero(send_buffer, sizeof(send_buffer));
	}
	return 1;
}

int receive_file(int socket, char *fileName)
{ 

	int buffersize = 0, recv_size = 0,size = 0, read_size, write_size, packet_index =1,stat;

	char imagearray[10241],verify = '1';
	FILE *file;

	//Find the size of the file
	do{
		stat = read(socket, &size, sizeof(int));
	}while(stat<0);

	char buffer[] = "Got it";
	//Send our verification signal
	do{
		stat = write(socket, &buffer, sizeof(int));
		printf("%i----\n",stat);
	}while(stat<0);

	printf("Reply sent\n");
	printf(" \n");

	file = fopen(fileName, "wb");

	if( file == NULL) {
		printf("Error has occurred. file file could not be opened\n");
		return -1;
	 }

	//Loop while we have not received the entire file yet


	int need_exit = 0;
	struct timeval timeout = {10,0};

	fd_set fds;
	int buffer_fd, buffer_out;

	while(recv_size < size) {

		FD_ZERO(&fds);
		FD_SET(socket,&fds);

		buffer_fd = select(FD_SETSIZE,&fds,NULL,NULL,&timeout);

		if (buffer_fd < 0)
			printf("error: bad file descriptor set.\n");

		if (buffer_fd == 0)
			printf("error: buffer read timeout expired.\n");

		if (buffer_fd > 0)
		{
			do{
				read_size = read(socket,imagearray, 10241);
			}while(read_size <0);

			printf("Packet size: %i\n",read_size);


			//Write the currently read data into our file file
			 write_size = fwrite(imagearray,1,read_size, file);
			 printf("Written file size: %i\n",write_size); 

			if(read_size !=write_size) {
				 printf("error in read write\n");    
			}


			//Increment the total number of bytes read
			recv_size += read_size;
			printf("Total received file size: %i\n",recv_size);
			printf(" \n");
			printf(" \n");
		}

	}


	fclose(file);
	printf("file successfully Received!\n");
	return 1;
}
