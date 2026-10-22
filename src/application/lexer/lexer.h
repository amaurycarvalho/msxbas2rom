/***
 * @file lexer.h
 * @brief MSX-BASIC lexer class header
 * @author Amaury Carvalho (2019-2025)
 * @note
 *   [Lexical analysis](https://en.wikipedia.org/wiki/Lexical_analysis)
 */

#ifndef LEX_H_INCLUDED
#define LEX_H_INCLUDED

#include <memory>
#include <string>
#include <vector>

class LexerLineEvaluator;
class BuildOptions;
class Logger;
class SourceResolver;
class SourceDocument;

using namespace std;

/***
 * @class Lexer
 * @brief Lexer class specialized as a MSX-BASIC lexical analyzer
 */
class Lexer {
 private:
  shared_ptr<Logger> logger;
  shared_ptr<SourceResolver> sourceResolver;

 public:
  vector<shared_ptr<LexerLineEvaluator>> lines;
  shared_ptr<BuildOptions> opts;

  void clear();
  bool load(string filename);
  bool load(shared_ptr<BuildOptions> opts);
  bool loadDocument(const SourceDocument& document);
  bool evaluate();
  string toString();

  shared_ptr<Logger> getLogger();
  shared_ptr<SourceResolver> getSourceResolver();

  explicit Lexer(shared_ptr<SourceResolver> resolver);
  ~Lexer();
};

#endif  // LEX_H_INCLUDED
