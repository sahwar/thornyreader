
#include "debug_generate_crash.h"
#include <unistd.h>

static volatile bool true_var = true;

void debug_generate_segfault()
{
#ifdef TRDEBUG
    *(int*) 0 = 0;
#endif
}

void debug_generate_busyloop()
{
#ifdef TRDEBUG
    while (true_var) {
        sleep(1);
    }
#endif
}