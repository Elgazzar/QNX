// Include the iostream library for standard input/output streams
#include <iostream>
// Include the time.h library for POSIX timer functions and structures
#include <time.h>
// Include the siginfo.h library for signal and event structures
#include <sys/siginfo.h>
// Include the neutrino.h library for QNX microkernel message passing and channel APIs
#include <sys/neutrino.h>
// Include the unistd.h library for standard symbolic constants and types
#include <unistd.h>
// Include the errno.h library for error number definitions
#include <errno.h>
// Include the string.h library for string manipulation functions like strerror
#include <string.h>
// Include the stdint.h library for fixed-width integer types
#include <stdint.h>

// Define a custom pulse code for our high-resolution timer expiration event
#define PULSE_CODE_HR_TIMER_EXPIRED _PULSE_CODE_MINAVAIL

// Define a helper function to calculate the time difference in nanoseconds
uint64_t diff_timespec_ns(const struct timespec* start, const struct timespec* end) {
    // Convert the start time's seconds to nanoseconds and add the nanoseconds part
    uint64_t start_ns = (uint64_t)start->tv_sec * 1000000000ULL + start->tv_nsec;
    // Convert the end time's seconds to nanoseconds and add the nanoseconds part
    uint64_t end_ns = (uint64_t)end->tv_sec * 1000000000ULL + end->tv_nsec;
    // Return the difference between the end time and the start time
    return end_ns - start_ns;
// End of the helper function
}

// Define the main function where program execution begins
int main(int argc, char *argv[]) {
    // Print a welcome message to the standard output
    std::cout << "--- QNX High Resolution Timer API Example ---" << std::endl;

    // Create a new channel for receiving pulses
    int chid = ChannelCreate(0);
    // Check if the channel creation failed
    if (chid == -1) {
        // Print an error message to the standard error stream
        std::cerr << "Failed to create channel: " << strerror(errno) << std::endl;
        // Return an error code and exit the program
        return 1;
    // Close the if block
    }
    
    // Attach a connection to the newly created channel
    int coid = ConnectAttach(0, 0, chid, _NTO_SIDE_CHANNEL, 0);
    // Check if the connection attachment failed
    if (coid == -1) {
        // Print an error message to the standard error stream
        std::cerr << "Failed to attach connection: " << strerror(errno) << std::endl;
        // Destroy the previously created channel to prevent resource leaks
        ChannelDestroy(chid);
        // Return an error code and exit the program
        return 1;
    // Close the if block
    }

    // Declare a sigevent structure to hold the timer notification configuration
    struct sigevent event;
    // Initialize the sigevent structure to deliver a pulse when the timer expires
    SIGEV_PULSE_INIT(&event, coid, 10, PULSE_CODE_HR_TIMER_EXPIRED, 0);

    // Declare a timer_t variable to hold the ID of the new timer
    timer_t timer_id;
    // Create a new monotonic timer and link it to our pulse event
    if (timer_create(CLOCK_MONOTONIC, &event, &timer_id) == -1) {
        // Print an error message to the standard error stream if timer creation fails
        std::cerr << "Failed to create timer: " << strerror(errno) << std::endl;
        // Detach the connection to clean up resources
        ConnectDetach(coid);
        // Destroy the channel to clean up resources
        ChannelDestroy(chid);
        // Return an error code and exit the program
        return 1;
    // Close the if block
    }
    // Print a success message indicating the timer was created
    std::cout << "Timer created successfully (CLOCK_MONOTONIC)." << std::endl;

    // Declare an itimerspec structure to hold the tolerance configuration
    struct itimerspec tolerance_spec;
    // Clear the memory of the tolerance_spec structure to ensure all fields are zeroed
    memset(&tolerance_spec, 0, sizeof(tolerance_spec));
    // Set the nanosecond portion of the initial value to 0 to specify zero tolerance
    tolerance_spec.it_value.tv_nsec = 0; 
    
    // Call timer_settime with the TIMER_TOLERANCE flag to configure the timer's tolerance
    if (timer_settime(timer_id, TIMER_TOLERANCE, &tolerance_spec, NULL) == -1) {
        // Print an error message if setting the tolerance fails
        std::cerr << "Failed to set timer tolerance: " << strerror(errno) << std::endl;
        // Delete the timer to clean up resources
        timer_delete(timer_id);
        // Detach the connection to clean up resources
        ConnectDetach(coid);
        // Destroy the channel to clean up resources
        ChannelDestroy(chid);
        // Return an error code and exit the program
        return 1;
    // Close the if block
    }
    // Print a success message indicating the high-resolution mode was requested
    std::cout << "Timer tolerance set to 0 ns (High-Resolution mode requested)." << std::endl;

    // Declare an itimerspec structure to hold the actual timer expiration configuration
    struct itimerspec itime;
    // Clear the memory of the itime structure to ensure all fields are zeroed
    memset(&itime, 0, sizeof(itime));
    // Set the seconds portion of the initial expiration time to 0
    itime.it_value.tv_sec = 0;
    // Set the nanoseconds portion of the initial expiration time to 500,000 (500 microseconds)
    itime.it_value.tv_nsec = 500000; 
    // Set the seconds portion of the reload interval to 0
    itime.it_interval.tv_sec = 0;
    // Set the nanoseconds portion of the reload interval to 500,000 (500 microseconds)
    itime.it_interval.tv_nsec = 500000; 

    // Print a message indicating the timer is about to be armed
    std::cout << "Arming timer to fire every 500 microseconds..." << std::endl;
    // Call timer_settime normally to arm the timer with the specified interval
    if (timer_settime(timer_id, 0, &itime, NULL) == -1) {
        // Print an error message if arming the timer fails
        std::cerr << "Failed to set timer: " << strerror(errno) << std::endl;
        // Delete the timer to clean up resources
        timer_delete(timer_id);
        // Detach the connection to clean up resources
        ConnectDetach(coid);
        // Destroy the channel to clean up resources
        ChannelDestroy(chid);
        // Return an error code and exit the program
        return 1;
    // Close the if block
    }

    // Declare and initialize a variable to count the number of received pulses
    int pulse_count = 0;
    // Declare a constant for the maximum number of pulses to receive before exiting
    const int MAX_PULSES = 10;
    // Declare timespec structures to store the previous and current timestamps
    struct timespec prev_time, curr_time;
    
    // Get the current monotonic time and store it in prev_time
    clock_gettime(CLOCK_MONOTONIC, &prev_time);
    
    // Start a loop that will run until the pulse count reaches the maximum
    while (pulse_count < MAX_PULSES) {
        // Declare a pulse structure to receive the incoming message
        struct _pulse pulse;
        
        // Wait and block until a message or pulse is received on the channel
        int rcvid = MsgReceive(chid, &pulse, sizeof(pulse), NULL);
        
        // Check if the received message is a pulse (rcvid == 0)
        if (rcvid == 0) {
            // Check if the pulse code matches our high-resolution timer expiration code
            if (pulse.code == PULSE_CODE_HR_TIMER_EXPIRED) {
                // Get the current monotonic time and store it in curr_time
                clock_gettime(CLOCK_MONOTONIC, &curr_time);
                // Calculate the difference in nanoseconds between prev_time and curr_time
                uint64_t diff = diff_timespec_ns(&prev_time, &curr_time);
                // Update prev_time to the current time for the next iteration
                prev_time = curr_time;
                
                // Increment the pulse count
                pulse_count++;
                // Print a message showing the pulse count and the time difference in microseconds
                std::cout << "HR Timer Expired! [" << pulse_count << "/" << MAX_PULSES 
                          << "] Time since last pulse: " << diff / 1000 << " us" << std::endl;
            // Close the inner if block
            }
        // Check if MsgReceive returned an error (-1)
        } else if (rcvid == -1) {
            // Print an error message if MsgReceive failed
            std::cerr << "MsgReceive failed: " << strerror(errno) << std::endl;
            // Break out of the loop since we cannot receive more pulses
            break;
        // Close the else if block
        }
    // Close the while loop
    }

    // Print a message indicating that the maximum number of pulses was reached
    std::cout << "Max pulses reached. Cleaning up." << std::endl;

    // Delete the timer to clean up resources
    timer_delete(timer_id);
    // Detach the connection to clean up resources
    ConnectDetach(coid);
    // Destroy the channel to clean up resources
    ChannelDestroy(chid);

    // Return a success code and exit the program
    return 0;
// Close the main function
}
