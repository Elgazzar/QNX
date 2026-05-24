#include <iostream>
#include <pthread.h>
#include <sys/neutrino.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

using namespace std;

// Global channel ID
int server_chid;

// Custom Header Structure
typedef struct {
    uint16_t msg_type;
    uint32_t payload_size;
} custom_header_t;

// =========================================================
// Server Thread (Receiver)
// =========================================================
void* server_thread(void* arg) {
    int rcvid;
    custom_header_t header; // NOTICE: We ONLY allocate enough memory for the header!

    cout << "[Server] Starting up..." << endl;

    server_chid = ChannelCreate(0);
    if (server_chid == -1) {
        cerr << "[Server] Failed to create channel!" << endl;
        return NULL;
    }

    while (true) {
        // 1. Receive ONLY the header!
        // We pass sizeof(header), so the OS kernel ONLY copies the first few bytes 
        // from the client's massive message into our tiny memory space.
        rcvid = MsgReceive(server_chid, &header, sizeof(header), NULL);
        if (rcvid == -1) break;

        if (rcvid > 0) {
            cout << "\n[Server] Received a Header!" << endl;
            cout << "  -> Message Type: " << header.msg_type << endl;
            cout << "  -> Payload Size: " << header.payload_size << " bytes" << endl;

            if (header.msg_type == 99) {
                cout << "[Server] Shutdown command received. Exiting." << endl;
                break;
            }

            // 2. Now we know how big the payload is! Allocate exact memory needed dynamically.
            char* big_payload = (char*)malloc(header.payload_size);

            // 3. Go back and read the rest of the message from the client's memory!
            // MsgRead( rcvid, buffer, size, offset_in_client_message )
            // We set offset = sizeof(header) because we already read the header bytes!
            MsgRead(rcvid, big_payload, header.payload_size, sizeof(header));

            cout << "[Server] Successfully used MsgRead to pull the payload: \"" << big_payload << "\"" << endl;

            // 4. Reply to the client and clean up our dynamic memory
            const char* reply = "Got your header, and successfully read your payload!";
            MsgReply(rcvid, 0, reply, strlen(reply) + 1);

            free(big_payload);
        }
    }
    
    ChannelDestroy(server_chid);
    cout << "[Server] Exiting cleanly." << endl;
    return NULL;
}

// =========================================================
// Client Thread (Sender)
// =========================================================
void* client_thread(void* arg) {
    int coid;
    char reply_buffer[256];

    sleep(1); // Wait for server to set up

    coid = ConnectAttach(0, 0, server_chid, _NTO_SIDE_CHANNEL, 0);
    if (coid == -1) return NULL;

    // --- SETUP MESSAGE ---
    // The client builds a single continuous buffer (or could use IOVs)
    // For simplicity, we just use an inline struct that has a header and payload.
    struct {
        custom_header_t header;
        char payload[128];
    } full_msg;

    full_msg.header.msg_type = 1;
    strcpy(full_msg.payload, "This is a massive payload that the server didn't have memory for initially!");
    full_msg.header.payload_size = strlen(full_msg.payload) + 1;

    cout << "\n[Client] Sending full message (Header + Payload)..." << endl;
    
    // The client sends the ENTIRE message at once. 
    // It blocks completely unaware that the server is reading it in pieces!
    MsgSend(coid, &full_msg, sizeof(full_msg), reply_buffer, sizeof(reply_buffer));
    
    cout << "[Client] Server Replied: \"" << reply_buffer << "\"" << endl;

    sleep(1);

    // Send shutdown message
    full_msg.header.msg_type = 99;
    MsgSend(coid, &full_msg, sizeof(full_msg.header), reply_buffer, sizeof(reply_buffer));

    ConnectDetach(coid);
    cout << "[Client] Exiting cleanly." << endl;
    return NULL;
}

// =========================================================
// MAIN
// =========================================================
int main() {
    pthread_t server_tid, client_tid;
    
    cout << "--- QNX MsgRead Two-Step Receive Example ---" << endl;

    pthread_create(&server_tid, NULL, server_thread, NULL);
    pthread_create(&client_tid, NULL, client_thread, NULL);

    pthread_join(client_tid, NULL);
    pthread_join(server_tid, NULL);

    cout << "\n--- Example Complete ---" << endl;
    return 0;
}
