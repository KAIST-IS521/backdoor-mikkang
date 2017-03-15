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
uint8_t *heap = NULL;

void check_addr_range(uint32_t addr) {
    if (addr >= 8192) {
        perror("addr is out of range");
        exit(1);
    }
}

uint8_t read_mem(uint32_t addr) {
    check_addr_range(addr);
    return *(heap + addr);
}

void write_mem(uint32_t addr, uint8_t val) {
    check_addr_range(addr);
    *(heap + addr) = val;
}

// Implement Opcode function
void mini_halt(struct VMContext* ctx, const uint32_t instr) {
    is_running = false;
}

void mini_load(struct VMContext* ctx, const uint32_t instr) {
    const uint8_t r0 = EXTRACT_B1(instr);
    const uint8_t r1 = EXTRACT_B2(instr);
    const uint32_t addr = ctx->r[r1].value;
    ctx->r[r0].value = read_mem(addr);
}

void mini_store(struct VMContext* ctx, const uint32_t instr) {
    const uint8_t r0 = EXTRACT_B1(instr);
    const uint8_t r1 = EXTRACT_B2(instr);
    const uint32_t addr = ctx->r[r0].value;
    write_mem(addr, EXTRACT_B0(ctx->r[r1].value));
}

void mini_move(struct VMContext* ctx, const uint32_t instr) {
    const uint8_t r0 = EXTRACT_B1(instr);
    const uint8_t r1 = EXTRACT_B2(instr);
    ctx->r[r1].value = ctx->r[r0].value;
}

void mini_puti(struct VMContext* ctx, const uint32_t instr) {
    const uint8_t r0 = EXTRACT_B1(instr);
    const uint8_t imm = EXTRACT_B2(instr);
    ctx->r[r0].value = 0x000000ff & imm;
}

void mini_add(struct VMContext* ctx, const uint32_t instr) {
    const uint8_t r0 = EXTRACT_B1(instr);
    const uint8_t r1 = EXTRACT_B2(instr);
    const uint8_t r2 = EXTRACT_B3(instr);
    ctx->r[r0].value = ctx->r[r1].value + ctx->r[r2].value;
}

void mini_sub(struct VMContext* ctx, const uint32_t instr) {
    const uint8_t r0 = EXTRACT_B1(instr);
    const uint8_t r1 = EXTRACT_B2(instr);
    const uint8_t r2 = EXTRACT_B3(instr);
    ctx->r[r0].value = ctx->r[r2].value + ctx->r[r1].value;
}

void mini_gt(struct VMContext* ctx, const uint32_t instr) {
    const uint8_t r0 = EXTRACT_B1(instr);
    const uint8_t r1 = EXTRACT_B2(instr);
    const uint8_t r2 = EXTRACT_B3(instr);
    if (ctx->r[r1].value > ctx->r[r2].value)
        ctx->r[r0].value = 1;
    else
        ctx->r[r0].value = 0;
}

void mini_ge(struct VMContext* ctx, const uint32_t instr) {
    const uint8_t r0 = EXTRACT_B1(instr);
    const uint8_t r1 = EXTRACT_B2(instr);
    const uint8_t r2 = EXTRACT_B3(instr);
    if (ctx->r[r1].value >= ctx->r[r2].value)
        ctx->r[r0].value = 1;
    else
        ctx->r[r0].value = 0;
}

void mini_eq(struct VMContext* ctx, const uint32_t instr) {
    const uint8_t r0 = EXTRACT_B1(instr);
    const uint8_t r1 = EXTRACT_B2(instr);
    const uint8_t r2 = EXTRACT_B3(instr);
    if (ctx->r[r1].value == ctx->r[r2].value)
        ctx->r[r0].value = 1;
    else
        ctx->r[r0].value = 0;
}

void mini_ite(struct VMContext* ctx, const uint32_t instr) {
    const uint8_t r0 = EXTRACT_B1(instr);
    const uint8_t imm1 = EXTRACT_B2(instr);
    const uint8_t imm2 = EXTRACT_B3(instr);
    if (ctx->r[r0].value > 0)
        ctx->pc = imm1;
    else
        ctx->pc = imm2;
}

void mini_jump(struct VMContext* ctx, const uint32_t instr) {
    const uint8_t imm = EXTRACT_B1(instr);
    ctx->pc = imm;
}

void mini_puts(struct VMContext* ctx, const uint32_t instr) {
    const uint8_t r0 = EXTRACT_B1(instr);
    uint32_t addr = ctx->r[r0].value;
    uint8_t val;
    while(true) {
        val = read_mem(addr);
        if (val) {
            putchar(val);
            addr++;
        }
        else
            break;
    }
}

void mini_gets(struct VMContext* ctx, const uint32_t instr) {
    const uint8_t r0 = EXTRACT_B1(instr);
    uint32_t addr = ctx->r[r0].value;
    uint8_t val;
    while(true) {
        val = (uint8_t) getchar();
        if (val == '\n') break;
        write_mem(addr, val);
        addr++;
    }
    write_mem(addr, '\0');
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

    heap = (uint8_t*) malloc(8192);
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
