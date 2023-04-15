#include <errno.h>
#include <sys/types.h>

#ifndef _BSD_CADDR_T_
#define _BSD_CADDR_T_
typedef char* caddr_t;
#endif

caddr_t
sbrk(int incr) {
    errno = ENOMEM;
    return (caddr_t)-1;
}
