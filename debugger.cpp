#include "debugger.h"
#include "disasm.h"
#include "exec_format.h"
#include "isa.h"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <sstream>
#include <stdexcept>

static std::string trim(const std::string& s) {
    size_t b = s.find_first_not_of(" \t\r\n");
    if (b == std::string::npos) return "";
    size_t e = s.find_last_not_of(" \t\r\n");
    return s.substr(b, e - b + 1);
}

static std::vector<std::string> tokenize(const std::string& line) {
    std::vector<std::string> tokens;
    std::string cur;
    bool inQuote = false;

    for (size_t i = 0; i < line.size(); ++i) {
        char c = line[i];
        if (c == '"') {
            inQuote = !inQuote;
            continue;
        }
        if (!inQuote && std::isspace(static_cast<unsigned char>(c))) {
            if (!cur.empty()) {
                tokens.push_back(cur);
                cur.clear();
            }
            continue;
        }
        cur.push_back(c);
    }

    if (!cur.empty())
        tokens.push_back(cur);

    return tokens;
}

uint32_t Debugger::parseAddr(const std::string& s) {
    if (s.rfind("0x", 0) == 0 || s.rfind("0X", 0) == 0)
        return static_cast<uint32_t>(std::stoul(s, nullptr, 16));
    return static_cast<uint32_t>(std::stoul(s));
}

bool Debugger::loadFile(const std::string& path) {
    std::vector<uint32_t> code;
    std::vector<int> data;
    symbols_.clear();
    functions_.clear();

    uint32_t entryPc = 0;
    if (!readExecFile(path, code, data, &symbols_, &entryPc))
        return false;

    if (!vm_.load(path))
        return false;

    for (auto& sym : symbols_)
        functions_[sym.name] = sym.pc;

    loaded_ = true;
    std::cout << "Loaded " << path
        << "  (code=" << code.size()
        << " words, data=" << data.size()
        << " words, functions=" << symbols_.size() << ")\n";
    return true;
}

void Debugger::printHelp() const {
    std::cout <<
        "Commands (verb arg1 arg2 --flag value):\n"
        "  load -f <path>          Load executable file\n"
        "  print -mode functions   List functions and entry PCs\n"
        "  print -mode regs        Show registers\n"
        "  print -mode state       Show IP, SP, BP, running flag\n"
        "  print -mode insn        Disassemble current instruction\n"
        "  print -mode code [-n N] Disassemble N instructions from IP\n"
        "  step                    Execute one instruction (step over calls)\n"
        "  step in                 Execute one instruction (enter calls)\n"
        "  step out                Run until current function returns\n"
        "  go                      Continue until breakpoint or halt\n"
        "  br -b <addr>            Set breakpoint at address\n"
        "  br -d <addr>            Delete breakpoint\n"
        "  br                      List breakpoints\n"
        "  set ip <addr>           Set instruction pointer (e.g. 0x0)\n"
        "  trace on|off            Enable/disable instruction trace\n"
        "  help                    Show this help\n"
        "  quit                    Exit debugger\n";
}

void Debugger::printFunctions() const {
    if (symbols_.empty()) {
        std::cout << "(no symbol table)\n";
        return;
    }
    for (auto& sym : symbols_)
        std::cout << sym.name << " @ " << sym.pc << "\n";
}

void Debugger::printRegisters() const {
    for (int i = 0; i < isa::REG_COUNT; ++i)
        std::cout << "r" << i << " = " << vm_.reg(i) << "\n";
}

void Debugger::printState() const {
    std::cout << "IP = 0x" << std::hex << vm_.ip() << std::dec
        << " (" << vm_.ip() << ")\n";
    std::cout << "SP = " << vm_.sp() << "\n";
    std::cout << "BP = " << vm_.bp() << "\n";
    std::cout << "running = " << (vm_.running() ? "true" : "false") << "\n";
}

void Debugger::printInsn(uint32_t pc) const {
    if (pc >= static_cast<uint32_t>(vm_.codeWords())) {
        std::cout << pc << ": <end>\n";
        return;
    }
    std::cout << disassemble(vm_.memory()[pc], pc) << "\n";
}

void Debugger::printInsnRange(uint32_t start, int count) const {
    for (int i = 0; i < count; ++i)
        printInsn(start + static_cast<uint32_t>(i));
}

bool Debugger::handleCommand(const std::string& line) {
    std::string cmdLine = trim(line);
    if (cmdLine.empty()) return true;

    auto tokens = tokenize(cmdLine);
    if (tokens.empty()) return true;

    std::string verb = tokens[0];
    std::transform(verb.begin(), verb.end(), verb.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    if (verb == "quit" || verb == "exit")
        return false;

    if (verb == "help") {
        printHelp();
        return true;
    }

    if (verb == "load") {
        std::string path;
        for (size_t i = 1; i < tokens.size(); ++i) {
            if (tokens[i] == "-f" && i + 1 < tokens.size()) {
                path = tokens[++i];
            }
        }
        if (path.empty()) {
            std::cout << "Usage: load -f <path>\n";
            return true;
        }
        if (!loadFile(path))
            std::cout << "Failed to load " << path << "\n";
        return true;
    }

    if (!loaded_) {
        std::cout << "No program loaded. Use: load -f <path>\n";
        return true;
    }

    if (verb == "print") {
        std::string mode;
        int count = 5;

        for (size_t i = 1; i < tokens.size(); ++i) {
            if (tokens[i] == "-mode" && i + 1 < tokens.size())
                mode = tokens[++i];
            else if (tokens[i] == "-n" && i + 1 < tokens.size())
                count = std::stoi(tokens[++i]);
        }

        if (mode == "functions") printFunctions();
        else if (mode == "regs") printRegisters();
        else if (mode == "state") printState();
        else if (mode == "insn") printInsn(vm_.ip());
        else if (mode == "code") printInsnRange(vm_.ip(), count);
        else std::cout << "Usage: print -mode functions|regs|state|insn|code\n";
        return true;
    }

    if (verb == "step") {
        StepKind kind = StepKind::Over;
        if (tokens.size() >= 2) {
            std::string sub = tokens[1];
            std::transform(sub.begin(), sub.end(), sub.begin(),
                [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            if (sub == "in") kind = StepKind::In;
            else if (sub == "out") kind = StepKind::Out;
        }

        if (!vm_.running()) {
            std::cout << "Program not running.\n";
            return true;
        }

        vm_.debugStep(kind);
        if (!vm_.running())
            std::cout << "Program halted. R1 = " << vm_.reg(1) << "\n";
        else
            printInsn(vm_.ip());
        return true;
    }

    if (verb == "go") {
        if (!vm_.running()) {
            std::cout << "Program not running.\n";
            return true;
        }

        vm_.debugGo();
        if (vm_.running() && vm_.atBreakpoint()) {
            std::cout << "Breakpoint hit at IP = 0x" << std::hex << vm_.ip()
                << std::dec << " (" << vm_.ip() << ")\n";
            printInsn(vm_.ip());
        }
        else if (!vm_.running()) {
            std::cout << "Program halted. R1 = " << vm_.reg(1) << "\n";
        }
        return true;
    }

    if (verb == "br") {
        bool listOnly = true;
        for (size_t i = 1; i < tokens.size(); ++i) {
            if (tokens[i] == "-b" && i + 1 < tokens.size()) {
                vm_.addBreakpoint(parseAddr(tokens[++i]));
                listOnly = false;
            }
            else if (tokens[i] == "-d" && i + 1 < tokens.size()) {
                vm_.removeBreakpoint(parseAddr(tokens[++i]));
                listOnly = false;
            }
        }

        if (listOnly) {
            if (vm_.breakpoints().empty())
                std::cout << "(no breakpoints)\n";
            else {
                for (auto addr : vm_.breakpoints())
                    std::cout << "  " << addr << "  " << disassemble(vm_.memory()[addr], addr) << "\n";
            }
        }
        return true;
    }

    if (verb == "set" && tokens.size() >= 3 && tokens[1] == "ip") {
        uint32_t addr = parseAddr(tokens[2]);
        vm_.setIp(addr);
        std::cout << "IP = 0x" << std::hex << addr << std::dec << " (" << addr << ")\n";
        return true;
    }

    if (verb == "trace" && tokens.size() >= 2) {
        std::string onoff = tokens[1];
        std::transform(onoff.begin(), onoff.end(), onoff.begin(),
            [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        vm_.setTrace(onoff == "on");
        std::cout << "trace " << (vm_.trace() ? "on" : "off") << "\n";
        return true;
    }

    std::cout << "Unknown command: " << verb << "  (type help)\n";
    return true;
}

int Debugger::run(const std::string& initialExec) {
    std::cout << "Course VM Debugger  (type help for commands)\n";

    if (!initialExec.empty()) {
        if (!loadFile(initialExec))
            std::cout << "Failed to load " << initialExec << "\n";
    }

    std::string line;
    while (true) {
        std::cout << "(dbg) ";
        if (!std::getline(std::cin, line))
            break;
        if (!handleCommand(line))
            break;
    }

    return 0;
}
