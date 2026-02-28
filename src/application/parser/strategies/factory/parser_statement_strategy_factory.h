/***
 * @file parser_statement_strategy_factory.h
 * @brief Factory for parser statement strategies
 * @author Amaury Carvalho (2019-2026)
 * @note
 */

#ifndef PARSER_STATEMENT_STRATEGY_FACTORY_H_INCLUDED
#define PARSER_STATEMENT_STRATEGY_FACTORY_H_INCLUDED

#include <string>
#include <unordered_map>

#include "alias_statement_strategy.h"
#include "command_statement_strategy.h"
#include "data_statement_strategy.h"
#include "def_statement_strategy.h"
#include "generic_statement_strategy.h"
#include "if_statement_strategy.h"
#include "input_statement_strategy.h"
#include "play_statement_strategy.h"

using namespace std;

class ParserStatementStrategyFactory {
 private:
  AliasStatementStrategy apostropheStrategy;
  AliasStatementStrategy questionMarkStrategy;
  AliasStatementStrategy underscoreStrategy;

  CommandStatementStrategy noOpStrategy;
  DefStatementStrategy defStrategy;
  DefStatementStrategy defIntStrategy;
  DefStatementStrategy defStrStrategy;
  DefStatementStrategy defSngStrategy;
  DefStatementStrategy defDblStrategy;
  GenericStatementStrategy genericStrategy;
  CommandStatementStrategy screenStrategy;
  PlayStatementStrategy playStrategy;
  CommandStatementStrategy letStrategy;
  CommandStatementStrategy dimStrategy;
  CommandStatementStrategy printStrategy;
  InputStatementStrategy inputStrategy;
  CommandStatementStrategy spriteStrategy;
  CommandStatementStrategy baseStrategy;
  CommandStatementStrategy vdpStrategy;
  CommandStatementStrategy putStrategy;
  CommandStatementStrategy timeStrategy;
  CommandStatementStrategy setStrategy;
  CommandStatementStrategy getStrategyItem;
  CommandStatementStrategy onStrategy;
  CommandStatementStrategy intervalStrategy;
  CommandStatementStrategy stopStrategy;
  CommandStatementStrategy keyStrategy;
  CommandStatementStrategy strigStrategy;
  CommandStatementStrategy colorStrategy;
  DataStatementStrategy dataStrategy;
  DataStatementStrategy idataStrategy;
  IfStatementStrategy ifStrategy;
  CommandStatementStrategy forStrategy;
  CommandStatementStrategy nextStrategy;
  CommandStatementStrategy psetStrategy;
  CommandStatementStrategy lineStrategy;
  CommandStatementStrategy circleStrategy;
  CommandStatementStrategy paintStrategy;
  CommandStatementStrategy copyStrategy;
  CommandStatementStrategy callStrategy;
  CommandStatementStrategy cmdStrategy;
  CommandStatementStrategy openStrategy;
  CommandStatementStrategy closeStrategy;
  CommandStatementStrategy maxfilesStrategy;

  unordered_map<string, IParserStatementStrategy*> strategies;

 public:
  ParserStatementStrategyFactory();
  IParserStatementStrategy* getStrategy(const string& keyword);
};

#endif  // PARSER_STATEMENT_STRATEGY_FACTORY_H_INCLUDED
