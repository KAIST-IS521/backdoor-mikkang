// This is where you put your VM code.
// I am trying to follow the coding style of the original.

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "minivm.h"

#define NUM_REGS   (256)
#define NUM_FUNCS  (256)

// Global variable that indicates if the process is running.
static bool is_running = true;
void *heap = NULL;

// Implement Opcode function
void mini_halt(struct VMContext* ctx, const uint32_t instr) {
    is_running = false;
}

void mini_load(struct VMContext* ctx, const uint32_t instr) {

}

void mini_store(struct VMContext* ctx, const uint32_t instr) {
}

void mini_move(struct VMContext* ctx, const uint32_t instr) {
}

void mini_puti(struct VMContext* ctx, const uint32_t instr) {
}

void mini_add(struct VMContext* ctx, const uint32_t instr) {
}

void mini_sub(struct VMContext* ctx, const uint32_t instr) {
}

void mini_gt(struct VMContext* ctx, const uint32_t instr) {
}

void mini_ge(struct VMContext* ctx, const uint32_t instr) {
}

void mini_eq(struct VMContext* ctx, const uint32_t instr) {
}

void mini_ite(struct VMContext* ctx, const uint32_t instr) {
}

void mini_jump(struct VMContext* ctx, const uint32_t instr) {
}

void mini_puts(struct VMContext* ctx, const uint32_t instr) {
}

void mini_gets(struct VMContext* ctx, const uint32_t instr) {
}

void usageExit() {
    printf("[*]Usage: interpreter <bytecode>\n");
    exit(1);
}

void initFuncs(FunPtr *f, uint32_t cnt) {
    uint32_t i;
    for (i = 0; i < cnt; i++) {
        f[i] = NULL;
    }

    f[0x00] = mini_halt;
    f[0x10] = mini_load;
    f[0x20] = mini_store;
    f[0x30] = mini_move;
    f[0x40] = mini_puti;
    f[0x50] = mini_add;
    f[0x60] = mini_sub;
    f[0x70] = mini_gt;
    f[0x80] = mini_ge;
    f[0x90] = mini_eq;
    f[0xa0] = mini_ite;
    f[0xb0] = mini_jump;
    f[0xc0] = mini_puts;
    f[0xd0] = mini_gets;
}

void initRegs(Reg *r, uint32_t cnt)
{
    uint32_t i;
    for (i = 0; i < cnt; i++) {
        r[i].type = 0;
        r[i].value = 0;
    }
}

int main(int argc, char** argv) {
    VMContext vm;
    Reg r[NUM_REGS];
    FunPtr f[NUM_FUNCS];
    FILE* bytecode;
    uint32_t* pc;

    // There should be at least one argument.
    if (argc < 2) usageExit();

    // Initialize registers.
    initRegs(r, NUM_REGS);
    // Initialize interpretation functions.
    initFuncs(f, NUM_FUNCS);
    // Initialize VM context.
    initVMContext(&vm, NUM_REGS, NUM_FUNCS, r, f);

    // Load bytecode file
    bytecode = fopen(argv[1], "rb");
    if (bytecode == NULL) {
        perror("fopen");
        return 1;
    }

    heap = malloc(8192);
    if (heap == NULL) {
        perror("malloc");
        return 1;
    }

    while (is_running) {
        // TODO: Read 4-byte bytecode, and set the pc accordingly
        stepVMContext(&vm, &pc);
    }

    fclose(bytecode);

    // Zero indicates normal termination.
    return 0;
}
