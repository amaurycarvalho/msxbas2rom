/***
 * @file compiler_code_optimizerr.cpp
 * @brief Compiler code optimizer
 * @author Amaury Carvalho (2019-2026)
 */

#include "compiler_code_optimizer.h"

#include "build_options.h"
#include "compiler_context.h"
#include "compiler_hooks.h"
#include "fix_node.h"
#include "kernel_image.h"

void CompilerCodeOptimizer::addByteOptimized(unsigned char byte) {
  auto& cpu = *context->getCpu();
  auto& opts = *context->getOpts();
  auto& cpuctx = *cpu.context;
  bool optimized = false;
  int code_pointer_saved = cpuctx.getCodePointer();
  int code_reduced = 0;
  unsigned char* s;

  if (!opts.megaROM) {
    switch (byte) {
      // pop de
      case 0xD1: {
        // ld hl,x | push hl | ld hl,x | pop de
        if (*cpuctx.getCodePipeline()[0] == 0x21 &&
            *cpuctx.getCodePipeline()[1] == 0xE5 &&
            *cpuctx.getCodePipeline()[2] == 0x21 &&
            *cpuctx.getCodePipeline()[3] != 0xDD &&
            *cpuctx.getCodePipeline()[3] != 0xFD) {
          // optimize to ld de,x | ld hl,x
          s = cpuctx.getCodePipeline()[2];
          s[0] = 0x11;
          s[3] = 0x21;
          s[4] = s[5];
          s[5] = s[6];
          cpu.popLastCode();
          cpuctx.getCodePipeline()[1] = s;
          cpuctx.getCodePipeline()[0] = &s[3];
          code_pointer_saved = (cpuctx.getCodePipeline()[0] - &cpuctx.getCode()[0]);
          code_reduced = 1;
          cpuctx.getCodePointer() -= code_reduced;
          cpuctx.getCodeSize() -= code_reduced;
          optimized = true;

          // push hl | ld hl,x | pop de
        } else if (*cpuctx.getCodePipeline()[0] == 0x21 &&
                   *cpuctx.getCodePipeline()[1] == 0xE5) {
          // optimize to ex de,hl | ld hl,x
          s = cpuctx.getCodePipeline()[1];
          s[0] = 0xEB;
          optimized = true;

          // ld hl,(x) | push hl | ld hl,(x) | pop de
        } else if (*cpuctx.getCodePipeline()[0] == 0x2A &&
                   *cpuctx.getCodePipeline()[1] == 0xE5 &&
                   *cpuctx.getCodePipeline()[2] == 0x2A &&
                   *cpuctx.getCodePipeline()[3] != 0xDD &&
                   *cpuctx.getCodePipeline()[3] != 0xFD) {
          // optimize to ld hl,(x) | ex de,hl | ld hl,(x)
          s = cpuctx.getCodePipeline()[1];
          s[0] = 0xEB;
          optimized = true;

          // push hl | ld hl,(x) | pop de
        } else if (*cpuctx.getCodePipeline()[0] == 0x2A &&
                   *cpuctx.getCodePipeline()[1] == 0xE5) {
          // optimize to ex de,hl | ld hl,(x)
          s = cpuctx.getCodePipeline()[1];
          s[0] = 0xEB;
          optimized = true;
        }

      } break;

      // add hl,de
      case 0x19: {
        // ex de,hl | ld hl,x | add hl,de
        if (*cpuctx.getCodePipeline()[0] == 0x21 &&
            *cpuctx.getCodePipeline()[1] == 0xEB) {
          s = cpuctx.getCodePipeline()[0];
          // optimize to inc hl if 1, 2 or 3
          if (s[2] == 0x00 && s[1] == 1) {
            s = cpuctx.getCodePipeline()[1];
            s[0] = 0x23;
            cpuctx.getCodePointer() -= 3;
            cpuctx.getCodeSize() -= 3;
            cpu.popLastCode();
            optimized = true;
          } else if (s[2] == 0x00 && s[1] == 2) {
            s = cpuctx.getCodePipeline()[1];
            s[0] = 0x23;
            s[1] = 0x23;
            cpuctx.getCodePointer() -= 2;
            cpuctx.getCodeSize() -= 2;
            cpu.popLastCode();
            optimized = true;
          } else if (s[2] == 0x00 && s[1] == 3) {
            s = cpuctx.getCodePipeline()[1];
            s[0] = 0x23;
            s[1] = 0x23;
            s[2] = 0x23;
            cpuctx.getCodePointer()--;
            cpuctx.getCodeSize()--;
            cpu.popLastCode();
            optimized = true;
          } else {
            // optimize to ld de,x | add hl,de
            s = cpuctx.getCodePipeline()[1];
            s[0] = 0x11;
            s[1] = s[2];
            s[2] = s[3];
            s[3] = byte;
            cpuctx.getCodePipeline()[0] = &s[3];
            optimized = true;
          }

          // ld hl,(x) | ld de,x | add hl,de
        } else if (*cpuctx.getCodePipeline()[0] == 11 &&
                   *cpuctx.getCodePipeline()[1] == 0x2A) {
          // optimize to ld hl,(x) | inc hl if 1, 2 or 3
          s = cpuctx.getCodePipeline()[0];
          if (s[2] == 0x00) {
            switch (s[1]) {
              case 1: {
                s[0] = 0x23;
                cpuctx.getCodePointer() -= 2;
                cpuctx.getCodeSize() -= 2;
                optimized = true;
              } break;

              case 2: {
                s[0] = 0x23;
                s[1] = 0x23;
                cpuctx.getCodePointer()--;
                cpuctx.getCodeSize()--;
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
        if (*cpuctx.getCodePipeline()[0] == 0x21 &&
            *cpuctx.getCodePipeline()[1] == 0xEB) {
          // optimize to ld de,x
          s = cpuctx.getCodePipeline()[1];
          s[0] = 0x11;
          s[1] = s[2];
          s[2] = s[3];
          cpu.popLastCode();
          cpuctx.getCodePipeline()[0] = s;
          code_pointer_saved = (s - &cpuctx.getCode()[0]);
          code_reduced = 1;
          cpuctx.getCodePointer()--;
          cpuctx.getCodeSize()--;
          optimized = true;

          // ld de,x | ld hl,x | ex de,hl
        } else if (*cpuctx.getCodePipeline()[0] == 0x21 &&
                   *cpuctx.getCodePipeline()[1] == 0x11 &&
                   *cpuctx.getCodePipeline()[2] != 0xDD &&
                   *cpuctx.getCodePipeline()[2] != 0xFD) {
          // optimize to ld hl,x | ld de,x
          s = cpuctx.getCodePipeline()[1];
          s[0] = 0x21;
          s[3] = 0x11;
          optimized = true;

          // ld hl,x | ld de,x | ex de,hl
        } else if (*cpuctx.getCodePipeline()[0] == 0x11 &&
                   *cpuctx.getCodePipeline()[1] == 0x21 &&
                   *cpuctx.getCodePipeline()[2] != 0xDD &&
                   *cpuctx.getCodePipeline()[2] != 0xFD) {
          // optimize to ld de,x | ld hl,x
          s = cpuctx.getCodePipeline()[1];
          s[0] = 0x11;
          s[3] = 0x21;
          optimized = true;
        }

      } break;
    }
  }

  if (optimized) {
    if (code_reduced) {
      unsigned int i, t = context->getFixes().size();
      shared_ptr<FixNode> fix;

      for (i = 0; i < t; i++) {
        fix = context->getFixes()[i];
        if (fix) {
          if (fix->address >= code_pointer_saved) {
            fix->address -= code_reduced;
          }
        }
      }
    }

  } else {
    cpu.pushLastCode();
    cpu.addCodeByte(byte);
  }
}

/***
 * @remarks
 * optimized kernel functions calls
 * skipping the jump map
 */
void CompilerCodeOptimizer::addKernelCall(unsigned int index) {
  auto& cpu = *context->getCpu();
  if (index >= DISP_ENTRIES) __throw_range_error("Invalid kernel call");

  context->getUsedDispatch().insert(index);

  DispatchFixup fixup;
  fixup.dispatchIndex = index;
  fixup.address = cpu.context->getCodePointer() + 1;
  context->getDispatchFixups().push_back(fixup);

  cpu.addCall(0x0000);
}

void CompilerCodeOptimizer::addKernelCallNZ(unsigned char index) {
  auto& cpu = *context->getCpu();
  if (index >= DISP_ENTRIES) __throw_range_error("Invalid kernel call");

  context->getUsedDispatch().insert(index);

  DispatchFixup fixup;
  fixup.dispatchIndex = index;
  fixup.address = cpu.context->getCodePointer() + 1;
  context->getDispatchFixups().push_back(fixup);

  cpu.addCallNZ(0x0000);
}

int CompilerCodeOptimizer::getKernelCallAddr(unsigned int address) {
  if (!context->getKernelImage()) return address;
  return context->getKernelImage()->resolveCall(address);
}

void CompilerCodeOptimizer::addLdHLmegarom() {
  auto& cpu = *context->getCpu();
  cpu.addCmd(0xFF, 0x0000);
}

CompilerCodeOptimizer::CompilerCodeOptimizer(
    shared_ptr<CompilerContext> context)
    : context(context) {}

CompilerCodeOptimizer::~CompilerCodeOptimizer() = default;
