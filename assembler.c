#include "assembler.h"

typedef struct {
    const char *mnemonic;
    uint16_t opcode;
} MnemonicTable;

const MnemonicTable mnemonic_table[MNEMONIC_COUNT] = {
    {"NOP", 0x00}, {"STA", 0x10}, {"LDA", 0x20}, {"ADD", 0x30},
    {"OR", 0x40},  {"AND", 0x50}, {"NOT", 0x60}, {"JMP", 0x80},
    {"JN", 0x90},  {"JZ", 0xA0},  {"HLT", 0xF0}
};

Mnemonic get_mnemonic(const char *mnemonic) {
    for (int i = 0; i < MNEMONIC_COUNT; i++) {
        if (strcmp(mnemonic, mnemonic_table[i].mnemonic) == 0) {
            return i;
        }
    }
    return INVALID;
}

int write_instruction(uint16_t *mem, int *pos, Mnemonic mnemonic, uint16_t value) {
    if (mnemonic == INVALID) {
        fprintf(stderr, "Erro: Mnemônico inválido!\n");
        return -1;
    }

    if (*pos >= MEM_SIZE - 1) {
        fprintf(stderr, "Erro: Estouro de memória!\n");
        return -1;
    }

    if (mnemonic == NOP || mnemonic == NOT || mnemonic == HLT) {
        mem[*pos] = mnemonic_table[mnemonic].opcode;
        (*pos)++;
    } else {
        mem[*pos] = mnemonic_table[mnemonic].opcode;
        mem[*pos + 1] = value;
        (*pos) += 2;
    }

    return 0;
}

int save_memory(uint16_t *mem) {
    FILE *file = fopen("memoria.mem", "wb");
    if (!file) {
        perror("Erro ao abrir arquivo de saída");
        return -1;
    }

    uint8_t headers[4] = {0x03, 0x4E, 0x44, 0x52};  // "03NDR"
    if (fwrite(headers, sizeof(headers), 1, file) != 1) {
        perror("Erro ao escrever cabeçalho");
        fclose(file);
        return -1;
    }

    if (fwrite(mem, sizeof(uint16_t), MEM_SIZE, file) != MEM_SIZE) {
        perror("Erro ao escrever memória");
        fclose(file);
        return -1;
    }

    fclose(file);
    return 0;
}

int process_code_block(FILE *file, uint16_t *mem, int *pos) {
    char line[LINE_SIZE];

    while (fgets(line, sizeof(line), file)) {
        char mnemonic_str[10];
        int value = 0;
        int args = sscanf(line, "%s %d", mnemonic_str, &value);

        if (strncmp(mnemonic_str, "END", 3) == 0) break;

        Mnemonic mnemonic = get_mnemonic(mnemonic_str);
        if ((args == 1 && write_instruction(mem, pos, mnemonic, 0) < 0) ||
            (args == 2 && write_instruction(mem, pos, mnemonic, value) < 0)) {
            return -1;
        }
    }
    return 0;
}

int process_data_block(FILE *file, uint16_t *mem) {
    char line[LINE_SIZE];

    while (fgets(line, sizeof(line), file)) {
        int address, value;
        if (sscanf(line, "%d %d", &address, &value) == 2) {
            if (address < 0 || address >= MEM_SIZE) {
                fprintf(stderr, "Erro: Endereço inválido (%d)!\n", address);
                return -1;
            }
            if (value > 255) {
                fprintf(stderr, "Erro: Valor %d maior que 255!\n", value);
                return -1;
            }
            mem[address] = value;
        }
        if (strncmp(line, "END", 3) == 0) break;
    }
    return 0;
}

int assembler(void) {
    FILE *file = fopen("assembly.txt", "r");
    if (!file) {
        perror("Erro ao abrir o arquivo de entrada");
        return EXIT_FAILURE;
    }

    uint16_t mem[MEM_SIZE] = {0};
    char line[LINE_SIZE];
    int pos = 0;

    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "DATA", 4) == 0) {
            if (process_data_block(file, mem) < 0) {
                fclose(file);
                return EXIT_FAILURE;
            }
        } else if (strncmp(line, "CODE", 4) == 0) {
            if (process_code_block(file, mem, &pos) < 0) {
                fclose(file);
                return EXIT_FAILURE;
            }
        }
    }

    fclose(file);
    if (save_memory(mem) < 0) {
        return EXIT_FAILURE;
    }

    printf("Assembler concluído com sucesso!\n");
    return EXIT_SUCCESS;
}
