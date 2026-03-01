#ifndef PARSER_STATEMENT_STRATEGY_FACTORY_H_INCLUDED
#define PARSER_STATEMENT_STRATEGY_FACTORY_H_INCLUDED

#include <map>
#include <string>

#include "call_statement_strategy.h"
#include "cmd_statement_strategy.h"
#include "color_statement_strategy.h"
#include "data_statement_strategy.h"
#include "def_statement_strategy.h"
#include "direct_statement_strategy.h"
#include "dim_statement_strategy.h"
#include "generic_statement_strategy.h"
#include "graphics_statement_strategy.h"
#include "idata_statement_strategy.h"
#include "if_statement_strategy.h"
#include "input_statement_strategy.h"
#include "let_statement_strategy.h"
#include "get_statement_strategy.h"
#include "for_statement_strategy.h"
#include "next_statement_strategy.h"
#include "noop_statement_strategy.h"
#include "on_statement_strategy.h"
#include "print_statement_strategy.h"
#include "put_statement_strategy.h"
#include "screen_statement_strategy.h"
#include "set_statement_strategy.h"
#include "sprite_statement_strategy.h"

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
  GraphicsStatementStrategy graphicsStrategy;
  PutStatementStrategy putStrategy;
  GetStatementStrategy getStrategy;
  SetStatementStrategy setStrategy;
  ScreenStatementStrategy screenStrategy;
  SpriteStatementStrategy spriteStrategy;
  ColorStatementStrategy colorStrategy;
  DefStatementStrategy defStrategy;
  CallStatementStrategy callStrategy;
  CmdStatementStrategy cmdStrategy;
  DirectStatementStrategy baseStrategy;
  DirectStatementStrategy vdpStrategy;
  DirectStatementStrategy timeStrategy;
  OnStatementStrategy onStrategy;
  DirectStatementStrategy openStrategy;
  DirectStatementStrategy closeStrategy;
  DirectStatementStrategy maxStrategy;
  IfStatementStrategy ifStrategy;
  ForStatementStrategy forStrategy;
  NextStatementStrategy nextStrategy;
  map<string, IParserStatementStrategy*> strategies;

 public:
  ParserStatementStrategyFactory();
  IParserStatementStrategy* getStrategyByKeyword(const string& keyword);
};

#endif  // PARSER_STATEMENT_STRATEGY_FACTORY_H_INCLUDED
