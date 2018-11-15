#include "common.h"
#include "input.h"	
#include <string.h>
#include "util.h"

void stdin_function(void)
{
	Sender * sender;
	int src, dst;
	void * cmd=(void*)malloc(MAX_COMMAND_LENGTH);
	void * input=(void*)malloc(MAX_INPUT_SIZE);
	void * msg=(void*)malloc(MAX_INPUT_SIZE);
	while (1)
	{
		//malloc cmd every loop
		
		//don't memset the cmd,it will be null and cause error in gets()
		//memset(&cmd, 0, MAX_COMMAND_LENGTH);
		memset(cmd, 0, MAX_COMMAND_LENGTH);
		memset(input, 0, MAX_INPUT_SIZE);
		memset(msg, 0, MAX_INPUT_SIZE);
		//judge the input cmd
		gets(input);
		sscanf(input, "%s %d %d %s", cmd, &src, &dst, msg);
		//sprintf("%s %d %d %s\n", cmd, src, dst, msg);
		if (strcmp(cmd, "msg") == 0)
		{
			//fprintf("send message from sender %d to recevier %d \n", src, dst);
			if (src >= glb_senders_array_length || dst >= glb_receivers_array_length)return;

			int msg_len = strlen(msg);
			int frame_len = (int)(msg_len / FRAME_PAYLOAD_SIZE) + 1;
			//convert frames to char* and add to the cmdlist
			sender = &glb_senders_array[src];
			Frame * frames;
			ll_split_to_frames(msg, &frames, src, dst, frame_len);
			//fprintf("frame_len is %d\n", frame_len);
			for (int i = 0; i < frame_len; i++)
			{
				char * temp = convert_frame_to_char(&frames[i]);
				Frame * test_frame = convert_char_to_frame(temp);
				//add the crc to the frame
				//fprintf("tail of temp is %c\n", temp[MAX_FRAME_SIZE - 1]);
				append_crc(temp, MAX_FRAME_SIZE);
				//we convert the frame to char for transportation
				//fprintf("data:%s\n", test_frame->data);
				//you must add a lock to the subthread.
				pthread_mutex_lock(&sender->buffer_mutex);
				ll_append_node(&sender->input_cmdlist_head, (void *)temp);
				//avoid the source crush
				pthread_cond_signal(&sender->buffer_cv);
				pthread_mutex_unlock(&sender->buffer_mutex);
				//fprintf("appended\n");
			}
		}
		else
		{
			if (strcmp(cmd, "exit") == 0)
			{
				//fprintf("exit\n");
				break;
			}
		}
	}
	fprintf(stderr, "input terminated\n");
	pthread_exit(NULL);
	return;
}