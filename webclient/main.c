#include <stdio.h>
#include <stdlib.h>     // exit, atoi
#include <errno.h>		// errno
#include <sys/socket.h> // socket, bind, listen, connect, accept, send, recv
#include <netinet/in.h> // sockaddr_in
#include <netdb.h>      // hostent (hostname), gethostbyname
#include <string.h>     // memset


#include "helpers.h"

#define BUFFER_SIZE 2000


int main(int argc, char* argv[]) {
	struct sockaddr_in server;
	struct hostent* hostname;
	data_byte buffer[BUFFER_SIZE] = {0};
	response_type response = RES_DEFAULT;
	size_t i;
	size_t data_packets = 1;
	size_t bytes_read;
	FILE* f;

	transaction_header header;

	sockid_t sockid = socket(PF_INET, SOCK_STREAM, 0);

	if (argc != 4)
    {
        fprintf(stderr, "Usage: %s [hostname] [port] [item]\n", argv[0]);
        exit(-1);
    }

	if(!(f = fopen(argv[3], "r"))) {
		if(errno == ENOENT)
			fprintf(stderr, "Could not find file %s\n", argv[3]);
		exit(-1);
	}
	fseek(f, 0, SEEK_END);
	data_packets = ftell(f) / BUFFER_SIZE;
	if(ftell(f) % BUFFER_SIZE)
		data_packets++;
	
	rewind(f);


	hostname = gethostbyname(argv[1]);
	if(hostname == (struct hostent*)0) {
		fprintf(stderr, "gethostbyname failed\n");
        exit(-1);
	}


	if(sockid == -1) {
		fprintf(stderr, "Error while establishing an IPv4 socket\n");
		exit(-1);
	}
	
	server.sin_family = PF_INET;
	server.sin_port = (uint16_t) atoi(argv[2]);
	server.sin_addr.s_addr = *((in_addr_t*)hostname->h_addr);

	if(connect(sockid, (struct sockaddr*) &server, sizeof(server)) == -1) {
		fprintf(stderr, "Failed to connect server\n");
		exit(-1);
	} else {
		printf("Connecting to server %s\n", hostname->h_addr);
	}

	header.buffer_size = BUFFER_SIZE;
	header.data_packets = data_packets;
	strcpy(header.filename, argv[3]);
	header.type = TEXT_DOC;

	// send header first and wait for "READY"
	printf("Sending header...\n");
	if (send(sockid, &header, sizeof(transaction_header), 0) < 0) {
		fprintf(stderr, "Failed to send the transaction header to server\n");
		exit(-1);
	} else {
		if(recv(sockid, &response, sizeof(response), 0) == -1) {
			fprintf(stderr, "Failed to receive a response from server\n");
			exit(-1);
		}
		if(response != RES_READY) {
			fprintf(stderr, "Server is busy or has sent the wrong response.\nExiting...\n");
			exit(-1);
		}

		response = RES_DEFAULT;
	}


	for(i = 0; i < data_packets; ++i) {
		if(i == data_packets - 1)
			memset(buffer, 0, BUFFER_SIZE);
		
		fread(buffer, BUFFER_SIZE, 1, f);

		printf("Sending data to server...[%lu/%lu]\n", i+1, data_packets);
		if (send(sockid, buffer, BUFFER_SIZE, 0) < 0) {
			fprintf(stderr, "Failed to transmit to server\n");
			exit(-1);
		} else {
			if (recv(sockid, &response, sizeof(response), 0) == -1) {
				fprintf(stderr, "Acknowledge not received from server on packet %lu\n", i+1);
				exit(-1);
			} else {
				if(response != RES_RECEIVED)
					printf("Something bad happened with the server on packet %lu\n", i+1);
			}

			response = RES_DEFAULT;
		}

	}

	fclose(f);

	if(closeSocket(sockid) != 0) {
		fprintf(stderr, "Could not terminate the socket correctly\n");
		exit(-1);
	}

	printf("Client stopped...\n");

	
	return 0;
}
