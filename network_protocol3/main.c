#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include "common.h"
#include "sender.h"
#include "receiver.h"
#include "input.h"	

void init_sender_array(Sender ** sender_array, int array_len)
{
	for (int i = 0; i < array_len; i++)
	{
		(*sender_array)[i].send_id = i;
	}
}

void init_receiver_array(Receiver ** recv_array, int array_len)
{
	for (int i = 0; i < array_len; i++)
	{
		(*recv_array)[i].recv_id = i;
	}
}

// The main function is set the system configure
int main(int argc, char *argv[])
{
	pthread_t stdin_thread;
	pthread_t * sender_threads;
	pthread_t * receiver_threads;
	SysConfig config;
    fprintf(stderr, "*****The SWP program******\n");
	if (argv == 1)
	{
		fprintf(stderr, "Default parameters\n");
		fprintf(stderr, "drop_prob: 0\n");
		fprintf(stderr, "corrupt_prob: 0\n");
		fprintf(stderr, "glb_receiver_array_length: 1\n");
		fprintf(stderr, "glb_sender_array_length: 1\n");
		fprintf(stderr, "If you want to change the system configure, please start the main function with arguments\n");
	}
	
	
	//Default parameters
	glb_sysconfig.drop_prob = 0.0;
	glb_sysconfig.corrupt_prob = 0.0;
	glb_senders_array_length = 1;
	glb_receivers_array_length = 1;
	glb_sysconfig.automated = 0;
	memset(glb_sysconfig.automated_file, 0, AUTOMATED_FILENAME);
	CORRUPTION_BITS = (int)MAX_FRAME_SIZE / 2;
	send_seq = 0;
	recv_seq = 0;
	//Assign the configure parameters if get paired commands
	if (argc != 1)
	{
		for (int i = 0; i < argc; i++)
		{
			if (strcmp(argv[i], "-d")==0)
			{
				sscanf(argv[i + 1], "%f", &glb_sysconfig.drop_prob);
			}
			if (strcmp(argv[i], "-c")==0)
			{
				sscanf(argv[i + 1], "%f", &glb_sysconfig.corrupt_prob);
			}
			if(strcmp(argv[i],"-s")==0)
			{
				sscanf(argv[i + 1], "%d", &glb_senders_array_length);
			}
			if (strcmp(argv[i], "-r") == 0)
			{
				sscanf(argv[i + 1], "%d", &glb_receivers_array_length);
			}
		}
	}

	fprintf(stderr, "Message will be dropped with probability=%1.6f\n", glb_sysconfig.drop_prob);
	fprintf(stderr, "Message will be corrupted with probability=%1.6f\n", glb_sysconfig.corrupt_prob);

	//Check the parameters
	if (glb_senders_array_length <= 0 || glb_receivers_array_length <= 0 || (glb_sysconfig.drop_prob < 0 || glb_sysconfig.drop_prob>1) || (glb_sysconfig.corrupt_prob < 0 || glb_sysconfig.corrupt_prob>1))
	{
		fprintf(stderr, "The parameters you inputed is illegal\n");
		exit(1);
	}

	//use the sender_threads store every threads we create.
	sender_threads = (pthread_t *)malloc(glb_senders_array_length * sizeof(pthread_t));
	receiver_threads = (pthread_t *)malloc(glb_receivers_array_length * sizeof(pthread_t));

	//use the glb array to store the buffer we need for threads.
	glb_senders_array = (Sender *)malloc(glb_senders_array_length * sizeof(Sender));
	glb_receivers_array = (Receiver *)malloc(glb_receivers_array_length * sizeof(Receiver));

	//Create sender and receiver threads
	for (int i = 0; i < glb_senders_array_length;)
	{
		init_sender(&glb_senders_array[i],i);
		int state = (int)pthread_create(&sender_threads + i, NULL, sender, (void *)&glb_senders_array[i]);
		if (state != 0)
		{
			//printf("Create %d sender thread faile\n",i);
		}
		else
		{
			//printf("Created sender id : %d\n", i);
			i++;
		}
	}
	fprintf(stderr, "Available sender id(s) :\n");
	for (int i = 0; i < glb_senders_array_length; i++)
	{
		fprintf(stderr, "sender_id=%d\n", i);
	}


	//create the receiver thread
	for (int i = 0; i < glb_receivers_array_length;)
	{
		init_receiver(&glb_receivers_array[i], i);
		int state = (int)pthread_create(&receiver_threads + i, NULL, receiver, (void *)&glb_receivers_array[i]);
		if (state != 0)
		{
			//fprintf(stderr,"Create %d receiver thread faile\n", i);
		}
		else
		{
			//fprintf(stderr,"Created receiver id : %d\n", i);
			i++;
		}
	}

	fprintf(stderr, "Avaiable receiver id(s) :\n");
	for (int i = 0; i < glb_senders_array_length; i++)
	{
		fprintf(stderr, "receiver_id=%d\n", i);
	}

	//let the sender and receiver wait and enter the input thread.
	int state = pthread_create(&stdin_thread, NULL, stdin_function, NULL);
	if (state != 0)
	{
		fprintf(stderr,"Create stdin_thread failed\n");
		exit(1);
	}

	//wait until the stdin_thread is terminated.
	pthread_join(stdin_thread,NULL);

	//terminate all the sender and receiver thread
	for (int i = 0; i < glb_senders_array_length; i++)
	{
		pthread_cancel(sender_threads[i]);
		//pthread_join(sender_threads[i],NULL);
	}
	for (int i = 0; i < glb_receivers_array_length; i++)
	{
		pthread_cancel(receiver_threads[i]);
		//pthread_join(receiver_threads[i], NULL);
	}
	//fprintf(stderr,"terminated receiver and sender\n");
	//free(sender_threads);
	//free(receiver_threads);
	free(glb_senders_array);
	free(glb_receivers_array);

    return 0;
}