#include "fix_node.h"

#include "symbol_node.h"

void FixNode::aimHere() {
  if (codePointer && symbol) symbol->address = *codePointer;
}

FixNode::FixNode() : codePointer(nullptr) {}

FixNode::FixNode(int* codePointer) : codePointer(codePointer) {}

FixNode::~FixNode() = default;
