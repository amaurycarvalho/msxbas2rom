#ifndef DIRECT_STATEMENT_STRATEGY_H_INCLUDED
#define DIRECT_STATEMENT_STRATEGY_H_INCLUDED

#include "parser_statement_strategy.h"

class DirectStatementStrategy : public IParserStatementStrategy {
 public:
  enum StatementType {
    SCREEN,
    SPRITE,
    BASE,
    VDP,
    PUT,
    TIME,
    SET,
    GET
  };

 private:
  StatementType type;

 public:
  explicit DirectStatementStrategy(StatementType type) : type(type) {}

  bool execute(Parser& parser, LexerLine* statement, Lexeme* lexeme) override;
};

#endif  // DIRECT_STATEMENT_STRATEGY_H_INCLUDED
