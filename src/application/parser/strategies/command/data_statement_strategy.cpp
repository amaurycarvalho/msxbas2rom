/***
 * @file data_statement_strategy.cpp
 * @brief Parser DATA statement strategy
 * @author Amaury Carvalho (2019-2026)
 * @note
 */

#include "data_statement_strategy.h"

#include "parser.h"

DataStatementStrategy::DataStatementStrategy(Lexeme::LexemeSubType subtype) {
  this->subtype = subtype;
}

bool DataStatementStrategy::handle(Parser& parser, LexerLine* statement,
                                   Lexeme* lexeme) {
  (void)lexeme;
  return parser.eval_cmd_data(statement, subtype);
}
