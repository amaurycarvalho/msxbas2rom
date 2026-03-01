#ifndef PARSER_STATEMENT_STRATEGY_FACTORY_H_INCLUDED
#define PARSER_STATEMENT_STRATEGY_FACTORY_H_INCLUDED

#include <map>
#include <string>

#include "generic_statement_strategy.h"
#include "noop_statement_strategy.h"

using namespace std;

class ParserStatementStrategyFactory {
 private:
  NoopStatementStrategy noopStrategy;
  GenericStatementStrategy genericStrategy;
  map<string, IParserStatementStrategy*> strategies;

 public:
  ParserStatementStrategyFactory();
  IParserStatementStrategy* getStrategy(const string& keyword);
};

#endif  // PARSER_STATEMENT_STRATEGY_FACTORY_H_INCLUDED
