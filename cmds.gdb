target remote localhost:3333
monitor reset halt
load
b main
layout asm

display/x $r0
display/x $r1
display/x $r2
display/x $r3
display/x $r4
display/x $r7
#display $r8
#display 0x20000300
#display/20 0x20000300
#break *0x8000190
break *0x8000212
#continue

#x/20 0x20000300
#x/20x 0x40000000
#set {int}0x40000010 = 0
