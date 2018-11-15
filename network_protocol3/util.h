#pragma once
// Include the Common to give the definition of type.
#include "common.h"
#include <stdlib.h>
#include <string.h>
#include "crc.h"
#include "communicate.h"

// Linked list functions
int ll_get_length(LLnode *);
void ll_append_node(LLnode **, void *);
LLnode * ll_pop_node(LLnode **);
void ll_destroy_node(LLnode *);

// Print functions
void print_cmd(Cmd *);

// Time functions 
long timeval_usecdiff(struct timeval *start_time, struct timeval *finish_time);

// TODO:Implement these functions 
//convert the frame to char means that we convert a frame to char and send it
char * convert_frame_to_char(Frame *);
Frame * convert_char_to_frame(char*);

//number of split.
void ll_split_to_frames(char *msg,Frame **frames,int src,int dst,int frame_len);

void caculate_timeout(struct timeval * timeout);