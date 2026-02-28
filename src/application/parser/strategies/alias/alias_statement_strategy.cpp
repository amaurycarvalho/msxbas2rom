/***
 * @file alias_statement_strategy.cpp
 * @brief Parser alias statement strategy
 * @author Amaury Carvalho (2019-2026)
 * @note
 */

#include "alias_statement_strategy.h"

#include "parser.h"

AliasStatementStrategy::AliasStatementStrategy(string aliasValue,
                                               ParserStatementAction action,
                                               bool promoteToKeyword) {
  this->aliasValue = aliasValue;
  this->action = action;
  this->promoteToKeyword = promoteToKeyword;
}

bool AliasStatementStrategy::handle(Parser& parser, LexerLine* statement,
                                    Lexeme* lexeme) {
  if (lexeme) {
    if (promoteToKeyword) {
      lexeme->type = Lexeme::type_keyword;
    }
    lexeme->name = aliasValue;
    lexeme->value = aliasValue;
  }

  return parser.executeStatementCommand(action, statement, lexeme);
}
