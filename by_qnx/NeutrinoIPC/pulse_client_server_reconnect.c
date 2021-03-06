/*
 * pulse_client.c
 * 
 * This program, along with pulse_server.c, demonstrate pulses between a
 * server and a client using MsgDeliverEvent().
 * 
 * The client finds the server using name_open(), passing it the name the 
 * server has registered with name_attach().
 * 
 * The code to set up the event structure for sending to the server has 
 * been removed, you will need to fill it in.
 * 
 *  To test it, first run pulse_server and then run the client as follows:
 *    pulse_client
 */


#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/siginfo.h>
#include <sys/neutrino.h>
#include <sys/dispatch.h>

#include "pulse_server.h"
#include <sched.h>

#define PROGNAME "pulse_client: "

#define MY_PULSE_CODE (_PULSE_CODE_MINAVAIL + 3)

union recv_msg {
	struct _pulse pulse;
	short type;
} recv_buf;

int find_server() {
	int coid;

	/* look for server */
	coid = name_open(RECV_NAME, 0);
	while (coid == -1) {  
		sleep(1);
		coid = name_open(RECV_NAME, 0);
	}
	return coid;
}

void server_register(int server_coid, int self_coid) {
	struct reg_msg msg;
	msg.type = REG_MSG;

	/* class: Initialize the sigevent structure (msg.ev) in the message 
	 * to be sent to the server.
	 */
	SIGEV_PULSE_INIT(&msg.ev, self_coid, 500, MY_PULSE_CODE, 0);

	if (MsgSend(server_coid, &msg, sizeof(msg), NULL, 0)) { 
		perror(PROGNAME "MsgSend");
		exit(EXIT_FAILURE);
	}
}

int main(int argc, char *argv[]) {
	int server_coid, self_coid, chid, rcvid;


	setvbuf (stdout, NULL, _IOLBF, 0);

	/* look for server */
	server_coid = find_server();

	chid = ChannelCreate(_NTO_CHF_COID_DISCONNECT);
	if (-1 == chid) {
		perror(PROGNAME "ChannelCreate");
		exit(EXIT_FAILURE);
	}
	self_coid = ConnectAttach(0, 0, chid, _NTO_SIDE_CHANNEL, 0);
	if (-1 == self_coid) {
		perror(PROGNAME "ConnectAttach");
		exit(EXIT_FAILURE);
	}

	server_register(server_coid, self_coid);

	while (1) {
		rcvid = MsgReceive(chid, &recv_buf, sizeof(recv_buf), NULL);
		if (-1 == rcvid) {
			perror(PROGNAME "MsgReceive");
			continue;
		}
		if (0 == rcvid) {
			if (MY_PULSE_CODE == recv_buf.pulse.code) {
				printf(PROGNAME "got my pulse, value is %d\n", recv_buf.pulse.value.sival_int);
			} else if (_PULSE_CODE_COIDDEATH  == recv_buf.pulse.code) {
				printf(PROGNAME "my server went away, try to find a new one. \n");
				/* look for server */
				server_coid = find_server();
				// register with new server
				server_register(server_coid, self_coid);
			} else {
				printf(PROGNAME "got unexpected pulse with code %d, value %d\n", 
						recv_buf.pulse.code, recv_buf.pulse.value.sival_int);
			}
		} else {
			printf(PROGNAME "got unexpected message, type: %d\n", recv_buf.type);
			MsgError(rcvid, ENOSYS);
		}
	}
}
