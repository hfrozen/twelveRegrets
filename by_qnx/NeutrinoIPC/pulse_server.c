/*
 * pulse_server.c
 * 
 * This program, along with pulse_client.c, demonstrate pulses between a
 * server and a client using MsgDeliverEvent().
 * 
 * The server attaches a name for the client to find using name_attach().
 * Since it is using name_attach(), it will have to handle some system pulses
 * and possibly messages, as well as the ones we're interested in.
 * 
 * It will get a register message from the client.  This message will
 * contain an event to be delivered to the client.  It will also get a 
 * regular pulse, and when that happens, it will deliver the event to 
 * the client.
 *
 * When it gets a disconnect notification from the client, it needs to
 * clean up.
 * 
 *  To test it, run it as follows:
 *    pulse_server
 *
 */
 
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <stdint.h>
#include <sys/neutrino.h>
#include <sys/dispatch.h>

#include "pulse_server.h"
#include <unistd.h>

#define PROGNAME "pulse_server: "

/* server for client-server pulsing exercise */
union recv_msgs 
{
    struct reg_msg client_msg;
    struct _pulse pulse;
    uint16_t type;
} recv_buf;

// client tracking information
int save_rcvid = 0;
int save_scoid = 0;
struct sigevent save_event;
int notify_count = 0;

// this thread will notify every second any client that needs notification 
void * notify_thread( void * ignore)
{
  printf("notify thread started\n");
  while(1) {
    sleep(1);
    printf("save rcvid is %d\n", save_rcvid );
    if (save_rcvid) {
      printf(PROGNAME "deliver pulse to client %X\n", save_rcvid);

      /* 
       * Server send event to the client using saved event.
       */
      save_event.sigev_value.sival_int = ++notify_count;
      if (-1 == MsgDeliverEvent( save_rcvid, &save_event ) )
      {
        perror( "MsgDeliverEvent");
      }

    }
  }
}

int main(int argc, char *argv[]) {
  name_attach_t *att;
  int rcvid;
  struct _msg_info msg_info;

  setvbuf (stdout, NULL, _IOLBF, 0);

  att = name_attach(NULL,RECV_NAME, 0 );
  if (NULL == att ) {
    perror(PROGNAME "name_attach()");
    exit(EXIT_FAILURE);
  }

  // create the client notification thread
  pthread_create( NULL, NULL, notify_thread, NULL );

  // main receive loop
  while(1) {
    rcvid = MsgReceive( att->chid, &recv_buf, sizeof (recv_buf), &msg_info );
    if( -1 == rcvid ) {
      perror(PROGNAME "MsgReceive failed");
      continue;
    }
    if ( 0 == rcvid ) {
      /* we received a pulse, often a side-effect of using name_attach()
       * we need to deal with the system pulses appropriately 
       */
      switch( recv_buf.pulse.code ) {
        /* system disconnect pulse */
        case _PULSE_CODE_DISCONNECT: 
          /* A client has disconnected.  Verify that it is
           * our client, and if so, clean up our saved date 
           */
          if( recv_buf.pulse.scoid == save_scoid ) {
            /* our client went away */
            save_rcvid = 0;
            save_scoid = 0;
            notify_count = 0;
          }

          /* always do the ConnectDetach(), though */
          ConnectDetach( recv_buf.pulse.scoid );
          printf(PROGNAME "disconnect from a client %X\n", recv_buf.pulse.scoid);
          break;
          /* our pulse */
        default:
          printf(PROGNAME "unexpect pulse code: %d\n", recv_buf.pulse.code );
          break;
      }
      continue;
    }
    /* not an error, not a pulse, therefor a message */
    if ( recv_buf.type == _IO_CONNECT ) {
      /* _IO_CONNECT because someone did a name_open() to us, must EOK it. */
      MsgReply( rcvid, EOK, NULL, 0 );
      continue;
    }
    if ( recv_buf.type > _IO_BASE && recv_buf.type <= _IO_MAX ) {
      /* unexpected system message, may be side effect of name_attach() */
      MsgError( rcvid, ENOSYS );
      continue;
    }
    // messages
    switch( recv_buf.type ) {
      case REG_MSG:

        /* Later we will be calling MsgDeliverEvent().  We
         * will need to pass it a rcvid and an event to deliver.
         * We've just received a message from the client containing 
         * the event.  It is in recv_buf.client_msg.ev.  It is saved
         * the scoid in save_scoid, the rcvid in save_rcvid,  
         * and the event in save_event.  
         */
        if( save_rcvid ) {
          /* we already have a client, reject new client with EBUSY */
          printf("rejecting connect from client %X\n", rcvid );
          MsgError( rcvid, EBUSY );
        } else {
          save_rcvid = rcvid;
          save_scoid = msg_info.scoid;
          save_event = recv_buf.client_msg.ev;

          MsgReply(rcvid, 0, NULL, 0 );
          printf(PROGNAME "got register message from client: %X\n", rcvid );
        }
        break;
      default:
        /* some other unexpect message */
        printf(PROGNAME "unexpect message type: %d\n", recv_buf.type );
        MsgError(rcvid, ENOSYS );
        break;
    }
  }
}
