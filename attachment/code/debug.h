#include <stdio.h>
#include <stdarg.h>

#define DEBUG 0

void Dprintf(const char *format, ...) {
    if (!DEBUG) return;

    va_list arg;
    va_start(arg, format);
    vprintf(format, arg);
    va_end(arg);
}