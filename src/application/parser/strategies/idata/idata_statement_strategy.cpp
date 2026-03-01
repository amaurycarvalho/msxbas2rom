#include "idata_statement_strategy.h"

#include "lexer.h"
#include "parser.h"

bool IDataStatementStrategy::execute(Parser& parser, LexerLine* statement,
                                     Lexeme* lexeme) {
  (void)lexeme;
  return parser.evalCmdData(statement, Lexeme::subtype_integer_data);
}
