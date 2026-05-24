#include <iostream>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>

using namespace std;

#define SHM_NAME "/my_shared_memory"
#define SHM_SIZE 1024

void run_writer() {
    cout << "[Writer] Opening shared memory..." << endl;
    
    // 1. Create and open a shared memory object
    int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (fd == -1) {
        perror("[Writer] shm_open failed");
        return;
    }

    // 2. Set the size of the shared memory object
    ftruncate(fd, SHM_SIZE);

    // 3. Map the shared memory into our virtual address space
    void *ptr = mmap(0, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) {
        perror("[Writer] mmap failed");
        return;
    }

    // 4. Close the file descriptor (we don't need it after mmap is done)
    close(fd);

    // 5. Write some data to the shared memory
    string msg = "Hello from the Writer!";
    sprintf((char*)ptr, "%s", msg.c_str());
    cout << "[Writer] Successfully wrote: '" << msg << "' to " << SHM_NAME << endl;

    // Note: we purposely don't call shm_unlink() here so the reader can access it later.
}

void run_reader() {
    cout << "[Reader] Opening shared memory..." << endl;

    // 1. Open the existing shared memory object (O_RDONLY)
    int fd = shm_open(SHM_NAME, O_RDONLY, 0666);
    if (fd == -1) {
        perror("[Reader] shm_open failed (Did you run the writer first?)");
        return;
    }

    // 2. Map the shared memory into our virtual address space (PROT_READ)
    void *ptr = mmap(0, SHM_SIZE, PROT_READ, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) {
        perror("[Reader] mmap failed");
        return;
    }

    // 3. Close the file descriptor
    close(fd);

    // 4. Read and print the data
    cout << "[Reader] Read from shared memory: '" << (char*)ptr << "'" << endl;

    // 5. Cleanup
    munmap(ptr, SHM_SIZE);
    
    // Unlink removes the object from /dev/shmem
    cout << "[Reader] Cleaning up and unlinking " << SHM_NAME << endl;
    shm_unlink(SHM_NAME);
}

int main(int argc, char *argv[]) {
    if (argc > 1 && string(argv[1]) == "write") {
        run_writer();
    } else if (argc > 1 && string(argv[1]) == "read") {
        run_reader();
    } else {
        cout << "No arguments provided (likely running from IDE Play button)." << endl;
        cout << "Running both writer and reader sequentially..." << endl;
        cout << "--------------------------------------------------------" << endl;
        
        run_writer();
        
        cout << endl;
        sleep(1); // Small pause for output readability
        
        run_reader();
        
        cout << "--------------------------------------------------------" << endl;
        cout << "Tip: You can also pass 'write' or 'read' as launch arguments." << endl;
    }

    return 0;
}
