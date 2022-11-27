target remote localhost:3333
monitor reset halt
load
symbol-file main.elf
#break main
break demo_dma
#break Dma::transfer_data
#break Lcd::write_int_to_ram_buf(int)
#break Lcd::write_string_to_ram_buf
#break Lcd::display_digit_in_location
#break Lcd::display_pixel
layout asm
continue

display/x $r0
display/x $r1
display/x $r2
display/x $r3
display/x $r4
display/x $r5
display/x $r6
display/x $r7
display/x $r8

#display 0x20000300
#display/20 0x20000300
#display/20 0x40002400
#display/20 *0x40002800

#break *0x8000190

# strlen
#break *0x80001d4

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
#display/x *0x40023834
# PWR CR:
#display/x *0x40007000
# RCC APB1ENR:
#display/x *0x40023824
# LCD CR:
#display/x *0x40002400
# LCD FCR:
#display/x *0x40002404
# LCD SR:
#display/x *0x40002408
# TIM2 CNT:
#display/x *0x40000024
# Write:
#set {int}0x40000010 = 0

# DMA1 interrupt status register
display/x *0x40026000

# DMA1 interrupt flag clear register
display/x *0x40026004

display/x *0x40026008
display/x *0x4002600C
display/x *0x40026010
display/x *0x40026014

# SRAM SRC and DEST
display/x *0x20001000
display/x *0x20002000
