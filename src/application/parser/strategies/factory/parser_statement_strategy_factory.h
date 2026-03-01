#ifndef PARSER_STATEMENT_STRATEGY_FACTORY_H_INCLUDED
#define PARSER_STATEMENT_STRATEGY_FACTORY_H_INCLUDED

#include <map>
#include <string>

#include "call_statement_strategy.h"
#include "data_statement_strategy.h"
#include "direct_statement_strategy.h"
#include "dim_statement_strategy.h"
#include "generic_statement_strategy.h"
#include "idata_statement_strategy.h"
#include "input_statement_strategy.h"
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
  InputStatementStrategy inputStrategy;
  DataStatementStrategy dataStrategy;
  IDataStatementStrategy idataStrategy;
  CallStatementStrategy callStrategy;
  DirectStatementStrategy screenStrategy;
  DirectStatementStrategy spriteStrategy;
  DirectStatementStrategy baseStrategy;
  DirectStatementStrategy vdpStrategy;
  DirectStatementStrategy putStrategy;
  DirectStatementStrategy timeStrategy;
  DirectStatementStrategy setStrategy;
  DirectStatementStrategy getStatementStrategy;
  DirectStatementStrategy onStrategy;
  DirectStatementStrategy intervalStrategy;
  DirectStatementStrategy stopStrategy;
  DirectStatementStrategy keyStrategy;
  DirectStatementStrategy strigStrategy;
  DirectStatementStrategy colorStrategy;
  DirectStatementStrategy cmdStrategy;
  DirectStatementStrategy openStrategy;
  DirectStatementStrategy closeStrategy;
  DirectStatementStrategy maxStrategy;
  map<string, IParserStatementStrategy*> strategies;

 public:
  ParserStatementStrategyFactory();
  IParserStatementStrategy* getStrategyByKeyword(const string& keyword);
};

#endif  // PARSER_STATEMENT_STRATEGY_FACTORY_H_INCLUDED
