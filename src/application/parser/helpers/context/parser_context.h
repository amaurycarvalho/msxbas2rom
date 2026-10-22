/***
 * @file parser_context.h
 * @brief ParserContext class header
 * @author Amaury Carvalho (2019-2025)
 */

#ifndef PARSER_CONTEXT_H_INCLUDED
#define PARSER_CONTEXT_H_INCLUDED

#include <memory>
#include <stack>
#include <string>
#include <vector>

class Logger;
class Lexer;
class BuildOptions;
class ParserStatementStrategyFactory;
class ExpressionEvaluator;
class AssignmentEvaluator;
class ParserLineEvaluator;

class ActionNode;
class TagNode;
class Lexeme;
class LexerLineContext;

using namespace std;

/***
 * @class ParserContext
 * @brief Mutable parser execution state shared across parser methods.
 */
class ParserContext {
 private:
  //! support objects
  shared_ptr<Lexer> lexer;
  shared_ptr<BuildOptions> opts;

  //! support helpers
  shared_ptr<ParserStatementStrategyFactory> statementStrategyFactory;
  shared_ptr<ExpressionEvaluator> exprEval;
  shared_ptr<AssignmentEvaluator> assignEval;
  shared_ptr<ParserLineEvaluator> lineEval;

  //! templates
  shared_ptr<Lexeme> lex_null, lex_index, lex_empty_string;
  shared_ptr<Lexeme> lex_rgb, lex_zero;

  //! collections
  stack<shared_ptr<ActionNode>> actionStack;
  stack<shared_ptr<Lexeme>> expressionList;
  vector<shared_ptr<TagNode>> tags;
  vector<shared_ptr<Lexeme>> symbolList;
  vector<shared_ptr<Lexeme>> datas;

  //! flags
  bool eval_expr_error, line_comment;
  bool has_traps, has_defusr, has_data, has_idata;
  bool has_play, has_input, has_font, has_mtf;
  bool has_pt3, has_akm, has_resource_restore, has_file_support;

  //! counters
  int deftbl[26];
  int lineNumber;
  int resourceCount;

  //! helper objects
  shared_ptr<Logger> logger;

  shared_ptr<TagNode> tag;
  shared_ptr<ActionNode> actionRoot;
  shared_ptr<LexerLineContext> error_line;


 public:
  shared_ptr<Lexer>& getLexer() { return lexer; }
  shared_ptr<BuildOptions>& getOpts() { return opts; }
  shared_ptr<ParserStatementStrategyFactory>& getStatementStrategyFactory() {
    return statementStrategyFactory;
  }
  shared_ptr<ExpressionEvaluator>& getExprEval() { return exprEval; }
  shared_ptr<AssignmentEvaluator>& getAssignEval() { return assignEval; }
  shared_ptr<ParserLineEvaluator>& getLineEval() { return lineEval; }

  shared_ptr<Lexeme>& getLexNull() { return lex_null; }
  shared_ptr<Lexeme>& getLexIndex() { return lex_index; }
  shared_ptr<Lexeme>& getLexEmptyString() { return lex_empty_string; }
  shared_ptr<Lexeme>& getLexRgb() { return lex_rgb; }
  shared_ptr<Lexeme>& getLexZero() { return lex_zero; }

  stack<shared_ptr<ActionNode>>& getActionStack() { return actionStack; }
  stack<shared_ptr<Lexeme>>& getExpressionList() { return expressionList; }
  vector<shared_ptr<TagNode>>& getTags() { return tags; }
  vector<shared_ptr<Lexeme>>& getSymbolList() { return symbolList; }
  vector<shared_ptr<Lexeme>>& getDatas() { return datas; }

  bool& getEvalExprError() { return eval_expr_error; }
  bool& getLineComment() { return line_comment; }
  bool& getHasTraps() { return has_traps; }
  bool& getHasDefusr() { return has_defusr; }
  bool& getHasData() { return has_data; }
  bool& getHasIData() { return has_idata; }
  bool& getHasPlay() { return has_play; }
  bool& getHasInput() { return has_input; }
  bool& getHasFont() { return has_font; }
  bool& getHasMtf() { return has_mtf; }
  bool& getHasPt3() { return has_pt3; }
  bool& getHasAkm() { return has_akm; }
  bool& getHasResourceRestore() { return has_resource_restore; }
  bool& getHasFileSupport() { return has_file_support; }

  int (&getDeftbl())[26] { return deftbl; }
  int& getLineNumber() { return lineNumber; }
  int& getResourceCount() { return resourceCount; }

  shared_ptr<Logger>& getLogger() { return logger; }
  shared_ptr<TagNode>& getTag() { return tag; }
  shared_ptr<ActionNode>& getActionRoot() { return actionRoot; }
  shared_ptr<LexerLineContext>& getErrorLine() { return error_line; }

  //! helper methods
  void setHelpers(
      shared_ptr<ParserContext> context,
      shared_ptr<ParserStatementStrategyFactory> statementStrategyFactory);
  void reset();

  int gfxOperatorCode(shared_ptr<Lexeme> lexeme);
  shared_ptr<ActionNode> pushActionFromLexeme(shared_ptr<Lexeme> lexeme);
  void pushStackFromLexeme(shared_ptr<Lexeme> lexeme);
  void pushActionRoot(shared_ptr<ActionNode> action);
  void popActionRoot();
  shared_ptr<Lexeme> coalesceSymbols(shared_ptr<Lexeme> lexeme);

  ParserContext();
  ~ParserContext();
};

#endif  // PARSER_CONTEXT_H_INCLUDED
