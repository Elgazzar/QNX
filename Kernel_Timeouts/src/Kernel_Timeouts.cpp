// Include the standard input/output stream library
#include <iostream>
// Include the time library for clock operations and structures
#include <time.h>
// Include the neutrino library for QNX native message passing and channels
#include <sys/neutrino.h>
// Include the standard symbolic constants and types library
#include <unistd.h>
// Include the error number library to check return codes
#include <errno.h>
// Include the string library for string manipulation functions
#include <string.h>
// Include standard integer types
#include <stdint.h>

// Define the main function of the application
int main(int argc, char *argv[]) {
    // Print the start of the demonstration
    std::cout << "--- QNX Kernel Call Timeouts (TimerTimeout) ---" << std::endl;

    // Create a new channel to simulate waiting for a message
    int chid = ChannelCreate(0);
    // Check if channel creation failed
    if (chid == -1) {
        // Print an error message if the channel could not be created
        std::cerr << "Failed to create channel" << std::endl;
        // Return an error code to the OS
        return 1;
    // Close the channel error check block
    }

    // Declare a sigevent structure to dictate how the timeout wakes the thread
    struct sigevent event;
    // Initialize the event to unblock the thread when the timer expires
    SIGEV_UNBLOCK_INIT(&event);

    // Declare a 64-bit unsigned integer to hold the timeout duration in nanoseconds
    uint64_t timeout_ns;
    // Set the timeout duration to 2,000,000,000 nanoseconds (2 seconds)
    timeout_ns = 2000000000ULL;

    // Print a message indicating that we are arming the kernel timeout
    std::cout << "Arming a 2-second timeout on the next MsgReceive() call..." << std::endl;

    // Arm the timeout for the _NTO_TIMEOUT_RECEIVE blocking state
    // The timer starts immediately, and if we enter the RECEIVE state and wait longer than timeout_ns, we are unblocked.
    if (TimerTimeout(CLOCK_MONOTONIC, _NTO_TIMEOUT_RECEIVE, &event, &timeout_ns, NULL) == -1) {
        // Print an error message if the timeout could not be armed
        std::cerr << "Failed to set TimerTimeout: " << strerror(errno) << std::endl;
        // Return an error code to the OS
        return 1;
    // Close the TimerTimeout error check block
    }

    // Declare a dummy buffer to receive a message
    char msg[256];
    // Block on MsgReceive; nobody will ever send a message to this channel, so it will block
    int rcvid = MsgReceive(chid, msg, sizeof(msg), NULL);

    // Check if the receive call failed (which we expect because of the timeout)
    if (rcvid == -1) {
        // Check if the specific error is ETIMEDOUT
        if (errno == ETIMEDOUT) {
            // Print a success message confirming that the timeout correctly interrupted the block
            std::cout << "Success! MsgReceive() unblocked exactly after 2 seconds with ETIMEDOUT." << std::endl;
        // Close the ETIMEDOUT check block
        } else {
            // Print an error if MsgReceive failed for a different reason
            std::cout << "MsgReceive failed, but not due to a timeout. Errno: " << strerror(errno) << std::endl;
        // Close the else block
        }
    // Close the rcvid error check block
    } else {
        // Print an error if a message was miraculously received
        std::cout << "Unexpectedly received a message!" << std::endl;
    // Close the else block
    }

    // Destroy the channel to free up system resources
    ChannelDestroy(chid);

    // Return success to the operating system
    return 0;
// Close the main function
}
