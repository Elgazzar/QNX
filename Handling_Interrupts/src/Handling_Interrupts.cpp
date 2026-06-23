#include <iostream>
#include <cstdlib>
#include <pthread.h>
#include <sys/neutrino.h>
#include <sys/procmgr.h>
#include <hw/inout.h>
#include <unistd.h>

using namespace std;

// We will use a dummy interrupt vector (e.g., a timer or serial port).
// Note: In a real system, you must use a valid hardware interrupt vector.
#define DUMMY_INTR_VECTOR 0 

void demo_attach_thread() {
    cout << "\n--- Demo: InterruptAttachThread ---" << endl;
    
    // Attaches THIS thread directly to the hardware interrupt.
    // The thread becomes a dedicated Interrupt Service Thread (IST).
    int intr_id = InterruptAttachThread(DUMMY_INTR_VECTOR, _NTO_INTR_FLAGS_TRK_MSK);
    if (intr_id == -1) {
        cerr << "Error: InterruptAttachThread failed." << endl;
        return;
    }
    cout << "Thread attached as IST. Simulating waiting for interrupt..." << endl;
    
    // Simulate a timeout so we don't hang in a mock environment without real HW
    uint64_t timeout = 1000000000; // 1 second timeout
    TimerTimeout(CLOCK_REALTIME, _NTO_TIMEOUT_INTR, NULL, &timeout, NULL);
    
    // Thread blocks here until the hardware interrupt fires
    if (InterruptWait(0, NULL) == -1) {
        cout << "InterruptWait timed out (expected in mock environment)." << endl;
    } else {
        cout << "Interrupt fired!" << endl;
        
        InterruptMask(DUMMY_INTR_VECTOR, intr_id);
        cout << "Processing data from hardware..." << endl;
        InterruptUnmask(DUMMY_INTR_VECTOR, intr_id);
    }
    
    InterruptDetach(intr_id);
    cout << "Detached IST thread." << endl;
}

void demo_attach_event() {
    cout << "\n--- Demo: InterruptAttachEvent ---" << endl;
    
    struct sigevent event;
    int intr_id;

    // Initialize the event for InterruptWait. 
    SIGEV_INTR_INIT(&event);

    // Attach the event to our specific hardware interrupt vector.
    intr_id = InterruptAttachEvent(DUMMY_INTR_VECTOR, &event, _NTO_INTR_FLAGS_TRK_MSK);
    if (intr_id == -1) {
        cerr << "Error: InterruptAttachEvent failed." << endl;
        return;
    }
    
    cout << "Event attached. Simulating waiting for interrupt..." << endl;
    
    uint64_t timeout = 1000000000; // 1 second timeout
    TimerTimeout(CLOCK_REALTIME, _NTO_TIMEOUT_INTR, NULL, &timeout, NULL);
    
    // Thread blocks until the OS delivers the SIGEV_INTR event
    if (InterruptWait(0, NULL) == -1) {
        cout << "InterruptWait timed out (expected in mock environment)." << endl;
    } else {
        cout << "Interrupt fired!" << endl;
        
        InterruptMask(DUMMY_INTR_VECTOR, intr_id);
        cout << "Processing data from hardware..." << endl;
        InterruptUnmask(DUMMY_INTR_VECTOR, intr_id);
    }
    
    InterruptDetach(intr_id);
    cout << "Detached event." << endl;
}


int main() {
    cout << "--- QNX Interrupt Management Example ---" << endl;

    // 1. Request security abilities from the Process Manager
    // PROCMGR_AID_INTERRUPT: Required to attach to interrupts
    // PROCMGR_AID_IO: Required to use ThreadCtl(_NTO_TCTL_IO, 0)
    if (procmgr_ability(0, 
            PROCMGR_ADN_ROOT | PROCMGR_AOP_ALLOW | PROCMGR_AID_INTERRUPT,
            PROCMGR_ADN_ROOT | PROCMGR_AOP_ALLOW | PROCMGR_AID_IO,
            PROCMGR_AID_EOL) == -1) {
        cerr << "Warning: Could not set abilities. (May not be root?)" << endl;
    }

    // 2. Activate I/O privileges for this thread
    // This requires the PROCMGR_AID_IO ability we just requested.
    // It is mandatory for InterruptDisable() and InterruptEnable().
    if (ThreadCtl(_NTO_TCTL_IO, 0) == -1) {
        cerr << "Error: ThreadCtl failed to grant I/O privileges." << endl;
        return EXIT_FAILURE;
    }

    // --- InterruptDisable and InterruptEnable ---
    // Example of a tiny critical section where we disable ALL hardware interrupts globally.
    // CAUTION: This affects the entire CPU. Use only for very brief periods!
    InterruptDisable();
    // ... hardware critical section ...
    InterruptEnable();
    cout << "Successfully tested global InterruptDisable/Enable." << endl;

    // Run the two demonstrations sequentially
    demo_attach_thread();
    demo_attach_event();

    return EXIT_SUCCESS;
}
