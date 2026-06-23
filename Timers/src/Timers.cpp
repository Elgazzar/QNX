#include <iostream>
#include <time.h>
#include <sys/siginfo.h>
#include <sys/neutrino.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#define PULSE_CODE_TIMER_EXPIRED _PULSE_CODE_MINAVAIL

int main(int argc, char *argv[]) {
    std::cout << "--- QNX Timer API Example ---" << std::endl;

    // T004: Implement ClockPeriod query
    struct _clockperiod clock_res;
    if (ClockPeriod(CLOCK_REALTIME, NULL, &clock_res, 0) == -1) {
        std::cerr << "Failed to get clock period: " << strerror(errno) << std::endl;
        return 1;
    }
    std::cout << "System clock period (CLOCK_REALTIME): " << clock_res.nsec << " nanoseconds." << std::endl;

    // T008: Create a channel and connect
    int chid = ChannelCreate(0);
    if (chid == -1) {
        std::cerr << "Failed to create channel: " << strerror(errno) << std::endl;
        return 1;
    }
    
    int coid = ConnectAttach(0, 0, chid, _NTO_SIDE_CHANNEL, 0);
    if (coid == -1) {
        std::cerr << "Failed to attach connection: " << strerror(errno) << std::endl;
        ChannelDestroy(chid);
        return 1;
    }

    // T009: Update timer_create to use SIGEV_PULSE_INIT
    struct sigevent event;
    // Deliver a pulse with priority 10 to our connection
    SIGEV_PULSE_INIT(&event, coid, 10, PULSE_CODE_TIMER_EXPIRED, 0);

    // T006: Implement timer_create initialization
    timer_t timer_id;
    // We'll use CLOCK_MONOTONIC for the timer because we want an increasing timer unaffected by system time jumps
    if (timer_create(CLOCK_MONOTONIC, &event, &timer_id) == -1) {
        std::cerr << "Failed to create timer: " << strerror(errno) << std::endl;
        ConnectDetach(coid);
        ChannelDestroy(chid);
        return 1;
    }
    std::cout << "Timer created successfully (CLOCK_MONOTONIC)." << std::endl;

    // T005 & T007: Define struct itimerspec setup and timer_settime
    struct itimerspec itime;
    // Initial fire time: 1 second relative
    itime.it_value.tv_sec = 1;
    itime.it_value.tv_nsec = 0;
    // Reload value: 1 second for periodic behavior
    itime.it_interval.tv_sec = 1;
    itime.it_interval.tv_nsec = 0;

    std::cout << "Starting timer with initial fire in 1s, and repeating every 1s..." << std::endl;
    // Set relative timer (flags = 0)
    if (timer_settime(timer_id, 0, &itime, NULL) == -1) {
        std::cerr << "Failed to set timer: " << strerror(errno) << std::endl;
        timer_delete(timer_id);
        ConnectDetach(coid);
        ChannelDestroy(chid);
        return 1;
    }

    // T010 & T011: MsgReceive loop and exit condition
    int pulse_count = 0;
    const int MAX_PULSES = 5;
    
    while (pulse_count < MAX_PULSES) {
        struct _pulse pulse;
        
        // Wait for the pulse event
        int rcvid = MsgReceive(chid, &pulse, sizeof(pulse), NULL);
        
        if (rcvid == 0) { // Received a pulse
            if (pulse.code == PULSE_CODE_TIMER_EXPIRED) {
                pulse_count++;
                std::cout << "Timer Expired! Pulse count: " << pulse_count << " of " << MAX_PULSES << std::endl;
            } else {
                std::cout << "Received unknown pulse code: " << pulse.code << std::endl;
            }
        } else if (rcvid == -1) {
            std::cerr << "MsgReceive failed: " << strerror(errno) << std::endl;
            break;
        } else {
            std::cout << "Received a non-pulse message, rcvid=" << rcvid << std::endl;
        }
    }

    std::cout << "Max pulses reached. Cleaning up." << std::endl;

    // Clean up
    timer_delete(timer_id);
    ConnectDetach(coid);
    ChannelDestroy(chid);

    std::cout << "Exiting gracefully." << std::endl;
    return 0;
}
