#ifndef IF_STATEMENT_STRATEGY_H_INCLUDED
#define IF_STATEMENT_STRATEGY_H_INCLUDED

#include "parser_statement_strategy.h"

class ParserStatementStrategyFactory;
class ExpressionEvaluator;
class AssignmentEvaluator;
class ParserLineEvaluator;

class IfStatementStrategy : public IParserStatementStrategy {
 public:
  bool parseStatement(shared_ptr<ParserContext> context,
                      shared_ptr<LexerLineContext> statement, int level);
  bool execute(shared_ptr<ParserContext> context,
               shared_ptr<LexerLineContext> statement,
               shared_ptr<Lexeme> lexeme) override;

 private:
  bool evalPhrase(shared_ptr<ParserContext> context,
                  shared_ptr<LexerLineContext> phrase);
};

#endif  // IF_STATEMENT_STRATEGY_H_INCLUDED
