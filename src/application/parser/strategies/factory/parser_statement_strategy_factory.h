#ifndef PARSER_STATEMENT_STRATEGY_FACTORY_H_INCLUDED
#define PARSER_STATEMENT_STRATEGY_FACTORY_H_INCLUDED

#include <map>
#include <string>

#include "dim_statement_strategy.h"
#include "generic_statement_strategy.h"
#include "let_statement_strategy.h"
#include "noop_statement_strategy.h"
#include "print_statement_strategy.h"

using namespace std;

class ParserStatementStrategyFactory {
 private:
  NoopStatementStrategy noopStrategy;
  GenericStatementStrategy genericStrategy;
  LetStatementStrategy letStrategy;
  DimStatementStrategy dimStrategy;
  PrintStatementStrategy printStrategy;
  map<string, IParserStatementStrategy*> strategies;

 public:
  ParserStatementStrategyFactory();
  IParserStatementStrategy* getStrategy(const string& keyword);
};

#endif  // PARSER_STATEMENT_STRATEGY_FACTORY_H_INCLUDED
