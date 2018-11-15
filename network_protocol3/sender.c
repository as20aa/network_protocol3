#include "sender.h"
#include "common.h"
#include <sys/time.h>

//check the cmdlist and send the frame to all receviers
//actually you just need copying the frame of sender to all the receivers' framelist.
//and receivers will check whether the frame is they want.

void init_sender(Sender * sender, int i)
{
	sender->send_id = i;
	sender->input_cmdlist_head = NULL;
	sender->input_framelist_head = NULL;
	sender->LAR = -1;
	sender->LFS = -1;
	for (int i = 0; i < MAX_BUFFER_SIZE; i++)
	{
		sender->sendQ[i].finished = 1;
	}
}

//handle the cmdlist
void handle_sender_cmdlist(Sender * sender)
{
	if (sender->input_cmdlist_head == NULL)return;
	if (ll_get_length(sender->input_cmdlist_head) > 0)
	{
		//copy the message from cmdlist to sendQ
		if ((sender->LFS - sender->LAR) < MAX_SWP_SIZE || (sender->LFS + MAX_BUFFER_SIZE - sender->LAR) < MAX_SWP_SIZE)
		{
			//printf("sender had received command\n");
			//before poping a node, you have to  lock the source
			pthread_mutex_lock(&sender->buffer_mutex);
			LLnode * node = ll_pop_node(&sender->input_cmdlist_head);
			//pthread_cond_signal(&sender->buffer_cv);
			pthread_mutex_unlock(&sender->buffer_mutex);
			//printf("sender poped a node\n");
			char * frame_char = node->value;

			//May the return frame is just null and can't store the msg.
			Frame * frame = convert_char_to_frame(frame_char);
			//printf("frame seq is :%d\n", frame->seq);
			pthread_mutex_lock(&sender->buffer_mutex);
			sender->LFS = (sender->LFS + 1) % MAX_BUFFER_SIZE;
			sender->sendQ[sender->LFS].frame = frame;
			caculate_timeout(&sender->sendQ[sender->LFS].timeout);
			sender->sendQ[sender->LFS].finished = 0;
			pthread_mutex_unlock(&sender->buffer_mutex);
		}
	}
}

//handle the finished frame.
void handle_sender_framelist(Sender *sender)
{
	if (ll_get_length(sender->input_framelist_head) > 0)
	{
		//If got the ack
		//printf("got a ack from receiver\n");
		pthread_mutex_lock(&sender->buffer_mutex);
		LLnode * node = ll_pop_node(&sender->input_framelist_head);
		pthread_mutex_unlock(&sender->buffer_mutex);
		char * msg = node->value;
		Frame * frame = convert_char_to_frame(msg);
		//printf("received ack seq:%d\n", frame->seq);
		//check thee dst == send_id
		//printf("check dst\n");
		//printf("dst :%d sender_id :%d\n", frame->dst,sender->send_id);
		if (frame->src != sender->send_id) 
		{
			//printf("dst error!\n");
			return;
		}
		//check the crc
		//printf("check crc\n");
		if (is_corrupted(msg, MAX_FRAME_SIZE))
		{
			//printf("crc erros!\n");
			return;
		}
		//printf("begin to process the sendQ\n");
		//printf("LAR:%d LFS:%d\n", sender->LAR, sender->LFS);
		int seq = frame->seq;
		int end;
		if (sender->LFS < sender->LAR)end = sender->LFS + MAX_BUFFER_SIZE;
		else end = sender->LFS;
		for (int i = (sender->LAR + 1)%MAX_BUFFER_SIZE; i <= end; i++)
		{
			int pos = i % MAX_BUFFER_SIZE;
			if (sender->sendQ[pos].frame->seq == seq)
			{
				pthread_mutex_lock(&sender->buffer_mutex);
				sender->sendQ[pos].finished = 1;
				pthread_mutex_unlock(&sender->buffer_mutex);
			}
		}

		//update the LAR
		//printf("update LAR\n");
		for (int i = (sender->LAR+1)%MAX_BUFFER_SIZE; i <= end; i++)
		{
			int pos = i % MAX_BUFFER_SIZE;

			if (sender->sendQ[pos].finished == 1)sender->LAR++;
			else break;
		}
		//printf("Finished\n");
	}
}

//handle the unfinished frame.
void handle_sender_sendQ(Sender * sender)
{
	for (int i = (sender->LAR + 1) % MAX_BUFFER_SIZE; i <= sender->LFS; i++)
	{
		int pos = i % MAX_BUFFER_SIZE;
		if (sender->sendQ[pos].finished != 1)
		{
			//failed to flash
			//printf("There is a unfinished frame in sendQ\n");
			struct timeval time;
			gettimeofday(&time, NULL);
			long time_diff = timeval_usecdiff(&time, &sender->sendQ[pos].timeout);
			//printf("time_diff:%ld\n", time_diff);
			//check whether is after 1s
			if (time_diff < 0)
			{
				//update the sendQ.timeout
				pthread_mutex_lock(&sender->buffer_mutex);
				gettimeofday(&sender->sendQ[pos].timeout, NULL);
				caculate_timeout(&sender->sendQ[pos].timeout);
				pthread_mutex_unlock(&sender->buffer_mutex);

				//printf("Send message to receivers\n");
				char * temp = convert_frame_to_char(sender->sendQ[pos].frame);			
				send_msg_to_receivers(temp);
			}
		}
	}
}

void sender(void * input_sender)
{
	struct timespec   time_spec;
	struct timeval    curr_timeval;
	const int WAIT_SEC_TIME = 0;
	const long WAIT_USEC_TIME = 100000;
	LLnode * outgoing_frames_head;
	struct timeval * expiring_timeval;
	long sleep_usec_time, sleep_sec_time;

	Sender *sender = (Sender *)input_sender;
	//sender->LAR = (sender->LAR + MAX_BUFFER_SIZE) % MAX_BUFFER_SIZE;
	pthread_cond_init(&sender->buffer_cv, NULL);
	pthread_mutex_init(&sender->buffer_mutex, NULL);

	//printf("sender %d is here\n", sender->send_id);
	while (1)
	{
		outgoing_frames_head = NULL;

		gettimeofday(&curr_timeval,NULL);

		//Check whether anything has arrived
		//Nothing (cmd nor incoming frame) has arrived, so do a timed wait on the sender's condition variable (releases lock)
		//A signal on the condition variable will wakeup the thread and reaquire the lock

		handle_sender_cmdlist(sender);
		handle_sender_sendQ(sender);
		handle_sender_framelist(sender);
	}
	pthread_exit(NULL);
}