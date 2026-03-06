/***
 * @file compiler_symbol_resolver.cpp
 * @brief Compiler symbol resolver
 * @author Amaury Carvalho (2019-2026)
 */

#include "compiler_symbol_resolver.h"

#include "compiler_code_helper.h"
#include "compiler_context.h"
#include "compiler_fixup_resolver.h"
#include "compiler_hooks.h"

SymbolNode* CompilerSymbolResolver::getSymbol(Lexeme* lexeme) {
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

SymbolNode* CompilerSymbolResolver::addSymbol(Lexeme* lexeme) {
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
    if (context->current_tag)
      tag->value = context->current_tag->name;
    else
      tag->value = "SUPPORT_ROUTINE";
    symbol = addSymbol(tag);
  }

  return symbol;
}

void CompilerSymbolResolver::addSupportSymbols() {
  // IO REDIRECT FUNCTION
  if (context->io_redirect_mark) {
    context->io_redirect_mark->symbol->address =
        context->cpu->context->code_pointer;
    // ld a, l  ; io number
    context->cpu->addLdAL();
    // ld hl, fake empty line
    context->cpu->addLdHL(def_ENDPRG);
    // ld ix, IOREDIRECT     ; in: a = i/o number
    context->cpu->addLdIX(def_IOREDIRECT);
    // call CALBAS
    context->cpu->addCall(def_CALBAS);
    // ei
    context->cpu->addEI();
    // ret
    context->cpu->addRet();
  }

  // IO SCREEN FUNCTION
  if (context->io_screen_mark) {
    context->io_screen_mark->symbol->address =
        context->cpu->context->code_pointer;
    // ld hl, fake empty line
    context->cpu->addLdHL(def_ENDPRG);
    // ld ix, IOTOSCREEN
    context->cpu->addLdIX(def_IOTOSCREEN);
    // call CALBAS
    context->cpu->addCall(def_CALBAS);
    // ei
    context->cpu->addEI();
    // ret
    context->cpu->addRet();
  }

  // DRAW STATEMENT - in: hl (pointer to string)
  if (context->draw_mark) {
    context->draw_mark->symbol->address = context->cpu->context->code_pointer;
    // ld a, (SCRMOD)
    context->cpu->addLdAii(def_SCRMOD);
    // cp 2
    context->cpu->addCp(0x02);
    // ret c                    ; return if text mode
    context->cpu->addRetC();

    // push hl
    context->cpu->addPushHL();

    context->codeHelper->addEnableBasicSlot();

    // pop hl
    context->cpu->addPopHL();

    // xor a
    context->cpu->addXorA();
    // ld bc, disable basic slot
    if (context->disable_basic_mark)
      context->fixupResolver->addFix(context->disable_basic_mark->symbol);
    else
      context->disable_basic_mark = context->fixupResolver->addMark();
    context->cpu->addLdBC(0x0000);
    // push bc
    context->cpu->addPushBC();
    // ld bc, 0
    context->cpu->addLdBC(0x0000);
    // push bc
    context->cpu->addPushBC();
    // push bc
    context->cpu->addPushBC();
    // push bc
    context->cpu->addPushBC();
    // ld de, 0x5D83
    context->cpu->addLdDE(0x5D83);
    // ld (0xFCBB), a   ; DRWFLG
    context->cpu->addLdiiA(0xFCBB);
    // ld (0xF958), a   ; MCLFLG
    context->cpu->addLdiiA(0xF958);
    // ld (0xF956), de  ; MCLTAB
    context->cpu->addLdiiDE(0xF956);
    // ld a, (hl)
    context->cpu->addLdAiHL();
    // inc hl
    context->cpu->addIncHL();
    // ld (0xFB3B), a   ; MCLLEN
    context->cpu->addLdiiA(0xFB3B);
    // jp 0x5691    ; DRAW subroutine = 0x568C+5  (main routine hook address =
    // 0x39A8)
    context->cpu->addJp(0x5691);
  }

  // ENABLE BASIC SLOT FUNCTION
  if (context->enable_basic_mark) {
    context->enable_basic_mark->symbol->address =
        context->cpu->context->code_pointer;
    // ld a, (EXPTBL)
    context->cpu->addLdAii(def_EXPTBL);
    // jr $+4            ; jump to disable code
    context->cpu->addJr(0x03);
  }

  // DISABLE BASIC SLOT FUNCTION
  if (context->disable_basic_mark) {
    context->disable_basic_mark->symbol->address =
        context->cpu->context->code_pointer;
    // ld a, (SLTSTR)
    context->cpu->addLdAii(def_SLTSTR);
    // ld h,040h        ; <--- enable jump to here
    context->cpu->addLdH(0x40);
    // call ENASLT		; Select the ROM on page 4000h
    context->cpu->addCall(def_ENASLT);
    // ei
    context->cpu->addEI();
    // ret
    context->cpu->addRet();
  }
}

void CompilerSymbolResolver::clearSymbols() {
  context->clear();
}

int CompilerSymbolResolver::saveSymbols() {
  unsigned int i, t = context->symbols.size();
  SymbolNode* symbol;
  CodeNode* codeItem;
  Lexeme* lexeme;
  char* s;
  int length = 0, var_size = 0, literal_count = 0;

  for (i = 0; i < t; i++) {
    if ((symbol = context->symbols[i])) {
      if ((lexeme = symbol->lexeme)) {
        if (lexeme->isAbstract) continue;

        if (lexeme->type == Lexeme::type_literal) {
          symbol->address = context->cpu->context->code_pointer;

          // string constant or binary data
          if (lexeme->subtype == Lexeme::subtype_string ||
              lexeme->subtype == Lexeme::subtype_binary_data ||
              lexeme->subtype == Lexeme::subtype_integer_data) {
            int k, tt = lexeme->value.size();

            codeItem = new CodeNode();
            codeItem->name = "LIT_" + to_string(literal_count);
            codeItem->start = context->cpu->context->code_pointer;

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
              context->cpu->addWord(k);
            } else {
              context->cpu->addByte(tt);

              for (k = 0; k < tt; k++) {
                context->cpu->addByte(s[k]);
              }
            }

            codeItem->length =
                context->cpu->context->code_pointer - codeItem->start;
            codeItem->is_code = false;
            codeItem->debug = true;
            context->symbolManager.codeList.push_back(codeItem);

            length += codeItem->length;
          }

        } else if (lexeme->type == Lexeme::type_identifier) {
          codeItem = new CodeNode();
          codeItem->name = "VAR_" + lexeme->value;
          codeItem->start = context->cpu->context->ram_pointer;
          codeItem->addr_within_segm = context->cpu->context->ram_page +
                                       context->cpu->context->ram_pointer;
          codeItem->is_code = false;
          codeItem->debug = true;
          codeItem->lexeme = lexeme;
          context->symbolManager.dataList.push_back(codeItem);

          var_size = 0;

          symbol->address = context->cpu->context->ram_pointer;

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

          context->cpu->context->ram_size += var_size;
          context->cpu->context->ram_pointer += var_size;

          if (context->cpu->context->ram_size > def_RAM_SIZE) {
            context->syntaxError("Not enough memory to variables [" +
                                 to_string(context->cpu->context->ram_size) +
                                 "bytes occupied from RAM]");
          }
        }
      }
    }
  }

  context->temp_str_mark->address = context->cpu->context->ram_pointer;

  var_size = (256 * 5);  // temporary strings
  context->cpu->context->ram_size += var_size;
  context->cpu->context->ram_pointer += var_size;

  if (context->parser->getHasFont()) {
    context->cpu->context->ram_size += def_RAM_BUFSIZ;
    context->cpu->context->ram_pointer += def_RAM_BUFSIZ;
  }

  context->heap_mark->address = context->cpu->context->ram_pointer;

  return length;
}
