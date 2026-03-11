/***
 * @file lexer.cpp
 * @brief MSX BASIC lexer class implementation
 * @author Amaury Carvalho (2019-2025)
 * @note
 *   https://en.wikipedia.org/wiki/Lexical_analysis
 */

#include "lexer.h"

#include <stdio.h>

#include <cstring>

#include "build_options.h"
#include "lexer_line.h"
#include "logger.h"

/***
 * @name Lexer class functions
 */

Lexer::Lexer() {
  logger.reset(new Logger());
  clear();
}

Lexer::~Lexer() = default;

void Lexer::clear() {
  lines.clear();
}

bool Lexer::load(string filename) {
  return load(new BuildOptions(filename));
}

bool Lexer::load(BuildOptions* opts) {
  FILE* file;
  char line[255];
  unsigned char header[3];
  int len = 255, bytes;
  LexerLine* lexerLine;

  this->opts = opts;

  clear();

  logger->setFile(opts->inputFilename);

  if ((file = fopen(opts->inputFilename.c_str(), "rb"))) {
    memset(header, 0, 3);
    bytes = fread(header, 1, 3, file);
    fclose(file);

    if (bytes == 0) {
      logger->error("Empty file");
      return false;
    }
    if (header[0] < 0x20 || header[0] > 126) {
      if (header[0] == 0xFF && header[2] == 0x80) {
        snprintf(line, sizeof(line),
                 "Tokenized MSX BASIC source code file detected\nSave it as a "
                 "plain text to use it "
                 "with MSXBAS2ROM:\nSAVE \"%s\",A",
                 opts->inputFilename.c_str());
        logger->error(line);
        return false;
      } else if (header[0] != 0x0D && header[0] != 0x0A &&
                 header[0] != 0x0C) {  // CR LF FF
        logger->error("This is not a MSX BASIC source code file.");
        return false;
      }
    }
  } else {
    logger->error("File doesn't exist");
    return false;
  }

  if ((file = fopen(opts->inputFilename.c_str(), "r"))) {
    while (fgets(line, len, file)) {
      lexerLine = new LexerLine();
      lexerLine->line = line;
      lines.push_back(lexerLine);
    }

    fclose(file);
  } else {
    logger->error("File doesn't exist");
    return false;
  }

  return true;
}

bool Lexer::evaluate() {
  LexerLine* lexerLine;
  logger->debug("Displaying lexical analysis:");
  for (unsigned int i = 0; i < lines.size(); i++) {
    int lineNumber = i + 1;
    logger->setLineNumber(lineNumber);
    lexerLine = lines[i];
    if (lexerLine) {
      if (!lexerLine->evaluate()) {
        logger->error("Lexical error");
        logger->info(lexerLine->toString());
        return false;
      }
      logger->debug(lexerLine->toString());
    } else {
      logger->error("Lexical error");
      logger->info("Cannot evaluate a null line");
    }
  }
  return true;
}

string Lexer::toString() {
  string out;
  LexerLine* lexerLine;
  for (unsigned int i = 0; i < lines.size(); i++) {
    lexerLine = lines[i];
    if (lexerLine) out += lexerLine->toString();
  }
  return out;
}

Logger* Lexer::getLogger() {
  return logger.get();
}
