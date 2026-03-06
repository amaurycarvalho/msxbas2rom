#include "compiler_clear_statement_strategy.h"

#include "compiler_context.h"
#include "compiler_hooks.h"

//! @brief CLEAR statement
//! @note https://www.msx.org/wiki/CLEAR

void CompilerClearStatementStrategy::cmd_clear(CompilerContext* context) {
  auto& cpu = *context->cpu;

  /// clear variables workarea
  cpu.addXorA();                //! A = 0
  cpu.addLdHLii(def_HEAPSTR);   //! HL = heap start address
  cpu.addLdDE(def_RAM_BOTTOM);  //! DE = variables start address
  cpu.addSbcHLDE();             //! HL -= DE
  cpu.addLdCL();
  cpu.addLdBH();  //! BC = HL (variables workarea size)
  cpu.addLdLE();
  cpu.addLdHD();    //! HL = DE
  cpu.addIncDE();   //! DE = HL + 1
  cpu.addLdiHLA();  //! (HL) = A
  cpu.addLDIR();    //! (DE++) = (HL++), until BC-- = 0
}

bool CompilerClearStatementStrategy::execute(CompilerContext* context) {
  /// @remark issue #11
  cmd_clear(context);
  return context->compiled;
}
