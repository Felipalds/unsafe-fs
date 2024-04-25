//
// Created by Luiz Felipe Fonseca Rosa on 25/04/24.
//
#ifndef UNSAFE_FS_UTILS_H
#define UNSAFE_FS_UTILS_H
#include <stdio.h>
#include "consts.h"

void print(char *COLOR, char *message) {
   printf("%s%s%s",COLOR, message, RESET );
}

#endif //UNSAFE_FS_UTILS_H
