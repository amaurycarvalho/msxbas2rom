/***
 * @file elf_export_strategy.cpp
 * @brief ELF + DWARF export strategy implementation
 */

#include "elf_export_strategy.h"

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include "build_options.h"
#include "code_node.h"
#include "symbol_export_context.h"

using namespace std;

#define ELF_ST_INFO(bind, type) (((bind) << 4) + ((type) & 0xf))

/* ------------------------------------------------------------ */
/* ELF structures                                               */
/* ------------------------------------------------------------ */

struct Elf32_Ehdr {
  unsigned char e_ident[16];
  uint16_t e_type;
  uint16_t e_machine;
  uint32_t e_version;
  uint32_t e_entry;
  uint32_t e_phoff;
  uint32_t e_shoff;
  uint32_t e_flags;
  uint16_t e_ehsize;
  uint16_t e_phentsize;
  uint16_t e_phnum;
  uint16_t e_shentsize;
  uint16_t e_shnum;
  uint16_t e_shstrndx;
};

struct Elf32_Shdr {
  uint32_t sh_name;
  uint32_t sh_type;
  uint32_t sh_flags;
  uint32_t sh_addr;
  uint32_t sh_offset;
  uint32_t sh_size;
  uint32_t sh_link;
  uint32_t sh_info;
  uint32_t sh_addralign;
  uint32_t sh_entsize;
};

struct Elf32_Sym {
  uint32_t st_name;
  uint32_t st_value;
  uint32_t st_size;
  unsigned char st_info;
  unsigned char st_other;
  uint16_t st_shndx;
};

namespace {

/* ============================================================ */
/* STRING TABLE BUILDER                                         */
/* ============================================================ */

class StringTableBuilder {
 public:
  vector<char> table;

  StringTableBuilder() {
    table.push_back('\0');
  }

  uint32_t add(const string& s) {
    uint32_t pos = table.size();
    table.insert(table.end(), s.begin(), s.end());
    table.push_back('\0');
    return pos;
  }
};

/* ============================================================ */
/* SYMBOL TABLE BUILDER                                         */
/* ============================================================ */

class SymbolTableBuilder {
 public:
  vector<Elf32_Sym> symbols;

  SymbolTableBuilder() {
    symbols.push_back({0, 0, 0, 0, 0, 0});  // NULL symbol
  }

  void addFunction(uint32_t name, uint32_t addr) {
    Elf32_Sym s{};
    s.st_name = name;
    s.st_value = addr;
    s.st_info = ELF_ST_INFO(1, 2);  // global function
    s.st_shndx = 1;

    symbols.push_back(s);
  }

  void addVariable(uint32_t name, uint32_t addr, uint32_t size) {
    Elf32_Sym s{};
    s.st_name = name;
    s.st_value = addr;
    s.st_size = size;
    s.st_info = ELF_ST_INFO(1, 1);  // global object
    s.st_shndx = 2;

    symbols.push_back(s);
  }
};

/* ============================================================ */
/* DWARF BUILDER                                                */
/* ============================================================ */

class DwarfBuilder {
 public:
  vector<char> debug_info;
  vector<char> debug_abbrev;
  vector<char> debug_line;
  vector<char> debug_str;

  void buildAbbrev() {
    debug_abbrev = {1,     // abbrev code
                    0x11,  // DW_TAG_compile_unit
                    1,     // children

                    0x03, 0x08,  // DW_AT_name / string
                    0x11, 0x01,  // DW_AT_low_pc / addr
                    0x12, 0x01,  // DW_AT_high_pc / addr

                    0,    0,    0};
  }

  uint32_t addString(const string& s) {
    uint32_t pos = debug_str.size();
    debug_str.insert(debug_str.end(), s.begin(), s.end());
    debug_str.push_back('\0');
    return pos;
  }

  void buildCompileUnit(uint32_t fileNameOffset) {
    debug_info.resize(11);

    uint32_t len = 7;
    memcpy(&debug_info[0], &len, 4);

    debug_info[4] = 4;
    debug_info[5] = 0;

    uint32_t abbrev = 0;
    memcpy(&debug_info[6], &abbrev, 4);

    debug_info[10] = 4;

    debug_info.push_back(1);

    debug_info.insert(debug_info.end(), (char*)&fileNameOffset,
                      (char*)&fileNameOffset + 4);

    uint32_t low = 0;
    uint32_t high = 0xFFFF;

    debug_info.insert(debug_info.end(), (char*)&low, (char*)&low + 4);

    debug_info.insert(debug_info.end(), (char*)&high, (char*)&high + 4);

    debug_info.push_back(0);
  }

  void buildLineTable(const vector<shared_ptr<CodeNode>>& code) {
    for (auto c : code) {
      if (!c || !c->debug) continue;

      uint32_t addr = (c->segm << 16) | c->addr_within_segm;

      debug_line.insert(debug_line.end(), (char*)&addr, (char*)&addr + 4);
    }
  }
};

/* ============================================================ */
/* ELF LAYOUT                                                   */
/* ============================================================ */

struct ElfLayout {
  uint32_t symOff;
  uint32_t strOff;
  uint32_t dbgInfoOff;
  uint32_t dbgAbbrevOff;
  uint32_t dbgStrOff;
  uint32_t dbgLineOff;
  uint32_t shstrOff;
  uint32_t shOff;
};

/* ============================================================ */
/* ELF WRITER                                                   */
/* ============================================================ */

class ElfWriter {
 public:
  static bool write(const string& file, SymbolTableBuilder& sym,
                    StringTableBuilder& strtab, StringTableBuilder& shstr,
                    DwarfBuilder& dwarf) {
    FILE* f = fopen(file.c_str(), "wb");
    if (!f) return false;

    ElfLayout L = computeLayout(sym, strtab, dwarf, shstr);

    writeHeader(f, L);

    fwrite(sym.symbols.data(), sym.symbols.size() * sizeof(Elf32_Sym), 1, f);

    fwrite(strtab.table.data(), strtab.table.size(), 1, f);

    fwrite(dwarf.debug_info.data(), dwarf.debug_info.size(), 1, f);

    fwrite(dwarf.debug_abbrev.data(), dwarf.debug_abbrev.size(), 1, f);

    fwrite(dwarf.debug_str.data(), dwarf.debug_str.size(), 1, f);

    fwrite(dwarf.debug_line.data(), dwarf.debug_line.size(), 1, f);

    fwrite(shstr.table.data(), shstr.table.size(), 1, f);

    fclose(f);

    return true;
  }

 private:
  static ElfLayout computeLayout(SymbolTableBuilder& sym,
                                 StringTableBuilder& strtab,
                                 DwarfBuilder& dwarf,
                                 StringTableBuilder& shstr) {
    ElfLayout L;

    uint32_t off = sizeof(Elf32_Ehdr);

    L.symOff = off;
    off += sym.symbols.size() * sizeof(Elf32_Sym);

    L.strOff = off;
    off += strtab.table.size();

    L.dbgInfoOff = off;
    off += dwarf.debug_info.size();

    L.dbgAbbrevOff = off;
    off += dwarf.debug_abbrev.size();

    L.dbgStrOff = off;
    off += dwarf.debug_str.size();

    L.dbgLineOff = off;
    off += dwarf.debug_line.size();

    L.shstrOff = off;
    off += shstr.table.size();

    L.shOff = off;

    return L;
  }

  static void writeHeader(FILE* f, const ElfLayout& L) {
    Elf32_Ehdr eh{};
    memset(&eh, 0, sizeof(eh));

    eh.e_ident[0] = 0x7f;
    eh.e_ident[1] = 'E';
    eh.e_ident[2] = 'L';
    eh.e_ident[3] = 'F';

    eh.e_ident[4] = 1;
    eh.e_ident[5] = 1;
    eh.e_ident[6] = 1;

    eh.e_type = 1;
    eh.e_machine = 0xDC;
    eh.e_version = 1;

    eh.e_ehsize = sizeof(eh);
    eh.e_shentsize = sizeof(Elf32_Shdr);
    eh.e_shnum = 9;
    eh.e_shoff = L.shOff;
    eh.e_shstrndx = 8;

    fwrite(&eh, sizeof(eh), 1, f);
  }
};

}  // namespace

/* ============================================================ */
/* STRATEGY ENTRY                                               */
/* ============================================================ */

bool ElfExportStrategy::save(shared_ptr<SymbolExportContext> context,
                             shared_ptr<BuildOptions> opts) {
  StringTableBuilder strtab;
  StringTableBuilder shstrtab;
  SymbolTableBuilder symtab;
  DwarfBuilder dwarf;

  /* section names */

  shstrtab.add(".text");
  shstrtab.add(".data");
  shstrtab.add(".symtab");
  shstrtab.add(".strtab");
  shstrtab.add(".debug_info");
  shstrtab.add(".debug_abbrev");
  shstrtab.add(".debug_str");
  shstrtab.add(".debug_line");
  shstrtab.add(".shstrtab");

  /* functions */

  for (auto c : context->codeList) {
    if (!c || !c->debug) continue;

    uint32_t addr = (c->segm << 16) | c->addr_within_segm;
    uint32_t name = strtab.add(c->name);

    symtab.addFunction(name, addr);
  }

  /* variables */

  for (auto v : context->dataList) {
    if (!v || !v->debug) continue;

    uint32_t addr = (v->segm << 16) | v->addr_within_segm;
    uint32_t name = strtab.add(v->name);

    symtab.addVariable(name, addr, v->length);
  }

  /* dwarf */

  dwarf.buildAbbrev();

  uint32_t fname = dwarf.addString(opts->inputFilename);

  dwarf.buildCompileUnit(fname);

  dwarf.buildLineTable(context->codeList);

  /* write ELF */

  context->exportFilename = opts->baseFilename + ".elf";

  return ElfWriter::write(context->exportFilename, symtab, strtab, shstrtab,
                          dwarf);
}