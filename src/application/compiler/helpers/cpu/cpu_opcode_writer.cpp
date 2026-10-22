/***
 * @file cpu_opcode_writer.cpp
 * @brief CPU opcode writer abstraction for compiler application layer
 */

#include "cpu_opcode_writer.h"

ICpuOpcodeWriter::ICpuOpcodeWriter() {
  context = nullptr;
}

ICpuOpcodeWriter::ICpuOpcodeWriter(shared_ptr<CpuWorkspaceContext> context) {
  this->context = context;
}

void ICpuOpcodeWriter::pushLastCode() {
  if (context)
    if (!context->getCode().empty()) {
      context->getCodePipeline().push_front(&context->getCode()[context->getCodePointer()]);
    }
}

void ICpuOpcodeWriter::popLastCode() {
  if (context) context->getCodePipeline().pop_front();
}

void ICpuOpcodeWriter::addCodeByte(unsigned char byte) {
  if (context)
    if (!context->getCode().empty()) {
      if (context->getCodePointer() >= context->getCodeSizeLimit()) {
        if (!context->getError()) {
          context->getError() = true;
        }
        return;
      }
      context->getCode()[context->getCodePointer()++] = byte;
      context->getCodeSize()++;
    }
}

void ICpuOpcodeWriter::addByte(unsigned char byte) {
  pushLastCode();
  addCodeByte(byte);
}

void ICpuOpcodeWriter::addWord(unsigned int word) {
  pushLastCode();
  addCodeByte(word & 0xff);
  addCodeByte((word >> 8) & 0xff);
}

void ICpuOpcodeWriter::addWord(unsigned char byte1, unsigned char byte2) {
  pushLastCode();
  addCodeByte(byte1);
  addCodeByte(byte2);
}

void ICpuOpcodeWriter::addCmd(unsigned char byte, unsigned int word) {
  pushLastCode();
  addCodeByte(byte);
  addCodeByte(word & 0xff);
  addCodeByte((word >> 8) & 0xff);
}
