#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MEM_SIZE 256
#define LINE_SIZE 100
#define MNEMONIC_COUNT 11

typedef enum {
    NOP, STA, LDA, ADD, OR, AND, NOT, JMP, JN, JZ, HLT, INVALID
} Mnemonic;

int assembler(void);

#endif
