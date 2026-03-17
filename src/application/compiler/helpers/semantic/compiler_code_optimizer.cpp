/***
 * @file compiler_code_optimizerr.cpp
 * @brief Compiler code optimizer
 * @author Amaury Carvalho (2019-2026)
 */

#include "compiler_code_optimizer.h"

#include "compiler_context.h"

extern unsigned char bin_header_bin[];

void CompilerCodeOptimizer::addByteOptimized(unsigned char byte) {
  auto& cpu = *context->cpu;
  auto& opts = *context->opts;
  auto& cpuctx = *cpu.context;
  bool optimized = false;
  int code_pointer_saved = cpuctx.code_pointer;
  int code_reduced = 0;
  unsigned char* s;

  if (!opts.megaROM) {
    switch (byte) {
      // pop de
      case 0xD1: {
        // ld hl,x | push hl | ld hl,x | pop de
        if (*cpuctx.code_pipeline[0] == 0x21 &&
            *cpuctx.code_pipeline[1] == 0xE5 &&
            *cpuctx.code_pipeline[2] == 0x21 &&
            *cpuctx.code_pipeline[3] != 0xDD &&
            *cpuctx.code_pipeline[3] != 0xFD) {
          // optimize to ld de,x | ld hl,x
          s = cpuctx.code_pipeline[2];
          s[0] = 0x11;
          s[3] = 0x21;
          s[4] = s[5];
          s[5] = s[6];
          cpu.popLastCode();
          cpuctx.code_pipeline[1] = s;
          cpuctx.code_pipeline[0] = &s[3];
          code_pointer_saved = (cpuctx.code_pipeline[0] - &cpuctx.code[0]);
          code_reduced = 1;
          cpuctx.code_pointer -= code_reduced;
          cpuctx.code_size -= code_reduced;
          optimized = true;

          // push hl | ld hl,x | pop de
        } else if (*cpuctx.code_pipeline[0] == 0x21 &&
                   *cpuctx.code_pipeline[1] == 0xE5) {
          // optimize to ex de,hl | ld hl,x
          s = cpuctx.code_pipeline[1];
          s[0] = 0xEB;
          optimized = true;

          // ld hl,(x) | push hl | ld hl,(x) | pop de
        } else if (*cpuctx.code_pipeline[0] == 0x2A &&
                   *cpuctx.code_pipeline[1] == 0xE5 &&
                   *cpuctx.code_pipeline[2] == 0x2A &&
                   *cpuctx.code_pipeline[3] != 0xDD &&
                   *cpuctx.code_pipeline[3] != 0xFD) {
          // optimize to ld hl,(x) | ex de,hl | ld hl,(x)
          s = cpuctx.code_pipeline[1];
          s[0] = 0xEB;
          optimized = true;

          // push hl | ld hl,(x) | pop de
        } else if (*cpuctx.code_pipeline[0] == 0x2A &&
                   *cpuctx.code_pipeline[1] == 0xE5) {
          // optimize to ex de,hl | ld hl,(x)
          s = cpuctx.code_pipeline[1];
          s[0] = 0xEB;
          optimized = true;
        }

      } break;

      // add hl,de
      case 0x19: {
        // ex de,hl | ld hl,x | add hl,de
        if (*cpuctx.code_pipeline[0] == 0x21 &&
            *cpuctx.code_pipeline[1] == 0xEB) {
          s = cpuctx.code_pipeline[0];
          // optimize to inc hl if 1, 2 or 3
          if (s[2] == 0x00 && s[1] == 1) {
            s = cpuctx.code_pipeline[1];
            s[0] = 0x23;
            cpuctx.code_pointer -= 3;
            cpuctx.code_size -= 3;
            cpu.popLastCode();
            optimized = true;
          } else if (s[2] == 0x00 && s[1] == 2) {
            s = cpuctx.code_pipeline[1];
            s[0] = 0x23;
            s[1] = 0x23;
            cpuctx.code_pointer -= 2;
            cpuctx.code_size -= 2;
            cpu.popLastCode();
            optimized = true;
          } else if (s[2] == 0x00 && s[1] == 3) {
            s = cpuctx.code_pipeline[1];
            s[0] = 0x23;
            s[1] = 0x23;
            s[2] = 0x23;
            cpuctx.code_pointer--;
            cpuctx.code_size--;
            cpu.popLastCode();
            optimized = true;
          } else {
            // optimize to ld de,x | add hl,de
            s = cpuctx.code_pipeline[1];
            s[0] = 0x11;
            s[1] = s[2];
            s[2] = s[3];
            s[3] = byte;
            cpuctx.code_pipeline[0] = &s[3];
            optimized = true;
          }

          // ld hl,(x) | ld de,x | add hl,de
        } else if (*cpuctx.code_pipeline[0] == 11 &&
                   *cpuctx.code_pipeline[1] == 0x2A) {
          // optimize to ld hl,(x) | inc hl if 1, 2 or 3
          s = cpuctx.code_pipeline[0];
          if (s[2] == 0x00) {
            switch (s[1]) {
              case 1: {
                s[0] = 0x23;
                cpuctx.code_pointer -= 2;
                cpuctx.code_size -= 2;
                optimized = true;
              } break;

              case 2: {
                s[0] = 0x23;
                s[1] = 0x23;
                cpuctx.code_pointer--;
                cpuctx.code_size--;
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
        if (*cpuctx.code_pipeline[0] == 0x21 &&
            *cpuctx.code_pipeline[1] == 0xEB) {
          // optimize to ld de,x
          s = cpuctx.code_pipeline[1];
          s[0] = 0x11;
          s[1] = s[2];
          s[2] = s[3];
          cpu.popLastCode();
          cpuctx.code_pipeline[0] = s;
          code_pointer_saved = (s - &cpuctx.code[0]);
          code_reduced = 1;
          cpuctx.code_pointer--;
          cpuctx.code_size--;
          optimized = true;

          // ld de,x | ld hl,x | ex de,hl
        } else if (*cpuctx.code_pipeline[0] == 0x21 &&
                   *cpuctx.code_pipeline[1] == 0x11 &&
                   *cpuctx.code_pipeline[2] != 0xDD &&
                   *cpuctx.code_pipeline[2] != 0xFD) {
          // optimize to ld hl,x | ld de,x
          s = cpuctx.code_pipeline[1];
          s[0] = 0x21;
          s[3] = 0x11;
          optimized = true;

          // ld hl,x | ld de,x | ex de,hl
        } else if (*cpuctx.code_pipeline[0] == 0x11 &&
                   *cpuctx.code_pipeline[1] == 0x21 &&
                   *cpuctx.code_pipeline[2] != 0xDD &&
                   *cpuctx.code_pipeline[2] != 0xFD) {
          // optimize to ld de,x | ld hl,x
          s = cpuctx.code_pipeline[1];
          s[0] = 0x11;
          s[3] = 0x21;
          optimized = true;
        }

      } break;
    }
  }

  if (optimized) {
    if (code_reduced) {
      unsigned int i, t = context->fixes.size();
      FixNode* fix;

      for (i = 0; i < t; i++) {
        fix = context->fixes[i];
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
void CompilerCodeOptimizer::addKernelCall(unsigned int address) {
  auto& cpu = *context->cpu;
  cpu.addCall(getKernelCallAddr(address));
}

int CompilerCodeOptimizer::getKernelCallAddr(unsigned int address) {
  int result = address, i;

  if (address >= 0x4000 && address < 0x8000) {
    i = address - 0x4000;
    if (bin_header_bin[i] == 0xC3) {  // jp
      result = bin_header_bin[i + 1] | (bin_header_bin[i + 2] << 8);
    }
  }

  return result;
}

void CompilerCodeOptimizer::addLdHLmegarom() {
  auto& cpu = *context->cpu;
  cpu.addCmd(0xFF, 0x0000);
}

CompilerCodeOptimizer::CompilerCodeOptimizer(CompilerContext* context)
    : context(context) {}

CompilerCodeOptimizer::~CompilerCodeOptimizer() = default;
