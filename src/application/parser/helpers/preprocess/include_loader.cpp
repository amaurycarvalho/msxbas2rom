/***
 * @file include_loader.cpp
 * @brief Include loader class code
 * @author Amaury Carvalho (2019-2026)
 */

#include "include_loader.h"

#include "lexer_line_evaluator.h"
#include "parser_line_evaluator.h"

IncludeLoader::IncludeLoader(ParserLineEvaluator* lineEvaluator)
    : lineEvaluator(lineEvaluator) {}

IncludeLoader::~IncludeLoader() = default;

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
  FILE* file;

  /***
   * @remark
   * The lines in the included file will point to the INCLUDE line of the
   * original source.
   */

  if ((file = fopen(filename.c_str(), "r"))) {
    bool result = true;
    string originalLineText = lexerLine->lineText;
    char newLineText[255];

    while (fgets(newLineText, sizeof(newLineText), file)) {
      lexerLine->lineText = newLineText;
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

    fclose(file);

    return result;
  }

  return false;
}
