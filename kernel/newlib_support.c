#include <errno.h>
#include <sys/types.h>

#ifndef _BSD_CADDR_T_
#define _BSD_CADDR_T_
typedef char* caddr_t;
#endif

void
_exit(void) {
    while (1)
        __asm__("hlt");
}

caddr_t program_break, program_break_end;

caddr_t
sbrk(int incr) {
    if (program_break == 0 || program_break + incr >= program_break_end) {
        errno = ENOMEM;
        return (caddr_t)-1;
    }

    caddr_t prev_break = program_break;
    program_break += incr;
    return prev_break;
}

int
getpid(void) {
    return 1;
}

int
kill(int pid, int sig) {
    errno = EINVAL;
    return -1;
}
