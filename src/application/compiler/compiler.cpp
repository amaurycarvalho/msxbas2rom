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

#include "build_options.h"
#include "code_node.h"
#include "compiler_code_helper.h"
#include "compiler_code_optimizer.h"
#include "compiler_context.h"
#include "compiler_end_statement_strategy.h"
#include "compiler_evaluator.h"
#include "compiler_expression_evaluator.h"
#include "compiler_fixup_resolver.h"
#include "compiler_float_converter.h"
#include "compiler_hooks.h"
#include "compiler_start_statement_strategy.h"
#include "compiler_symbol_resolver.h"
#include "compiler_variable_emitter.h"
#include "lexeme.h"
#include "lexer_line_context.h"
#include "logger.h"
#include "parser.h"
#include "resources.h"
#include "symbol_export_context.h"
#include "symbol_manager.h"
#include "symbol_node.h"
#include "tag_node.h"

/***
 * @name Compiler class code
 */

Compiler::Compiler(shared_ptr<ICpuOpcodeWriter> cpu) {
  context = make_shared<CompilerContext>();
  context->setHelpers(context);
  if (cpu) {
    workspace = make_shared<CpuWorkspaceContext>(
        COMPILE_CODE_SIZE, COMPILE_RAM_SIZE, def_RAM_BOTTOM);
    cpu->context = workspace;
    context->cpu = cpu;
  }
  context->current_tag = 0;
  context->compiled = false;
}

Compiler::~Compiler() = default;

int Compiler::getCodeSize() const {
  return context->cpu->context->code_size;
}

float Compiler::getRamMemoryPerc() const {
  return context->cpu->context->ram_memory_perc;
}

bool Compiler::getPt3() const {
  return context->pt3;
}

bool Compiler::getAkm() const {
  return context->akm;
}

bool Compiler::getFont() const {
  return context->font;
}

bool Compiler::getHasTinySprite() const {
  return context->has_tiny_sprite;
}

shared_ptr<Logger> Compiler::getLogger() {
  return context->logger;
}

shared_ptr<SymbolManager> Compiler::getSymbolManager() {
  return context->symbolManager;
}

const shared_ptr<SymbolManager> Compiler::getSymbolManager() const {
  return context->symbolManager;
}

shared_ptr<ResourceManager> Compiler::getResourceManager() {
  return context->resourceManager;
}
const shared_ptr<ResourceManager> Compiler::getResourceManager() const {
  return context->resourceManager;
}

int Compiler::getRamSize() const {
  return context->cpu->context->ram_size;
}

shared_ptr<TagNode> Compiler::getCurrentTag() const {
  return context->current_tag;
}

shared_ptr<Parser> Compiler::getParser() const {
  return context->parser;
}

shared_ptr<BuildOptions> Compiler::getOpts() const {
  return context->opts;
}

bool Compiler::isCompiled() const {
  return context->compiled;
}

bool Compiler::build(shared_ptr<Parser> parser) {
  CompilerStartStatementStrategy startStmt;
  CompilerEndStatementStrategy endStmt;
  shared_ptr<TagNode> tag;
  shared_ptr<SymbolNode> symbol;
  shared_ptr<CodeNode> codeItem;
  unsigned int i, t;
  string compiledCodeInfo;

  context->parser = parser;
  context->opts = parser->getOpts();

  context->cpu->context->clear();

  context->symbolResolver->clearSymbols();

  context->logger->setFile(context->opts->inputFilename);

  t = parser->getTags().size();
  context->compiled = (t > 0);

  /// @remark END statement needs to be here (first segment) for MegaROM
  /// support
  context->logger->debug("Registering END statement...");

  codeItem = make_shared<CodeNode>();
  codeItem->name = "END_STMT";
  codeItem->start = context->cpu->context->code_pointer;
  //! register END statement code
  endStmt.registerEndRoutine(context);
  codeItem->length = context->cpu->context->code_pointer - codeItem->start;
  codeItem->is_code = true;
  codeItem->debug = true;
  context->symbolManager->context->codeList.push_back(codeItem);
  context->logger->debug("+--> size: " + to_string(codeItem->length) +
                         " byte(s)");

  context->logger->debug("Registering start of program...");

  codeItem = make_shared<CodeNode>();
  codeItem->name = "START_PGM";
  codeItem->start = context->cpu->context->code_pointer;
  startStmt.execute(context);
  codeItem->length = context->cpu->context->code_pointer - codeItem->start;
  codeItem->is_code = true;
  codeItem->debug = true;
  context->symbolManager->context->codeList.push_back(codeItem);
  context->logger->debug("+--> size: " + to_string(codeItem->length) +
                         " byte(s)");
  if (codeItem->length >= 0x4000) {
    context->syntaxError(
        "Maximum of start of program code per ROM reached (16k)");
    return false;
  }

  context->logger->debug("Registering compiled code (line/bytes):");
  compiledCodeInfo = "";

  for (i = 0; i < t; i++) {
    tag = parser->getTags()[i];

    if (tag) {
      context->logger->setLineNumber(i + 1);

      if (i)
        compiledCodeInfo += ", " + tag->name;  /// code line
      else
        compiledCodeInfo += tag->name;  /// code line

      if (tag->name != "DIRECTIVE") {
        // Trim leading zeros
        while (tag->name.find("0") == 0 && tag->name.size() > 1) {
          tag->name.erase(0, 1);
        }

        symbol = context->symbolResolver->getSymbol(tag);
        if (symbol) {
          if (symbol->address) {
            compiledCodeInfo += " error";
            context->current_tag = tag;
            context->syntaxError("Line number already declared");
            context->logger->info(tag->toString());
            break;
          }
        } else
          symbol = context->symbolResolver->addSymbol(tag);

        symbol->address = context->cpu->context->code_pointer;
      }

      codeItem = make_shared<CodeNode>();
      codeItem->start = context->cpu->context->code_pointer;
      codeItem->name = "LIN_" + tag->name;
      if (tag->lexerLine) codeItem->lineNumber = tag->lexerLine->lineNumber;

      if (!context->evaluator->evaluate(tag)) {
        compiledCodeInfo += " error";
        context->compiled = false;
        context->logger->info(tag->toString());
        break;
      }

      codeItem->length = context->cpu->context->code_pointer - codeItem->start;
      codeItem->is_code = true;
      codeItem->debug = true;
      context->symbolManager->context->codeList.push_back(codeItem);

      compiledCodeInfo += "/" + to_string(codeItem->length);  /// code bytes

      if (codeItem->length >= 0x4000) {
        compiledCodeInfo += " error";
        context->logger->debug(compiledCodeInfo);
        context->syntaxError("Maximum of code per line per ROM reached (16k)");
        context->logger->info(tag->toString());
        return false;
      }
    }
  }

  context->logger->debug(compiledCodeInfo);

  if (context->compiled) {
    if (context->forNextStack.size()) {
      context->current_tag = context->forNextStack.top()->tag;
      if (context->current_tag->lexerLine)
        context->logger->setLineNumber(
            context->current_tag->lexerLine->lineNumber);
      context->syntaxError("FOR without a NEXT");
      context->logger->info(context->current_tag->toString());
    }
  }

  if (context->compiled) {
    context->current_tag = 0;

    /// @remark END is always the last statement of the program
    context->logger->debug("Registering end of program...");

    codeItem = make_shared<CodeNode>();
    codeItem->name = "END_PGM";
    codeItem->start = context->cpu->context->code_pointer;
    //! jump to the real END statement
    endStmt.execute(context);
    codeItem->length = context->cpu->context->code_pointer - codeItem->start;
    codeItem->is_code = true;
    codeItem->debug = true;
    context->symbolManager->context->codeList.push_back(codeItem);
    context->logger->debug("+--> " + to_string(codeItem->length) + " byte(s)");
    if (codeItem->length >= 0x4000) {
      context->syntaxError(
          "Maximum of end of program code per ROM reached (16k)");
      return false;
    }

    context->logger->debug("Registering support code...");

    codeItem = make_shared<CodeNode>();
    codeItem->start = context->cpu->context->code_pointer;
    context->symbolResolver->addSupportSymbols();
    codeItem->length = context->cpu->context->code_pointer - codeItem->start;
    codeItem->is_code = true;
    codeItem->debug = false;
    context->symbolManager->context->codeList.push_back(codeItem);
    context->logger->debug("+--> " + to_string(codeItem->length) + " byte(s)");
    if (codeItem->length >= 0x4000) {
      context->syntaxError("Maximum of support code per ROM reached (16k)");
      return false;
    }

    if (parser->getHasIData()) {
      context->logger->debug("Registering IDATA resource...");
      context->resourceManager->addIDataResource(parser);
    }

    if (parser->getHasData()) {
      context->logger->debug("Registering DATA resource...");
      context->resourceManager->addDataResource(parser);
    }

    context->logger->debug("Registering symbols...");

    i = context->symbolResolver->saveSymbols();

    context->logger->debug("+--> " + to_string(i) + " byte(s)");

    context->logger->debug("Adjusting code/data memory address...");

    context->fixupResolver->doFix();

    //! @todo needs refactor the code below to eliminate its magic constants
    int freeRamSize = def_RAM_SIZE;
    if (context->file_support)
      freeRamSize = 5931;  // with disk support
    else
      freeRamSize = 10534;  // without disk support
    freeRamSize += 0x538;   // kernel RAM internal usage

    context->cpu->context->ram_memory_perc =
        (context->cpu->context->ram_size * 100.0) / freeRamSize;
  }

  return context->compiled;
}

int Compiler::write(unsigned char* dest, int start_address) {
  unsigned int i, t;
  unsigned int k, tt = 0, step;
  unsigned char *d, *s;
  int address, new_address, addr_within_segm;
  int segm_from, segm_to, length;
  shared_ptr<FixNode> fix, skip;
  shared_ptr<CodeNode> codeItem;
  vector<shared_ptr<FixNode>> skips;
  bool is_id, is_jump, is_load;

  // copy compiled code to final destination

  t = context->symbolManager->context->codeList.size();
  addr_within_segm = start_address;

  if (context->opts->megaROM) {
    skips.clear();

    context->cpu->context->segm_last =
        2;  // last ROM segment starts at segment 2
    context->cpu->context->segm_total = 4;  // 4 segments of 8kb (0, 1, 2, 3)
    length = (start_address - 0x8000);
    context->cpu->context->code_size = 0;
    d = dest;

    for (i = 0; i < t; i++) {
      codeItem = context->symbolManager->context->codeList[i];

      if (codeItem->length) {
        s = &context->cpu->context->code[codeItem->start];

        if (codeItem->is_code)
          step = 8;
        else
          step = 3;

        tt = (length + codeItem->length + step);

        if (tt >= 0x4000) {
          context->cpu->context->segm_last +=
              2;  // konami segments size are 8kb (0/1, 2/3...)
          addr_within_segm = 0x8000;

          if (codeItem->is_code) {
            // code to skip a segment to another

            // ld a, segmt
            d[0] = 0x3E;
            d[1] = context->cpu->context
                       ->segm_last;  // extra code will start after segment 3
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

          skip = make_shared<FixNode>();
          skip->address = codeItem->start;
          skip->step = step;
          skips.push_back(skip);

          context->cpu->context->code_size += length;

          length = 0;
        }

        memcpy(d, s, codeItem->length);

        codeItem->addr_within_segm = addr_within_segm;
        codeItem->segm = context->cpu->context->segm_last;
        if (codeItem->addr_within_segm >= (0xA000)) codeItem->segm++;

        d += codeItem->length;
        length += codeItem->length;
        addr_within_segm += codeItem->length;
      } else {
        codeItem->addr_within_segm = addr_within_segm;
        codeItem->segm = context->cpu->context->segm_last;
        if (codeItem->addr_within_segm >= (0xA000)) codeItem->segm++;
      }
    }

    context->cpu->context->code_size += length;

    context->cpu->context->segm_total =
        ((context->cpu->context->segm_last + 1) / 16 + 1) * 16;

    tt = skips.size();

  } else {
    for (i = 0; i < t; i++) {
      codeItem = context->symbolManager->context->codeList[i];
      codeItem->segm = 0;
      codeItem->addr_within_segm = addr_within_segm;
      addr_within_segm += codeItem->length;
    }

    memcpy(dest, context->cpu->context->code.data(),
           context->cpu->context->code_size);
  }

  // reallocate code pointers to new start address

  t = context->fixes.size();

  for (i = 0; i < t; i++) {
    fix = context->fixes[i];
    if (fix) {
      if (fix->symbol) {
        if (fix->symbol->lexeme) {
          is_id = (fix->symbol->lexeme->type == Lexeme::type_identifier);
        } else {
          is_id = false;
        }

        address = fix->address;

        if (context->opts->megaROM) {
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
        context->syntaxError("Error fixing symbol (null)");
    } else
      context->syntaxError("Error in the fix object (null)");
  }

  return context->cpu->context->code_size;
}
