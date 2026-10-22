/***
 * @file include_loader.cpp
 * @brief Include loader class code
 * @author Amaury Carvalho (2019-2026)
 */

#include "include_loader.h"

#include "lexer_line_evaluator.h"
#include "parser_line_evaluator.h"
#include "source_document.h"
#include "source_resolver.h"

IncludeLoader::IncludeLoader(ParserLineEvaluator* lineEvaluator)
    : lineEvaluator(lineEvaluator) {}

IncludeLoader::~IncludeLoader() = default;

void IncludeLoader::setSourceResolver(shared_ptr<SourceResolver> resolver) {
  sourceResolver = resolver;
}

bool IncludeLoader::load(shared_ptr<Lexeme> lexeme,
                         shared_ptr<LexerLineEvaluator> lexerLine) {
  if (lexeme) {
    string s = lexeme->value;

    // Remove leading quote
    if (!s.empty() && s.front() == '"') {
      s.erase(s.begin());
    }
    // Remove trailing quote
    if (!s.empty() && s.back() == '"') {
      s.pop_back();
    }
    return load(s, lexerLine);
  }

  return false;
}

bool IncludeLoader::load(const string& filename,
                         shared_ptr<LexerLineEvaluator> lexerLine) {
  SourceDocument document;

  /***
   * @remark
   * The lines in the included file will point to the INCLUDE line of the
   * original source.
   */

  if (!sourceResolver || !sourceResolver->resolve(filename, document)) {
    return false;
  }

  bool result = true;
  string originalLineText = lexerLine->lineText;

  for (const string& lineText : splitSourceLines(document.content)) {
    lexerLine->lineText = lineText;
    if (lexerLine->evaluate()) {
      if (!lineEvaluator->evaluateLine(lexerLine)) {
        result = false;
        break;
      }
    } else {
      result = false;
      break;
    }
  }

  lexerLine->lineText = originalLineText;
  return result;
}
