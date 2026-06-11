#include "vm.h"
#include "disasm.h"
#include "exec_format.h"
#include "isa.h"
#include <iostream>

int VirtualMachine::reg(int i) const {
    if (i < 0 || i >= isa::REG_COUNT) return 0;
    return regs_[i];
}

int VirtualMachine::readMem(int addr) const {
    int idx = addr / isa::WORD_SIZE;
    if (addr < 0 || idx < 0 || idx >= static_cast<int>(mem_.size())) return 0;
    return static_cast<int>(mem_[idx]);
}

void VirtualMachine::writeMem(int addr, int value) {
    int idx = addr / isa::WORD_SIZE;
    if (addr < 0 || idx < 0 || idx >= static_cast<int>(mem_.size())) return;
    mem_[idx] = static_cast<uint32_t>(value);
}

void VirtualMachine::push(int v) {
    sp_ -= 4;
    writeMem(sp_, v);
}

int VirtualMachine::pop() {
    int v = readMem(sp_);
    sp_ += 4;
    return v;
}

bool VirtualMachine::load(const std::string& execPath) {
    std::vector<uint32_t> code;
    std::vector<int> data;
    uint32_t entryPc = 0;

    if (!readExecFile(execPath, code, data, nullptr, &entryPc)) return false;

    mem_.assign(isa::MEM_SIZE / isa::WORD_SIZE, 0);

    int codeWords = static_cast<int>(code.size());

    for (int i = 0; i < codeWords; ++i) mem_[i] = code[i];

    dataBase_ = codeWords * isa::WORD_SIZE;

    for (size_t i = 0; i < data.size(); ++i)
        writeMem(dataBase_ + static_cast<int>(i * 4), data[i]);

    ip_ = entryPc;
    sp_ = isa::STACK_TOP;
    bp_ = isa::STACK_TOP;
    running_ = true;

    return true;
}

uint32_t VirtualMachine::fetch() {
    return mem_[ip_++];
}

uint32_t VirtualMachine::peekInsn() const {
    if (ip_ >= mem_.size()) return 0;
    return mem_[ip_];
}

int VirtualMachine::codeWords() const {
    return dataBase_ / isa::WORD_SIZE;
}

void VirtualMachine::addBreakpoint(uint32_t addr) {
    breakpoints_.insert(addr);
}

void VirtualMachine::removeBreakpoint(uint32_t addr) {
    breakpoints_.erase(addr);
}

void VirtualMachine::clearBreakpoints() {
    breakpoints_.clear();
}

bool VirtualMachine::atBreakpoint() const {
    return breakpoints_.count(ip_) > 0;
}

void VirtualMachine::traceStep(uint32_t insnPc, uint8_t op) {
    if (!trace_) return;
    std::cerr << "step ip=" << insnPc << " op=" << static_cast<int>(op)
        << "  " << disassemble(mem_[insnPc], insnPc) << "\n";
}

void VirtualMachine::step() {
    if (!running_ || ip_ >= mem_.size()) return;

    uint32_t insnPc = ip_;
    uint32_t w = fetch();

    uint8_t op = isa::opcode(w);
    uint8_t rd = isa::rd(w);
    uint8_t rs1 = isa::rs1(w);
    uint8_t rs2 = isa::rs2(w);
    int16_t imm = isa::imm12(w);

    auto R = [&](uint8_t i) -> int& { return regs_[i]; };

    switch (op) {
    case isa::MOV_REG: R(rd) = R(rs1); break;
    case isa::MOV_IMM: R(rd) = imm; break;
    case isa::LOAD_DATA: R(rd) = readMem(dataBase_ + imm); break;
    case isa::STORE_DATA: writeMem(dataBase_ + imm, R(rs2)); break;
    case isa::LOAD_LOCAL: R(rd) = readMem(bp_ + imm); break;
    case isa::STORE_LOCAL: writeMem(bp_ + imm, R(rs2)); break;
    case isa::ADD: R(rd) = R(rs1) + R(rs2); break;
    case isa::SUB: R(rd) = R(rs1) - R(rs2); break;
    case isa::MUL: R(rd) = R(rs1) * R(rs2); break;
    case isa::DIV: R(rd) = (R(rs2) == 0) ? 0 : R(rs1) / R(rs2); break;

    case isa::CMP:
        cmpEq_ = (R(rs1) == R(rs2));
        cmpLt_ = (R(rs1) < R(rs2));
        cmpGt_ = (R(rs1) > R(rs2));
        break;

    case isa::BR_EQ:
        if (cmpEq_) ip_ = static_cast<uint32_t>(static_cast<int>(ip_) + imm);
        break;

    case isa::BR_NE:
        if (!cmpEq_) ip_ = static_cast<uint32_t>(static_cast<int>(ip_) + imm);
        break;

    case isa::BR_LT:
        if (cmpLt_) ip_ = static_cast<uint32_t>(static_cast<int>(ip_) + imm);
        break;

    case isa::BR_GT:
        if (cmpGt_) ip_ = static_cast<uint32_t>(static_cast<int>(ip_) + imm);
        break;

    case isa::BR_LTE:
        if (cmpEq_ || cmpLt_) ip_ = static_cast<uint32_t>(static_cast<int>(ip_) + imm);
        break;

    case isa::BR_GTE:
        if (cmpEq_ || cmpGt_) ip_ = static_cast<uint32_t>(static_cast<int>(ip_) + imm);
        break;

    case isa::JMP:
        ip_ = static_cast<uint32_t>(static_cast<int>(ip_) + imm);
        break;

    case isa::PUSH:
        push(R(rs1));
        break;

    case isa::POP:
        R(rd) = pop();
        break;

    case isa::PUSH_BP:
        push(bp_);
        break;

    case isa::PUSH_NEXT_PC:
        push(static_cast<int>(ip_) + 1);
        break;

    case isa::CALL: {
        bp_ = sp_ + 4;
        ip_ = static_cast<uint32_t>(static_cast<int>(ip_) + imm);
        break;
    }

    case isa::RET: {
        int retVal = R(rs1);
        int frameBp = bp_;

        int retPc = readMem(frameBp - 4);

        bp_ = readMem(frameBp);

        sp_ = frameBp - 4;

        ip_ = static_cast<uint32_t>(retPc);

        R(1) = retVal;

        break;
    }

    case isa::PRINT:
        std::cout << R(rs1) << std::endl;
        break;

    case isa::INPUT: {
        int x;
        std::cin >> x;
        R(rd) = x;
        break;
    }

    case isa::EXIT:
        running_ = false;
        break;

    case isa::ALLOC_STACK:
        sp_ -= imm;
        break;

    default:
        break;
    }

    traceStep(insnPc, op);
}

bool VirtualMachine::debugStep(StepKind kind) {
    if (!running_ || ip_ >= static_cast<uint32_t>(codeWords()))
        return false;

    if (kind == StepKind::Out) {
        int targetBp = readMem(bp_);
        while (running_ && ip_ < static_cast<uint32_t>(codeWords())) {
            uint32_t insnPc = ip_;
            uint8_t op = isa::opcode(peekInsn());
            step();
            if (!running_) return false;
            if (op == isa::RET && bp_ == targetBp)
                return true;
            if (atBreakpoint())
                return true;
        }
        return running_;
    }

    if (kind == StepKind::Over) {
        uint8_t op = isa::opcode(peekInsn());
        if (op == isa::CALL) {
            uint32_t returnIp = ip_ + 1;
            step();
            if (!running_) return false;
            while (running_ && ip_ != returnIp && ip_ < static_cast<uint32_t>(codeWords())) {
                step();
                if (!running_) return false;
                if (atBreakpoint()) return true;
            }
            return running_;
        }
    }

    step();
    if (!running_) return false;
    if (atBreakpoint()) return true;
    return running_;
}

bool VirtualMachine::debugGo() {
    const int maxSteps = 500000;
    int steps = 0;

    while (running_ && ip_ < static_cast<uint32_t>(codeWords())) {
        if (++steps > maxSteps) {
            std::cerr << "VM error: step limit exceeded at ip=" << ip_ << "\n";
            running_ = false;
            return false;
        }

        if (atBreakpoint())
            return true;

        step();
    }

    return running_;
}

int VirtualMachine::run() {
    int steps = 0;
    const int maxSteps = 500000;

    while (running_ && ip_ < static_cast<uint32_t>(codeWords())) {

        if (++steps > maxSteps) {
            std::cerr << "VM error: step limit exceeded at ip=" << ip_ << "\n";
            return 1;
        }

        step();
    }

    return regs_[1];
}
