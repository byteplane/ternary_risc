.start 0
# Set rF to maxint18
LUI rF, 193709880
ADDI rF, rF, 364
#ADDI rF, r0, 364
:loop
# Decrement counter
ADDI rF, rF, -1
# If result of last operation was positive, jump back to loop
BR.P -1
# DUMP
# SYSCALLR rF, 6
# HALT
SYSCALLR rF, 1
