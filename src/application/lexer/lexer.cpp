/***
 * @file lexer.cpp
 * @brief MSX-BASIC lexer class implementation
 * @author Amaury Carvalho (2019-2025)
 * @note
 *   https://en.wikipedia.org/wiki/Lexical_analysis
 */

#include "lexer.h"

#include "build_options.h"
#include "build_options_resolver.h"
#include "lexer_line_evaluator.h"
#include "logger.h"
#include "source_document.h"
#include "source_resolver.h"

/***
 * @name Lexer class functions
 */

Lexer::Lexer(shared_ptr<SourceResolver> resolver) {
  logger = make_shared<Logger>();
  sourceResolver = resolver;
  clear();
}

Lexer::~Lexer() = default;

void Lexer::clear() {
  lines.clear();
}

bool Lexer::load(string filename) {
  auto opts = make_shared<BuildOptions>();
  build_options::setInputFilename(opts, filename);
  return load(opts);
}

bool Lexer::load(shared_ptr<BuildOptions> opts) {
  SourceDocument document;

  this->opts = opts;
  clear();

  if (opts) logger->setFile(opts->inputFilename);

  if (!sourceResolver) {
    logger->error("Source resolver is not configured");
    return false;
  }

  if (!sourceResolver->resolve(opts ? opts->inputFilename : string(),
                               document)) {
    string message = sourceResolver->lastError();
    logger->error(message.empty() ? "Cannot load input file" : message);
    return false;
  }

  return loadDocument(document);
}

bool Lexer::loadDocument(const SourceDocument& document) {
  int lineNumber = 0;

  clear();
  logger->setFile(document.logicalName);

  for (const string& lineText : splitSourceLines(document.content)) {
    lines.emplace_back(new LexerLineEvaluator());
    auto& lexerLine = lines.back();
    if (lexerLine) {
      lexerLine->lineText = lineText;
      lexerLine->lineNumber = ++lineNumber;
    } else {
      logger->warning("Cannot allocate memory for source code line " +
                      to_string(++lineNumber));
    }
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

shared_ptr<Logger> Lexer::getLogger() {
  return logger;
}

shared_ptr<SourceResolver> Lexer::getSourceResolver() {
  return sourceResolver;
}
