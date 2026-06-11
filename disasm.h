#pragma once

#include "isa.h"
#include <cstdint>
#include <string>

std::string opcodeName(uint8_t op);
std::string disassemble(uint32_t word, uint32_t pc);
