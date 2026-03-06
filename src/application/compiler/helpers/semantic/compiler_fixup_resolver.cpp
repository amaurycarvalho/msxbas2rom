/***
 * @file compiler_fixup_resolver.cpp
 * @brief Compiler fixup resolver
 * @author Amaury Carvalho (2019-2026)
 */

#include "compiler_fixup_resolver.h"

#include "compiler_code_helper.h"
#include "compiler_context.h"
#include "compiler_symbol_resolver.h"

FixNode* CompilerFixupResolver::addFix(SymbolNode* symbol) {
  FixNode* fix = new FixNode();
  bool is_id = false;

  if (symbol->lexeme) {
    is_id = (symbol->lexeme->type == Lexeme::type_identifier);
  }

  if (context->opts->megaROM && !is_id) {
    // nop, nop      ; reserved to "jr ?, ??" when "call ?, ??" or "jp ?, ??"
    context->cpu->addNop();
    context->cpu->addNop();
    // ex AF, AF'    ; save registers (will be restored by MR_ function)
    context->cpu->addExAF();
    // exx
    context->cpu->addExx();
    // ld A, <segm>
    context->cpu->addLdA(0x00);
    // ld HL, <address>
    context->cpu->addLdHL(0x0000);
    // CALL MR_????
  }

  fix->symbol = symbol;
  fix->address = context->cpu->context->code_pointer + 1;
  fix->step = 0;
  context->fixes.push_back(fix);

  return fix;
}

FixNode* CompilerFixupResolver::addFix(Lexeme* lexeme) {
  SymbolNode* symbol = context->symbolResolver->addSymbol(lexeme);
  return addFix(symbol);
}

FixNode* CompilerFixupResolver::addFix(string line) {
  return addFix(context->symbolResolver->addSymbol(line));
}

SymbolNode* CompilerFixupResolver::addPreMark() {
  string mark_name = "MARK_" + to_string(context->mark_count);
  context->mark_count++;
  return context->symbolResolver->addSymbol(mark_name);
}

FixNode* CompilerFixupResolver::addMark() {
  return addFix(addPreMark());
}

void CompilerFixupResolver::doFix() {
  unsigned int i, t = context->fixes.size(), address;
  FixNode* fix;
  SymbolNode* symbol;

  for (i = 0; i < t; i++) {
    fix = context->fixes[i];
    symbol = fix->symbol;
    address = symbol->address;

    if (!address) {
      if (symbol->lexeme) {
        context->syntaxError(
            "Symbol reference not found: variable or constant\n" +
            symbol->lexeme->toString());
      } else if (symbol->tag) {
        context->current_tag = symbol->tag;
        context->syntaxError(
            "Symbol reference not found: line number in GOTO/GOSUB/RETURN");
      } else
        context->syntaxError("Symbol reference not found");
      break;
    }

    address += fix->step;

    context->cpu->context->code[fix->address] = address & 0xFF;
    context->cpu->context->code[fix->address + 1] = (address >> 8) & 0xFF;
  }
}
