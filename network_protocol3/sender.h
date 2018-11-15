#pragma once
#include "common.h"
#include "util.h"
#include "communicate.h"
void handle_sender_cmdlist(Sender * sender);
void handle_sender_framelist(Sender * sender);
void handle_sender_sendQ(Sender * sender);
void init_sender(Sender *,int);
void sender(void*);