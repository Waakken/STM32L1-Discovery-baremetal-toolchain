#pragma once

#include "reg_addr.hpp"
#include "reg_defs.hpp"

REG *get_sram(void);
REG *get_flash(void);
REG get_sram_size(void);
REG get_flash_size(void);
