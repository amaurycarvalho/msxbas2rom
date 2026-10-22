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
#include "compiler_hooks.h"
#include "compiler_end_statement_strategy.h"
#include "compiler_evaluator.h"
#include "compiler_expression_evaluator.h"
#include "compiler_fixup_resolver.h"
#include "compiler_float_converter.h"
#include "compiler_hooks.h"
#include "compiler_start_statement_strategy.h"
#include "compiler_symbol_resolver.h"
#include "compiler_variable_emitter.h"
#include "kernel_image.h"
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

Compiler::Compiler(shared_ptr<ICpuOpcodeWriter> cpu,
                   shared_ptr<KernelImage> kernelImage,
                   shared_ptr<ResourceContentSource> resourceContentSource,
                   shared_ptr<SymbolOutputSink> symbolOutputSink) {
  context = make_shared<CompilerContext>(kernelImage, resourceContentSource,
                                         symbolOutputSink);
  context->setHelpers(context);
  if (cpu) {
    workspace = make_shared<CpuWorkspaceContext>(
        COMPILE_CODE_SIZE, COMPILE_RAM_SIZE, def_RAM_BOTTOM);
    cpu->context = workspace;
    context->getCpu() = cpu;
  }
  context->getCurrentTag() = 0;
  context->getCompiled() = false;
}

Compiler::~Compiler() = default;

shared_ptr<CompilerContext> Compiler::getContext() const {
  return context;
}

int Compiler::getCodeSize() const {
  return context->getCpu()->context->getCodeSize();
}

float Compiler::getRamMemoryPerc() const {
  return context->getCpu()->context->getRamMemoryPerc();
}

bool Compiler::getPt3() const {
  return context->getPt3();
}

bool Compiler::getAkm() const {
  return context->getAkm();
}

bool Compiler::getFont() const {
  return context->getFont();
}

bool Compiler::getHasTinySprite() const {
  return context->getHasTinySprite();
}

shared_ptr<Logger> Compiler::getLogger() {
  return context->getLogger();
}

shared_ptr<SymbolManager> Compiler::getSymbolManager() {
  return context->getSymbolManager();
}

const shared_ptr<SymbolManager> Compiler::getSymbolManager() const {
  return context->getSymbolManager();
}

shared_ptr<ResourceManager> Compiler::getResourceManager() {
  return context->getResourceManager();
}
const shared_ptr<ResourceManager> Compiler::getResourceManager() const {
  return context->getResourceManager();
}

int Compiler::getRamSize() const {
  return context->getCpu()->context->getRamSize();
}

shared_ptr<TagNode> Compiler::getCurrentTag() const {
  return context->getCurrentTag();
}

shared_ptr<Parser> Compiler::getParser() const {
  return context->getParser();
}

shared_ptr<BuildOptions> Compiler::getOpts() const {
  return context->getOpts();
}

shared_ptr<KernelImage> Compiler::getKernelImage() const {
  return context->getKernelImage();
}

bool Compiler::isCompiled() const {
  return context->getCompiled();
}

bool Compiler::build(shared_ptr<Parser> parser) {
  CompilerStartStatementStrategy startStmt;
  CompilerEndStatementStrategy endStmt;
  shared_ptr<TagNode> tag;
  shared_ptr<SymbolNode> symbol;
  shared_ptr<CodeNode> codeItem;
  unsigned int i, t;
  string compiledCodeInfo;

  context->getParser() = parser;
  context->getOpts() = parser->getOpts();

  context->getCpu()->context->clear();

  context->getSymbolResolver()->clearSymbols();

  context->getLogger()->setFile(context->getOpts()->inputFilename);

  t = parser->getTags().size();
  context->getCompiled() = (t > 0);

  /// @remark END statement needs to be here (first segment) for MegaROM
  /// support
  context->getLogger()->debug("Registering END statement...");

  codeItem = make_shared<CodeNode>();
  codeItem->name = "END_STMT";
  codeItem->start = context->getCpu()->context->getCodePointer();
  //! register END statement code
  endStmt.registerEndRoutine(context);
  codeItem->length = context->getCpu()->context->getCodePointer() - codeItem->start;
  codeItem->is_code = true;
  codeItem->debug = true;
  context->getSymbolManager()->context->getCodeList().push_back(codeItem);
  context->getLogger()->debug("+--> size: " + to_string(codeItem->length) +
                         " byte(s)");

  context->getLogger()->debug("Registering start of program...");

  codeItem = make_shared<CodeNode>();
  codeItem->name = "START_PGM";
  codeItem->start = context->getCpu()->context->getCodePointer();
  startStmt.execute(context);
  codeItem->length = context->getCpu()->context->getCodePointer() - codeItem->start;
  codeItem->is_code = true;
  codeItem->debug = true;
  context->getSymbolManager()->context->getCodeList().push_back(codeItem);
  context->getLogger()->debug("+--> size: " + to_string(codeItem->length) +
                         " byte(s)");
  if (codeItem->length >= 0x4000) {
    context->syntaxError(
        "Maximum of start of program code per ROM reached (16k)");
    return false;
  }

  context->getLogger()->debug("Registering compiled code (line/bytes):");
  compiledCodeInfo = "";

  for (i = 0; i < t; i++) {
    tag = parser->getTags()[i];

    if (tag) {
      context->getLogger()->setLineNumber(tag->lexerLine ? tag->lexerLine->lineNumber : i + 1);

      if (i)
        compiledCodeInfo += ", " + tag->name;  /// code line
      else
        compiledCodeInfo += tag->name;  /// code line

      if (tag->name != "DIRECTIVE") {
        // Trim leading zeros
        while (tag->name.find("0") == 0 && tag->name.size() > 1) {
          tag->name.erase(0, 1);
        }

        symbol = context->getSymbolResolver()->getSymbol(tag);
        if (symbol) {
          if (symbol->address) {
            compiledCodeInfo += " error";
            context->getCurrentTag() = tag;
            context->syntaxError("Line number already declared");
            context->getLogger()->info(tag->toString());
            break;
          }
        } else
          symbol = context->getSymbolResolver()->addSymbol(tag);

        symbol->address = context->getCpu()->context->getCodePointer();
      }

      codeItem = make_shared<CodeNode>();
      codeItem->start = context->getCpu()->context->getCodePointer();
      codeItem->name = "LIN_" + tag->name;
      if (tag->lexerLine) codeItem->lineNumber = tag->lexerLine->lineNumber;

      if (!context->getEvaluator()->evaluate(tag)) {
        compiledCodeInfo += " error";
        context->getCompiled() = false;
        context->getLogger()->info(tag->toString());
        break;
      }

      codeItem->length = context->getCpu()->context->getCodePointer() - codeItem->start;
      codeItem->is_code = true;
      codeItem->debug = true;
      context->getSymbolManager()->context->getCodeList().push_back(codeItem);

      compiledCodeInfo += "/" + to_string(codeItem->length);  /// code bytes

      if (codeItem->length >= 0x4000) {
        compiledCodeInfo += " error";
        context->getLogger()->debug(compiledCodeInfo);
        context->syntaxError("Maximum of code per line per ROM reached (16k)");
        context->getLogger()->info(tag->toString());
        return false;
      }
    }
  }

  context->getLogger()->debug(compiledCodeInfo);

  if (context->getCompiled()) {
    if (context->getForNextStack().size()) {
      context->getCurrentTag() = context->getForNextStack().top()->tag;
      if (context->getCurrentTag()->lexerLine)
        context->getLogger()->setLineNumber(
            context->getCurrentTag()->lexerLine->lineNumber);
      context->syntaxError("FOR without a NEXT");
      context->getLogger()->info(context->getCurrentTag()->toString());
    }
  }

  if (context->getCompiled()) {
    context->getCurrentTag() = 0;

    /// @remark END is always the last statement of the program
    context->getLogger()->debug("Registering end of program...");

    codeItem = make_shared<CodeNode>();
    codeItem->name = "END_PGM";
    codeItem->start = context->getCpu()->context->getCodePointer();
    //! jump to the real END statement
    endStmt.execute(context);
    codeItem->length = context->getCpu()->context->getCodePointer() - codeItem->start;
    codeItem->is_code = true;
    codeItem->debug = true;
    context->getSymbolManager()->context->getCodeList().push_back(codeItem);
    context->getLogger()->debug("+--> " + to_string(codeItem->length) + " byte(s)");
    if (codeItem->length >= 0x4000) {
      context->syntaxError(
          "Maximum of end of program code per ROM reached (16k)");
      return false;
    }

    context->getLogger()->debug("Registering support code...");

    codeItem = make_shared<CodeNode>();
    codeItem->start = context->getCpu()->context->getCodePointer();
    context->getSymbolResolver()->addSupportSymbols();
    codeItem->length = context->getCpu()->context->getCodePointer() - codeItem->start;
    codeItem->is_code = true;
    codeItem->debug = false;
    context->getSymbolManager()->context->getCodeList().push_back(codeItem);
    context->getLogger()->debug("+--> " + to_string(codeItem->length) + " byte(s)");
    if (codeItem->length >= 0x4000) {
      context->syntaxError("Maximum of support code per ROM reached (16k)");
      return false;
    }

    if (parser->getHasIData()) {
      context->getLogger()->debug("Registering IDATA resource...");
      context->getResourceManager()->addIDataResource(parser);
    }

    if (parser->getHasData()) {
      context->getLogger()->debug("Registering DATA resource...");
      context->getResourceManager()->addDataResource(parser);
    }

    context->getLogger()->debug("Registering symbols...");

    i = context->getSymbolResolver()->saveSymbols();

    context->getLogger()->debug("+--> " + to_string(i) + " byte(s)");

    context->getLogger()->debug("Adjusting code/data memory address...");

    context->getFixupResolver()->doFix();

    int varRam = context->getFileSupport() ? def_VAR_RAM_DISK : def_VAR_RAM_NON_DISK;
    int freeRamSize = varRam + def_KERNEL_RAM_OVERHEAD;

    context->getCpu()->context->getRamMemoryPerc() =
        (context->getCpu()->context->getRamSize() * 100.0) / freeRamSize;
  }

  return context->getCompiled();
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

  // Resolve MR wrapper target addresses from the read-only kernel image
  unsigned int mr_call_target =
      context->getKernelImage() ? context->getKernelImage()->dispatchAddress(DISP_MR_CALL)
                           : 0;
  unsigned int mr_jump_target =
      context->getKernelImage() ? context->getKernelImage()->dispatchAddress(DISP_MR_JUMP)
                           : 0;
  unsigned int mr_get_data_target =
      context->getKernelImage()
          ? context->getKernelImage()->dispatchAddress(DISP_MR_GET_DATA)
          : 0;

  // copy compiled code to final destination

  t = context->getSymbolManager()->context->getCodeList().size();
  addr_within_segm = start_address;

  if (context->getOpts()->megaROM) {
    skips.clear();

    context->getCpu()->context->getSegmLast() =
        2;  // last ROM segment starts at segment 2
    context->getCpu()->context->getSegmTotal() = 4;  // 4 segments of 8kb (0, 1, 2, 3)
    length = (start_address - 0x8000);
    context->getCpu()->context->getCodeSize() = 0;
    d = dest;

    if (context->getCpu()->context->getError()) {
      context->getLogger()->error(
          "Compiled code exceeds maximum limit of " +
          to_string(COMPILE_CODE_SIZE) + " bytes (" +
          to_string(COMPILE_MAX_PAGES) + " pages)");
      return 0;
    }

    for (i = 0; i < t; i++) {
      codeItem = context->getSymbolManager()->context->getCodeList()[i];

      if (codeItem->length) {
        s = &context->getCpu()->context->getCode()[codeItem->start];

        if (codeItem->is_code)
          step = 8;
        else
          step = 3;

        tt = (length + codeItem->length + step);

        if (tt >= 0x4000) {
          context->getCpu()->context->getSegmLast() +=
              2;  // konami segments size are 8kb (0/1, 2/3...)
          addr_within_segm = 0x8000;

          if (codeItem->is_code) {
            // code to skip a segment to another

            // ld a, segmt
            d[0] = 0x3E;
            d[1] = context->getCpu()->context->getSegmLast();
            // extra code will start after segment 3
            // ld hl, 0x8000
            d[2] = 0x21;
            d[3] = 0x00;
            d[4] = 0x80;
            // jp MR_JUMP
            d[5] = 0xC3;
            d[6] = (mr_jump_target & 0xFF);
            d[7] = ((mr_jump_target >> 8) & 0xFF);

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

          context->getCpu()->context->getCodeSize() += length;

          length = 0;
        }

        memcpy(d, s, codeItem->length);

        codeItem->addr_within_segm = addr_within_segm;
        codeItem->segm = context->getCpu()->context->getSegmLast();
        if (codeItem->addr_within_segm >= (0xA000)) codeItem->segm++;

        d += codeItem->length;
        length += codeItem->length;
        addr_within_segm += codeItem->length;
      } else {
        codeItem->addr_within_segm = addr_within_segm;
        codeItem->segm = context->getCpu()->context->getSegmLast();
        if (codeItem->addr_within_segm >= (0xA000)) codeItem->segm++;
      }
    }

    context->getCpu()->context->getCodeSize() += length;

    context->getCpu()->context->getSegmTotal() =
        ((context->getCpu()->context->getSegmLast() + 1) / 16 + 1) * 16;

    tt = skips.size();

  } else {
    for (i = 0; i < t; i++) {
      codeItem = context->getSymbolManager()->context->getCodeList()[i];
      codeItem->segm = 0;
      codeItem->addr_within_segm = addr_within_segm;
      addr_within_segm += codeItem->length;
    }

    memcpy(dest, context->getCpu()->context->getCode().data(),
           context->getCpu()->context->getCodeSize());
  }

  // reallocate code pointers to new start address

  t = context->getFixes().size();

  for (i = 0; i < t; i++) {
    fix = context->getFixes()[i];
    if (fix) {
      if (fix->symbol) {
        if (fix->symbol->lexeme) {
          is_id = (fix->symbol->lexeme->type == Lexeme::type_identifier);
        } else {
          is_id = false;
        }

        address = fix->address;

        if (context->getOpts()->megaROM) {
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
                  dest[address] = mr_call_target & 0xFF;
                  dest[address + 1] = (mr_call_target >> 8) & 0xFF;
                } break;
                case 0xD4: {                  // call nc,
                  dest[address - 10] = 0x38;  // jr c, $+11
                  dest[address - 9] = 0x0A;
                  dest[address - 1] = 0xCD;  // change to call
                  dest[address] = mr_call_target & 0xFF;
                  dest[address + 1] = (mr_call_target >> 8) & 0xFF;
                } break;
                case 0xC4: {                  // call nz,
                  dest[address - 10] = 0x28;  // jr z, $+11
                  dest[address - 9] = 0x0A;
                  dest[address - 1] = 0xCD;  // change to call
                  dest[address] = mr_call_target & 0xFF;
                  dest[address + 1] = (mr_call_target >> 8) & 0xFF;
                } break;
                case 0xDC: {                  // call c,
                  dest[address - 10] = 0x30;  // jr nc, $+11
                  dest[address - 9] = 0x0A;
                  dest[address - 1] = 0xCD;  // change to call
                  dest[address] = mr_call_target & 0xFF;
                  dest[address + 1] = (mr_call_target >> 8) & 0xFF;
                } break;
                case 0xCC: {                  // call z,
                  dest[address - 10] = 0x20;  // jr nz, $+11
                  dest[address - 9] = 0x0A;
                  dest[address - 1] = 0xCD;  // change to call
                  dest[address] = mr_call_target & 0xFF;
                  dest[address + 1] = (mr_call_target >> 8) & 0xFF;
                } break;
                case 0xCD: {  // call
                  dest[address] = mr_call_target & 0xFF;
                  dest[address + 1] = (mr_call_target >> 8) & 0xFF;
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
                  dest[address] = mr_jump_target & 0xFF;
                  dest[address + 1] = (mr_jump_target >> 8) & 0xFF;
                } break;
                case 0xD2: {                  // jp nc,
                  dest[address - 10] = 0x38;  // jr c, $+11
                  dest[address - 9] = 0x0A;
                  dest[address - 1] = 0xC3;  // change to jp
                  dest[address] = mr_jump_target & 0xFF;
                  dest[address + 1] = (mr_jump_target >> 8) & 0xFF;
                } break;
                case 0xC2: {                  // jp nz,
                  dest[address - 10] = 0x28;  // jr z, $+11
                  dest[address - 9] = 0x0A;
                  dest[address - 1] = 0xC3;  // change to jp
                  dest[address] = mr_jump_target & 0xFF;
                  dest[address + 1] = (mr_jump_target >> 8) & 0xFF;
                } break;
                case 0xDA: {                  // jp c,
                  dest[address - 10] = 0x30;  // jr nc, $+11
                  dest[address - 9] = 0x0A;
                  dest[address - 1] = 0xC3;  // change to jp
                  dest[address] = mr_jump_target & 0xFF;
                  dest[address + 1] = (mr_jump_target >> 8) & 0xFF;
                } break;
                case 0xCA: {                  // jp z,
                  dest[address - 10] = 0x20;  // jr nz, $+11
                  dest[address - 9] = 0x0A;
                  dest[address - 1] = 0xC3;  // change to jp
                  dest[address] = mr_jump_target & 0xFF;
                  dest[address + 1] = (mr_jump_target >> 8) & 0xFF;
                } break;
                case 0xC3: {  // jp
                  dest[address] = mr_jump_target & 0xFF;
                  dest[address + 1] = (mr_jump_target >> 8) & 0xFF;
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
                  dest[address] = mr_get_data_target & 0xFF;
                  dest[address + 1] = (mr_get_data_target >> 8) & 0xFF;
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

  return context->getCpu()->context->getCodeSize();
}
