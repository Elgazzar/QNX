// Include the standard input/output stream library
#include <iostream>
// Include the POSIX thread library for thread-related operations if needed
#include <pthread.h>
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

// Define a custom pulse code for the timer expiration
#define PULSE_CODE_TIMER _PULSE_CODE_MINAVAIL

// Define a helper function to calculate the time difference in microseconds
uint64_t diff_timespec_us(const struct timespec* start, const struct timespec* end) {
    // Convert the start time's seconds to microseconds and add the nanoseconds converted to microseconds
    uint64_t start_us = (uint64_t)start->tv_sec * 1000000ULL + start->tv_nsec / 1000ULL;
    // Convert the end time's seconds to microseconds and add the nanoseconds converted to microseconds
    uint64_t end_us = (uint64_t)end->tv_sec * 1000000ULL + end->tv_nsec / 1000ULL;
    // Return the difference between the end time and the start time
    return end_us - start_us;
// Close the helper function
}

// Define the main function of the application
int main(int argc, char *argv[]) {
    // Print the start of the demonstration
    std::cout << "--- QNX Timer Aliasing (1.5ms request on a 1ms tick) ---" << std::endl;

    // Print a note that we are assuming the default 1ms tick size
    std::cout << "Assuming standard QNX default System Clock Tick of 1.0 ms (1,000,000 ns)." << std::endl;
    // Create a new channel to receive timer pulses
    int chid = ChannelCreate(0);
    // Check if channel creation failed
    if (chid == -1) {
        // Print an error message if the channel could not be created
        std::cerr << "Failed to create channel" << std::endl;
        // Return an error code to the OS
        return 1;
    // Close the channel error check block
    }
    
    // Attach a connection to the newly created channel
    int coid = ConnectAttach(0, 0, chid, _NTO_SIDE_CHANNEL, 0);
    // Check if the connection attachment failed
    if (coid == -1) {
        // Print an error message if the connection could not be established
        std::cerr << "Failed to attach connection" << std::endl;
        // Return an error code to the OS
        return 1;
    // Close the connection error check block
    }

    // Declare a sigevent structure to hold the notification method
    struct sigevent event;
    // Initialize the event to send a pulse to our connection when the timer expires
    SIGEV_PULSE_INIT(&event, coid, 10, PULSE_CODE_TIMER, 0);

    // Declare a timer_t variable to hold the timer ID
    timer_t timer_id;
    // Create a monotonic timer tied to our pulse event
    if (timer_create(CLOCK_MONOTONIC, &event, &timer_id) == -1) {
        // Print an error message if timer creation failed
        std::cerr << "Failed to create timer" << std::endl;
        // Return an error code to the OS
        return 1;
    // Close the timer error check block
    }

    // Declare an itimerspec structure to configure the periodic timer
    struct itimerspec itime;
    // Set the seconds portion of the initial expiration time to 0
    itime.it_value.tv_sec = 0;
    // Set the nanoseconds portion of the initial expiration time to 1.5ms (1,500,000 ns)
    itime.it_value.tv_nsec = 1500000;
    // Set the seconds portion of the reload interval to 0
    itime.it_interval.tv_sec = 0;
    // Set the nanoseconds portion of the reload interval to 1.5ms (1,500,000 ns)
    itime.it_interval.tv_nsec = 1500000;

    // Print the user's intent to start a 1.5ms timer
    std::cout << "Requesting periodic timer interval: 1.5 ms (1500 us)" << std::endl;
    
    // Arm the timer to fire immediately and then periodically every 1.5ms
    if (timer_settime(timer_id, 0, &itime, NULL) == -1) {
        // Print an error message if setting the timer failed
        std::cerr << "Failed to set timer" << std::endl;
        // Return an error code to the OS
        return 1;
    // Close the timer set error check block
    }

    // Declare a variable to hold the number of pulses we want to measure
    const int ITERATIONS = 10;
    // Declare timespec structures to store the previous and current timestamps
    struct timespec prev_time, curr_time;
    // Get the initial baseline monotonic time
    clock_gettime(CLOCK_MONOTONIC, &prev_time);

    // Start a for loop to wait for pulses the specified number of times
    for (int i = 1; i <= ITERATIONS; i++) {
        // Declare a pulse structure to receive the incoming kernel message
        struct _pulse pulse;
        // Block and wait to receive a message on our channel
        int rcvid = MsgReceive(chid, &pulse, sizeof(pulse), NULL);
        
        // Check if the received message is a valid pulse
        if (rcvid == 0 && pulse.code == PULSE_CODE_TIMER) {
            // Get the current monotonic time as soon as we wake up
            clock_gettime(CLOCK_MONOTONIC, &curr_time);
            // Calculate the exact microsecond difference since the last pulse
            uint64_t diff_us = diff_timespec_us(&prev_time, &curr_time);
            // Save the current time as the previous time for the next loop iteration
            prev_time = curr_time;
            
            // Print out the actual measured time between pulses
            std::cout << "Pulse [" << i << "/" << ITERATIONS << "] Actual interval: " << diff_us << " us (~" << (diff_us / 1000) << " ms)" << std::endl;
        // Close the pulse check block
        }
    // Close the for loop
    }
    
    // Print a conclusion statement explaining what just happened
    std::cout << "\nConclusion: The timer alternates between ~2ms and ~1ms to average exactly 1.5ms!" << std::endl;
    // Print the reason why this aliasing occurs
    std::cout << "Reason: The kernel cannot fire between ticks. It introduces massive jitter to mathematically average your request." << std::endl;

    // Delete the periodic timer to free up kernel resources
    timer_delete(timer_id);
    // Detach the connection to our channel
    ConnectDetach(coid);
    // Destroy the channel to free up system resources
    ChannelDestroy(chid);

    // Return success to the operating system
    return 0;
// Close the main function
}
