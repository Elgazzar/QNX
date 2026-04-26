////////////////////////////////////////////////////////////////////////////////
// server.c
//
// A QNX msg passing server.  It should receive a string from a client,
// calculate a checksum on it, and reply back the client with the checksum.
//
// The server prints out its pid and chid so that the client can be made aware
// of them.
//
// Using the comments below, put code in to complete the program.  Look up
// function arguments in the course book or the QNX documentation.
////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/neutrino.h>
#include <process.h>

#include "msg_def.h"  //layout of msg's should be defined by a struct, here's its definition

int
calculate_checksum(char *text);

int main(void)
{
	int chid;
	int pid;
	rcvid_t rcvid;
	cksum_msg_t msg;
	int status;
	int checksum;

	chid = ChannelCreate(0); //create a channel for receiving messages
	if (chid == -1)
	{ //was there an error creating the channel?
		perror("ChannelCreate()"); //look up the errno code and print
		exit(EXIT_FAILURE);
	}

	pid = getpid(); //get our own pid
	printf("Server's pid: %d, chid: %d\n", pid, chid); //print our pid/chid so
	//client can be told where to connect

	while (1)
	{
		rcvid = MsgReceive(chid, &msg, sizeof(msg), NULL); //block waiting for a client message

		if (rcvid == -1)
		{ //was there an error receiving msg?
			perror("MsgReceive"); //look up errno code and print
			exit(EXIT_FAILURE); //give up
		}

		checksum = calculate_checksum(msg.string_to_cksum); //calculate checksum of received string
		printf("Calculated checksum: %d for string: \"%s\"\n", checksum, msg.string_to_cksum);

		status = MsgReply(rcvid, EOK, &checksum, sizeof(checksum)); //reply to client with checksum

		if (status == -1)
		{
			perror("MsgReply");
		}
	}
	return 0;
}

int calculate_checksum(char *text)
{
	char *c;
	int cksum = 0;

	for (c = text; *c != '\0'; c++)
		cksum += *c;
	return cksum;
}

