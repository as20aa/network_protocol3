#include "communicate.h"

//send a frame the receivers
void send_msg_to_receivers(char * buffer)
{
	send_frame(buffer, ReceiveDst);
}

//send a frame to senders
void send_msg_to_senders(char * buffer)
{
	send_frame(buffer, SenderDst);
}

//send the frame and implement the corrupt and drop function
void send_frame(char * frame, SendFrame_DstType dst)
{
	//TODO:Implement the corrupt and drop probility
	//send the frame to receiver
	//just copy the frame to all the glb_receiver_array.
	int drop_prob, corrupt_prob;
	drop_prob = (int)(glb_sysconfig.drop_prob * 1000);
	corrupt_prob = (int)(glb_sysconfig.corrupt_prob * 1000);
	int corrupt_bits[CORRUPTION_BITS];

	srand(time(0));

	int prob;

	//get the dst array length
	int array_len;
	if (dst == SenderDst)
	{
		array_len = glb_senders_array_length;
	}
	else
	{
		array_len = glb_receivers_array_length;
	}

	//copy the frame to the dst array
	for (int i = 0; i < array_len; i++)
	{
		//whether to drop the frame
		prob = rand() % 1000;
		if (prob < drop_prob)
		{
			break;
		}

		//copy the frame to the memory
		char * temp = (char*)malloc(MAX_FRAME_SIZE*sizeof(char));
		memcpy(temp, frame, MAX_FRAME_SIZE);

		//wheter to corrupt
		prob = rand() % 1000;
		if (prob < corrupt_prob)
		{
			for (int j = 0; j < CORRUPTION_BITS; j++)
			{
				corrupt_bits[j] = rand() % MAX_FRAME_SIZE;
				temp[corrupt_bits[j]] = ~temp[corrupt_bits[j]];
			}
		}

		if (dst == ReceiveDst)
		{
			Receiver * receiver = &glb_receivers_array[i];
			pthread_mutex_lock(&receiver->buffer_mutex);
			ll_append_node(&receiver->input_framelist_head, (void*)temp);
			pthread_cond_signal(&receiver->buffer_cv);
			pthread_mutex_unlock(&receiver->buffer_mutex);
		}
		if(dst == SenderDst)
		{
			Sender * sender = &glb_senders_array[i];
			pthread_mutex_lock(&sender->buffer_mutex);
			ll_append_node(&sender->input_framelist_head, (void*)temp);
			pthread_cond_signal(&sender->buffer_cv);
			pthread_mutex_unlock(&sender->buffer_mutex);
		}
	}
	
	free(frame);
}

