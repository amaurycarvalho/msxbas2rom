/***
 * @file include_loader.cpp
 * @brief Include loader class code
 * @author Amaury Carvalho (2019-2026)
 */

#include "include_loader.h"

IncludeLoader::IncludeLoader(IParserProcessor& parserProcessor)
    : parserProcessor(parserProcessor) {}

bool IncludeLoader::load(Lexeme* lexeme) {
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
    return load(s);
  }

  return false;
}

bool IncludeLoader::load(const string& filename) {
  FILE* file;
  char line[255];
  LexerLine* lexerLine;

  /***
   * @remark
   * 1. instanciate a new LexerLine;
   * 2. read the file, line by line;
   * 3. evaluate it lexing tokens;
   * 4. and process it by calling evalLine(LexerLine).
   */

  if ((file = fopen(filename.c_str(), "r"))) {
    bool result = true;

    lexerLine = new LexerLine();

    while (fgets(line, sizeof(line), file)) {
      lexerLine->line = line;
      if (lexerLine->evaluate()) {
        if (!parserProcessor.processLine(lexerLine)) {
          result = false;
          break;
        }
      } else {
        result = false;
        break;
      }
    }

    delete lexerLine;
    fclose(file);

    return result;
  }

  return false;
}
