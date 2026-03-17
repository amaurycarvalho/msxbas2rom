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
  char lineText[255];
  unsigned char header[3];
  int len = 255, bytes;

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
        snprintf(lineText, sizeof(lineText),
                 "Tokenized MSX BASIC source code file detected\nSave it as a "
                 "plain text to use it "
                 "with MSXBAS2ROM:\nSAVE \"%s\",A",
                 opts->inputFilename.c_str());
        logger->error(lineText);
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
    int lineNumber = 0;
    while (fgets(lineText, len, file)) {
      lines.emplace_back(new LexerLine());
      auto& lexerLine = lines.back();
      if (lexerLine.get()) {
        lexerLine->lineText = lineText;
        lexerLine->lineNumber = ++lineNumber;
      } else {
        logger->warning("Cannot allocate memory for source code line " +
                        to_string(++lineNumber));
      }
    }

    fclose(file);
  } else {
    logger->error("File doesn't exist");
    return false;
  }

  return true;
}

bool Lexer::evaluate() {
  int lineNumber = 0;
  logger->debug("Displaying lexical analysis:");
  for (auto& lexerLine : lines) {
    logger->setLineNumber(++lineNumber);
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
  for (auto& lexerLine : lines) {
    if (lexerLine) out += lexerLine->toString();
  }
  return out;
}

Logger* Lexer::getLogger() {
  return logger.get();
}
