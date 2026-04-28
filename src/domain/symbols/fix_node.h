/***
 * @file fix_node.h
 * @brief FixNode class header
 * @author Amaury Carvalho (2019-2025)
 * @note
 */

#ifndef FIX_NODE_H_INCLUDED
#define FIX_NODE_H_INCLUDED

#include <memory>

class SymbolNode;
class ICpuOpcodeWriter;

using namespace std;

/***
 * @class FixNode
 * @brief Symbol address to be calculated during linking phase
 */
class FixNode {
 private:
  shared_ptr<ICpuOpcodeWriter> cpu;

 public:
  shared_ptr<SymbolNode> symbol;
  int address;
  int step;
  void aimHere();

  FixNode();
  FixNode(shared_ptr<ICpuOpcodeWriter> cpu);
  ~FixNode();
};

#endif  // FIX_NODE_H_INCLUDED
