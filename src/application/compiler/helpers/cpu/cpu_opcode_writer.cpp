/***
 * @file cpu_opcode_writer.cpp
 * @brief CPU opcode writer abstraction for compiler application layer
 */

#include "cpu_opcode_writer.h"

ICpuOpcodeWriter::ICpuOpcodeWriter() {
  cpuContext = nullptr;
}

ICpuOpcodeWriter::ICpuOpcodeWriter(CpuWorkspaceContext* cpuContext) {
  this->cpuContext = cpuContext;
}

void ICpuOpcodeWriter::setWorkspaceContext(CpuWorkspaceContext* cpuContext) {
  this->cpuContext = cpuContext;
}

CpuWorkspaceContext* ICpuOpcodeWriter::getWorkspaceContext() {
  return cpuContext;
};

void ICpuOpcodeWriter::pushLastCode() {
  if (cpuContext)
    if (cpuContext->code) {
      cpuContext->code_pipeline.push_front(
          &cpuContext->code[cpuContext->code_pointer]);
    }
}

void ICpuOpcodeWriter::popLastCode() {
  if (cpuContext) cpuContext->code_pipeline.pop_front();
}

void ICpuOpcodeWriter::addCodeByte(unsigned char byte) {
  if (cpuContext)
    if (cpuContext->code) {
      cpuContext->code[cpuContext->code_pointer++] = byte;
      cpuContext->code_size++;
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
