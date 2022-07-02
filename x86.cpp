#ifdef __x86_64
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#endif

void printf_x86(const char *fmt, ...)
{
#ifdef __x86_64
    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
#else
    return;
#endif
}
