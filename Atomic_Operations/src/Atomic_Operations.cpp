#include <iostream>
#include <atomic.h>
#include <pthread.h>

using namespace std;

// Shared global variables
// Note: For atomic operations, variables should be declared 'volatile unsigned'
volatile unsigned shared_counter = 0;
volatile unsigned shared_flags = 0;

// =========================================================
// Thread 1: Math Operations
// =========================================================
void* math_thread(void* arg) {
    int id = *(int*)arg;
    
    // Both threads will rapidly loop 10,000 times trying to modify the counter simultaneously
    for (int i = 0; i < 10000; ++i) {
        
        // Safely add 2
        atomic_add(&shared_counter, 2);
        
        // Safely subtract 1
        atomic_sub(&shared_counter, 1);
        
        // Net result of each loop iteration is +1.
    }
    cout << "[Math Thread " << id << "] Finished 10,000 math loops." << endl;
    return NULL;
}

// =========================================================
// Thread 2: Bitwise Operations
// =========================================================
void* bitwise_thread(void* arg) {
    cout << "\n[Bitwise Thread] Starting bitwise manipulations..." << endl;
    
    // 1. Set bit 0 (Binary: 0001)
    atomic_set(&shared_flags, 1 << 0);
    
    // 2. Set bit 1 (Binary: 0010)
    // Flags is now Binary 0011
    atomic_set(&shared_flags, 1 << 1);
    
    // 3. Clear bit 0 (Binary: 0001)
    // Flags is now Binary 0010
    atomic_clr(&shared_flags, 1 << 0);
    
    // 4. Toggle bit 2 (Binary: 0100) -> Flips it from 0 to 1
    // Flags is now Binary 0110
    atomic_toggle(&shared_flags, 1 << 2);
    
    // 5. Toggle bit 2 again -> Flips it back from 1 to 0
    // Flags is now Binary 0010 (Decimal 2)
    atomic_toggle(&shared_flags, 1 << 2);
    
    cout << "[Bitwise Thread] Finished manipulating flags." << endl;
    return NULL;
}

// =========================================================
// MAIN
// =========================================================
int main() {
    pthread_t t1, t2;
    int id1 = 1, id2 = 2;
    
    cout << "--- QNX Atomic Operations Example ---" << endl;

    // --- TEST 1: MATH (Add / Sub) ---
    // Start two threads simultaneously racing to modify the same variable
    pthread_create(&t1, NULL, math_thread, &id1);
    pthread_create(&t2, NULL, math_thread, &id2);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    // Because we used atomic operations, NO race conditions occurred! 
    // We didn't even need a mutex.
    // Each thread ran 10,000 times, adding a net of 1 each time. 
    // Total should be exactly 20,000.
    cout << "\nFinal shared_counter: " << shared_counter << " (Expected exactly: 20000)" << endl;

    // --- TEST 2: BITWISE (Set / Clr / Toggle) ---
    pthread_create(&t1, NULL, bitwise_thread, NULL);
    pthread_join(t1, NULL);
    
    // Based on the bitwise logic above, only bit 1 should remain ON.
    // Binary 0010 is Decimal 2.
    cout << "\nFinal shared_flags: " << shared_flags << " (Expected exactly: 2)" << endl;

    return 0;
}
