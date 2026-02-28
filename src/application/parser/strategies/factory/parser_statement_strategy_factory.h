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

using namespace std;

class ParserStatementStrategyFactory {
 private:
  AliasStatementStrategy apostropheStrategy;
  AliasStatementStrategy questionMarkStrategy;
  AliasStatementStrategy underscoreStrategy;

  CommandStatementStrategy noOpStrategy;
  CommandStatementStrategy defStrategy;
  CommandStatementStrategy defIntStrategy;
  CommandStatementStrategy defStrStrategy;
  CommandStatementStrategy defSngStrategy;
  CommandStatementStrategy defDblStrategy;
  CommandStatementStrategy genericStrategy;
  CommandStatementStrategy screenStrategy;
  CommandStatementStrategy playStrategy;
  CommandStatementStrategy letStrategy;
  CommandStatementStrategy dimStrategy;
  CommandStatementStrategy printStrategy;
  CommandStatementStrategy inputStrategy;
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
  CommandStatementStrategy dataStrategy;
  CommandStatementStrategy idataStrategy;
  CommandStatementStrategy ifStrategy;
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
