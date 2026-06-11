#include "exec_format.h" 
#include "isa.h" 
#include <fstream> // file-eri het ashxatanqi hamar
#include <stdexcept> // exception-neri hamar

#pragma pack(push, 1) // memory alignment-y 1 byte a sarqum

struct FileHeader {

    uint32_t magic; // file-i signature
    uint32_t headerSize; // header-i chapy
    uint32_t sectionCount; // section-neri qanaky
    uint32_t entryPc; // cragri mtaci hasce
};

struct PackedSectionHeader {

    uint32_t type; // section-i tesaky
    uint32_t size; // section-i chapy
    uint32_t offset; // section-i texy file-i mej
};

#pragma pack(pop) // veradardznuma normal alignment

bool writeExecFile(const std::string& path, const CompiledProgram& prog) {

    // 3 section-i hamar memory enq pahum
    std::vector<PackedSectionHeader> sections(3);

    // file-i header-i chapy
    uint32_t hdrSize =
        static_cast<uint32_t>(
            sizeof(FileHeader) +
            3 * sizeof(PackedSectionHeader)
        );

    uint32_t off = hdrSize;

    // code section
    sections[0].type =
        static_cast<uint32_t>(SectionType::Code);

    sections[0].size =
        static_cast<uint32_t>(
            prog.code.size() * isa::WORD_SIZE
            );

    sections[0].offset = off;

    off += sections[0].size;

    // data section
    sections[1].type =
        static_cast<uint32_t>(SectionType::Data);

    sections[1].size =
        static_cast<uint32_t>(
            prog.data.size() * isa::WORD_SIZE
            );

    sections[1].offset = off;

    off += sections[1].size;

    // symbol table section
    sections[2].type =
        static_cast<uint32_t>(SectionType::SymbolTable);

    uint32_t symSize = 0;
    for (auto& kv : prog.functions)
        symSize += static_cast<uint32_t>(4 + kv.first.size() + 4);

    sections[2].size = symSize;

    sections[2].offset = off;

    // binary file bacuma grelu hamar
    std::ofstream out(path, std::ios::binary);

    if (!out)
        return false;

    // file-i glxavor header
    FileHeader fh{
        isa::EXEC_MAGIC,
        hdrSize,
        3,
        prog.entryPc
    };

    // gruma header-y
    out.write(
        reinterpret_cast<const char*>(&fh),
        sizeof(fh)
    );

    // gruma section-neri header-ner
    out.write(
        reinterpret_cast<const char*>(sections.data()),
        sections.size() * sizeof(PackedSectionHeader)
    );

    // code section
    for (auto w : prog.code)

        out.write(
            reinterpret_cast<const char*>(&w),
            sizeof(w)
        );

    // data section
    for (auto d : prog.data)

        out.write(
            reinterpret_cast<const char*>(&d),
            sizeof(d)
        );

    // symbol table: nameLen + name + pc
    for (auto& kv : prog.functions) {
        uint32_t nameLen = static_cast<uint32_t>(kv.first.size());
        uint32_t pc = kv.second;

        out.write(reinterpret_cast<const char*>(&nameLen), 4);
        out.write(kv.first.data(), nameLen);
        out.write(reinterpret_cast<const char*>(&pc), 4);
    }

    return true;
}

bool readExecFile(
    const std::string& path,
    std::vector<uint32_t>& code,
    std::vector<int>& data,
    std::vector<ExecSymbol>* symbols,
    uint32_t* entryPc
) {

    // binary file bacuma kardalu hamar
    std::ifstream in(path, std::ios::binary);

    if (!in)
        return false;

    FileHeader fh{};

    // karduma file-i header-y
    in.read(
        reinterpret_cast<char*>(&fh),
        sizeof(fh)
    );

    // stuguma signature-y
    if (fh.magic != isa::EXEC_MAGIC)

        throw std::runtime_error(
            "bad exec signature"
        );

    if (entryPc)
        *entryPc = fh.entryPc;

    // karduma section-neri header-ner
    std::vector<PackedSectionHeader>
        sections(fh.sectionCount);

    in.read(
        reinterpret_cast<char*>(sections.data()),
        fh.sectionCount * sizeof(PackedSectionHeader)
    );

    // ancnuma bolor section-nerov
    for (auto& s : sections) {

        // gnuma section-i texy
        in.seekg(s.offset);

        // code section
        if (
            s.type ==
            static_cast<uint32_t>(SectionType::Code)
            ) {

            code.resize(s.size / isa::WORD_SIZE);

            in.read(
                reinterpret_cast<char*>(code.data()),
                s.size
            );
        }

        // data section
        else if (
            s.type ==
            static_cast<uint32_t>(SectionType::Data)
            ) {

            data.resize(s.size / isa::WORD_SIZE);

            in.read(
                reinterpret_cast<char*>(data.data()),
                s.size
            );
        }

        // symbol table
        else if (
            symbols &&
            s.type ==
            static_cast<uint32_t>(SectionType::SymbolTable)
            ) {

            uint32_t remaining = s.size;
            while (remaining >= 8) {
                uint32_t nameLen = 0;
                in.read(reinterpret_cast<char*>(&nameLen), 4);
                remaining -= 4;

                if (nameLen == 0 || nameLen > remaining - 4)
                    break;

                std::string name(nameLen, '\0');
                in.read(name.data(), nameLen);
                remaining -= nameLen;

                uint32_t pc = 0;
                in.read(reinterpret_cast<char*>(&pc), 4);
                remaining -= 4;

                symbols->push_back({ name, pc });
            }
        }
    }

    return true;
}