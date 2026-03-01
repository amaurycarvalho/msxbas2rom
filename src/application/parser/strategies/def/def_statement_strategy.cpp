#include "def_statement_strategy.h"

#include "parser.h"

bool DefStatementStrategy::execute(Parser& parser, LexerLine* statement,
                                   Lexeme* lexeme) {
  int vartype = 0;

  if (lexeme->value == "DEFINT")
    vartype = 2;
  else if (lexeme->value == "DEFSTR")
    vartype = 3;
  else if (lexeme->value == "DEFSNG")
    vartype = 4;
  else if (lexeme->value == "DEFDBL")
    vartype = 8;

  return parser.evalCmdDef(statement, vartype);
}
