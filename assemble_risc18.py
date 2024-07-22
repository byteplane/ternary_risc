#assemble_risc18.ph
import os, sys, struct

DEBUG = 1

regtable = {
    'r0': 0,
    'rA': 1,
    'rB': 2,
    'rC': 3,
    'rD': 4,
    'rE': 5,
    'rF': 6,
    'rG': 7,
    'rH': 8,
    'rI': 9,
    'rJ': 10,
    'rK': 11,
    'rL': 12,
    'rM': 13,
    'rN': 14,
    'rO': 15,
    'rP': 16,
    'rQ': 17,
    'rR': 18,
    'rS': 19,
    'rT': 20,
    'rU': 21,
    'rV': 22,
    'rW': 23,
    'rX': 24,
    'rY': 25,
    'rZ': 26
}

br_table = {
    'LT': (0, 0),
    'N':  (0, 0),
    'NS':  (0, 0),
    'EQ': (0, 1),
    'Z':  (0, 1),
    'ZS':  (0, 1),
    'GT': (0, 2),
    'P':  (0, 2),
    'PS':  (0, 2),
    'NO': (1, 0),
    'ZO': (1, 1),
    'PO': (1, 2),
    'NT': (2, 0),
    'ZT': (2, 1),
    'PT': (2, 2),
    'NR': (2, 0),
    'ZR': (2, 1),
    'PR': (2, 2)
}

def trit_offset(val, places):
    return val * pow(3, places)

def encode_rd(rd):
    if DEBUG: print(f'rd: {regtable[rd]}')
    return trit_offset(regtable[rd], 3)

def encode_rs1(rs1):
    if DEBUG: print(f'rs1: {regtable[rs1]}')
    return trit_offset(regtable[rs1], 8)

def encode_rs2(rs2):
    if DEBUG: print(f'rs2: {regtable[rs2]}')
    return trit_offset(regtable[rs2], 11)

def encode_func4(func):
    if DEBUG: print(f'func4: {func}')
    return trit_offset(func % 9, 6) + trit_offset(func // 9, 14)

def encode_shamt(shamt):
    if DEBUG: print(f'shamt: {int(shamt)}')
    return trit_offset(int(shamt), 11)

def encode_i_imm(immstr):
    imm = int(immstr) + 364 # 6-bit bias
    if DEBUG: print(f'i imm: {imm}')
    return trit_offset(imm % 3, 7) + trit_offset(imm // 3, 11)

def encode_s_imm(immstr):
    imm = int(immstr) + 364 # 6-bit bias
    if DEBUG: print(f's imm: {imm}')
    return trit_offset(imm % 3, 7) + trit_offset((imm // 3) % 27, 3) + trit_offset(imm // 81, 14)

def encode_u_imm(immstr):
    imm = int(immstr) + 193710244 # 18-bit bias
    if DEBUG: print(f'u imm: {imm}')
    return trit_offset(imm // 729, 6)

def encode_j_imm(immstr):
    imm = int(immstr)
    imm += 21523360 # 16-bit bias
    if DEBUG: print(f'j imm: {int(imm)}')
    return trit_offset((imm // 3) % 177147, 11) + trit_offset(imm // 531441, 3)

def encode_syscalli_val(valstr):
    val = int(valstr)
    val += 364 # 6-bit bias
    if DEBUG: print(f'syscalli val: {val}')
    return trit_offset(val % 9, 16) + trit_offset(val // 9, 3)

def encode_syscalli_imm(immstr):
    imm = int(immstr)
    imm *= 3 # For convenience, SYSCALL<I/R> instructions can be indexed by 1, but are still encoded as multiples of 3
    imm += 21523360 # 16-bit bias
    if DEBUG: print(f'syscalli imm: {imm}')
    return trit_offset(imm // 3, 7)

def encode_l_imm(immstr, is_syscall):
    imm = int(immstr)
    if is_syscall:
        imm *= 3 # For convenience, SYSCALL<I/R> instructions can be indexed by 1, but are still encoded as multiples of 3
    imm += 797161 # 13-bit bias
    if DEBUG: print(f'l imm: {int(imm)}')
    return trit_offset((imm // 3) % 2187, 11) + trit_offset(imm // 6561, 3)

def encode_condition(branch):
    flag = br_table[branch][0]
    value = br_table[branch][1]
    if DEBUG: print(f'flag: {flag}, value: {value}')
    return trit_offset(flag, 4) + trit_offset(value, 6)

def encode_b_imm(immstr):
    imm = int(immstr) + 797161 # 13-bit bias
    if DEBUG: print(f'b imm: {int(imm)}')
    return trit_offset((imm // 3) % 177147, 7) + trit_offset(imm // 531441, 3)

def encode_size(size):
    return trit_offset(size, 17)

def assemble_rtype(tokens):
    opname = tokens[0]
    opcode = optable[opname][1]
    func = encode_func4(optable[opname][2])
    rd = encode_rd(tokens[1])
    rs1 = encode_rs1(tokens[2])
    rs2 = encode_rs2(tokens[3])
    flagbank = 0
    size = encode_size(1)
    if len(tokens) > 4:
        flagbank = int(tokens[4])
    flagbank = trit_offset(flagbank, 16)
    return opcode + func + rd + rs1 + rs2 + flagbank + size

def assemble_ttype(tokens):
    opname = tokens[0]
    opcode = optable[opname][1]
    rd = encode_rd(tokens[1])
    rs1 = encode_rs1(tokens[2])
    shamt = encode_shamt(tokens[3])
    func = trit_offset(optable[opname][2], 6)
    flag_or_value = 0 # default value if not specified in instruction
    if len(tokens > 4):
        flag_or_value = int(tokens[4])
    flag_or_value = trit_offset(flag_or_value, 16)
    size = encode_size(1)
    return opcode + func + rd + rs1 + shamt + flag_or_value + size

def assemble_itype(tokens):
    opname = tokens[0]
    opcode = optable[opname][1]
    func = optable[opname][2]
    rd = encode_rd(tokens[1])
    rs1 = encode_rs1(tokens[2])
    imm = encode_i_imm(tokens[3])
    flag_or_inc = 0
    if len(tokens) > 4:
        if tokens[4] == 'PRE': flag_or_inc = -1
        elif tokens[4] == 'POST': flag_or_inc = 1
        else: flag_or_inc = int(tokens[4])
    flag_or_inc = trit_offset(flag_or_inc, 16)
    size = encode_size(1)
    return opcode + func + rd + rs1 + imm + flag_or_inc + size

def assemble_stype(tokens):
    opname = tokens[0]
    opcode = optable[opname][1]
    func = optable[opname][2]
    rs1 = encode_rs1(tokens[1])
    rs2 = encode_rs2(tokens[2])
    imm = encode_s_imm(tokens[3])
    inc = 0
    if len(tokens) > 4:
        if tokens[4] == 'PRE': inc = -1
        if tokens[4] == 'POST': inc = 1
    inc = trit_offset(inc, 16)
    size = encode_size(1)
    return opcode + func + rs1 + rs2 + imm + inc + size

def assemble_utype(tokens):
    opname = tokens[0]
    opcode = optable[opname][1]
    rd = encode_rd(tokens[1])
    imm = encode_u_imm(tokens[2])
    return opcode + rd + imm

def assemble_jtype(tokens):
    opname = tokens[0]
    opcode = optable[opname][1]
    if opname == 'SYSCALLI':
        val = encode_syscalli_val(tokens[1])
        imm = encode_syscalli_imm(tokens[2])
        return opcode + imm + val
    else:
        imm = encode_j_imm(tokens[1])
        return opcode + imm

def assemble_ltype(tokens):
    opname = tokens[0]
    opcode = optable[opname][1]
    rs1 = encode_rs1(tokens[1])
    imm = encode_l_imm(tokens[2], opname=='SYSCALLR')
    return opcode + rs1 + imm

def assemble_btype(tokens):
    opname = tokens[0]
    opcode = optable[opname][1]
    cond = encode_condition(tokens[1])
    imm = encode_b_imm(tokens[2])
    flagbank = 0
    if len(tokens) > 3:
        flagbank = int(tokens[3])
    flagbank = trit_offset(flagbank, 5)
    return opcode + cond + imm + flagbank

optable = {
    'ADD':      (assemble_rtype,    0,  0),
    'SUB':      (assemble_rtype,    0,  1),
    'SHIFT':    (assemble_rtype,    0,  2),
    'AND':      (assemble_rtype,    0,  3),
    'OR':       (assemble_rtype,    0,  4),
    'XOR':      (assemble_rtype,    0,  5),
    'MULH':     (assemble_rtype,    0,  9),
    'MUL':      (assemble_rtype,    0,  10),
    'DIV':      (assemble_rtype,    0,  12),
    'REM':      (assemble_rtype,    0,  13),
    'FADD':     (assemble_rtype,    0,  18),
    'FSUB':     (assemble_rtype,    0,  19),
    'FMUL':     (assemble_rtype,    0,  21),
    'FDIV':     (assemble_rtype,    0,  22),
    'MOV':      (assemble_rtype,    0,  27),
    'TEST':     (assemble_ttype,    1,  0),
    'SET':      (assemble_ttype,    1,  1),
    'SHIFT':    (assemble_ttype,    1,  2),
    'ADDI':     (assemble_itype,    9,  0),
    'SHIFTI':   (assemble_itype,    9,  2),
    'ANDI':     (assemble_itype,    10, 0),
    'ORI':      (assemble_itype,    10, 1),
    'XORI':     (assemble_itype,    10, 2),
    'LOAD':     (assemble_itype,    12, 0),
    'FLOAD':    (assemble_itype,    12, 1),
    'STORE':    (assemble_stype,    13, 0),
    'FSTORE':   (assemble_stype,    13, 1),
    'LUI':      (assemble_utype,    15),
    'AUIPC':    (assemble_utype,    16),
    'JAL':      (assemble_jtype,    18),
    'JMP':      (assemble_jtype,    19),
    'SYSCALLI': (assemble_jtype,    20),
    'JALR':     (assemble_ltype,    21),
    'JMPR':     (assemble_ltype,    22),
    'SYSCALLR': (assemble_ltype,    23),
    'BR':       (assemble_btype,    24),
    'FBR':      (assemble_btype,    25)
}

def bin_to_tern(bin, separator=''):
    is_negative = bin < 0
    if is_negative: bin = -bin
    tern = ''
    for i in range(18):
        if i in (6, 12): tern = separator + tern
        trit = bin % 3
        tern = str(trit) + tern
        bin = bin // 3

    if is_negative:
        tern = '-' + tern
    else:
        tern = '+' + tern
    return tern

def asm_to_bin(asm_text, bin_file):
    for line_idx, line in enumerate(asm_text):
        if line.startswith('.'):
            # Handle directive
            continue
        if line.startswith(':'):
            # Handle label
            continue
        if line.startswith('#'):
            # Comment line, skip
            continue
        line = line.replace(',', ' ')
        line = line.replace('.', ' ')
        parts = line.split()
        if not parts[0] in optable:
            print(f'Error: unknown token {parts[0]} found in line {line_idx}')
            sys.exit()

        op_val = optable[parts[0]][0](parts)
        op_tern = bin_to_tern(op_val)

        print(f'Line {line_idx}: {" ".join(parts)} -> {bin_to_tern(op_val, ":")}')

        bin_file.write(op_tern + '\n')

def main(argv):
    if len(argv) < 3:
        print('Usage: assemble_risc18.py <input_asm_filename> <output_bin_filename>')
        return

    asm_filename = argv[1]
    bin_filename = argv[2]

    print(f'Assembling ASM file {asm_filename} to binary file {bin_filename}')

    with open(asm_filename) as asm_file:
        asm_text = asm_file.readlines()

    if os.path.exists(bin_filename):
        os.remove(bin_filename)
    with open(bin_filename, 'w') as bin_file:
        asm_to_bin(asm_text, bin_file)



if __name__ == '__main__':
    main(sys.argv)
