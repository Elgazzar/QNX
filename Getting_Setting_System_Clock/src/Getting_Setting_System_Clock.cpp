#include <iostream>
#include <time.h>
#include <errno.h>
#include <string.h>

using namespace std;

int main() {
    struct timespec ts;

    cout << "--- QNX System Clock Example ---" << endl;

    // 1. Get the current system time
    if (clock_gettime(CLOCK_REALTIME, &ts) == -1) {
        cerr << "Error getting time: " << strerror(errno) << endl;
        return 1;
    }
    
    cout << "Current time (seconds since Epoch): " << ts.tv_sec << endl;
    cout << "Nanoseconds: " << ts.tv_nsec << endl;

    // 2. Modify the time (e.g., add 1 hour)
    cout << "\nAttempting to set the clock forward by 1 hour..." << endl;
    ts.tv_sec += (60 * 60);

    // 3. Set the new system time
    // NOTE: This will fail with EPERM (Operation not permitted) unless you run this program as root.
    if (clock_settime(CLOCK_REALTIME, &ts) == -1) {
        if (errno == EPERM) {
            cerr << "Error setting time: Permission denied. You must run this program as root." << endl;
        } else {
            cerr << "Error setting time: " << strerror(errno) << endl;
        }
        return 1;
    }

    cout << "Successfully set the new time!" << endl;

    // 4. Verify the new time
    if (clock_gettime(CLOCK_REALTIME, &ts) == -1) {
        cerr << "Error getting time after setting: " << strerror(errno) << endl;
        return 1;
    }
    
    cout << "New time (seconds since Epoch): " << ts.tv_sec << endl;

    return 0;
}
