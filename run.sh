clear
python assemble_risc18.py test1.asm test1.bin
gcc -O3 ternary_risc_sim_v2.cpp -lm -o ternary_risc_sim
./ternary_risc_sim test1.bin

