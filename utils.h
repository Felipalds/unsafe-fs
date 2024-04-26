//
// Created by Luiz Felipe Fonseca Rosa on 25/04/24.
//
#include <stdio.h>
#include <stdarg.h>


#define RED "\033[1;31m"
#define YELLOW "\033[1;33m"
#define BLUE "\033[1;34m"
#define GREEN "\033[0;32m\t"
#define WHITE "\033[4;37m"
#define RESET "\033[0m"


void ceprintf(const char *COLOR, const char *fmt, ...) {
   printf("%s", COLOR);
   va_list args;
   va_start(args, fmt);
   vfprintf(stderr, fmt, args);
   va_end(args);
   printf("%s", RESET);
}

void cprintf(const char *COLOR, const char *fmt, ...) {
   printf("%s", COLOR);
   va_list args;
   va_start(args, fmt);
   vprintf(fmt, args);
   va_end(args);
   printf("%s", RESET);
}

