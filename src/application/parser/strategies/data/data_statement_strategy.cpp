#include "data_statement_strategy.h"

#include "lexer.h"
#include "parser.h"

bool DataStatementStrategy::execute(Parser& parser, LexerLine* statement,
                                    Lexeme* lexeme) {
  (void)lexeme;
  return parser.evalCmdData(statement, Lexeme::subtype_string);
}
