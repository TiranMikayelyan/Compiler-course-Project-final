#pragma once
#include "isa.h"
#include <cstdint>
#include <set>
#include <string>
#include <vector>

enum class StepKind {
    In,
    Over,
    Out
};

class VirtualMachine {
public:
    bool load(const std::string& execPath);
    int run();

    void setTrace(bool on) { trace_ = on; }
    bool trace() const { return trace_; }

    int readMem(int addr) const;
    void writeMem(int addr, int value);
    void push(int v);
    int pop();
    uint32_t fetch();
    void step();

    uint32_t ip() const { return ip_; }
    void setIp(uint32_t ip) { ip_ = ip; }

    int sp() const { return sp_; }
    int bp() const { return bp_; }
    int dataBase() const { return dataBase_; }
    bool running() const { return running_; }

    int reg(int i) const;
    const std::vector<uint32_t>& memory() const { return mem_; }

    void addBreakpoint(uint32_t addr);
    void removeBreakpoint(uint32_t addr);
    void clearBreakpoints();
    const std::set<uint32_t>& breakpoints() const { return breakpoints_; }
    bool atBreakpoint() const;

    bool debugStep(StepKind kind);
    bool debugGo();

    uint32_t peekInsn() const;
    int codeWords() const;

private:
    std::vector<uint32_t> mem_;
    int regs_[16]{};
    uint32_t ip_ = 0;
    int sp_ = isa::STACK_TOP;
    int bp_ = isa::STACK_TOP;
    int dataBase_ = 0;
    bool cmpEq_ = false, cmpLt_ = false, cmpGt_ = false;
    bool running_ = false;
    bool trace_ = false;

    std::set<uint32_t> breakpoints_;

    void traceStep(uint32_t insnPc, uint8_t op);
};
