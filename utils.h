#include <stdio.h>
#include <stdarg.h>

#define RESET       "\033[0m"
#define UNDERLINE   "\033[4m"
#define RED         "\033[31m"
#define GREEN       "\033[32m"
#define YELLOW      "\033[33m"
#define BLUE        "\033[34m"

void ceprintf(const char *color, const char *fmt, ...) {
   fprintf(stderr, "%s", color);
   va_list args;
   va_start(args, fmt);
   vfprintf(stderr, fmt, args);
   va_end(args);
   fprintf(stderr, "%s", RESET);
}

void cprintf(const char *color, const char *fmt, ...) {
   printf("%s", color);
   va_list args;
   va_start(args, fmt);
   vprintf(fmt, args);
   va_end(args);
   printf("%s", RESET);
}

