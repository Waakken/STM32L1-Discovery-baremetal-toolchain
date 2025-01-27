## USER DEFINED VARIABLES ##
PROJ_NAME = main
SRC = $(wildcard *.cpp)
HEADERS = $(wildcard *.hpp)

## COMPILER VARIABLES ##
CC_PREFIX	= arm-none-eabi-
CC		= $(CC_PREFIX)g
CXX		= $(CC_PREFIX)g++
OBJCOPY		= $(CC_PREFIX)objcopy
STARTUP		= startup_stm32l1xx_md.s
#ARM_CFLAGS	= -mthumb -mcpu=cortex-m3 -mfix-cortex-m3-ldrd -msoft-float -O -g
#ARM_CFLAGS	= -mthumb -mcpu=cortex-m3 -Wunused -Werror  -O2
CFLAGS    	= -Wunused -Werror -O2 -g -Wall -fno-inline-small-functions -fno-threadsafe-statics -fno-use-cxa-atexit
#CFLAGS    	= -Wunused -Werror -O1 -g
#CFLAGS    	= -Wunused -Werror -O0 -g
ARM_CFLAGS    	= -mthumb -mcpu=cortex-m3 $(CFLAGS) -fno-exceptions

## OPENOCD VARIABLES  ##
OOCD_BOARD = stm32ldiscovery.cfg

## TARGETS ##
# List of all binaries to build
all: program

program: $(PROJ_NAME).hex

# Create a raw binary file from the ELF version
%.hex: %.elf
	$(Q)$(OBJCOPY) -Oihex $^ $@

# Create the ELF version by mixing together the startup file,
# application, and linker file
%.elf: $(STARTUP) $(SRC) $(HEADERS)
	$(CXX) -o $@ $(ARM_CFLAGS) -nostartfiles -nostdlib -Wl,-Tstm32.ld -Wl,--wrap=atexit $(STARTUP) $(SRC)

x86: $(SRC)
	g++ -o main_$@ $(CFLAGS) $^

install:
	pkill openocd || true
	openocd -f board/$(OOCD_BOARD) \
					-c "init" -c "targets" -c "halt" \
					-c "flash write_image erase $(PROJ_NAME).hex" \
					-c "verify_image $(PROJ_NAME).hex" \
					-c "reset run" \
					-c "shutdown"
	openocd -f board/stm32ldiscovery.cfg

clean:
	@rm -f *.elf
	@rm -f *.hex

debug:
	gdb main.hex

format:
	clang-format --style="{ BasedOnStyle: LLVM, IndentWidth: 4, BreakBeforeBraces: Linux, ColumnLimit: 100, AccessModifierOffset: -4, BreakConstructorInitializers: BeforeComma}" -i $(SRC) $(HEADERS)

objdump: main.elf
	arm-none-eabi-objdump -d main.elf | less

.PHONY: all program
