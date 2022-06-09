#ifndef _MYUTIL_H_
#define _MYUTIL_H_

#include <unistd.h>


typedef int sockid_t;
typedef int connected_socket_t;
typedef char data_byte;

// Wrapper for close(socket) for more clarification
int closeSocket(sockid_t sockid) {
	return close(sockid);
}

typedef enum __response_type {
	RES_DEFAULT = -1,
	RES_OK = 0,
	RES_RECEIVED,
	RES_READY,
	RES_FAILED
} response_type;


typedef enum __file_type {
	GENERIC = 0,
	TEXT_DOC,
	JPG_JPEG,
	PNG,
	MP3,
	WAV,
	MP4,
	MKV
} file_type;

typedef struct {
	size_t buffer_size;
	size_t data_packets;
	file_type type;
	char filename[40];
} transaction_header;


#endif //_MYUTIL_H_