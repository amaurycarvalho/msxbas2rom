#ifndef COMPILER_STATEMENT_STRATEGY_H_INCLUDED
#define COMPILER_STATEMENT_STRATEGY_H_INCLUDED

class CompilerContext;

class ICompilerStatementStrategy {
 public:
  virtual ~ICompilerStatementStrategy() {}
  virtual bool execute(CompilerContext* context) = 0;
};

#endif  // COMPILER_STATEMENT_STRATEGY_H_INCLUDED
