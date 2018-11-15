#pragma once
#include "common.h"
#include "util.h"

void handle_recv_framelist(Receiver *recv);
void handle_recv_RecvQ(Receiver * recv);
void init_receiver(Receiver *, int);
void receiver(void *);