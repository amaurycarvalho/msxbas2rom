#ifndef LET_STATEMENT_STRATEGY_H_INCLUDED
#define LET_STATEMENT_STRATEGY_H_INCLUDED

#include "parser_statement_strategy.h"

class ExpressionEvaluator;
class AssignmentEvaluator;

class LetStatementStrategy : public IParserStatementStrategy {
 public:
  bool parseStatement(shared_ptr<ParserContext> context,
                      shared_ptr<LexerLineContext> statement);
  bool execute(shared_ptr<ParserContext> context,
               shared_ptr<LexerLineContext> statement,
               shared_ptr<Lexeme> lexeme) override;
};

#endif  // LET_STATEMENT_STRATEGY_H_INCLUDED
