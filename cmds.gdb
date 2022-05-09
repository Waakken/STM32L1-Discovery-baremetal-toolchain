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
#display/20 0x40002400
#display/20 *0x40002800

#break *0x8000190

# strlen
break *0x80001d4

# main
#break *0x8000302

# init_lcd
#break *0x80001a0

# lcd loop
#break *0x80001be

#continue

# Read
#x/20 0x20000300
#x/20x 0x40000000
#x/20x 0x40023800

# RTC WPR:
#x 0x40002824
# RCC CSR:
display/x *0x40023834
# PWR CR:
display/x *0x40007000
# RCC APB1ENR:
display/x *0x40023824
# LCD CR:
display/x *0x40002400
# LCD FCR:
display/x *0x40002404
# LCD SR:
display/x *0x40002408

# Write:
#set {int}0x40000010 = 0
