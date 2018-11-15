#pragma once
#include "common.h"
#include "util.h"
#include <stdlib.h>
#include <string.h>

//The senders and receivers is global variables.
//use the memery copy method to simulate the transport of information
void send_msg_to_receivers(char *);
void send_msg_to_senders(char *);
//the common send function
void send_frame(char *, SendFrame_DstType);