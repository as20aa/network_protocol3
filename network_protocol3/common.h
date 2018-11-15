#pragma once
#include "pthread.h"
#include <sys/time.h>
#define MAX_COMMAND_LENGTH 16				//the length of the head of the command
#define AUTOMATED_FILENAME 512				//the length of the message you can input
#define MAX_SWP_SIZE 4
#define MAX_BUFFER_SIZE 8
#define MAX_INPUT_SIZE 1024
typedef unsigned char uchar_t;


//System configuration information
struct SysConfig_t
{
	float drop_prob;	//drop probability
	float corrupt_prob;	//corrupt probabiliby
	unsigned char automated;	//
	char automated_file[AUTOMATED_FILENAME];
};
typedef struct SysConfig_t SysConfig;


typedef unsigned int uint16_t;

//Command line input information
struct Cmd_t
{
	uint16_t src_id;	//the source thread id
	uint16_t dst_id;	//the destination thread id
	char * message;		//message should send
};
typedef struct Cmd_t Cmd;

//Linked list information
enum LLtype
{
	llt_string,
	llt_frame,
	llt_integer,
	llt_head
}LLtype;

struct LLnode_t
{
	struct LLnode_t * prev;		//the previous node
	struct LLnode_t * next;		//the next node
	enum LLtype type;			//the type of this node
	void * value;				//the value of the node
};
typedef struct LLnode_t LLnode;


typedef enum SendFrame_DstType
{
	ReceiveDst,
	SenderDst
}SendFrame_DstType;

typedef struct Sender_t Sender;
typedef struct Receiver_t Receiver;

#define MAX_FRAME_SIZE 64
//TODO:You should change this!
//Remember,your frame can be AT MOST 64 bytes!
#define FRAME_PAYLOAD_SIZE 59
struct Frame_t
{
	unsigned int src;
	unsigned int dst;
	unsigned int tail;			//the ack can be used to tag the head and tail
	unsigned int seq;			//seq cann't lager than 255, the number of the frame
	char data[FRAME_PAYLOAD_SIZE];
};
typedef struct Frame_t Frame;

//Receiver and sender data structures
struct recvQ_slot_t {
	unsigned int received;
	Frame * frame;
};
typedef struct recvQ_slot_t recvQ_slot;

struct Receiver_t
{
	//DO NOT CHANGE;
	// buffer_mutex
	// buffer_cv
	// input_framelist_head
	// recv_id
	pthread_mutex_t		buffer_mutex;				//the thread lock
	pthread_cond_t		buffer_cv;					//the variable accept the signal
	LLnode *			input_framelist_head;		//the buffer which will accept the message sended from sender
	int					LAR;
	int					LFS;
	int					recv_id;					//the receiver thread id.
	recvQ_slot			recvQ[MAX_BUFFER_SIZE];		//The receiver buffer
};

struct sendQ_slot_t {
	int finished;
	struct timeval timeout;  //the time expected to get the ack
	Frame * frame;
};
typedef struct sendQ_slot_t sendQ_slot;

struct Sender_t
{
	//DO NOT CHANGE
	// buffer_mutex
	// buffer_cv
	// input_cmdlist_head
	// input_framelist_head
	// send_id
	pthread_mutex_t buffer_mutex;		//the thread lock
	pthread_cond_t buffer_cv;			//the variable accept the signal
	LLnode * input_cmdlist_head;		//the head pointer of input command list, accept the input from stdin_thread.
	LLnode * input_framelist_head;		//the head pointer of input frame list, accept the input from the receiver thread.
	int					LAR;
	int					LFS;
	int	send_id;						//the sender's id

	//the sender buffer
	sendQ_slot sendQ[MAX_BUFFER_SIZE];		//The sender buffer
};


struct timeval_t
{
	time_t tv_sec;
	suseconds_t tv_usec;
};
//typedef struct timeval_t timeval;
typedef struct timeval_t timeval;


//Delcare global variables here
//DO NOT CHANGE
// glb_senders_array
// glb_receivers_array
// glb_senders_array_length
// glb_receivers_array_length
// glb_sysconfig
// CORRUPTION_BITS
Sender * glb_senders_array;
Receiver * glb_receivers_array;
int glb_senders_array_length;
int glb_receivers_array_length;
SysConfig glb_sysconfig;
int CORRUPTION_BITS;
unsigned int send_seq;
unsigned int recv_seq;
