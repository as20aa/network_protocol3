#include "receiver.h"

void init_receiver(Receiver * receiver, int i)
{
	receiver->recv_id = i;
	receiver->input_framelist_head = NULL;
	receiver->LAR = -1;
	receiver->LFS = -1;
}

//handle the received frame and send ack
void handle_recv_framelist(Receiver * recv)
{
	if (recv->input_framelist_head == NULL)return;
	//if received a frame, bug it will lost in the loop and check the wrong head?
	if (ll_get_length(recv->input_framelist_head) == 0)return;
	pthread_mutex_lock(&recv->buffer_mutex);
	LLnode * node = ll_pop_node(&recv->input_framelist_head);
	//pthread_cond_signal(&recv->buffer_cv);
	pthread_mutex_unlock(&recv->buffer_mutex);
	//printf("received a msg from sender\n");
	char * msg = node->value;
	Frame * frame = convert_char_to_frame(msg);
	//printf("msg :%s\n", frame->data);
	//check the frame.
	if (frame->dst != recv->recv_id)return;
	
	//printf("check the crc(receiver)\n");
	if (is_corrupted(msg,MAX_FRAME_SIZE))return;

	//printf("get the correct msg from sender\n");
	int seq = frame->seq;
	//move the frame to recvQ
	//Set the next expected seq num
	int s_seq = 0;//the expected received seq
	if (recv->LAR != recv->LFS)s_seq = recv->recvQ[recv->LAR].frame->seq + 1;

	//printf("recv->LAR :%d recv->LFS :%d\n", recv->LAR, recv->LFS);
	for (int i = 1; i < MAX_SWP_SIZE; i++)
	{
		int pos = (recv->LAR + i) % MAX_BUFFER_SIZE;

		//if the recvQ is empty and the seq is the expected.
		if (recv->recvQ[i].received == 0 && (s_seq + i - 1) % 256 == seq)
		{
			recv->recvQ[pos].frame = frame;
			recv->recvQ[pos].received = 1;
			char * ack = convert_frame_to_char(frame);
			send_msg_to_senders(ack);
			//printf("Send back the ack to sender\n");
			//update the LFS
			recv->LFS = pos;
		}
	}
}

//make sure that we received the correct frame and send ack to sender.
void handle_recv_RecvQ(Receiver * recv)
{
	if (recv->LAR == recv->LFS)return;
	int next = (recv->LAR + 1) % MAX_BUFFER_SIZE;
	if (recv->recvQ[next].received==0)return;
	int end = recv->LFS;
	if (recv->LAR > recv->LFS)end = recv->LFS + MAX_BUFFER_SIZE;
	
	int lar = next;
	//find the lar
	for (; lar <= end; lar++)
	{
		if (recv->recvQ[lar % MAX_BUFFER_SIZE].received == 0)break;
	}

	if (lar != end)lar--;
	//update the received
	for (int i = next; i <= lar; i++)
	{
		int pos = i % MAX_BUFFER_SIZE;
		recv->recvQ[pos].received = 0;
		printf("<RECV_%d>: %s\n", recv->recv_id, recv->recvQ[pos].frame->data);
	}
	recv->LAR = lar % MAX_BUFFER_SIZE;
}

void receiver(void * input_receiver)
{
	Receiver *receiver = (Receiver *)input_receiver;
	//initial the pthread lock
	pthread_cond_init(&receiver->buffer_cv, NULL);
	pthread_mutex_init(&receiver->buffer_mutex, NULL);

	//printf("receiver_id %d\n",receiver->recv_id);
	while (1)
	{
		handle_recv_framelist(receiver);
		handle_recv_RecvQ(receiver);
	}
	pthread_exit(NULL);
}