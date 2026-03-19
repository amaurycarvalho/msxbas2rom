#ifndef COMPILER_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_STATEMENT_STRATEGY_H_INCLUDED

#include <memory>

class CompilerContext;

using namespace std;

class ICompilerStatementStrategy {
 public:
  virtual ~ICompilerStatementStrategy() {}
  virtual bool execute(shared_ptr<CompilerContext> context) = 0;
};

#endif  // COMPILER_STATEMENT_STRATEGY_H_INCLUDED
