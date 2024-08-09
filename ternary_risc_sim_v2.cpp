#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "ternary_risc_sim.h"

#define SHOW_CPU_STATUS do { print_cpu_status(iregs, fregs, iflags, fflags, imem, dmem); } while(0)

//#define INSTR_END { instr_run++; dec_instr = *dip; }
#define INSTR_END { instr_run++; dec_instr = *dip; }
#define NEXT switch(dec_instr.opcode) { case 0: goto r_type_instr; case 1: goto t_type_instr; case 2: goto invalid_instr; case 3: goto invalid_instr; case 4: goto invalid_instr; case 5: goto invalid_instr; \
case 6: goto invalid_instr; case 7: goto invalid_instr; case 8: goto invalid_instr; case 9: goto imm1_type_instr; case 10: goto imm2_type_instr; case 11: goto invalid_instr; \
case 12: goto load_instr; case 13: goto s_type_instr; case 15: goto lui_instr; case 16: goto auipc_instr; case 17: goto invalid_instr; \
case 18: goto jal_instr; case 19: goto jmp_instr; case 20: goto syscalli_instr; case 21: goto jalr_instr; case 22: goto jr_instr; case 23: goto syscallr_instr; \
case 24: goto br_instr; case 25: goto fbr_instr; default: goto invalid_instr; }

void calc_limits() {
    float64 exponent = 40.0;
    float64 mantissa = 265720.0;
    float64 scaled_mantissa = 1.0 + (mantissa / 531441.0);
    float64 max_f18 = scaled_mantissa * pow(3.0, exponent);
    printf("exponent: %0.3f, mantissa: %0.3f, scaled_mantissa: %0.3f, max_f18: %0.20f\n", exponent, mantissa, scaled_mantissa, max_f18);
}

void init_cpu(int32 *iregs, uint32 *iflags, float32 *fregs, uint32 *fflags) {
    for (int i = 0; i < 27; i++) {
        iregs[i] = 0;
        fregs[i] = 0.0;
    }
    for (int i = 0; i < 9; i++) {
        iflags[i] = 0;
        fflags[i] = 0;
    }
}

int sim(uint32 *imem, decoded_instruction *dec_imem, int32 *dmem, int32 *cpu_retval) {
    DEBUG_PRINT("sim()\n");

    decoded_instruction *dip = dec_imem;
    uint64 instr_run = 0;

    int32 iregs[32];
    float32 fregs[32];
    uint32 iflags[9];
    uint32 fflags[9];

    decoded_instruction dec_instr;
    uint32 rd, rs1, rs2, function, size, flagbank, flagstate, flag, test_flag;
    int32 shamt, imm, inc, val;
    int32 a, b, out;
    float32 fa, fb, fout;

    init_cpu(iregs, iflags, fregs, fflags);

    dec_instr = dec_imem[0];
	NEXT;

r_type_instr:
    DEBUG_PRINT("r_type_instr\n");
    function = dec_instr.func;
    rd = dec_instr.rd;
    rs1 = dec_instr.rs1;
    rs2 = dec_instr.rs2;
    size = dec_instr.size_state;
    flagbank = dec_instr.flagbank_inc;
    DEBUG_PRINT1("instruction PC: %d\n", pc_from_dip(dip, dec_imem));
    DEBUG_PRINT6("function : %d, rd: %d, rs1: %d, rs2: %d, size: %d, flagbank: %d\n", function, rd, rs1, rs2, size, flagbank);
    if (dec_instr.func < 18) {
        a = get_reg_i(iregs, rs1, size);
        b = get_reg_i(iregs, rs2, size);
        out = alu_op_i18(function, a, b, iflags, flagbank);
        DEBUG_PRINT4("api_op_18(%d, %d, %d) -> %d\n", function, a, b, out);
        set_reg_i(iregs, rd, out, size);
    } else if (function < 27) {
        fa = get_reg_f18(fregs, rs1);
        fb = get_reg_f18(fregs, rs2);
        fout = alu_op_f18(function, rs1, rs2, fflags, flagbank);
        set_reg_f18(fregs, rd, fout);
    } else if (function == 27) { // MOV between integer and float (and other?) registers
        move_regs(iregs, fregs, rd, rs1, rs2, size);
    }
    dip++;
    INSTR_END;
	NEXT;
t_type_instr:
    DEBUG_PRINT("t_type_instr\n");
    function = dec_instr.func;
    rd = dec_instr.rd;
    rs1 = dec_instr.rs1;
    size = dec_instr.size_state;
    shamt = dec_instr.imm;
    flagbank = dec_instr.flagbank_inc;
    test_op_i18(iregs, iflags, function, rs1, rd, size, shamt, flagbank);
    dip++;
    INSTR_END;
	NEXT;
invalid_instr:
    DEBUG_PRINT("invalid_instr\n");
    exit(-1);
    INSTR_END;
	NEXT;
imm1_type_instr:
imm2_type_instr:
    DEBUG_PRINT("i_type_instr\n");
    rd = dec_instr.rd;
    rs1 = dec_instr.rs1;
    size = dec_instr.size_state;
    flagbank = dec_instr.flagbank_inc;
    function = dec_instr.func;
    imm = dec_instr.imm;
    DEBUG_PRINT1("instruction PC: %d\n", pc_from_dip(dip, dec_imem));
    DEBUG_PRINT6("function: %d, rd: %d, rs2: %d, imm: %d, flagbank: %d, size: %d\n", function, rd, rs1, imm, flagbank, size);

    a = get_reg_i(iregs, rs1, size);
    out = alu_op_i18(function, a, imm, iflags, flagbank);
    set_reg_i(iregs, rd, out, size);
    dip++;
    INSTR_END;
	NEXT;
load_instr:
    DEBUG_PRINT("l_type_instr\n");
    function = dec_instr.func;
    rd = dec_instr.rd;
    rs1 = dec_instr.rs1;
    imm = dec_instr.imm;
    inc = dec_instr.flagbank_inc;
    size = dec_instr.size_state;
    DEBUG_PRINT1("instruction PC: %d\n", pc_from_dip(dip, dec_imem));
    DEBUG_PRINT6("rd = %d, rs1 = %d, imm = %d, inc = %d, size = %d, function = %d\n", rd, rs1, imm, inc, size, function);
    if (function == 0) { // Integer load
        a = get_reg_i(iregs, rs1, 1);
        b = a + imm;
        if (inc == 1) { // Standard load
            out = get_dmem_i(dmem, b, size);
        } else if (inc == 0) { // Preincrement
            out = get_dmem_i(dmem, b, size);
            set_reg_i(iregs, rs1, b, 1);
        } else { // Postincrement
            out = get_dmem_i(dmem, a, size);
            set_reg_i(iregs, rs1, b, 1);
        }
        set_reg_i(iregs, rd, out, 1);
    } else if (function == 1) { // Floating point load
        a = get_reg_i(iregs, rs1, 1);
        b = a + imm;
        if (inc == 1) { // Standard load
            fout = get_dmem_f(dmem, b);
        } else if (inc == 0) { // Preincrement
            fout = get_dmem_f(dmem, b);
            set_reg_i(iregs, rs1, b, 1);
        } else { // Postincrement
            fout = get_dmem_f(dmem, a);
            set_reg_i(iregs, rs1, b, 1);
        }
        set_reg_f18(fregs, rd, fout);
    }
    dip++;
    INSTR_END;
	NEXT;
s_type_instr:
    DEBUG_PRINT("s_type_instr\n");
    function = dec_instr.func;
    rs1 = dec_instr.rs1;
    rs2 = dec_instr.rs2;
    imm = dec_instr.imm;
    inc = dec_instr.flagbank_inc;
    size = dec_instr.size_state;
    if (function == 0) { // Integer store
        a = get_reg_i(iregs, rs1, 1);
        b = a + imm;
        out = get_reg_i(iregs, rs2, size);
        if (inc == 1) { // Standard store
            set_dmem_i(dmem, b, out, size);
        } else if (inc == 0) { // Preincrement
            set_dmem_i(dmem, b, out, size);
            set_reg_i(iregs, rs1, b, 1);
        } else { // Postincrement
            set_dmem_i(dmem, a, out, size);
            set_reg_i(iregs, rs1, b, 1);
        }
    } else if (function == 1) { // Floating point store
        a = get_reg_i(iregs, rs1, 1);
        b = a + imm;
        fout = get_reg_f18(fregs, rs2);
        if (inc == 1) { // Standard store
            set_dmem_f(dmem, b, fout);
        } else if (inc == 0) { // Preincrement
            set_dmem_f(dmem, b, fout);
            set_reg_i(iregs, rs1, b, 1);
        } else { // Postincrement
            set_dmem_f(dmem, a, fout);
            set_reg_i(iregs, rs1, b, 1);
        }
    }
    dip++;
    INSTR_END;
	NEXT;
lui_instr:
    DEBUG_PRINT("LUI instr\n");
    rd = dec_instr.rd;
    imm = dec_instr.imm;
    set_reg_i(iregs, rd, imm, 1);
    dip++;
    INSTR_END;
	NEXT;
auipc_instr:
    DEBUG_PRINT("LUI instr\n");
    rd = dec_instr.rd;
    imm = dec_instr.imm;
    a = (pc_from_dip(dip, dec_imem) + imm - RANGE_1TRIT) % RANGE_18TRIT;
    set_reg_i(iregs, rd, a, 1);
    dip++;
    INSTR_END;
	NEXT;
jal_instr:
    DEBUG_PRINT("JAL instr\n");
    imm = dec_instr.imm;
    a = pc_from_dip(dip, dec_imem);
    set_reg_i(iregs, 1, a, 1);
    a = (a + imm - RANGE_1TRIT) % RANGE_18TRIT;
    dip = dip_from_pc(a, dec_imem);
    INSTR_END;
	NEXT;
jmp_instr:
    DEBUG_PRINT("JMP instr\n");
    imm = dec_instr.imm;
    a = pc_from_dip(dip, dec_imem) - RANGE_1TRIT; // Since we've already incremented PC we need to deincrement
    a = (a + imm - RANGE_1TRIT) % RANGE_18TRIT;
    dip = dip_from_pc(a, dec_imem);
    INSTR_END;
	NEXT;
syscalli_instr:
    DEBUG_PRINT("SYSCALLI instr\n");
    imm = dec_instr.imm / 3;
    val = dec_instr.val;
    DEBUG_PRINT2("Called with imm: %d, val %d\n", imm, val);
    switch(imm) {
        case 1: // Yield
            // TODO: Use this to handle incoming events in the simulator
            break;
        case 2: // HALT
            *cpu_retval = val;
            return(instr_run);
            break;
        case 3: // INPUT
            break;
        case 4: // ICOUNT
            break;
        case 5: // OUTPUT
            putchar(bin_to_ternchar(val));
            break;
        case 6: // OCOUNT
            break;
        case 7: // DUMP
            SHOW_CPU_STATUS;
            break;
        default:
            break;
    }
    dip++;
    INSTR_END;
	NEXT;
jalr_instr:
    DEBUG_PRINT("JALR instr\n");
    imm = dec_instr.imm;
    rs1 = dec_instr.rs1;
    a = pc_from_dip(dip, dec_imem);
    set_reg_i(iregs, 1, a, 1);
    b = get_reg_i(iregs, rs1, 1);
    b = (b + imm) % RANGE_18TRIT;
    dip = dip_from_pc(b, dec_imem);
    DEBUG_PRINT6("imm: %d, rs1: %d, a: %d, b: %d, dip: %ld, pc: %d\n", imm, rs1, a, b, (uint64)dip, pc_from_dip(dip, dec_imem));
    INSTR_END;
	NEXT;
jr_instr:
    DEBUG_PRINT("JR instr\n");
    imm = dec_instr.imm;
    rs1 = dec_instr.rs1;
    b = get_reg_i(iregs, rs1, 1);
    b = (b + imm) % RANGE_18TRIT;
    dip = dip_from_pc(b, dec_imem);
    INSTR_END;
	NEXT;
syscallr_instr:
    DEBUG_PRINT("SYSCALLR instr\n");
    rs1 = dec_instr.rs1;
    imm = dec_instr.imm / 3;
    a = get_reg_i(iregs, rs1, 1);
    DEBUG_PRINT3("rs1: %d, imm: %d, a: %d\n", rs1, imm, a);
    switch(imm) {
        case 1: // Yield
            // TODO: Use this to process incoming events in the simulator
            break;
        case 2: // HALT
            printf("\nHALT\n");
            *cpu_retval = a;
            return(instr_run);
            break;
        case 3: // INPUT
            break;
        case 4: // ICOUNT
            break;
        case 5: // OUTPUT
            putchar(bin_to_ternchar(a));
            break;
        case 6: // OCOUNT
            break;
        case 7: // DUMP
            SHOW_CPU_STATUS;
            break;
        default:
            break;
    }
    dip++;
    INSTR_END;
	NEXT;
br_instr:
    DEBUG_PRINT("BR instr\n");
    imm = dec_instr.imm;
    flag = dec_instr.val;
    flagbank = dec_instr.flagbank_inc;
    flagstate = dec_instr.size_state;
    test_flag = get_cpu_flag(flag, flagbank, iflags);
    DEBUG_PRINT1("instruction PC: %d\n", pc_from_dip(dip, dec_imem));
    DEBUG_PRINT5("imm: %d, flag: %d, flagbank: %d, flagstate: %d, test_flag: %d\n", imm, flag, flagbank, flagstate, test_flag);
    if (test_flag == flagstate) { // Branch is taken
        a = pc_from_dip(dip, dec_imem);
        a = (a + imm) % RANGE_18TRIT;
        dip = dip_from_pc(a, dec_imem);
    } else {
        dip++;
    }
    INSTR_END;
	NEXT;
fbr_instr:
    DEBUG_PRINT("FBR instr\n");
    imm = dec_instr.imm;
    flag = dec_instr.val;
    flagbank = dec_instr.flagbank_inc;
    flagstate = dec_instr.size_state;
    test_flag = get_cpu_flag(flag, flagbank, fflags);
    if (test_flag == flagstate) { // Branch is taken
        a = pc_from_dip(dip, dec_imem);
        a = (a + imm - RANGE_1TRIT) % RANGE_18TRIT;
        dip = dip_from_pc(a, dec_imem);
    } else {
        dip++;
    }
    INSTR_END;
	NEXT;
}

uint32 parse_ternary_string(char* tern) {
    uint32 val = 0;
    for (int i = 0; i < 18; i++) {
        val = val * 3;
        if (tern[i+1] == '1') val += 1;
        if (tern[i+1] == '2') val += 2;
    }
    return val;
}

decoded_instruction parse_ternary_instruction(uint32 instr) {
    decoded_instruction dec_instr;
    uint32 opcode = extract_opcode(instr);
    dec_instr.opcode = opcode;
    switch(opcode) {
        case 0: // Type-R
            dec_instr.rd = extract_rd(instr);
            dec_instr.func = extract_r_func(instr);
            dec_instr.rs1 = extract_rs1(instr);
            dec_instr.rs2 = extract_rs2(instr);
            dec_instr.flagbank_inc = extract_r_flagbank(instr);
            dec_instr.size_state = extract_regsize(instr);
            break;
        case 1: // Type-T
            dec_instr.rd = extract_rd(instr);
            dec_instr.func = extract_t_func(instr);
            dec_instr.rs1 = extract_rs1(instr);
            dec_instr.flagbank_inc = extract_r_flagbank(instr);
            dec_instr.size_state = extract_regsize(instr);
            dec_instr.imm = extract_shift_amount(instr);
            dec_instr.val = extract_shift_value(instr);
            break;
        case 9: // Type-I
        case 10: // Type-I
        case 11: // Type-I
            dec_instr.rd = extract_rd(instr);
            dec_instr.func = extract_i_func(instr);
            dec_instr.rs1 = extract_rs1(instr);
            dec_instr.imm = extract_i_imm(instr);
            dec_instr.flagbank_inc = extract_r_flagbank(instr);
            dec_instr.size_state = extract_regsize(instr);
            break;
        case 12: // Loads
            dec_instr.rd = extract_rd(instr);
            dec_instr.func = extract_load_func(instr);
            dec_instr.rs1 = extract_rs1(instr);
            dec_instr.imm = extract_i_imm(instr);
            dec_instr.flagbank_inc = extract_r_flagbank(instr);
            dec_instr.size_state = extract_regsize(instr);
            break;
        case 13: // Type-S
            dec_instr.func = extract_load_func(instr);
            dec_instr.rs1 = extract_rs1(instr);
            dec_instr.rs2 = extract_rs2(instr);
            dec_instr.imm = extract_s_imm(instr);
            dec_instr.flagbank_inc = extract_r_flagbank(instr);
            dec_instr.size_state = extract_regsize(instr);
            break;
        case 15: // LUI
        case 16: // AUIPC
            dec_instr.rd = extract_rd(instr);
            dec_instr.imm = extract_u_imm(instr);
            break;
        case 18: // JAL
        case 19: // JMP
            dec_instr.imm = extract_j_imm(instr);
            break;
        case 20: // SYSCALLI
            dec_instr.imm = extract_syscalli_imm(instr);
            dec_instr.val = extract_syscalli_val(instr);
            break;
        case 21: // JALR
        case 22: // JMPR
        case 23: // SYSCALLR
            dec_instr.rs1 = extract_rs1(instr);
            dec_instr.imm = extract_l_imm(instr);
            break;
        case 24: // BR
        case 25: // FBR
            dec_instr.val = extract_b_flag(instr);
            dec_instr.flagbank_inc = extract_b_flagbank(instr);
            dec_instr.size_state = extract_b_state(instr);
            dec_instr.imm = extract_b_imm(instr);
            break;
        default:
            break;
    }
    return(dec_instr);
}

int load_mem_file(char *filename, uint32 *imem, decoded_instruction *dec_imem) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error opening imem file %s\n", filename);
        exit(-1);
    }

    int imem_size = 0;

    size_t max_line_size = 128; // Leaving room for long directives
    char *line = (char*)malloc(sizeof(char) * max_line_size);
    int line_length;
    int line_idx = 0;
    while ((line_length = getline(&line, &max_line_size, file)) != -1) {
        // Process one line of file
        line_idx += 1;
        if (line[0] == '.') {
            // Handle directive
            continue;
        }

        uint32 imem_val = parse_ternary_string(line);
        decoded_instruction dec_imem_val = parse_ternary_instruction(imem_val);

        dec_imem[imem_size] = dec_imem_val;
        imem[imem_size] = imem_val;
        imem_size += 1;

        printf("%d <- %s", imem_val, line);

    }

    free(line);
    fclose(file);

    return imem_size;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: ternary_risc_sim <input_filename>\n");
        exit(-1);
    }

    char* input_filename = argv[1];

    clock_t start_time = clock();

    uint32 *imem = (uint32*)calloc(IMEM_SIZE_WORDS, sizeof(uint32));
    int32 *dmem = (int32*)calloc(DMEM_SIZE_WORDS, sizeof(int32));

    decoded_instruction *dec_imem = (decoded_instruction*)calloc(IMEM_SIZE_WORDS, sizeof(decoded_instruction));

    int n_instr = load_mem_file(input_filename, imem, dec_imem);
    printf("%d instruction words loaded from file %s\n", n_instr, input_filename);

    int32 cpu_retval;
    uint64 instr_run = sim(imem, dec_imem, dmem, &cpu_retval);
    printf("CPU returned value: %d\n", cpu_retval);

    clock_t end_time = clock();
    float64 elapsed_seconds = ((float64)end_time - (float64)start_time) / CLOCKS_PER_SEC;
    float64 ns_per_instr = 100000000000.0 * elapsed_seconds / (float64)instr_run;
    float64 sim_mhz = ((float64)instr_run / elapsed_seconds) / 1000000.0;
    printf("%ld instructions executed in: %0.3f msec, %0.3f ns / instruction, simulation rate: %0.3f MHz\n", 
        instr_run, elapsed_seconds * 1000.0, ns_per_instr, sim_mhz);


    return 0;
}
