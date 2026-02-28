/***
 * @file parser_statement_strategy.h
 * @brief Parser statement strategy interface
 * @author Amaury Carvalho (2019-2026)
 * @note
 */

#ifndef PARSER_STATEMENT_STRATEGY_H_INCLUDED
#define PARSER_STATEMENT_STRATEGY_H_INCLUDED

class Parser;
class LexerLine;
class Lexeme;

class IParserStatementStrategy {
 public:
  virtual ~IParserStatementStrategy() = default;
  virtual bool handle(Parser& parser, LexerLine* statement, Lexeme* lexeme) = 0;
};

#endif  // PARSER_STATEMENT_STRATEGY_H_INCLUDED
