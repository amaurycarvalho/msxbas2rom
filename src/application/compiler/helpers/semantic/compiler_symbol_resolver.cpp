/***
 * @file compiler_symbol_resolver.cpp
 * @brief Compiler symbol resolver
 * @author Amaury Carvalho (2019-2026)
 */

#include "compiler_symbol_resolver.h"

#include "code_node.h"
#include "compiler_code_helper.h"
#include "compiler_context.h"
#include "compiler_fixup_resolver.h"
#include "compiler_hooks.h"
#include "lexeme.h"
#include "lexer_line.h"
#include "symbol_manager.h"
#include "symbol_node.h"
#include "tag_node.h"

SymbolNode* CompilerSymbolResolver::getSymbol(shared_ptr<Lexeme> lexeme) {
  unsigned int i, t = context->symbols.size();
  bool found = false;
  SymbolNode* symbol;

  for (i = 0; i < t; i++) {
    symbol = context->symbols[i];
    if (symbol->lexeme) {
      if (symbol->lexeme->type == lexeme->type &&
          symbol->lexeme->subtype == lexeme->subtype &&
          symbol->lexeme->name == lexeme->name &&
          symbol->lexeme->value == lexeme->value) {
        found = true;
        break;
      }
    }
  }

  if (!found) symbol = 0;

  return symbol;
}

SymbolNode* CompilerSymbolResolver::addSymbol(shared_ptr<Lexeme> lexeme) {
  SymbolNode* symbol = getSymbol(lexeme);

  if (!symbol) {
    symbol = new SymbolNode();
    symbol->lexeme = lexeme;
    symbol->tag = 0;
    symbol->address = 0;
    context->symbols.push_back(symbol);
  }

  return symbol;
}

SymbolNode* CompilerSymbolResolver::getSymbol(TagNode* tag) {
  unsigned int i, t = context->symbols.size();
  bool found = false;
  SymbolNode* symbol;

  for (i = 0; i < t; i++) {
    symbol = context->symbols[i];
    if (symbol->tag) {
      if (symbol->tag->name == tag->name) {
        found = true;
        break;
      }
    }
  }

  if (!found) symbol = 0;

  return symbol;
}

SymbolNode* CompilerSymbolResolver::addSymbol(TagNode* tag) {
  SymbolNode* symbol = getSymbol(tag);

  if (!symbol) {
    symbol = new SymbolNode();
    symbol->lexeme = 0;
    symbol->tag = tag;
    symbol->address = 0;
    context->symbols.push_back(symbol);
  }

  return symbol;
}

SymbolNode* CompilerSymbolResolver::addSymbol(string line) {
  unsigned int i, t = context->symbols.size();
  bool found = false;
  SymbolNode* symbol;
  TagNode* tag;

  for (i = 0; i < t; i++) {
    symbol = context->symbols[i];
    if (symbol->tag) {
      if (symbol->tag->name == line) {
        found = true;
        break;
      }
    }
  }

  if (!found) {
    tag = new TagNode();
    tag->name = line;
    if (context->current_tag) {
      tag->value = context->current_tag->name;
      tag->lexerLine = context->current_tag->lexerLine;
    } else {
      tag->value = "SUPPORT_ROUTINE";
      tag->lexerLine = nullptr;
    }
    symbol = addSymbol(tag);
  }

  return symbol;
}

void CompilerSymbolResolver::addSupportSymbols() {
  auto& cpu = *context->cpu;
  // IO REDIRECT FUNCTION
  if (context->io_redirect_mark) {
    context->io_redirect_mark->symbol->address = cpu.context->code_pointer;
    // ld a, l  ; io number
    cpu.addLdAL();
    // ld hl, fake empty line
    cpu.addLdHL(def_ENDPRG);
    // ld ix, IOREDIRECT     ; in: a = i/o number
    cpu.addLdIX(def_IOREDIRECT);
    // call CALBAS
    cpu.addCall(def_CALBAS);
    // ei
    cpu.addEI();
    // ret
    cpu.addRet();
  }

  // IO SCREEN FUNCTION
  if (context->io_screen_mark) {
    context->io_screen_mark->symbol->address = cpu.context->code_pointer;
    // ld hl, fake empty line
    cpu.addLdHL(def_ENDPRG);
    // ld ix, IOTOSCREEN
    cpu.addLdIX(def_IOTOSCREEN);
    // call CALBAS
    cpu.addCall(def_CALBAS);
    // ei
    cpu.addEI();
    // ret
    cpu.addRet();
  }

  // DRAW STATEMENT - in: hl (pointer to string)
  if (context->draw_mark) {
    context->draw_mark->symbol->address = cpu.context->code_pointer;
    // ld a, (SCRMOD)
    cpu.addLdAii(def_SCRMOD);
    // cp 2
    cpu.addCp(0x02);
    // ret c                    ; return if text mode
    cpu.addRetC();

    // push hl
    cpu.addPushHL();

    context->codeHelper->addEnableBasicSlot();

    // pop hl
    cpu.addPopHL();

    // xor a
    cpu.addXorA();
    // ld bc, disable basic slot
    if (context->disable_basic_mark)
      context->fixupResolver->addFix(context->disable_basic_mark->symbol);
    else
      context->disable_basic_mark = context->fixupResolver->addMark();
    cpu.addLdBC(0x0000);
    // push bc
    cpu.addPushBC();
    // ld bc, 0
    cpu.addLdBC(0x0000);
    // push bc
    cpu.addPushBC();
    // push bc
    cpu.addPushBC();
    // push bc
    cpu.addPushBC();
    // ld de, 0x5D83
    cpu.addLdDE(0x5D83);
    // ld (0xFCBB), a   ; DRWFLG
    cpu.addLdiiA(0xFCBB);
    // ld (0xF958), a   ; MCLFLG
    cpu.addLdiiA(0xF958);
    // ld (0xF956), de  ; MCLTAB
    cpu.addLdiiDE(0xF956);
    // ld a, (hl)
    cpu.addLdAiHL();
    // inc hl
    cpu.addIncHL();
    // ld (0xFB3B), a   ; MCLLEN
    cpu.addLdiiA(0xFB3B);
    // jp 0x5691    ; DRAW subroutine = 0x568C+5  (main routine hook address =
    // 0x39A8)
    cpu.addJp(0x5691);
  }

  // ENABLE BASIC SLOT FUNCTION
  if (context->enable_basic_mark) {
    context->enable_basic_mark->symbol->address = cpu.context->code_pointer;
    // ld a, (EXPTBL)
    cpu.addLdAii(def_EXPTBL);
    // jr $+4            ; jump to disable code
    cpu.addJr(0x03);
  }

  // DISABLE BASIC SLOT FUNCTION
  if (context->disable_basic_mark) {
    context->disable_basic_mark->symbol->address = cpu.context->code_pointer;
    // ld a, (SLTSTR)
    cpu.addLdAii(def_SLTSTR);
    // ld h,040h        ; <--- enable jump to here
    cpu.addLdH(0x40);
    // call ENASLT		; Select the ROM on page 4000h
    cpu.addCall(def_ENASLT);
    // ei
    cpu.addEI();
    // ret
    cpu.addRet();
  }
}

void CompilerSymbolResolver::clearSymbols() {
  context->clear();
}

int CompilerSymbolResolver::saveSymbols() {
  auto& cpu = *context->cpu;
  unsigned int i, t = context->symbols.size();
  SymbolNode* symbol;
  CodeNode* codeItem;
  shared_ptr<Lexeme> lexeme;
  char* s;
  int length = 0, var_size = 0, literal_count = 0;

  for (i = 0; i < t; i++) {
    if ((symbol = context->symbols[i])) {
      if ((lexeme = symbol->lexeme)) {
        if (lexeme->isAbstract) continue;

        if (lexeme->type == Lexeme::type_literal) {
          symbol->address = cpu.context->code_pointer;

          // string constant or binary data
          if (lexeme->subtype == Lexeme::subtype_string ||
              lexeme->subtype == Lexeme::subtype_binary_data ||
              lexeme->subtype == Lexeme::subtype_integer_data) {
            int k, tt = lexeme->value.size();

            codeItem = new CodeNode();
            codeItem->name = "LIT_" + to_string(literal_count);
            codeItem->start = cpu.context->code_pointer;

            literal_count++;

            s = (char*)lexeme->value.c_str();

            if (lexeme->subtype == Lexeme::subtype_string) {
              if (tt) {
                // strip quotes

                if (s[tt - 1] == '"') tt--;
                if (s[0] == '"') {
                  s++;
                  tt--;
                }

                if (tt < 0) tt = 0;
              }
            }

            if (lexeme->subtype == Lexeme::subtype_integer_data) {
              k = atoi(s);
              cpu.addWord(k);
            } else {
              cpu.addByte(tt);

              for (k = 0; k < tt; k++) {
                cpu.addByte(s[k]);
              }
            }

            codeItem->length = cpu.context->code_pointer - codeItem->start;
            codeItem->is_code = false;
            codeItem->debug = true;
            context->symbolManager->codeList.push_back(codeItem);

            length += codeItem->length;
          }

        } else if (lexeme->type == Lexeme::type_identifier) {
          codeItem = new CodeNode();
          codeItem->name = "VAR_" + lexeme->value;
          codeItem->start = cpu.context->ram_pointer;
          codeItem->addr_within_segm =
              cpu.context->ram_page + cpu.context->ram_pointer;
          codeItem->is_code = false;
          codeItem->debug = true;
          codeItem->lexeme = lexeme;
          context->symbolManager->dataList.push_back(codeItem);

          var_size = 0;

          symbol->address = cpu.context->ram_pointer;

          // string variable
          if (lexeme->subtype == Lexeme::subtype_string) {
            var_size = 256;

            // integer variable
          } else if (lexeme->subtype == Lexeme::subtype_numeric) {
            var_size = 2;

            // double variable
          } else if (lexeme->subtype == Lexeme::subtype_double_decimal) {
            var_size = 3;

            // single variable
          } else if (lexeme->subtype == Lexeme::subtype_single_decimal) {
            var_size = 3;
          }

          if (lexeme->isArray) {
            var_size = lexeme->array_size;
            if (!var_size) {
              context->syntaxError("Array [" + lexeme->value +
                                   "] declaration is missing");
            }
          }

          codeItem->length = var_size;

          cpu.context->ram_size += var_size;
          cpu.context->ram_pointer += var_size;

          if (cpu.context->ram_size > def_RAM_SIZE) {
            context->syntaxError("Not enough memory to variables [" +
                                 to_string(cpu.context->ram_size) +
                                 "bytes occupied from RAM]");
          }
        }
      }
    }
  }

  context->temp_str_mark->address = cpu.context->ram_pointer;

  var_size = (256 * 5);  // temporary strings
  cpu.context->ram_size += var_size;
  cpu.context->ram_pointer += var_size;

  if (context->parser->getHasFont()) {
    cpu.context->ram_size += def_RAM_BUFSIZ;
    cpu.context->ram_pointer += def_RAM_BUFSIZ;
  }

  context->heap_mark->address = cpu.context->ram_pointer;

  return length;
}

CompilerSymbolResolver::CompilerSymbolResolver(CompilerContext* context)
    : context(context) {}

CompilerSymbolResolver::~CompilerSymbolResolver() = default;
