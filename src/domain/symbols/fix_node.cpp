#include "fix_node.h"

#include "cpu_opcode_writer.h"
#include "symbol_node.h"

void FixNode::aimHere() {
  if (cpu) {
    if (cpu->context)
      if (symbol) symbol->address = cpu->context->code_pointer;
  }
}

FixNode::FixNode() {}

FixNode::FixNode(shared_ptr<ICpuOpcodeWriter> cpu) : cpu(cpu) {}

FixNode::~FixNode() = default;
