#include "disasm.h"
#include <sstream>

std::string opcodeName(uint8_t op) {
    switch (op) {
    case isa::NOP: return "nop";
    case isa::MOV_REG: return "mov";
    case isa::MOV_IMM: return "mov";
    case isa::LOAD_DATA: return "load";
    case isa::STORE_DATA: return "store";
    case isa::LOAD_LOCAL: return "load";
    case isa::STORE_LOCAL: return "store";
    case isa::ADD: return "add";
    case isa::SUB: return "sub";
    case isa::MUL: return "mul";
    case isa::DIV: return "div";
    case isa::CMP: return "cmp";
    case isa::BR_EQ: return "br.eq";
    case isa::BR_NE: return "br.ne";
    case isa::BR_LT: return "br.lt";
    case isa::BR_GT: return "br.gt";
    case isa::BR_LTE: return "br.lte";
    case isa::BR_GTE: return "br.gte";
    case isa::JMP: return "jmp";
    case isa::PUSH: return "push";
    case isa::POP: return "pop";
    case isa::CALL: return "call";
    case isa::RET: return "ret";
    case isa::PRINT: return "print";
    case isa::INPUT: return "input";
    case isa::EXIT: return "exit";
    case isa::ALLOC_STACK: return "alloc";
    case isa::PUSH_BP: return "push.bp";
    case isa::PUSH_NEXT_PC: return "push.ret";
    default: return "???";
    }
}

static std::string regName(uint8_t r) {
    return "r" + std::to_string(r);
}

std::string disassemble(uint32_t word, uint32_t pc) {
    uint8_t op = isa::opcode(word);
    uint8_t rd = isa::rd(word);
    uint8_t rs1 = isa::rs1(word);
    uint8_t rs2 = isa::rs2(word);
    int16_t imm = isa::imm12(word);

    std::ostringstream os;
    os << pc << ": 0x" << std::hex << word << std::dec << "  " << opcodeName(op);

    switch (op) {
    case isa::MOV_REG:
        os << " " << regName(rd) << ", " << regName(rs1);
        break;
    case isa::MOV_IMM:
        os << " " << regName(rd) << ", #" << imm;
        break;
    case isa::LOAD_DATA:
    case isa::STORE_DATA:
        os << " " << regName(rd);
        if (op == isa::STORE_DATA) os << ", " << regName(rs2);
        os << ", [data+" << imm << "]";
        break;
    case isa::LOAD_LOCAL:
    case isa::STORE_LOCAL:
        os << " " << regName(rd);
        if (op == isa::STORE_LOCAL) os << ", " << regName(rs2);
        os << ", [bp+" << imm << "]";
        break;
    case isa::ADD:
    case isa::SUB:
    case isa::MUL:
    case isa::DIV:
    case isa::CMP:
        os << " " << regName(rd) << ", " << regName(rs1) << ", " << regName(rs2);
        break;
    case isa::BR_EQ:
    case isa::BR_NE:
    case isa::BR_LT:
    case isa::BR_GT:
    case isa::BR_LTE:
    case isa::BR_GTE:
    case isa::JMP:
    case isa::CALL:
        os << " " << (static_cast<int>(pc) + 1 + imm);
        break;
    case isa::PUSH:
    case isa::POP:
    case isa::RET:
    case isa::PRINT:
        os << " " << regName(op == isa::PUSH ? rs1 : rd);
        break;
    case isa::INPUT:
        os << " " << regName(rd);
        break;
    case isa::ALLOC_STACK:
        os << " " << imm;
        break;
    case isa::PUSH_BP:
    case isa::PUSH_NEXT_PC:
    case isa::EXIT:
    case isa::NOP:
        break;
    default:
        break;
    }

    return os.str();
}
