#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>


#include "helpers.h"


int main(int argc, char* argv[]) {
	struct sockaddr_in server;
	struct sockaddr_in client;
	connected_socket_t new_socket;
	socklen_t client_length;
	data_byte* buffer = NULL;
	FILE* f;
	response_type response = RES_DEFAULT;
	size_t i;

	transaction_header header;

	sockid_t sockid = socket(PF_INET, SOCK_STREAM, 0);
	if(sockid == -1) {
		fprintf(stderr, "Error while establishing an IPv4 socket\n");
		exit(-1);
	}

	if (argc != 2)
    {
        fprintf(stderr, "Usage: %s [port]\n", argv[0]);
        exit(-1);
    }


	server.sin_family = AF_INET;
	server.sin_port = (uint16_t) atoi(argv[1]);
	server.sin_addr.s_addr = INADDR_ANY;

	if(bind(sockid, (struct sockaddr*) &server, sizeof(server)) == -1) {
		fprintf(stderr, "Failure in binding socket %d with port %u\n", sockid, server.sin_port);
		exit(-1);
	}

	if(listen(sockid, 1) == -1) {
		fprintf(stderr, "Failed to listen on port %u\n", server.sin_port);
		exit(-1);
	} else
		printf("Listening on port %u\n", server.sin_port);

	client_length = sizeof(client);

#define ALWAYS_RUN
#ifdef ALWAYS_RUN	
	while(1) {
#endif
		
		if((new_socket = accept(sockid, (struct sockaddr*) &client, &client_length)) == -1) {
			fprintf(stderr, "Failed to accept connection to port %u\n", server.sin_port);
			exit(-1);
		} else {
			printf("Received connection from client %u\n", client.sin_addr.s_addr);
		}

		if(recv(new_socket, &header, sizeof(transaction_header), 0) == -1) {
			fprintf(stderr, "Error during header reception on port %u\n", server.sin_port);
			response = RES_FAILED;
		} else {
			buffer = (data_byte*) malloc(header.buffer_size * sizeof(data_byte));
			response = RES_READY;
		}
		send(new_socket, &response, sizeof(response), 0);
		response = RES_DEFAULT;


		f = fopen(header.filename, "w");
		for(i = 0; i < header.data_packets; ++i) {	
			printf("Receiving data packet [%lu/%lu]...\n", i+1, header.data_packets);
			if(recv(new_socket, buffer, header.buffer_size, 0) == -1) {
				fprintf(stderr, "Error during data reception on port %u\n", server.sin_port);
				response = RES_FAILED;
			} else {
				fwrite(buffer, header.buffer_size, 1, f);
				response = RES_RECEIVED;

			}
			if(send(new_socket, &response, sizeof(response), 0) < 0) {
				fprintf(stderr, "Could not send response on port %u\n", server.sin_port);
			}
			response = RES_DEFAULT;
		}

		fclose(f);
		if(buffer != NULL) {
			free(buffer);
			buffer = NULL;
		}

#ifdef ALWAYS_RUN
	}
#endif

	if(closeSocket(new_socket) != 0) {
		fprintf(stderr, "Could not terminate the connection new_socket correctly\n");
		exit(-1);
	}

	if(closeSocket(sockid) != 0) {
		fprintf(stderr, "Could not terminate the socket correctly\n");
		exit(-1);
	}

	printf("Server closing...\n");

	
	return 0;
}
