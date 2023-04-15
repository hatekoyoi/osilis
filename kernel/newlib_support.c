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

caddr_t
sbrk(int incr) {
    errno = ENOMEM;
    return (caddr_t)-1;
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
