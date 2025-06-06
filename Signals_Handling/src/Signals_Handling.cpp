#include <iostream>
#include <pthread.h>
#include <signal.h>


sigset_t signal_set;

using namespace std;

void* signalHandlerThread(void* arg) {
    int received_signal;

    while (true) {
        // Wait for a signal from the set
        int ret = sigwait(&signal_set, &received_signal);
        if (ret != 0) {
            std::cerr << "sigwait failed: " << ret << std::endl;
            pthread_exit(nullptr);
        }

        std::cout << "Received signal: " << strsignal(received_signal) << " (" << received_signal << ")" << std::endl;

        // Optional: exit on SIGTERM or SIGINT
        if (received_signal == SIGINT || received_signal == SIGTERM) {
            std::cout << "Exiting signal handler thread..." << std::endl;
            //pthread_exit(nullptr);
        }
    }
    return nullptr; // never reached
}

int main() {

	sigemptyset(&signal_set);
    // Add signals you want to handle (e.g., SIGINT, SIGTERM)
//    sigaddset(&signal_set, SIGINT);
//    sigaddset(&signal_set, SIGTERM);
	sigfillset(&signal_set); // Block all signals that can be caught


    // Block signals in this thread (so they go to the handler thread instead)
    if (pthread_sigmask(SIG_BLOCK, &signal_set, nullptr) != 0) {
        std::cerr << "Failed to block signals" << std::endl;
        return 1;
    }
    pthread_t thread_id;

    if (pthread_create(&thread_id, nullptr, signalHandlerThread, nullptr) != 0) {
        std::cerr << "Failed to create signal handling thread" << std::endl;
        return 1;
    }

    // Wait for the signal thread to exit (optional, or do other work)
    pthread_join(thread_id, nullptr);

    std::cout << "Program exiting..." << std::endl;

	cout << "Hello World!!!" << endl; // prints Hello World!!!
	return 0;
}
