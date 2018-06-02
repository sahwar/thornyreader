
#include "debug_generate_crash.h"
#include <unistd.h>
#include <cstdio>

static volatile bool true_var = true;

void debug_generate_sigsegv_segv_maperr()
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

void debug_proxy_call_complex_two()
{
#ifdef TRDEBUG
    printf("debug_proxy_call_complex_two");
    debug_generate_sigsegv_segv_maperr();
#endif
}

static void debug_proxy_call_oneline_static()
{
#ifdef TRDEBUG
    debug_proxy_call_complex_two();
#endif
}

void debug_proxy_call_oneline()
{
#ifdef TRDEBUG
    debug_proxy_call_oneline_static();
#endif
}

void debug_generate_long_backtrace()
{
#ifdef TRDEBUG
    printf("debug_generate_long_backtrace");
    debug_proxy_call_oneline();
#endif
}