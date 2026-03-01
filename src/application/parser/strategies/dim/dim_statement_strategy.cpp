#include "dim_statement_strategy.h"

#include "parser.h"

bool DimStatementStrategy::execute(Parser& parser, LexerLine* statement,
                                   Lexeme* lexeme) {
  (void)lexeme;
  return parser.evalCmdDim(statement);
}
