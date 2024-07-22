#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#ifndef TERNARY_RISC_SIM_H
#define TERNARY_RISC_SIM_H 1

//#define DEBUG 1

#define MAX_I6 (364)
#define MAX_I18 (193710244)
#define RANGE_1TRIT (3)
#define RANGE_2TRIT (9)
#define RANGE_3TRIT (27)
#define RANGE_4TRIT (81)
#define RANGE_5TRIT (243)
#define RANGE_6TRIT (729)
#define RANGE_7TRIT (2187)
#define RANGE_8TRIT (6561)
#define RANGE_9TRIT (19683)
#define RANGE_10TRIT (59049)
#define RANGE_11TRIT (177147)
#define RANGE_12TRIT (531441)
#define RANGE_13TRIT (1594323)
#define RANGE_14TRIT (4782969)
#define RANGE_15TRIT (14348907)
#define RANGE_16TRIT (43046721)
#define RANGE_17TRIT (129140163)
#define RANGE_18TRIT (387420489)
#define BIAS_1TRIT ((RANGE_1TRIT-1)/2)
#define BIAS_2TRIT ((RANGE_2TRIT-1)/2)
#define BIAS_3TRIT ((RANGE_3TRIT-1)/2)
#define BIAS_4TRIT ((RANGE_4TRIT-1)/2)
#define BIAS_5TRIT ((RANGE_5TRIT-1)/2)
#define BIAS_6TRIT ((RANGE_6TRIT-1)/2)
#define BIAS_7TRIT ((RANGE_7TRIT-1)/2)
#define BIAS_8TRIT ((RANGE_8TRIT-1)/2)
#define BIAS_9TRIT ((RANGE_9TRIT-1)/2)
#define BIAS_10TRIT ((RANGE_10TRIT-1)/2)
#define BIAS_11TRIT ((RANGE_11TRIT-1)/2)
#define BIAS_12TRIT ((RANGE_12TRIT-1)/2)
#define BIAS_13TRIT ((RANGE_13TRIT-1)/2)
#define BIAS_14TRIT ((RANGE_14TRIT-1)/2)
#define BIAS_15TRIT ((RANGE_15TRIT-1)/2)
#define BIAS_16TRIT ((RANGE_16TRIT-1)/2)
#define BIAS_17TRIT ((RANGE_17TRIT-1)/2)
#define BIAS_18TRIT ((RANGE_18TRIT-1)/2)

#define MAX_F18 (18236463873396711424.0)
#define INFINITY_F18 (18236486750189166592.0)

// Machine parameters
#define IMEM_SIZE_WORDS (RANGE_12TRIT)
#define DMEM_SIZE_WORDS (RANGE_12TRIT)
#define SYSCALL_TABLE_SIZE (RANGE_4TRIT)

// These are just on my machine, will need to be made portable
typedef signed char int8;
typedef unsigned char uint8;
typedef signed short int16;
typedef unsigned short uint16;
typedef signed int int32;
typedef unsigned int uint32;
typedef signed long int64;
typedef unsigned long uint64;
typedef float float32;
typedef double float64;

// Adding any more fields to this will increase the size dramatically, consider repurposing fields if possible
typedef struct {
    uint8 opcode;
    uint8 rd;
    uint8 rs1;
    uint8 rs2;
    uint8 func;
    uint8 flagbank_inc;
    uint8 size_state;
    uint8 val;
    uint32 imm;
} decoded_instruction;

#define guard(n) asm("#" #n)
//#define guard(n) printf("%d ", n)

#ifdef DEBUG
#define DEBUG_PRINT(str) do { printf(str); } while(0)
#define DEBUG_PRINT1(str, v1) do { printf(str, v1); } while(0)
#define DEBUG_PRINT2(str, v1, v2) do { printf(str, v1, v2); } while(0)
#define DEBUG_PRINT3(str, v1, v2, v3) do { printf(str, v1, v2, v3); } while(0)
#define DEBUG_PRINT4(str, v1, v2, v3, v4) do { printf(str, v1, v2, v3, v4); } while(0)
#define DEBUG_PRINT5(str, v1, v2, v3, v4, v5) do { printf(str, v1, v2, v3, v4, v5); } while(0)
#define DEBUG_PRINT6(str, v1, v2, v3, v4, v5, v6) do { printf(str, v1, v2, v3, v4, v5, v6); } while(0)
#else
#define DEBUG_PRINT(str) do {} while(0)
#define DEBUG_PRINT1(str, v1) do {} while(0)
#define DEBUG_PRINT2(str, v1, v2) do {} while(0)
#define DEBUG_PRINT3(str, v1, v2, v3) do {} while(0)
#define DEBUG_PRINT4(str, v1, v2, v3, v4) do {} while(0)
#define DEBUG_PRINT5(str, v1, v2, v3, v4, v5) do {} while(0)
#define DEBUG_PRINT6(str, v1, v2, v3, v4, v5, v6) do {} while(0)
#endif

enum IFlag {IS = 0, IV, IT};
enum FFlag {FS = 0, FV, FE};

int32 power(int32 base, int32 exp){
    if (exp == 0)
        return 1;
    int temp = power(base, exp/2);
    if (exp%2 == 0)
        return temp*temp;
    else
        return base*temp*temp;
}

/* Notes:
	For this version, all values are stored in binary and only converting them to ternary when needed.
    Might need to rethink this for floating point, especially emulating zero-boxing.
    All instruction bitfields are treated as unbalanced ternary (0 to 2), but emulated values are balanced ternary (-1 to 1)
*/

uint32 extract_opcode(uint32 instr) {
    return instr % RANGE_3TRIT;
}

uint32 extract_rd(uint32 instr) {
    return (instr / RANGE_3TRIT) % RANGE_3TRIT;
}

uint32 extract_rs1(uint32 instr) {
    return (instr / RANGE_8TRIT) % RANGE_3TRIT;
}

uint32 extract_rs2(uint32 instr) {
    return (instr / RANGE_11TRIT) % RANGE_3TRIT;
}

uint32 extract_r_func(uint32 instr) {
    return ((instr / RANGE_6TRIT) % RANGE_2TRIT) + RANGE_2TRIT * ((instr / RANGE_14TRIT) % RANGE_2TRIT);
}

uint32 extract_regsize(uint32 instr) {
    return (instr / RANGE_17TRIT);
}

uint32 extract_r_flagbank(uint32 instr) {
    return (instr / RANGE_16TRIT) % RANGE_1TRIT;
}

uint32 extract_t_func(uint32 instr) {
    return ((instr / RANGE_6TRIT) % RANGE_2TRIT);
}

uint32 extract_mov_src_reg_type(uint32 instr) {
    return (instr / RANGE_12TRIT) % RANGE_1TRIT;
}

uint32 extract_mov_dst_reg_type(uint32 instr) {
    return (instr / RANGE_11TRIT) % RANGE_1TRIT;
}

int32 extract_shift_amount(uint32 instr) {
    return ((instr / RANGE_11TRIT) % RANGE_5TRIT) - 121;
}

uint32 extract_shift_value(uint32 instr) {
    return (instr / RANGE_16TRIT) % RANGE_1TRIT;
}

uint32 extract_i_func(uint32 instr) {
    return (instr % RANGE_1TRIT) + RANGE_1TRIT * ((instr / RANGE_6TRIT) % RANGE_1TRIT);
}

int32 extract_i_imm(uint32 instr) {
    return (((instr / RANGE_7TRIT) % RANGE_1TRIT) + RANGE_1TRIT * ((instr / RANGE_11TRIT) % RANGE_5TRIT)) - 364;
}

uint32 extract_load_func(uint32 instr) {
    return ((instr / RANGE_6TRIT) % RANGE_1TRIT);
}

uint32 extract_s_func(uint32 instr) {
    return ((instr / RANGE_6TRIT) % RANGE_1TRIT);
}

int32 extract_s_imm(uint32 instr) {
    return (((instr / RANGE_7TRIT) % RANGE_1TRIT) + RANGE_1TRIT * ((instr / RANGE_3TRIT) % RANGE_3TRIT) + RANGE_5TRIT * ((instr / RANGE_14TRIT) % RANGE_2TRIT)) - 364;
}

int32 extract_u_imm(uint32 instr) {
    uint32 val = instr / RANGE_6TRIT;
    val = (val * RANGE_6TRIT) - 193709880;
    return val;
}

int32 extract_j_imm(uint32 instr) {
    uint32 imm = RANGE_1TRIT * (((instr / RANGE_7TRIT) % RANGE_11TRIT) + RANGE_11TRIT * ((instr / RANGE_3TRIT) % RANGE_4TRIT));
    imm -= 21523360;
    return imm;
}

int32 extract_l_imm(uint32 instr) {
    uint32 imm = RANGE_1TRIT * ((instr / RANGE_11TRIT) % RANGE_7TRIT) + RANGE_8TRIT *((instr / RANGE_3TRIT) % RANGE_5TRIT);
    imm -= 797160;
    return imm;
}

uint32 extract_inc(uint32 instr) {
    return (instr / RANGE_16TRIT) % RANGE_1TRIT;
}

uint32 extract_syscalli_imm(uint32 instr) {
    return RANGE_1TRIT * ((instr / RANGE_7TRIT) % RANGE_9TRIT);
}

uint32 extract_syscalli_val(uint32 instr) {
    return ((instr / RANGE_16TRIT) % RANGE_2TRIT) + RANGE_2TRIT * ((instr / RANGE_3TRIT) % RANGE_4TRIT);
}

uint32 extract_l_func(uint32 instr) {
    return (instr / RANGE_6TRIT) % RANGE_1TRIT;
}

int32 extract_b_imm(uint32 instr) {
    int32 imm  = RANGE_1TRIT * (instr / RANGE_7TRIT) + RANGE_12TRIT * ((instr / RANGE_3TRIT) % RANGE_1TRIT);
    return imm - 797163;
}

uint32 extract_b_flag(uint32 instr) {
    return (instr / RANGE_4TRIT) % RANGE_1TRIT;
}

uint32 extract_b_flagbank(uint32 instr) {
    return (instr / RANGE_5TRIT) % RANGE_1TRIT;
}

uint32 extract_b_state(uint32 instr) {
    return (instr / RANGE_6TRIT) % RANGE_1TRIT;
}

int32 get_dmem_i(int32 *mem, int32 loc, uint32 size) {
    uint32 memloc = loc / 3;
    uint32 offset = loc % RANGE_1TRIT;

    if (memloc > DMEM_SIZE_WORDS) return 0;
    if (memloc < 0) return 0; // TODO: Handle system memory access (negative indices)

    if (size == 0) {
        if (offset == 0) return mem[memloc] % RANGE_6TRIT;        
        if (offset == 1) return (mem[memloc] / RANGE_6TRIT) % RANGE_6TRIT;        
        if (offset == 2) return mem[memloc] / RANGE_12TRIT;        
    }

    if (offset == 0) return mem[memloc];
    if (offset == 1) return (mem[memloc] / RANGE_6TRIT) + RANGE_12TRIT * (mem[memloc+1] % RANGE_6TRIT);
    if (offset == 2) return (mem[memloc] / RANGE_12TRIT) + RANGE_6TRIT * (mem[memloc+1] % RANGE_12TRIT);
    return 0;
}

float32 get_dmem_f(int32 *mem, int32 loc) {
    uint32 memloc = loc / 3;
    uint32 offset = loc % RANGE_1TRIT;
    if (offset != 0) return (float32)0.0;

    if (memloc > DMEM_SIZE_WORDS) return (float32)0.0;
    if (memloc < 0) return (float32)0.0; // TODO: Handle system memory access (negative indices)

    int32 a = mem[memloc];
    return *(float32 *)&a;
}

void set_dmem_i(int32 *mem, int32 loc, int32 val, uint32 size) {
    if (loc > DMEM_SIZE_WORDS) return;
    if (loc < 0) return; // TODO: Handle system memory access (negative indices)

    uint32 memloc = loc / 3;
    uint32 offset = loc % RANGE_1TRIT;
    int32 a, b;

    if (size == 0) {
        if (offset == 0) { a = mem[memloc]; b = a % RANGE_6TRIT; a = a - b + val; return; }
        if (offset == 1) { a = mem[memloc]; b = (a / RANGE_6TRIT) % RANGE_6TRIT; a = a - b + (val * RANGE_6TRIT); return; }
        if (offset == 2) { a = mem[memloc]; b = (a / RANGE_12TRIT) % RANGE_6TRIT; a = a - b + (val * RANGE_12TRIT); return; }
    }

    if (offset == 0) mem[memloc] = val;
    if (offset == 1) { 
        a = mem[memloc]; b = (a / RANGE_6TRIT) * RANGE_6TRIT; a = a - b + ((val % RANGE_12TRIT) * RANGE_6TRIT); mem[memloc] = a;
        a = mem[memloc+1]; b = (a % RANGE_6TRIT); a = a - b + (val / RANGE_12TRIT); mem[memloc+1] = a;
    }
    if (offset == 2) { 
        a = mem[memloc]; b = (a / RANGE_12TRIT) * RANGE_12TRIT; a = a - b + ((val % RANGE_6TRIT) * RANGE_12TRIT); mem[memloc] = a;
        a = mem[memloc+1]; b = (a % RANGE_12TRIT); a = a - b + (val / RANGE_6TRIT); mem[memloc+1] = a;
    }
}

void set_dmem_f(int32 *mem, int32 loc, float32 val) {
    if (loc > DMEM_SIZE_WORDS) return;
    if (loc < 0) return;

    uint32 memloc = loc / 3;
    uint32 offset = loc % RANGE_1TRIT;
    if (offset != 0) return;

    int32 a = *(int32 *)&val;
    mem[memloc] = a;
}

uint32* ip_from_pc(int32 pc, uint32 *imem) {
    int32 scaled = (pc / 3);
    uint32 *ptr = &(imem[scaled]);
    return ptr;
}

decoded_instruction *dip_from_pc(int32 pc, decoded_instruction *dec_imem) {
    int32 scaled = (pc / 3);
    decoded_instruction *ptr = &(dec_imem[scaled]);
    return ptr;
}

int32 pc_from_ip(uint32 *ip, uint32 *imem) { 
    // TODO: will need to handle negative PC values (system memory?)
    return (int32)(3 * (((uint64)ip - (uint64)imem) / 4));
}

int32 pc_from_dip(decoded_instruction *dip, decoded_instruction *dec_imem) { 
    // TODO: will need to handle negative PC values (system memory?)
    return (int32)(3 * (((uint64)dip - (uint64)dec_imem) / sizeof(decoded_instruction)));
}

void set_reg_f18(float32 *regs, uint32 reg, float32 val) {
    regs[reg] = val;
}

float32 get_reg_f18(float32 *regs, uint32 reg) {
    return regs[reg];
}

void set_reg_i(int32 *regs, uint32 reg, int32 val, uint32 size) {
    if (reg == 0) return;
    int32 res = val;
    if (size == 0) {
        if (res >= 0) res = res % MAX_I6;
        res = -(-res % MAX_I6);
    }
    regs[reg] = res;
}

int32 get_reg_i(int32 *regs, uint32 reg, uint32 size) {
    int32 res = regs[reg];
    if (size == 0) {
        if (res >= 0) return res % MAX_I6;
        return -(-res % MAX_I6);
    }
    return res;
}

uint32 get_cpu_flag(uint32 flag, uint32 flagbank, uint32 *flagregs) {
    return flagregs[flagbank * 3 + flag];
}

void set_cpu_flag(uint32 flag, uint32 flagbank, uint32 *flagregs, uint32 value) {
    flagregs[flagbank * 3 + flag] = value;
}

int32 ternary_and_18(int32 a, int32 b) {
    // Ternary and is equivalent to the min of a and b
    int32 res, tmp_a, tmp_b, tmp_res;
    int32 tmp_scale = 1;
    for (int i = 0; i < 18; i++) {
        tmp_a = a % 3;
        tmp_b = b % 3;
        tmp_res = (tmp_a > tmp_b) ? tmp_b : tmp_a;
        res += (tmp_res * tmp_scale);
        tmp_scale *= 3;
        a /= 3;
        b /= 3;
    }
    return res;
}

int32 ternary_or_18(int32 a, int32 b) {
    // Ternary or is equivalent to the max of a and b
    int32 res, tmp_a, tmp_b, tmp_res;
    int32 tmp_scale = 1;
    for (int i = 0; i < 18; i++) {
        tmp_a = a % 3;
        tmp_b = b % 3;
        tmp_res = (tmp_a > tmp_b) ? tmp_a : tmp_b;
        res += (tmp_res * tmp_scale);
        tmp_scale *= 3;
        a /= 3;
        b /= 3;
    }
    return res;
}

int32 ternary_xor_18(int32 a, int32 b) {
    // Ternary xor: if a == 0, result is b, if a == 1, result is 1, if a == 2, result is inverted b
    int32 res, tmp_a, tmp_b, tmp_res;
    int32 tmp_scale = 1;
    for (int i = 0; i < 18; i++) {
        tmp_a = a % 3;
        tmp_b = b % 3;
        if (tmp_a == 2) tmp_res = tmp_b; else if (tmp_a == 0) tmp_res = 2 - tmp_b; else tmp_res = 1;
        res += (tmp_res * tmp_scale);
        tmp_scale *= 3;
        a /= 3;
        b /= 3;
    }
    return res;
}

int32 ternary_shift_18(int32 a, int32 b) {
    if (b > 0) return a * power(3, b);
    else if (b < 0) return a / power(3, b);
    return a;
}

int32 alu_op_i18(uint32 func, int32 rs1, int32 rs2, uint32 *flags, uint32 flagbank) {
    int64 res = 0;
    uint32 is = 1;
    uint32 iv = 1;
    uint8 set_flags = 1;
    switch(func) {
        case 0: // Add
            res = rs1 + rs2;
            break;
        case 1: // Subtract
            res = rs1 - rs2;
            break;
        case 2: // Shift
            res = ternary_shift_18(rs1, rs2);
            break;
        case 3: // And
            res = ternary_and_18(rs1, rs2);
            break;
        case 4: // Or
            res = ternary_or_18(rs1, rs2);
            break;
        case 5: // Xor
            res = ternary_xor_18(rs1, rs2);
            break;
        case 9: // Multiply (high 18)
            set_flags = 0;
            res = (rs1 * rs2) / MAX_I18;
            break;
        case 10: // Multiply (low 18)
            set_flags = 0;
            res = rs1 * rs2;
            break;
        case 12: // Divide
            set_flags = 0;
            res = rs1 / rs2;
            break;
        case 13: // Remainder
            set_flags = 0;
            res = rs1 % rs2;
            break;
        default:
            break;
    }

    // Handle overflow
    if (res > MAX_I18) { 
        iv = 2; 
        res = res % MAX_I18; 
    } else if (res < -MAX_I18) {
        iv = 0; 
        res = 0 - ((0 - res) % MAX_I18); 
    }

    // Handle sign and set flag bits
    if (set_flags == 1) {
        if (res > 0) is = 2; else if (res < 0) is = 0; else is = 1;
        set_cpu_flag(IS, flagbank, flags, is);
        set_cpu_flag(IV, flagbank, flags, iv);
    }

    return (uint32)res;
}

void test_op_i18(int32 *iregs, uint32 *flagregs, uint32 function, uint32 rs1, uint32 rd, uint32 size, int32 shamt, uint32 flagbank) {
    int32 a;
    int32 b;
    int32 tmp;
    int32 result;
    int32 scale_factor = power(3, shamt);
    uint32 value = flagbank; // These are the same tritfield
    if (function == 0) { // TEST
        a = get_reg_i(iregs, rs1, size);
        result = (a / scale_factor) % 3;
        set_cpu_flag(IT, flagbank, flagregs, result);
    } else if (function == 1) { // SET
        a = get_reg_i(iregs, rs1, size);
        tmp = (a / scale_factor) % 3;
        result = a - tmp + (value * scale_factor);
        set_reg_i(iregs, rd, result, size);
    } else if (function == 2) { // SHIFT
        a = get_reg_i(iregs, rs1, size);
        result = ternary_shift_18(a, shamt);
        set_reg_i(iregs, rd, result, size);
    }
}


float32 alu_op_f18(uint32 func, float32 rs1, float32 rs2, uint32 *flags, uint32 flagbank) {
    float32 res = 0.0;
    uint32 fs, fv, fe;
    switch(func) {
        case 18: // FAdd
            res = rs1 + rs2;
            break;
        case 19: // FSubtract
            res = rs1 - rs2;
            break;
        case 21: // Multiply
            res = (rs1 * rs2);
            break;
        case 22: // Divide
            if (rs2 == 0) {
                // Set result to NaN
                if (rs1 >= 0) {
                    res = INFINITY_F18;
                } else {
                    res = -INFINITY_F18;
                }
            } else {
                res = rs1 / rs2;
            }
            break;
        default:
            break;
    }

    // Handle overflow
    if (res > MAX_F18) { 
        fv = 2; 
        res = MAX_F18;
    } else if (res < -MAX_F18) {
        fv = 0; 
        res = -MAX_F18; 
    }

    // Handle sign and set flag bits
    if (res > 0) fs = 2; else if (res < 0) fs = 0; else fs = 1;
    set_cpu_flag(FS, flagbank, flags, fs);
    set_cpu_flag(FV, flagbank, flags, fv);
//    set_cpu_flag(FE, flagbank, flags, fe;) // TODO: calculate exactness

    return (uint32)res;
}

void move_regs(int32 *iregs, float32 *fregs, uint32 rd, uint32 rs1, uint32 rs2, uint32 size) {
    // Only valid for int->float, float->int for now
    switch(rs2) {
        case 1: // int->float
            fregs[rs1] = (float32)iregs[rs1];
            break;
        case 3: // float->int
            if (rd == 0) return;
            iregs[rs1] = (int32)fregs[rs1];
            break;
        default:
            return;
    }
}

int bin_to_ternchar(int32 val) {
    if (val < 0) { printf("Control characters are unimplemented!\n"); return 0;}
    if (val == 0) return 0;
    if (val < 27) return (val - 1) + 'A';
    if (val == 27) return ' ';
    if (val < 54) return (val - 28) + 'a';
    if (val < 64) return (val - 54) + '0';
    switch(val) {
        case 64: return '-'; break;
        case 65: return '+'; break;
        case 66: return '='; break;
        case 67: return '!'; break;
        case 68: return '@'; break;
        case 69: return '#'; break;
        case 70: return '$'; break;
        case 71: return '%'; break;
        case 72: return '^'; break;
        case 73: return '&'; break;
        case 74: return '*'; break;
        case 75: return '('; break;
        case 76: return ')'; break;
        case 77: return '{'; break;
        case 78: return '}'; break;
        case 79: return '['; break;
        case 80: return ']'; break;
        case 81: return '\r'; break;
        case 82: return '\n'; break;
        case 83: return '\t'; break;
        case 84: return '.'; break;
        case 85: return ','; break;
        case 86: return ':'; break;
        case 87: return ';'; break;
        case 88: return '\''; break;
        case 89: return '\"'; break;
        case 90: return '/'; break;
        case 91: return '?'; break;
        case 92: return '\\'; break;
        case 93: return '|'; break;
        case 94: return '_'; break;
        case 95: return '`'; break;
        case 96: return '~'; break;
        case 97: return '<'; break;
        case 98: return '>'; break;
        default: printf("Characters > 98 are unimplemented!\n"); return 0;
    }
}

void print_cpu_status(int32 *iregs, float32 *fregs, uint32 *iflags, uint32 *fflags, uint32 *imem, int32 *dmem) {
    for (int row = 0; row < 3; row++) {
        printf("R%02d-%02d:", row*3, row*3+9);
        for (int col = 0; col < 9; col++) {
            printf(" %10d", iregs[row*9+col]);
        }
        printf("\n");
    }
    for (int row = 0; row < 3; row++) {
        printf("  Flag bank %d:", row);
        for (int col = 0; col < 3; col++) {
            printf(" %2d", iflags[row * 3 + col]);
        }
    }
    printf("\n");
}

#endif //TERNARY_RISC_SIM_H
