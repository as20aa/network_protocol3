#include "util.h"
#include "crc.h"

//No head and tail, traverse this linked list
int ll_get_length(LLnode * llnode)
{
	int i = 0;
	LLnode * node = llnode;
	//Return 1 when the llnode has no next node.
	//Return 0 when llnode is NULL.
	while (node != NULL)
	{
		node = node->next;
		i++;
	}
	return i;
}

void ll_append_node(LLnode ** llnode, void * value)
{
	if (llnode == NULL)return;
	Frame * frame = convert_char_to_frame(value);
	//if empty, *llnode == NULL
	//fprintf("*llnode is %d\n", *llnode);
	//fprintf("The length of llnode is %d\n", ll_get_length(*llnode));
	//malloc a new memeory to store the new node 
	LLnode *new_node = (LLnode *)malloc(sizeof(LLnode));
	new_node->next = NULL;
	new_node->prev = NULL;
	new_node->value = value;

	if (*llnode == NULL)
	{
		//assign new value to llnode
		//but will fail in the first server times.
		//add a lock to the subthread
		(*llnode) = new_node;
	}
	else
	{
		LLnode * tail = *llnode;
		while (tail->next != NULL)
		{
			tail = tail->next;
		}
		tail->next = new_node;
		new_node->prev = tail;
	}
}

//Double linked list pop node function
LLnode * ll_pop_node(LLnode ** llnode)
{
	LLnode * tail = (*llnode);
	if (ll_get_length(*llnode) == 1)
	{
		*llnode = NULL;
	}
	else
	{
		*llnode = (*llnode)->next;
		(*llnode)->prev = NULL;
	}
	return tail;
}

void ll_destroy_node(LLnode * llnode)
{
	LLnode *temp;
	while (llnode != NULL)
	{
		temp = llnode;
		llnode = llnode->next;
		free(temp);
	}
	
}

void print_cmd(Cmd * cmd)
{
	fprintf("Source id : %d\n", cmd->src_id);
	fprintf("Destination id : %d\n", cmd->dst_id);
	fprintf("Message : %s", cmd->message);
}


long timeval_usecdiff(struct timeval *start_time,
	struct timeval *finish_time)
{
	long usec;
	usec = (finish_time->tv_sec - start_time->tv_sec) * 1000000;
	usec += (finish_time->tv_usec - start_time->tv_usec);
	return usec;
}

void caculate_timeout(struct timeval * timeout)
{
	gettimeofday(timeout, NULL);
	timeout->tv_usec += 1000000;//add 0.1s
	if (timeout->tv_usec >= 1000000)
	{
		timeout->tv_usec -= 1000000;
		timeout->tv_sec += 1;
	}
}

//convert the frames to the char
//input:		the frames
//return:		the char format frame
char * convert_frame_to_char(Frame * frame)
{
	char * data = (char *)malloc(MAX_FRAME_SIZE);
	memset(data, 0, MAX_FRAME_SIZE);
	memcpy(data, frame, MAX_FRAME_SIZE);
	/*
	data[0] = (char)frame->src;
	data[1] = (char)frame->dst;
	data[2] = (char)frame->ack;
	data[3] = (char)frame->seq;
	memcpy(data+8, frame->data, FRAME_PAYLOAD_SIZE);
	fprintf("%s\n", data[4]);
	append_crc(data, MAX_FRAME_SIZE);
	*/
	return data;
}

//convert the char to the frame we have to send
//caculate the number of frame we have to send
//auto add the frame head.
//input:		the input content
//return:		the pointer tot the frames.
Frame * convert_char_to_frame(char *buffer)
{
	Frame * frame = (Frame*)malloc(MAX_FRAME_SIZE);
	memset(frame, 0, MAX_FRAME_SIZE);
	/*
	frame->src = (unsigned int)buffer[0];
	frame->dst = (unsigned int)buffer[1];
	frame->ack = (unsigned int)buffer[2];
	frame->seq = (unsigned int)buffer[3];
	memcpy(frame->data, buffer[4], FRAME_PAYLOAD_SIZE);
	*/
	memcpy(frame, buffer, MAX_FRAME_SIZE);
	return frame;
}

void ll_split_to_frames(char *buffer,Frame **frame,int src,int dst,int frame_len)
{
	(*frame) = (Frame*)malloc(sizeof(Frame)*frame_len);
	memset((*frame), 0, sizeof(Frame)*frame_len);

	for (int i = 0; i < frame_len; i++)
	{
		//TODO:fill the frame
		(*frame)[i].src = src;
		(*frame)[i].dst = dst;
		memcpy((*frame)[i].data, &buffer[i*FRAME_PAYLOAD_SIZE], FRAME_PAYLOAD_SIZE);
		(*frame)[i].seq = i;
		//fprintf("sender seq is :%d\n", i);
		(*frame)[i].tail = 0;		//the tail flag
	}
	(*frame)[frame_len - 1].tail = 1;
}

int ll_split_char(char * buffer)
{
	return (int)strlen(buffer) / FRAME_PAYLOAD_SIZE + 1;
}