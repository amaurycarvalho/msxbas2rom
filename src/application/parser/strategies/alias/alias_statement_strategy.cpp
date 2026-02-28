/***
 * @file alias_statement_strategy.cpp
 * @brief Parser alias statement strategy
 * @author Amaury Carvalho (2019-2026)
 * @note
 */

#include "alias_statement_strategy.h"

#include "parser.h"

AliasStatementStrategy::AliasStatementStrategy(string aliasValue,
                                               bool promoteToKeyword,
                                               bool (Parser::*handler)(
                                                   LexerLine*)) {
  this->aliasValue = aliasValue;
  this->promoteToKeyword = promoteToKeyword;
  this->handler = handler;
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

  if (!handler) return true;
  return (parser.*handler)(statement);
}
