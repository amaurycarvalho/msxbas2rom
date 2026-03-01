#ifndef PARSER_STATEMENT_STRATEGY_H_INCLUDED
#define PARSER_STATEMENT_STRATEGY_H_INCLUDED

class Parser;
class LexerLine;
class Lexeme;

class IParserStatementStrategy {
 public:
  virtual ~IParserStatementStrategy() {}
  virtual bool execute(Parser& parser, LexerLine* statement, Lexeme* lexeme) = 0;
};

#endif  // PARSER_STATEMENT_STRATEGY_H_INCLUDED
