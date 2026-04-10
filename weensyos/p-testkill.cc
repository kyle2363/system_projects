#include "u-lib.hh"

void process_main() {
    pid_t pid = sys_fork();

    if (pid == 0) {
        // child spins forever
        while (1) { }
    } else {
        // parent kills child
        sys_kill(pid);
    }
}