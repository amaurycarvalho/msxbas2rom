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
  this->opts = parser->opts;

  clearSymbols();

  t = parser->tags.size();
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
    tag = parser->tags[i];

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

    if (parser->has_idata) {
      if (opts->debug) printf("Registering IDATA resource...");
      resourceManager.addIDataResource(parser);
    }

    if (parser->has_data) {
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

  if (parser->has_font) {
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
        symbol->lexeme->print();
        syntaxError("Symbol reference not found: variable or constant");
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
  bool traps_checked = false;

  if (action) {
    current_action = action;
    lexeme = action->lexeme;

    if (lexeme->type == Lexeme::type_keyword) {
      if (lexeme->name == "END") {
        traps_checked = addCheckTraps();
        cmd_end(false);  //! jump to the real END statement
      } else if (lexeme->name == "REM" || lexeme->name == "'") {
        return true;
      } else if (lexeme->name == "CLEAR") {
        /// @remark issue #11
        cmd_clear();
      } else if (lexeme->name == "DEF" || lexeme->name == "DEFINT" ||
                 lexeme->name == "DEFSNG" || lexeme->name == "DEFDBL" ||
                 lexeme->name == "DEFSTR") {
        cmd_def();
      } else if (lexeme->name == "CLS") {
        cmd_cls();
      } else if (lexeme->name == "BEEP") {
        cmd_beep();
      } else if (lexeme->name == "PRINT") {
        cmd_print();
      } else if (lexeme->name == "INPUT") {
        cmd_input(true);
      } else if (lexeme->name == "GOTO") {
        traps_checked = addCheckTraps();
        cmd_goto();
      } else if (lexeme->name == "GOSUB") {
        traps_checked = addCheckTraps();
        cmd_gosub();
      } else if (lexeme->name == "RETURN") {
        traps_checked = addCheckTraps();
        cmd_return();
      } else if (lexeme->name == "SOUND") {
        cmd_sound();
      } else if (lexeme->name == "OUT") {
        cmd_out();
      } else if (lexeme->name == "POKE") {
        cmd_poke();
      } else if (lexeme->name == "VPOKE") {
        cmd_vpoke();
      } else if (lexeme->name == "IPOKE") {
        cmd_ipoke();
      } else if (lexeme->name == "PLAY") {
        cmd_play();
      } else if (lexeme->name == "DRAW") {
        cmd_draw();
      } else if (lexeme->name == "LET") {
        cmd_let();
      } else if (lexeme->name == "IF") {
        traps_checked = addCheckTraps();
        cmd_if();
      } else if (lexeme->name == "FOR") {
        cmd_for();
      } else if (lexeme->name == "NEXT") {
        traps_checked = addCheckTraps();
        cmd_next();
      } else if (lexeme->name == "TIME") {
        cmd_let();
      } else if (lexeme->name == "DIM") {
        cmd_dim();
      } else if (lexeme->name == "REDIM") {
        cmd_redim();
      } else if (lexeme->name == "RANDOMIZE") {
        cmd_randomize();
      } else if (lexeme->name == "LOCATE") {
        cmd_locate();
      } else if (lexeme->name == "SCREEN") {
        cmd_screen();
      } else if (lexeme->name == "WIDTH") {
        cmd_width();
      } else if (lexeme->name == "COLOR") {
        cmd_color();
      } else if (lexeme->name == "PSET") {
        cmd_pset(true);
      } else if (lexeme->name == "PRESET") {
        cmd_pset(false);
      } else if (lexeme->name == "LINE") {
        cmd_line();
      } else if (lexeme->name == "PAINT") {
        cmd_paint();
      } else if (lexeme->name == "CIRCLE") {
        cmd_circle();
      } else if (lexeme->name == "COPY") {
        cmd_copy();
      } else if (lexeme->name == "PUT") {
        cmd_put();
      } else if (lexeme->name == "DATA") {
        cmd_data();
      } else if (lexeme->name == "IDATA") {
        cmd_idata();
      } else if (lexeme->name == "READ") {
        cmd_read();
      } else if (lexeme->name == "IREAD") {
        cmd_iread();
      } else if (lexeme->name == "RESTORE") {
        cmd_restore();
      } else if (lexeme->name == "IRESTORE") {
        cmd_irestore();
      } else if (lexeme->name == "RESUME") {
        cmd_resume();
      } else if (lexeme->name == "GET") {
        cmd_get();
      } else if (lexeme->name == "SET") {
        cmd_set();
      } else if (lexeme->name == "ON") {
        traps_checked = addCheckTraps();
        cmd_on();
      } else if (lexeme->name == "INTERVAL") {
        cmd_interval();
      } else if (lexeme->name == "KEY") {
        cmd_key();
      } else if (lexeme->name == "STRIG") {
        cmd_strig();
      } else if (lexeme->name == "SPRITE") {
        cmd_sprite();
      } else if (lexeme->name == "STOP") {
        traps_checked = addCheckTraps();
        cmd_stop();
      } else if (lexeme->name == "WAIT") {
        cmd_wait();
      } else if (lexeme->name == "SWAP") {
        cmd_swap();
      } else if (lexeme->name == "CALL") {
        cmd_call();
      } else if (lexeme->name == "CMD") {
        cmd_cmd();
      } else if (lexeme->name == "MAXFILES") {
        cmd_maxfiles();
      } else if (lexeme->name == "OPEN") {
        cmd_open();
      } else if (lexeme->name == "OPEN_GRP") {
        has_open_grp = true;
      } else if (lexeme->name == "CLOSE") {
        cmd_close();
      } else if (lexeme->name == "FILE") {
        cmd_file();
      } else if (lexeme->name == "TEXT") {
        cmd_text();
      } else if (lexeme->name == "BLOAD") {
        cmd_bload();
      } else {
        syntaxError();
        return false;
      }

      if (!traps_checked) {
        addCheckTraps();
      }

    } else {
      syntaxError();
      return false;
    }

  } else {
    syntaxError();
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

void Compiler::cmd_start() {
  // ld (SAVSTK), sp
  addLdiiSP(0xf6b1);

  // ld a, (SLTSTR)     ; start slot
  addLdAii(def_SLTSTR);

  // ld h, 0x40
  addLdH(0x40);

  // call ENASLT        ; enable xbasic page
  addCall(def_ENASLT);

  // ld hl, HEAP START ADDRESS
  addFix(heap_mark);
  addLdHL(0x0000);

  // ld de, TEMPORARY STRING START ADDRESS
  addFix(temp_str_mark);
  addLdDE(0x0000);

  if (parser->has_font) {
    // ld ix, FONT BUFFER START ADDRESS
    addByte(0xDD);
    addFix(heap_mark)->step = -def_RAM_BUFSIZ;
    addLdHL(0x0000);
  } else {
    // push hl
    addPushHL();
    // pop ix
    addPopIX();
  }

  // call XBASIC INIT                  ; hl=heap start address, de=temporary
  // string start address, bc=data address, ix=font address, a=data segment
  addCall(def_XBASIC_INIT);

  if (parser->has_traps) {
    if (opts->megaROM) {
      // ld a, 0xFF
      addLdA(0xFF);
    } else {
      // xor a
      addXorA();
    }
    // ld (0xFC82), a          ; start of TRPTBL reserved area (megaROM flag to
    // traps)
    addLdiiA(def_MR_TRAP_FLAG);
  }

  if (opts->megaROM) {
    // ld a, 2
    addLdA(0x02);
    // call MR_CHANGE_SGM
    addCall(def_MR_CHANGE_SGM);
  }

  if (parser->has_data || parser->has_idata) {
    // ld hl, data resource number
    addLdHL(parser->resourceCount);
    // ld (DAC), hl
    addLdiiHL(def_DAC);
    // call cmd_restore
    addCall(def_cmd_restore);  // MSXBAS2ROM resource RESTORE statement
  }

  if (parser->has_akm) {
    // initialize AKM player
    addCall(def_player_initialize);
  }

  // ei
  addEI();
}

void Compiler::cmd_end(bool doCodeRegistering) {
  if (doCodeRegistering) {
    /// @remark first instruction needs to be a skip to the program start code
    if (parser->has_akm) {
      addJr(1 + 3 + 10);
    } else
      addJr(1 + 10);

    /// mark the END statement start code
    end_mark = addPreMark();
    end_mark->address = code_pointer;

    /// write the END statement code
    if (parser->has_akm) {
      // disable AKM player
      addCall(def_player_unhook);
    }

    // call XBASIC_END
    addCall(def_XBASIC_END);

    // ld sp, (SAVSTK)
    addLdSPii(0xf6b1);

    // ld hl, fake empty line
    addLdHL(def_ENDPRG);

    // ret               ; return to basic
    addRet();

  } else {
    /// jump to the real END statement
    // jp end_mark
    if (end_mark) {
      addFix(end_mark);
      addJp(0x0000);
    }
  }
}

void Compiler::cmd_cls() {
  addCall(def_XBASIC_CLS);  // call cls
}

void Compiler::cmd_clear() {
  /// clear variables workarea
  addXorA();                //! A = 0
  addLdHLii(def_HEAPSTR);   //! HL = heap start address
  addLdDE(def_RAM_BOTTOM);  //! DE = variables start address
  addSbcHLDE();             //! HL -= DE
  addLdCL();
  addLdBH();  //! BC = HL (variables workarea size)
  addLdLE();
  addLdHD();    //! HL = DE
  addIncDE();   //! DE = HL + 1
  addLdiHLA();  //! (HL) = A
  addLDIR();    //! (DE++) = (HL++), until BC-- = 0
}

void Compiler::cmd_beep() {
  addCall(0x00c0);  // call beep
}

void Compiler::cmd_randomize() {
  // ld hl, 0x3579      ; RANDOMIZE 1 - FIX
  addLdHL(0x3579);
  // ld (0xF7BC+0), hl  ; SWPTMP+0
  addLdiiHL(0xF7BC);

  // ld hl, (0xFC9E)    ; RANDOMIZE 2 - JIFFY
  addLdHLii(0xFC9E);
  // ld (0xF7BC+2), hl  ; SWPTMP+2
  addLdiiHL(0xF7BE);
}

void Compiler::cmd_goto() {
  Lexeme* lexeme;

  if (current_action->actions.size() == 1) {
    lexeme = current_action->actions[0]->lexeme;
    if (lexeme) {
      if (lexeme->type == Lexeme::type_literal &&
          lexeme->subtype == Lexeme::subtype_numeric) {
        // Trim leading zeros
        while (lexeme->value.find("0") == 0 && lexeme->value.size() > 1) {
          lexeme->value.erase(0, 1);
        }

        // jp address
        addFix(lexeme->value);
        addJp(0x0000);
        return;
      }
    }
  }

  syntaxError("Invalid GOTO parameters");
}

void Compiler::cmd_gosub() {
  Lexeme* lexeme;

  if (current_action->actions.size() == 1) {
    lexeme = current_action->actions[0]->lexeme;
    if (lexeme) {
      if (lexeme->type == Lexeme::type_literal &&
          lexeme->subtype == Lexeme::subtype_numeric) {
        // Trim leading zeros
        while (lexeme->value.find("0") == 0 && lexeme->value.size() > 1) {
          lexeme->value.erase(0, 1);
        }

        // call address
        addFix(lexeme->value);
        addCall(0x0000);
        return;
      }
    }
  }

  syntaxError("Invalid GOSUB parameters");
}

void Compiler::cmd_return() {
  Lexeme* lexeme;
  int t = current_action->actions.size();

  if (t == 0) {
    // ret
    addRet();
    return;

  } else if (t == 1) {
    lexeme = current_action->actions[0]->lexeme;
    if (lexeme) {
      if (lexeme->type == Lexeme::type_literal &&
          lexeme->subtype == Lexeme::subtype_numeric) {
        // Trim leading zeros
        while (lexeme->value.find("0") == 0 && lexeme->value.size() > 1) {
          lexeme->value.erase(0, 1);
        }

        if (opts->megaROM) {
          // pop bc           ; delete old return segment/address
          addPopBC();
          // pop de           ; delete old return segment/address
          addPopDE();
          // ld a, 0x48       ; verify if running on trap (MR_CALL_TRAP)
          addLdA(0x48);
          // cp b
          addCpB();
          // jp nz, address   ; if not, jump to new address and segment
          addFix(lexeme->value);
          addJpNZ(0x0000);

          // pop de           ; fix trap return control
          addPopDE();
          // pop hl
          addPopHL();
          // exx
          addExx();
          // special ld hl, address
          addFix(lexeme->value);
          addLdHLmegarom();
          // ex (sp), hl      ; new return address
          addExiSPHL();
          // exx
          addExx();
          // push hl
          addPushHL();
          // push de
          addPushDE();
          // push af          ; new return segment
          addPushAF();
          // push bc          ; trap return
          addPushBC();
          // ret
          addRet();

        } else {
          // pop bc           ; delete old return address
          addPopBC();
          // ld a, 0x6C       ; verify if running on trap
          addLdA(0x6C);
          // cp b
          addCpB();
          // jp nz, address   ; jump to new address
          addFix(lexeme->value);
          addJpNZ(0x0000);

          // pop de           ; fix trap return control
          addPopDE();
          // ld hl, address
          addFix(lexeme->value);
          addLdHL(0x0000);
          // ex (sp), hl
          addExiSPHL();
          // push de
          addPushDE();
          // push bc
          addPushBC();
          // ret
          addRet();
        }

        return;
      }
    }
  }

  syntaxError("Invalid RETURN parameters");
}

void Compiler::cmd_print() {
  Lexeme *lexeme, *last_lexeme = 0;
  ActionNode *action, *subaction;
  unsigned int i, t = current_action->actions.size();
  int result_subtype;
  bool redirected = false;

  if (t) {
    for (i = 0; i < t; i++) {
      action = current_action->actions[i];
      lexeme = action->lexeme;
      last_lexeme = lexeme;

      if (lexeme) {
        if (lexeme->type == Lexeme::type_separator) {
          if (lexeme->value == ",") {
            addCall(def_XBASIC_PRINT_TAB);  // call print_tab
          } else if (lexeme->value == ";") {
            continue;
          } else if (lexeme->value == "#") {
            if (has_open_grp) continue;

            redirected = true;
            subaction = action->actions[0];
            result_subtype = evalExpression(subaction);
            addCast(result_subtype, Lexeme::subtype_numeric);

            // call io redirect
            if (io_redirect_mark)
              addFix(io_redirect_mark->symbol);
            else
              io_redirect_mark = addMark();
            addCall(0x0000);

            continue;
          } else {
            syntaxError("Invalid PRINT parameter separator");
            return;
          }
        } else {
          result_subtype = evalExpression(action);

          if (result_subtype == Lexeme::subtype_string) {
            addCall(def_XBASIC_PRINT_STR);  // call print_str

          } else if (result_subtype == Lexeme::subtype_numeric) {
            addCall(def_XBASIC_PRINT_INT);  // call print_int

          } else if (result_subtype == Lexeme::subtype_single_decimal ||
                     result_subtype == Lexeme::subtype_double_decimal) {
            addCall(def_XBASIC_PRINT_FLOAT);  // call print_float

          } else {
            syntaxError("Invalid PRINT parameter");
            return;
          }
        }
      }
    }

  } else {
    addCall(def_XBASIC_PRINT_CRLF);  // call print_crlf
  }

  if (last_lexeme) {
    if (last_lexeme->type != Lexeme::type_separator ||
        (last_lexeme->value != ";" && last_lexeme->value != ",")) {
      addCall(def_XBASIC_PRINT_CRLF);  // call print_crlf
    }
  }

  if (redirected) {
    // call io screen
    if (io_screen_mark)
      addFix(io_screen_mark->symbol);
    else
      io_screen_mark = addMark();
    addCall(0x0000);
  }
}

void Compiler::cmd_input(bool question) {
  Lexeme* lexeme;
  ActionNode *action, *subaction;
  unsigned int i, t = current_action->actions.size();
  int result_subtype;
  bool redirected = false;

  if (t) {
    for (i = 0; i < t; i++) {
      action = current_action->actions[i];
      lexeme = action->lexeme;
      if (lexeme) {
        if (lexeme->type == Lexeme::type_separator) {
          if (lexeme->value == ",") {
            addCall(def_XBASIC_PRINT_TAB);  // call print_tab
          } else if (lexeme->value == ";") {
            continue;
          } else if (lexeme->value == "#") {
            redirected = true;
            subaction = action->actions[0];
            result_subtype = evalExpression(subaction);
            addCast(result_subtype, Lexeme::subtype_numeric);

            // call io redirect
            if (io_redirect_mark)
              addFix(io_redirect_mark->symbol);
            else
              io_redirect_mark = addMark();
            addCall(0x0000);

            continue;
          } else {
            syntaxError("Invalid INPUT parameter separator");
            return;
          }
        } else {
          if (lexeme->type == Lexeme::type_identifier) {
            // call INPUT or LINE INPUT

            if (question) {
              addCall(def_XBASIC_INPUT_1);
            } else {
              addCall(def_XBASIC_INPUT_2);
            }

            // do assignment

            addCast(Lexeme::subtype_string, lexeme->subtype);

            if (!addAssignment(action)) return;

          } else {
            result_subtype = evalExpression(action);

            if (result_subtype == Lexeme::subtype_string) {
              addCall(def_XBASIC_PRINT_STR);  // call print_str

            } else if (result_subtype == Lexeme::subtype_numeric) {
              addCall(def_XBASIC_PRINT_INT);  // call print_int

            } else if (result_subtype == Lexeme::subtype_single_decimal ||
                       result_subtype == Lexeme::subtype_double_decimal) {
              addCall(def_XBASIC_PRINT_FLOAT);  // call print_float

            } else {
              syntaxError("Invalid INPUT parameter");
              return;
            }
          }
        }
      }
    }

    if (redirected) {
      // call io screen
      if (io_screen_mark)
        addFix(io_screen_mark->symbol);
      else
        io_screen_mark = addMark();
      addCall(0x0000);
    }

  } else {
    syntaxError();
  }
}

void Compiler::cmd_line() {
  Lexeme* lexeme;
  ActionNode *action, *sub_action;
  unsigned int i, t = current_action->actions.size();
  int line_type = 0, result_subtype, state;
  bool has_x0_coord = false, has_x1_coord = false, has_y0_coord = false,
       has_y1_coord = false;
  bool has_color = false, has_line_type = false, has_operator = false;

  if (t) {
    action = current_action->actions[0];
    lexeme = action->lexeme;
    if (lexeme->type == Lexeme::type_keyword && lexeme->value == "INPUT") {
      current_action = action;
      return cmd_input(false);
    }

    state = 0;

    for (i = 0; i < t; i++) {
      action = current_action->actions[i];

      switch (state) {
        case 0: {
          if (action->actions.size() != 2) {
            syntaxError("Coordenates parameters error on LINE");
            return;
          } else if (action->lexeme->value == "COORD") {
            sub_action = action->actions[0];

            result_subtype = evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
              // ld hl, (0xFCB7)  ;GRPACX
              addLdHLii(0xFCB7);
              // push hl
              addPushHL();
              has_x0_coord = true;
            } else {
              addCast(result_subtype, Lexeme::subtype_numeric);

              // ld (0xFCB7), hl  ;GRPACX
              addLdiiHL(0xFCB7);
              // push hl
              addPushHL();
              has_x0_coord = true;
            }

            sub_action = action->actions[1];

            result_subtype = evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
              // ld hl, (0xFCB9)  ;GRPACY
              addLdHLii(0xFCB9);
              // push hl
              addPushHL();
              has_y0_coord = true;
            } else {
              addCast(result_subtype, Lexeme::subtype_numeric);

              // ld (0xFCB9), hl  ;GRPACY
              addLdiiHL(0xFCB9);
              // push hl
              addPushHL();
              has_y0_coord = true;
            }

          } else if (action->lexeme->value == "STEP") {
            sub_action = action->actions[0];

            result_subtype = evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
              // ld hl, (0xFCB7)  ;GRPACX
              addLdHLii(0xFCB7);
              // push hl
              addPushHL();
              has_x0_coord = true;
            } else {
              addCast(result_subtype, Lexeme::subtype_numeric);
              // ld de, (0xFCB7)  ;GRPACX
              addLdDEii(0xFCB7);
              // add hl, de
              addAddHLDE();
              // ld (0xFCB7), hl  ;GRPACX
              addLdiiHL(0xFCB7);
              // push hl
              addPushHL();
              has_x0_coord = true;
            }

            sub_action = action->actions[1];

            result_subtype = evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
              // ld hl, (0xFCB9)  ;GRPACY
              addLdHLii(0xFCB9);
              // push hl
              addPushHL();
              has_y0_coord = true;
            } else {
              addCast(result_subtype, Lexeme::subtype_numeric);
              // ld de, (0xFCB9)  ;GRPACY
              addLdDEii(0xFCB9);
              // add hl, de
              addAddHLDE();
              // ld (0xFCB9), hl  ;GRPACY
              addLdiiHL(0xFCB9);
              // push hl
              addPushHL();
              has_y0_coord = true;
            }

          } else if (action->lexeme->value == "TO_COORD") {
            state++;

            if (!has_x0_coord) {
              // ex de,hl
              addExDEHL();
              // ld hl, (0xFCB7)  ;GRPACX
              addLdHLii(0xFCB7);
              // push hl
              addPushHL();
              // ld hl, (0xFCB9)  ;GRPACY
              addLdHLii(0xFCB9);
              // push hl
              addPushHL();
              // ex de,hl
              addExDEHL();
              has_x0_coord = true;
              has_y0_coord = true;
            }

            sub_action = action->actions[0];

            result_subtype = evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
            } else {
              addCast(result_subtype, Lexeme::subtype_numeric);

              // ld (0xFCB7), hl  ;GRPACX
              addLdiiHL(0xFCB7);
              // push hl
              addPushHL();
              has_x1_coord = true;
            }

            sub_action = action->actions[1];

            result_subtype = evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
            } else {
              addCast(result_subtype, Lexeme::subtype_numeric);

              // ld (0xFCB9), hl  ;GRPACY
              addLdiiHL(0xFCB9);
              // push hl
              addPushHL();
              has_y1_coord = true;
            }

          } else if (action->lexeme->value == "TO_STEP") {
            state++;

            if (!has_x0_coord) {
              // ex de,hl
              addExDEHL();
              // ld hl, (0xFCB7)  ;GRPACX
              addLdHLii(0xFCB7);
              // push hl
              addPushHL();
              // ld hl, (0xFCB9)  ;GRPACY
              addLdHLii(0xFCB9);
              // push hl
              addPushHL();
              // ex de,hl
              addExDEHL();
              has_x0_coord = true;
              has_y0_coord = true;
            }

            sub_action = action->actions[0];

            result_subtype = evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
            } else {
              addCast(result_subtype, Lexeme::subtype_numeric);
              // ld de, (0xFCB7)  ;GRPACX
              addLdDEii(0xFCB7);
              // add hl, de
              addAddHLDE();
              // ld (0xFCB7), hl  ;GRPACX
              addLdiiHL(0xFCB7);
              // push hl
              addPushHL();
              has_x1_coord = true;
            }

            sub_action = action->actions[1];

            result_subtype = evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
            } else {
              addCast(result_subtype, Lexeme::subtype_numeric);
              // ld de, (0xFCB9)  ;GRPACY
              addLdDEii(0xFCB9);
              // add hl, de
              addAddHLDE();
              // ld (0xFCB9), hl  ;GRPACY
              addLdiiHL(0xFCB9);
              // push hl
              addPushHL();
              has_y1_coord = true;
            }

          } else {
            syntaxError("Invalid coordenates on LINE");
            return;
          }

        } break;

        case 1: {
          state++;

          result_subtype = evalExpression(action);

          if (result_subtype == Lexeme::subtype_null) continue;

          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld a, l
          addLdAL();

          has_color = true;

        } break;

        case 2: {
          state++;

          lexeme = action->lexeme;
          if (lexeme->subtype == Lexeme::subtype_null) continue;

          if (lexeme->type == Lexeme::type_literal) {
            try {
              line_type = stoi(lexeme->value);
            } catch (exception& e) {
              printf("Warning: error while converting numeric constant %s\n",
                     lexeme->value.c_str());
              line_type = 0;
            }
          } else {
            syntaxError("Invalid shape parameter");
          }

          has_line_type = true;

        } break;

        case 3: {
          state++;

          result_subtype = evalExpression(action);

          if (result_subtype == Lexeme::subtype_null) continue;

          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld b, l
          addLdBL();

          has_operator = true;

        } break;

        default: {
          syntaxError("LINE parameters not supported");
          return;
        }
      }
    }

    if (!has_color) {
      // ld a, (0xF3E9)       ; FORCLR
      addLdAii(0xF3E9);
    }

    if (!has_operator) {
      // ld b, 0
      addLdB(0x00);
    }

    if (has_y1_coord) {
      // pop hl
      addPopHL();
    } else {
      // ld hl, (0xFCB9)  ;GRPACY
      addLdHLii(0xFCB9);
    }

    if (has_x1_coord) {
      // pop de
      addPopDE();
    } else {
      // ld de, (0xFCB7)  ;GRPACX
      addLdDEii(0xFCB7);
    }

    if (has_y0_coord) {
      // pop iy
      addPopIY();
    } else {
      // ld iy, (0xFCB9)  ;GRPACY
      addLdIYii(0xFCB9);
    }

    if (has_x0_coord) {
      // pop ix
      addPopIX();
    } else {
      // ld ix, (0xFCB7)  ;GRPACX
      addLdIXii(0xFCB7);
    }

    if (line_type == 0 || !has_line_type) {
      // call 0x6DA7   ; xbasic LINE (in: ix=x0, iy=y0, de=x1, hl=y1, a=color,
      // b=operator)
      addCall(def_XBASIC_LINE);
    } else if (line_type == 1) {
      // call 0x6D49   ; xbasic BOX (in: ix=x0, iy=y0, de=x1, hl=y1, a=color,
      // b=operator)
      addCall(def_XBASIC_BOX);
    } else {
      // call 0x6E27   ; xbasic BOX FILLED (in: ix=x0, iy=y0, de=x1, hl=y1,
      // a=color, b=operator)
      addCall(def_XBASIC_BOXF);
    }

  } else {
    syntaxError("LINE with empty parameters");
  }
}

void Compiler::cmd_copy() {
  ActionNode *action, *sub_action;
  Lexeme* lexeme;
  unsigned int i, t = current_action->actions.size();
  int result_subtype, state;
  bool has_x0_coord = false, has_x1_coord = false, has_x2_coord = false;
  bool has_y0_coord = false, has_y1_coord = false, has_y2_coord = false;
  bool has_src_page = false, has_dest_page = false, has_operator = false;
  bool has_address_from = false, has_address_to = false;

  if (t) {
    // action = current_action->actions[0];

    state = 0;

    for (i = 0; i < t; i++) {
      action = current_action->actions[i];

      switch (state) {
        case 0: {
          if (action->lexeme->value == "SCREEN") {
            return cmd_copy_screen();
          } else if (action->lexeme->value == "COORD") {
            if (action->actions.size() != 2) {
              syntaxError("Coordenates parameters error on COPY");
              return;
            }

            sub_action = action->actions[0];

            result_subtype = evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
              // ld hl, (0xFCB7)  ;GRPACX
              addLdHLii(0xFCB7);
              // push hl
              addPushHL();
              has_x0_coord = true;
            } else {
              addCast(result_subtype, Lexeme::subtype_numeric);

              // ld (0xFCB7), hl  ;GRPACX
              addLdiiHL(0xFCB7);
              // push hl
              addPushHL();
              has_x0_coord = true;
            }

            sub_action = action->actions[1];

            result_subtype = evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
              // ld hl, (0xFCB9)  ;GRPACY
              addLdHLii(0xFCB9);
              // push hl
              addPushHL();
              has_y0_coord = true;
            } else {
              addCast(result_subtype, Lexeme::subtype_numeric);

              // ld (0xFCB9), hl  ;GRPACY
              addLdiiHL(0xFCB9);
              // push hl
              addPushHL();
              has_y0_coord = true;
            }

          } else if (action->lexeme->value == "STEP") {
            sub_action = action->actions[0];

            result_subtype = evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
              // ld hl, (0xFCB7)  ;GRPACX
              addLdHLii(0xFCB7);
              // push hl
              addPushHL();
              has_x0_coord = true;
            } else {
              addCast(result_subtype, Lexeme::subtype_numeric);
              // ld de, (0xFCB7)  ;GRPACX
              addLdDEii(0xFCB7);
              // add hl, de
              addAddHLDE();
              // ld (0xFCB7), hl  ;GRPACX
              addLdiiHL(0xFCB7);
              // push hl
              addPushHL();
              has_x0_coord = true;
            }

            sub_action = action->actions[1];

            result_subtype = evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
              // ld hl, (0xFCB9)  ;GRPACY
              addLdHLii(0xFCB9);
              // push hl
              addPushHL();
              has_y0_coord = true;
            } else {
              addCast(result_subtype, Lexeme::subtype_numeric);
              // ld de, (0xFCB9)  ;GRPACY
              addLdDEii(0xFCB9);
              // add hl, de
              addAddHLDE();
              // ld (0xFCB9), hl  ;GRPACY
              addLdiiHL(0xFCB9);
              // push hl
              addPushHL();
              has_y0_coord = true;
            }

          } else if (action->lexeme->value == "TO_COORD") {
            state++;

            sub_action = action->actions[0];

            result_subtype = evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
            } else {
              addCast(result_subtype, Lexeme::subtype_numeric);

              // ld (0xFCB7), hl  ;GRPACX
              addLdiiHL(0xFCB7);
              // push hl
              addPushHL();
              has_x1_coord = true;
            }

            sub_action = action->actions[1];

            result_subtype = evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
            } else {
              addCast(result_subtype, Lexeme::subtype_numeric);

              // ld (0xFCB9), hl  ;GRPACY
              addLdiiHL(0xFCB9);
              // push hl
              addPushHL();
              has_y1_coord = true;
            }

          } else if (action->lexeme->value == "TO_STEP") {
            state++;

            sub_action = action->actions[0];

            result_subtype = evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
            } else {
              addCast(result_subtype, Lexeme::subtype_numeric);
              // ld de, (0xFCB7)  ;GRPACX
              addLdDEii(0xFCB7);
              // add hl, de
              addAddHLDE();
              // ld (0xFCB7), hl  ;GRPACX
              addLdiiHL(0xFCB7);
              // push hl
              addPushHL();
              has_x1_coord = true;
            }

            sub_action = action->actions[1];

            result_subtype = evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
            } else {
              addCast(result_subtype, Lexeme::subtype_numeric);
              // ld de, (0xFCB9)  ;GRPACY
              addLdDEii(0xFCB9);
              // add hl, de
              addAddHLDE();
              // ld (0xFCB9), hl  ;GRPACY
              addLdiiHL(0xFCB9);
              // push hl
              addPushHL();
              has_y1_coord = true;
            }

          } else {
            // copy from RAM address to vram
            has_address_from = true;

            if ((lexeme = action->lexeme)) {
              if (lexeme->type == Lexeme::type_identifier && lexeme->isArray) {
                // ld hl, variable
                addFix(lexeme);
                addLdHL(0x0000);
                result_subtype = Lexeme::subtype_numeric;
              } else {
                result_subtype = evalExpression(action);
              }
            } else {
              result_subtype = evalExpression(action);
            }

            if (result_subtype == Lexeme::subtype_null ||
                result_subtype == Lexeme::subtype_single_decimal ||
                result_subtype == Lexeme::subtype_double_decimal) {
              syntaxError("Invalid address in COPY");
              return;
            }

            // push hl
            addPushHL();

            state++;
          }

        } break;

        case 1: {
          state++;

          result_subtype = evalExpression(action);

          if (result_subtype == Lexeme::subtype_null) continue;

          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld a, l
          addLdAL();
          // push af
          addPushAF();

          has_src_page = true;

        } break;

        case 2: {
          if (action->lexeme->value == "TO_DEST") {
            state++;

            sub_action = action->actions[0];

            result_subtype = evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
            } else {
              addCast(result_subtype, Lexeme::subtype_numeric);

              // ld (0xFCB7), hl  ;GRPACX
              addLdiiHL(0xFCB7);
              // push hl
              addPushHL();
              has_x2_coord = true;
            }

            sub_action = action->actions[1];

            result_subtype = evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
            } else {
              addCast(result_subtype, Lexeme::subtype_numeric);

              // ld (0xFCB9), hl  ;GRPACY
              addLdiiHL(0xFCB9);
              // push hl
              addPushHL();
              has_y2_coord = true;
            }

          } else {
            // copy from VRAM to RAM address
            has_address_to = true;

            if ((lexeme = action->lexeme)) {
              if (lexeme->type == Lexeme::type_identifier && lexeme->isArray) {
                // ld hl, variable
                addFix(lexeme);
                addLdHL(0x0000);
                result_subtype = Lexeme::subtype_numeric;
              } else {
                result_subtype = evalExpression(action);
              }
            } else {
              result_subtype = evalExpression(action);
            }

            if (result_subtype == Lexeme::subtype_null ||
                result_subtype == Lexeme::subtype_single_decimal ||
                result_subtype == Lexeme::subtype_double_decimal) {
              syntaxError("Invalid address in COPY");
              return;
            }

            state = 99;  // exit loop
          }

        } break;

        case 3: {
          state++;

          result_subtype = evalExpression(action);

          if (result_subtype == Lexeme::subtype_null) continue;

          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld a, l
          addLdAL();
          // push af
          addPushAF();

          has_dest_page = true;

        } break;

        case 4: {
          state++;

          result_subtype = evalExpression(action);

          if (result_subtype == Lexeme::subtype_null) continue;

          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld a, l
          addLdAL();
          addPushAF();

          has_operator = true;

        } break;

        default: {
          syntaxError("COPY parameters not supported");
          return;
        }
      }
    }

    if (has_address_from) {
      if (!has_operator) {
        // xor a
        addXorA();
      } else {
        addPopAF();
      }
      // ld b, a
      addLdBA();
      // ld (LOGOP), a
      addLdiiA(def_LOGOP);

      if (has_dest_page) {
        // pop af
        addPopAF();
        // ld (ACPAGE), a
        addLdiiA(def_ACPAGE);
      }

      if (has_y2_coord) {
        // pop hl
        addPopHL();
      } else {
        // ld hl, (0xFCB9)  ;GRPACY
        addLdHLii(0xFCB9);
      }
      // ld (DY), hl
      addLdiiHL(def_DY);

      if (has_x2_coord) {
        // pop hl
        addPopHL();
      } else {
        // ld hl, (0xFCB7)  ;GRPACX
        addLdHLii(0xFCB7);
      }
      // ld (DX), hl
      addLdiiHL(def_DX);

      if (has_src_page) {
        // pop af
        addPopAF();
        // add a,a
        addAddA();
        // add a,a
        addAddA();
        // ld (ARGT), a    ; direction/expansion (0000DDEE)
        addLdiiA(def_ARGT);
      }

      // pop hl
      addPopHL();

      // call XBASIC_COPY_FROM
      addCall(def_XBASIC_COPY_FROM);

    } else if (has_address_to) {
      // ex de,hl      ; address to (hl to de)
      addExDEHL();

      if (has_src_page) {
        // pop af
        addPopAF();
        // ld (ACPAGE), a
        addLdiiA(def_ACPAGE);
      }

      if (has_y1_coord) {
        // pop hl
        addPopHL();
      } else {
        // ld hl, (0xFCB9)  ;GRPACY
        addLdHLii(0xFCB9);
      }
      // ld (NY), hl
      addLdiiHL(def_NY);

      if (has_x1_coord) {
        // pop hl
        addPopHL();
      } else {
        // ld hl, (0xFCB7)  ;GRPACX
        addLdHLii(0xFCB7);
      }
      // ld (NX), hl
      addLdiiHL(def_NX);

      if (has_y0_coord) {
        // pop hl
        addPopHL();
      } else {
        // ld hl, (0xFCB9)  ;GRPACY
        addLdHLii(0xFCB9);
      }
      // ld (SY), hl
      addLdiiHL(def_SY);

      if (has_x0_coord) {
        // pop hl
        addPopHL();
      } else {
        // ld hl, (0xFCB7)  ;GRPACX
        addLdHLii(0xFCB7);
      }
      // ld (SX), hl
      addLdiiHL(def_SX);

      // ex de,hl
      addExDEHL();

      // call XBASIC_COPY_TO
      addCall(def_XBASIC_COPY_TO);

    } else {
      if (!has_operator) {
        // ld b, 0
        addLdB(0x00);
      } else {
        addPopBC();
      }

      if (has_dest_page) {
        // pop af
        addPopAF();
      } else {
        // ld a, (ACPAGE)
        addLdAii(def_ACPAGE);
      }
      // ld (0xFC19), a
      addLdiiA(0xFC19);

      if (has_y2_coord) {
        // pop hl
        addPopHL();
        // ld (0xFCB9), hl  ;GRPACY
        addLdiiHL(0xFCB9);
      }

      if (has_x2_coord) {
        // pop hl
        addPopHL();
        // ld (0xFCB7), hl  ;GRPACX
        addLdiiHL(0xFCB7);
      }

      if (has_src_page) {
        // pop af
        addPopAF();
      } else {
        // ld a, (ACPAGE)
        addLdAii(def_ACPAGE);
      }
      // ld (0xFC18), a
      addLdiiA(0xFC18);

      if (has_y1_coord) {
        // pop iy
        addPopIY();
      } else {
        // ld iy, (0xFCB9)  ;GRPACY
        addLdIYii(0xFCB9);
      }

      if (has_x1_coord) {
        // pop ix
        addPopIX();
      } else {
        // ld ix, (0xFCB7)  ;GRPACX
        addLdIXii(0xFCB7);
      }

      if (has_y0_coord) {
        // pop hl
        addPopHL();
      } else {
        // ld hl, (0xFCB9)  ;GRPACY
        addLdHLii(0xFCB9);
      }

      if (has_x0_coord) {
        // pop de
        addPopDE();
      } else {
        // ld de, (0xFCB7)  ;GRPACX
        addLdDEii(0xFCB7);
      }

      // call COPY    ; in: de=x0, hl=y0, ix=x1, iy=y1, 0xFC18=srcpg, 0xFCB7=x2,
      // 0xFCB9=y2, 0xFC19=destpg, b=operator
      addCall(def_XBASIC_COPY);
    }

  } else {
    syntaxError("COPY with empty parameters");
  }
}

void Compiler::cmd_copy_screen() {
  int t = current_action->actions.size();

  if (t > 1) {
    syntaxError("Invalid COPY SCREEN parameters");
  } else {
    if (t == 0) {
      // xor a
    } else if (t == 1) {
      ActionNode *action = current_action->actions[0], *sub_action;
      int result_subtype;

      sub_action = action->actions[0];
      result_subtype = evalExpression(sub_action);

      addCast(result_subtype, Lexeme::subtype_numeric);

      // ld a, l
      addLdAL();
    }

    // and 1
    addAnd(0x01);
    // inc a
    addIncA();
    // or 0x10
    addOr(0x10);
    // ld hl, BUF
    addLdHL(def_BUF);
    // push hl
    addPushHL();
    //   ld (hl), 0xC5  ; SCREEN token
    addLdiHL(0xC5);
    //   inc hl
    addIncHL();
    //   ld (hl), a
    addLdiHLA();
    //   inc hl
    addIncHL();
    //   xor a
    addXorA();
    //   ld (hl), a
    addLdiHLA();
    //   inc hl
    addIncHL();
    //   ld (hl), a
    addLdiHLA();
    // pop hl
    addPopHL();

    // ld a, (VERSION)
    addLdAii(def_VERSION);
    // and a
    addAndA();
    // jr z, skip
    addJrZ(9);

    //   ld a, (hl)       ; first character
    addLdAiHL();
    //   ld ix, (COPY)    ; COPY
    addLdIXii(def_COPY_STMT);
    //   call CALBAS
    addCall(def_CALBAS);
    //   ei
    addEI();

    // skip:
  }

  return;
}

void Compiler::cmd_sound() {
  Lexeme* lexeme;
  ActionNode* action;
  unsigned int i, t = current_action->actions.size();
  int result_subtype;

  if (t == 2) {
    for (i = 0; i < t; i++) {
      action = current_action->actions[i];
      lexeme = action->lexeme;
      if (lexeme) {
        result_subtype = evalExpression(action);

        // cast
        addCast(result_subtype, Lexeme::subtype_numeric);

        if (i == 0) {
          // ld a, l
          addLdAL();
          // push af
          addPushAF();
        } else {
          // ld e, l
          addLdEL();
          // pop af
          addPopAF();

          // call sound function
          addCall(def_XBASIC_SOUND);
        }
      }
    }

  } else {
    syntaxError("Invalid SOUND parameters");
  }
}

void Compiler::cmd_bload() {
  Lexeme* lexeme;
  ActionNode* action;
  unsigned int t = current_action->actions.size();
  bool isTinySprite;
  int resource_number;
  string filename, fileext;

  if (t == 2) {
    action = current_action->actions[1];
    lexeme = action->lexeme;
    if (lexeme) {
      if (lexeme->name == "S") {
        action = current_action->actions[0];
        lexeme = action->lexeme;
        if (lexeme) {
          if (lexeme->type == Lexeme::type_literal &&
              lexeme->subtype == Lexeme::subtype_string) {
            // add to resource list

            // verify file type (screen or sprite)

            filename = removeQuotes(lexeme->value);
            fileext = getFileExtension(filename);
            isTinySprite = (strcasecmp((char*)fileext.c_str(), ".SPR") == 0);

            resource_number = resourceManager.resources.size();
            lexeme->name = "FILE";
            resourceManager.addFile(filename, opts->inputPath);

            // execute a resource screen load

            // ld hl, resource number
            addLdHL(resource_number);

            // ld (DAC), hl
            addLdiiHL(def_DAC);

            if (isTinySprite) {
              // call cmd_wrtspr                    ; tiny sprite loader
              addCall(def_cmd_wrtspr);
            } else {
              // call screen_load
              addCall(def_cmd_screen_load);
            }

          } else {
            syntaxError("BLOAD 1st parameter must be a string constant");
          }

        } else {
          syntaxError("BLOAD 1st parameter error");
        }

      } else {
        syntaxError("BLOAD valid only to screen");
      }

    } else {
      syntaxError("BLOAD 2nd parameter error");
    }

  } else {
    syntaxError("Invalid BLOAD parameters count");
  }
}

void Compiler::cmd_play() {
  ActionNode* action;
  unsigned int i, t = current_action->actions.size();
  int result_subtype;
  bool xor_a = true;

  if (t >= 1 && t <= 3) {
    for (i = 0; i < 3; i++) {
      if (i < t) {
        action = current_action->actions[i];
        result_subtype = evalExpression(action);
        if (result_subtype != Lexeme::subtype_string) {
          syntaxError("Invalid PLAY parameter");
          return;
        }
        // push hl
        addPushHL();
      } else {
        if (xor_a) {
          // xor a
          addXorA();
          xor_a = false;
        }
        // push af
        addPushAF();
      }
    }

    // pop bc
    addPopBC();
    // pop de
    addPopDE();
    // pop hl
    addPopHL();

    addCall(def_XBASIC_PLAY);

  } else {
    syntaxError("Invalid PLAY parameters");
  }
}

void Compiler::cmd_draw() {
  Lexeme* lexeme;
  ActionNode* action;
  unsigned int i, t = current_action->actions.size();
  int result_subtype;

  if (t == 1) {
    i = 0;
    action = current_action->actions[i];
    lexeme = action->lexeme;
    if (lexeme) {
      result_subtype = evalExpression(action);

      if (result_subtype == Lexeme::subtype_string) {
        // call draw function
        if (draw_mark)
          addFix(draw_mark->symbol);
        else
          draw_mark = addMark();
        addCall(0x0000);

      } else {
        syntaxError("Invalid DRAW parameter");
        return;
      }
    }

  } else {
    syntaxError("Invalid DRAW parameters");
  }
}

void Compiler::cmd_dim() {
  Lexeme *lexeme, *parm_lexeme;
  ActionNode* action;
  unsigned int i, k, w, tt, t = current_action->actions.size();
  int new_size;

  if (!t) {
    syntaxError("DIM parameters is missing");
  } else {
    for (i = 0; i < t; i++) {
      action = current_action->actions[i];
      lexeme = action->lexeme;

      if (lexeme->type == Lexeme::type_identifier) {
        if (lexeme->subtype == Lexeme::subtype_string)
          lexeme->x_factor = 256;
        else if (lexeme->subtype == Lexeme::subtype_numeric)
          lexeme->x_factor = 2;
        else if (lexeme->subtype == Lexeme::subtype_single_decimal ||
                 lexeme->subtype == Lexeme::subtype_double_decimal)
          lexeme->x_factor = 3;
        else
          lexeme->x_factor = 0;

        tt = action->actions.size();
        if (tt >= 1 && tt <= 2) {
          lexeme->isArray = true;

          if (tt == 1) {
            k = 0;
            w = 1;
          } else {
            k = 1;
            w = 0;
          }

          parm_lexeme = action->actions[k]->lexeme;
          if (parm_lexeme->type == Lexeme::type_literal &&
              parm_lexeme->subtype == Lexeme::subtype_numeric) {
            try {
              lexeme->x_size = stoi(parm_lexeme->value) + 1;
            } catch (exception& e) {
              printf("Warning: error while converting numeric constant %s\n",
                     parm_lexeme->value.c_str());
              lexeme->x_size = 0;
            }
            if (!lexeme->x_size) {
              syntaxError("Array 1st dimension index cannot be zero");
              break;
            }
          } else {
            syntaxError("Array 1st dimension index must be a integer constant");
            break;
          }

          if (tt == 2) {
            parm_lexeme = action->actions[w]->lexeme;
            if (parm_lexeme->type == Lexeme::type_literal &&
                parm_lexeme->subtype == Lexeme::subtype_numeric) {
              try {
                lexeme->y_size = stoi(parm_lexeme->value) + 1;
              } catch (exception& e) {
                printf("Warning: error while converting numeric constant %s\n",
                       parm_lexeme->value.c_str());
                lexeme->y_size = 0;
              }
              if (!lexeme->y_size) {
                syntaxError("Array 2nd dimension index cannot be zero");
                break;
              }
            } else {
              syntaxError(
                  "Array 2nd dimension index must be a integer constant");
              break;
            }
          } else
            lexeme->y_size = 1;

          lexeme->y_factor = lexeme->x_factor * lexeme->x_size;
          new_size = lexeme->y_factor * lexeme->y_size;

          if (lexeme->array_size < new_size) lexeme->array_size = new_size;
        } else {
          syntaxError("Arrays with more than 2 dimensions isn't supported");
          break;
        }

      } else {
        syntaxError("Invalid DIM parameter");
        break;
      }
    }
  }
}

void Compiler::cmd_redim() {
  cmd_dim();
}

void Compiler::cmd_let() {
  Lexeme* lexeme;
  ActionNode *action, *lex_action;
  unsigned int t = current_action->actions.size();
  int result_subtype, result[3];

  if (t != 2) {
    syntaxError("Invalid LET parameters count");
    return;
  }

  lex_action = current_action->actions[0];
  lexeme = lex_action->lexeme;

  // ld hl, data parameter

  action = current_action->actions[1];
  result_subtype = evalExpression(action);

  // do assignment

  if (lexeme->value == "MID$") {
    // cast
    addCast(result_subtype, Lexeme::subtype_string);

    // push hl
    addPushHL();

    t = lex_action->actions.size();

    if (t < 2 || t > 3) {
      syntaxError("Invalid MID$ assignment parameters count");
      return;
    }

    if (!evalOperatorParms(lex_action, t)) {
      syntaxError("Invalid MID$ assignment parameters");
      return;
    }

    result[0] = lex_action->actions[0]->subtype;
    result[1] = lex_action->actions[1]->subtype;

    if (t == 2) {
      if (result[0] == Lexeme::subtype_single_decimal ||
          result[0] == Lexeme::subtype_double_decimal) {
        // cast
        addCast(result[0], Lexeme::subtype_numeric);
        result[0] = Lexeme::subtype_numeric;
      }

      if (result[1] == Lexeme::subtype_string &&
          result[0] == Lexeme::subtype_numeric) {
        // ld a, l         ; start char
        addLdAL();
        // pop de          ; de=destination string
        addPopDE();
        // pop hl          ; hl=source string
        addPopHL();

        // ld b, (hl)      ; number of chars (all from source)
        addLdBiHL();

        // call 0x7dd8    ; mid assignment (in: hl=source string, b=size,
        // a=start, de=destination string)
        addCall(def_XBASIC_MID_ASSIGN);

        return;
      }

    } else {
      result[2] = lex_action->actions[2]->subtype;

      if (result[0] == Lexeme::subtype_single_decimal ||
          result[0] == Lexeme::subtype_double_decimal) {
        // cast
        addCast(result[0], Lexeme::subtype_numeric);
        result[0] = Lexeme::subtype_numeric;
      }

      // ld b, l             ; number of chars
      addLdBL();

      if (result[1] == Lexeme::subtype_single_decimal ||
          result[1] == Lexeme::subtype_double_decimal) {
        // ld a, b
        addLdAB();
        // pop bc
        addPopBC();
        // pop hl
        addPopHL();
        // push af
        addPushAF();
        // cast
        addCast(result[1], Lexeme::subtype_numeric);
        // pop bc
        addPopBC();
        result[1] = Lexeme::subtype_numeric;
      } else {
        // pop hl
        addPopHL();
      }

      // ld a, l         ; start char
      addLdAL();

      if (result[2] == Lexeme::subtype_string &&
          result[1] == Lexeme::subtype_numeric &&
          result[0] == Lexeme::subtype_numeric) {
        // pop de          ; de=destination string
        addPopDE();
        // pop hl          ; hl=source string
        addPopHL();

        // call 0x7dd8    ; mid assignment (in: hl=source string, b=size,
        // a=start, de=destination string)
        addCall(def_XBASIC_MID_ASSIGN);

        return;
      }
    }

    syntaxError("Invalid MID$ assignment type");

  } else if (lexeme->value == "VDP") {
    // cast
    addCast(result_subtype, Lexeme::subtype_numeric);

    // push hl          ; data
    addPushHL();

    t = lex_action->actions.size();

    if (t != 1) {
      syntaxError("Invalid VDP assignment parameters count");
      return;
    }

    if (!evalOperatorParms(lex_action, t)) {
      syntaxError("Invalid VDP assignment parameters");
      return;
    }

    result[0] = lex_action->actions[0]->subtype;

    // cast
    addCast(result[0], Lexeme::subtype_numeric);
    result[0] = Lexeme::subtype_numeric;

    // pop bc           ; data
    addPopBC();

    // call VDP.set     ; VDP assignment (in: bc=data, hl=register)
    addCall(def_vdp_set);

  } else if (lexeme->value == "SPRITE$") {
    // cast
    addCast(result_subtype, Lexeme::subtype_string);

    // push hl
    addPushHL();

    t = lex_action->actions.size();

    if (t != 1) {
      syntaxError("Invalid SPRITE$ assignment parameters count");
      return;
    }

    if (!evalOperatorParms(lex_action, t)) {
      syntaxError("Invalid SPRITE$ assignment parameters");
      return;
    }

    result[0] = lex_action->actions[0]->subtype;

    // cast
    addCast(result[0], Lexeme::subtype_numeric);
    result[0] = Lexeme::subtype_numeric;

    // ld a, l
    addLdAL();
    // pop hl
    addPopHL();

    // call 0x7143     ; xbasic SPRITE assignment (in: a=sprite pattern,
    // hl=string)
    addCall(def_XBASIC_SPRITE_ASSIGN);

  } else {
    // cast

    addCast(result_subtype, lexeme->subtype);

    // do assignment

    addAssignment(lex_action);
  }
}

bool Compiler::addAssignment(ActionNode* action) {
  if (action->lexeme->type == Lexeme::type_keyword) {
    if (action->lexeme->value == "TIME") {
      // ld (0xFC9E), hl    ; JIFFY
      addLdiiHL(0xFC9E);

    } else if (action->lexeme->value == "MAXFILES") {
      // ld a, l
      addLdAL();
      // ld ix, MAXFILES
      addLdIX(def_MAXFILES);
      // call CALBAS
      addCall(def_CALBAS);
      // ei
      addEI();

    } else {
      syntaxError("Invalid KEYWORD/FUNCTION assignment");
    }

  } else if (action->lexeme->type == Lexeme::type_identifier) {
    if (action->lexeme->isArray ||
        action->lexeme->subtype == Lexeme::subtype_string) {
      // push hl
      addPushHL();

      if (action->lexeme->subtype == Lexeme::subtype_single_decimal ||
          action->lexeme->subtype == Lexeme::subtype_double_decimal) {
        // push bc
        addPushBC();
      }

      if (!addVarAddress(action)) return false;

      if (action->lexeme->subtype == Lexeme::subtype_string) {
        // pop de
        addPopDE();
        // ex de,hl
        addExDEHL();

        // call 0x7e9d   ; xbasic copy string (in: hl=source, de=dest; out: hl
        // end of string)
        addCall(def_XBASIC_COPY_STRING);

      } else if (action->lexeme->subtype == Lexeme::subtype_numeric) {
        // pop de
        addPopDE();
        // ld (hl),e
        addLdiHLE();
        // inc hl
        addIncHL();
        // ld (hl),d
        addLdiHLD();

      } else if (action->lexeme->subtype == Lexeme::subtype_single_decimal ||
                 action->lexeme->subtype == Lexeme::subtype_double_decimal) {
        // pop bc
        addPopBC();
        // pop de
        addPopDE();
        // ld (hl),b
        addLdiHLB();
        // inc hl
        addIncHL();
        // ld (hl),e
        addLdiHLE();
        // inc hl
        addIncHL();
        // ld (hl),d
        addLdiHLD();

      } else {
        syntaxError("Invalid assignment");
        return false;
      }

    } else {
      // assignment optimization

      if (action->lexeme->subtype == Lexeme::subtype_numeric) {
        // ld (var), hl
        addFix(action->lexeme);
        addLdiiHL(0x0000);

      } else if (action->lexeme->subtype == Lexeme::subtype_single_decimal ||
                 action->lexeme->subtype == Lexeme::subtype_double_decimal) {
        // ld a, b
        addLdAB();
        // ld (var), a
        addFix(action->lexeme);
        addLdiiA(0x0000);
        // ld (var+1), hl
        addFix(action->lexeme)->step = 1;
        addLdiiHL(0x0000);

      } else {
        syntaxError("Invalid assignment");
        return false;
      }
    }

  } else {
    syntaxError("Invalid constant/expression assignment");
    return false;
  }

  return true;
}

void Compiler::cmd_if() {
  Lexeme *lexeme, *last_lexeme;
  ActionNode *action, *saved_action = current_action, *last_action;
  unsigned int i, t = saved_action->actions.size(), tt;
  int result_subtype;
  FixNode *mark_else = 0, *mark_endif = 0;
  bool isLastActionGoto = false, isElseLikeEndif = true;

  if (!t) {
    syntaxError("IF parameters is missing");
  } else {
    for (i = 0; i < t; i++) {
      action = saved_action->actions[i];
      lexeme = action->lexeme;

      if (lexeme->type == Lexeme::type_keyword) {
        if (lexeme->value == "COND") {
          // ld hl, data parameter

          result_subtype = evalExpression(action->actions[0]);

          if (result_subtype == Lexeme::subtype_numeric) {
            // ld a, l
            addLdAL();
            // or h
            addOrH();
            // jp z, ELSE or ENDIF
            mark_else = addMark();
            addJpZ(0x0000);

          } else {
            syntaxError("Invalid condition expression");
            break;
          }

        } else if (lexeme->value == "THEN") {
          tt = action->actions.size();
          if (tt) {
            last_action = action->actions[tt - 1];
            last_lexeme = last_action->lexeme;
            isLastActionGoto = (last_lexeme->type == Lexeme::type_keyword &&
                                last_lexeme->value == "GOTO");
          }

          if (!evalActions(action)) break;

        } else if (lexeme->value == "GOTO") {
          isLastActionGoto = true;

          if (!evalAction(action)) break;

        } else if (lexeme->value == "GOSUB") {
          if (!evalAction(action)) break;

        } else if (lexeme->value == "ELSE") {
          if (!isLastActionGoto) {
            // jp ENDIF
            mark_endif = addMark();
            addJp(0x0000);
          }

          isLastActionGoto = false;
          isElseLikeEndif = false;

          tt = action->actions.size();
          if (tt == 1) {
            last_action = action->actions[0];
            last_lexeme = last_action->lexeme;
            isLastActionGoto = (last_lexeme->type == Lexeme::type_keyword &&
                                last_lexeme->value == "GOTO");
          }

          if (isLastActionGoto) {
            if (last_action->actions.size()) {
              last_lexeme = last_action->actions[0]->lexeme;
            }

            if (last_lexeme->type == Lexeme::type_literal &&
                last_lexeme->subtype == Lexeme::subtype_numeric) {
              // mark ELSE position
              if (mark_else) {
                mark_else->symbol = addSymbol(last_lexeme->value);
              } else {
                syntaxError("ELSE parameter is missing");
              }
            } else {
              syntaxError("Invalid GOTO parameter");
            }

          } else {
            // mark ELSE position
            if (mark_else) mark_else->symbol->address = code_pointer;

            if (!evalActions(action)) break;
          }

        } else {
          syntaxError("Invalid IF syntax");
          break;
        }

      } else {
        syntaxError("Invalid IF parameter type");
        break;
      }
    }

    // mark ENDIF position
    if (mark_endif)
      mark_endif->symbol->address = code_pointer;
    else if (mark_else)
      if (isElseLikeEndif) mark_else->symbol->address = code_pointer;
  }
}

void Compiler::cmd_for() {
  Lexeme *lexeme, *lex_var = 0;
  ActionNode *action, *var_action, *saved_action = current_action;
  unsigned int i, t = saved_action->actions.size();
  int result_subtype;
  ForNextNode* forNext;
  bool has_let = false, has_to = false, has_step = false;

  if (!t) {
    syntaxError("FOR parameters is missing");
  } else {
    for_count++;

    forNext = new ForNextNode();
    forNextStack.push(forNext);

    forNext->index = for_count;
    forNext->tag = current_tag;
    forNext->for_to =
        new Lexeme(Lexeme::type_identifier, Lexeme::subtype_numeric,
                   "FOR_TO_" + to_string(for_count));
    forNext->for_to_action = new ActionNode();
    forNext->for_to_action->lexeme = forNext->for_to;
    addSymbol(forNext->for_to);

    forNext->for_step =
        new Lexeme(Lexeme::type_identifier, Lexeme::subtype_numeric,
                   "FOR_STEP_" + to_string(for_count));
    forNext->for_step_action = new ActionNode();
    forNext->for_step_action->lexeme = forNext->for_step;
    addSymbol(forNext->for_step);

    for (i = 0; i < t; i++) {
      action = saved_action->actions[i];
      lexeme = action->lexeme;

      if (lexeme->type == Lexeme::type_keyword) {
        if (lexeme->value == "LET") {
          if (action->actions.size()) {
            var_action = action->actions[0];
            lex_var = var_action->lexeme;
            forNext->for_var = lex_var;
            forNext->for_to->subtype = lex_var->subtype;
            forNext->for_step->subtype = lex_var->subtype;
          } else {
            syntaxError("Invalid FOR expression (variable assignment)");
            return;
          }

          if (lex_var->type != Lexeme::type_identifier) {
            syntaxError("Invalid FOR expression (variable is missing)");
            return;
          } else {
            if (lex_var->subtype != Lexeme::subtype_numeric &&
                lex_var->subtype != Lexeme::subtype_single_decimal &&
                lex_var->subtype != Lexeme::subtype_double_decimal) {
              syntaxError("Invalid FOR expression (wrong data type)");
              return;
            }
          }

          if (!evalAction(action)) return;

          has_let = true;

        } else if (lexeme->value == "TO") {
          // ld hl, data parameter

          result_subtype = evalExpression(action->actions[0]);

          if (result_subtype == Lexeme::subtype_numeric ||
              result_subtype == Lexeme::subtype_single_decimal ||
              result_subtype == Lexeme::subtype_double_decimal) {
            addCast(result_subtype, lex_var->subtype);

            addAssignment(forNext->for_to_action);

            has_to = true;

          } else {
            syntaxError("Invalid TO expression (wrong data type)");
            return;
          }

        } else if (lexeme->value == "STEP") {
          // ld hl, data parameter

          result_subtype = evalExpression(action->actions[0]);

          if (result_subtype == Lexeme::subtype_numeric ||
              result_subtype == Lexeme::subtype_single_decimal ||
              result_subtype == Lexeme::subtype_double_decimal) {
            addCast(result_subtype, lex_var->subtype);

            addAssignment(forNext->for_step_action);

            has_step = true;

          } else {
            syntaxError("Invalid STEP expression (wrong data type)");
            return;
          }

        } else {
          syntaxError("Invalid FOR syntax");
          return;
        }

      } else {
        syntaxError("Invalid FOR parameter type");
        return;
      }
    }

    if (has_let && has_to) {
      if (!has_step && lex_var->subtype != Lexeme::subtype_numeric) {
        // ld hl, 1
        addLdHL(0x0001);

        addCast(Lexeme::subtype_numeric, forNext->for_step->subtype);

        addAssignment(forNext->for_step_action);
      }

      if (lex_var->subtype == Lexeme::subtype_numeric) {
        // ld hl, (variable)
        addFix(forNext->for_var);
        addLdHLii(0x0000);

        // jr $+12      ; jump to check code
        if (has_step) {
          addJr(0x0B);
        } else {
          addJr(0x07);
        }

        // ;step code
        forNext->for_step_mark = addPreMark();
        forNext->for_step_mark->address = code_pointer;

        // ld hl, (variable)
        addFix(forNext->for_var);
        addLdHLii(0x0000);

        if (has_step) {
          // ld de, (step)
          addByte(0xED);
          addFix(forNext->for_step);
          addCmd(0x5B, 0x0000);
          // add hl,de
          addAddHLDE();
        } else {
          // inc hl
          addIncHL();
        }

        // ld (variable), hl
        addFix(forNext->for_var);
        addLdiiHL(0x0000);

        // ;check code

        // ex de, hl         ; after, de = (variable)
        addExDEHL();

        // ld hl, (to)
        addFix(forNext->for_to);
        addLdHLii(0x0000);

        if (has_step) {
          // ld a, (step+1)
          addFix(forNext->for_step)->step = 1;
          addLdAii(0x0000);

          // bit 7, a
          addWord(0xCB, 0x7F);
          // jr z, $+2
          addJrZ(0x01);
          //   ex de,hl
          addExDEHL();
        }

        // ;var > to? goto end for

        // call intCompareGT
        // addCall(def_intCompareGT);
        addKernelCall(def_intCompareGT);

        // jp nz, end_for
        forNext->for_end_mark = addMark();
        addJpNZ(0x0000);

        // body start

      } else {
        // jr $+26      ; jump to check code
        addJr(0x19);

        // ;step code
        forNext->for_step_mark = addPreMark();
        forNext->for_step_mark->address = code_pointer;

        // ld a, (variable)
        addFix(forNext->for_var);
        addLdAii(0x0000);
        // ld b, a
        addLdBA();
        // ld hl, (variable+1)
        addFix(forNext->for_var)->step = 1;
        addLdHLii(0x0000);

        // ld a, (step)
        addFix(forNext->for_step);
        addLdAii(0x0000);
        // ld c, a
        addLdCA();
        // ld de, (step)
        addByte(0xED);
        addFix(forNext->for_step)->step = 1;
        addCmd(0x5B, 0x0000);
        // call 0x76c1     ; add floats (b:hl + c:de = b:hl)
        addCall(def_XBASIC_ADD_FLOATS);

        // ld a, b
        addLdAB();
        // ld (variable), a
        addFix(forNext->for_var);
        addLdiiA(0x0000);
        // ld (variable+1), hl
        addFix(forNext->for_var)->step = 1;
        addLdiiHL(0x0000);

        // ;check code

        // ld a, (to)
        addFix(forNext->for_to);
        addLdAii(0x0000);
        // ld b, a
        addLdBA();
        // ld hl, (to+1)
        addFix(forNext->for_to)->step = 1;
        addLdHLii(0x0000);

        // ld a, (variable)
        addFix(forNext->for_var);
        addLdAii(0x0000);
        // ld c, a
        addLdCA();
        // ld de, (variable+1)
        addByte(0xED);
        addFix(forNext->for_var)->step = 1;
        addCmd(0x5B, 0x0000);

        // ld a, (step+2)
        addFix(forNext->for_step)->step = 2;
        addLdAii(0x0000);

        // bit 7, a
        addWord(0xCB, 0x7F);
        // jr nz, $+5
        addJrNZ(0x04);
        //   ex de,hl
        addExDEHL();
        //   ld a, c
        addLdAC();
        //   ld c, b
        addLdCB();
        //   ld b, a
        addLdBA();

        // ;var > to? goto end for

        // 78a4 xbasic compare floats (<=)
        addCall(def_XBASIC_COMPARE_FLOATS_LE);

        // ld a, l
        addLdAL();
        // or h
        addOrH();

        // jp z, end_for
        forNext->for_end_mark = addMark();
        addJpZ(0x0000);

        // body start
      }

    } else {
      syntaxError("Incomplete FOR syntax");
    }
  }
}

void Compiler::cmd_next() {
  ForNextNode* forNext;

  if (forNextStack.size()) {
    forNext = forNextStack.top();
    forNextStack.pop();

    // jp step
    addFix(forNext->for_step_mark);
    addJp(0x0000);

    if (forNext->for_end_mark)
      forNext->for_end_mark->symbol->address = code_pointer;

  } else {
    syntaxError("NEXT without a FOR");
  }
}

void Compiler::cmd_locate() {
  ActionNode* action;
  unsigned int t = current_action->actions.size();
  int result_subtype;

  if (t != 2) {
    syntaxError("LOCATE without enough parameters");
  } else {
    action = current_action->actions[0];
    result_subtype = evalExpression(action);

    addCast(result_subtype, Lexeme::subtype_numeric);

    // push hl
    addPushHL();

    action = current_action->actions[1];
    result_subtype = evalExpression(action);

    addCast(result_subtype, Lexeme::subtype_numeric);

    // pop de
    addByteOptimized(0xD1);
    // call XBASIC_LOCATE    ; hl = y, de = x
    addCall(def_XBASIC_LOCATE);
  }
}

void Compiler::cmd_screen() {
  ActionNode* action;
  Lexeme* lexeme;
  unsigned int i, t = current_action->actions.size();
  int result_subtype;

  if (t) {
    action = current_action->actions[0];
    lexeme = action->lexeme;
    if (lexeme->value == "COPY") {
      current_action = action;
      cmd_screen_copy();
      return;
    } else if (lexeme->value == "PASTE") {
      current_action = action;
      cmd_screen_paste();
      return;
    } else if (lexeme->value == "SCROLL") {
      current_action = action;
      cmd_screen_scroll();
      return;
    } else if (lexeme->value == "LOAD") {
      current_action = action;
      cmd_screen_load();
      return;
    } else if (lexeme->value == "ON") {
      current_action = action;
      cmd_screen_on();
      return;
    } else if (lexeme->value == "OFF") {
      current_action = action;
      cmd_screen_off();
      return;
    }

    for (i = 0; i < t; i++) {
      action = current_action->actions[i];
      result_subtype = evalExpression(action);

      if (result_subtype == Lexeme::subtype_null) continue;

      addCast(result_subtype, Lexeme::subtype_numeric);

      if (i != 5) {
        // ld a, l
        addLdAL();
      }

      switch (i) {
        // display mode
        case 0: {
          // call XBASIC_SCREEN ; xbasic SCREEN change mode (in: a, l = screen
          // mode)
          addCall(def_XBASIC_SCREEN);
        } break;

        // sprite size
        case 1: {
          // call 0x70bc    ; xbasic SCREEN sprite (in: a = sprite mode)
          addCall(def_XBASIC_SCREEN_SPRITE);
        } break;

        // key click
        case 2: {
          // ld (CLIKSW), a   ; 0=keyboard click off, 1=keyboard click on
          addLdiiA(def_CLIKSW);
        } break;

        // baud rate
        case 3: {
          // ; original code: C1F63 on subrom of TurboR and A7A2D on main rom of
          // the rest ld bc,5
          addLdBC(0x0005);
          // and a
          addAndA();
          // ld hl,CS1200
          addLdHL(def_CS1200);
          // jr z,skip
          addJrZ(0x01);
          //   add hl,bc
          addAddHLBC();
          // skip:
          // ld de,LOW
          addLdDE(def_LOW);
          // ldir
          addLDIR();
        } break;

        // printer type
        case 4: {
          // ld (NTMSXP), a   ; printer type (0=default)
          addLdiiA(def_NTMSXP);
        } break;

        // interlace mode
        case 5: {
          // ld a, (VERSION)
          addLdAii(def_VERSION);
          // and a
          addAndA();
          // jr z, skip1
          addJrZ(25);

          //   ld a, l
          addLdAL();

          //   ; original code: J1F45 on subrom of MSX2 and above
          //   and 3     ; must be 0 to 3
          addAnd(0x03);
          //   add a, a
          addAddA();
          //   bit 1, a
          addWord(0xCB, 0x4F);
          //   jr z, skip2
          addJrZ(0x02);
          //     set 3, a
          addWord(0xCB, 0xDF);
          //   skip2:
          //   and 0x0C
          addAnd(0x0C);
          //   ld b, a
          addLdBA();
          //   ld a, (RG9SAV)
          addLdAii(def_RG9SAV);
          //   and 0xF3
          addAnd(0xF3);
          //   or b
          addOrB();
          //   ld c, 9
          addLdC(0x09);
          //   ld b, a
          addLdBA();
          //   call WRTVDP
          addCall(def_WRTVDP);

          // skip1:
        } break;

        default: {
          syntaxError("SCREEN parameters not supported");
          return;
        }
      }
    }

  } else {
    syntaxError("SCREEN with empty parameters");
  }
}

void Compiler::cmd_screen_copy() {
  Lexeme* lexeme;
  ActionNode* action;
  unsigned int i, t = current_action->actions.size();
  int result_subtype;

  if (t) {
    if (t > 2) {
      syntaxError("SCREEN COPY with excess of parameters");
      return;
    }

    for (i = 0; i < t; i++) {
      action = current_action->actions[i];
      lexeme = action->lexeme;

      if (i) {
        // push hl
        addPushHL();

        result_subtype = evalExpression(action);
        addCast(result_subtype, Lexeme::subtype_numeric);

        // ld a, l                 ; copy parameter to A
        addLdAL();

        // pop hl
        addPopHL();
      } else {
        if (lexeme->type == Lexeme::type_identifier) {
          // ld hl, variable
          addFix(lexeme);
          addLdHL(0x0000);
          result_subtype = Lexeme::subtype_numeric;
        } else {
          result_subtype = evalExpression(action);
        }
        addCast(result_subtype, Lexeme::subtype_numeric);
        if (t == 1) {
          // xor a
          addXorA();
        }
      }
    }

    // call screen_copy
    addCall(def_cmd_screen_copy);

  } else {
    syntaxError("SCREEN COPY with empty parameters");
  }
}

void Compiler::cmd_screen_paste() {
  Lexeme* lexeme;
  ActionNode* action;
  unsigned int i, t = current_action->actions.size();
  int result_subtype;

  if (t) {
    if (t > 1) {
      syntaxError("SCREEN PASTE with excess of parameters");
      return;
    }

    for (i = 0; i < t; i++) {
      action = current_action->actions[i];
      lexeme = action->lexeme;

      if (lexeme->type == Lexeme::type_identifier) {
        // ld hl, variable
        addFix(lexeme);
        addLdHL(0x0000);
        result_subtype = Lexeme::subtype_numeric;
      } else {
        result_subtype = evalExpression(action);
      }
      addCast(result_subtype, Lexeme::subtype_numeric);
    }

    // call screen_paste
    addCall(def_cmd_screen_paste);

  } else {
    syntaxError("SCREEN PASTE with empty parameters");
  }
}

void Compiler::cmd_screen_scroll() {
  ActionNode* action;
  unsigned int i, t = current_action->actions.size();
  int result_subtype;

  if (t) {
    if (t > 1) {
      syntaxError("SCREEN SCROLL with excess of parameters");
      return;
    }

    for (i = 0; i < t; i++) {
      action = current_action->actions[i];
      result_subtype = evalExpression(action);
      addCast(result_subtype, Lexeme::subtype_numeric);

      // ld a, l                 ; copy parameter to A
      addLdAL();

      // ld hl, (HEAPSTR)
      addLdHLii(def_HEAPSTR);

      // push hl
      addPushHL();

      // call screen_copy
      addCall(def_cmd_screen_copy);

      // pop hl
      addPopHL();

      // call screen_paste
      addCall(def_cmd_screen_paste);
    }

  } else {
    syntaxError("SCREEN SCROLL with empty parameters");
  }
}

void Compiler::cmd_screen_load() {
  ActionNode* action;
  unsigned int i, t = current_action->actions.size();
  int result_subtype;

  if (t) {
    for (i = 0; i < t; i++) {
      action = current_action->actions[i];
      result_subtype = evalExpression(action);
      addCast(result_subtype, Lexeme::subtype_numeric);

      // ld (DAC), hl
      addLdiiHL(def_DAC);

      // call screen_load
      addCall(def_cmd_screen_load);
    }

  } else {
    syntaxError("SCREEN LOAD with empty parameters");
  }
}

void Compiler::cmd_screen_on() {
  // call cmd_enascr
  addCall(def_cmd_enascr);
}

void Compiler::cmd_screen_off() {
  // call cmd_disscr
  addCall(def_cmd_disscr);
}

void Compiler::cmd_width() {
  ActionNode* action;
  unsigned int t = current_action->actions.size();
  int result_subtype;

  if (t == 1) {
    action = current_action->actions[0];
    result_subtype = evalExpression(action);

    addCast(result_subtype, Lexeme::subtype_numeric);

    // call XBASIC WIDTH   ; xbasic WIDTH (in: l = size)
    addCall(def_XBASIC_WIDTH);

  } else {
    syntaxError("WIDTH syntax error");
  }
}

void Compiler::cmd_color() {
  ActionNode *action, *subaction;
  Lexeme* lexeme;
  unsigned int i, t = current_action->actions.size();
  int result_subtype;

  if (t) {
    action = current_action->actions[0];
    lexeme = action->lexeme;

    if (lexeme->type == Lexeme::type_keyword) {
      if (lexeme->value == "NEW") {
        // ld a, (VERSION)
        addLdAii(def_VERSION);
        // and a
        addAndA();
        // jr z, skip
        addJrZ(8);
        //   ld ix, 0x0141
        addLdIX(0x0141);
        //   call EXTROM
        addCall(def_EXTROM);
        //   ei
        addEI();
        // skip:

      } else if (lexeme->value == "RESTORE") {
        // ld a, (VERSION)
        addLdAii(def_VERSION);
        // and a
        addAndA();
        // jr z, skip
        addJrZ(8);
        //   ld ix, 0x0145
        addLdIX(0x0145);
        //   call EXTROM
        addCall(def_EXTROM);
        //   ei
        addEI();
        // skip:

      } else if (lexeme->value == "SPRITE") {
        t = action->actions.size();

        if (t != 2) {
          syntaxError("Invalid COLOR SPRITE parameters count");
          return;
        }

        for (i = 0; i < t; i++) {
          subaction = action->actions[i];
          result_subtype = evalExpression(subaction);

          if (result_subtype != Lexeme::subtype_null) {
            addCast(result_subtype, Lexeme::subtype_numeric);

            // ld a, l
            addLdAL();
          }

          if (i < 1) {
            // push af
            addPushAF();
          }
        }

        // pop bc
        addPopBC();
        // call COLOR_SPRITE   ; in: b, a
        addCall(def_XBASIC_COLOR_SPRITE);

      } else if (lexeme->value == "SPRITE$") {
        t = action->actions.size();

        if (t != 2) {
          syntaxError("Invalid COLOR SPRITE$ parameters count");
          return;
        }

        for (i = 0; i < t; i++) {
          subaction = action->actions[i];
          result_subtype = evalExpression(subaction);

          if (i < 1) {
            addCast(result_subtype, Lexeme::subtype_numeric);

            // ld a, l
            addLdAL();
            // push af
            addPushAF();
          }
        }

        // pop bc
        addPopBC();
        // call COLOR_SPRSTR   ; in: b, hl
        addCall(def_XBASIC_COLOR_SPRSTR);

      } else if (lexeme->value == "RGB") {
        t = action->actions.size();

        if (t < 2 || t > 4) {
          syntaxError("Invalid COLOR RGB parameters count");
          return;
        }

        for (i = 0; i < t; i++) {
          subaction = action->actions[i];
          result_subtype = evalExpression(subaction);

          if (result_subtype != Lexeme::subtype_null) {
            addCast(result_subtype, Lexeme::subtype_numeric);

            // ld a, l
            addLdAL();
          }

          if (i < 3) {
            // push af
            addPushAF();
          }
        }

        if (t < 4) {
          // xor a
          addXorA();
        }
        // ld b, a
        addLdBA();

        if (t < 3) {
          // ld h, a
          addLdHA();
        } else {
          // pop hl
          addPopHL();
        }

        // pop de
        addPopDE();
        // pop af
        addPopAF();

        // call COLOR_RGB   ; in: a, d, h, b
        addCall(def_XBASIC_COLOR_RGB);

      } else {
        syntaxError("Invalid COLOR parameters");
      }

    } else {
      for (i = 0; i < t; i++) {
        action = current_action->actions[i];
        result_subtype = evalExpression(action);

        if (result_subtype == Lexeme::subtype_null) continue;

        addCast(result_subtype, Lexeme::subtype_numeric);

        // ld a, l
        addLdAL();

        switch (i) {
          case 0: {
            // ld (0xF3E9), a   ; FORCLR
            addLdiiA(0xF3E9);
            // ld (ATRBYT), a   ; ATRBYT
            addLdiiA(def_ATRBYT);
          } break;

          case 1: {
            // ld (0xF3EA), a   ; BAKCLR
            addLdiiA(0xF3EA);
          } break;

          case 2: {
            // ld (0xF3EB), a   ; BDRCLR
            addLdiiA(0xF3EB);
          } break;

          default: {
            syntaxError("COLOR parameters not supported");
            return;
          }
        }
      }

      // ld a, (SCRMOD)
      addLdAii(def_SCRMOD);

      // call 0x0062   ; CHGCLR
      addCall(0x0062);
    }

  } else {
    syntaxError("COLOR with empty parameters");
  }
}

void Compiler::cmd_pset(bool forecolor) {
  ActionNode *action, *sub_action;
  unsigned int i, t = current_action->actions.size();
  int result_subtype;
  bool has_x_coord = false, has_y_coord = false, has_color = false,
       has_operator = false;

  if (t) {
    for (i = 0; i < t; i++) {
      action = current_action->actions[i];

      switch (i) {
        case 0: {
          if (action->actions.size() != 2) {
            if (forecolor)
              syntaxError("Coordenates parameters error on PSET");
            else
              syntaxError("Coordenates parameters error on PRESET");
            return;
          } else if (action->lexeme->value == "COORD") {
            sub_action = action->actions[0];

            result_subtype = evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
            } else {
              addCast(result_subtype, Lexeme::subtype_numeric);

              // ld (0xFCB7), hl  ;GRPACX
              addLdiiHL(0xFCB7);
              // push hl
              addPushHL();

              has_x_coord = true;
            }

            sub_action = action->actions[1];

            result_subtype = evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
            } else {
              addCast(result_subtype, Lexeme::subtype_numeric);

              // ld (0xFCB9), hl  ;GRPACY
              addLdiiHL(0xFCB9);

              // push hl
              addPushHL();

              has_y_coord = true;
            }

          } else if (action->lexeme->value == "STEP") {
            sub_action = action->actions[0];

            result_subtype = evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
            } else {
              addCast(result_subtype, Lexeme::subtype_numeric);
              // ld de, (0xFCB7)  ;GRPACX
              addLdDEii(0xFCB7);
              // add hl, de
              addAddHLDE();
              // ld (0xFCB7), hl  ;GRPACX
              addLdiiHL(0xFCB7);
              // push hl
              addPushHL();

              has_x_coord = true;
            }

            sub_action = action->actions[1];

            result_subtype = evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
            } else {
              addCast(result_subtype, Lexeme::subtype_numeric);
              // ld de, (0xFCB9)  ;GRPACY
              addLdDEii(0xFCB9);
              // add hl, de
              addAddHLDE();
              // ld (0xFCB9), hl  ;GRPACY
              addLdiiHL(0xFCB9);
              // push hl
              addPushHL();

              has_y_coord = true;
            }

          } else {
            if (forecolor)
              syntaxError("Invalid coordenates on PSET");
            else
              syntaxError("Invalid coordenates on PRESET");
            return;
          }

        } break;

        case 1: {
          result_subtype = evalExpression(action);

          if (result_subtype == Lexeme::subtype_null) continue;

          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld a, l
          addLdAL();

          // push af       ; save color
          addPushAF();

          has_color = true;

        } break;

        case 2: {
          result_subtype = evalExpression(action);

          if (result_subtype == Lexeme::subtype_null) continue;

          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld a, l
          addLdAL();

          // ld (LOGOPR), a          ; save new logical operator to basic
          // interpreter
          addLdiiA(def_LOGOPR);

          has_operator = true;

        } break;

        default: {
          if (forecolor)
            syntaxError("PSET parameters not supported");
          else
            syntaxError("PRESET parameters not supported");
          return;
        }
      }
    }

    if (!has_operator) {
      // ld a, (LOGOPR)      ; get default operator from basic interpreter
      addLdAii(def_LOGOPR);
    }

    // ld b, a      ; get operator
    addLdBA();

    if (has_color) {
      // pop af       ; color
      addPopAF();
    } else {
      if (forecolor) {
        // ld a, (0xF3E9)       ; FORCLR
        addLdAii(def_FORCLR);
      } else {
        // ld a, (0xF3EA)       ; BAKCLR
        addLdAii(def_BAKCLR);
      }
    }

    if (has_y_coord) {
      // pop hl
      addPopHL();
    } else {
      // ld hl, (0xFCB9)  ;GRPACY
      addLdHLii(0xFCB9);
    }

    if (has_x_coord) {
      // pop de
      addPopDE();
    } else {
      // ld de, (0xFCB7)  ;GRPACX
      addLdDEii(0xFCB7);
    }

    // call 0x6F71   ; xbasic PSET (in: hl=y, de=x, a=color, b=operator)
    addCall(def_XBASIC_PSET);

  } else {
    if (forecolor)
      syntaxError("PSET with empty parameters");
    else
      syntaxError("PRESET with empty parameters");
  }
}

void Compiler::cmd_paint() {
  ActionNode *action, *sub_action;
  unsigned int i, t = current_action->actions.size();
  int result_subtype;
  bool has_x_coord = false, has_y_coord = false, has_color = false,
       has_border = false;

  if (t) {
    for (i = 0; i < t; i++) {
      action = current_action->actions[i];

      switch (i) {
        case 0: {
          if (action->actions.size() != 2) {
            syntaxError("Coordenates parameters error on PAINT");
            return;
          } else if (action->lexeme->value == "COORD") {
            sub_action = action->actions[0];

            result_subtype = evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
            } else {
              addCast(result_subtype, Lexeme::subtype_numeric);

              // ld (0xFCB7), hl  ;GRPACX
              addLdiiHL(0xFCB7);
              // push hl
              addPushHL();

              has_x_coord = true;
            }

            sub_action = action->actions[1];

            result_subtype = evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
            } else {
              addCast(result_subtype, Lexeme::subtype_numeric);

              // ld (0xFCB9), hl  ;GRPACY
              addLdiiHL(0xFCB9);

              // push hl
              addPushHL();

              has_y_coord = true;
            }

          } else if (action->lexeme->value == "STEP") {
            sub_action = action->actions[0];

            result_subtype = evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
            } else {
              addCast(result_subtype, Lexeme::subtype_numeric);
              // ld de, (0xFCB7)  ;GRPACX
              addLdDEii(0xFCB7);
              // add hl, de
              addAddHLDE();
              // ld (0xFCB7), hl  ;GRPACX
              addLdiiHL(0xFCB7);
              // push hl
              addPushHL();

              has_x_coord = true;
            }

            sub_action = action->actions[1];

            result_subtype = evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
            } else {
              addCast(result_subtype, Lexeme::subtype_numeric);
              // ld de, (0xFCB9)  ;GRPACY
              addLdDEii(0xFCB9);
              // add hl, de
              addAddHLDE();
              // ld (0xFCB9), hl  ;GRPACY
              addLdiiHL(0xFCB9);
              // push hl
              addPushHL();

              has_y_coord = true;
            }

          } else {
            syntaxError("Invalid coordenates on PAINT");
            return;
          }

        } break;

        case 1: {
          result_subtype = evalExpression(action);

          if (result_subtype == Lexeme::subtype_null) continue;

          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld b, l       ; paint color
          addLdBL();

          has_color = true;

        } break;

        case 2: {
          if (has_color) {
            // push bc    ; save paint color
            addPushBC();
          }

          result_subtype = evalExpression(action);

          if (result_subtype == Lexeme::subtype_null) continue;

          addCast(result_subtype, Lexeme::subtype_numeric);

          has_border = true;

        } break;

        default: {
          syntaxError("PAINT parameters not supported");
          return;
        }
      }
    }

    if (has_border) {
      if (has_color) {
        // pop bc               ; restore paint color
        addPopBC();
      } else {
        // ld a, (ATRBYT)
        addLdAii(def_ATRBYT);
        // ld b, a              ; paint color = default color
        addLdBA();
      }
      //   ld a, l                ; border color
      addLdAL();

    } else {
      if (has_color) {
        // ld a, l              ; border color = paint color
        addLdAL();
      } else {
        // ld a, (ATRBYT)       ; border color = default color
        addLdAii(def_ATRBYT);
        // ld b, a              ; paint color = default color
        addLdBA();
      }
    }

    if (has_y_coord) {
      // pop hl
      addPopHL();
    } else {
      // ld hl, (0xFCB9)  ;GRPACY
      addLdHLii(0xFCB9);
    }

    if (has_x_coord) {
      // pop de
      addPopDE();
    } else {
      // ld de, (0xFCB7)  ;GRPACX
      addLdDEii(0xFCB7);
    }

    // call 0x74B3   ; xbasic PAINT (in: hl=y, de=x, b=filling color, a=border
    // color)
    addCall(def_XBASIC_PAINT);

  } else {
    syntaxError("PAINT with empty parameters");
  }
}

void Compiler::cmd_circle() {
  ActionNode *action, *sub_action;
  unsigned int i, t = current_action->actions.size();
  int result_subtype;
  bool has_x_coord = false, has_y_coord = false, has_radius = false,
       has_color = false;
  bool has_trace1 = false, has_trace2 = false, has_aspect = false;

  if (t) {
    for (i = 0; i < t; i++) {
      action = current_action->actions[i];

      switch (i) {
        // coord
        case 0: {
          if (action->actions.size() != 2) {
            syntaxError("Coordenates parameters error on CIRCLE");
            return;
          } else if (action->lexeme->value == "COORD") {
            sub_action = action->actions[0];

            result_subtype = evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
            } else {
              addCast(result_subtype, Lexeme::subtype_numeric);

              // ld (0xFCB7), hl  ;GRPACX
              addLdiiHL(0xFCB7);

              has_x_coord = true;
            }

            sub_action = action->actions[1];

            result_subtype = evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
            } else {
              addCast(result_subtype, Lexeme::subtype_numeric);

              // ld (0xFCB9), hl  ;GRPACY
              addLdiiHL(0xFCB9);

              has_y_coord = true;
            }

          } else if (action->lexeme->value == "STEP") {
            sub_action = action->actions[0];

            result_subtype = evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
            } else {
              addCast(result_subtype, Lexeme::subtype_numeric);
              // ld de, (0xFCB7)  ;GRPACX
              addLdDEii(0xFCB7);
              // add hl, de
              addAddHLDE();
              // ld (0xFCB7), hl  ;GRPACX
              addLdiiHL(0xFCB7);

              has_x_coord = true;
            }

            sub_action = action->actions[1];

            result_subtype = evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
            } else {
              addCast(result_subtype, Lexeme::subtype_numeric);
              // ld de, (0xFCB9)  ;GRPACY
              addLdDEii(0xFCB9);
              // add hl, de
              addAddHLDE();
              // ld (0xFCB9), hl  ;GRPACY
              addLdiiHL(0xFCB9);

              has_y_coord = true;
            }

          } else {
            syntaxError("Invalid coordenates on CIRCLE");
            return;
          }

        } break;

        // radius
        case 1: {
          result_subtype = evalExpression(action);

          if (result_subtype == Lexeme::subtype_null) continue;

          addCast(result_subtype, Lexeme::subtype_numeric);

          has_radius = true;

        } break;

        // color
        case 2: {
          if (has_radius) {
            // push hl
            addPushHL();
          }

          result_subtype = evalExpression(action);

          if (result_subtype == Lexeme::subtype_null) {
            if (has_radius) {
              code_pointer--;
              code_size--;
            }
            continue;
          }

          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld a, l
          addLdAL();

          has_color = true;

        } break;

        // tracing start
        case 3: {
          if (has_radius) {
            if (has_color) {
              // pop hl
              addPopHL();
            }
            // ld c, l
            addLdCL();
          }

          // ld hl, BUF
          addLdHL(def_BUF);
          // push hl
          addPushHL();
          //   ld (hl), 0x2C  ; comma
          // addLdiHL(0x2C);
          //   inc hl
          // addIncHL();

          if (has_radius) {
            //   ld (hl), 0x0F  ; short interger marker
            addLdiHL(0x0F);
            //   inc hl
            addIncHL();
            //   ld (hl), c     ; radius
            addLdiHLC();
            //   inc hl
            addIncHL();
          }

          //   ld (hl), 0x2C  ; comma
          addLdiHL(0x2C);
          //   inc hl
          addIncHL();

          if (has_color) {
            //   ld (hl), 0x0F  ; short interger marker
            addLdiHL(0x0F);
            //   inc hl
            addIncHL();
            //   ld (hl), a     ; color
            addLdiHLA();
            //   inc hl
            addIncHL();
          }

          //   ld (hl), 0x2C  ; comma
          addLdiHL(0x2C);
          //   inc hl
          addIncHL();

          // push hl
          addPushHL();

          result_subtype = evalExpression(action);

          if (result_subtype == Lexeme::subtype_null) {
            code_pointer--;
            code_size--;
            continue;
          }

          addCast(result_subtype, Lexeme::subtype_single_decimal);

          // pop de
          addByteOptimized(0xD1);

          // call WriteParamBCD             ; b:hl, de -> hl
          addCall(def_WriteParamBCD);

          has_trace1 = true;

        } break;

        // tracing end
        case 4: {
          //   ld (hl), 0x2C  ; comma
          addLdiHL(0x2C);
          //   inc hl
          addIncHL();

          // push hl
          addPushHL();

          result_subtype = evalExpression(action);

          if (result_subtype == Lexeme::subtype_null) {
            code_pointer--;
            code_size--;
            continue;
          }

          addCast(result_subtype, Lexeme::subtype_single_decimal);

          // pop de
          addByteOptimized(0xD1);

          // call WriteParamBCD             ; b:hl, de -> hl
          addCall(def_WriteParamBCD);

          has_trace2 = true;
        } break;

        // aspect ratio
        case 5: {
          //   ld (hl), 0x2C  ; comma
          addLdiHL(0x2C);
          //   inc hl
          addIncHL();

          // push hl
          addPushHL();

          result_subtype = evalExpression(action);

          if (result_subtype == Lexeme::subtype_null) {
            code_pointer -= 3;
            code_size -= 3;
            continue;
          }

          addCast(result_subtype, Lexeme::subtype_single_decimal);

          // pop de
          addByteOptimized(0xD1);

          // call WriteParamBCD             ; b:hl, de -> hl
          addCall(def_WriteParamBCD);

          has_aspect = true;
        } break;

        default: {
          syntaxError("Invalid CIRCLE parameters");
          return;
        }
      }
    }

    if (has_trace1 || has_trace2 || has_aspect) {
      //   ld (hl), 0x00
      addLdiHL(0x00);
      //   inc hl
      addIncHL();

      // pop hl
      addPopHL();

      // ld a, (hl)         ; first character
      addLdAiHL();
      // ld ix, M5B16       ; rom basic circle without coords
      addLdIX(0x5B16);
      // call xbasic CIRCLE2 (in: hl = basic line starting on radius parameter)
      addCall(def_XBASIC_CIRCLE2);

    } else {
      if (has_color && has_radius) {
        // pop hl
        addPopHL();
      } else if (!has_color) {
        // ld a, (0xF3E9)       ; FORCLR
        addLdAii(0xF3E9);
      }

      if (!has_radius) {
        // ld hl, 0
        addLdHL(0x0000);
      }

      if (!has_y_coord) {
      }

      if (!has_x_coord) {
      }

      // call xbasic CIRCLE (in: GRPACX/GRPACY, hl=radius, a=color)
      addCall(def_XBASIC_CIRCLE);
    }

  } else {
    syntaxError("CIRCLE with empty parameters");
  }
}

void Compiler::cmd_put() {
  ActionNode* action;
  Lexeme* lexeme;
  unsigned int t = current_action->actions.size();

  if (t) {
    action = current_action->actions[0];
    lexeme = action->lexeme;

    if (lexeme->type == Lexeme::type_keyword && lexeme->value == "SPRITE") {
      current_action = action;
      cmd_put_sprite();
    } else if (lexeme->type == Lexeme::type_keyword &&
               lexeme->value == "TILE") {
      current_action = action;
      cmd_put_tile();
    } else {
      syntaxError("Invalid PUT statement");
    }

  } else {
    syntaxError("Empty PUT statement");
  }
}

void Compiler::cmd_put_sprite() {
  ActionNode *action, *sub_action;
  unsigned int i, t = current_action->actions.size();
  int result_subtype, parm_flag = 0;
  bool has_sprite = false, has_x_coord = false, has_y_coord = false,
       has_color = false, has_pattern = false;

  if (t) {
    for (i = 0; i < t; i++) {
      action = current_action->actions[i];

      switch (i) {
        case 0: {
          result_subtype = evalExpression(action);

          if (result_subtype == Lexeme::subtype_null) continue;

          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld a, l
          addLdAL();
          // push af
          addPushAF();

          has_sprite = true;

        } break;

        case 1: {
          if (action->actions.size() != 2) {
            syntaxError("Coordenates parameters error on PUT SPRITE");
            return;
          } else if (action->lexeme->value == "COORD") {
            sub_action = action->actions[0];

            result_subtype = evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
            } else {
              addCast(result_subtype, Lexeme::subtype_numeric);

              // ld (0xFCB7), hl  ;GRPACX
              addLdiiHL(0xFCB7);

              // push hl
              addPushHL();

              has_x_coord = true;
            }

            sub_action = action->actions[1];

            result_subtype = evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
            } else {
              addCast(result_subtype, Lexeme::subtype_numeric);

              // ld (0xFCB9), hl  ;GRPACY
              addLdiiHL(0xFCB9);

              // push hl
              addPushHL();

              has_y_coord = true;
            }

          } else if (action->lexeme->value == "STEP") {
            sub_action = action->actions[0];

            result_subtype = evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
            } else {
              addCast(result_subtype, Lexeme::subtype_numeric);
              // ld de, (0xFCB7)  ;GRPACX
              addLdDEii(0xFCB7);
              // add hl, de
              addAddHLDE();
              // ld (0xFCB7), hl  ;GRPACX
              addLdiiHL(0xFCB7);

              // push hl
              addPushHL();

              has_x_coord = true;
            }

            sub_action = action->actions[1];

            result_subtype = evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
            } else {
              addCast(result_subtype, Lexeme::subtype_numeric);
              // ld de, (0xFCB9)  ;GRPACY
              addLdDEii(0xFCB9);
              // add hl, de
              addAddHLDE();
              // ld (0xFCB9), hl  ;GRPACY
              addLdiiHL(0xFCB9);

              // push hl
              addPushHL();

              has_y_coord = true;
            }

          } else {
            syntaxError("Invalid coordenates on PUT SPRITE");
            return;
          }

        } break;

        case 2: {
          result_subtype = evalExpression(action);

          if (result_subtype == Lexeme::subtype_null) continue;

          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld a, l
          addLdAL();
          // push af
          addPushAF();

          has_color = true;

        } break;

        case 3: {
          result_subtype = evalExpression(action);

          if (result_subtype == Lexeme::subtype_null) continue;

          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld a, l
          addLdAL();
          // push af
          addPushAF();

          has_pattern = true;

        } break;

        default: {
          syntaxError("PUT SPRITE parameters not supported");
          return;
        }
      }
    }

    if (has_pattern) {
      // pop hl
      addPopHL();
      parm_flag |= 0x20;
    } else {
      // ld h, 0
      // addLdHn(0x00);
    }

    if (has_color) {
      // pop de
      addPopDE();
      parm_flag |= 0x40;
    } else {
      // ld d, 0
      // addWord(0x16, 0x00);
    }

    if (has_y_coord) {
      // pop iy
      addPopIY();
      parm_flag |= 0x80;
    } else {
      // ld iy, 0x0000
      // addLdIYii(0x0000);
    }

    if (has_x_coord) {
      // pop ix
      addPopIX();
      parm_flag |= 0x80;
    } else {
      // ld ix, 0x0000
      // addLdIXnn(0x0000);
    }

    if (has_sprite) {
      // pop af
      addPopAF();
    } else {
      // xor a
      addXorA();
    }

    // ld b, parameters flag
    addLdB(parm_flag);

    // call xbasic PUT SPRITE (in: ix=x, iy=y, d=color, a=sprite number,
    // h=pattern number, b=parameters flag (b11100000)
    addCall(def_XBASIC_PUT_SPRITE);

  } else {
    syntaxError("PUT SPRITE with empty parameters");
  }
}

void Compiler::cmd_put_tile() {
  ActionNode *action, *sub_action;
  unsigned int i, t = current_action->actions.size();
  int result_subtype;
  bool has_tile = false, has_x_coord = false, has_y_coord = false;

  if (t) {
    for (i = 0; i < t; i++) {
      action = current_action->actions[i];

      switch (i) {
        case 0: {
          result_subtype = evalExpression(action);

          if (result_subtype == Lexeme::subtype_null) continue;

          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld a, l
          addLdAL();
          // push af
          addPushAF();

          has_tile = true;

        } break;

        case 1: {
          if (action->actions.size() != 2) {
            syntaxError("Coordenates parameters error on PUT TILE");
            return;
          } else if (action->lexeme->value == "COORD") {
            sub_action = action->actions[0];

            result_subtype = evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
            } else {
              addCast(result_subtype, Lexeme::subtype_numeric);

              // ld (0xFCB7), hl  ;GRPACX
              addLdiiHL(0xFCB7);

              // push hl
              addPushHL();

              has_x_coord = true;
            }

            sub_action = action->actions[1];

            result_subtype = evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
            } else {
              addCast(result_subtype, Lexeme::subtype_numeric);

              // ld (0xFCB9), hl  ;GRPACY
              addLdiiHL(0xFCB9);

              // push hl
              addPushHL();

              has_y_coord = true;
            }

          } else if (action->lexeme->value == "STEP") {
            sub_action = action->actions[0];

            result_subtype = evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
            } else {
              addCast(result_subtype, Lexeme::subtype_numeric);
              // ld de, (0xFCB7)  ;GRPACX
              addLdDEii(0xFCB7);
              // add hl, de
              addAddHLDE();
              // ld (0xFCB7), hl  ;GRPACX
              addLdiiHL(0xFCB7);

              // push hl
              addPushHL();

              has_x_coord = true;
            }

            sub_action = action->actions[1];

            result_subtype = evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
            } else {
              addCast(result_subtype, Lexeme::subtype_numeric);
              // ld de, (0xFCB9)  ;GRPACY
              addLdDEii(0xFCB9);
              // add hl, de
              addAddHLDE();
              // ld (0xFCB9), hl  ;GRPACY
              addLdiiHL(0xFCB9);

              // push hl
              addPushHL();

              has_y_coord = true;
            }

          } else {
            syntaxError("Invalid coordenates on PUT TILE");
            return;
          }

        } break;

        default: {
          syntaxError("PUT TILE parameters not supported");
          return;
        }
      }
    }

    if (has_y_coord) {
      // pop hl
      addPopHL();
      // inc l        ; y coord (1 based for bios)
      addIncL();
    } else {
      // ld a, (CSRY)
      addLdAii(def_CSRY);
      // ld l, a
      addLdLA();
    }

    if (has_x_coord) {
      // pop bc
      addPopBC();
      // ld h, c
      addLdHC();
      // inc h        ; x coord (1 based for bios)
      addIncH();
    } else {
      // ld a, (CSRX)
      addLdAii(def_CSRX);
      // ld h, a
      addLdHA();
    }

    //   call TileAddress   ; in hl=xy, out: hl
    addCall(def_tileAddress);

    if (has_tile) {
      // pop af
      addPopAF();
    } else {
      // xor a
      addXorA();
    }

    // call 0x70b5                  ; xbasic VPOKE (in: hl=address, a=byte)
    addCall(def_XBASIC_VPOKE);

  } else {
    syntaxError("PUT TILE with empty parameters");
  }
}

void Compiler::cmd_set() {
  ActionNode* action;
  Lexeme* next_lexeme;
  unsigned int t = current_action->actions.size();
  FixNode* mark;

  if (t == 1) {
    action = current_action->actions[0];
    next_lexeme = action->lexeme;

    if (next_lexeme->type == Lexeme::type_keyword) {
      if (next_lexeme->value == "TILE") {
        cmd_set_tile();
        return;
      } else if (next_lexeme->value == "SPRITE") {
        cmd_set_sprite();
        return;
      } else if (next_lexeme->value == "FONT") {
        cmd_set_font();
        return;
      }
    }

    // ld a, (BIOS VERSION)
    addLdAii(def_VERSION);
    // and a
    addAndA();
    // jp z, $                ; skip if MSX1
    mark = addMark();
    addJpZ(0x0000);

    if (next_lexeme->type == Lexeme::type_keyword &&
        next_lexeme->value == "ADJUST") {
      cmd_set_adjust();
    } else if (next_lexeme->type == Lexeme::type_keyword &&
               next_lexeme->value == "BEEP") {
      cmd_set_beep();
    } else if (next_lexeme->type == Lexeme::type_keyword &&
               next_lexeme->value == "DATE") {
      cmd_set_date();
    } else if (next_lexeme->type == Lexeme::type_keyword &&
               next_lexeme->value == "PAGE") {
      cmd_set_page();
    } else if (next_lexeme->type == Lexeme::type_keyword &&
               next_lexeme->value == "PASSWORD") {
      syntaxError("SET PASSWORD will not be supported for end-user security");
    } else if (next_lexeme->type == Lexeme::type_keyword &&
               next_lexeme->value == "PROMPT") {
      cmd_set_prompt();
    } else if (next_lexeme->type == Lexeme::type_keyword &&
               next_lexeme->value == "SCREEN") {
      cmd_set_screen();
    } else if (next_lexeme->type == Lexeme::type_keyword &&
               next_lexeme->value == "SCROLL") {
      cmd_set_scroll();
    } else if (next_lexeme->type == Lexeme::type_keyword &&
               next_lexeme->value == "TIME") {
      cmd_set_time();
    } else if (next_lexeme->type == Lexeme::type_keyword &&
               next_lexeme->value == "TITLE") {
      cmd_set_title();
    } else if (next_lexeme->type == Lexeme::type_keyword &&
               next_lexeme->value == "VIDEO") {
      cmd_set_video();
    } else {
      syntaxError("Invalid SET statement");
    }

    mark->symbol->address = code_pointer;

  } else {
    syntaxError("Wrong SET parameters count");
  }
}

void Compiler::beginBasicSetStmt(string name) {
  int i, l = name.size();
  char* s = (char*)name.c_str();

  // ld hl, BUF
  addLdHL(def_BUF);
  // push hl
  addPushHL();

  // ; set subcommand
  for (i = 0; i < l; i++) {
    addBasicChar(s[i]);
  }
}

void Compiler::endBasicSetStmt() {
  //   xor a
  addXorA();
  //   ld (hl), a
  addLdiHLA();
  //   inc hl
  addIncHL();
  //   ld (hl), a
  addLdiHLA();

  // pop hl
  addPopHL();

  // ld a, (VERSION)
  addLdAii(def_VERSION);
  // and a
  addAndA();
  // jr z, skip
  addJrZ(9);

  //   ld a, (hl)      ; first character
  addLdAiHL();
  //   ld ix, (SET)    ; SET
  addLdIXii(def_SET_STMT);
  //   call CALBAS
  addCall(def_CALBAS);
  //   ei
  addEI();

  // skip:
}

void Compiler::addBasicChar(char c) {
  // ld (hl), char
  addLdiHL(c);
  // inc hl
  addIncHL();
}

void Compiler::cmd_set_video() {
  ActionNode *action = current_action->actions[0], *sub_action;
  unsigned int i, t = action->actions.size();
  int result_subtype;

  if (t) {
    if (t > 7) {
      syntaxError("Invalid SET VIDEO parameters");
      return;
    }

    beginBasicSetStmt("VIDEO");

    for (i = 0; i < t; i++) {
      if (i) {
        // comma
        addBasicChar(',');
      }

      // push hl
      addPushHL();

      sub_action = action->actions[i];
      result_subtype = evalExpression(sub_action);

      if (result_subtype == Lexeme::subtype_null) {
        code_pointer--;
        code_size--;
        continue;
      }

      addCast(result_subtype, Lexeme::subtype_numeric);

      //   ld a, l
      addLdAL();
      //   and 3
      addAnd(0x03);
      //   inc a
      addIncA();
      //   or 0x10
      addOr(0x10);

      // pop hl
      addPopHL();

      // ld (hl), a
      addLdiHLA();
      // inc hl
      addIncHL();
    }

    endBasicSetStmt();

  } else {
    syntaxError("SET VIDEO with empty parameters");
  }
}

void Compiler::cmd_set_adjust() {
  ActionNode *action = current_action->actions[0], *sub_action;
  unsigned int i, t = action->actions.size();
  int result_subtype;

  if (t == 2) {
    // get parameters

    for (i = 0; i < t; i++) {
      if (i) {
        // push hl
        addPushHL();
      }
      // ld hl, parameter value
      sub_action = action->actions[i];
      result_subtype = evalExpression(sub_action);
      addCast(result_subtype, Lexeme::subtype_numeric);
    }

    // ex de,hl   ; DE = second parameter
    addExDEHL();
    // pop bc     ; BC = first parameter
    addPopBC();

    // build command string

    beginBasicSetStmt("ADJUST");

    // (
    addBasicChar('(');

    // integer prefix
    addBasicChar(0x1C);
    // ld (hl), c      ; first parameter
    addLdiHLC();
    // inc hl
    addIncHL();
    // ld (hl), b
    addLdiHLB();
    // inc hl
    addIncHL();

    // comma
    addBasicChar(',');

    // integer prefix
    addBasicChar(0x1C);
    // ld (hl), e      ; second parameter
    addLdiHLE();
    // inc hl
    addIncHL();
    // ld (hl), d
    addLdiHLD();
    // inc hl
    addIncHL();

    // )
    addBasicChar(')');

    endBasicSetStmt();

  } else {
    syntaxError("Wrong parameters count on SET ADJUST statement");
  }
}

void Compiler::cmd_set_screen() {
  ActionNode* action = current_action->actions[0];
  unsigned int t = action->actions.size();

  if (t == 0) {
    // build command string

    beginBasicSetStmt("");
    addBasicChar(0xC5);  // token for SCREEN

    endBasicSetStmt();

  } else {
    syntaxError("Wrong parameters count on SET SCREEN statement");
  }
}

void Compiler::cmd_set_beep() {
  ActionNode *action = current_action->actions[0], *sub_action;
  unsigned int i, t = action->actions.size();
  int result_subtype;

  if (t == 2) {
    // get parameters

    for (i = 0; i < t; i++) {
      if (i) {
        // push hl
        addPushHL();
      }
      // ld hl, parameter value
      sub_action = action->actions[i];
      result_subtype = evalExpression(sub_action);
      addCast(result_subtype, Lexeme::subtype_numeric);
    }

    // ex de,hl   ; DE = second parameter
    addExDEHL();
    // pop bc     ; BC = first parameter
    addPopBC();

    // build command string

    beginBasicSetStmt("");
    addBasicChar(0xC0);  // token for BEEP

    // short integer prefix
    addBasicChar(0x0F);
    // ld (hl), c      ; first parameter
    addLdiHLC();
    // inc hl
    addIncHL();

    // comma
    addBasicChar(',');

    // short integer prefix
    addBasicChar(0x0F);
    // ld (hl), e      ; second parameter
    addLdiHLE();
    // inc hl
    addIncHL();

    endBasicSetStmt();

  } else {
    syntaxError("Wrong parameters count on SET BEEP statement");
  }
}

void Compiler::cmd_set_title() {
  ActionNode *action = current_action->actions[0], *sub_action;
  unsigned int t = action->actions.size();
  int result_subtype;

  if (t == 2) {
    // get parameters

    // ld hl, parameter value
    sub_action = action->actions[1];
    result_subtype = evalExpression(sub_action);
    addCast(result_subtype, Lexeme::subtype_numeric);

    // push hl
    addPushHL();

    // ld hl, parameter value
    sub_action = action->actions[0];
    result_subtype = evalExpression(sub_action);
    addCast(result_subtype, Lexeme::subtype_string);

    // ex de,hl   ; DE = first parameter
    addExDEHL();
    // pop bc     ; BC = second parameter
    addPopBC();

    // build command string

    beginBasicSetStmt("TITLE");

    // double quote
    addBasicChar('"');
    // push bc
    addPushBC();
    // ex de,hl
    addExDEHL();
    //   ld c, (hl)  ; string size
    addLdCiHL();
    //   ld b, 0
    addLdB(0x00);
    //   inc hl
    addIncHL();
    //   ldir
    addLDIR();
    // ex de,hl
    addExDEHL();
    // pop bc
    addPopBC();
    // double quote
    addBasicChar('"');

    // comma
    addBasicChar(',');

    // short integer prefix
    addBasicChar(0x0F);
    // ld (hl), c      ; second parameter
    addLdiHLC();
    // inc hl
    addIncHL();

    endBasicSetStmt();

  } else {
    syntaxError("Wrong parameters count on SET TITLE statement");
  }
}

void Compiler::cmd_set_prompt() {
  ActionNode *action = current_action->actions[0], *sub_action;
  unsigned int t = action->actions.size();
  int result_subtype;

  if (t == 1) {
    // get parameters

    // ld hl, parameter value
    sub_action = action->actions[0];
    result_subtype = evalExpression(sub_action);
    addCast(result_subtype, Lexeme::subtype_string);

    // ex de,hl   ; DE = first parameter
    addExDEHL();

    // build command string

    beginBasicSetStmt("PROMPT");

    // double quote
    addBasicChar('"');
    // ex de,hl
    addExDEHL();
    //   ld c, (hl)  ; string size
    addLdCiHL();
    //   ld b, 0
    addLdB(0x00);
    //   inc hl
    addIncHL();
    //   ldir
    addLDIR();
    // ex de,hl
    addExDEHL();
    // double quote
    addBasicChar('"');

    endBasicSetStmt();

  } else {
    syntaxError("Wrong parameters count on SET PROMPT statement");
  }
}

void Compiler::cmd_set_page() {
  ActionNode *action = current_action->actions[0], *sub_action;
  Lexeme* lexeme;
  unsigned int t = action->actions.size();
  int result_subtype;

  if (t >= 1 && t <= 2) {
    // ld hl, parameter value
    sub_action = action->actions[0];
    lexeme = sub_action->lexeme;

    if (!(lexeme->type == Lexeme::type_literal &&
          lexeme->subtype == Lexeme::subtype_null)) {
      result_subtype = evalExpression(sub_action);
      addCast(result_subtype, Lexeme::subtype_numeric);

      // ld a,l
      addLdAL();

      // call SET_PAGE       ; in: a = display page
      addCall(def_XBASIC_SET_PAGE);
    }

    if (t == 2) {
      sub_action = action->actions[1];
      result_subtype = evalExpression(sub_action);
      addCast(result_subtype, Lexeme::subtype_numeric);

      //   ld a,l
      addLdAL();
      //   ld (ACPAGE), a    ; in: a = active page (write and read)
      addLdiiA(def_ACPAGE);
    }

  } else {
    syntaxError("Wrong parameters count on SET PAGE statement");
  }
}

void Compiler::cmd_set_scroll() {
  ActionNode *action = current_action->actions[0], *sub_action;
  Lexeme* lexeme;
  unsigned int i, t = action->actions.size();
  int result_subtype;

  if (t > 0 && t <= 4) {
    for (i = 0; i < t; i++) {
      sub_action = action->actions[i];
      lexeme = sub_action->lexeme;

      if (lexeme->type == Lexeme::type_literal &&
          lexeme->subtype == Lexeme::subtype_null) {
        // ld hl, 0xffff
        addLdHL(0xFFFF);
      } else {
        // ld hl, parameter value
        result_subtype = evalExpression(sub_action);
        addCast(result_subtype, Lexeme::subtype_numeric);
      }

      if (i > 1) {
        // ld h, l
        addLdHL();
      }

      // push hl
      addPushHL();
    }

    if (t == 4) {
      // pop af
      addPopAF();
    } else {
      // ld a, 0xff
      addLdA(0xFF);
    }

    if (t >= 3) {
      // pop bc
      addPopBC();
    } else {
      // ld b, 0xff
      addLdB(0xFF);
    }

    if (t >= 2) {
      // pop hl
      addPopHL();
    } else {
      // ld hl, 0xffff
      addLdHL(0xFFFF);
    }

    // pop de
    addPopDE();

    // call SET_SCROLL      ; in: de=x, hl=y, b=mask mode, a=page mode
    addCall(def_XBASIC_SET_SCROLL);

  } else {
    syntaxError("Wrong parameters count on SET SCROLL statement");
  }
}

void Compiler::cmd_set_tile() {
  ActionNode *action = current_action->actions[0], *sub_action, *sub_sub_action;
  Lexeme* lexeme;
  unsigned int i, t, tt;
  int result_subtype;

  t = action->actions.size();
  if (t) {
    action = action->actions[0];
    lexeme = action->lexeme;
    t = action->actions.size();

    if (lexeme->value == "ON") {
      // ld a, 2                   ; tiled mode
      addLdA(2);
      // ld (SOMODE), a
      addLdiiA(def_SOMODE);
      // clear screen, set font to default and put cursor on home
      cmd_cls();
      // ld hl, 0
      addLdHL(0x0000);
      // ld (DAC), hl
      addLdiiHL(def_DAC);
      // ld d, h
      addLdDH();
      // ld e, l
      addLdEL();
      // call XBASIC_LOCATE    ; hl = y, de = x
      addCall(def_XBASIC_LOCATE);
      // ld a, 0xff                ; it means all screen banks
      addLdA(0xFF);
      // ld (ARG), a
      addLdiiA(def_ARG);
      // call cmd_setfnt
      addCall(def_cmd_setfnt);

    } else if (lexeme->value == "OFF") {
      // ld a, 1      ; graphical mode
      addLdA(1);
      // ld (SOMODE), a
      addLdiiA(def_SOMODE);

    } else if (lexeme->value == "FLIP") {
      if (t == 2) {
        // tile number
        sub_action = action->actions[0];
        // ld hl, parameter value    ; parameter
        result_subtype = evalExpression(sub_action);
        addCast(result_subtype, Lexeme::subtype_numeric);
        // push hl
        addPushHL();

        // direction
        sub_action = action->actions[1];
        // ld hl, parameter value    ; parameter
        result_subtype = evalExpression(sub_action);
        addCast(result_subtype, Lexeme::subtype_numeric);
        // pop de
        addPopDE();

        addCall(def_set_tile_flip);

      } else {
        syntaxError("Wrong parameters count on SET TILE FLIP statement");
      }

    } else if (lexeme->value == "ROTATE") {
      if (t == 2) {
        // tile number
        sub_action = action->actions[0];
        // ld hl, parameter value    ; parameter
        result_subtype = evalExpression(sub_action);
        addCast(result_subtype, Lexeme::subtype_numeric);
        // push hl
        addPushHL();

        // direction
        sub_action = action->actions[1];
        // ld hl, parameter value    ; parameter
        result_subtype = evalExpression(sub_action);
        addCast(result_subtype, Lexeme::subtype_numeric);
        // pop de
        addPopDE();

        addCall(def_set_tile_rotate);

      } else {
        syntaxError("Wrong parameters count on SET TILE ROTATE statement");
      }

    } else if (lexeme->value == "PATTERN") {
      if (t >= 2 && t <= 3) {
        // tile number
        sub_action = action->actions[0];
        // ld hl, parameter value    ; tile number
        result_subtype = evalExpression(sub_action);
        addCast(result_subtype, Lexeme::subtype_numeric);
        // ld (ARG), hl
        addLdiiHL(def_ARG);

        // bank number
        if (t == 3) {
          sub_action = action->actions[2];
          // ld hl, parameter value    ; tile number
          result_subtype = evalExpression(sub_action);
          addCast(result_subtype, Lexeme::subtype_numeric);
          // ld h, l
          addLdHL();
        } else {
          // ld h, 0x03
          addLdH(0x03);
        }
        // ld (ARG2), hl
        addLdiiHL(def_ARG2);

        // pattern data
        sub_action = action->actions[1];
        lexeme = sub_action->lexeme;

        if (lexeme->value == "ARRAY") {
          tt = sub_action->actions.size();

          for (i = 0; i < tt; i++) {
            sub_sub_action = sub_action->actions[i];
            lexeme = sub_sub_action->lexeme;

            if (lexeme->type == Lexeme::type_literal &&
                lexeme->subtype == Lexeme::subtype_null) {
              continue;

            } else {
              // ld hl, parameter value    ; pattern data parameter
              result_subtype = evalExpression(sub_sub_action);
              addCast(result_subtype, Lexeme::subtype_numeric);

              // ld bc, (ARG2)
              addLdBCii(def_ARG2);
              // ld c, l
              addLdCL();
              // ld de, *i*
              addLdDE(i);
              // ld hl, (ARG)
              addLdHLii(def_ARG);

              // call set_tile_pattern ; hl = tile number, de = line number, b =
              // bank number (3=all), c = pattern data
              addCall(def_set_tile_pattern);
            }
          }

        } else {
          syntaxError("Wrong pattern parameter on SET TILE PATTERN statement");
        }

      } else {
        syntaxError("Wrong parameters count on SET TILE PATTERN statement");
      }

    } else if (lexeme->value == "COLOR") {
      if (t >= 2 && t <= 4) {
        // tile number
        sub_action = action->actions[0];
        // ld hl, parameter value    ; tile number
        result_subtype = evalExpression(sub_action);
        addCast(result_subtype, Lexeme::subtype_numeric);
        // ld (ARG), hl
        addLdiiHL(def_ARG);

        // bank number
        if (t == 4) {
          sub_action = action->actions[3];
          // ld hl, parameter value    ; tile number
          result_subtype = evalExpression(sub_action);
          addCast(result_subtype, Lexeme::subtype_numeric);
          // ld h, l
          addLdHL();
        } else {
          // ld h, 0x03
          addLdH(0x03);
        }
        // ld (ARG2), hl
        addLdiiHL(def_ARG2);

        // color data
        sub_action = action->actions[1];
        lexeme = sub_action->lexeme;

        if (lexeme->value == "ARRAY") {
          tt = sub_action->actions.size();

          for (i = 0; i < tt; i++) {
            sub_sub_action = sub_action->actions[i];
            lexeme = sub_sub_action->lexeme;

            if (lexeme->type == Lexeme::type_literal &&
                lexeme->subtype == Lexeme::subtype_null) {
              continue;

            } else {
              // ld hl, parameter value    ; color FC data parameter
              result_subtype = evalExpression(sub_sub_action);
              addCast(result_subtype, Lexeme::subtype_numeric);
              // ld a, l
              addLdAL();
              // rla
              addRLA();
              // rla
              addRLA();
              // rla
              addRLA();
              // rla
              addRLA();
              // and 0xF0
              addAnd(0xF0);

              if (t >= 3) {
                // color data
                sub_sub_action = action->actions[2];
                lexeme = sub_sub_action->lexeme;
                if (lexeme->value != "ARRAY") {
                  syntaxError(
                      "Syntax not supported on SET TILE COLOR statement");
                  return;
                }
                if (i < sub_sub_action->actions.size()) {
                  sub_sub_action = sub_sub_action->actions[i];
                  lexeme = sub_sub_action->lexeme;
                  if (!(lexeme->type == Lexeme::type_literal &&
                        lexeme->subtype == Lexeme::subtype_null)) {
                    // push af
                    addPushAF();
                    // ld hl, parameter value    ; color BC data parameter
                    result_subtype = evalExpression(sub_sub_action);
                    addCast(result_subtype, Lexeme::subtype_numeric);
                    // pop af
                    addPopAF();
                    // or l
                    addOrL();
                  }
                }
              }

              // ld bc, (ARG2)
              addLdBCii(def_ARG2);
              // ld c, a
              addLdCA();
              // ld de, *i*
              addLdDE(i);
              // ld hl, (ARG)        ; tile number
              addLdHLii(def_ARG);

              // call set_tile_color ; hl = tile number, de = line number
              // (15=all), b = bank number (3=all), c = color data (FC,BC)
              addCall(def_set_tile_color);
            }
          }

        } else {
          // ld hl, parameter value    ; color FC data parameter
          result_subtype = evalExpression(sub_action);
          addCast(result_subtype, Lexeme::subtype_numeric);
          // ld a, l
          addLdAL();
          // rla
          addRLA();
          // rla
          addRLA();
          // rla
          addRLA();
          // rla
          addRLA();
          // and 0xF0
          addAnd(0xF0);

          if (t >= 3) {
            // color data
            sub_sub_action = action->actions[2];
            lexeme = sub_sub_action->lexeme;
            if (lexeme->value == "ARRAY") {
              syntaxError("Syntax not supported on SET TILE COLOR statement");
              return;
            }
            // push af
            addPushAF();
            // ld hl, parameter value    ; color BC data parameter
            result_subtype = evalExpression(sub_sub_action);
            addCast(result_subtype, Lexeme::subtype_numeric);
            // pop af
            addPopAF();
            // or l
            addOrL();
          }

          // ld bc, (ARG2)
          addLdBCii(def_ARG2);
          // ld c, a
          addLdCA();
          // ld de, 0x000F         ; all lines
          addLdDE(0x000F);
          // ld hl, (ARG)        ; tile number
          addLdHLii(def_ARG);

          // call set_tile_color ; hl = tile number, de = line number (15=all),
          // b = bank number (3=all), c = color data (FC,BC)
          addCall(def_set_tile_color);
        }

      } else {
        syntaxError("Wrong parameters count on SET TILE COLOR statement");
      }

    } else {
      syntaxError("Invalid syntax on SET TILE statement");
    }

  } else {
    syntaxError("Missing parameters on SET TILE statement");
  }
}

void Compiler::cmd_set_font() {
  ActionNode *action = current_action->actions[0], *sub_action1, *sub_action2;
  unsigned int t;
  int result_subtype;

  t = action->actions.size();
  if (t) {
    font = true;

    if (t == 1) {
      sub_action1 = action->actions[0];
      result_subtype = evalExpression(sub_action1);
      addCast(result_subtype, Lexeme::subtype_numeric);

      // ld (DAC), hl
      addLdiiHL(def_DAC);

      // ld a, 0xff                ; it means all screen banks
      addLdA(0xFF);
      // ld (ARG), a
      addLdiiA(def_ARG);

      // call cmd_setfnt
      addCall(def_cmd_setfnt);

    } else if (t == 2) {
      sub_action1 = action->actions[0];
      result_subtype = evalExpression(sub_action1);
      addCast(result_subtype, Lexeme::subtype_numeric);

      // ld (DAC), hl
      addLdiiHL(def_DAC);

      sub_action2 = action->actions[1];
      result_subtype = evalExpression(sub_action2);
      addCast(result_subtype, Lexeme::subtype_numeric);

      // ld (ARG), hl             ; screen font bank number
      addLdiiHL(def_ARG);

      // call cmd_setfnt
      addCall(def_cmd_setfnt);

    } else {
      syntaxError("Wrong number of parameters on SET FONT");
    }

  } else {
    syntaxError("SET FONT syntax error");
  }
}

void Compiler::cmd_set_sprite() {
  ActionNode *action = current_action->actions[0], *sub_action, *sub_sub_action;
  Lexeme* lexeme;
  unsigned int i, t, tt;
  int result_subtype;

  t = action->actions.size();
  if (t) {
    action = action->actions[0];
    lexeme = action->lexeme;
    t = action->actions.size();

    if (lexeme->value == "FLIP") {
      if (t == 2) {
        // sprite number
        sub_action = action->actions[0];
        // ld hl, parameter value    ; parameter
        result_subtype = evalExpression(sub_action);
        addCast(result_subtype, Lexeme::subtype_numeric);
        // push hl
        addPushHL();

        // direction
        sub_action = action->actions[1];
        // ld hl, parameter value    ; parameter
        result_subtype = evalExpression(sub_action);
        addCast(result_subtype, Lexeme::subtype_numeric);
        // pop de
        addPopDE();

        addCall(def_set_sprite_flip);

      } else {
        syntaxError("Wrong parameters count on SET SPRITE FLIP statement");
      }

    } else if (lexeme->value == "ROTATE") {
      if (t == 2) {
        // sprite number
        sub_action = action->actions[0];
        // ld hl, parameter value    ; parameter
        result_subtype = evalExpression(sub_action);
        addCast(result_subtype, Lexeme::subtype_numeric);
        // push hl
        addPushHL();

        // direction
        sub_action = action->actions[1];
        // ld hl, parameter value    ; parameter
        result_subtype = evalExpression(sub_action);
        addCast(result_subtype, Lexeme::subtype_numeric);
        // pop de
        addPopDE();

        addCall(def_set_sprite_rotate);

      } else {
        syntaxError("Wrong parameters count on SET SPRITE ROTATE statement");
      }

    } else if (lexeme->value == "PATTERN") {
      if (t >= 2 && t <= 3) {
        // tile number
        sub_action = action->actions[0];
        // ld hl, parameter value    ; tile number
        result_subtype = evalExpression(sub_action);
        addCast(result_subtype, Lexeme::subtype_numeric);
        // ld (ARG), hl
        addLdiiHL(def_ARG);

        // bank number
        if (t == 3) {
          sub_action = action->actions[2];
          // ld hl, parameter value    ; tile number
          result_subtype = evalExpression(sub_action);
          addCast(result_subtype, Lexeme::subtype_numeric);
          // ld h, l
          addLdHL();
        } else {
          // ld h, 0x03
          addLdH(0x03);
        }
        // ld (ARG2), hl
        addLdiiHL(def_ARG2);

        // pattern data
        sub_action = action->actions[1];
        lexeme = sub_action->lexeme;

        if (lexeme->value == "ARRAY") {
          tt = sub_action->actions.size();

          for (i = 0; i < tt; i++) {
            sub_sub_action = sub_action->actions[i];
            lexeme = sub_sub_action->lexeme;

            if (lexeme->type == Lexeme::type_literal &&
                lexeme->subtype == Lexeme::subtype_null) {
              continue;

            } else {
              // ld hl, parameter value    ; pattern data parameter
              result_subtype = evalExpression(sub_sub_action);
              addCast(result_subtype, Lexeme::subtype_numeric);

              // ld bc, (ARG2)
              addLdBCii(def_ARG2);
              // ld c, l
              addLdCL();
              // ld de, *i*
              addLdDE(i);
              // ld hl, (ARG)
              addLdHLii(def_ARG);

              // call set_tile_pattern ; hl = tile number, de = line number, b =
              // bank number (3=all), c = pattern data
              addCall(def_set_sprite_pattern);
            }
          }

        } else {
          syntaxError(
              "Wrong pattern parameter on SET SPRITE PATTERN statement");
        }

      } else {
        syntaxError("Wrong parameters count on SET SPRITE PATTERN statement");
      }

    } else if (lexeme->value == "COLOR") {
      if (t >= 2 && t <= 4) {
        // tile number
        sub_action = action->actions[0];
        // ld hl, parameter value    ; tile number
        result_subtype = evalExpression(sub_action);
        addCast(result_subtype, Lexeme::subtype_numeric);
        // ld (ARG), hl
        addLdiiHL(def_ARG);

        // bank number
        if (t == 4) {
          sub_action = action->actions[3];
          // ld hl, parameter value    ; tile number
          result_subtype = evalExpression(sub_action);
          addCast(result_subtype, Lexeme::subtype_numeric);
          // ld h, l
          addLdHL();
        } else {
          // ld h, 0x03
          addLdH(0x03);
        }
        // ld (ARG2), hl
        addLdiiHL(def_ARG2);

        // color data
        sub_action = action->actions[1];
        lexeme = sub_action->lexeme;

        if (lexeme->value == "ARRAY") {
          tt = sub_action->actions.size();

          for (i = 0; i < tt; i++) {
            sub_sub_action = sub_action->actions[i];
            lexeme = sub_sub_action->lexeme;

            if (lexeme->type == Lexeme::type_literal &&
                lexeme->subtype == Lexeme::subtype_null) {
              continue;

            } else {
              // ld hl, parameter value    ; color FC data parameter
              result_subtype = evalExpression(sub_sub_action);
              addCast(result_subtype, Lexeme::subtype_numeric);
              // ld a, l
              addLdAL();
              // rla
              addRLA();
              // rla
              addRLA();
              // rla
              addRLA();
              // rla
              addRLA();
              // and 0xF0
              addAnd(0xF0);

              if (t >= 3) {
                // color data
                sub_sub_action = action->actions[2];
                lexeme = sub_sub_action->lexeme;
                if (lexeme->value != "ARRAY") {
                  syntaxError(
                      "Syntax not supported on SET SPRITE COLOR statement");
                  return;
                }
                if (i < sub_sub_action->actions.size()) {
                  sub_sub_action = sub_sub_action->actions[i];
                  lexeme = sub_sub_action->lexeme;
                  if (!(lexeme->type == Lexeme::type_literal &&
                        lexeme->subtype == Lexeme::subtype_null)) {
                    // push af
                    addPushAF();
                    // ld hl, parameter value    ; color BC data parameter
                    result_subtype = evalExpression(sub_sub_action);
                    addCast(result_subtype, Lexeme::subtype_numeric);
                    // pop af
                    addPopAF();
                    // or l
                    addOrL();
                  }
                }
              }

              // ld bc, (ARG2)
              addLdBCii(def_ARG2);
              // ld c, a
              addLdCA();
              // ld de, *i*
              addLdDE(i);
              // ld hl, (ARG)        ; tile number
              addLdHLii(def_ARG);

              // call set_tile_color ; hl = tile number, de = line number
              // (15=all), b = bank number (3=all), c = color data (FC,BC)
              addCall(def_set_sprite_color);
            }
          }

        } else {
          // ld hl, parameter value    ; color FC data parameter
          result_subtype = evalExpression(sub_action);
          addCast(result_subtype, Lexeme::subtype_numeric);
          // ld a, l
          addLdAL();
          // rla
          addRLA();
          // rla
          addRLA();
          // rla
          addRLA();
          // rla
          addRLA();
          // and 0xF0
          addAnd(0xF0);

          if (t >= 3) {
            // color data
            sub_sub_action = action->actions[2];
            lexeme = sub_sub_action->lexeme;
            if (lexeme->value == "ARRAY") {
              syntaxError("Syntax not supported on SET SPRITE COLOR statement");
              return;
            }
            // push af
            addPushAF();
            // ld hl, parameter value    ; color BC data parameter
            result_subtype = evalExpression(sub_sub_action);
            addCast(result_subtype, Lexeme::subtype_numeric);
            // pop af
            addPopAF();
            // or l
            addOrL();
          }

          // ld bc, (ARG2)
          addLdBCii(def_ARG2);
          // ld c, a
          addLdCA();
          // ld de, 0x000F         ; all lines
          addLdDE(0x000F);
          // ld hl, (ARG)        ; tile number
          addLdHLii(def_ARG);

          // call set_tile_color ; hl = tile number, de = line number (15=all),
          // b = bank number (3=all), c = color data (FC,BC)
          addCall(def_set_tile_color);
        }

      } else {
        syntaxError("Wrong parameters count on SET SPRITE COLOR statement");
      }

    } else {
      syntaxError("Invalid syntax on SET SPRITE statement");
    }

  } else {
    syntaxError("Missing parameters on SET SPRITE statement");
  }
}

void Compiler::cmd_set_date() {
  Lexeme* lexeme;
  ActionNode *action = current_action->actions[0], *sub_action;
  unsigned int i, t = action->actions.size();
  int result_subtype;

  if (t == 3) {
    for (i = 0; i < t; i++) {
      sub_action = action->actions[i];
      lexeme = sub_action->lexeme;
      if (lexeme) {
        result_subtype = evalExpression(sub_action);
        addCast(result_subtype, Lexeme::subtype_numeric);

        switch (i) {
          case 0: {
            addPushHL();
          } break;
          case 1: {
            addLdDL();
            addPushDE();
          } break;
          case 2: {
            addPopDE();
            addLdEL();
            addPopHL();
          } break;
        }
      }
    }

    addCall(def_set_date);

  } else {
    syntaxError(
        "Wrong SET DATE parameters count.\nTry: SET DATE iYear, iMonth, iDay");
  }
}

void Compiler::cmd_set_time() {
  Lexeme* lexeme;
  ActionNode *action = current_action->actions[0], *sub_action;
  unsigned int i, t = action->actions.size();
  int result_subtype;

  if (t == 3) {
    for (i = 0; i < t; i++) {
      sub_action = action->actions[i];
      lexeme = sub_action->lexeme;
      if (lexeme) {
        result_subtype = evalExpression(sub_action);
        addCast(result_subtype, Lexeme::subtype_numeric);

        switch (i) {
          case 0: {
            addLdHL();
            addPushHL();
          } break;
          case 1: {
            addPopDE();
            addLdEL();
            addPushDE();
          } break;
          case 2: {
            addLdAL();
            addPopHL();
          } break;
        }
      }
    }

    addCall(def_set_time);

  } else {
    syntaxError(
        "Wrong SET TIME parameters count.\nTry: SET TIME iHour, iMinute, "
        "iSecond");
  }
}

void Compiler::cmd_get() {
  ActionNode* action;
  Lexeme* next_lexeme;
  unsigned int t = current_action->actions.size();

  if (t == 1) {
    action = current_action->actions[0];
    next_lexeme = action->lexeme;

    if (next_lexeme->type == Lexeme::type_keyword) {
      if (next_lexeme->value == "DATE") {
        cmd_get_date();
      } else if (next_lexeme->value == "TIME") {
        cmd_get_time();
      } else {
        syntaxError("Invalid GET statement");
      }
    } else {
      syntaxError("Invalid GET statement");
    }

  } else {
    syntaxError("Wrong GET parameters count");
  }
}

void Compiler::cmd_get_date() {
  Lexeme* lexeme;
  ActionNode *action = current_action->actions[0], *sub_action;
  unsigned int i, t = action->actions.size();

  if (t >= 3 && t <= 5) {
    addCall(def_get_date);
    addLdB(0);
    if (t > 4) {
      addExAF();
      addLdAii(0x002B);
      addLdCA();
      addPushBC();  // date format
      addExAF();
    }
    if (t > 3) {
      addLdCA();
      addPushBC();  // week
    }
    addLdCE();
    addPushBC();  // day
    addLdCD();
    addPushBC();  // month
    addPushHL();  // year

    for (i = 0; i < t; i++) {
      sub_action = action->actions[i];
      lexeme = sub_action->lexeme;
      if (lexeme) {
        if (lexeme->type == Lexeme::type_identifier) {
          if (lexeme->subtype == Lexeme::subtype_numeric) {
            addVarAddress(sub_action);
            addPopDE();
            addLdiHLE();
            addIncHL();
            addLdiHLD();

          } else {
            syntaxError(
                "Invalid GET DATE parameter type.\nTry: GET DATE iYear, "
                "iMonth, iDay, iWeek, "
                "iDateFmt");
            return;
          }

        } else {
          syntaxError(
              "Invalid GET DATE parameter: it must be an integer variable.");
          return;
        }
      }
    }

  } else {
    syntaxError(
        "Wrong GET DATE parameters count.\nTry: GET DATE iYear, iMonth, iDay, "
        "iWeek, iDateFmt");
  }
}

void Compiler::cmd_get_time() {
  Lexeme* lexeme;
  ActionNode *action = current_action->actions[0], *sub_action;
  unsigned int i, t = action->actions.size();

  if (t == 3) {
    addCall(def_get_time);
    addLdB(0);
    addLdCA();
    addPushBC();  // seconds
    addLdCL();
    addPushBC();  // minutes
    addLdCH();
    addPushBC();  // hours

    for (i = 0; i < t; i++) {
      sub_action = action->actions[i];
      lexeme = sub_action->lexeme;
      if (lexeme) {
        if (lexeme->type == Lexeme::type_identifier) {
          if (lexeme->subtype == Lexeme::subtype_numeric) {
            addVarAddress(sub_action);
            addPopDE();
            addLdiHLE();
            addIncHL();
            addLdiHLD();

          } else {
            syntaxError(
                "Invalid GET TIME parameter type.\nTry: GET TIME iHour, "
                "iMinute, iSecond");
            return;
          }

        } else {
          syntaxError(
              "Invalid GET TIME parameter: it must be an integer variable.");
          return;
        }
      }
    }

  } else {
    syntaxError(
        "Wrong GET TIME parameters count.\nTry: GET TIME iHour, iMinute, "
        "iSecond");
  }
}

void Compiler::cmd_on() {
  ActionNode* action;
  Lexeme* next_lexeme;
  unsigned int t = current_action->actions.size();

  if (t) {
    action = current_action->actions[0];
    next_lexeme = action->lexeme;

    if (next_lexeme->type == Lexeme::type_keyword &&
        next_lexeme->value == "ERROR") {
      cmd_on_error();
    } else if (next_lexeme->type == Lexeme::type_keyword &&
               next_lexeme->value == "INTERVAL") {
      cmd_on_interval();
    } else if (next_lexeme->type == Lexeme::type_keyword &&
               next_lexeme->value == "KEY") {
      cmd_on_key();
    } else if (next_lexeme->type == Lexeme::type_keyword &&
               next_lexeme->value == "SPRITE") {
      cmd_on_sprite();
    } else if (next_lexeme->type == Lexeme::type_keyword &&
               next_lexeme->value == "STOP") {
      cmd_on_stop();
    } else if (next_lexeme->type == Lexeme::type_keyword &&
               next_lexeme->value == "STRIG") {
      cmd_on_strig();
    } else if (next_lexeme->type == Lexeme::type_keyword &&
               next_lexeme->value == "INDEX") {
      cmd_on_goto_gosub();
    } else {
      syntaxError("Invalid ON statement");
    }

  } else {
    syntaxError("Empty ON statement");
  }
}

void Compiler::cmd_on_error() {
  syntaxError("Not implemented yet");
}

void Compiler::cmd_on_interval() {
  ActionNode *action, *sub_action, *parm_action;
  Lexeme *lexeme, *parm_lexeme;
  unsigned int t;
  int result_subtype;

  action = current_action->actions[0];
  t = action->actions.size();

  if (t == 2) {
    // INDEX VARIABLE

    sub_action = action->actions[0];
    lexeme = sub_action->lexeme;
    if (lexeme->value != "INDEX") {
      syntaxError("Interval index is missing in ON INTERVAL");
      return;
    }
    if (sub_action->actions.size() != 1) {
      syntaxError("Wrong parameter count in interval index from ON INTERVAL");
      return;
    }

    parm_action = sub_action->actions[0];

    // ld hl, variable
    result_subtype = evalExpression(parm_action);

    addCast(result_subtype, Lexeme::subtype_numeric);

    // di
    addDI();
    //   ld (0xFCA0), hl   ; INTVAL
    addLdiiHL(0xFCA0);
    //   xor a
    addXorA();
    //   ld (0xFC7F), a    ; ON INTERVAL STATE (0=off, 1=on)
    addLdiiA(0xFC7F);
    //   ld (0xFCA3), a    ; INTCNT - initialize with zero (2 bytes)
    addLdiiA(0xFCA3);
    addLdiiA(0xFCA4);
    // ei
    addEI();

    // GOSUB

    sub_action = action->actions[1];
    lexeme = sub_action->lexeme;
    if (lexeme->value != "GOSUB") {
      syntaxError("GOSUB is missing in ON INTERVAL");
      return;
    }
    if (sub_action->actions.size() != 1) {
      syntaxError("Wrong parameter count in GOSUB from ON INTERVAL");
      return;
    }

    parm_action = sub_action->actions[0];
    parm_lexeme = parm_action->lexeme;

    if (parm_lexeme->type == Lexeme::type_literal &&
        parm_lexeme->subtype == Lexeme::subtype_numeric) {
      if (opts->megaROM) {
        // ld hl, GOSUB ADDRESS
        addFix(parm_lexeme->value);
        addLdHLmegarom();
        // ld (0xFC80), hl                ; INTERVAL ADDRESS
        addLdiiHL(0xFC80);
        // ld (MR_TRAP_SEGMS+17), a       ; INTERVAL segment
        addLdiiA(def_MR_TRAP_SEGMS + 17);

      } else {
        // ld hl, GOSUB ADDRESS
        addFix(parm_lexeme->value);
        addLdHL(0x0000);
        // ld (0xFC80), hl   ; GOSUB ADDRESS
        addLdiiHL(0xFC80);
      }

    } else {
      syntaxError("Invalid GOSUB parameter in ON INTERVAL");
      return;
    }

  } else {
    syntaxError("ON INTERVAL with empty parameters");
  }
}

void Compiler::cmd_interval() {
  ActionNode* action;
  Lexeme* next_lexeme;
  unsigned int t = current_action->actions.size();

  if (t == 1) {
    action = current_action->actions[0];
    next_lexeme = action->lexeme;

    // ld hl, 0xFC7F    ; INTERVAL state
    addLdHL(0xfc7f);

    if (next_lexeme->type == Lexeme::type_keyword &&
        next_lexeme->value == "ON") {
      // call 0x6c89   ; xbasic turn on trap (hl=trap state address)
      addCall(def_XBASIC_TRAP_ON);
    } else if (next_lexeme->type == Lexeme::type_keyword &&
               next_lexeme->value == "OFF") {
      // call 0x6c9c   ; xbasic turn off trap (hl=trap state address)
      addCall(def_XBASIC_TRAP_OFF);
    } else if (next_lexeme->type == Lexeme::type_keyword &&
               next_lexeme->value == "STOP") {
      // call 0x6ca5   ; xbasic turn stop trap (hl=trap state address)
      addCall(def_XBASIC_TRAP_STOP);
    } else {
      syntaxError("Invalid INTERVAL statement");
    }

  } else {
    syntaxError("Empty INTERVAL statement");
  }
}

void Compiler::cmd_stop() {
  ActionNode* action;
  Lexeme* next_lexeme;
  unsigned int t = current_action->actions.size();

  if (t == 1) {
    action = current_action->actions[0];
    next_lexeme = action->lexeme;

    // ld hl, 0xFC6A    ; STOP state
    addLdHL(0xFC6A);

    if (next_lexeme->type == Lexeme::type_keyword &&
        next_lexeme->value == "ON") {
      // call 0x6c89   ; xbasic turn on trap (hl=trap state address)
      addCall(def_XBASIC_TRAP_ON);
    } else if (next_lexeme->type == Lexeme::type_keyword &&
               next_lexeme->value == "OFF") {
      // call 0x6c9c   ; xbasic turn off trap (hl=trap state address)
      addCall(def_XBASIC_TRAP_OFF);
    } else if (next_lexeme->type == Lexeme::type_keyword &&
               next_lexeme->value == "STOP") {
      // call 0x6ca5   ; xbasic turn stop trap (hl=trap state address)
      addCall(def_XBASIC_TRAP_STOP);
    } else {
      syntaxError("Invalid STOP statement");
    }

  } else if (t == 0) {
    cmd_end(false);  //! jump to the real END statement
  } else {
    syntaxError("Wrong number of parameters in STOP");
  }
}

void Compiler::cmd_sprite() {
  ActionNode* action;
  Lexeme* next_lexeme;
  unsigned int t = current_action->actions.size();

  if (t == 1) {
    action = current_action->actions[0];
    next_lexeme = action->lexeme;

    if (next_lexeme->value == "LOAD") {
      current_action = action;
      cmd_sprite_load();
      return;
    }

    // ld hl, 0xFC6D   ; SPRITE state
    addLdHL(0xFC6D);

    if (next_lexeme->type == Lexeme::type_keyword &&
        next_lexeme->value == "ON") {
      // call 0x6c89   ; xbasic turn on trap (hl=trap state address)
      addCall(def_XBASIC_TRAP_ON);
    } else if (next_lexeme->type == Lexeme::type_keyword &&
               next_lexeme->value == "OFF") {
      // call 0x6c9c   ; xbasic turn off trap (hl=trap state address)
      addCall(def_XBASIC_TRAP_OFF);
    } else if (next_lexeme->type == Lexeme::type_keyword &&
               next_lexeme->value == "STOP") {
      // call 0x6ca5   ; xbasic turn stop trap (hl=trap state address)
      addCall(def_XBASIC_TRAP_STOP);
    } else {
      syntaxError("Invalid SPRITE statement");
    }

  } else {
    syntaxError("Empty SPRITE statement");
  }
}

void Compiler::cmd_sprite_load() {
  ActionNode* action;
  unsigned int i, t = current_action->actions.size();
  int result_subtype;

  if (t) {
    if (t > 1) {
      syntaxError("SPRITE LOAD with excess of parameters");
      return;
    }

    for (i = 0; i < t; i++) {
      action = current_action->actions[i];
      result_subtype = evalExpression(action);
      addCast(result_subtype, Lexeme::subtype_numeric);

      // ld (DAC), hl
      addLdiiHL(def_DAC);
      // call cmd_wrtspr                    ; tiny sprite loader
      addCall(def_cmd_wrtspr);
    }

  } else {
    syntaxError("SPRITE LOAD with empty parameters");
  }
}

void Compiler::cmd_key() {
  ActionNode *action1, *action2;
  Lexeme* next_lexeme;
  unsigned int t = current_action->actions.size();
  int result_subtype;

  if (t == 1) {
    action1 = current_action->actions[0];
    next_lexeme = action1->lexeme;

    if (next_lexeme->type == Lexeme::type_keyword &&
        next_lexeme->value == "ON") {
      // call 0x00CF   ; DSPFNK - (0xF3DE = CNSDFG: function keys presentation)
      addCall(0x00CF);
    } else if (next_lexeme->type == Lexeme::type_keyword &&
               next_lexeme->value == "OFF") {
      // call 0x00CC   ; ERAFNK
      addCall(0x00CC);
    } else {
      syntaxError("Invalid KEY statement");
    }

  } else if (t == 2) {
    action2 = current_action->actions[1];
    next_lexeme = action2->lexeme;

    // ld hl, key number
    action1 = current_action->actions[0];
    result_subtype = evalExpression(action1);
    addCast(result_subtype, Lexeme::subtype_numeric);

    // dec hl
    addDecHL();

    if ((next_lexeme->type == Lexeme::type_keyword &&
         next_lexeme->subtype == Lexeme::subtype_function) ||
        (next_lexeme->type == Lexeme::type_operator &&
         next_lexeme->value == "+") ||
        (next_lexeme->subtype == Lexeme::subtype_string)) {
      // add hl, hl
      addAddHLHL();
      // add hl, hl
      addAddHLHL();
      // add hl, hl
      addAddHLHL();
      // add hl, hl      ; key number * 16
      addAddHLHL();
      // ld de, 0xF87F   ; FNKSTR (10 x 16 bytes)
      addLdDE(0xf87f);
      // add hl, de
      addAddHLDE();

      // push hl
      addPushHL();
      //   ld a, 32        ; space
      addLdA(0x20);
      //   ld (hl), a
      addLdiHLA();
      //   ld e, l
      addLdEL();
      //   ld d, h
      addLdDH();
      //   inc de
      addIncDE();
      //   ld bc, 15
      addLdBC(0x000F);
      //   ldir
      addLDIR();

      //   ld hl, variable address
      result_subtype = evalExpression(action2);
      addCast(result_subtype, Lexeme::subtype_string);

      //   xor a
      addXorA();
      //   ld c, (hl)
      addLdCiHL();
      //   ld b, a
      addLdBA();
      //   inc hl
      addIncHL();

      // pop de
      addPopDE();

      // or c
      addOrC();
      // jr z, $+3
      addJrZ(0x02);
      //   ldir
      addLDIR();
      // ld (de), a
      addLdiDEA();
      // call 0x00C9    ; FNKSB
      addCall(0x00C9);

    } else {
      // ld e, l
      addLdEL();
      // ld d, h
      addLdDH();
      // add hl, de
      addAddHLDE();
      // add hl, de       ; key number * 3
      addAddHLDE();

      // ld de, 0xFC4C    ; KEY state position = key number * 3 + 0xFC4C
      addLdDE(0xfc4c);

      // add hl, de
      addAddHLDE();

      if (next_lexeme->type == Lexeme::type_keyword &&
          next_lexeme->value == "ON") {
        // call 0x6c89   ; xbasic turn on trap (hl=trap state address)
        addCall(def_XBASIC_TRAP_ON);
      } else if (next_lexeme->type == Lexeme::type_keyword &&
                 next_lexeme->value == "OFF") {
        // call 0x6c9c   ; xbasic turn off trap (hl=trap state address)
        addCall(def_XBASIC_TRAP_OFF);
      } else if (next_lexeme->type == Lexeme::type_keyword &&
                 next_lexeme->value == "STOP") {
        // call 0x6ca5   ; xbasic turn stop trap (hl=trap state address)
        addCall(def_XBASIC_TRAP_STOP);
      } else {
        syntaxError("Invalid KEY statement");
      }
    }

  } else {
    syntaxError("Empty KEY statement");
  }
}

void Compiler::cmd_strig() {
  ActionNode *action1, *action2;
  Lexeme* next_lexeme;
  unsigned int t = current_action->actions.size();
  int result_subtype;

  if (t == 2) {
    action2 = current_action->actions[1];
    next_lexeme = action2->lexeme;

    // ld hl, strig number
    action1 = current_action->actions[0];
    result_subtype = evalExpression(action1);
    addCast(result_subtype, Lexeme::subtype_numeric);

    // dec hl
    // addDecHL();

    // ld e, l
    addLdEL();
    // ld d, h
    addLdDH();
    // add hl, de
    addAddHLDE();
    // add hl, de       ; strig number * 3
    addAddHLDE();

    // ld de, 0xFC70    ; STRIG state position = key number * 3 + 0xFC70
    addLdDE(0xFC70);

    // add hl, de
    addAddHLDE();

    if (next_lexeme->type == Lexeme::type_keyword &&
        next_lexeme->value == "ON") {
      // call 0x6c89   ; xbasic turn on trap (hl=trap state address)
      addCall(def_XBASIC_TRAP_ON);
    } else if (next_lexeme->type == Lexeme::type_keyword &&
               next_lexeme->value == "OFF") {
      // call 0x6c9c   ; xbasic turn off trap (hl=trap state address)
      addCall(def_XBASIC_TRAP_OFF);
    } else if (next_lexeme->type == Lexeme::type_keyword &&
               next_lexeme->value == "STOP") {
      // call 0x6ca5   ; xbasic turn stop trap (hl=trap state address)
      addCall(def_XBASIC_TRAP_STOP);
    } else {
      syntaxError("Invalid STRIG statement");
    }

  } else {
    syntaxError("Wrong number of parameters in STRIG statement");
  }
}

bool Compiler::addCheckTraps() {
  if (parser->has_traps) {
    // call 0x6c25   ; xbasic check traps
    addCall(def_XBASIC_TRAP_CHECK);
  }

  return parser->has_traps;
}

void Compiler::cmd_on_key() {
  ActionNode *action, *sub_action;
  Lexeme *lexeme, *sub_lexeme;
  unsigned int i, t;

  action = current_action->actions[0];
  if (action->actions.size() != 1) {
    syntaxError("Wrong parameters in ON KEY");
    return;
  }

  action = action->actions[0];
  lexeme = action->lexeme;
  if (lexeme->value != "GOSUB") {
    syntaxError("GOSUB parameters is missing in ON KEY");
    return;
  }
  t = action->actions.size();

  if (t) {
    // GOSUB LIST

    // ld hl, 0xFC4D    ; KEY first GOSUB position = 0xFC4C+1
    addLdHL(0xfc4D);

    for (i = 0; i < t; i++) {
      sub_action = action->actions[i];
      sub_lexeme = sub_action->lexeme;

      if (sub_lexeme->type == Lexeme::type_literal &&
          sub_lexeme->subtype == Lexeme::subtype_numeric) {
        if (opts->megaROM) {
          // push hl
          addPushHL();
          //   ld hl, GOSUB ADDRESS
          addFix(sub_lexeme->value);
          addLdHLmegarom();
          //   ld (MR_TRAP_SEGMS), a       ; KEY segment
          addLdiiA(def_MR_TRAP_SEGMS + i);
          //   ex de, hl
          addExDEHL();
          // pop hl
          addPopHL();

        } else {
          // ld de, call address
          addFix(sub_lexeme->value);
          addLdDE(0x0000);
        }

      } else {
        // ld hl, 0x368D   ; dummy bios RET address
        addLdHL(def_XBASIC_DUMMY_RET);
      }

      // ld (hl), e
      addLdiHLE();
      // inc hl
      addIncHL();
      // ld (hl), d
      addLdiHLD();
      // inc hl
      addIncHL();
      // inc hl
      addIncHL();
    }

  } else {
    syntaxError("ON KEY with empty parameters");
  }
}

void Compiler::cmd_on_sprite() {
  ActionNode *action, *sub_action;
  Lexeme *lexeme, *sub_lexeme;
  unsigned int t, i = 0;

  action = current_action->actions[0];
  if (action->actions.size() != 1) {
    syntaxError("Wrong parameters in ON SPRITE");
    return;
  }

  action = action->actions[0];
  lexeme = action->lexeme;
  if (lexeme->value != "GOSUB") {
    syntaxError("GOSUB parameters is missing in ON SPRITE");
    return;
  }
  t = action->actions.size();

  if (t == 1) {
    // GOSUB address

    sub_action = action->actions[0];
    sub_lexeme = sub_action->lexeme;

    if (sub_lexeme->type == Lexeme::type_literal &&
        sub_lexeme->subtype == Lexeme::subtype_numeric) {
      if (opts->megaROM) {
        // push hl
        addPushHL();
        //   ld hl, GOSUB ADDRESS
        addFix(sub_lexeme->value);
        addLdHLmegarom();
        //   ld (MR_TRAP_SEGMS+11), a       ; SPRITE segment
        addLdiiA(def_MR_TRAP_SEGMS + 11 + i);
        //   ex de, hl
        addExDEHL();
        // pop hl
        addPopHL();

      } else {
        // ld hl, call address
        addFix(sub_lexeme->value);
        addLdHL(0x0000);
      }

    } else {
      // ld hl, 0x368D   ; dummy bios RET address
      addLdHL(def_XBASIC_DUMMY_RET);
    }

    // ld (0xFC6E), hl     ; STOP GOSUB position = 0xFC6D+1
    addLdiiHL(0xFC6E);

  } else {
    syntaxError("ON SPRITE with wrong count of parameters");
  }
}

void Compiler::cmd_on_stop() {
  ActionNode *action, *sub_action;
  Lexeme *lexeme, *sub_lexeme;
  unsigned int t, i = 0;

  action = current_action->actions[0];
  if (action->actions.size() != 1) {
    syntaxError("Wrong parameters in ON STOP");
    return;
  }

  action = action->actions[0];
  lexeme = action->lexeme;
  if (lexeme->value != "GOSUB") {
    syntaxError("GOSUB parameters is missing in ON STOP");
    return;
  }
  t = action->actions.size();

  if (t == 1) {
    // GOSUB address

    sub_action = action->actions[0];
    sub_lexeme = sub_action->lexeme;

    if (sub_lexeme->type == Lexeme::type_literal &&
        sub_lexeme->subtype == Lexeme::subtype_numeric) {
      if (opts->megaROM) {
        // push hl
        addPushHL();
        //   ld hl, GOSUB ADDRESS
        addFix(sub_lexeme->value);
        addLdHLmegarom();
        //   ld (MR_TRAP_SEGMS+10), a       ; STOP segment
        addLdiiA(def_MR_TRAP_SEGMS + 10 + i);
        //   ex de, hl
        addExDEHL();
        // pop hl
        addPopHL();

      } else {
        // ld hl, call address
        addFix(sub_lexeme->value);
        addLdHL(0x0000);
      }

    } else {
      // ld hl, 0x368D   ; dummy bios RET address
      addLdHL(def_XBASIC_DUMMY_RET);
    }

    // ld (0xFC6B), hl     ; STOP GOSUB position = 0xFC6A+1
    addLdiiHL(0xfc6B);

  } else {
    syntaxError("ON STOP with wrong count of parameters");
  }
}

void Compiler::cmd_on_strig() {
  ActionNode *action, *sub_action;
  Lexeme *lexeme, *sub_lexeme;
  unsigned int i, t;

  action = current_action->actions[0];
  if (action->actions.size() != 1) {
    syntaxError("Wrong parameters in ON STRIG");
    return;
  }

  action = action->actions[0];
  lexeme = action->lexeme;
  if (lexeme->value != "GOSUB") {
    syntaxError("GOSUB parameters is missing in ON STRIG");
    return;
  }
  t = action->actions.size();

  if (t) {
    // GOSUB LIST

    // ld hl, 0xFC71    ; STRIG first GOSUB position = 0xFC70+1
    addLdHL(0xFC71);

    for (i = 0; i < t; i++) {
      sub_action = action->actions[i];
      sub_lexeme = sub_action->lexeme;

      if (sub_lexeme->type == Lexeme::type_literal &&
          sub_lexeme->subtype == Lexeme::subtype_numeric) {
        if (opts->megaROM) {
          // push hl
          addPushHL();
          //   ld hl, GOSUB ADDRESS
          addFix(sub_lexeme->value);
          addLdHLmegarom();
          //   ld (MR_TRAP_SEGMS+12), a       ; STRIG segment
          addLdiiA(def_MR_TRAP_SEGMS + 12 + i);
          //   ex de, hl
          addExDEHL();
          // pop hl
          addPopHL();

        } else {
          // ld de, call address
          addFix(sub_lexeme->value);
          addLdDE(0x0000);
        }

      } else {
        // ld de, 0x368D   ; dummy bios RET address
        addLdDE(def_XBASIC_DUMMY_RET);
      }

      // ld (hl), e
      addLdiHLE();
      // inc hl
      addIncHL();
      // ld (hl), d
      addLdiHLD();
      // inc hl
      addIncHL();
      // inc hl
      addIncHL();
    }

  } else {
    syntaxError("ON STRIG with empty parameters");
  }
}

void Compiler::cmd_on_goto_gosub() {
  ActionNode *action, *sub_action;
  Lexeme *lexeme, *sub_lexeme;
  FixNode* mark;
  unsigned int i, t = current_action->actions.size();
  int result_subtype;

  if (t == 2) {
    // INDEX VARIABLE
    // ld hl, variable

    action = current_action->actions[0];
    if (action->actions.size() == 0) {
      syntaxError("Empty parameter in ON GOTO/GOSUB");
      return;
    }

    sub_action = action->actions[0];

    result_subtype = evalExpression(sub_action);

    addCast(result_subtype, Lexeme::subtype_numeric);

    // GOTO / GOSUB LIST

    action = current_action->actions[1];
    lexeme = action->lexeme;
    t = action->actions.size();

    // ld a, l
    addLdAL();

    if (opts->megaROM) {
      // ld (TEMP), a
      addLdiiA(def_TEMP);
    }

    // and a
    addAndA();
    // jp z, address
    mark = addMark();
    addJpZ(0x0000);

    for (i = 0; i < t; i++) {
      sub_action = action->actions[i];
      sub_lexeme = sub_action->lexeme;

      if (sub_lexeme->type == Lexeme::type_literal &&
          sub_lexeme->subtype == Lexeme::subtype_numeric) {
        // Trim leading zeros
        while (sub_lexeme->value.find("0") == 0 &&
               sub_lexeme->value.size() > 1) {
          sub_lexeme->value.erase(0, 1);
        }

        if (lexeme->value == "GOTO") {
          if (opts->megaROM) {
            // ld a, (TEMP)
            addLdAii(def_TEMP);
          }

          // dec a
          addDecA();

          if (opts->megaROM) {
            // ld (TEMP), a
            addLdiiA(def_TEMP);
          }

          // jp z, address
          addFix(sub_lexeme->value);
          addJpZ(0x0000);

        } else {
          if (opts->megaROM) {
            // ld a, (TEMP)
            addLdAii(def_TEMP);
          }

          // dec a
          addDecA();

          if (opts->megaROM) {
            // ld (TEMP), a
            addLdiiA(def_TEMP);
          }

          if (opts->megaROM) {
            // jr nz, $+25
            addJrNZ(24);
          } else {
            // jr nz, $+7
            addJrNZ(0x06);
          }
          //   call address
          addFix(sub_lexeme->value);
          addCall(0x0000);
          //   jp address
          addFix(mark->symbol);
          addJp(0x0000);
        }

      } else {
        if (opts->megaROM) {
          // ld a, (TEMP)
          addLdAii(def_TEMP);
        }

        // dec a
        addDecA();

        if (opts->megaROM) {
          // ld (TEMP), a
          addLdiiA(def_TEMP);
        }
      }
    }

    mark->symbol->address = code_pointer;

  } else {
    syntaxError("ON GOTO/GOSUB with empty parameters");
  }
}

void Compiler::cmd_swap() {
  Lexeme *lexeme1, *lexeme2;
  ActionNode *action1, *action2;
  unsigned int t = current_action->actions.size();

  if (t == 2) {
    action1 = current_action->actions[0];
    lexeme1 = action1->lexeme;
    if (lexeme1->type != Lexeme::type_identifier) {
      syntaxError("Invalid parameter type in SWAP (1st)");
      return;
    }

    action2 = current_action->actions[1];
    lexeme2 = action2->lexeme;
    if (lexeme2->type != Lexeme::type_identifier) {
      syntaxError("Invalid parameter type in SWAP (2nd)");
      return;
    }

    if (lexeme1->subtype == lexeme2->subtype) {
      // ld hl, variable
      addVarAddress(action1);
      // push hl
      addPushHL();
      // ld hl, variable
      addVarAddress(action2);
      // pop de
      addByteOptimized(0xD1);

      if (lexeme1->subtype == Lexeme::subtype_numeric) {
        // call 0x6bf5    ; xbasic SWAP integers (in: hl=var1, de=var2)
        addCall(def_XBASIC_SWAP_INTEGER);
      } else if (lexeme1->subtype == Lexeme::subtype_string) {
        // call 0x6bf9    ; xbasic SWAP strings (in: hl=var1, de=var2)
        addCall(def_XBASIC_SWAP_STRING);
      } else {
        // call 0x6bfd    ; xbasic SWAP floats (in: hl=var1, de=var2)
        addCall(def_XBASIC_SWAP_FLOAT);
      }

    } else {
      syntaxError("Parameters type mismatch in SWAP");
    }

  } else {
    syntaxError("Invalid SWAP parameters");
  }
}

void Compiler::cmd_wait() {
  Lexeme* lexeme;
  ActionNode* action;
  unsigned int i, t = current_action->actions.size();
  int result_subtype;

  if (t == 2 || t == 3) {
    for (i = 0; i < t; i++) {
      action = current_action->actions[i];
      lexeme = action->lexeme;

      if (lexeme) {
        result_subtype = evalExpression(action);

        // cast
        addCast(result_subtype, Lexeme::subtype_numeric);

        switch (i) {
          case 0: {
            // push hl
            addPushHL();
          } break;

          case 1: {
            if (t == 3) {
              // ld a, l
              addLdAL();
              // push af
              addPushAF();
            } else {
              // ld d, l
              addLdDL();
              // pop bc
              addPopBC();
              //   in a,(c)
              addWord(0xED, 0x78);
              //   and d
              addAndD();
              // jr z, $-4
              addJrZ(0xFB);
            }
          } break;

          case 2: {
            // ld h, l
            addLdHL();
            // pop de
            addPopDE();
            // pop bc
            addPopBC();
            //   in a,(c)
            addWord(0xED, 0x78);
            //   xor h
            addXorH();
            //   and d
            addAndD();
            // jr z, $-5
            addJrZ(0xFA);

          } break;
        }
      }
    }

  } else {
    syntaxError("Invalid WAIT parameters");
  }
}

void Compiler::cmd_data() {
  if (!current_action->actions.size()) {
    syntaxError("DATA with empty parameters");
  }
}

void Compiler::cmd_idata() {
  if (!current_action->actions.size()) {
    syntaxError("IDATA with empty parameters");
  }
}

void Compiler::cmd_read() {
  ActionNode* action;
  Lexeme* lexeme;
  unsigned int i, t = current_action->actions.size();

  if (t) {
    for (i = 0; i < t; i++) {
      action = current_action->actions[i];
      lexeme = action->lexeme;

      if (lexeme->type != Lexeme::type_identifier) {
        syntaxError("Invalid READ parameter type");
        return;
      }

      // call read
      addCall(def_XBASIC_READ);

      addCast(Lexeme::subtype_string, lexeme->subtype);

      // do assignment

      addAssignment(action);
    }

  } else {
    syntaxError("READ with empty parameters");
  }
}

void Compiler::cmd_iread() {
  ActionNode* action;
  Lexeme* lexeme;
  unsigned int i, t = current_action->actions.size();

  if (t) {
    for (i = 0; i < t; i++) {
      action = current_action->actions[i];
      lexeme = action->lexeme;

      if (lexeme->type != Lexeme::type_identifier) {
        syntaxError("Invalid IREAD parameter type");
        return;
      }

      // call XBASIC_IREAD
      addCall(def_XBASIC_IREAD);

      addCast(Lexeme::subtype_numeric, lexeme->subtype);

      // do assignment

      addAssignment(action);
    }

  } else {
    syntaxError("IREAD with empty parameters");
  }
}

void Compiler::cmd_resume() {
  syntaxError("RESUME statement not supported in compiled mode");
}

void Compiler::cmd_restore() {
  ActionNode* action;
  Lexeme* lexeme;
  int t = current_action->actions.size();
  int result_subtype;

  if (t == 0) {
    // ld hl, 0
    addLdHL(0x0000);
    // call xbasic_restore
    addCall(def_XBASIC_RESTORE);  // standard BASIC RESTORE statement

  } else if (t == 1) {
    action = current_action->actions[0];

    lexeme = action->lexeme;
    if (lexeme) {
      result_subtype = evalExpression(action);

      // cast
      addCast(result_subtype, Lexeme::subtype_numeric);

      // call restore
      addCall(def_XBASIC_RESTORE);  // standard BASIC RESTORE statement
    }

  } else {
    syntaxError("RESTORE with wrong number of parameters");
  }
}

void Compiler::cmd_irestore() {
  ActionNode* action;
  Lexeme* lexeme;
  int t = current_action->actions.size();
  int result_subtype;

  if (t == 0) {
    // ld hl, (DATLIN)
    addLdHLii(def_DATLIN);
    // ld (DATPTR), hl
    addLdiiHL(def_DATPTR);

  } else if (t == 1) {
    action = current_action->actions[0];

    lexeme = action->lexeme;
    if (lexeme) {
      result_subtype = evalExpression(action);

      // cast
      addCast(result_subtype, Lexeme::subtype_numeric);

      // ex de, hl
      addExDEHL();
      // ld hl, (DATLIN)
      addLdHLii(def_DATLIN);
      // add hl, de
      addAddHLDE();
      // ld (DATPTR), hl
      addLdiiHL(def_DATPTR);
    }

  } else {
    syntaxError("IRESTORE with wrong number of parameters");
  }
}

void Compiler::cmd_out() {
  Lexeme* lexeme;
  ActionNode* action;
  unsigned int i, t = current_action->actions.size();
  int result_subtype;

  if (t == 2) {
    for (i = 0; i < t; i++) {
      action = current_action->actions[i];
      lexeme = action->lexeme;
      if (lexeme) {
        result_subtype = evalExpression(action);

        // cast
        addCast(result_subtype, Lexeme::subtype_numeric);

        if (i == 0) {
          // push hl
          addPushHL();
        } else {
          // pop bc
          addPopBC();
          // out (c), l
          addWord(0xED, 0x69);
        }
      }
    }

  } else {
    syntaxError("Invalid OUT parameters");
  }
}

void Compiler::cmd_poke() {
  Lexeme* lexeme;
  ActionNode* action;
  unsigned int i, t = current_action->actions.size();
  int result_subtype;

  if (t == 2) {
    for (i = 0; i < t; i++) {
      action = current_action->actions[i];
      lexeme = action->lexeme;
      if (lexeme) {
        result_subtype = evalExpression(action);

        // cast
        addCast(result_subtype, Lexeme::subtype_numeric);

        if (i == 0) {
          // push hl
          addPushHL();
        } else {
          // ex de, hl
          addExDEHL();
          // pop hl
          addPopHL();
          // ld (hl), e
          addLdiHLE();
        }
      }
    }

  } else {
    syntaxError("Invalid POKE parameters");
  }
}

void Compiler::cmd_ipoke() {
  Lexeme* lexeme;
  ActionNode* action;
  unsigned int i, t = current_action->actions.size();
  int result_subtype;

  if (t == 2) {
    for (i = 0; i < t; i++) {
      action = current_action->actions[i];
      lexeme = action->lexeme;
      if (lexeme) {
        result_subtype = evalExpression(action);

        // cast
        addCast(result_subtype, Lexeme::subtype_numeric);

        if (i == 0) {
          // push hl
          addPushHL();
        } else {
          // ex de, hl
          addExDEHL();
          // pop hl
          addPopHL();
          // ld (hl), e
          addLdiHLE();
          // inc hl
          addIncHL();
          // ld (hl), d
          addLdiHLD();
        }
      }
    }

  } else {
    syntaxError("Invalid IPOKE parameters");
  }
}

void Compiler::cmd_vpoke() {
  Lexeme* lexeme;
  ActionNode* action;
  unsigned int i, t = current_action->actions.size();
  int result_subtype;

  if (t == 2) {
    for (i = 0; i < t; i++) {
      action = current_action->actions[i];
      lexeme = action->lexeme;
      if (lexeme) {
        result_subtype = evalExpression(action);

        // cast
        addCast(result_subtype, Lexeme::subtype_numeric);

        if (i == 0) {
          // push hl
          addPushHL();
        } else {
          // ld a, l
          addLdAL();
          // pop hl
          addPopHL();
          // call 0x70b5    ; xbasic VPOKE (in: hl=address, a=byte)
          addCall(def_XBASIC_VPOKE);
        }
      }
    }

  } else {
    syntaxError("Invalid VPOKE parameters");
  }
}

void Compiler::cmd_file() {
  Lexeme* lexeme;
  ActionNode* action;
  unsigned int t = current_action->actions.size();
  string filename;

  if (t == 1) {
    action = current_action->actions[0];
    lexeme = action->lexeme;

    if (lexeme->type == Lexeme::type_literal &&
        lexeme->subtype == Lexeme::subtype_string) {
      lexeme->name = "FILE";
      filename = removeQuotes(lexeme->value);
      resourceManager.addFile(filename, opts->inputPath);
    } else {
      error_message = "Invalid parameter in FILE keyword";
    }

  } else {
    syntaxError("Wrong FILE parameters count");
  }
}

void Compiler::cmd_text() {
  Lexeme* lexeme;
  ActionNode* action;
  unsigned int t = current_action->actions.size();

  if (t == 1) {
    action = current_action->actions[0];
    lexeme = action->lexeme;

    if (lexeme->type == Lexeme::type_literal &&
        lexeme->subtype == Lexeme::subtype_string) {
      lexeme->name = "TEXT";
      resourceManager.addText(lexeme->value);
    } else {
      error_message = "Invalid parameter in TEXT keyword";
    }

  } else {
    syntaxError("Wrong TEXT parameters count");
  }
}

void Compiler::cmd_call() {
  ActionNode* action;
  Lexeme* lexeme;
  unsigned int i, t = current_action->actions.size();

  if (t) {
    for (i = 0; i < t; i++) {
      action = current_action->actions[i];
      lexeme = action->lexeme;

      if (lexeme->value == "TURBO") {
        // not need anymore... he he he...

      } else if (lexeme->value == "SYSTEM") {
        // ld hl, data address
        addVarAddress(action);
        // ld a, (hl)      ; first character
        addLdAiHL();
        // ld ix, (CALL)    ; CALL
        addLdIXii(def_CALL_STMT);
        // call CALBAS
        addCall(def_CALBAS);
        // ei
        addEI();

      } else {
        // todo: concatenate CALL statement  into a literal string
        //       and run it with BASIC like above
        syntaxError("CALL statement invalid");
        return;
      }
    }

  } else {
    syntaxError("CALL with empty parameters");
  }
}

void Compiler::cmd_maxfiles() {
  ActionNode* action;
  unsigned int t = current_action->actions.size();
  int result_subtype;
  FixNode* mark;

  if (t) {
    // xor a
    // addXorA();
    // ld (NLONLY), a
    // addLdiiA(def_NLONLY);
    // ld (DSKDIS), a
    // addLdiiA(0xFD99);

    action = current_action->actions[0];
    result_subtype = evalExpression(action);
    addCast(result_subtype, Lexeme::subtype_numeric);

    // ld a, (MAXFIL)
    addLdAii(def_MAXFIL);
    // cp l
    addCpL();
    // jp z, $                ; skip if equal
    mark = addMark();
    addJpZ(0x0000);

    // ld a, l
    addLdAL();

    // ld (MAXFIL), a
    addLdiiA(def_MAXFIL);

    // push af
    addPushAF();

    addEnableBasicSlot();

    // ld hl, fake empty line
    addLdHL(def_ENDPRG);

    // call CLOSE_ALL
    addCall(def_CLOSE_ALL);

    // pop af
    addPopAF();

    // call MAXFILES
    addCall(def_MAXFILES);

    // call 0x7304			; end printeroutput (basic interpreter
    // function)
    addCall(0x7304);
    // call 0x4AFF			; return interpreter output to screen
    // (basic interpreter function)
    addCall(0x4AFF);

    addDisableBasicSlot();

    // ; restore stack state
    // ld bc, 16
    addLdBC(0x0010);
    // ld (TEMP), sp
    addLdiiSP(def_TEMP);
    // ld hl, (TEMP)
    addLdHLii(def_TEMP);
    // xor a
    addXorA();
    // sbc hl, bc
    addSbcHLBC();
    // ld sp,hl
    addLdSPHL();
    // ex de,hl
    addExDEHL();
    // ld hl, (SAVSTK)
    addLdHLii(def_SAVSTK);
    // ldir
    addLDIR();
    // ld (SAVSTK), sp
    addLdiiSP(def_SAVSTK);

    mark->symbol->address = code_pointer;

  } else {
    syntaxError("Empty MAXFILES assignment");
  }
}

void Compiler::cmd_open() {
  ActionNode* action;
  Lexeme* lexeme;
  unsigned int i, t = current_action->actions.size();
  int result_subtype, state = 0;
  FixNode* mark;
  bool has[4];

  for (i = 0; i < 4; i++) has[i] = false;

  file_support = true;

  if (t) {
    for (i = 0; i < t; i++) {
      action = current_action->actions[i];
      lexeme = action->lexeme;

      switch (state) {
        // file name
        case 0: {
          has[state] = true;
          state = 1;
          result_subtype = evalExpression(action);
          addCast(result_subtype, Lexeme::subtype_string);

          // push hl
          addPushHL();
        } break;

        // FOR
        case 1: {
          has[state] = true;
          state = 2;
          if (lexeme->value == "OUT") {
            // ld a, 2     ; output mode
            addLdA(0x02);
          } else if (lexeme->value == "APP") {
            // ld a, 8     ; append mode
            addLdA(0x08);
          } else if (lexeme->value == "INPUT") {
            // ld a, 1     ; input mode
            addLdA(0x01);
          } else {
            // ld a, 4     ; random mode
            addLdA(0x04);
            i--;
          }
          // push af
          addPushAF();
        } break;

        // AS
        case 2: {
          has[state] = true;
          state = 3;
          result_subtype = evalExpression(action);
          addCast(result_subtype, Lexeme::subtype_numeric);
          // push hl
          addPushHL();
        } break;

        // LEN
        case 3: {
          has[state] = true;
          state = 4;

          result_subtype = evalExpression(action);
          addCast(result_subtype, Lexeme::subtype_numeric);
        } break;
      }
    }

    addEnableBasicSlot();

    // LEN
    if (!has[3]) {
      // ld hl, 256  ; default record size
      addLdHL(0x0100);
    }
    // ld (RECSIZ), hl
    addLdiiHL(def_RECSIZ);

    // AS
    if (has[2]) {
      // pop hl
      addPopHL();
      // ld a, l
      addLdAL();
    } else {
      // xor a
      addXorA();
    }
    // ld (TEMP), a
    addLdiiA(def_TEMP);

    // FOR
    if (has[1]) {
      // pop af
      addPopAF();
    } else {
      // xor a
      addXorA();
    }
    // ld (TEMP+1), a           ; file mode
    addLdiiA(def_TEMP + 1);

    if (opts->megaROM) {
      // ld hl, 0x0000             ; get return point address
      mark = addMark();
      addLdHLmegarom();
      // ld c, l
      addLdCL();
      // ld b, h
      addLdBH();
    } else {
      // ld bc, 0x0000             ; get return point address
      mark = addMark();
      addLdBC(0x0000);
    }

    // FILE
    if (has[0]) {
      // pop hl
      addPopHL();
    } else {
      // ld hl, NULL STR
      addLdHL(0x8000);
    }

    // push bc            ; save return point to run after FILESPEC
    addPushBC();

    // ld e, (hl)
    addLdEiHL();

    // inc hl
    addIncHL();

    // push hl
    addPushHL();

    // jp FILESPEC        ; in e=string size, hl=string start; out d = device
    // code, FILNAM
    addJp(def_FILESPEC);

    mark->symbol->address = code_pointer;  // return point after FILESPEC

    // ld a, (TEMP+1)
    addLdAii(def_TEMP + 1);
    // ld e, a                ; file mode
    addLdEA();

    // ld a, (TEMP)           ; io number
    addLdAii(def_TEMP);

    // ld hl, fake empty line
    addLdHL(def_ENDPRG);

    // call OPEN     ; in: a = i/o number, e = filemode, d = devicecode
    addCall(def_OPEN);

    addDisableBasicSlot();

  } else {
    syntaxError("Empty OPEN statement");
  }
}

void Compiler::cmd_close() {
  ActionNode* action;
  unsigned int i, t = current_action->actions.size();
  int result_subtype;

  if (t) {
    for (i = 0; i < t; i++) {
      action = current_action->actions[i];
      result_subtype = evalExpression(action);
      addCast(result_subtype, Lexeme::subtype_numeric);

      // ld a, l
      addLdAL();
      // push af
      addPushAF();

      addEnableBasicSlot();

      // pop af
      addPopAF();
      // ld hl, fake empty line
      addLdHL(def_ENDPRG);

      // call CLOSE    ; in: a = i/o number
      addCall(def_CLOSE);

      addDisableBasicSlot();
    }

  } else {
    addEnableBasicSlot();

    // ld hl, fake empty line
    addLdHL(def_ENDPRG);

    // call ClOSE ALL
    addCall(def_CLOSE_ALL);

    addDisableBasicSlot();
  }
}

void Compiler::cmd_def() {
  ActionNode *action, *subaction;
  Lexeme* lexeme;
  unsigned int t = current_action->actions.size();
  int result_subtype;

  if (t) {
    action = current_action->actions[0];
    lexeme = action->lexeme;

    if (lexeme->value == "USR") {
      t = action->actions.size();

      if (t == 2) {
        has_defusr = true;

        subaction = action->actions[0];
        result_subtype = evalExpression(subaction);
        addCast(result_subtype, Lexeme::subtype_numeric);

        // push hl
        addPushHL();

        subaction = action->actions[1];
        result_subtype = evalExpression(subaction);
        addCast(result_subtype, Lexeme::subtype_numeric);

        // ex de,hl
        addExDEHL();

        // pop hl
        addPopHL();
        // ld bc, USRTAB
        addLdBC(def_USRTAB);
        // add hl,hl
        addAddHLHL();
        // add hl,bc
        addAddHLBC();
        // ld (hl), e
        addLdiHLE();
        // inc hl
        addIncHL();
        // ld (hl), d
        addLdiHLD();

      } else {
        syntaxError("Wrong DEF USR parameters count");
      }
    }
  }
}

void Compiler::cmd_cmd() {
  ActionNode *action, *sub_action1, *sub_action2, *sub_action3;
  Lexeme* lexeme;
  unsigned int i, t = current_action->actions.size();
  int result_subtype;

  if (t) {
    for (i = 0; i < t; i++) {
      action = current_action->actions[i];
      lexeme = action->lexeme;

      if (lexeme->value == "RUNASM") {
        if (action->actions.size() == 1) {
          sub_action1 = action->actions[0];
          result_subtype = evalExpression(sub_action1);
          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (DAC), hl
          addLdiiHL(def_DAC);
          // call cmd_runasm
          addCall(def_cmd_runasm);

        } else {
          syntaxError("CMD RUNASM syntax error");
        }

      } else if (lexeme->value == "RUNBAS") {
        syntaxError("CMD RUNBAS not implemented yet");

      } else if (lexeme->value == "WRTVRAM") {
        if (action->actions.size() == 2) {
          sub_action1 = action->actions[0];
          result_subtype = evalExpression(sub_action1);
          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (DAC), hl
          addLdiiHL(def_DAC);

          sub_action2 = action->actions[1];
          result_subtype = evalExpression(sub_action2);
          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (ARG), hl
          addLdiiHL(def_ARG);

          // call cmd_runasm
          addCall(def_cmd_wrtvram);

        } else {
          syntaxError("CMD WRTVRAM syntax error");
        }

      } else if (lexeme->value == "WRTFNT") {
        if (action->actions.size() == 1) {
          sub_action1 = action->actions[0];
          result_subtype = evalExpression(sub_action1);
          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (DAC), hl
          addLdiiHL(def_DAC);
          // call cmd_wrtfnt
          addCall(def_cmd_wrtfnt);

        } else {
          syntaxError("CMD WRTFNT syntax error");
        }

      } else if (lexeme->value == "WRTCHR") {
        if (action->actions.size() == 1) {
          sub_action1 = action->actions[0];
          result_subtype = evalExpression(sub_action1);
          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (DAC), hl
          addLdiiHL(def_DAC);
          // call cmd_wrtchr
          addCall(def_cmd_wrtchr);

        } else {
          syntaxError("CMD WRTCHR syntax error");
        }

      } else if (lexeme->value == "WRTCLR") {
        if (action->actions.size() == 1) {
          sub_action1 = action->actions[0];
          result_subtype = evalExpression(sub_action1);
          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (DAC), hl
          addLdiiHL(def_DAC);
          // call cmd_wrtclr
          addCall(def_cmd_wrtclr);

        } else {
          syntaxError("CMD WRTCLR syntax error");
        }

      } else if (lexeme->value == "WRTSCR") {
        if (action->actions.size() == 1) {
          sub_action1 = action->actions[0];
          result_subtype = evalExpression(sub_action1);
          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (DAC), hl
          addLdiiHL(def_DAC);
          // call cmd_wrtscr
          addCall(def_cmd_wrtscr);

        } else {
          syntaxError("CMD WRTSCR syntax error");
        }

      } else if (lexeme->value == "WRTSPR") {
        if (action->actions.size() == 1) {
          sub_action1 = action->actions[0];
          result_subtype = evalExpression(sub_action1);
          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (DAC), hl
          addLdiiHL(def_DAC);
          // call cmd_wrtspr                    ; tiny sprite loader
          addCall(def_cmd_wrtspr);

        } else {
          syntaxError("CMD WRTSPR syntax error");
        }

      } else if (lexeme->value == "WRTSPRPAT") {
        if (action->actions.size() == 1) {
          sub_action1 = action->actions[0];
          result_subtype = evalExpression(sub_action1);
          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (DAC), hl
          addLdiiHL(def_DAC);
          // call cmd_wrtsprpat
          addCall(def_cmd_wrtsprpat);

        } else {
          syntaxError("CMD WRTSPRPAT syntax error");
        }

      } else if (lexeme->value == "WRTSPRCLR") {
        if (action->actions.size() == 1) {
          sub_action1 = action->actions[0];
          result_subtype = evalExpression(sub_action1);
          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (DAC), hl
          addLdiiHL(def_DAC);
          // call cmd_wrtsprclr
          addCall(def_cmd_wrtsprclr);

        } else {
          syntaxError("CMD WRTSPRCLR syntax error");
        }

      } else if (lexeme->value == "WRTSPRATR") {
        if (action->actions.size() == 1) {
          sub_action1 = action->actions[0];
          result_subtype = evalExpression(sub_action1);
          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (DAC), hl
          addLdiiHL(def_DAC);
          // call cmd_wrtspratr
          addCall(def_cmd_wrtspratr);

        } else {
          syntaxError("CMD WRTSPRATR syntax error");
        }

      } else if (lexeme->value == "RAMTOVRAM") {
        if (action->actions.size() == 3) {
          sub_action1 = action->actions[0];
          result_subtype = evalExpression(sub_action1);
          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (DAC), hl
          addLdiiHL(def_DAC);

          sub_action2 = action->actions[1];
          result_subtype = evalExpression(sub_action2);
          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (ARG), hl
          addLdiiHL(def_ARG);

          sub_action3 = action->actions[2];
          result_subtype = evalExpression(sub_action3);
          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (ARG2), hl
          addLdiiHL(def_ARG2);

          // call cmd_ramtovram
          addCall(def_cmd_ramtovram);

        } else {
          syntaxError("CMD RAMTOVRAM syntax error");
        }

      } else if (lexeme->value == "VRAMTORAM") {
        if (action->actions.size() == 3) {
          sub_action1 = action->actions[0];
          result_subtype = evalExpression(sub_action1);
          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (DAC), hl
          addLdiiHL(def_DAC);

          sub_action2 = action->actions[1];
          result_subtype = evalExpression(sub_action2);
          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (ARG), hl
          addLdiiHL(def_ARG);

          sub_action3 = action->actions[2];
          result_subtype = evalExpression(sub_action3);
          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (ARG2), hl
          addLdiiHL(def_ARG2);

          // call cmd_vramtoram
          addCall(def_cmd_vramtoram);

        } else {
          syntaxError("CMD VRAMTORAM syntax error");
        }

      } else if (lexeme->value == "DISSCR") {
        // call cmd_disscr
        addCall(def_cmd_disscr);

      } else if (lexeme->value == "ENASCR") {
        // call cmd_enascr
        addCall(def_cmd_enascr);

      } else if (lexeme->value == "KEYCLKOFF") {
        // call cmd_keyclkoff
        addCall(def_cmd_keyclkoff);

      } else if (lexeme->value == "MUTE") {
        // call cmd_mute
        addCall(def_cmd_mute);

      } else if (lexeme->value == "PLAY") {
        if (action->actions.size() == 1) {
          sub_action1 = action->actions[0];
          result_subtype = evalExpression(sub_action1);
          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (DAC), hl
          addLdiiHL(def_DAC);

          // xor a
          addXorA();
          // ld (ARG), a
          addLdiiA(def_ARG);

          // call cmd_play
          addCall(def_cmd_play);

        } else if (action->actions.size() == 2) {
          sub_action1 = action->actions[0];
          result_subtype = evalExpression(sub_action1);
          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (DAC), hl
          addLdiiHL(def_DAC);

          sub_action2 = action->actions[1];
          result_subtype = evalExpression(sub_action2);
          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (ARG), hl
          addLdiiHL(def_ARG);

          // call cmd_play
          addCall(def_cmd_play);

        } else {
          syntaxError("CMD PLAY syntax error");
        }

      } else if (lexeme->value == "DRAW") {
        if (action->actions.size() == 1) {
          sub_action1 = action->actions[0];
          result_subtype = evalExpression(sub_action1);
          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (DAC), hl
          addLdiiHL(def_DAC);
          // call cmd_draw
          addCall(def_cmd_draw);

        } else {
          syntaxError("CMD DRAW syntax error");
        }

      } else if (lexeme->value == "PT3LOAD") {
        pt3 = true;
        syntaxError("Arkos Tracker not permitted with PT3 player");

      } else if (lexeme->value == "PT3PLAY") {
        pt3 = true;
        syntaxError("Arkos Tracker not permitted with PT3 player");

      } else if (lexeme->value == "PT3MUTE") {
        pt3 = true;
        syntaxError("Arkos Tracker not permitted with PT3 player");

      } else if (lexeme->value == "PT3LOOP") {
        pt3 = true;
        syntaxError("Arkos Tracker not permitted with PT3 player");

      } else if (lexeme->value == "PT3REPLAY") {
        pt3 = true;
        syntaxError("Arkos Tracker not permitted with PT3 player");

      } else if (lexeme->value == "PLYLOAD") {
        akm = true;

        if (action->actions.size()) {
          sub_action1 = action->actions[0];
          result_subtype = evalExpression(sub_action1);
          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (DAC), hl
          addLdiiHL(def_DAC);

          if (action->actions.size() > 1) {
            sub_action2 = action->actions[1];
            result_subtype = evalExpression(sub_action2);
            addCast(result_subtype, Lexeme::subtype_numeric);
          }

          // ld (ARG), hl
          addLdiiHL(def_ARG);

        } else {
          syntaxError("CMD PLYLOAD syntax error");
        }

        // call cmd_plyload
        addCall(def_cmd_plyload);

      } else if (lexeme->value == "PLYSONG") {
        akm = true;

        if (action->actions.size() == 1) {
          sub_action1 = action->actions[0];
          result_subtype = evalExpression(sub_action1);
          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (DAC), hl
          addLdiiHL(def_DAC);

        } else {
          syntaxError("CMD PLYSONG syntax error");
        }

        // call cmd_plysong
        addCall(def_cmd_plysong);

      } else if (lexeme->value == "PLYPLAY") {
        akm = true;

        // call cmd_plyplay
        addCall(def_cmd_plyplay);

      } else if (lexeme->value == "PLYMUTE") {
        akm = true;

        // call cmd_akmmute
        addCall(def_cmd_plymute);

      } else if (lexeme->value == "PLYLOOP") {
        akm = true;

        if (action->actions.size() == 1) {
          sub_action1 = action->actions[0];
          result_subtype = evalExpression(sub_action1);
          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (DAC), hl
          addLdiiHL(def_DAC);
          // call cmd_plyloop
          addCall(def_cmd_plyloop);

        } else {
          syntaxError("CMD PLYLOOP syntax error");
        }

      } else if (lexeme->value == "PLYREPLAY") {
        akm = true;

        // call cmd_plyreplay
        addCall(def_cmd_plyreplay);

      } else if (lexeme->value == "PLYSOUND") {
        if (action->actions.size()) {
          sub_action1 = action->actions[0];
          result_subtype = evalExpression(sub_action1);
          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (DAC), hl
          addLdiiHL(def_DAC);

          if (action->actions.size() > 1) {
            sub_action2 = action->actions[1];
            result_subtype = evalExpression(sub_action2);
            addCast(result_subtype, Lexeme::subtype_numeric);
          } else {
            // ld hl, 0
            addLdHL(0x0000);
          }

          // ld (ARG), hl
          addLdiiHL(def_ARG);

          if (action->actions.size() > 2) {
            sub_action2 = action->actions[1];
            result_subtype = evalExpression(sub_action2);
            addCast(result_subtype, Lexeme::subtype_numeric);
          } else if (action->actions.size() > 1) {
            // ld hl, 0
            addLdHL(0x0000);
          }

          // ld (ARG+2), hl
          addLdiiHL(def_ARG + 2);

        } else {
          syntaxError("CMD PLYSOUND syntax error");
        }

        // call cmd_plysound
        addCall(def_cmd_plysound);

      } else if (lexeme->value == "SETFNT") {
        font = true;

        if (action->actions.size() == 1) {
          sub_action1 = action->actions[0];
          result_subtype = evalExpression(sub_action1);
          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (DAC), hl
          addLdiiHL(def_DAC);

          // ld a, 0xff                ; it means all screen banks
          addLdA(0xFF);
          // ld (ARG), a
          addLdiiA(def_ARG);

          // call cmd_setfnt
          addCall(def_cmd_setfnt);

        } else if (action->actions.size() == 2) {
          sub_action1 = action->actions[0];
          result_subtype = evalExpression(sub_action1);
          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (DAC), hl
          addLdiiHL(def_DAC);

          sub_action2 = action->actions[1];
          result_subtype = evalExpression(sub_action2);
          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (ARG), hl             ; screen font bank number
          addLdiiHL(def_ARG);

          // call cmd_setfnt
          addCall(def_cmd_setfnt);

        } else {
          syntaxError("CMD SETFNT syntax error");
        }

      } else if (lexeme->value == "UPDFNTCLR") {
        // call cmd_disscr
        addCall(def_cmd_updfntclr);

      } else if (lexeme->value == "CLRSCR") {
        // call cmd_clrscr
        addCall(def_cmd_clrscr);

      } else if (lexeme->value == "RAMTORAM") {
        if (action->actions.size() == 3) {
          sub_action1 = action->actions[0];
          result_subtype = evalExpression(sub_action1);
          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (DAC), hl
          addLdiiHL(def_DAC);

          sub_action2 = action->actions[1];
          result_subtype = evalExpression(sub_action2);
          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (ARG), hl
          addLdiiHL(def_ARG);

          sub_action3 = action->actions[2];
          result_subtype = evalExpression(sub_action3);
          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (ARG2), hl
          addLdiiHL(def_ARG2);

          // call cmd_ramtoram
          addCall(def_cmd_ramtoram);

        } else {
          syntaxError("CMD RAMTORAM syntax error");
        }

      } else if (lexeme->value == "RSCTORAM") {
        if (action->actions.size() == 2 || action->actions.size() == 3) {
          sub_action1 = action->actions[0];  // resource number
          result_subtype = evalExpression(sub_action1);
          addCast(result_subtype, Lexeme::subtype_numeric);

          // push hl
          addPushHL();

          sub_action2 = action->actions[1];  // ram dest address
          result_subtype = evalExpression(sub_action2);
          addCast(result_subtype, Lexeme::subtype_numeric);

          if (action->actions.size() == 2) {
            // ex de, hl
            addExDEHL();
            // xor a   ; no pletter
            addXorA();
          } else {
            // push hl
            addPushHL();

            sub_action3 = action->actions[2];  // pletter? 0=no, 1=yes
            result_subtype = evalExpression(sub_action3);
            addCast(result_subtype, Lexeme::subtype_numeric);

            // ld a, l
            addLdAL();

            // pop de
            addPopDE();
          }

          // pop hl
          addPopHL();

          // call cmd_rsctoram
          addCall(def_cmd_rsctoram);

        } else {
          syntaxError("CMD RSCTORAM syntax error");
        }

      } else if (lexeme->value == "CLRKEY") {
        // call cmd_clrkey
        addCall(def_cmd_clrkey);

      } else if (lexeme->value == "CLIP") {
        if (action->actions.size() == 1) {
          sub_action1 = action->actions[0];
          result_subtype = evalExpression(sub_action1);
          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld a, l
          addLdAL();
          // ld hl, 0xfafc
          addLdHL(0xfafc);
          // res 4, (hl)
          addWord(0xCB, 0xA6);
          // bit 0, a
          addWord(0xCB, 0x47);
          // jr z, +3
          addJrZ(0x02);
          //   set 4, (hl)
          addWord(0xCB, 0xE6);

        } else {
          syntaxError("CMD CLIP syntax error");
        }

      } else if (lexeme->value == "TURBO") {
        if (action->actions.size() == 1) {
          sub_action1 = action->actions[0];
          result_subtype = evalExpression(sub_action1);
          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (DAC), hl
          addLdiiHL(def_DAC);
          // call cmd_turbo
          addCall(def_cmd_turbo);

        } else {
          syntaxError("CMD TURBO syntax error");
        }

      } else if (lexeme->value == "RESTORE") {
        if (action->actions.size() == 1) {
          sub_action1 = action->actions[0];
          result_subtype = evalExpression(sub_action1);
          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (DAC), hl
          addLdiiHL(def_DAC);

          // call cmd_restore
          addCall(def_cmd_restore);  // MSXBAS2ROM resource RESTORE statement

        } else {
          syntaxError("CMD RESTORE syntax error");
        }

      } else if (lexeme->value == "PAGE") {
        if (action->actions.size() == 1) {
          sub_action1 = action->actions[0];
          result_subtype = evalExpression(sub_action1);
          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld de, 0                 ; delay = stop
          addLdDE(0);

          // push de
          addPushDE();

          // pop bc
          addPopBC();

        } else if (action->actions.size() == 2) {
          sub_action1 = action->actions[1];
          result_subtype = evalExpression(sub_action1);
          addCast(result_subtype, Lexeme::subtype_numeric);

          // push hl
          addPushHL();
          // push hl
          addPushHL();

          sub_action2 = action->actions[0];
          result_subtype = evalExpression(sub_action2);
          addCast(result_subtype, Lexeme::subtype_numeric);

          // pop de
          addPopDE();

          // pop bc
          addPopBC();

        } else if (action->actions.size() == 3) {
          sub_action1 = action->actions[2];
          result_subtype = evalExpression(sub_action1);
          addCast(result_subtype, Lexeme::subtype_numeric);

          // push hl
          addPushHL();

          sub_action1 = action->actions[1];
          result_subtype = evalExpression(sub_action1);
          addCast(result_subtype, Lexeme::subtype_numeric);

          // push hl
          addPushHL();

          sub_action2 = action->actions[0];
          result_subtype = evalExpression(sub_action2);
          addCast(result_subtype, Lexeme::subtype_numeric);

          // pop de
          addPopDE();

          // pop bc
          addPopBC();

        } else {
          syntaxError("CMD PAGE syntax error");
        }

        // call cmd_page (l = mode, e = delay #1, c = delay #2)
        addCall(def_cmd_page);

      } else {
        syntaxError("CMD statement invalid");
        return;
      }
    }

  } else {
    syntaxError("CMD with empty parameters");
  }
}

void Compiler::addEnableBasicSlot() {
  if (opts->megaROM) {
    // ld a, (EXPTBL)
    addLdAii(def_EXPTBL);
    // ld h,040h        ; <--- enable jump to here
    addLdH(0x40);
    // call ENASLT		; Select the ROM on page 4000h
    addCall(def_ENASLT);
    // ei
    addEI();
  } else {
    // call enable basic slot function
    if (enable_basic_mark) {
      addFix(enable_basic_mark->symbol);
    } else {
      enable_basic_mark = addMark();
    }
    addCall(0x0000);
  }
}

void Compiler::addDisableBasicSlot() {
  if (opts->megaROM) {
    // ld a, (SLTSTR)
    addLdAii(def_SLTSTR);
    // ld h,040h        ; <--- enable jump to here
    addLdH(0x40);
    // call ENASLT		; Select the ROM on page 4000h
    addCall(def_ENASLT);
    // ei
    addEI();
  } else {
    // call disable basic slot function
    if (disable_basic_mark)
      addFix(disable_basic_mark->symbol);
    else
      disable_basic_mark = addMark();
    addCall(0x0000);
  }
}

void Compiler::addSupportSymbols() {
  // IO REDIRECT FUNCTION
  if (io_redirect_mark) {
    io_redirect_mark->symbol->address = code_pointer;
    // ld a, l  ; io number
    addLdAL();
    // ld hl, fake empty line
    addLdHL(def_ENDPRG);
    // ld ix, IOREDIRECT     ; in: a = i/o number
    addLdIX(def_IOREDIRECT);
    // call CALBAS
    addCall(def_CALBAS);
    // ei
    addEI();
    // ret
    addRet();
  }

  // IO SCREEN FUNCTION
  if (io_screen_mark) {
    io_screen_mark->symbol->address = code_pointer;
    // ld hl, fake empty line
    addLdHL(def_ENDPRG);
    // ld ix, IOTOSCREEN
    addLdIX(def_IOTOSCREEN);
    // call CALBAS
    addCall(def_CALBAS);
    // ei
    addEI();
    // ret
    addRet();
  }

  // DRAW STATEMENT - in: hl (pointer to string)
  if (draw_mark) {
    draw_mark->symbol->address = code_pointer;
    // ld a, (SCRMOD)
    addLdAii(def_SCRMOD);
    // cp 2
    addCp(0x02);
    // ret c                    ; return if text mode
    addRetC();

    // push hl
    addPushHL();

    addEnableBasicSlot();

    // pop hl
    addPopHL();

    // xor a
    addXorA();
    // ld bc, disable basic slot
    if (disable_basic_mark)
      addFix(disable_basic_mark->symbol);
    else
      disable_basic_mark = addMark();
    addLdBC(0x0000);
    // push bc
    addPushBC();
    // ld bc, 0
    addLdBC(0x0000);
    // push bc
    addPushBC();
    // push bc
    addPushBC();
    // push bc
    addPushBC();
    // ld de, 0x5D83
    addLdDE(0x5D83);
    // ld (0xFCBB), a   ; DRWFLG
    addLdiiA(0xFCBB);
    // ld (0xF958), a   ; MCLFLG
    addLdiiA(0xF958);
    // ld (0xF956), de  ; MCLTAB
    addLdiiDE(0xF956);
    // ld a, (hl)
    addLdAiHL();
    // inc hl
    addIncHL();
    // ld (0xFB3B), a   ; MCLLEN
    addLdiiA(0xFB3B);
    // jp 0x5691    ; DRAW subroutine = 0x568C+5  (main routine hook address =
    // 0x39A8)
    addJp(0x5691);
  }

  // ENABLE BASIC SLOT FUNCTION
  if (enable_basic_mark) {
    enable_basic_mark->symbol->address = code_pointer;
    // ld a, (EXPTBL)
    addLdAii(def_EXPTBL);
    // jr $+4            ; jump to disable code
    addJr(0x03);
  }

  // DISABLE BASIC SLOT FUNCTION
  if (disable_basic_mark) {
    disable_basic_mark->symbol->address = code_pointer;
    // ld a, (SLTSTR)
    addLdAii(def_SLTSTR);
    // ld h,040h        ; <--- enable jump to here
    addLdH(0x40);
    // call ENASLT		; Select the ROM on page 4000h
    addCall(def_ENASLT);
    // ei
    addEI();
    // ret
    addRet();
  }
}

//-------------------------------------------------------------------------------------------

void Compiler::syntaxError() {
  syntaxError("Syntax error");
}

void Compiler::syntaxError(string msg) {
  compiled = false;
  error_message = msg;
  if (current_tag) error_message += " (line=" + current_tag->value + ")";
}
