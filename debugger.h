#pragma once

#include "exec_format.h"
#include "vm.h"
#include <string>
#include <unordered_map>
#include <vector>

class Debugger {
public:
    int run(const std::string& initialExec = "");

private:
    VirtualMachine vm_;
    std::vector<ExecSymbol> symbols_;
    std::unordered_map<std::string, uint32_t> functions_;
    bool loaded_ = false;

    bool loadFile(const std::string& path);
    bool handleCommand(const std::string& line);

    void printHelp() const;
    void printFunctions() const;
    void printRegisters() const;
    void printState() const;
    void printInsn(uint32_t pc) const;
    void printInsnRange(uint32_t start, int count) const;

    static uint32_t parseAddr(const std::string& s);
};
