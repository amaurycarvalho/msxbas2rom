#include "generic_statement_strategy.h"

#include "parser.h"

bool GenericStatementStrategy::execute(Parser& parser, LexerLine* statement,
                                       Lexeme* lexeme) {
  ParserContext& ctx = parser.getContext();

  if (lexeme->value == "BLOAD") ctx.resourceCount++;
  if (lexeme->value == "PLAY") ctx.has_play = true;

  return parser.evalCmdGeneric(statement);
}
