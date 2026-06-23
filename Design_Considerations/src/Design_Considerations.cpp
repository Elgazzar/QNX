// Include the standard input/output stream library
#include <iostream>
// Include the POSIX thread library for thread-related operations if needed
#include <pthread.h>
// Include the time library for clock operations and structures
#include <time.h>
// Include the neutrino library for QNX native message passing and channels
#include <sys/neutrino.h>
// Include the standard symbolic constants and types library for delay()
#include <unistd.h>
// Include the error number library to check return codes
#include <errno.h>
// Include the string library for string manipulation functions
#include <string.h>
// Include standard integer types
#include <stdint.h>

// Define a custom pulse code for the timer expiration
#define PULSE_CODE_TIMER _PULSE_CODE_MINAVAIL

// Define a helper function to calculate the time difference in milliseconds
uint64_t diff_timespec_ms(const struct timespec* start, const struct timespec* end) {
    // Convert the start time's seconds to milliseconds and add the nanoseconds converted to milliseconds
    uint64_t start_ms = (uint64_t)start->tv_sec * 1000ULL + start->tv_nsec / 1000000ULL;
    // Convert the end time's seconds to milliseconds and add the nanoseconds converted to milliseconds
    uint64_t end_ms = (uint64_t)end->tv_sec * 1000ULL + end->tv_nsec / 1000000ULL;
    // Return the difference between the end time and the start time
    return end_ms - start_ms;
// Close the helper function
}

// Define a function to simulate some workload that takes time
void simulate_workload() {
    // Sleep for 2 milliseconds to simulate CPU processing time or I/O
    delay(2);
// Close the workload function
}

// Define the main function of the application
int main(int argc, char *argv[]) {
    // Print the start of the demonstration
    std::cout << "--- QNX Design: delay() vs POSIX Timers ---" << std::endl;

    // Declare timespec structures to hold our start and end times for measurement
    struct timespec start_time, end_time;
    // Declare a variable to hold the total elapsed time in milliseconds
    uint64_t elapsed_ms;
    // Declare a constant for the number of loop iterations
    const int ITERATIONS = 10;
    // Declare a constant for our target cycle time of 5 milliseconds
    const int CYCLE_TIME_MS = 5;

    // Print a header for the bad design approach
    std::cout << "\n[BAD DESIGN] Using delay() in a loop:" << std::endl;
    // Record the start time using the monotonic clock
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    
    // Start a for loop to iterate the specified number of times
    for (int i = 0; i < ITERATIONS; i++) {
        // Call the workload function that takes ~2ms to execute
        simulate_workload();
        // Call delay to sleep for the target cycle time of 5ms
        delay(CYCLE_TIME_MS);
    // Close the for loop
    }
    
    // Record the end time using the monotonic clock
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    // Calculate the total elapsed time for the bad design
    elapsed_ms = diff_timespec_ms(&start_time, &end_time);
    // Print the expected time (iterations * 5ms = 50ms)
    std::cout << "Expected time: " << (ITERATIONS * CYCLE_TIME_MS) << " ms" << std::endl;
    // Print the actual elapsed time, demonstrating the scheduling drift (approx 70ms)
    std::cout << "Actual time  : " << elapsed_ms << " ms (Notice the drift!)" << std::endl;


    // Print a header for the good design approach
    std::cout << "\n[GOOD DESIGN] Using POSIX Timers and Pulses:" << std::endl;
    
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
    // Set the nanoseconds portion of the initial expiration time to 5 million (5ms)
    itime.it_value.tv_nsec = CYCLE_TIME_MS * 1000000;
    // Set the seconds portion of the reload interval to 0
    itime.it_interval.tv_sec = 0;
    // Set the nanoseconds portion of the reload interval to 5 million (5ms)
    itime.it_interval.tv_nsec = CYCLE_TIME_MS * 1000000;

    // Record the start time using the monotonic clock before arming the timer
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    
    // Arm the timer to fire immediately and then periodically every 5ms
    if (timer_settime(timer_id, 0, &itime, NULL) == -1) {
        // Print an error message if setting the timer failed
        std::cerr << "Failed to set timer" << std::endl;
        // Return an error code to the OS
        return 1;
    // Close the timer set error check block
    }

    // Start a for loop to wait for pulses the specified number of times
    for (int i = 0; i < ITERATIONS; i++) {
        // Declare a pulse structure to receive the incoming kernel message
        struct _pulse pulse;
        // Block and wait to receive a message on our channel
        int rcvid = MsgReceive(chid, &pulse, sizeof(pulse), NULL);
        
        // Check if the received message is a valid pulse
        if (rcvid == 0 && pulse.code == PULSE_CODE_TIMER) {
            // Call the workload function that takes ~2ms to execute
            simulate_workload();
        // Close the pulse check block
        }
    // Close the for loop
    }
    
    // Record the end time using the monotonic clock after all iterations
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    // Calculate the total elapsed time for the good design
    elapsed_ms = diff_timespec_ms(&start_time, &end_time);
    // Print the expected time (iterations * 5ms = 50ms)
    std::cout << "Expected time: " << (ITERATIONS * CYCLE_TIME_MS) << " ms" << std::endl;
    // Print the actual elapsed time, demonstrating strict deterministic timing
    std::cout << "Actual time  : " << elapsed_ms << " ms (No drift!)" << std::endl;

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
