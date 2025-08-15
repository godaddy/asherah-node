#include <stdio.h>
#include <signal.h>
#include <unistd.h>

static void print_signal_status(const char* name, int sig) {
    sigset_t mask;
    sigprocmask(SIG_BLOCK, NULL, &mask);
    int blocked = sigismember(&mask, sig);
    printf("%s (%d): %s\n", name, sig, blocked ? "BLOCKED" : "unblocked");
}

int main() {
    printf("Signal mask status:\n");
    print_signal_status("SIGPROF", SIGPROF);
    print_signal_status("SIGPIPE", SIGPIPE);
    print_signal_status("SIGXFSZ", SIGXFSZ);
    print_signal_status("SIGTERM", SIGTERM);
    print_signal_status("SIGINT", SIGINT);
    print_signal_status("SIGUSR1", SIGUSR1);
    print_signal_status("SIGUSR2", SIGUSR2);
    print_signal_status("SIGCHLD", SIGCHLD);
    
    return 0;
}