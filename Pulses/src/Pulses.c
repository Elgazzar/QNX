#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/neutrino.h>
#include <unistd.h>

// It is best practice to start custom pulse codes at _PULSE_CODE_MINAVAIL
// so they do not conflict with OS-reserved pulse codes.
#define MY_PULSE_CODE _PULSE_CODE_MINAVAIL

// Global channel ID
int server_chid;

// =========================================================
// Server Thread (Receives Pulses)
// =========================================================
void *server_thread(void *arg) {
  int rcvid;
  struct _pulse pulse; // QNX standard structure for pulses

  printf("[Server] Starting up...\n");

  // 1. Create a channel for receiving messages/pulses
  server_chid = ChannelCreate(0);
  if (server_chid == -1) {
    perror("ChannelCreate");
    return NULL;
  }

  printf("[Server] Channel created. Listening for pulses...\n");

  // 2. Loop to process pulses
  while (1) {
    // MsgReceive blocks until a message or pulse arrives
    rcvid = MsgReceive(server_chid, &pulse, sizeof(pulse), NULL);

    if (rcvid == -1) {
      perror("MsgReceive");
      break;
    }

    // rcvid == 0 mathematically indicates a PULSE!
    if (rcvid == 0) {
      printf("[Server] Received a PULSE! Code: %d, Value: %d\n", pulse.code,
             pulse.value.sival_int);

      // Check if it's our custom pulse code
      if (pulse.code == MY_PULSE_CODE) {
        if (pulse.value.sival_int == 99) {
          printf("[Server] Received shutdown pulse. Exiting loop.\n");
          break;
        }
      } else if (pulse.code == _PULSE_CODE_DISCONNECT) {
        // OS sends this pulse automatically when a client calls ConnectDetach
        printf("[Server] Received disconnect pulse. Client disconnected.\n");
      }
    } else {
      // A rcvid > 0 means a normal message (MsgSend), which we aren't handling
      // in this example
      printf("[Server] Received unexpected regular message.\n");
      MsgError(rcvid, ENOSYS);
    }
  }

  ChannelDestroy(server_chid);
  printf("[Server] Exiting cleanly.\n");
  return NULL;
}

// =========================================================
// Client Thread (Sends Pulses)
// =========================================================
void *client_thread(void *arg) {
  int coid;

  // Give the server a moment to start and create the channel
  sleep(1);

  printf("\n[Client] Starting up...\n");

  // 1. Connect to the server's channel
  coid = ConnectAttach(0, 0, server_chid, _NTO_SIDE_CHANNEL, 0);
  if (coid == -1) {
    perror("ConnectAttach");
    return NULL;
  }

  // 2. Send some pulses
  // MsgSendPulse( Connection_ID, Priority, Code, Value )
  printf("[Client] Sending pulse with value 10...\n");
  MsgSendPulse(coid, 10, MY_PULSE_CODE, 10);

  // Notice that unlike MsgSend, MsgSendPulse returns immediately. The thread
  // does not block!
  printf("[Client] Notice how the client DOES NOT block! Sending another "
         "pulse...\n");
  MsgSendPulse(coid, 10, MY_PULSE_CODE, 20);

  sleep(1); // Pause for readability

  printf("\n[Client] Sending shutdown pulse...\n");
  MsgSendPulse(coid, 10, MY_PULSE_CODE, 99);

  // Clean up connection (this automatically sends a _PULSE_CODE_DISCONNECT to
  // the server)
  ConnectDetach(coid);
  printf("[Client] Exiting cleanly.\n");
  return NULL;
}

// =========================================================
// MAIN
// =========================================================
int main(void) {
  pthread_t server_tid, client_tid;

  printf("--- QNX MsgSendPulse Example ---\n");

  // Start server first
  pthread_create(&server_tid, NULL, server_thread, NULL);

  // Start client
  pthread_create(&client_tid, NULL, client_thread, NULL);

  // Wait for both to finish
  pthread_join(client_tid, NULL);
  pthread_join(server_tid, NULL);

  printf("\n--- Example Complete ---\n");
  return EXIT_SUCCESS;
}
