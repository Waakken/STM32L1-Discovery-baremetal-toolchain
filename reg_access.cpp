#ifdef __x86_64
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#endif

#include "reg_defs.hpp"

#define SRAM 0x20000000
#define SRAM_SIZE 0x7FFF
#define FLASH 0x08000000
#define FLASH_SIZE 0x3FFFF

REG *sram = (REG *)SRAM;
REG *flash = (REG *)FLASH;

REG *get_sram(void) { return sram; }
REG *get_flash(void) { return flash; }
REG get_sram_size(void) { return SRAM_SIZE; }
REG get_flash_size(void) { return FLASH_SIZE; }
