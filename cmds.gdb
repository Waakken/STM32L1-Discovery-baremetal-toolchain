target remote localhost:3333
monitor reset halt
load
b main
layout asm
#display $r7
#display $r8
#display 0x20000300
#display/20 0x20000300
#break *0x800014a
#continue

#x/20 0x20000300