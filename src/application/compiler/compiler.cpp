/***
 * @file compiler.cpp
 * @brief Compiler class implementation for semantic analysis,
 *        specialized as a Z80 code builder for MSX system
 * @author Amaury Carvalho (2019-2025)
 * @note
 *   https://en.wikipedia.org/wiki/Semantic_analysis_(computational)
 *   https://refactoring.guru/design-patterns/bridge
 *   Z80 Opcodes:
 *     http://z80-heaven.wikidot.com/instructions-set
 *     https://clrhome.org/table/
 *   Basic Kun Math Pack:
 *     https://www.msx.org/wiki/Category:X-BASIC#Floating_points
 */

#include "compiler.h"

#include "compiler_hooks.h"

/***
 * @name Compiler class code
 */

Compiler::Compiler() : IZ80() {
  code = (unsigned char*)malloc(COMPILE_CODE_SIZE);
  ram = (unsigned char*)malloc(COMPILE_RAM_SIZE);

  memset(code, 0x00, COMPILE_CODE_SIZE);
  memset(ram, 0x00, COMPILE_RAM_SIZE);

  code_size = 0;
  code_start = 0;
  code_pointer = code_start;

  ram_size = 0;
  ram_page = 0x8000;
  ram_start = def_RAM_BOTTOM - ram_page;
  ram_pointer = ram_start;
  ramMemoryPerc = 0;
  segm_last = 0;
  segm_total = 0;

  error_message = "";
  current_tag = 0;
  compiled = false;
}

Compiler::~Compiler() {
  free(code);
  free(ram);
}

bool Compiler::build(Parser* parser) {
  TagNode* tag;
  SymbolNode* symbol;
  CodeNode* codeItem;
  unsigned int i, t;

  this->parser = parser;
  this->opts = parser->getOpts();

  clearSymbols();

  t = parser->getTags().size();
  compiled = (t > 0);

  code_pointer = code_start;
  code_size = 0;
  ram_pointer = ram_start;
  ram_size = 0;
  for (i = 0; i < 5; i++) last_code[i] = &code[code_pointer];

  /// @remark END statement needs to be here (first segment) for MegaROM support
  if (opts->debug) printf("Registering END statement...");

  codeItem = new CodeNode();
  codeItem->name = "END_STMT";
  codeItem->start = code_pointer;
  cmd_end(true);  //! write END statement code
  codeItem->length = code_pointer - codeItem->start;
  codeItem->is_code = true;
  codeItem->debug = true;
  symbolManager.codeList.push_back(codeItem);
  if (opts->debug) printf(" %i byte(s)\n", codeItem->length);

  if (opts->debug) printf("Registering start of program...");

  codeItem = new CodeNode();
  codeItem->name = "START_PGM";
  codeItem->start = code_pointer;
  cmd_start();
  codeItem->length = code_pointer - codeItem->start;
  codeItem->is_code = true;
  codeItem->debug = true;
  symbolManager.codeList.push_back(codeItem);
  if (opts->debug) printf(" %i byte(s)\n", codeItem->length);
  if (codeItem->length >= 0x4000) {
    syntaxError("Maximum of start of program code per ROM reached (16k)");
    return false;
  }

  if (opts->debug) printf("Registering compiled code (line/bytes): ");

  for (i = 0; i < t; i++) {
    tag = parser->getTags()[i];

    if (tag) {
      if (opts->debug) {
        if (i)
          printf(", %s", tag->name.c_str());
        else
          printf("%s", tag->name.c_str());
      }

      if (tag->name != "DIRECTIVE") {
        // Trim leading zeros
        while (tag->name.find("0") == 0 && tag->name.size() > 1) {
          tag->name.erase(0, 1);
        }

        symbol = getSymbol(tag);
        if (symbol) {
          if (symbol->address) {
            if (opts->debug) printf(" error\n");
            current_tag = tag;
            syntaxError("Line number already declared");
            break;
          }
        } else
          symbol = addSymbol(tag);

        symbol->address = code_pointer;
      }

      codeItem = new CodeNode();
      codeItem->start = code_pointer;
      codeItem->name = "LIN_" + tag->name;

      if (!evaluate(tag)) {
        compiled = false;
        break;
      }

      codeItem->length = code_pointer - codeItem->start;
      codeItem->is_code = true;
      codeItem->debug = true;
      symbolManager.codeList.push_back(codeItem);

      if (opts->debug) printf("/%i", codeItem->length);

      if (codeItem->length >= 0x4000) {
        if (opts->debug) printf(" error\n");
        syntaxError("Maximum of code per line per ROM reached (16k)");
        return false;
      }
    }
  }

  if (opts->debug) printf("\n");

  if (compiled) {
    if (forNextStack.size()) {
      current_tag = forNextStack.top()->tag;
      syntaxError("FOR without a NEXT");
    }
  }

  if (compiled) {
    current_tag = 0;

    /// @remark END is always the last statement of the program
    if (opts->debug) printf("Registering end of program...");

    codeItem = new CodeNode();
    codeItem->name = "END_PGM";
    codeItem->start = code_pointer;
    cmd_end(false);  //! jump to the real END statement
    codeItem->length = code_pointer - codeItem->start;
    codeItem->is_code = true;
    codeItem->debug = true;
    symbolManager.codeList.push_back(codeItem);
    if (opts->debug) printf(" %i byte(s)\n", codeItem->length);
    if (codeItem->length >= 0x4000) {
      syntaxError("Maximum of end of program code per ROM reached (16k)");
      return false;
    }

    if (opts->debug) printf("Registering support code...");

    codeItem = new CodeNode();
    codeItem->start = code_pointer;
    addSupportSymbols();
    codeItem->length = code_pointer - codeItem->start;
    codeItem->is_code = true;
    codeItem->debug = false;
    symbolManager.codeList.push_back(codeItem);
    if (opts->debug) printf(" %i byte(s)\n", codeItem->length);
    if (codeItem->length >= 0x4000) {
      syntaxError("Maximum of support code per ROM reached (16k)");
      return false;
    }

    if (parser->getHasIData()) {
      if (opts->debug) printf("Registering IDATA resource...");
      resourceManager.addIDataResource(parser);
    }

    if (parser->getHasData()) {
      if (opts->debug) printf("Registering DATA resource...");
      resourceManager.addDataResource(parser);
    }

    if (opts->debug) printf("Registering symbols..");

    if (opts->debug) printf(".");

    i = saveSymbols();

    if (opts->debug) printf(" %i byte(s)\n", i);

    if (opts->debug) printf("Adjusting code/data memory address...\n");

    doFix();

    ramMemoryPerc = (ram_size * 100.0) / def_RAM_SIZE;
  }

  return compiled;
}

void Compiler::clearSymbols() {
  code_size = 0;
  code_start = 0;
  code_pointer = code_start;

  ram_size = 0;
  ram_start =
      def_RAM_BOTTOM - 0x8000;  // code_start + 0x4000 + def_RAM_INTVARSIZ;
  ram_pointer = ram_start;
  ramMemoryPerc = 0;

  error_message = "";
  current_tag = 0;

  mark_count = 0;
  for_count = 0;
  pt3 = false;
  akm = false;
  font = false;
  file_support = false;
  has_defusr = false;
  has_open_grp = false;
  has_tiny_sprite = false;
  compiled = false;

  end_mark = 0;
  enable_basic_mark = 0;
  disable_basic_mark = 0;
  draw_mark = 0;
  io_redirect_mark = 0;
  io_screen_mark = 0;

  symbols.clear();
  fixes.clear();

  symbolManager.clear();
  resourceManager.clear();

  while (!forNextStack.empty()) forNextStack.pop();

  temp_str_mark = new SymbolNode();
  temp_str_mark->lexeme = new Lexeme(
      Lexeme::type_identifier, Lexeme::subtype_numeric, "_TEMPSTR_START_", "0");
  temp_str_mark->lexeme->isAbstract = true;

  heap_mark = new SymbolNode();
  heap_mark->lexeme = new Lexeme(Lexeme::type_identifier,
                                 Lexeme::subtype_numeric, "_HEAP_", "0");
  heap_mark->lexeme->isAbstract = true;
}

int Compiler::saveSymbols() {
  unsigned int i, t = symbols.size();
  SymbolNode* symbol;
  CodeNode* codeItem;
  Lexeme* lexeme;
  char* s;
  int length = 0, var_size = 0, literal_count = 0;

  for (i = 0; i < t; i++) {
    if ((symbol = symbols[i])) {
      if ((lexeme = symbol->lexeme)) {
        if (lexeme->isAbstract) continue;

        if (lexeme->type == Lexeme::type_literal) {
          symbol->address = code_pointer;

          // string constant or binary data
          if (lexeme->subtype == Lexeme::subtype_string ||
              lexeme->subtype == Lexeme::subtype_binary_data ||
              lexeme->subtype == Lexeme::subtype_integer_data) {
            int k, tt = lexeme->value.size();

            codeItem = new CodeNode();
            codeItem->name = "LIT_" + to_string(literal_count);
            codeItem->start = code_pointer;

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
              addWord(k);
            } else {
              addByte(tt);

              for (k = 0; k < tt; k++) {
                addByte(s[k]);
              }
            }

            codeItem->length = code_pointer - codeItem->start;
            codeItem->is_code = false;
            codeItem->debug = true;
            symbolManager.codeList.push_back(codeItem);

            length += codeItem->length;
          }

        } else if (lexeme->type == Lexeme::type_identifier) {
          codeItem = new CodeNode();
          codeItem->name = "VAR_" + lexeme->value;
          codeItem->start = ram_pointer;
          codeItem->addr_within_segm = ram_page + ram_pointer;
          codeItem->is_code = false;
          codeItem->debug = true;
          codeItem->lexeme = lexeme;
          symbolManager.dataList.push_back(codeItem);

          var_size = 0;

          symbol->address = ram_pointer;

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
              syntaxError("Array [" + lexeme->value +
                          "] declaration is missing");
            }
          }

          codeItem->length = var_size;

          ram_size += var_size;
          ram_pointer += var_size;

          if (ram_size > def_RAM_SIZE) {
            syntaxError("Not enough memory to variables [" +
                        to_string(ram_size) + "bytes occupied from RAM]");
          }
        }
      }
    }
  }

  temp_str_mark->address = ram_pointer;

  var_size = (256 * 5);  // temporary strings
  ram_size += var_size;
  ram_pointer += var_size;

  if (parser->getHasFont()) {
    ram_size += def_RAM_BUFSIZ;
    ram_pointer += def_RAM_BUFSIZ;
  }

  heap_mark->address = ram_pointer;

  return length;
}

void Compiler::doFix() {
  unsigned int i, t = fixes.size(), address;
  FixNode* fix;
  SymbolNode* symbol;

  for (i = 0; i < t; i++) {
    fix = fixes[i];
    symbol = fix->symbol;
    address = symbol->address;

    if (!address) {
      if (symbol->lexeme) {
        syntaxError("Symbol reference not found: variable or constant\n" +
                    symbol->lexeme->toString());
      } else if (symbol->tag) {
        current_tag = symbol->tag;
        syntaxError(
            "Symbol reference not found: line number in GOTO/GOSUB/RETURN");
      } else
        syntaxError("Symbol reference not found");
      break;
    }

    address += fix->step;

    code[fix->address] = address & 0xFF;
    code[fix->address + 1] = (address >> 8) & 0xFF;
  }
}

void Compiler::addByteOptimized(unsigned char byte) {
  bool optimized = false;
  int code_pointer_saved = code_pointer;
  int code_reduced = 0;
  unsigned char* s;

  if (!opts->megaROM) {
    switch (byte) {
      // pop de
      case 0xD1: {
        // ld hl,x | push hl | ld hl,x | pop de
        if (*last_code[0] == 0x21 && *last_code[1] == 0xE5 &&
            *last_code[2] == 0x21 && *last_code[3] != 0xDD &&
            *last_code[3] != 0xFD) {
          // optimize to ld de,x | ld hl,x
          s = last_code[2];
          s[0] = 0x11;
          s[3] = 0x21;
          s[4] = s[5];
          s[5] = s[6];
          popLastCode();
          last_code[1] = s;
          last_code[0] = &s[3];
          code_pointer_saved = (last_code[0] - &code[0]);
          code_reduced = 1;
          code_pointer -= code_reduced;
          code_size -= code_reduced;
          optimized = true;

          // push hl | ld hl,x | pop de
        } else if (*last_code[0] == 0x21 && *last_code[1] == 0xE5) {
          // optimize to ex de,hl | ld hl,x
          s = last_code[1];
          s[0] = 0xEB;
          optimized = true;

          // ld hl,(x) | push hl | ld hl,(x) | pop de
        } else if (*last_code[0] == 0x2A && *last_code[1] == 0xE5 &&
                   *last_code[2] == 0x2A && *last_code[3] != 0xDD &&
                   *last_code[3] != 0xFD) {
          // optimize to ld hl,(x) | ex de,hl | ld hl,(x)
          s = last_code[1];
          s[0] = 0xEB;
          optimized = true;

          // push hl | ld hl,(x) | pop de
        } else if (*last_code[0] == 0x2A && *last_code[1] == 0xE5) {
          // optimize to ex de,hl | ld hl,(x)
          s = last_code[1];
          s[0] = 0xEB;
          optimized = true;
        }

      } break;

      // add hl,de
      case 0x19: {
        // ex de,hl | ld hl,x | add hl,de
        if (*last_code[0] == 0x21 && *last_code[1] == 0xEB) {
          s = last_code[0];
          // optimize to inc hl if 1, 2 or 3
          if (s[2] == 0x00 && s[1] == 1) {
            s = last_code[1];
            s[0] = 0x23;
            code_pointer -= 3;
            code_size -= 3;
            popLastCode();
            optimized = true;
          } else if (s[2] == 0x00 && s[1] == 2) {
            s = last_code[1];
            s[0] = 0x23;
            s[1] = 0x23;
            code_pointer -= 2;
            code_size -= 2;
            popLastCode();
            optimized = true;
          } else if (s[2] == 0x00 && s[1] == 3) {
            s = last_code[1];
            s[0] = 0x23;
            s[1] = 0x23;
            s[2] = 0x23;
            code_pointer--;
            code_size--;
            popLastCode();
            optimized = true;
          } else {
            // optimize to ld de,x | add hl,de
            s = last_code[1];
            s[0] = 0x11;
            s[1] = s[2];
            s[2] = s[3];
            s[3] = byte;
            last_code[0] = &s[3];
            optimized = true;
          }

          // ld hl,(x) | ld de,x | add hl,de
        } else if (*last_code[0] == 11 && *last_code[1] == 0x2A) {
          // optimize to ld hl,(x) | inc hl if 1, 2 or 3
          s = last_code[0];
          if (s[2] == 0x00) {
            switch (s[1]) {
              case 1: {
                s[0] = 0x23;
                code_pointer -= 2;
                code_size -= 2;
                optimized = true;
              } break;

              case 2: {
                s[0] = 0x23;
                s[1] = 0x23;
                code_pointer--;
                code_size--;
                optimized = true;
              } break;

              case 3: {
                s[0] = 0x23;
                s[1] = 0x23;
                s[2] = 0x23;
                optimized = true;
              } break;
            }
          }
        }

      } break;

      // ex de,hl
      case 0xEB: {
        // ex de,hl | ld hl,x | ex de,hl
        if (*last_code[0] == 0x21 && *last_code[1] == 0xEB) {
          // optimize to ld de,x
          s = last_code[1];
          s[0] = 0x11;
          s[1] = s[2];
          s[2] = s[3];
          popLastCode();
          last_code[0] = s;
          code_pointer_saved = (s - &code[0]);
          code_reduced = 1;
          code_pointer--;
          code_size--;
          optimized = true;

          // ld de,x | ld hl,x | ex de,hl
        } else if (*last_code[0] == 0x21 && *last_code[1] == 0x11 &&
                   *last_code[2] != 0xDD && *last_code[2] != 0xFD) {
          // optimize to ld hl,x | ld de,x
          s = last_code[1];
          s[0] = 0x21;
          s[3] = 0x11;
          optimized = true;

          // ld hl,x | ld de,x | ex de,hl
        } else if (*last_code[0] == 0x11 && *last_code[1] == 0x21 &&
                   *last_code[2] != 0xDD && *last_code[2] != 0xFD) {
          // optimize to ld de,x | ld hl,x
          s = last_code[1];
          s[0] = 0x11;
          s[3] = 0x21;
          optimized = true;
        }

      } break;
    }
  }

  if (optimized) {
    if (code_reduced) {
      unsigned int i, t = fixes.size();
      FixNode* fix;

      for (i = 0; i < t; i++) {
        fix = fixes[i];
        if (fix) {
          if (fix->address >= code_pointer_saved) {
            fix->address -= code_reduced;
          }
        }
      }
    }

  } else {
    pushLastCode();
    addCodeByte(byte);
  }
}

/***
 * @remarks
 * optimized kernel functions calls
 * skipping the jump map
 */
void Compiler::addKernelCall(unsigned int address) {
  addCall(getKernelCallAddr(address));
}

int Compiler::getKernelCallAddr(unsigned int address) {
  int result = address, i;

  if (address >= 0x4000 && address < 0x8000) {
    i = address - 0x4000;
    if (bin_header_bin[i] == 0xC3) {  // jp
      result = bin_header_bin[i + 1] | (bin_header_bin[i + 2] << 8);
    }
  }

  return result;
}

void Compiler::addLdHLmegarom() {
  addCmd(0xFF, 0x0000);
}

SymbolNode* Compiler::getSymbol(Lexeme* lexeme) {
  unsigned int i, t = symbols.size();
  bool found = false;
  SymbolNode* symbol;

  for (i = 0; i < t; i++) {
    symbol = symbols[i];
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

SymbolNode* Compiler::addSymbol(Lexeme* lexeme) {
  SymbolNode* symbol = getSymbol(lexeme);

  if (!symbol) {
    symbol = new SymbolNode();
    symbol->lexeme = lexeme;
    symbol->tag = 0;
    symbol->address = 0;
    symbols.push_back(symbol);
  }

  return symbol;
}

SymbolNode* Compiler::getSymbol(TagNode* tag) {
  unsigned int i, t = symbols.size();
  bool found = false;
  SymbolNode* symbol;

  for (i = 0; i < t; i++) {
    symbol = symbols[i];
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

SymbolNode* Compiler::addSymbol(TagNode* tag) {
  SymbolNode* symbol = getSymbol(tag);

  if (!symbol) {
    symbol = new SymbolNode();
    symbol->lexeme = 0;
    symbol->tag = tag;
    symbol->address = 0;
    symbols.push_back(symbol);
  }

  return symbol;
}

SymbolNode* Compiler::addSymbol(string line) {
  unsigned int i, t = symbols.size();
  bool found = false;
  SymbolNode* symbol;
  TagNode* tag;

  for (i = 0; i < t; i++) {
    symbol = symbols[i];
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
    if (current_tag)
      tag->value = current_tag->name;
    else
      tag->value = "SUPPORT_ROUTINE";
    symbol = addSymbol(tag);
  }

  return symbol;
}

FixNode* Compiler::addFix(SymbolNode* symbol) {
  FixNode* fix = new FixNode();
  bool is_id = false;

  if (symbol->lexeme) {
    is_id = (symbol->lexeme->type == Lexeme::type_identifier);
  }

  if (opts->megaROM && !is_id) {
    // nop, nop      ; reserved to "jr ?, ??" when "call ?, ??" or "jp ?, ??"
    addNop();
    addNop();
    // ex AF, AF'    ; save registers (will be restored by MR_ function)
    addExAF();
    // exx
    addExx();
    // ld A, <segm>
    addLdA(0x00);
    // ld HL, <address>
    addLdHL(0x0000);
    // CALL MR_????
  }

  fix->symbol = symbol;
  fix->address = code_pointer + 1;
  fix->step = 0;
  fixes.push_back(fix);

  return fix;
}

FixNode* Compiler::addFix(Lexeme* lexeme) {
  SymbolNode* symbol = addSymbol(lexeme);
  return addFix(symbol);
}

FixNode* Compiler::addFix(string line) {
  return addFix(addSymbol(line));
}

SymbolNode* Compiler::addPreMark() {
  string mark_name = "MARK_" + to_string(mark_count);
  mark_count++;
  return addSymbol(mark_name);
}

FixNode* Compiler::addMark() {
  return addFix(addPreMark());
}

int Compiler::write(unsigned char* dest, int start_address) {
  unsigned int i, t;
  unsigned int k, tt = 0, step;
  unsigned char *d, *s;
  int address, new_address, addr_within_segm;
  int segm_from, segm_to, length;
  FixNode *fix, *skip;
  CodeNode* codeItem;
  vector<FixNode*> skips;
  bool is_id, is_jump, is_load;

  // copy compiled code to final destination

  t = symbolManager.codeList.size();
  addr_within_segm = start_address;

  if (opts->megaROM) {
    skips.clear();

    segm_last = 2;   // last ROM segment starts at segment 2
    segm_total = 4;  // 4 segments of 8kb (0, 1, 2, 3)
    length = (start_address - 0x8000);
    code_size = 0;
    d = dest;

    for (i = 0; i < t; i++) {
      codeItem = symbolManager.codeList[i];

      // printf("%i address %i size %i\n", i, codeItem->start,
      // codeItem->length);

      if (codeItem->length) {
        s = &code[codeItem->start];

        if (codeItem->is_code)
          step = 8;
        else
          step = 3;

        tt = (length + codeItem->length + step);

        if (tt >= 0x4000) {
          segm_last += 2;  // konami segments size are 8kb (0/1, 2/3...)
          addr_within_segm = 0x8000;

          if (codeItem->is_code) {
            // code to skip a segment to another

            // ld a, segmt
            d[0] = 0x3E;
            d[1] = segm_last;  // extra code will start after segment 3
            // ld hl, 0x8000
            d[2] = 0x21;
            d[3] = 0x00;
            d[4] = 0x80;
            // jp MR_JUMP
            d[5] = 0xC3;
            d[6] = (def_MR_JUMP & 0xFF);
            d[7] = ((def_MR_JUMP >> 8) & 0xFF);

          } else {
            // data mark to skip a segment to another
            d[0] = 0xFF;
            d[1] = 0xFF;
            d[2] = 0xFF;
          }

          d += step;
          length += step;

          while (length < 0x4000) {
            d[0] = 0;
            d++;
            length++;
            step++;
          }

          skip = new FixNode();
          skip->address = codeItem->start;
          skip->step = step;
          skips.push_back(skip);

          code_size += length;

          length = 0;
        }

        memcpy(d, s, codeItem->length);

        codeItem->addr_within_segm = addr_within_segm;
        codeItem->segm = segm_last;
        if (codeItem->addr_within_segm >= (0xA000)) codeItem->segm++;

        d += codeItem->length;
        length += codeItem->length;
        addr_within_segm += codeItem->length;
      } else {
        codeItem->addr_within_segm = addr_within_segm;
        codeItem->segm = segm_last;
        if (codeItem->addr_within_segm >= (0xA000)) codeItem->segm++;
      }
    }

    code_size += length;

    segm_total = ((segm_last + 1) / 16 + 1) * 16;

    tt = skips.size();

  } else {
    for (i = 0; i < t; i++) {
      codeItem = symbolManager.codeList[i];
      codeItem->segm = 0;
      codeItem->addr_within_segm = addr_within_segm;
      addr_within_segm += codeItem->length;
    }

    memcpy(dest, code, code_size);
  }

  // reallocate code pointers to new start address

  t = fixes.size();

  for (i = 0; i < t; i++) {
    fix = fixes[i];
    if (fix) {
      if (fix->symbol) {
        if (fix->symbol->lexeme) {
          is_id = (fix->symbol->lexeme->type == Lexeme::type_identifier);
        } else {
          is_id = false;
        }

        address = fix->address;

        if (opts->megaROM) {
          step = 0;
          for (k = 0; k < tt; k++) {
            skip = skips[k];
            if (address >= skip->address) {
              step += skip->step;
            }
          }
          address += step;

          new_address = fix->symbol->address + fix->step;

          if (is_id) {
            new_address += 0x8000;

            dest[address] = new_address & 0xFF;
            dest[address + 1] = (new_address >> 8) & 0xFF;

          } else {
            step = 0;
            for (k = 0; k < tt; k++) {
              skip = skips[k];
              if (new_address >= skip->address) {
                step += skip->step;
              }
            }
            new_address += start_address + step;

            segm_from = ((address + start_address - 0x8000) / 0x4000) * 2 +
                        2;  // segments (8k/8k): 0/1, 2/3, ....
            segm_to = ((new_address - 0x8000) / 0x4000) * 2 +
                      2;  // segments (8k/8k): 0/1, 2/3, ....

            new_address = (new_address % 0x4000) + 0x8000;

            switch (dest[address - 1]) {
              case 0xFA:  // jp m,
              case 0xEA:  // jp pe,
              case 0xF2:  // jp p,
              case 0xE2:  // jp po,
              case 0xD2:  // jp nc,
              case 0xC2:  // jp nz,
              case 0xDA:  // jp c,
              case 0xCA:  // jp z,
              case 0xC3:  // jp
                is_jump = true;
                is_load = false;
                break;
              case 0xF4:  // call p,
              case 0xE4:  // call po,
              case 0xFC:  // call m,
              case 0xEC:  // call pe,
              case 0xD4:  // call nc,
              case 0xC4:  // call nz,
              case 0xDC:  // call c,
              case 0xCC:  // call z,
              case 0xCD:  // call
              case 0xFF:  // special load
                is_jump = false;
                is_load = false;
                break;
              default:
                is_jump = false;
                is_load = true;
            }

            if (segm_from == segm_to && (is_jump || is_load)) {
              dest[address] = new_address & 0xFF;
              dest[address + 1] = (new_address >> 8) & 0xFF;

              for (k = 2; k < 11; k++) {
                dest[address - k] = 0;  // 9 nops at previous bytes
              }

              dest[address - 10] = 0x18;  // jr $+8
              dest[address - 9] = 0x07;

            } else {
              // ld a, segm        ; 2 bytes
              dest[address - 5] = segm_to & 0xFF;
              // ld hl, address    ; 3 bytes
              dest[address - 3] = new_address & 0xFF;
              dest[address - 2] = (new_address >> 8) & 0xFF;

              // call MR_....      ; 3 bytes
              switch (dest[address - 1]) {
                // CALL
                case 0xF4:                    // call p,
                case 0xE4:                    // call po,
                case 0xFC:                    // call m,
                case 0xEC: {                  // call pe,
                  dest[address - 10] = 0xF5;  // push af
                  dest[address - 9] = 0x08;   // ex af, af'
                  dest[address - 8] = 0xD9;   // exx
                  dest[address - 7] = 0xF1;   // pop af
                  dest[address] = def_MR_CALL & 0xFF;
                  dest[address + 1] = (def_MR_CALL >> 8) & 0xFF;
                } break;
                case 0xD4: {                  // call nc,
                  dest[address - 10] = 0x38;  // jr c, $+11
                  dest[address - 9] = 0x0A;
                  dest[address - 1] = 0xCD;  // change to call
                  dest[address] = def_MR_CALL & 0xFF;
                  dest[address + 1] = (def_MR_CALL >> 8) & 0xFF;
                } break;
                case 0xC4: {                  // call nz,
                  dest[address - 10] = 0x28;  // jr z, $+11
                  dest[address - 9] = 0x0A;
                  dest[address - 1] = 0xCD;  // change to call
                  dest[address] = def_MR_CALL & 0xFF;
                  dest[address + 1] = (def_MR_CALL >> 8) & 0xFF;
                } break;
                case 0xDC: {                  // call c,
                  dest[address - 10] = 0x30;  // jr nc, $+11
                  dest[address - 9] = 0x0A;
                  dest[address - 1] = 0xCD;  // change to call
                  dest[address] = def_MR_CALL & 0xFF;
                  dest[address + 1] = (def_MR_CALL >> 8) & 0xFF;
                } break;
                case 0xCC: {                  // call z,
                  dest[address - 10] = 0x20;  // jr nz, $+11
                  dest[address - 9] = 0x0A;
                  dest[address - 1] = 0xCD;  // change to call
                  dest[address] = def_MR_CALL & 0xFF;
                  dest[address + 1] = (def_MR_CALL >> 8) & 0xFF;
                } break;
                case 0xCD: {  // call
                  dest[address] = def_MR_CALL & 0xFF;
                  dest[address + 1] = (def_MR_CALL >> 8) & 0xFF;
                } break;
                // JUMP
                case 0xFA:                    // jp m,
                case 0xEA:                    // jp pe,
                case 0xF2:                    // jp p,
                case 0xE2: {                  // jp po,
                  dest[address - 10] = 0xF5;  // push af
                  dest[address - 9] = 0x08;   // ex af, af'
                  dest[address - 8] = 0xD9;   // exx
                  dest[address - 7] = 0xF1;   // pop af
                  dest[address] = def_MR_JUMP & 0xFF;
                  dest[address + 1] = (def_MR_JUMP >> 8) & 0xFF;
                } break;
                case 0xD2: {                  // jp nc,
                  dest[address - 10] = 0x38;  // jr c, $+11
                  dest[address - 9] = 0x0A;
                  dest[address - 1] = 0xC3;  // change to jp
                  dest[address] = def_MR_JUMP & 0xFF;
                  dest[address + 1] = (def_MR_JUMP >> 8) & 0xFF;
                } break;
                case 0xC2: {                  // jp nz,
                  dest[address - 10] = 0x28;  // jr z, $+11
                  dest[address - 9] = 0x0A;
                  dest[address - 1] = 0xC3;  // change to jp
                  dest[address] = def_MR_JUMP & 0xFF;
                  dest[address + 1] = (def_MR_JUMP >> 8) & 0xFF;
                } break;
                case 0xDA: {                  // jp c,
                  dest[address - 10] = 0x30;  // jr nc, $+11
                  dest[address - 9] = 0x0A;
                  dest[address - 1] = 0xC3;  // change to jp
                  dest[address] = def_MR_JUMP & 0xFF;
                  dest[address + 1] = (def_MR_JUMP >> 8) & 0xFF;
                } break;
                case 0xCA: {                  // jp z,
                  dest[address - 10] = 0x20;  // jr nz, $+11
                  dest[address - 9] = 0x0A;
                  dest[address - 1] = 0xC3;  // change to jp
                  dest[address] = def_MR_JUMP & 0xFF;
                  dest[address + 1] = (def_MR_JUMP >> 8) & 0xFF;
                } break;
                case 0xC3: {  // jp
                  dest[address] = def_MR_JUMP & 0xFF;
                  dest[address + 1] = (def_MR_JUMP >> 8) & 0xFF;
                } break;
                // special LOAD (segment address into a:hl)
                case 0xFF: {
                  dest[address - 10] = dest[address - 6];
                  dest[address - 9] = dest[address - 5];
                  dest[address - 8] = dest[address - 4];
                  dest[address - 7] = dest[address - 3];
                  dest[address - 6] = dest[address - 2];
                  dest[address - 5] = 0x18;  // jr $+6
                  dest[address - 4] = 0x05;  // nop
                  dest[address - 3] = 0x00;  // nop
                  dest[address - 2] = 0x00;  // nop
                  dest[address - 1] = 0x00;  // nop
                  dest[address] = 0x00;      // nop
                  dest[address + 1] = 0x00;  // nop
                } break;
                // LOAD
                default: {
                  dest[address - 1] = 0xCD;
                  dest[address] = def_MR_GET_DATA & 0xFF;
                  dest[address + 1] = (def_MR_GET_DATA >> 8) & 0xFF;
                }
              }
            }
          }

        } else {
          new_address = (dest[address + 1] << 8) | dest[address];

          if (is_id) {
            new_address += 0x8000;
          } else {
            new_address += start_address;
          }

          dest[address] = new_address & 0xFF;
          dest[address + 1] = (new_address >> 8) & 0xFF;
        }

      } else
        printf("Error fixing symbol (null)\n");
    } else
      printf("Error in fix object (null)\n");
  }

  return code_size;
}

void Compiler::double2FloatLib(double value, int* words) {
  float value2 = value;
  float2FloatLib(value2, words);
}

void Compiler::float2FloatLib(float value, int* words) {
  unsigned short* pi = (unsigned short*)&value;
  unsigned char* pc = (unsigned char*)&value;
  unsigned char sign = 0;

  if (value == 0.0) {
    words[0] = 0;
    words[1] = 0;
    words[2] = 0;
    words[3] = 0;
    return;
  }

  if (pc[3] & 0x80) sign = 0x80;
  pc[3] <<= 1;

  if (pc[2] & 0x80) pc[3] |= 1;

  pc[2] &= 0x7F;
  pc[2] |= sign;

  pc[3]++;

  words[0] = pi[0];
  words[1] = pi[1];
  words[2] = 0;
  words[3] = 0;
}

int Compiler::str2FloatLib(string text) {
  int words[4];
  float value;
  try {
    value = stof(text);
  } catch (exception& e) {
    printf("Warning: error while converting numeric constant %s\n",
           text.c_str());
    value = 0;
  }
  float2FloatLib(value, (int*)&words);
  return ((words[1] + 0x0100) << 8) | (words[0] >> 8 & 0xff);
}

int Compiler::getUsingFormat(string text) {
  int c = 0x80;         // format style (7=1 6=, 5=* 4=$ 3=+ 2=- 1=0  0=^)
  int d = 0, e = 0;     // d=thousand digits, e=decimal digits
  int b = text.size();  // format string size
  int i;
  char* s = (char*)text.c_str();

  for (i = 0; i < b; i++) {
    switch (s[i]) {
      // 0, set zeros formating
      case '0': {
        c |= 1 << 1;  // set bit 1
        if (e)
          e++;
        else
          d++;
      } break;

      // # numeric format char, handle
      case '#': {
        if (e)
          e++;
        else
          d++;
      } break;

      // +, set sign flag and continue
      case '+': {
        c |= 1 << 3;  // set bit 3
        if (d)
          c |= 1 << 2;  // set bit 2
        else {
          if (e)
            e++;
          else
            d++;
        }
      } break;

      // -, set sign after number
      case '-': {
        c |= 1 << 2;  // set bit 2
      } break;

      // decimal point
      case '.': {
        e++;
      } break;

      // mark symbol
      case '*': {
        c |= 1 << 5;  // set bit 5
        if (e)
          e++;
        else
          d++;
      } break;

      // currency symbol
      case '$': {
        c |= 1 << 4;  // set bit 4
        if (e)
          e++;
        else
          d++;
      } break;

      // thousand separator
      case ',': {
        c |= 1 << 6;  // set bit 6
        if (e)
          e++;
        else
          d++;
      } break;

      // exponential representation
      case '^': {
        c |= 1;  // set bit 0
        if (e)
          e++;
        else
          d++;
      } break;
    }
  }

  return ((e & 0xF) | ((d & 0xF) << 4) | (c << 8));
}

//-------------------------------------------------------------------------------------------

bool Compiler::evaluate(TagNode* tag) {
  ActionNode* action;
  unsigned int i, t = tag->actions.size(), lin;

  current_tag = tag;

  if (opts->lineNumber) {
    try {
      lin = stoi(tag->name);
    } catch (exception& e) {
      printf("Warning: error while converting numeric constant %s\n",
             tag->name.c_str());
      lin = 0;
    }
    // ld hl, line number
    addLdHL(lin);
    // ld (CURLIN), hl
    addLdiiHL(def_CURLIN);
  }

  for (i = 0; i < t && compiled; i++) {
    action = tag->actions[i];

    if (!evalAction(action)) return false;
  }

  return compiled;
}

bool Compiler::evalActions(ActionNode* action) {
  ActionNode* sub_action;
  unsigned int i, t = action->actions.size();

  for (i = 0; i < t && compiled; i++) {
    sub_action = action->actions[i];

    if (!evalAction(sub_action)) return false;
  }

  return compiled;
}

bool Compiler::evalAction(ActionNode* action) {
  Lexeme* lexeme;
  ICompilerStatementStrategy* strategy;
  CompilerStatementContext ctx;

  if (!action) {
    syntaxError();
    return false;
  }

  current_action = action;
  lexeme = action->lexeme;
  if (!lexeme || lexeme->type != Lexeme::type_keyword) {
    syntaxError();
    return false;
  }

  strategy = statementStrategyFactory.getStrategyByKeyword(lexeme->name);
  if (!strategy) {
    syntaxError();
    return false;
  }

  ctx.action = action;
  ctx.lexeme = lexeme;
  ctx.dispatch = [this](CompilerCommandId command, bool& traps_checked) {
    return dispatchStatementCommand(command, traps_checked);
  };

  if (!strategy->execute(ctx)) {
    if (error_message.empty()) syntaxError();
    return false;
  }

  if (!ctx.skip_post_trap_check && !ctx.traps_checked) {
    addCheckTraps();
  }

  return compiled;
}

bool Compiler::dispatchStatementCommand(CompilerCommandId command,
                                        bool& traps_checked) {
  switch (command) {
    case CompilerCommandId::end_stmt:
      traps_checked = addCheckTraps();
      cmd_end(false);
      break;
    case CompilerCommandId::clear_stmt:
      cmd_clear();
      break;
    case CompilerCommandId::def_stmt:
      cmd_def();
      break;
    case CompilerCommandId::cls_stmt:
      cmd_cls();
      break;
    case CompilerCommandId::beep_stmt:
      cmd_beep();
      break;
    case CompilerCommandId::print_stmt:
      cmd_print();
      break;
    case CompilerCommandId::input_stmt:
      cmd_input(true);
      break;
    case CompilerCommandId::goto_stmt:
      traps_checked = addCheckTraps();
      cmd_goto();
      break;
    case CompilerCommandId::gosub_stmt:
      traps_checked = addCheckTraps();
      cmd_gosub();
      break;
    case CompilerCommandId::return_stmt:
      traps_checked = addCheckTraps();
      cmd_return();
      break;
    case CompilerCommandId::sound_stmt:
      cmd_sound();
      break;
    case CompilerCommandId::out_stmt:
      cmd_out();
      break;
    case CompilerCommandId::poke_stmt:
      cmd_poke();
      break;
    case CompilerCommandId::vpoke_stmt:
      cmd_vpoke();
      break;
    case CompilerCommandId::ipoke_stmt:
      cmd_ipoke();
      break;
    case CompilerCommandId::play_stmt:
      cmd_play();
      break;
    case CompilerCommandId::draw_stmt:
      cmd_draw();
      break;
    case CompilerCommandId::let_stmt:
      cmd_let();
      break;
    case CompilerCommandId::if_stmt:
      traps_checked = addCheckTraps();
      cmd_if();
      break;
    case CompilerCommandId::for_stmt:
      cmd_for();
      break;
    case CompilerCommandId::next_stmt:
      traps_checked = addCheckTraps();
      cmd_next();
      break;
    case CompilerCommandId::dim_stmt:
      cmd_dim();
      break;
    case CompilerCommandId::redim_stmt:
      cmd_redim();
      break;
    case CompilerCommandId::randomize_stmt:
      cmd_randomize();
      break;
    case CompilerCommandId::locate_stmt:
      cmd_locate();
      break;
    case CompilerCommandId::screen_stmt:
      cmd_screen();
      break;
    case CompilerCommandId::width_stmt:
      cmd_width();
      break;
    case CompilerCommandId::color_stmt:
      cmd_color();
      break;
    case CompilerCommandId::pset_stmt:
      cmd_pset(true);
      break;
    case CompilerCommandId::preset_stmt:
      cmd_pset(false);
      break;
    case CompilerCommandId::line_stmt:
      cmd_line();
      break;
    case CompilerCommandId::paint_stmt:
      cmd_paint();
      break;
    case CompilerCommandId::circle_stmt:
      cmd_circle();
      break;
    case CompilerCommandId::copy_stmt:
      cmd_copy();
      break;
    case CompilerCommandId::put_stmt:
      cmd_put();
      break;
    case CompilerCommandId::data_stmt:
      cmd_data();
      break;
    case CompilerCommandId::idata_stmt:
      cmd_idata();
      break;
    case CompilerCommandId::read_stmt:
      cmd_read();
      break;
    case CompilerCommandId::iread_stmt:
      cmd_iread();
      break;
    case CompilerCommandId::restore_stmt:
      cmd_restore();
      break;
    case CompilerCommandId::irestore_stmt:
      cmd_irestore();
      break;
    case CompilerCommandId::resume_stmt:
      cmd_resume();
      break;
    case CompilerCommandId::get_stmt:
      cmd_get();
      break;
    case CompilerCommandId::set_stmt:
      cmd_set();
      break;
    case CompilerCommandId::on_stmt:
      traps_checked = addCheckTraps();
      cmd_on();
      break;
    case CompilerCommandId::interval_stmt:
      cmd_interval();
      break;
    case CompilerCommandId::key_stmt:
      cmd_key();
      break;
    case CompilerCommandId::strig_stmt:
      cmd_strig();
      break;
    case CompilerCommandId::sprite_stmt:
      cmd_sprite();
      break;
    case CompilerCommandId::stop_stmt:
      traps_checked = addCheckTraps();
      cmd_stop();
      break;
    case CompilerCommandId::wait_stmt:
      cmd_wait();
      break;
    case CompilerCommandId::swap_stmt:
      cmd_swap();
      break;
    case CompilerCommandId::call_stmt:
      cmd_call();
      break;
    case CompilerCommandId::cmd_stmt:
      cmd_cmd();
      break;
    case CompilerCommandId::maxfiles_stmt:
      cmd_maxfiles();
      break;
    case CompilerCommandId::open_stmt:
      cmd_open();
      break;
    case CompilerCommandId::open_grp_stmt:
      has_open_grp = true;
      break;
    case CompilerCommandId::close_stmt:
      cmd_close();
      break;
    case CompilerCommandId::file_stmt:
      cmd_file();
      break;
    case CompilerCommandId::text_stmt:
      cmd_text();
      break;
    case CompilerCommandId::bload_stmt:
      cmd_bload();
      break;
    default:
      return false;
  }

  return compiled;
}

int Compiler::evalExpression(ActionNode* action) {
  int result = Lexeme::subtype_unknown;
  Lexeme* lexeme;

  lexeme = action->lexeme;

  if (lexeme) {
    if (lexeme->type == Lexeme::type_identifier) {
      result = lexeme->subtype;

      if (lexeme->isArray || result == Lexeme::subtype_string) {
        if (!addVarAddress(action)) {
          if (!lexeme->isArray && action->actions.size()) {
            syntaxError("Undeclared array or unknown function");
          } else
            result = Lexeme::subtype_unknown;
        } else {
          if (lexeme->subtype == Lexeme::subtype_numeric) {
            // ld e, (hl)
            addLdEiHL();
            // inc hl
            addIncHL();
            // ld d, (hl)
            addLdDiHL();
            // ex de, hl
            addExDEHL();
          } else if (lexeme->subtype == Lexeme::subtype_single_decimal ||
                     lexeme->subtype == Lexeme::subtype_double_decimal) {
            // ld b, (hl)
            addLdBiHL();
            // inc hl
            addIncHL();
            // ld e, (hl)
            addLdEiHL();
            // inc hl
            addIncHL();
            // ld d, (hl)
            addLdDiHL();
            // ex de, hl
            addExDEHL();
          }
        }
      } else {
        if (action->actions.size()) {
          syntaxError("Undeclared array or unknown function");
          return result;
        }

        // get numeric variable data optimization

        if (lexeme->subtype == Lexeme::subtype_numeric) {
          // ld hl, (variable)
          addFix(lexeme);
          addLdHLii(0x0000);
        } else if (lexeme->subtype == Lexeme::subtype_single_decimal ||
                   lexeme->subtype == Lexeme::subtype_double_decimal) {
          // ld a, (variable)
          addFix(lexeme);
          addLdAii(0x0000);
          // ld b, a
          addLdBA();
          // ld hl, (variable+1)
          addFix(lexeme)->step = 1;
          addLdHLii(0x0000);
        }
      }

    } else if (lexeme->type == Lexeme::type_keyword) {
      result = evalFunction(action);

    } else if (lexeme->type == Lexeme::type_operator) {
      result = evalOperator(action);

    } else if (lexeme->type == Lexeme::type_separator) {
      syntaxError("Invalid separator on expression");
      result = Lexeme::subtype_numeric;

    } else if (lexeme->type == Lexeme::type_literal) {
      result = lexeme->subtype;

      if (lexeme->subtype == Lexeme::subtype_string) {
        addFix(lexeme);

        addLdHL(0x0000);  // ld hl, string

      } else if (lexeme->subtype == Lexeme::subtype_numeric) {
        char* s = (char*)lexeme->value.c_str();
        try {
          if (s[0] == '&') {
            if (s[1] == 'h' || s[1] == 'H')
              addLdHL(stoi(lexeme->value.substr(2), 0, 16));  // ld hl, value
            else if (s[1] == 'o' || s[1] == 'O')
              addLdHL(stoi(lexeme->value.substr(2), 0, 8));  // ld hl, value
            else if (s[1] == 'b' || s[1] == 'B')
              addLdHL(stoi(lexeme->value.substr(2), 0, 2));  // ld hl, value
            else
              result = Lexeme::subtype_unknown;
          } else
            addLdHL(stoi(lexeme->value));  // ld hl, value
        } catch (exception& e) {
          printf("Warning: error while converting numeric constant %s\n",
                 lexeme->value.c_str());
          addLdHL(0x0000);  // ld hl, value
        }

      } else if (lexeme->subtype == Lexeme::subtype_single_decimal ||
                 lexeme->subtype == Lexeme::subtype_double_decimal) {
        int value = str2FloatLib(lexeme->value);

        addLdB((value >> 16) & 0xff);  // ld b, value
        addLdHL(value & 0xffff);       // ld hl, value

      } else if (lexeme->subtype == Lexeme::subtype_null) {
      } else {
        result = Lexeme::subtype_unknown;
      }
    } else
      result = Lexeme::subtype_unknown;
  }

  return result;
}

int Compiler::evalOperator(ActionNode* action) {
  int result = Lexeme::subtype_unknown;
  Lexeme* lexeme;
  ActionNode* next_action;
  unsigned int t = action->actions.size(), i, n, k;
  unsigned char* s;

  if (!t) return result;

  lexeme = action->lexeme;

  if (lexeme) {
    if (t == 1) {
      if (!evalOperatorParms(action, 1)) return result;

      next_action = action->actions[0];
      result = next_action->subtype;

      if (lexeme->value == "NOT") {
        if (result == Lexeme::subtype_numeric) {
          // call intCompareNOT
          // addCall(def_intCompareNOT);
          addKernelCall(def_intCompareNOT);

        } else
          result = Lexeme::subtype_unknown;

      } else if (lexeme->value == "-") {
        if (result == Lexeme::subtype_numeric) {
          // call intNEG
          // addCall(def_intNEG);
          addKernelCall(def_intNEG);

        } else if (result == Lexeme::subtype_single_decimal ||
                   result == Lexeme::subtype_double_decimal) {
          // call floatNeg
          // addCall(def_floatNEG);
          addKernelCall(def_floatNEG);

        } else
          result = Lexeme::subtype_unknown;

      } else if (lexeme->value == "+") {
        return result;
      } else
        result = Lexeme::subtype_unknown;

    } else if (t == 2) {
      if (!evalOperatorParms(action, 2)) return result;

      result = evalOperatorCast(action);
      if (result == Lexeme::subtype_unknown) return result;

      if (lexeme->value == "AND") {
        if (result == Lexeme::subtype_numeric) {
          // pop de
          addByteOptimized(0xD1);

          // call intCompareAND
          // addCall(def_intCompareAND);
          addKernelCall(def_intCompareAND);

        } else
          result = Lexeme::subtype_unknown;

      } else if (lexeme->value == "OR") {
        if (result == Lexeme::subtype_numeric) {
          // pop de
          addByteOptimized(0xD1);

          // call intCompareOR
          // addCall(def_intCompareOR);
          addKernelCall(def_intCompareOR);

        } else
          result = Lexeme::subtype_unknown;

      } else if (lexeme->value == "XOR") {
        if (result == Lexeme::subtype_numeric) {
          // pop de
          addByteOptimized(0xD1);

          // call intCompareXOR
          // addCall(def_intCompareXOR);
          addKernelCall(def_intCompareXOR);

        } else
          result = Lexeme::subtype_unknown;

      } else if (lexeme->value == "EQV") {
        // same as: not a xor b

        if (result == Lexeme::subtype_numeric) {
          // pop de
          addByteOptimized(0xD1);

          // ld a, l
          addLdAL();
          // xor e
          addXorE();
          // cpl
          addCPL();
          // ld l, a
          addLdLA();
          // ld a, h
          addAddH();
          // xor d
          addXorD();
          // cpl
          addCPL();
          // ld h, a
          addLdHA();

        } else
          result = Lexeme::subtype_unknown;

      } else if (lexeme->value == "IMP") {
        if (result == Lexeme::subtype_numeric) {
          // same as: not a or b

          // pop de
          addByteOptimized(0xD1);

          // ld a, e
          addLdAE();
          // cpl
          addCPL();
          // or l
          addOrL();
          // ld l, a
          addLdLA();
          // ld a, d
          addLdAD();
          // cpl
          addCPL();
          // or h
          addOrH();
          // ld h, a
          addLdHA();

        } else
          result = Lexeme::subtype_unknown;

      } else if (lexeme->value == "=") {
        if (result == Lexeme::subtype_numeric) {
          // pop de
          addByteOptimized(0xD1);

          // call intCompareEQ
          // addCall(def_intCompareEQ);
          addKernelCall(def_intCompareEQ);

        } else if (result == Lexeme::subtype_single_decimal ||
                   result == Lexeme::subtype_double_decimal) {
          // pop af
          addPopAF();
          // pop de
          addPopDE();
          // ld c, a
          addLdCA();

          // 7876 xbasic compare floats (=)
          addCall(def_XBASIC_COMPARE_FLOATS_EQ);

          result = Lexeme::subtype_numeric;

        } else if (result == Lexeme::subtype_string) {
          // 7e99 xbasic copy string to NULBUF
          addCall(def_XBASIC_COPY_STRING_TO_NULBUF);
          // pop hl
          addPopHL();

          // 7eae xbasic compare string (NULBUF = string)
          addCall(def_XBASIC_COMPARE_STRING_WITH_NULBUF_EQ);

          result = Lexeme::subtype_numeric;

        } else
          result = Lexeme::subtype_unknown;

      } else if (lexeme->value == "<>") {
        if (result == Lexeme::subtype_numeric) {
          // pop de
          addByteOptimized(0xD1);

          // call intCompareNE
          // addCall(def_intCompareNE);
          addKernelCall(def_intCompareNE);

        } else if (result == Lexeme::subtype_single_decimal ||
                   result == Lexeme::subtype_double_decimal) {
          // pop af
          addPopAF();
          // pop de
          addPopDE();
          // ld c, a
          addLdCA();

          // 787f xbasic compare floats (<>)
          addCall(def_XBASIC_COMPARE_FLOATS_NE);

          result = Lexeme::subtype_numeric;

        } else if (result == Lexeme::subtype_string) {
          // 7e99 xbasic copy string to NULBUF
          addCall(def_XBASIC_COPY_STRING_TO_NULBUF);
          // pop hl
          addPopHL();

          // 7ec9 xbasic compare string (NULBUF <> string)
          addCall(def_XBASIC_COMPARE_STRING_WITH_NULBUF_NE);

          result = Lexeme::subtype_numeric;

        } else
          result = Lexeme::subtype_unknown;

      } else if (lexeme->value == "<") {
        if (result == Lexeme::subtype_numeric) {
          // pop de
          addByteOptimized(0xD1);

          // call intCompareLT
          // addCall(def_intCompareLT);
          addKernelCall(def_intCompareLT);

        } else if (result == Lexeme::subtype_single_decimal ||
                   result == Lexeme::subtype_double_decimal) {
          // pop af
          addPopAF();
          // pop de
          addPopDE();
          // ld c, a
          addLdCA();

          // 7888 xbasic compare floats (>)
          addCall(def_XBASIC_COMPARE_FLOATS_GT);

          result = Lexeme::subtype_numeric;

        } else if (result == Lexeme::subtype_string) {
          // 7e99 xbasic copy string to NULBUF
          addCall(def_XBASIC_COPY_STRING_TO_NULBUF);
          // pop hl
          addPopHL();

          // 7ea4 xbasic compare string (NULBUF > string)
          addCall(def_XBASIC_COMPARE_STRING_WITH_NULBUF_GT);

          result = Lexeme::subtype_numeric;

        } else
          result = Lexeme::subtype_unknown;

      } else if (lexeme->value == "<=") {
        if (result == Lexeme::subtype_numeric) {
          // pop de
          addByteOptimized(0xD1);

          // call intCompareLE
          // addCall(def_intCompareLE);
          addKernelCall(def_intCompareLE);

        } else if (result == Lexeme::subtype_single_decimal ||
                   result == Lexeme::subtype_double_decimal) {
          // pop af
          addPopAF();
          // pop de
          addPopDE();
          // ld c, a
          addLdCA();

          // 7892 xbasic compare floats (>=)
          addCall(def_XBASIC_COMPARE_FLOATS_GE);

          result = Lexeme::subtype_numeric;

        } else if (result == Lexeme::subtype_string) {
          // 7e99 xbasic copy string to NULBUF
          addCall(def_XBASIC_COPY_STRING_TO_NULBUF);
          // pop hl
          addPopHL();

          // 7eb7 xbasic compare string (NULBUF >= string)
          addCall(def_XBASIC_COMPARE_STRING_WITH_NULBUF_GE);

          result = Lexeme::subtype_numeric;

        } else
          result = Lexeme::subtype_unknown;

      } else if (lexeme->value == ">") {
        if (result == Lexeme::subtype_numeric) {
          // pop de
          addByteOptimized(0xD1);

          // call intCompareGT
          // addCall(def_intCompareGT);
          addKernelCall(def_intCompareGT);

        } else if (result == Lexeme::subtype_single_decimal ||
                   result == Lexeme::subtype_double_decimal) {
          // pop af
          addPopAF();
          // pop de
          addPopDE();
          // ld c, a
          addLdCA();

          // 789b xbasic compare floats (<)
          addCall(def_XBASIC_COMPARE_FLOATS_LT);

          result = Lexeme::subtype_numeric;

        } else if (result == Lexeme::subtype_string) {
          // 7e99 xbasic copy string to NULBUF
          addCall(def_XBASIC_COPY_STRING_TO_NULBUF);
          // pop hl
          addPopHL();

          // 7ec0 xbasic compare string (NULBUF < string)
          addCall(def_XBASIC_COMPARE_STRING_WITH_NULBUF_LT);

          result = Lexeme::subtype_numeric;

        } else
          result = Lexeme::subtype_unknown;

      } else if (lexeme->value == ">=") {
        if (result == Lexeme::subtype_numeric) {
          // pop de
          addByteOptimized(0xD1);

          // call intCompareGE
          // addCall(def_intCompareGE);
          addKernelCall(def_intCompareGE);

        } else if (result == Lexeme::subtype_single_decimal ||
                   result == Lexeme::subtype_double_decimal) {
          // pop af
          addPopAF();
          // pop de
          addPopDE();
          // ld c, a
          addLdCA();

          // 78a4 xbasic compare floats (<=)
          addCall(def_XBASIC_COMPARE_FLOATS_LE);

          result = Lexeme::subtype_numeric;

        } else if (result == Lexeme::subtype_string) {
          // 7e99 xbasic copy string to NULBUF
          addCall(def_XBASIC_COPY_STRING_TO_NULBUF);
          // pop hl
          addPopHL();

          // 7ed2 xbasic compare string (NULBUF <= string)
          addCall(def_XBASIC_COMPARE_STRING_WITH_NULBUF_LE);

          result = Lexeme::subtype_numeric;

        } else
          result = Lexeme::subtype_unknown;

      } else if (lexeme->value == "+") {
        if (result == Lexeme::subtype_numeric) {
          // pop de
          addByteOptimized(0xD1);
          // add hl, de      ; add integers (math optimized)
          addByteOptimized(0x19);

        } else if (result == Lexeme::subtype_single_decimal ||
                   result == Lexeme::subtype_double_decimal) {
          // pop af
          addPopAF();
          // pop de
          addPopDE();
          // ld c, a
          addLdCA();
          // call 0x76c1     ; add floats (b:hl + c:de = b:hl)
          addCall(def_XBASIC_ADD_FLOATS);

        } else if (result == Lexeme::subtype_string) {
          // pop bc                      ; bc=string 1, hl=string 2
          addPopBC();
          // ld de, temporary variable   ; de=string destination
          addTempStr(false);
          // push de
          addPushDE();
          //   call 0x7f05               ; xbasic concat strings (in: bc=str1,
          //   hl=str2, de=strdest; out: hl fake)
          addCall(def_XBASIC_CONCAT_STRINGS);
          // pop hl                      ; correct destination
          addPopHL();

        } else
          result = Lexeme::subtype_unknown;

      } else if (lexeme->value == "-") {
        if (result == Lexeme::subtype_numeric) {
          // pop de
          addByteOptimized(0xD1);
          // ex de,hl
          addByteOptimized(0xEB);

          s = last_code[0];
          i = s[1] | (s[2] << 8);
          if (s[0] == 0x11 && i <= 4) {  // ld de, n
            code_pointer -= 3;
            code_size -= 3;
            while (i) {
              // dec hl
              addDecHL();
              i--;
            }
          } else {
            // and a
            addAndA();
            // sbc hl, de      ; subtract integers
            addSbcHLDE();
          }

        } else if (result == Lexeme::subtype_single_decimal ||
                   result == Lexeme::subtype_double_decimal) {
          // pop af
          addPopAF();
          // pop de
          addPopDE();
          // ex de,hl
          addExDEHL();
          // ld c, b
          addLdCB();
          // ld b, a
          addLdBA();
          // call 0x76bd     ; subtract floats (b:hl - c:de = b:hl)
          addCall(def_XBASIC_SUBTRACT_FLOATS);

        } else
          result = Lexeme::subtype_unknown;

      } else if (lexeme->value == "*") {
        if (result == Lexeme::subtype_numeric) {
          // pop de
          addByteOptimized(0xD1);

          /// @remark math optimization when second parameter is a integer
          /// constant
          if (opts->megaROM)
            s = last_code[1];
          else
            s = last_code[0];

          i = s[1] | (s[2] << 8);

          if (action->actions[0]->lexeme->type == Lexeme::type_literal &&
              i <= 256) {
            if (opts->megaROM) {
              code_pointer -= 5;  //! @todo verify if 5 or 6
              code_size -= 5;
            } else {
              code_pointer -= 4;
              code_size -= 4;

              if (action->actions[1]->lexeme->type == Lexeme::type_literal) {
                code_pointer += 1;
                code_size += 1;
                s = &code[code_pointer - 3];
                if (s[0] == 0x11) s[0] = 0x21;  // change "ld de,n" to "ld hl,n"
              }
            }

            switch (i) {
              case 0: {
                addLdHL(0x0000);
              } break;

              case 128: {
                // XOR A | SRL H | RR L | RRA | LD H, L | LD L, A
                addXorA();
                addSRLH();
                addRRL();
                addRRA();
                addLdHL();
                addLdLA();
              } break;

              case 256: {
                //  LD H, L | LD L, 0
                addLdHL();
                addLdL(0);
              } break;

              default: {
                char b[100];
                n = 0;
                k = 0;
                while (i > 1 && n < 100) {
                  b[n] = (i & 1);
                  if (b[n]) k++;
                  i >>= 1;
                  n++;
                }
                if (k) {
                  addLdEL();
                  addLdDH();
                }
                while (n) {
                  n--;
                  addAddHLHL();
                  if (b[n]) {
                    addAddHLDE();
                  }
                }
              } break;
            }

          } else {
            // call 0x761b     ; multiply integers (hl = hl * de)
            addCall(def_XBASIC_MULTIPLY_INTEGERS);
          }

        } else if (result == Lexeme::subtype_single_decimal ||
                   result == Lexeme::subtype_double_decimal) {
          // pop af
          addPopAF();
          // pop de
          addPopDE();
          // ld c, a
          addLdCA();
          // call 0x7732     ; multiply floats
          addCall(def_XBASIC_MULTIPLY_FLOATS);

        } else
          result = Lexeme::subtype_unknown;

      } else if (lexeme->value == "/") {
        if (result == Lexeme::subtype_numeric) {
          // pop de
          addByteOptimized(0xD1);
          // ex de,hl
          addByteOptimized(0xEB);

          // math optimization when second parameter is a integer constant
          s = last_code[0];
          if (s[0] == 0x11) {  // ld de, n
            i = s[1] | (s[2] << 8);

            switch (i) {
              case 0: {
                code_pointer -= 3;
                code_size -= 3;
                addLdHL(0x0000);
              } break;

              case 1: {
                code_pointer -= 3;
                code_size -= 3;
              } break;

              case 2: {
                code_pointer -= 3;
                code_size -= 3;
                // sra h | rr l
                addSRAH();
                addRRL();
                // JRNC $+6   ; jump if there's no rest of division by 2
                addJrNC(0x06);
                //   LD A, H
                addLdAH();
                //   AND 0x80  ; sign bit
                addAnd(0x80);
                //   JRZ $+1
                addJrZ(0x01);
                //     INC HL
                addIncHL();
              } break;

              case 4: {
                code_pointer -= 3;
                code_size -= 3;
                // LD A, L
                addLdAL();
                // sra h | rr l (2 times)
                addSRAH();
                addRRL();
                addSRAH();
                addRRL();
                // and 0x03
                addAnd(0x03);
                // JRZ $+6   ; jump if there's no rest of division by 4
                addJrZ(0x06);
                //   LD A, H
                addLdAH();
                //   AND 0x80  ; sign bit
                addAnd(0x80);
                //   JRZ $+1
                addJrZ(0x01);
                //     INC HL
                addIncHL();
              } break;

              case 8: {
                // OLD: LD A, L | SRA H | RRA | SRL H | RRA | SRL H | RRA | LD
                // L, A
                code_pointer -= 3;
                code_size -= 3;
                // LD A, L
                addLdAL();
                // sra h | rr l (3 times)
                addSRAH();
                addRRL();
                addSRAH();
                addRRL();
                addSRAH();
                addRRL();
                // and 0x07
                addAnd(0x07);
                // JRZ $+6   ; jump if there's no rest of division by 8
                addJrZ(0x06);
                //   LD A, H
                addLdAH();
                //   AND 0x80  ; sign bit
                addAnd(0x80);
                //   JRZ $+1
                addJrZ(0x01);
                //     INC HL
                addIncHL();
              } break;

              case 16: {
                // old: XOR A | ADD HL, HL | RLA | ADD HL, HL | RLA | ADD HL, HL
                // | RLA | ADD HL, HL | RLA | LD L, H | LD H, A
                code_pointer -= 3;
                code_size -= 3;
                // LD A, L
                addLdAL();
                // sra h | rr l (4 times)
                addSRAH();
                addRRL();
                addSRAH();
                addRRL();
                addSRAH();
                addRRL();
                addSRAH();
                addRRL();
                // and 0x0F
                addAnd(0x0F);
                // JRZ $+6   ; jump if there's no rest of division by 16
                addJrZ(0x06);
                //   LD A, H
                addLdAH();
                //   AND 0x80  ; sign bit
                addAnd(0x80);
                //   JRZ $+1
                addJrZ(0x01);
                //     INC HL
                addIncHL();
              } break;

              case 32: {
                // old: XOR A | ADD HL, HL | RLA | ADD HL, HL | RLA | ADD HL, HL
                // | RLA | LD L, H | LD H, A
                code_pointer -= 3;
                code_size -= 3;
                // LD A, L
                addLdAL();
                // sra h | rr l (5 times)
                addSRAH();
                addRRL();
                addSRAH();
                addRRL();
                addSRAH();
                addRRL();
                addSRAH();
                addRRL();
                addSRAH();
                addRRL();
                // and 0x1F
                addAnd(0x1F);
                // JRZ $+6   ; jump if there's no rest of division by 32
                addJrZ(0x06);
                //   LD A, H
                addLdAH();
                //   AND 0x80  ; sign bit
                addAnd(0x80);
                //   JRZ $+1
                addJrZ(0x01);
                //     INC HL
                addIncHL();
              } break;

              case 64: {
                // old: XOR A | ADD HL, HL | RLA | ADD HL, HL | RLA | LD L, H |
                // LD H, A
                code_pointer -= 3;
                code_size -= 3;
                // LD A, L
                addLdAL();
                // sra h | rr l (6 times)
                addSRAH();
                addRRL();
                addSRAH();
                addRRL();
                addSRAH();
                addRRL();
                addSRAH();
                addRRL();
                addSRAH();
                addRRL();
                addSRAH();
                addRRL();
                // and 0x3F
                addAnd(0x3F);
                // JRZ $+6   ; jump if there's no rest of division by 64
                addJrZ(0x06);
                //   LD A, H
                addLdAH();
                //   AND 0x80  ; sign bit
                addAnd(0x80);
                //   JRZ $+1
                addJrZ(0x01);
                //     INC HL
                addIncHL();
              } break;

              case 128: {
                // old: XOR A | ADD HL, HL | RLA | LD L, H | LD H, A
                code_pointer -= 3;
                code_size -= 3;
                // LD A, L
                addLdAL();
                // sra h | rr l (7 times)
                addSRAH();
                addRRL();
                addSRAH();
                addRRL();
                addSRAH();
                addRRL();
                addSRAH();
                addRRL();
                addSRAH();
                addRRL();
                addSRAH();
                addRRL();
                addSRAH();
                addRRL();
                // and 0x7F
                addAnd(0x7F);
                // JRZ $+6   ; jump if there's no rest of division by 64
                addJrZ(0x06);
                //   LD A, H
                addLdAH();
                //   AND 0x80  ; sign bit
                addAnd(0x80);
                //   JRZ $+1
                addJrZ(0x01);
                //     INC HL
                addIncHL();
              } break;

              case 256: {
                code_pointer -= 3;
                code_size -= 3;
                // LD E, L
                addLdEL();
                // LD L, H
                addLdLH();
                // LD H, 0x00
                addLdH(0x00);
                // LD A, L
                addLdAL();
                // AND 0x80    ; sign bit
                addAnd(0x80);
                // JRZ $+7
                addJrZ(0x07);
                //   LD H, 0xFF
                addLdH(0xFF);
                //   XOR A
                addXorA();
                //   OR E
                addOrE();
                //   JRZ $+1
                addJrZ(0x01);
                //     INC HL
                addIncHL();
              } break;

              default: {
                // call divide integers
                addCall(def_XBASIC_DIVIDE_INTEGERS);
              } break;
            }

          } else {
            // call divide integers
            addCall(def_XBASIC_DIVIDE_INTEGERS);
          }

        } else if (result == Lexeme::subtype_single_decimal ||
                   result == Lexeme::subtype_double_decimal) {
          // pop af
          addPopAF();
          // pop de
          addPopDE();
          // ex de,hl
          addExDEHL();
          // ld c, b
          addLdCB();
          // ld b, a
          addLdBA();
          // call 0x7775     ; divide floats
          addCall(def_XBASIC_DIVIDE_FLOATS);

        } else
          result = Lexeme::subtype_unknown;

      } else if (lexeme->value == "\\") {
        if (result == Lexeme::subtype_numeric) {
          // pop de
          addByteOptimized(0xD1);
          // ex de,hl
          addByteOptimized(0xEB);
          // call 0x762d     ; divide integers
          addCall(def_XBASIC_DIVIDE_INTEGERS);

        } else if (result == Lexeme::subtype_single_decimal ||
                   result == Lexeme::subtype_double_decimal) {
          // cast
          addCast(result, Lexeme::subtype_numeric);

          // pop bc
          addPopBC();
          // pop de
          addPopDE();
          // push hl
          addPushHL();
          // ex de, hl
          addExDEHL();

          // cast
          addCast(result, Lexeme::subtype_numeric);

          // pop de
          addPopDE();

          // call 0x762d     ; divide integers
          addCall(def_XBASIC_DIVIDE_INTEGERS);

          result = Lexeme::subtype_numeric;

        } else
          result = Lexeme::subtype_unknown;

      } else if (lexeme->value == "^") {
        if (result == Lexeme::subtype_numeric) {
          // ex de,hl
          addByteOptimized(0xEB);

          // pop hl
          addPopHL();

          //   call 0x782D     ; integer to float
          addCall(def_XBASIC_CAST_INTEGER_TO_FLOAT);

          // call 0x77C1       ; power float ^ integer
          addCall(def_XBASIC_POWER_FLOAT_TO_INTEGER);

          result = Lexeme::subtype_single_decimal;

        } else if (result == Lexeme::subtype_single_decimal ||
                   result == Lexeme::subtype_double_decimal) {
          // pop af
          addPopAF();
          // pop de
          addPopDE();
          // ex de,hl
          addExDEHL();
          // ld c, b
          addLdCB();
          // ld b, a
          addLdBA();
          // call 0x780d      ; power float ^ float
          addCall(def_XBASIC_POWER_FLOAT_TO_FLOAT);

        } else
          result = Lexeme::subtype_unknown;

      } else if (lexeme->value == "MOD") {
        if (result == Lexeme::subtype_numeric) {
          // pop de
          addByteOptimized(0xD1);
          // ex de,hl
          addByteOptimized(0xEB);
          // call 0x762d     ; divide integers
          addCall(def_XBASIC_DIVIDE_INTEGERS);
          // ex de, hl       ; remainder
          addExDEHL();

        } else if (result == Lexeme::subtype_single_decimal ||
                   result == Lexeme::subtype_double_decimal) {
          // cast
          addCast(result, Lexeme::subtype_numeric);

          // pop bc
          addPopBC();
          // pop de
          addPopDE();
          // push hl
          addPushHL();
          // ex de, hl
          addExDEHL();

          // cast
          addCast(result, Lexeme::subtype_numeric);

          // pop de
          addPopDE();

          // call 0x762d     ; divide integers
          addCall(def_XBASIC_DIVIDE_INTEGERS);

          // ex de, hl       ; remainder
          addExDEHL();

          result = Lexeme::subtype_numeric;

        } else
          result = Lexeme::subtype_unknown;

      } else if (lexeme->value == "SHR") {
        if (result == Lexeme::subtype_numeric) {
          // pop de
          addByteOptimized(0xD1);

          // call intSHR
          // addCall(def_intSHR);
          addKernelCall(def_intSHR);

        } else
          result = Lexeme::subtype_unknown;

      } else if (lexeme->value == "SHL") {
        if (result == Lexeme::subtype_numeric) {
          // pop de
          addByteOptimized(0xD1);

          // call intSHL
          // addCall(def_intSHL);
          addKernelCall(def_intSHL);

        } else
          result = Lexeme::subtype_unknown;

      } else
        result = Lexeme::subtype_unknown;

    } else
      result = Lexeme::subtype_unknown;
  }

  return result;
}

bool Compiler::evalOperatorParms(ActionNode* action, int parmCount) {
  bool result = false;
  int subtype;
  ActionNode* next_action;
  int i, t = action->actions.size();

  if (t == parmCount) {
    result = true;
    for (i = t - 1; i >= 0 && result; i--) {
      next_action = action->actions[i];

      subtype = evalExpression(next_action);
      result &= (subtype != Lexeme::subtype_unknown);

      next_action->subtype = subtype;

      if (i) {
        // push hl
        addPushHL();
        if (subtype == Lexeme::subtype_single_decimal ||
            subtype == Lexeme::subtype_double_decimal) {
          // push bc
          addPushBC();
        }
      }
    }
  }

  return result;
}

int Compiler::evalOperatorCast(ActionNode* action) {
  int result = Lexeme::subtype_unknown;
  ActionNode *next_action1, *next_action2;

  next_action1 = action->actions[0];
  next_action2 = action->actions[1];

  if (next_action1->subtype == next_action2->subtype) {
    result = next_action1->subtype;

  } else if (next_action2->subtype == Lexeme::subtype_numeric &&
             (next_action1->subtype == Lexeme::subtype_single_decimal ||
              next_action1->subtype == Lexeme::subtype_double_decimal)) {
    // ex: (float) + (int)
    //     200.0 + 100

    // call castParamFloatInt
    addCall(def_castParamFloatInt);

    result = next_action1->subtype;

  } else if (next_action1->subtype == Lexeme::subtype_numeric &&
             (next_action2->subtype == Lexeme::subtype_single_decimal ||
              next_action2->subtype == Lexeme::subtype_double_decimal)) {
    // ex: (int) + (float)
    // ex: 200 + 100.0

    // cast
    addCast(next_action1->subtype, next_action2->subtype);

    result = next_action2->subtype;

  } else if (next_action2->subtype == Lexeme::subtype_numeric &&
             next_action1->subtype == Lexeme::subtype_string) {
    // pop de   ; swap parameters code
    addPopDE();
    // push hl
    addPushHL();
    // ex de,hl
    addExDEHL();

    // cast
    addCast(next_action1->subtype, next_action2->subtype);

    // pop de   ; swap again
    addPopDE();
    // push hl
    addPushHL();
    // ex de,hl
    addExDEHL();

    result = next_action2->subtype;

  } else if (next_action1->subtype == Lexeme::subtype_numeric &&
             next_action2->subtype == Lexeme::subtype_string) {
    // cast
    addCast(next_action1->subtype, next_action2->subtype);

    result = next_action2->subtype;

  } else if ((next_action2->subtype == Lexeme::subtype_single_decimal ||
              next_action2->subtype == Lexeme::subtype_double_decimal) &&
             next_action1->subtype == Lexeme::subtype_string) {
    result = Lexeme::subtype_unknown;

  } else if ((next_action1->subtype == Lexeme::subtype_single_decimal ||
              next_action1->subtype == Lexeme::subtype_double_decimal) &&
             next_action2->subtype == Lexeme::subtype_string) {
    // cast
    addCast(next_action1->subtype, next_action2->subtype);

    result = next_action2->subtype;

  } else if ((next_action1->subtype == Lexeme::subtype_single_decimal &&
              next_action2->subtype == Lexeme::subtype_double_decimal) ||
             (next_action2->subtype == Lexeme::subtype_single_decimal &&
              next_action1->subtype == Lexeme::subtype_double_decimal)) {
    result = Lexeme::subtype_double_decimal;

  } else
    result = Lexeme::subtype_unknown;

  return result;
}

int Compiler::evalFunction(ActionNode* action) {
  int result[4];
  Lexeme *lexeme, *lexeme2;
  ActionNode* next_action;
  unsigned int i, t = action->actions.size();

  for (i = 0; i < 4; i++) result[i] = Lexeme::subtype_unknown;

  lexeme = action->lexeme;

  if (lexeme) {
    if (t) {
      if (lexeme->value == "VARPTR") {
        next_action = action->actions[0];
        if (next_action->lexeme->type == Lexeme::type_identifier) {
          addVarAddress(next_action);
          return Lexeme::subtype_numeric;
        } else
          return Lexeme::subtype_unknown;
      } else if (lexeme->value == "USING$") {
        if (t >= 2) {
          next_action = action->actions[1];
          lexeme2 = next_action->lexeme;
          if (lexeme2) {
            if (lexeme2->type == Lexeme::type_literal &&
                lexeme2->subtype == Lexeme::subtype_string) {
              int r = getUsingFormat(lexeme2->value);
              lexeme2->subtype = Lexeme::subtype_numeric;
              lexeme2->value = to_string(r);
            }
          }
        }
      }

      if (!evalOperatorParms(action, t)) return result[0];

      for (i = 0; i < t; i++) {
        next_action = action->actions[i];
        result[i] = next_action->subtype;

        if (result[i] == Lexeme::subtype_unknown) return result[i];
      }
    }

    switch (t) {
      case 0: {
        if (lexeme->value == "TIME") {
          // ld hl, (0xFC9E)    ; JIFFY
          addLdHLii(0xFC9E);
          result[0] = Lexeme::subtype_numeric;

        } else if (lexeme->value == "POS") {
          // ld hl, (0xF661)  ; TTYPOS
          addLdHLii(0xF661);
          // ld h, 0
          addLdH(0x00);
          result[0] = Lexeme::subtype_numeric;

        } else if (lexeme->value == "LPOS") {
          // ld hl, (0xF415)  ; LPTPOS
          addLdHLii(0xF415);
          // ld h, 0
          addLdH(0x00);
          result[0] = Lexeme::subtype_numeric;

        } else if (lexeme->value == "CSRLIN") {
          // ld hl, (0xF3DC)  ; CSRY
          addLdHLii(def_CSRY);
          // ld h, 0
          addLdH(0x00);
          result[0] = Lexeme::subtype_numeric;

        } else if (lexeme->value == "INKEY") {
          // ld hl, 0
          addLdHL(0x0000);
          // call 0x009C        ; CHSNS
          addCall(0x009C);
          // jr z,$+5
          addJrZ(0x04);
          //   call 0x009F        ; CHGET
          addCall(0x009F);
          //   ld l, a
          addLdLA();

          result[0] = Lexeme::subtype_numeric;

        } else if (lexeme->value == "INKEY$") {
          // call 0x009C        ; CHSNS
          addCall(0x009C);
          // ld hl, temporary string
          addTempStr(true);
          // call 0x7e5e   ; xbasic INKEY$ (in: hl=dest; out: hl=result)
          addCall(def_XBASIC_INKEY);

          result[0] = Lexeme::subtype_string;

        } else if (lexeme->value == "MAXFILES") {
          // ld hl, (MAXFIL)
          addLdHLii(0xF85F);

          result[0] = Lexeme::subtype_numeric;

        } else if (lexeme->value == "FRE") {
          // ld hl, (HEAPSIZ)
          addLdHLii(def_HEAPSIZ);

          result[0] = Lexeme::subtype_numeric;

        } else if (lexeme->value == "HEAP") {
          // ld hl, (HEAPSTR)
          addLdHLii(def_HEAPSTR);

          result[0] = Lexeme::subtype_numeric;

        } else if (lexeme->value == "MSX") {
          // ld hl, (VERSION)                    ; 0 = MSX1, 1 = MSX2, 2 =
          // MSX2+, 3 = MSXturboR
          addLdHLii(def_VERSION);
          // ld h, 0
          addLdH(0x00);

          result[0] = Lexeme::subtype_numeric;

        } else if (lexeme->value == "NTSC") {
          // ld hl, 0
          addLdHL(0x0000);
          // ld a, (NTSC)
          addLdAii(def_NTSC);
          // and 128   ; bit 7 on?
          addAnd(0x80);
          // jr nz, $+1
          addJrNZ(0x01);
          //    dec hl
          addDecHL();

          result[0] = Lexeme::subtype_numeric;

        } else if (lexeme->value == "VDP") {
          // VDP() without parameters returns VDP version
          // ld a, 4
          addLdA(4);
          // CALL USR2
          addCall(getKernelCallAddr(def_usr2) + 1);

          result[0] = Lexeme::subtype_numeric;

        } else if (lexeme->value == "TURBO") {
          // ld a, 5
          addLdA(5);
          // CALL USR2
          addCall(getKernelCallAddr(def_usr2) + 1);

          result[0] = Lexeme::subtype_numeric;

        } else if (lexeme->value == "COLLISION") {
          // CALL SUB_SPRCOL_ALL
          addCall(def_usr3_COLLISION_ALL);

          result[0] = Lexeme::subtype_numeric;

        } else if (lexeme->value == "MAKER") {
          // ld a, 6
          addLdA(6);
          // CALL USR2
          addCall(getKernelCallAddr(def_usr2) + 1);

          result[0] = Lexeme::subtype_numeric;

        } else if (lexeme->value == "PLYSTATUS") {
          // CALL usr2_player_status
          addCall(def_usr2_player_status);

          result[0] = Lexeme::subtype_numeric;

        } else
          result[0] = Lexeme::subtype_unknown;

      } break;

      case 1: {
        if (lexeme->value == "INT") {
          if (result[0] == Lexeme::subtype_numeric) {
            // its ok, return same parameter value
            return result[0];

          } else if (result[0] == Lexeme::subtype_single_decimal ||
                     result[0] == Lexeme::subtype_double_decimal) {
            // call 0x78e5         ; xbasic INT
            addCall(def_XBASIC_INT);

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "FIX") {
          if (result[0] == Lexeme::subtype_numeric) {
            // cast
            addCast(result[0], Lexeme::subtype_single_decimal);
            result[0] = Lexeme::subtype_single_decimal;
          }

          if (result[0] == Lexeme::subtype_single_decimal ||
              result[0] == Lexeme::subtype_double_decimal) {
            // call 0x78d8         ; xbasic FIX (in b:hl, out b:hl)
            addCall(def_XBASIC_FIX);

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "RND") {
          if (result[0] == Lexeme::subtype_numeric) {
            // cast
            addCast(result[0], Lexeme::subtype_single_decimal);
            result[0] = Lexeme::subtype_single_decimal;
          }

          if (result[0] == Lexeme::subtype_single_decimal ||
              result[0] == Lexeme::subtype_double_decimal) {
            // call 0x7678         ; xbasic RND (in b:hl, out b:hl)
            addCall(def_XBASIC_RND);

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "SIN") {
          if (result[0] == Lexeme::subtype_numeric) {
            // cast
            addCast(result[0], Lexeme::subtype_single_decimal);
            result[0] = Lexeme::subtype_single_decimal;
          }

          if (result[0] == Lexeme::subtype_single_decimal ||
              result[0] == Lexeme::subtype_double_decimal) {
            // call 0x7936         ; xbasic SIN (in b:hl, out b:hl)
            addCall(def_XBASIC_SIN);

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "COS") {
          if (result[0] == Lexeme::subtype_numeric) {
            // cast
            addCast(result[0], Lexeme::subtype_single_decimal);
            result[0] = Lexeme::subtype_single_decimal;
          }

          if (result[0] == Lexeme::subtype_single_decimal ||
              result[0] == Lexeme::subtype_double_decimal) {
            // call 0x792e         ; xbasic COS (in b:hl, out b:hl)
            addCall(def_XBASIC_COS);

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "TAN") {
          if (result[0] == Lexeme::subtype_numeric) {
            // cast
            addCast(result[0], Lexeme::subtype_single_decimal);
            result[0] = Lexeme::subtype_single_decimal;
          }

          if (result[0] == Lexeme::subtype_single_decimal ||
              result[0] == Lexeme::subtype_double_decimal) {
            // call 0x7990         ; xbasic TAN (in b:hl, out b:hl)
            addCall(def_XBASIC_TAN);

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "ATN") {
          if (result[0] == Lexeme::subtype_numeric) {
            // cast
            addCast(result[0], Lexeme::subtype_single_decimal);
            result[0] = Lexeme::subtype_single_decimal;
          }

          if (result[0] == Lexeme::subtype_single_decimal ||
              result[0] == Lexeme::subtype_double_decimal) {
            // call 0x79b2         ; xbasic ATN (in b:hl, out b:hl)
            addCall(def_XBASIC_ATN);

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "EXP") {
          if (result[0] == Lexeme::subtype_numeric) {
            // cast
            addCast(result[0], Lexeme::subtype_single_decimal);
            result[0] = Lexeme::subtype_single_decimal;
          }

          if (result[0] == Lexeme::subtype_single_decimal ||
              result[0] == Lexeme::subtype_double_decimal) {
            // call 0x79fa         ; xbasic EXP (in b:hl, out b:hl)
            addCall(def_XBASIC_EXP);

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "LOG") {
          if (result[0] == Lexeme::subtype_numeric) {
            // cast
            addCast(result[0], Lexeme::subtype_single_decimal);
            result[0] = Lexeme::subtype_single_decimal;
          }

          if (result[0] == Lexeme::subtype_single_decimal ||
              result[0] == Lexeme::subtype_double_decimal) {
            // call 0x7a53         ; xbasic LOG (in b:hl, out b:hl)
            addCall(def_XBASIC_LOG);

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "SQR") {
          if (result[0] == Lexeme::subtype_numeric) {
            // cast
            addCast(result[0], Lexeme::subtype_single_decimal);
            result[0] = Lexeme::subtype_single_decimal;
          }

          if (result[0] == Lexeme::subtype_single_decimal ||
              result[0] == Lexeme::subtype_double_decimal) {
            // call 0x7ab5         ; xbasic SQR (in b:hl, out b:hl)
            addCall(def_XBASIC_SQR);

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "SGN") {
          if (result[0] == Lexeme::subtype_numeric) {
            // call 0x5b5d         ; xbasic SGN (in hl, out hl)
            addCall(def_XBASIC_SGN_INT);

          } else if (result[0] == Lexeme::subtype_single_decimal ||
                     result[0] == Lexeme::subtype_double_decimal) {
            // call 0x5b72         ; xbasic SGN (in b:hl, out b:hl)
            addCall(def_XBASIC_SGN_FLOAT);

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "ABS") {
          if (result[0] == Lexeme::subtype_numeric) {
            // call 0x5b36         ; xbasic ABS (in hl, out hl)
            addCall(def_XBASIC_ABS_INT);

          } else if (result[0] == Lexeme::subtype_single_decimal ||
                     result[0] == Lexeme::subtype_double_decimal) {
            // xbasic ABS (in b:hl, out b:hl)
            // res 7,h
            addWord(0xCB, 0xBC);

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "VAL") {
          if (result[0] == Lexeme::subtype_numeric) {
            // its ok, return same parameter value
            return result[0];

          } else if (result[0] == Lexeme::subtype_single_decimal ||
                     result[0] == Lexeme::subtype_double_decimal) {
            // its ok, return same parameter value
            return result[0];

          } else if (result[0] == Lexeme::subtype_string) {
            // call 0x7e07   ; VAL function - xbasic string to float (in hl, out
            // b:hl)
            addCall(def_XBASIC_VAL);
            result[0] = Lexeme::subtype_single_decimal;

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "PEEK") {
          if (result[0] == Lexeme::subtype_single_decimal ||
              result[0] == Lexeme::subtype_double_decimal) {
            // cast
            addCast(result[0], Lexeme::subtype_numeric);
            result[0] = Lexeme::subtype_numeric;
          }

          if (result[0] == Lexeme::subtype_numeric) {
            // ld l,(hl)
            addLdLiHL();
            // ld h, 0
            addLdH(0x00);

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "IPEEK") {
          if (result[0] == Lexeme::subtype_single_decimal ||
              result[0] == Lexeme::subtype_double_decimal) {
            // cast
            addCast(result[0], Lexeme::subtype_numeric);
            result[0] = Lexeme::subtype_numeric;
          }

          if (result[0] == Lexeme::subtype_numeric) {
            // ld e,(hl)
            addLdEiHL();
            // inc HL
            addIncHL();
            // ld d, (hl)
            addLdDiHL();
            // ex de, hl
            addExDEHL();

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "VPEEK") {
          if (result[0] == Lexeme::subtype_single_decimal ||
              result[0] == Lexeme::subtype_double_decimal) {
            // cast
            addCast(result[0], Lexeme::subtype_numeric);
            result[0] = Lexeme::subtype_numeric;
          }

          if (result[0] == Lexeme::subtype_numeric) {
            // call 0x70a1    ; xbasic VPEEK (in:hl, out:hl)
            addCall(def_XBASIC_VPEEK);

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "INP") {
          if (result[0] == Lexeme::subtype_single_decimal ||
              result[0] == Lexeme::subtype_double_decimal) {
            // cast
            addCast(result[0], Lexeme::subtype_numeric);
            result[0] = Lexeme::subtype_numeric;
          }

          if (result[0] == Lexeme::subtype_numeric) {
            // ld c, l
            addLdCL();
            // in a, (c)
            addWord(0xED, 0x78);
            // ld l, a
            addLdLA();
            // ld h, 0
            addLdH(0x00);

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "EOF") {
          if (result[0] == Lexeme::subtype_single_decimal ||
              result[0] == Lexeme::subtype_double_decimal) {
            // cast
            addCast(result[0], Lexeme::subtype_numeric);
            result[0] = Lexeme::subtype_numeric;
          }

          if (result[0] == Lexeme::subtype_numeric) {
            // ld (DAC+2), hl
            addLdiiHL(def_DAC + 2);

            addEnableBasicSlot();

            // call HSAVD          ; alloc disk
            addCall(0xFE94);
            // call GETIOBLK       ; get io channel control block from DAC
            addCall(0x6A6A);
            // jr z, $+6           ; file not open
            addJrZ(0x05);
            // jr c, $+4           ; not a disk drive device
            addJrC(0x03);
            // call HEOF           ; put in DAC end of file status
            addCall(0xFEA3);

            addDisableBasicSlot();

            // ld hl, (DAC+2)
            addLdHLii(def_DAC + 2);

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "VDP") {
          if (result[0] == Lexeme::subtype_single_decimal ||
              result[0] == Lexeme::subtype_double_decimal) {
            // cast
            addCast(result[0], Lexeme::subtype_numeric);
            result[0] = Lexeme::subtype_numeric;
          }

          if (result[0] == Lexeme::subtype_numeric) {
            // call 0x7337         ; xbasic VDP (in: hl, out: hl)
            addCall(def_XBASIC_VDP);

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "PSG") {
          if (result[0] == Lexeme::subtype_single_decimal ||
              result[0] == Lexeme::subtype_double_decimal) {
            // cast
            addCast(result[0], Lexeme::subtype_numeric);
            result[0] = Lexeme::subtype_numeric;
          }

          if (result[0] == Lexeme::subtype_numeric) {
            // ld a, l
            addLdAL();
            // cp 16
            addCp(0x10);
            // jr nc, $+4
            addJrNC(0x03);
            //   call 0x0096         ; RDPSG (in: a = PSG register)
            addCall(0x0096);
            //   ld l, a
            addLdLA();
            //   ld h, 0
            addLdH(0x00);

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "PLAY") {
          if (result[0] == Lexeme::subtype_single_decimal ||
              result[0] == Lexeme::subtype_double_decimal) {
            // cast
            addCast(result[0], Lexeme::subtype_numeric);
            result[0] = Lexeme::subtype_numeric;
          }

          if (result[0] == Lexeme::subtype_numeric) {
            // call usr2_play
            addCall(def_usr2_play);

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "STICK") {
          if (result[0] == Lexeme::subtype_single_decimal ||
              result[0] == Lexeme::subtype_double_decimal) {
            // cast
            addCast(result[0], Lexeme::subtype_numeric);
            result[0] = Lexeme::subtype_numeric;
          }

          if (result[0] == Lexeme::subtype_numeric) {
            // ld a, l
            addLdAL();
            // call 0x00D5      ; GTSTCK
            addCall(0x00D5);
            // ld h, 0
            addLdH(0x00);
            // ld l, a
            addLdLA();

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "STRIG") {
          if (result[0] == Lexeme::subtype_single_decimal ||
              result[0] == Lexeme::subtype_double_decimal) {
            // cast
            addCast(result[0], Lexeme::subtype_numeric);
            result[0] = Lexeme::subtype_numeric;
          }

          if (result[0] == Lexeme::subtype_numeric) {
            // ld a, l
            addLdAL();
            // call 0x00D8      ; GTTRIG
            addCall(0x00D8);
            // ld h, a
            addLdHA();
            // ld l, a
            addLdLA();

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "PAD") {
          if (result[0] == Lexeme::subtype_single_decimal ||
              result[0] == Lexeme::subtype_double_decimal) {
            // cast
            addCast(result[0], Lexeme::subtype_numeric);
            result[0] = Lexeme::subtype_numeric;
          }

          if (result[0] == Lexeme::subtype_numeric) {
            // call cmd_pad
            addCall(def_cmd_pad);

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "PDL") {
          if (result[0] == Lexeme::subtype_single_decimal ||
              result[0] == Lexeme::subtype_double_decimal) {
            // cast
            addCast(result[0], Lexeme::subtype_numeric);
            result[0] = Lexeme::subtype_numeric;
          }

          if (result[0] == Lexeme::subtype_numeric) {
            // ld a, l
            addLdAL();
            // call 0x00DE      ; GTPDL
            addCall(0x00DE);
            // ld h, 0
            addLdH(0x00);
            // ld l, a
            addLdLA();

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "BASE") {
          if (result[0] == Lexeme::subtype_single_decimal ||
              result[0] == Lexeme::subtype_double_decimal) {
            // cast
            addCast(result[0], Lexeme::subtype_numeric);
            result[0] = Lexeme::subtype_numeric;
          }

          if (result[0] == Lexeme::subtype_numeric) {
            // call base function
            addCall(def_XBASIC_BASE);
            // addCall(def_XBASIC_ABS_INT);  // abs()

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "ASC") {
          if (result[0] == Lexeme::subtype_string) {
            // ex de, hl
            addByteOptimized(0xEB);
            // ld hl, 0
            addLdHL(0x0000);
            // ld a, (de)
            addLdAiDE();
            // and a
            addAndA();
            // jr z,$+4
            addJrZ(0x03);
            //   inc de
            addIncDE();
            //   ld a, (de)
            addLdAiDE();
            //   ld l, a
            addLdLA();

            result[0] = Lexeme::subtype_numeric;

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "LEN") {
          if (result[0] == Lexeme::subtype_string) {
            // ld l, (hl)
            addLdLiHL();
            // ld h, 0
            addLdH(0x00);

            result[0] = Lexeme::subtype_numeric;

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "CSNG") {
          // cast
          addCast(result[0], Lexeme::subtype_single_decimal);

          result[0] = Lexeme::subtype_single_decimal;

        } else if (lexeme->value == "CDBL") {
          // cast
          addCast(result[0], Lexeme::subtype_double_decimal);

          result[0] = Lexeme::subtype_double_decimal;

        } else if (lexeme->value == "CINT") {
          // cast
          addCast(result[0], Lexeme::subtype_numeric);

          result[0] = Lexeme::subtype_numeric;

        } else if (lexeme->value == "POS") {
          // ld hl, (0xF661)  ; TTYPOS
          addLdHLii(0xF661);
          // ld h, 0
          addLdH(0x00);
          result[0] = Lexeme::subtype_numeric;

        } else if (lexeme->value == "LPOS") {
          // ld hl, (0xF415)  ; LPTPOS
          addLdHLii(0xF415);
          // ld h, 0
          addLdH(0x00);
          result[0] = Lexeme::subtype_numeric;

        } else if (lexeme->value == "CSRLIN") {
          // ld hl, (0xF3DC)  ; CSRY
          addLdHLii(0xF3DC);
          // ld h, 0
          addLdH(0x00);
          result[0] = Lexeme::subtype_numeric;

        } else if (lexeme->value == "CHR$") {
          if (result[0] == Lexeme::subtype_single_decimal ||
              result[0] == Lexeme::subtype_double_decimal) {
            // cast
            addCast(result[0], Lexeme::subtype_numeric);
            result[0] = Lexeme::subtype_numeric;
          }

          if (result[0] == Lexeme::subtype_numeric) {
            // ex de, hl
            addByteOptimized(0xEB);
            // ld hl, temporary string
            addTempStr(true);
            // inc d
            addIncD();
            // ld (hl), d
            addLdiHLD();
            // inc hl
            addIncHL();
            // ld (hl), e
            addLdiHLE();
            // dec hl
            addDecHL();

            result[0] = Lexeme::subtype_string;

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "SPACE$" || lexeme->value == "SPC") {
          if (result[0] == Lexeme::subtype_single_decimal ||
              result[0] == Lexeme::subtype_double_decimal) {
            // cast
            addCast(result[0], Lexeme::subtype_numeric);
            result[0] = Lexeme::subtype_numeric;
          }

          if (result[0] == Lexeme::subtype_numeric) {
            // ld a, 0x20      ; space
            addLdA(0x20);
            // ld b, l
            addLdBL();
            // ld hl, temporary string
            addTempStr(true);
            // call 0x7e4c    ; STRING$ (hl=destination, b=number of chars,
            // a=char)
            addCall(def_XBASIC_STRING);

            result[0] = Lexeme::subtype_string;

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "TAB") {
          if (result[0] == Lexeme::subtype_single_decimal ||
              result[0] == Lexeme::subtype_double_decimal) {
            // cast
            addCast(result[0], Lexeme::subtype_numeric);
            result[0] = Lexeme::subtype_numeric;
          }

          if (result[0] == Lexeme::subtype_numeric) {
            // call tab function
            addCall(def_XBASIC_TAB);

            result[0] = Lexeme::subtype_string;

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "STR$") {
          // cast
          addCast(result[0], Lexeme::subtype_string);

          result[0] = Lexeme::subtype_string;

        } else if (lexeme->value == "INPUT$") {
          if (result[0] == Lexeme::subtype_single_decimal ||
              result[0] == Lexeme::subtype_double_decimal) {
            addCast(result[0], Lexeme::subtype_numeric);
            result[0] = Lexeme::subtype_numeric;
          }

          if (result[0] == Lexeme::subtype_numeric) {
            // ex de, hl
            addByteOptimized(0xEB);
            // ld hl, temporary string
            addTempStr(true);
            // push hl
            addPushHL();
            //   ld (hl), e
            addLdiHLE();
            //   inc hl
            addIncHL();
            //     call 0x009F        ; CHGET
            addCall(0x009F);
            //     ld (hl), a
            addLdiHLA();
            //     inc hl
            addIncHL();
            //     dec e
            addDecE();
            //   jr nz,$-8
            addJrNZ((unsigned char)(0xFF - 7));
            // pop hl
            addPopHL();

            result[0] = Lexeme::subtype_string;

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "BIN$") {
          if (result[0] == Lexeme::subtype_single_decimal ||
              result[0] == Lexeme::subtype_double_decimal) {
            // cast
            addCast(result[0], Lexeme::subtype_numeric);
            result[0] = Lexeme::subtype_numeric;
          }

          if (result[0] == Lexeme::subtype_numeric) {
            // ld c, 1
            addLdC(0x01);
            // call 0x7e22    ; xbasic OCT$/HEX$/BIN$ (in: hl=integer, de=BUF,
            // c=mode [1=bin, 3=oct, 4=hex]; out: hl destination corrected)
            addCall(def_XBASIC_OCT_HEX_BIN);
            // ld de, temporary string
            addTempStr(false);
            // push de
            addPushDE();
            //   call 0x7e9d   ; xbasic copy string (in: hl=source, de=dest;
            //   out: hl end of string)
            addCall(def_XBASIC_COPY_STRING);
            // pop hl
            addPopHL();

            result[0] = Lexeme::subtype_string;

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "OCT$") {
          if (result[0] == Lexeme::subtype_single_decimal ||
              result[0] == Lexeme::subtype_double_decimal) {
            // cast
            addCast(result[0], Lexeme::subtype_numeric);
            result[0] = Lexeme::subtype_numeric;
          }

          if (result[0] == Lexeme::subtype_numeric) {
            // ld c, 3
            addLdC(0x03);
            // call 0x7e22    ; xbasic OCT$/HEX$/BIN$ (in: hl=integer, de=BUF,
            // c=mode [1=bin, 3=oct, 4=hex]; out: hl destination corrected)
            addCall(def_XBASIC_OCT_HEX_BIN);
            // ld de, temporary string
            addTempStr(false);
            // push de
            addPushDE();
            //   call 0x7e9d   ; xbasic copy string (in: hl=source, de=dest;
            //   out: hl end of string)
            addCall(def_XBASIC_COPY_STRING);
            // pop hl
            addPopHL();

            result[0] = Lexeme::subtype_string;

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "HEX$") {
          if (result[0] == Lexeme::subtype_single_decimal ||
              result[0] == Lexeme::subtype_double_decimal) {
            // cast
            addCast(result[0], Lexeme::subtype_numeric);
            result[0] = Lexeme::subtype_numeric;
          }

          if (result[0] == Lexeme::subtype_numeric) {
            // ld c, 4
            addLdC(0x04);
            // call 0x7e22    ; xbasic OCT$/HEX$/BIN$ (in: hl=integer, de=BUF,
            // c=mode [1=bin, 3=oct, 4=hex]; out: hl destination corrected)
            addCall(def_XBASIC_OCT_HEX_BIN);
            // ld de, temporary string
            addTempStr(false);
            // push de
            addPushDE();
            //   call 0x7e9d   ; xbasic copy string (in: hl=source, de=dest;
            //   out: hl end of string)
            addCall(def_XBASIC_COPY_STRING);
            // pop hl
            addPopHL();

            result[0] = Lexeme::subtype_string;

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "RESOURCE") {
          // cast
          addCast(result[0], Lexeme::subtype_numeric);
          result[0] = Lexeme::subtype_numeric;

          // call usr0
          addCall(def_usr0);

        } else if (lexeme->value == "RESOURCESIZE") {
          // cast
          addCast(result[0], Lexeme::subtype_numeric);
          result[0] = Lexeme::subtype_numeric;

          // call usr1
          addCall(def_usr1);

        } else if (lexeme->value == "COLLISION") {
          // cast
          addCast(result[0], Lexeme::subtype_numeric);
          result[0] = Lexeme::subtype_numeric;

          // call SUB_SPRCOL_ONE
          addCall(def_usr3_COLLISION_ONE);

        } else if (lexeme->value == "USR" || lexeme->value == "USR0") {
          // cast
          addCast(result[0], Lexeme::subtype_numeric);
          result[0] = Lexeme::subtype_numeric;

          if (has_defusr) {
            // xor a
            addXorA();
            // call XBASIC_USR
            addCall(def_XBASIC_USR);
          } else {
            // call usr0
            addCall(def_usr0);
          }

        } else if (lexeme->value == "USR1") {
          // cast
          addCast(result[0], Lexeme::subtype_numeric);
          result[0] = Lexeme::subtype_numeric;

          if (has_defusr) {
            // ld a, 1
            addLdA(0x01);
            // call XBASIC_USR
            addCall(def_XBASIC_USR);
          } else {
            // call usr1
            addCall(def_usr1);
          }

        } else if (lexeme->value == "USR2") {
          // cast
          addCast(result[0], Lexeme::subtype_numeric);
          result[0] = Lexeme::subtype_numeric;

          if (has_defusr) {
            // ld a, 2
            addLdA(0x02);
            // call XBASIC_USR
            addCall(def_XBASIC_USR);
          } else {
            // call usr2
            addCall(def_usr2);
          }

        } else if (lexeme->value == "USR3") {
          // cast
          addCast(result[0], Lexeme::subtype_numeric);
          result[0] = Lexeme::subtype_numeric;

          if (has_defusr) {
            // ld a, 3
            addLdA(0x03);
            // call XBASIC_USR
            addCall(def_XBASIC_USR);
          } else {
            // call usr3
            addCall(def_usr3);
          }

        } else if (lexeme->value == "USR4") {
          // cast
          addCast(result[0], Lexeme::subtype_numeric);
          result[0] = Lexeme::subtype_numeric;

          if (has_defusr) {
            // ld a, 4
            addLdA(0x04);
            // call XBASIC_USR
            addCall(def_XBASIC_USR);
          }

        } else if (lexeme->value == "USR5") {
          // cast
          addCast(result[0], Lexeme::subtype_numeric);
          result[0] = Lexeme::subtype_numeric;

          if (has_defusr) {
            // ld a, 5
            addLdA(0x05);
            // call XBASIC_USR
            addCall(def_XBASIC_USR);
          }

        } else if (lexeme->value == "USR6") {
          // cast
          addCast(result[0], Lexeme::subtype_numeric);
          result[0] = Lexeme::subtype_numeric;

          if (has_defusr) {
            // ld a, 6
            addLdA(0x06);
            // call XBASIC_USR
            addCall(def_XBASIC_USR);
          }

        } else if (lexeme->value == "USR7") {
          // cast
          addCast(result[0], Lexeme::subtype_numeric);
          result[0] = Lexeme::subtype_numeric;

          if (has_defusr) {
            // ld a, 7
            addLdA(0x07);
            // call XBASIC_USR
            addCall(def_XBASIC_USR);
          }

        } else if (lexeme->value == "USR8") {
          // cast
          addCast(result[0], Lexeme::subtype_numeric);
          result[0] = Lexeme::subtype_numeric;

          if (has_defusr) {
            // ld a, 8
            addLdA(0x08);
            // call XBASIC_USR
            addCall(def_XBASIC_USR);
          }

        } else if (lexeme->value == "USR9") {
          // cast
          addCast(result[0], Lexeme::subtype_numeric);
          result[0] = Lexeme::subtype_numeric;

          if (has_defusr) {
            // ld a, 9
            addLdA(0x09);
            // call XBASIC_USR
            addCall(def_XBASIC_USR);
          }

        } else
          result[0] = Lexeme::subtype_unknown;

      } break;

      case 2: {
        if (lexeme->value == "POINT") {
          if (result[0] == Lexeme::subtype_single_decimal ||
              result[0] == Lexeme::subtype_double_decimal) {
            // cast
            addCast(result[0], Lexeme::subtype_numeric);
            result[0] = Lexeme::subtype_numeric;
          }

          if (result[1] == Lexeme::subtype_single_decimal ||
              result[1] == Lexeme::subtype_double_decimal) {
            // ex de,hl
            addByteOptimized(0xEB);
            // pop bc
            addPopBC();
            // pop hl
            addPopHL();
            // push de
            addPushDE();

            // cast
            addCast(result[1], Lexeme::subtype_numeric);
            result[1] = Lexeme::subtype_numeric;

            // pop de
            addPopDE();
            // ex de,hl
            addExDEHL();

          } else {
            // pop de
            addPopDE();
          }

          if (result[1] == Lexeme::subtype_numeric &&
              result[0] == Lexeme::subtype_numeric) {
            // call 0x6fa7     ; xbasic POINT (in: de=x, hl=y; out: hl=color)
            addCall(def_XBASIC_POINT);

            result[0] = Lexeme::subtype_numeric;

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "TILE") {
          if (result[0] == Lexeme::subtype_single_decimal ||
              result[0] == Lexeme::subtype_double_decimal) {
            // cast
            addCast(result[0], Lexeme::subtype_numeric);
            result[0] = Lexeme::subtype_numeric;
          }

          if (result[1] == Lexeme::subtype_single_decimal ||
              result[1] == Lexeme::subtype_double_decimal) {
            // ex de,hl
            addByteOptimized(0xEB);
            // pop bc
            addPopBC();
            // pop hl
            addPopHL();
            // push de
            addPushDE();

            // cast
            addCast(result[1], Lexeme::subtype_numeric);
            result[1] = Lexeme::subtype_numeric;

            // pop de
            addPopDE();
            // ex de,hl
            addExDEHL();

          } else {
            // pop de
            addPopDE();
          }

          if (result[1] == Lexeme::subtype_numeric &&
              result[0] == Lexeme::subtype_numeric) {
            // ld h, e
            addLdHE();
            // inc l
            addIncL();
            // inc h     ; bios based coord system (home=1,1)
            addIncH();

            // call def_tileAddress (in: hl=xy; out: hl=address)
            addCall(def_tileAddress);

            // call 0x70a1    ; xbasic VPEEK (in:hl, out:hl)
            addCall(def_XBASIC_VPEEK);

            result[0] = Lexeme::subtype_numeric;

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "COLLISION") {
          if (result[0] == Lexeme::subtype_single_decimal ||
              result[0] == Lexeme::subtype_double_decimal) {
            // cast
            addCast(result[0], Lexeme::subtype_numeric);
            result[0] = Lexeme::subtype_numeric;
          }

          if (result[1] == Lexeme::subtype_single_decimal ||
              result[1] == Lexeme::subtype_double_decimal) {
            // ex de,hl
            addByteOptimized(0xEB);
            // pop bc
            addPopBC();
            // pop hl
            addPopHL();
            // push de
            addPushDE();

            // cast
            addCast(result[1], Lexeme::subtype_numeric);
            result[1] = Lexeme::subtype_numeric;

            // pop de
            addPopDE();
            // ex de,hl
            addExDEHL();

          } else {
            // pop de
            addPopDE();
          }

          if (result[1] == Lexeme::subtype_numeric &&
              result[0] == Lexeme::subtype_numeric) {
            // call SUB_SPRCOL_COUPLE
            addCall(def_usr3_COLLISION_COUPLE);

            result[0] = Lexeme::subtype_numeric;

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "STRING$") {
          if (result[0] == Lexeme::subtype_single_decimal ||
              result[0] == Lexeme::subtype_double_decimal) {
            // cast
            addCast(result[0], Lexeme::subtype_numeric);
            result[0] = Lexeme::subtype_numeric;
          } else if (result[0] == Lexeme::subtype_string) {
            // inc hl
            addIncHL();
            // ld l, (hl)
            addLdLiHL();
            // ld h, 0
            addLdH(0x00);
            result[0] = Lexeme::subtype_numeric;
          }

          if (result[1] == Lexeme::subtype_numeric) {
            // pop de
            addPopDE();
          } else if (result[1] == Lexeme::subtype_single_decimal ||
                     result[1] == Lexeme::subtype_double_decimal) {
            // pop bc
            addPopAF();
            // pop de
            addPopDE();
            // push hl
            addPushHL();
            //   ex de,hl
            addExDEHL();
            // cast
            addCast(result[1], Lexeme::subtype_numeric);
            //   ex de,hl
            addExDEHL();
            // pop hl
            addPopHL();
            result[1] = Lexeme::subtype_numeric;
          } else
            result[1] = Lexeme::subtype_unknown;

          if (result[0] == Lexeme::subtype_numeric &&
              result[1] == Lexeme::subtype_numeric) {
            // ld a, l
            addLdAL();
            // ld b, e
            addLdBE();
            // ld hl, temporary string
            addTempStr(true);
            // call 0x7e4c    ; STRING$ (hl=destination, b=number of chars,
            // a=char)
            addCall(def_XBASIC_STRING);

            result[0] = Lexeme::subtype_string;

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "LEFT$") {
          if (result[0] == Lexeme::subtype_single_decimal ||
              result[0] == Lexeme::subtype_double_decimal) {
            // cast
            addCast(result[0], Lexeme::subtype_numeric);
            result[0] = Lexeme::subtype_numeric;
          }

          if (result[1] == Lexeme::subtype_string &&
              result[0] == Lexeme::subtype_numeric) {
            // ld a, l
            addLdAL();
            // pop hl
            addPopHL();

            // call 0x7d99     ; xbasic left string (in: a=size, hl=source; out:
            // hl=BUF)
            addCall(def_XBASIC_LEFT);
            // ld de, temporary string
            addTempStr(false);
            // push de
            addPushDE();
            //   call 0x7e9d   ; xbasic copy string (in: hl=source, de=dest;
            //   out: hl end of string)
            addCall(def_XBASIC_COPY_STRING);
            // pop hl
            addPopHL();

            result[0] = Lexeme::subtype_string;

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "RIGHT$") {
          if (result[0] == Lexeme::subtype_single_decimal ||
              result[0] == Lexeme::subtype_double_decimal) {
            // cast
            addCast(result[0], Lexeme::subtype_numeric);
            result[0] = Lexeme::subtype_numeric;
          }

          if (result[1] == Lexeme::subtype_string &&
              result[0] == Lexeme::subtype_numeric) {
            // ld a, l
            addLdAL();
            // pop hl
            addPopHL();

            // call 0x7da0     ; xbasic right string (in: a=size, hl=source;
            // out: hl=BUF)
            addCall(def_XBASIC_RIGHT);
            // ld de, temporary string
            addTempStr(false);
            // push de
            addPushDE();
            //   call 0x7e9d   ; xbasic copy string (in: hl=source, de=dest;
            //   out: hl end of string)
            addCall(def_XBASIC_COPY_STRING);
            // pop hl
            addPopHL();

            result[0] = Lexeme::subtype_string;

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "MID$") {
          if (result[0] == Lexeme::subtype_single_decimal ||
              result[0] == Lexeme::subtype_double_decimal) {
            // cast
            addCast(result[0], Lexeme::subtype_numeric);
            result[0] = Lexeme::subtype_numeric;
          }

          if (result[1] == Lexeme::subtype_string &&
              result[0] == Lexeme::subtype_numeric) {
            // ld b, l         ; start char
            addLdBL();
            // pop hl          ; source string
            addPopHL();

            // ld a, 0xff      ; number of chars (all left on source string)
            addLdA(0xff);

            // call 0x7db1     ; xbasic mid string (in: b=start, a=size,
            // hl=source; out: hl=BUF)
            addCall(def_XBASIC_MID);

            // ld de, temporary string
            addTempStr(false);
            // push de
            addPushDE();
            //   call 0x7e9d   ; xbasic copy string (in: hl=source, de=dest;
            //   out: hl end of string)
            addCall(def_XBASIC_COPY_STRING);
            // pop hl
            addPopHL();

            result[0] = Lexeme::subtype_string;

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "USING$") {
          if (result[0] == Lexeme::subtype_double_decimal ||
              result[0] == Lexeme::subtype_numeric) {
            // cast
            addCast(result[0], Lexeme::subtype_single_decimal);
            result[0] = Lexeme::subtype_single_decimal;
          }

          if (result[1] == Lexeme::subtype_string &&
              result[0] == Lexeme::subtype_single_decimal) {
            // ld c, b
            addLdCB();
            // ex de, hl
            addExDEHL();
            // pop hl
            addPopHL();

            // call XBASIC_USING    ; hl = item format string, c:de = float, out
            // hl=string
            addCall(def_XBASIC_USING);
            // ld de, temporary string
            addTempStr(false);
            // push de
            addPushDE();
            //   call 0x7e9d   ; xbasic copy string (in: hl=source, de=dest;
            //   out: hl end of string)
            addCall(def_XBASIC_COPY_STRING);
            // pop hl
            addPopHL();

            result[0] = Lexeme::subtype_string;

          } else if (result[1] == Lexeme::subtype_numeric &&
                     result[0] == Lexeme::subtype_single_decimal) {
            // pop de
            addPopDE();
            // push de
            addPushDE();
            //   ld a, e
            addLdAE();
            //   rrca
            addRRCA();
            //   rrca
            addRRCA();
            //   rrca
            addRRCA();
            //   rrca
            addRRCA();
            //   and 0x0F
            addAnd(0x0F);
            //   ld d, a
            addLdDA();
            //   ld a, e
            addLdAE();
            //   and 0x0F
            addAnd(0x0F);
            //   ld e, a
            addLdEA();
            // pop af
            addPopAF();

            // call XBASIC_USING_DO    ; a=format, d=thousand digits, e=decimal
            // digits, b:hl=number, out hl=string
            addCall(def_XBASIC_USING_DO);
            // ld de, temporary string
            addTempStr(false);
            // push de
            addPushDE();
            //   call 0x7e9d   ; xbasic copy string (in: hl=source, de=dest;
            //   out: hl end of string)
            addCall(def_XBASIC_COPY_STRING);
            // pop hl
            addPopHL();

            result[0] = Lexeme::subtype_string;

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "INSTR") {
          if (result[0] == Lexeme::subtype_string &&
              result[1] == Lexeme::subtype_string) {
            // ex de,hl        ; search string
            addExDEHL();
            // pop hl          ; source string
            addPopHL();

            // ld a, 0x01      ; search start
            addLdA(0x01);

            // call 0x7e6c     ; xbasic INSTR (in: a=start, hl=source,
            // de=search; out: hl=position)
            addCall(def_XBASIC_INSTR);

            result[0] = Lexeme::subtype_numeric;

          } else
            result[0] = Lexeme::subtype_unknown;

        } else
          result[0] = Lexeme::subtype_unknown;

      } break;

      case 3: {
        if (lexeme->value == "MID$") {
          if (result[0] == Lexeme::subtype_single_decimal ||
              result[0] == Lexeme::subtype_double_decimal) {
            // cast
            addCast(result[0], Lexeme::subtype_numeric);
            result[0] = Lexeme::subtype_numeric;
          }

          // ld a, l             ; number of chars
          addLdAL();

          if (result[1] == Lexeme::subtype_single_decimal ||
              result[1] == Lexeme::subtype_double_decimal) {
            // pop bc
            addPopBC();
            // pop hl
            addPopHL();
            // push af
            addPushAF();
            // cast
            addCast(result[1], Lexeme::subtype_numeric);
            // pop af
            addPopAF();
            result[1] = Lexeme::subtype_numeric;
          } else {
            // pop hl
            addPopHL();
          }

          if (result[2] == Lexeme::subtype_string &&
              result[1] == Lexeme::subtype_numeric &&
              result[0] == Lexeme::subtype_numeric) {
            // ld b, l         ; start char
            addLdBL();
            // pop hl          ; source string
            addPopHL();

            // call 0x7db1     ; xbasic mid string (in: b=start, a=size,
            // hl=source; out: hl=BUF)
            addCall(def_XBASIC_MID);

            // ld de, temporary string
            addTempStr(false);
            // push de
            addPushDE();
            //   call 0x7e9d   ; xbasic copy string (in: hl=source, de=dest;
            //   out: hl end of string)
            addCall(def_XBASIC_COPY_STRING);
            // pop hl
            addPopHL();

            result[0] = Lexeme::subtype_string;

          } else
            result[0] = Lexeme::subtype_unknown;

        } else if (lexeme->value == "INSTR") {
          // ex de,hl        ; search string
          addExDEHL();
          // pop hl          ; source string
          addPopHL();
          // pop bc          ; search start
          addPopBC();

          if (result[2] == Lexeme::subtype_single_decimal ||
              result[2] == Lexeme::subtype_double_decimal) {
            // ex (sp),hl
            addExiSPHL();
            // push de
            addPushDE();
            //   cast
            addCast(result[2], Lexeme::subtype_numeric);
            //   ld c, l
            addLdCL();
            // pop de
            addPopDE();
            // pop hl
            addPopHL();
            result[2] = Lexeme::subtype_numeric;
          }

          // ld a, c             ; search start
          addLdAC();

          if (result[0] == Lexeme::subtype_string &&
              result[1] == Lexeme::subtype_string &&
              result[2] == Lexeme::subtype_numeric) {
            // call 0x7e6c     ; xbasic INSTR (in: a=start, hl=source,
            // de=search; out: hl=position)
            addCall(def_XBASIC_INSTR);

            result[0] = Lexeme::subtype_numeric;

          } else
            result[0] = Lexeme::subtype_unknown;

        } else
          result[0] = Lexeme::subtype_unknown;

      } break;
    }
  }

  return result[0];
}

bool Compiler::addVarAddress(ActionNode* action) {
  Lexeme *lexeme, *lexeme1, *lexeme2;
  ActionNode *action1, *action2;
  unsigned int i, t;
  int factor, diff;
  int result_subtype;
  bool first;

  lexeme = action->lexeme;
  t = action->actions.size();

  if (lexeme->isArray) {
    switch (t) {
      case 0: {
        syntaxError("Array index is missing");
        return false;
      } break;

      case 1: {
        action1 = action->actions[0];
        lexeme1 = action1->lexeme;

        if (lexeme1->type == Lexeme::type_literal) {
          if (lexeme1->subtype == Lexeme::subtype_numeric ||
              lexeme1->subtype == Lexeme::subtype_single_decimal ||
              lexeme1->subtype == Lexeme::subtype_double_decimal) {
            try {
              i = stoi(lexeme1->value) * lexeme->x_factor;
            } catch (exception& e) {
              printf("Warning: error while converting numeric constant %s\n",
                     lexeme1->value.c_str());
              i = 0;
            }

            // ld hl, variable_address +  (x_index * x_factor)
            addFix(lexeme)->step = i;
            addLdHL(0x0000);

          } else {
            syntaxError("Invalid array index type");
          }

        } else {
          // ld hl, x index
          result_subtype = evalExpression(action1);
          addCast(result_subtype, Lexeme::subtype_numeric);

          if (lexeme->x_factor == 2) {
            // add hl, hl
            addAddHLHL();
          } else if (lexeme->x_factor == 3) {
            // ld d, h
            addLdDH();
            // ld e, l
            addLdEL();
            // add hl, hl
            addAddHLHL();
            // add hl, de
            addAddHLDE();
          } else if (lexeme->x_factor == 256) {
            // ld h, l
            addLdHL();
            // ld l, 0
            addLdL(0x00);
          } else {
            // ld de, x_factor
            addLdDE(lexeme->x_factor);

            // call 0x761b    ; integer multiplication (hl = hl * de)
            addCall(def_XBASIC_MULTIPLY_INTEGERS);
          }

          // ld de, variable
          addFix(lexeme);
          addLdDE(0x0000);

          // add hl, de   ; hl = variable_address +  (x_index * x_factor)
          addAddHLDE();
        }

      } break;

      case 2: {
        action1 = action->actions[1];
        lexeme1 = action1->lexeme;

        action2 = action->actions[0];
        lexeme2 = action2->lexeme;

        if (lexeme1->type == Lexeme::type_literal &&
            lexeme2->type == Lexeme::type_literal) {
          if (lexeme1->subtype == Lexeme::subtype_numeric ||
              lexeme1->subtype == Lexeme::subtype_single_decimal ||
              lexeme1->subtype == Lexeme::subtype_double_decimal) {
            try {
              i = stoi(lexeme1->value) * lexeme->x_factor;
            } catch (exception& e) {
              printf("Warning: error while converting numeric constant %s\n",
                     lexeme1->value.c_str());
              i = 0;
            }

            if (lexeme2->subtype == Lexeme::subtype_numeric ||
                lexeme2->subtype == Lexeme::subtype_single_decimal ||
                lexeme2->subtype == Lexeme::subtype_double_decimal) {
              try {
                i += stoi(lexeme2->value) * lexeme->y_factor;
              } catch (exception& e) {
                printf("Warning: error while converting numeric constant %s\n",
                       lexeme2->value.c_str());
              }

              // ld hl, variable_address +  (x_index * x_factor) + (y_index *
              // y_factor)
              addFix(lexeme)->step = i;
              addLdHL(0x0000);

            } else {
              syntaxError("Invalid array 2nd index type");
            }

          } else {
            syntaxError("Invalid array 1st index type");
          }

        } else {
          // ld hl, x index
          result_subtype = evalExpression(action1);
          addCast(result_subtype, Lexeme::subtype_numeric);

          if (lexeme->x_factor == 2) {
            // add hl, hl
            addAddHLHL();
          } else if (lexeme->x_factor == 3) {
            // ld d, h
            addLdDH();
            // ld e, l
            addLdEL();
            // add hl, hl
            addAddHLHL();
            // add hl, de
            addAddHLDE();
          } else if (lexeme->x_factor == 256) {
            // ld h, l
            addLdHL();
            // ld l, 0
            addLdL(0x00);
          } else {
            // ld de, x_factor
            addLdDE(lexeme->x_factor);

            // call 0x761b    ; integer multiplication (hl = hl * de)
            addCall(def_XBASIC_MULTIPLY_INTEGERS);
          }

          // push hl
          addPushHL();

          // ld hl, y index
          result_subtype = evalExpression(action2);
          addCast(result_subtype, Lexeme::subtype_numeric);

          // calculate y factor * index
          // ---------- old code
          // ld de, y_factor
          // addLdDEnn(lexeme->y_factor);
          // call 0x761b    ; integer multiplication (hl = hl * de)
          // addCall(def_XBASIC_MULTIPLY_INTEGERS);
          //--------------------

          factor = lexeme->y_factor;
          if (lexeme->x_factor == 2) {
            factor >>= 1;
          }

          first = true;

          // verify if factor is power of 2
          diff = 1;
          while (factor >= (diff << 1)) {
            diff <<= 1;  // diff *= 2;
          }
          if (diff != factor) {
            // ld b, h           ; save index in bc
            addLdBH();
            // ld c, l
            addLdCL();
          }

          while (factor) {
            if (!first) {
              // ex de, hl     ; save current total
              addExDEHL();
              // ld h, b       ; restore index from bc
              addLdHB();
              // ld l, c
              addLdLC();
            }

            diff = 1;

            while (factor >= (diff << 1)) {
              // add hl, hl    ; x 2
              addAddHLHL();

              diff <<= 1;  // diff *= 2;
            }

            if (first) {
              first = false;
            } else {
              // add hl, de    ; add last total
              addAddHLDE();
            }

            factor -= diff;
          }

          if (lexeme->x_factor == 2) {
            // add hl, hl    ; x 2
            addAddHLHL();
          }

          // pop de
          addPopDE();

          // add hl, de     ; hl = (x_index * x_factor) + (y_index * y_factor)
          addAddHLDE();

          // ld de, variable
          addFix(lexeme);
          addLdDE(0x0000);

          // add hl, de    ; hl += variable_adress
          addAddHLDE();
        }

      } break;

      default: {
        syntaxError("Wrong array parameters count");
        return false;
      }
    }

  } else {
    if (t) {
      syntaxError("Undeclared array or unknown function");
      return false;
    } else {
      // ld hl, variable
      addFix(lexeme);
      addLdHL(0x0000);
    }
  }

  return true;
}

void Compiler::addTempStr(bool atHL) {
  if (atHL) {
    // call GET_NEXT_TEMP_STRING_ADDRESS
    addCall(def_GET_NEXT_TEMP_STRING_ADDRESS);
  } else {
    // ex de, hl
    addExDEHL();
    // call GET_NEXT_TEMP_STRING_ADDRESS
    addCall(def_GET_NEXT_TEMP_STRING_ADDRESS);
    // ex de, hl
    addByteOptimized(0xEB);
  }
}

void Compiler::addCast(int from, int to) {
  if (from != to) {
    if (from == Lexeme::subtype_numeric) {
      if (to == Lexeme::subtype_numeric) {
        return;
      } else if (to == Lexeme::subtype_string) {
        // call 0x7b26   ; xbasic int to string (in hl, out hl)
        addCall(def_XBASIC_CAST_INTEGER_TO_STRING);
      } else if (to == Lexeme::subtype_single_decimal ||
                 to == Lexeme::subtype_double_decimal) {
        // call 0x782d   ; xbasic int to float (in hl, out b:hl)
        addCall(def_XBASIC_CAST_INTEGER_TO_FLOAT);
      }

    } else if (from == Lexeme::subtype_string) {
      if (to == Lexeme::subtype_numeric) {
        // call 0x7e07   ; VAL function - xbasic string to float (in hl, out
        // b:hl)
        addCall(def_XBASIC_CAST_STRING_TO_FLOAT);
        // call 0x784f   ; xbasic float to integer (in b:hl, out hl)
        addCall(def_XBASIC_CAST_FLOAT_TO_INTEGER);
      } else if (to == Lexeme::subtype_string) {
        return;
      } else if (to == Lexeme::subtype_single_decimal ||
                 to == Lexeme::subtype_double_decimal) {
        // call 0x7e07   ; VAL function - xbasic string to float (in hl, out
        // b:hl)
        addCall(def_XBASIC_CAST_STRING_TO_FLOAT);
      }

    } else if (from == Lexeme::subtype_single_decimal ||
               from == Lexeme::subtype_double_decimal) {
      if (to == Lexeme::subtype_numeric) {
        // call 0x784f   ; xbasic float to integer (in b:hl, out hl)
        addCall(def_XBASIC_CAST_FLOAT_TO_INTEGER);
      } else if (to == Lexeme::subtype_string) {
        // call 0x7b80   ; xbasic float to string (in b:hl, out hl)
        addCall(def_XBASIC_CAST_FLOAT_TO_STRING);
      } else if (to == Lexeme::subtype_single_decimal ||
                 to == Lexeme::subtype_double_decimal) {
        return;
      }

    } else {
      syntaxError("Unknown type to cast");
    }
  }
}

//-------------------------------------------------------------------------------------------
