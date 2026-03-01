/***
 * @file parser.cpp
 * @brief Parser class code specialized as a MSX BASIC syntax tree builder
 * @author Amaury Carvalho (2019-2025)
 * @note
 *   https://en.wikipedia.org/wiki/Parsing
 *   https://en.wikipedia.org/wiki/Recursive_descent_parser
 *   https://www.strchr.com/expression_evaluator
 *   https://en.wikipedia.org/wiki/Shunting-yard_algorithm
 */

#include "parser.h"
#include "call_statement_strategy.h"
#include "cmd_statement_strategy.h"
#include "file_statement_strategy.h"
#include "graphics_statement_strategy.h"
#include "get_statement_strategy.h"
#include "on_statement_strategy.h"
#include "put_statement_strategy.h"
#include "screen_statement_strategy.h"
#include "set_statement_strategy.h"
#include "sprite_statement_strategy.h"

/***
 * @name Parser class code
 */

Parser::Parser()
    : tag(ctx.tag),
      actionRoot(ctx.actionRoot),
      error_line(ctx.error_line),
      lex_null(ctx.lex_null),
      lex_index(ctx.lex_index),
      lex_empty_string(ctx.lex_empty_string),
      actionStack(ctx.actionStack),
      expressionList(ctx.expressionList),
      deftbl(ctx.deftbl),
      eval_expr_error(ctx.eval_expr_error),
      line_comment(ctx.line_comment),
      error_message(ctx.error_message),
      lineNo(ctx.lineNo),
      tags(ctx.tags),
      symbolList(ctx.symbolList),
      datas(ctx.datas),
      has_traps(ctx.has_traps),
      has_defusr(ctx.has_defusr),
      has_data(ctx.has_data),
      has_idata(ctx.has_idata),
      has_play(ctx.has_play),
      has_input(ctx.has_input),
      has_font(ctx.has_font),
      has_mtf(ctx.has_mtf),
      has_pt3(ctx.has_pt3),
      has_akm(ctx.has_akm),
      has_resource_restore(ctx.has_resource_restore),
      resourceCount(ctx.resourceCount),
      lexer(0),
      opts(0) {}

Parser::~Parser() {}

ParserContext& Parser::getContext() { return ctx; }

const ParserContext& Parser::getContext() const { return ctx; }

Lexeme* Parser::coalesceLexeme(Lexeme* lexeme) { return coalesceSymbols(lexeme); }

bool Parser::evalExpressionTokens(LexerLine* parm) {
  return eval_expression(parm);
}

bool Parser::evalPhraseTokens(LexerLine* phrase) { return eval_phrase(phrase); }

bool Parser::evalAssignmentTokens(LexerLine* assignment) {
  return eval_assignment(assignment);
}

int Parser::gfxOperatorFromLexeme(Lexeme* lexeme) {
  return gfxOperatorCode(lexeme);
}

void Parser::pushActionNodeRoot(ActionNode* action) { pushActionRoot(action); }

ActionNode* Parser::pushActionFromLexemeNode(Lexeme* lexeme) {
  return pushActionFromLexeme(lexeme);
}

void Parser::popActionNodeRoot() { popActionRoot(); }

bool Parser::evalCmdLet(LexerLine* statement) { return eval_cmd_let(statement); }

bool Parser::evalCmdDim(LexerLine* statement) { return eval_cmd_dim(statement); }

bool Parser::evalCmdPrint(LexerLine* statement) {
  return eval_cmd_print(statement);
}

bool Parser::evalCmdInput(LexerLine* statement) {
  return eval_cmd_input(statement);
}

bool Parser::evalCmdData(LexerLine* statement, Lexeme::LexemeSubType subtype) {
  return eval_cmd_data(statement, subtype);
}

bool Parser::evalCmdScreen(LexerLine* statement) {
  return eval_cmd_screen(statement);
}

bool Parser::evalCmdSprite(LexerLine* statement) {
  return eval_cmd_sprite(statement);
}

bool Parser::evalCmdBase(LexerLine* statement) { return eval_cmd_base(statement); }

bool Parser::evalCmdVdp(LexerLine* statement) { return eval_cmd_vdp(statement); }

bool Parser::evalCmdPut(LexerLine* statement) { return eval_cmd_put(statement); }

bool Parser::evalCmdTime(LexerLine* statement) {
  return eval_cmd_time(statement);
}

bool Parser::evalCmdSet(LexerLine* statement) { return eval_cmd_set(statement); }

bool Parser::evalCmdGet(LexerLine* statement) { return eval_cmd_get(statement); }

bool Parser::evalCmdOn(LexerLine* statement) { return eval_cmd_on(statement); }

bool Parser::evalCmdInterval(LexerLine* statement) {
  return eval_cmd_interval(statement);
}

bool Parser::evalCmdStop(LexerLine* statement) { return eval_cmd_stop(statement); }

bool Parser::evalCmdKey(LexerLine* statement) { return eval_cmd_key(statement); }

bool Parser::evalCmdStrig(LexerLine* statement) {
  return eval_cmd_strig(statement);
}

bool Parser::evalCmdColor(LexerLine* statement) {
  return eval_cmd_color(statement);
}

bool Parser::evalCmdCall(LexerLine* statement) {
  return eval_cmd_call(statement);
}

bool Parser::evalCmdCmd(LexerLine* statement) { return eval_cmd_cmd(statement); }

bool Parser::evalCmdOpen(LexerLine* statement) {
  return eval_cmd_open(statement);
}

bool Parser::evalCmdClose(LexerLine* statement) {
  return eval_cmd_close(statement);
}

bool Parser::evalCmdMaxfiles(LexerLine* statement) {
  return eval_cmd_maxfiles(statement);
}

bool Parser::evalCmdDef(LexerLine* statement, int vartype) {
  return eval_cmd_def(statement, vartype);
}

bool Parser::evalCmdIf(LexerLine* statement) {
  return eval_cmd_if(statement, 0);
}

bool Parser::evalCmdFor(LexerLine* statement) { return eval_cmd_for(statement); }

bool Parser::evalCmdNext(LexerLine* statement) {
  return eval_cmd_next(statement);
}

bool Parser::evalCmdPset(LexerLine* statement) {
  return eval_cmd_pset(statement);
}

bool Parser::evalCmdLine(LexerLine* statement) {
  return eval_cmd_line(statement);
}

bool Parser::evalCmdCircle(LexerLine* statement) {
  return eval_cmd_circle(statement);
}

bool Parser::evalCmdPaint(LexerLine* statement) {
  return eval_cmd_paint(statement);
}

bool Parser::evalCmdCopy(LexerLine* statement) {
  return eval_cmd_copy(statement);
}

bool Parser::evalCmdPutSprite(LexerLine* statement) {
  return eval_cmd_put_sprite(statement);
}

bool Parser::evalCmdPutTile(LexerLine* statement) {
  return eval_cmd_put_tile(statement);
}

bool Parser::evalCmdSetAdjust(LexerLine* statement) {
  return eval_cmd_set_adjust(statement);
}

bool Parser::evalCmdSetTile(LexerLine* statement) {
  return eval_cmd_set_tile(statement);
}

bool Parser::evalCmdSetSprite(LexerLine* statement) {
  return eval_cmd_set_sprite(statement);
}

bool Parser::evalCmdSetSpriteColpattra(LexerLine* statement) {
  return eval_cmd_set_sprite_colpattra(statement);
}

bool Parser::evalCmdGetTile(LexerLine* statement) {
  return eval_cmd_get_tile(statement);
}

bool Parser::evalCmdGetSprite(LexerLine* statement) {
  return eval_cmd_get_sprite(statement);
}

bool Parser::evalCmdScreenCopy(LexerLine* statement) {
  return eval_cmd_screen_copy(statement);
}

bool Parser::evalCmdScreenPaste(LexerLine* statement) {
  return eval_cmd_screen_paste(statement);
}

bool Parser::evalCmdScreenScroll(LexerLine* statement) {
  return eval_cmd_screen_scroll(statement);
}

bool Parser::evalCmdScreenLoad(LexerLine* statement) {
  return eval_cmd_screen_load(statement);
}

bool Parser::evalCmdScreenOn(LexerLine* statement) {
  return eval_cmd_screen_on(statement);
}

bool Parser::evalCmdScreenOff(LexerLine* statement) {
  return eval_cmd_screen_off(statement);
}

bool Parser::evalCmdSpriteLoad(LexerLine* statement) {
  return eval_cmd_sprite_load(statement);
}

bool Parser::evaluate(Lexer* lexer) {
  int i, t = lexer->lines.size();
  LexerLine* lexerLine;

  this->lexer = lexer;
  this->opts = lexer->opts;
  ctx.reset();

  for (i = 0, ctx.lineNo = 1; i < t; i++, ctx.lineNo++) {
    lexerLine = lexer->lines[i];
    // if(opts->debug) {
    // printf("%s\n",lexerLine->line.c_str());
    // }
    if (lexerLine->getLexemeCount() > 0) {
      ctx.line_comment = false;
      if (!eval_line(lexerLine)) return false;
    }
  }

  return true;
}

bool Parser::eval_line(LexerLine* lexerLine) {
  Lexeme* lexeme = lexerLine->getFirstLexeme();
  ActionNode* action;
  LexerLine phrase;
  int if_count = 0;

  ctx.error_line = lexerLine;
  ctx.actionRoot = 0;

  if (lexeme) {
    lexeme = coalesceSymbols(lexeme);

    if (lexeme->isLiteralNumeric()) {
      ctx.tag = new TagNode();  // register line number tag
      ctx.tag->name = lexeme->value;
      ctx.tag->value = ctx.tag->name;
      ctx.tags.push_back(ctx.tag);

      while ((lexeme = lexerLine->getNextLexeme())) {
        lexeme = coalesceSymbols(lexeme);

        if (lexeme->isKeyword("IF")) {
          if_count++;
        }

        if (lexeme->isSeparator(":") && if_count == 0) {
          ctx.actionRoot = 0;
          if (phrase.getLexemeCount())
            if (!eval_phrase(&phrase)) return false;
          phrase.clearLexemes();
          continue;

        } else if (lexeme->isOperator("'")) {
          break;

        } else {
          phrase.addLexeme(lexeme);
        }
      }

      if (phrase.getLexemeCount()) {
        ctx.actionRoot = 0;
        if (!eval_phrase(&phrase)) return false;
      }

    } else if (lexeme->type == Lexeme::type_keyword) {
      if (lexeme->value == "FILE" || lexeme->value == "TEXT") {
        ctx.resourceCount++;

        ctx.tag = new TagNode();  // register line number tag
        ctx.tag->name = "DIRECTIVE";
        ctx.tag->value = ctx.tag->name;
        ctx.tags.push_back(ctx.tag);

        action = new ActionNode(lexeme);
        pushActionRoot(action);

        lexeme = lexerLine->getNextLexeme();
        if (lexeme) pushActionFromLexeme(lexeme);

        popActionRoot();

      } else if (lexeme->value == "INCLUDE") {
        if ((lexeme = lexerLine->getNextLexeme())) {
          if (lexeme->type == Lexeme::type_literal &&
              lexeme->subtype == Lexeme::subtype_string) {
            if (!loadInclude(lexeme)) {
              ctx.error_message =
                  "INCLUDE file not found or with content syntax error";
              return false;
            }

          } else {
            ctx.error_message = "Invalid parameter in INCLUDE keyword";
            return false;
          }
        }
      }
    }
  }

  return true;
}

bool Parser::eval_phrase(LexerLine* phrase) {
  Lexeme* lexeme = phrase->getFirstLexeme();

  if (lexeme) {
    lexeme = coalesceSymbols(lexeme);

    if (lexeme->type == Lexeme::type_identifier ||
        (lexeme->type == Lexeme::type_keyword &&
         lexeme->subtype == Lexeme::subtype_function &&
         lexeme->value != "STRIG")) {
      phrase->setLexemeBOF();
      return eval_assignment(phrase);
    } else if (lexeme->type == Lexeme::type_keyword) {
      return eval_statement(phrase);
    } else if (lexeme->isOperator("'")) {
      return eval_statement(phrase);
    } else if (lexeme->isOperator("_")) {
      return eval_statement(phrase);
    }
  }

  ctx.error_message = "Invalid keyword/identifier";
  return false;
}

bool Parser::eval_statement(LexerLine* statement) {
  Lexeme* lexeme;
  ActionNode* action;
  IParserStatementStrategy* strategy;
  ActionNode* actionSaved = ctx.actionRoot;
  unsigned int actionCount = ctx.actionStack.size();
  bool result = true;

  lexeme = statement->getFirstLexeme();

  if (lexeme) {
    lexeme = coalesceSymbols(lexeme);

    action = new ActionNode(lexeme);
    pushActionRoot(action);

    strategy = statementStrategyFactory.getStrategyByKeyword(lexeme->value);
    if (strategy) {
      result = strategy->execute(*this, statement, lexeme);
      if (lexeme->value == "IF") return result;
    } else {
      result = false;
    }

    popActionRoot();
  }

  while (ctx.actionStack.size() > actionCount) ctx.actionStack.pop();

  ctx.actionRoot = actionSaved;

  return result;
}

Lexeme* Parser::coalesceSymbols(Lexeme* lexeme) {
  Lexeme* result = lexeme;
  unsigned int i, t = symbolList.size();
  bool ok = false;
  int c;
  char* s;

  if (lexeme) {
    if (lexeme->type == Lexeme::type_identifier) {
      for (i = 0; i < t; i++) {
        result = symbolList[i];
        if (result->type == lexeme->type &&
            result->subtype == lexeme->subtype &&
            result->name == lexeme->name && result->value == lexeme->value) {
          ok = true;
          break;
        }
      }

      if (!ok) {
        symbolList.push_back(lexeme);
        result = lexeme;
      }

      t = lexeme->value.size();

      if (lexeme->type == Lexeme::type_identifier && t) {
        s = (char*)lexeme->value.c_str();
        c = s[t - 1];
        if (c != '$' && c != '#' && c != '!' && c != '%') {
          c = s[0] - 'A';
          if (c >= 0 && c < 26) {
            switch (deftbl[c]) {
              case 2:
                lexeme->subtype = Lexeme::subtype_numeric;
                break;
              case 3:
                lexeme->subtype = Lexeme::subtype_string;
                break;
              case 4:
                lexeme->subtype = Lexeme::subtype_single_decimal;
                break;
              case 8:
                lexeme->subtype = Lexeme::subtype_double_decimal;
                break;
            }
          }
        }
      }
    }
  }

  return result;
}

bool Parser::eval_assignment(LexerLine* assignment) {
  Lexeme *lexeme = assignment->getNextLexeme(), *next_lexeme;
  LexerLine parm;
  ActionNode* action;
  Lexeme* lexLet;
  bool result, add_let_action = true;

  if (lexeme) {
    lexeme = coalesceSymbols(lexeme);

    if (lexeme->type == Lexeme::type_identifier ||
        (lexeme->type == Lexeme::type_keyword &&
         lexeme->subtype == Lexeme::subtype_function)) {
      if (ctx.actionRoot) {
        lexLet = ctx.actionRoot->lexeme;
        if (lexLet->isKeyword("LET")) {
          add_let_action = false;
        }
      }

      if (add_let_action) {
        action = new ActionNode("LET");
        /// @note "lexLet" value really needs to be updated?
        /// NOLINTNEXTLINE(clang-analyzer-deadcode.DeadStores)
        // lexLet = action->lexeme;
        pushActionRoot(action);
      }

      parm.clearLexemes();
      parm.addLexeme(lexeme);
      while ((next_lexeme = assignment->getNextLexeme())) {
        next_lexeme = coalesceSymbols(next_lexeme);
        if (next_lexeme->isOperator("=")) break;

        parm.addLexeme(next_lexeme);
      }
      parm.setLexemeBOF();
      if (!eval_expression(&parm)) {
        return false;
      }

      if (next_lexeme) {
        if (next_lexeme->isOperator("=")) {
          parm.clearLexemes();
          while ((next_lexeme = assignment->getNextLexeme())) {
            next_lexeme = coalesceSymbols(next_lexeme);
            parm.addLexeme(next_lexeme);
          }
          parm.setLexemeBOF();

          result = eval_expression(&parm);

          popActionRoot();

          return result;
        }
      }
    }
  }

  ctx.error_message = "Invalid LET statement";
  return false;
}

bool Parser::eval_expression(LexerLine* parm) {
  ActionNode* actionSaved = ctx.actionRoot;
  unsigned int actionCount = ctx.actionStack.size();

  while (!ctx.expressionList.empty()) ctx.expressionList.pop();

  if (!eval_expression_push(parm)) return false;

  while (!ctx.expressionList.empty()) {
    eval_expression_pop(1);
  }

  while (ctx.actionStack.size() > actionCount) ctx.actionStack.pop();

  ctx.actionRoot = actionSaved;

  return true;
}

void Parser::eval_expression_pop(int n) {
  Lexeme* lexeme;
  int k;

  while (n && !ctx.expressionList.empty()) {
    lexeme = ctx.expressionList.top();
    ctx.expressionList.pop();

    pushActionFromLexeme(lexeme);

    k = getOperatorParmCount(lexeme);
    if (k) {
      eval_expression_pop(k);
      popActionRoot();
    }

    n--;
  }
}

bool Parser::eval_expression_push(LexerLine* parm) {
  stack<Lexeme*> operatorStack;
  Lexeme *lexeme, *next_lexeme, *check_lexeme;
  LexerLine functionLexemes;
  int thisPreced, stackPreced;
  int outputCount = 0, sepcount = 0, parmcount = 0;
  bool ok, unary = false, lastWasFunction = false, lastWasIdentifier = false;

  lexeme = parm->getNextLexeme();
  if (lexeme) {
    lexeme = coalesceSymbols(lexeme);
    lastWasIdentifier = (lexeme->type == Lexeme::type_identifier);
  }

  while (lexeme) {
    lexeme = coalesceSymbols(lexeme);

    if (lexeme->type == Lexeme::type_keyword &&
        lexeme->subtype == Lexeme::subtype_function && lexeme->value == "USR") {
      check_lexeme = parm->getNextLexeme();
      if (check_lexeme) {
        if (check_lexeme->isLiteralNumeric()) {
          lexeme->value += check_lexeme->value;
          lexeme->name = lexeme->value;
        } else
          parm->getPreviousLexeme();
      } else
        parm->getPreviousLexeme();
    }

    // if operators, push to operators stack

    if (lexeme->type == Lexeme::type_operator ||
        lexeme->type == Lexeme::type_separator) {
      // test when first operand in expression or unary operator

      if (outputCount == 0 || unary) {
        if (outputCount == 0 && lexeme->value == ")") {
          ctx.eval_expr_error = true;
          ctx.error_message = "Mismatched parentheses error";
          return false;
        } else if (lexeme->value == "=") {
          next_lexeme = operatorStack.top();
          if (next_lexeme->value == "<") {
            next_lexeme->value = "<=";
            lexeme = parm->getNextLexeme();
            lexeme = coalesceSymbols(lexeme);
            continue;
          } else if (next_lexeme->value == ">") {
            next_lexeme->value = ">=";
            lexeme = parm->getNextLexeme();
            lexeme = coalesceSymbols(lexeme);
            continue;
          } else {
            ctx.eval_expr_error = true;
            ctx.error_message = "Invalid = symbol in expression";
            return false;
          }
        } else if (lexeme->value == ">") {
          next_lexeme = operatorStack.top();
          if (next_lexeme->value == "<") {
            next_lexeme->value = "<>";
            lexeme = parm->getNextLexeme();
            lexeme = coalesceSymbols(lexeme);
            continue;
          } else {
            ctx.eval_expr_error = true;
            ctx.error_message = "Invalid > symbol in expression";
            return false;
          }
        } else if (lexeme->value != "+" && lexeme->value != "-" &&
                   lexeme->value != "(" && lexeme->value != ")" &&
                   lexeme->value != "NOT") {
          ctx.eval_expr_error = true;
          ctx.error_message = "Invalid expression unary symbol";
          return false;
        } else {
          if (lexeme->value == "+" ||
              lexeme->value == "-") {  // unary plus or minus
            lexeme->isUnary = true;
            operatorStack.push(lexeme);
            lexeme = parm->getNextLexeme();
            lexeme = coalesceSymbols(lexeme);
            unary = false;
            continue;
          }
        }
      }

      if (lexeme->value == "(") {
        if (lastWasFunction || lastWasIdentifier) {
          if (ctx.expressionList.empty()) {
            ctx.eval_expr_error = true;
            ctx.error_message =
                "Invalid FUNCTION or ARRAY declaration in expression";
            return false;
          }

          next_lexeme = ctx.expressionList.top();
          ctx.expressionList.pop();

          // parse function/array parameters
          functionLexemes.clearLexemes();
          ok = false;
          sepcount = 0;
          parmcount = 0;
          while ((lexeme = parm->getNextLexeme())) {
            lexeme = coalesceSymbols(lexeme);
            if (lexeme->value == "(") {
              sepcount++;
            } else if (lexeme->value == ")") {
              if (sepcount)
                sepcount--;
              else {
                ok = true;
                functionLexemes.setLexemeBOF();
                if (!eval_expression_push(&functionLexemes)) return false;
                functionLexemes.clearLexemes();
                parmcount++;
                break;
              }
            } else if (lexeme->value == "," && sepcount == 0) {
              functionLexemes.setLexemeBOF();
              if (!eval_expression_push(&functionLexemes)) return false;
              functionLexemes.clearLexemes();
              parmcount++;
              continue;
            }
            functionLexemes.addLexeme(lexeme);
          }
          if (!ok) {
            ctx.eval_expr_error = true;
            ctx.error_message = "Mismatched parentheses error in function or array";
            return false;
          }

          if (!parmcount) {
            ctx.eval_expr_error = true;
            ctx.error_message =
                "Invalid FUNCTION or ARRAY declaration in expression (missing "
                "parameters?)";
            return false;
          }

          next_lexeme->parm_count = parmcount;

          pushStackFromLexeme(next_lexeme);

          lastWasFunction = false;
          lastWasIdentifier = false;
          unary = false;

        } else {
          operatorStack.push(lexeme);
          unary = true;
        }

        lexeme = parm->getNextLexeme();
        lexeme = coalesceSymbols(lexeme);
        continue;
      }

      lastWasIdentifier = false;
      lastWasFunction = false;

      // test this operator precedence against operators in stack

      thisPreced = getOperatorPrecedence(lexeme);

      while (!operatorStack.empty()) {
        next_lexeme = operatorStack.top();
        stackPreced = getOperatorPrecedence(next_lexeme);
        if ((thisPreced <= stackPreced ||
             next_lexeme->type == Lexeme::type_keyword) &&
            next_lexeme->value != "(") {
          pushStackFromLexeme(next_lexeme);
          operatorStack.pop();
        } else
          break;
      }

      unary = true;

      if (lexeme->value == ")") {
        unary = false;
        ok = false;
        while (!operatorStack.empty()) {
          next_lexeme = operatorStack.top();
          if (next_lexeme->value == "(") {
            operatorStack.pop();
            ok = true;
            break;
          } else {
            pushStackFromLexeme(next_lexeme);
            operatorStack.pop();
          }
        }
        if (!ok) {
          ctx.eval_expr_error = true;
          ctx.error_message = "Mismatched parentheses error";
          return false;  // parentheses is missing
        }
      } else
        operatorStack.push(lexeme);

    } else {
      // else, push operand to output stack

      lastWasIdentifier = (lexeme->type == Lexeme::type_identifier);
      lastWasFunction = (lexeme->type == Lexeme::type_keyword);

      pushStackFromLexeme(lexeme);

      outputCount++;
      unary = false;
    }

    lexeme = parm->getNextLexeme();
    lexeme = coalesceSymbols(lexeme);
  }

  // if operators stack is not empty, copy to output stack
  while (!operatorStack.empty()) {
    next_lexeme = operatorStack.top();
    pushStackFromLexeme(next_lexeme);
    operatorStack.pop();
  }

  return true;
}

int Parser::getOperatorPrecedence(Lexeme* lexeme) {
  int result;

  if (lexeme->value == "IMP")
    result = 1;
  else if (lexeme->value == "EQV")
    result = 2;
  else if (lexeme->value == "XOR")
    result = 3;
  else if (lexeme->value == "OR")
    result = 4;
  else if (lexeme->value == "AND")
    result = 5;
  else if (lexeme->value == "NOT")
    result = 6;
  else if (lexeme->value == ">" || lexeme->value == "<" ||
           lexeme->value == "=" || lexeme->value == ">=" ||
           lexeme->value == "<=" || lexeme->value == "<>")
    result = 7;
  else if (lexeme->value == "+" || lexeme->value == "-")
    result = 8;
  else if (lexeme->value == "*" || lexeme->value == "/" ||
           lexeme->value == "\\" || lexeme->value == "MOD" ||
           lexeme->value == "SHR" || lexeme->value == "SHL")
    result = 9;
  else if (lexeme->value == "^")
    result = 10;
  else
    result = 0;

  return result;
}

int Parser::getOperatorParmCount(Lexeme* lexeme) {
  int result = 0;

  if (lexeme) {
    if (lexeme->type == Lexeme::type_operator) {
      if (lexeme->value == "NOT" || lexeme->isUnary)
        result = 1;
      else if (lexeme->value == "AND" || lexeme->value == "OR" ||
               lexeme->value == "XOR" || lexeme->value == "EQV" ||
               lexeme->value == "IMP" || lexeme->value == ">" ||
               lexeme->value == "<" || lexeme->value == "=" ||
               lexeme->value == ">=" || lexeme->value == "<=" ||
               lexeme->value == "<>" || lexeme->value == "+" ||
               lexeme->value == "-" || lexeme->value == "*" ||
               lexeme->value == "/" || lexeme->value == "\\" ||
               lexeme->value == "^" || lexeme->value == "MOD" ||
               lexeme->value == "SHR" || lexeme->value == "SHL")
        result = 2;
    } else if (lexeme->type == Lexeme::type_identifier) {
      result = lexeme->parm_count;
    } else if (lexeme->type == Lexeme::type_keyword) {
      if (lexeme->value == "NOT")
        result = 1;
      else if (lexeme->value == "AND" || lexeme->value == "OR" ||
               lexeme->value == "XOR" || lexeme->value == "EQV" ||
               lexeme->value == "IMP" || lexeme->value == "MOD" ||
               lexeme->value == "SHR" || lexeme->value == "SHL")
        result = 2;
      else
        result = lexeme->parm_count;
    }
  }

  return result;
}

void Parser::pushStackFromLexeme(Lexeme* lexeme) {
  ctx.expressionList.push(lexeme);
}

ActionNode* Parser::pushActionFromLexeme(Lexeme* lexeme) {
  ActionNode* actionExpr = 0;

  actionExpr = new ActionNode(lexeme);

  if (lexeme->type == Lexeme::type_operator ||
      lexeme->type == Lexeme::type_separator ||
      (lexeme->type == Lexeme::type_keyword && lexeme->value != "TIME" &&
       lexeme->value != "INKEY" && lexeme->value != "INKEY$" &&
       lexeme->value != "MAXFILES" && lexeme->value != "ERL" &&
       lexeme->value != "ERR") ||
      (lexeme->type == Lexeme::type_identifier && lexeme->parm_count > 0)) {
    pushActionRoot(actionExpr);

  } else
    ctx.actionRoot->actions.push_back(actionExpr);

  return actionExpr;
}

//-----------------------------------------------------------------------------------------------------------------

bool Parser::eval_cmd_generic(LexerLine* statement) {
  GenericStatementStrategy strategy;
  return strategy.parseStatement(*this, statement);
}

bool Parser::eval_cmd_data(LexerLine* statement,
                           Lexeme::LexemeSubType subtype) {
  Lexeme *next_lexeme, *lexeme;
  string stext = "", sname;
  int i, itext;
  char* s;
  bool lastWasSeparator = true;

  if (subtype == Lexeme::subtype_integer_data) {
    has_idata = true;
    sname = "_IDATA_";
  } else {
    has_data = true;
    sname = "_DATA_";
  }

  while ((next_lexeme = statement->getNextLexeme())) {
    next_lexeme = coalesceSymbols(next_lexeme);

    if (next_lexeme->type == Lexeme::type_separator &&
        (next_lexeme->value == "," || next_lexeme->value == ";")) {
      if (lastWasSeparator) {
        i = datas.size() + 1;
        lexeme = new Lexeme(Lexeme::type_literal, Lexeme::subtype_string,
                            sname + to_string(i), "");
        if (lexeme) {
          lexeme->tag = tag->name;
          pushActionFromLexeme(lexeme);
          datas.push_back(lexeme);
        }

      } else if (stext.size()) {
        i = datas.size() + 1;
        next_lexeme = new Lexeme(Lexeme::type_literal, subtype,
                                 sname + to_string(i), stext);
        next_lexeme->tag = tag->name;

        s = (char*)stext.c_str();
        if (s[0] == '&') {
          try {
            if (s[1] == 'h' || s[1] == 'H')
              itext = stoi(stext.substr(2), 0, 16);
            else if (s[1] == 'o' || s[1] == 'O')
              itext = stoi(stext.substr(2), 0, 8);
            else if (s[1] == 'b' || s[1] == 'B')
              itext = stoi(stext.substr(2), 0, 2);
            else
              itext = 0;
          } catch (exception& e) {
            printf("Warning: error while converting numeric constant %s\n",
                   stext.c_str());
            itext = 0;
          }
          next_lexeme->value = to_string(itext);
        }

        pushActionFromLexeme(next_lexeme);
        datas.push_back(next_lexeme);

        stext = "";

      } else {
        error_message = "Invalid DATA parameter type";
        return false;
      }

      lastWasSeparator = true;
      continue;

    } else if (next_lexeme->isOperator("'")) {
      break;

    } else {
      stext += next_lexeme->value;
      lastWasSeparator = false;
    }
  }

  if (lastWasSeparator) {
    i = datas.size() + 1;
    lexeme = new Lexeme(Lexeme::type_literal, Lexeme::subtype_string,
                        sname + to_string(i), "");
    if (lexeme) {
      lexeme->tag = tag->name;
      pushActionFromLexeme(lexeme);
      datas.push_back(lexeme);
    }
  }

  if (stext.size()) {
    i = datas.size() + 1;
    next_lexeme = new Lexeme(Lexeme::type_literal, subtype,
                             "_DATA_" + to_string(i), stext);
    next_lexeme->tag = tag->name;

    s = (char*)stext.c_str();
    if (s[0] == '&') {
      try {
        if (s[1] == 'h' || s[1] == 'H')
          itext = stoi(stext.substr(2), 0, 16);
        else if (s[1] == 'o' || s[1] == 'O')
          itext = stoi(stext.substr(2), 0, 8);
        else if (s[1] == 'b' || s[1] == 'B')
          itext = stoi(stext.substr(2), 0, 2);
        else
          itext = 0;
      } catch (exception& e) {
        printf("Warning: error while converting numeric constant %s\n",
               stext.c_str());
        itext = 0;
      }
      next_lexeme->value = to_string(itext);
    }

    pushActionFromLexeme(next_lexeme);
    datas.push_back(next_lexeme);
  }

  return true;
}

bool Parser::eval_cmd_dim(LexerLine* statement) {
  DimStatementStrategy strategy;
  return strategy.parseStatement(*this, statement);
}

bool Parser::eval_cmd_let(LexerLine* statement) {
  LetStatementStrategy strategy;
  return strategy.parseStatement(*this, statement);
}

bool Parser::eval_cmd_print(LexerLine* statement) {
  PrintStatementStrategy strategy;
  return strategy.parseStatement(*this, statement);
}

bool Parser::eval_cmd_input(LexerLine* statement) {
  InputStatementStrategy strategy;
  return strategy.parseStatement(*this, statement);
}

bool Parser::eval_cmd_line_input(LexerLine* statement) {
  PrintStatementStrategy strategy;
  return strategy.parseStatement(*this, statement);
}

bool Parser::eval_cmd_color(LexerLine* statement) {
  ColorStatementStrategy strategy;
  return strategy.parseStatement(*this, statement);
}

bool Parser::eval_cmd_color_rgb(LexerLine* statement) {
  Lexeme *next_lexeme, *lex_rgb;
  LexerLine parm;
  int state = 0, sepCount = 0;

  while ((next_lexeme = statement->getNextLexeme())) {
    switch (state) {
      case 0: {
        if (next_lexeme->isSeparator("(")) {
          lex_rgb =
              new Lexeme(Lexeme::type_keyword, Lexeme::subtype_any, "RGB");
          pushActionFromLexeme(lex_rgb);

          state++;
          sepCount++;
          continue;

        } else if (next_lexeme->isKeyword("NEW") ||
                   next_lexeme->isKeyword("RESTORE")) {
          pushActionFromLexeme(next_lexeme);
          return true;

        } else
          return false;

      } break;

      case 1: {
        if (next_lexeme->isSeparator("(")) {
          sepCount++;
        } else if (next_lexeme->isSeparator(")")) {
          sepCount--;
        }

        if (next_lexeme->isSeparator(")") && sepCount == 0) {
          parm.setLexemeBOF();
          if (!eval_expression(&parm)) {
            return false;
          }
          parm.clearLexemes();

          popActionRoot();

          return true;

        } else if (next_lexeme->isSeparator(",")) {
          parm.setLexemeBOF();
          if (!eval_expression(&parm)) {
            return false;
          }
          parm.clearLexemes();

        } else {
          parm.addLexeme(next_lexeme);
        }

      } break;
    }
  }

  return false;
}

bool Parser::eval_cmd_color_sprite(LexerLine* statement) {
  Lexeme* next_lexeme;
  LexerLine parm;
  int state = 0, sepCount = 0;

  next_lexeme = statement->getCurrentLexeme();
  if (!next_lexeme) return false;

  pushActionFromLexeme(next_lexeme);

  while ((next_lexeme = statement->getNextLexeme())) {
    if (state == 0) {
      if (next_lexeme->isSeparator("(")) {
        state++;
        sepCount++;
        continue;
      } else
        return false;

    } else if (state == 1) {
      if (next_lexeme->isSeparator("(")) {
        sepCount++;
      } else if (next_lexeme->isSeparator(")")) {
        sepCount--;
      }

      if (next_lexeme->isSeparator(")") && sepCount == 0) {
        parm.setLexemeBOF();
        if (!eval_expression(&parm)) {
          return false;
        }
        parm.clearLexemes();

        state++;
        continue;
      } else {
        parm.addLexeme(next_lexeme);
      }

    } else if (state == 2) {
      if (next_lexeme->isOperator("=")) {
        state++;
        continue;
      } else
        return false;

    } else {
      parm.addLexeme(next_lexeme);
    }
  }

  if (state < 3) return false;

  if (parm.getLexemeCount()) {
    parm.setLexemeBOF();
    if (!eval_expression(&parm)) {
      return false;
    }
  } else
    return false;

  return true;
}

bool Parser::eval_cmd_def(LexerLine* statement, int vartype) {
  DefStatementStrategy strategy;
  return strategy.parseWithType(*this, statement, vartype);
}

bool Parser::eval_cmd_def_usr(LexerLine* statement) {
  DefStatementStrategy strategy;
  return strategy.parseWithType(*this, statement, 0);
}

bool Parser::eval_cmd_put(LexerLine* statement) {
  PutStatementStrategy strategy;
  return strategy.parseStatement(*this, statement);
}

bool Parser::eval_cmd_put_sprite(LexerLine* statement) {
  PutStatementStrategy strategy;
  return strategy.parsePutSprite(*this, statement);
}

bool Parser::eval_cmd_put_tile(LexerLine* statement) {
  PutStatementStrategy strategy;
  return strategy.parsePutTile(*this, statement);
}

bool Parser::eval_cmd_sprite(LexerLine* statement) {
  SpriteStatementStrategy strategy;
  return strategy.parseStatement(*this, statement);
}

bool Parser::eval_cmd_sprite_load(LexerLine* statement) {
  SpriteStatementStrategy strategy;
  return strategy.parseSpriteLoad(*this, statement);
}

bool Parser::eval_cmd_key(LexerLine* statement) {
  OnStatementStrategy strategy;
  return strategy.parseKey(*this, statement);
}

bool Parser::eval_cmd_strig(LexerLine* statement) {
  OnStatementStrategy strategy;
  return strategy.parseStrig(*this, statement);
}

bool Parser::eval_cmd_base(LexerLine* statement) {
  /*
      Lexeme *next_lexeme;
      LexerLine parm;
      ActionNode *action;
      int state = 0, sepCount = 0;

      while( (next_lexeme = statement->getNextLexeme()) ) {

          if(state == 0) {
              if(next_lexeme->type == Lexeme::type_separator &&
     next_lexeme->value == "(") {

                  action = new ActionNode("SET_BASE");
                  pushActionRoot(action);

                  state ++;
                  sepCount ++;
                  continue;

              } else
                  return false;

          } else if(state == 1) {
              if(next_lexeme->type == Lexeme::type_separator &&
     next_lexeme->value == "(") { sepCount ++; } else if(next_lexeme->type ==
     Lexeme::type_separator && next_lexeme->value == ")") { sepCount --;
              }
              if(next_lexeme->type == Lexeme::type_separator &&
     next_lexeme->value == ")" && sepCount == 0) {

                  parm.setLexemeBOF();
                  if(!eval_expression(&parm)) {
                      return false;
                  }
                  parm.clearLexemes();

                  state ++;
                  continue;
              } else {
                  parm.addLexeme(next_lexeme);
              }


          } else if(state == 2) {
              if(next_lexeme->type == Lexeme::type_operator &&
     next_lexeme->value == "=") { state ++; continue; } else return false;

          } else {
              parm.addLexeme(next_lexeme);

          }

      }

      if(state < 3)
          return false;

      if(parm.getLexemeCount()) {
          parm.setLexemeBOF();
          if(!eval_expression(&parm)) {
              return false;
          }
      } else
          return false;

      popActionRoot();
  */
  return true;
}

bool Parser::eval_cmd_vdp(LexerLine* statement) {
  /*
      Lexeme *next_lexeme;
      LexerLine parm;
      ActionNode *action;
      int state = 0, sepCount = 0;

      while( (next_lexeme = statement->getNextLexeme()) ) {

          if(state == 0) {
              if(next_lexeme->type == Lexeme::type_separator &&
     next_lexeme->value == "(") {

                  action = new ActionNode("SET_VDP");
                  pushActionRoot(action);

                  state ++;
                  sepCount ++;
                  continue;

              } else
                  return false;

          } else if(state == 1) {
              if(next_lexeme->type == Lexeme::type_separator &&
     next_lexeme->value == "(") { sepCount ++; } else if(next_lexeme->type ==
     Lexeme::type_separator && next_lexeme->value == ")") { sepCount --;
              }
              if(next_lexeme->type == Lexeme::type_separator &&
     next_lexeme->value == ")" && sepCount == 0) {

                  parm.setLexemeBOF();
                  if(!eval_expression(&parm)) {
                      return false;
                  }
                  parm.clearLexemes();

                  state ++;
                  continue;
              } else {
                  parm.addLexeme(next_lexeme);
              }


          } else if(state == 2) {
              if(next_lexeme->type == Lexeme::type_operator &&
     next_lexeme->value == "=") { state ++; continue; } else return false;

          } else {
              parm.addLexeme(next_lexeme);

          }

      }

      if(state < 3)
          return false;

      if(parm.getLexemeCount()) {
          parm.setLexemeBOF();
          if(!eval_expression(&parm)) {
              return false;
          }
      } else
          return false;

      popActionRoot();
  */
  return true;
}

bool Parser::eval_cmd_time(LexerLine* statement) {
  return eval_cmd_let(statement);
}

bool Parser::eval_cmd_if(LexerLine* statement, int level) {
  IfStatementStrategy strategy;
  return strategy.parseStatement(*this, statement, level);
}

bool Parser::eval_cmd_for(LexerLine* statement) {
  ForStatementStrategy strategy;
  return strategy.parseStatement(*this, statement);
}

bool Parser::eval_cmd_next(LexerLine* statement) {
  NextStatementStrategy strategy;
  return strategy.parseStatement(*this, statement);
}

bool Parser::eval_cmd_open(LexerLine* statement) {
  FileStatementStrategy strategy;
  return strategy.execute(*this, statement, statement->getCurrentLexeme());
}

bool Parser::eval_cmd_close(LexerLine* statement) {
  FileStatementStrategy strategy;
  return strategy.execute(*this, statement, statement->getCurrentLexeme());
}

bool Parser::eval_cmd_maxfiles(LexerLine* statement) {
  FileStatementStrategy strategy;
  return strategy.execute(*this, statement, statement->getCurrentLexeme());
}

bool Parser::eval_cmd_pset(LexerLine* statement) {
  GraphicsStatementStrategy strategy;
  return strategy.execute(*this, statement, statement->getCurrentLexeme());
}

bool Parser::eval_cmd_line(LexerLine* statement) {
  GraphicsStatementStrategy strategy;
  return strategy.execute(*this, statement, statement->getCurrentLexeme());
}

bool Parser::eval_cmd_circle(LexerLine* statement) {
  GraphicsStatementStrategy strategy;
  return strategy.execute(*this, statement, statement->getCurrentLexeme());
}

bool Parser::eval_cmd_paint(LexerLine* statement) {
  GraphicsStatementStrategy strategy;
  return strategy.execute(*this, statement, statement->getCurrentLexeme());
}

bool Parser::eval_cmd_copy(LexerLine* statement) {
  GraphicsStatementStrategy strategy;
  return strategy.execute(*this, statement, statement->getCurrentLexeme());
}

bool Parser::eval_cmd_set(LexerLine* statement) {
  SetStatementStrategy strategy;
  return strategy.parseStatement(*this, statement);
}

bool Parser::eval_cmd_set_adjust(LexerLine* statement) {
  SetStatementStrategy strategy;
  return strategy.parseSetAdjust(*this, statement);
}

bool Parser::eval_cmd_set_tile(LexerLine* statement) {
  SetStatementStrategy strategy;
  return strategy.parseSetTile(*this, statement);
}

bool Parser::eval_cmd_set_tile_colpat(LexerLine* statement) {
  SetStatementStrategy strategy;
  return strategy.parseSetTileColpat(*this, statement);
}

bool Parser::eval_cmd_set_sprite(LexerLine* statement) {
  SetStatementStrategy strategy;
  return strategy.parseSetSprite(*this, statement);
}

bool Parser::eval_cmd_set_sprite_colpattra(LexerLine* statement) {
  SetStatementStrategy strategy;
  return strategy.parseSetSpriteColpattra(*this, statement);
}

bool Parser::eval_cmd_get(LexerLine* statement) {
  GetStatementStrategy strategy;
  return strategy.parseStatement(*this, statement);
}

bool Parser::eval_cmd_get_tile(LexerLine* statement) {
  GetStatementStrategy strategy;
  return strategy.parseGetTile(*this, statement);
}

bool Parser::eval_cmd_get_sprite(LexerLine* statement) {
  GetStatementStrategy strategy;
  return strategy.parseGetSprite(*this, statement);
}

bool Parser::eval_cmd_screen(LexerLine* statement) {
  ScreenStatementStrategy strategy;
  return strategy.parseStatement(*this, statement);
}

bool Parser::eval_cmd_screen_copy(LexerLine* statement) {
  ScreenStatementStrategy strategy;
  return strategy.parseScreenCopy(*this, statement);
}

bool Parser::eval_cmd_screen_paste(LexerLine* statement) {
  ScreenStatementStrategy strategy;
  return strategy.parseScreenPaste(*this, statement);
}

bool Parser::eval_cmd_screen_scroll(LexerLine* statement) {
  ScreenStatementStrategy strategy;
  return strategy.parseScreenScroll(*this, statement);
}

bool Parser::eval_cmd_screen_load(LexerLine* statement) {
  ScreenStatementStrategy strategy;
  return strategy.parseScreenLoad(*this, statement);
}

bool Parser::eval_cmd_screen_on(LexerLine* statement) {
  ScreenStatementStrategy strategy;
  return strategy.parseScreenOn(*this, statement);
}

bool Parser::eval_cmd_screen_off(LexerLine* statement) {
  ScreenStatementStrategy strategy;
  return strategy.parseScreenOff(*this, statement);
}

bool Parser::eval_cmd_on(LexerLine* statement) {
  OnStatementStrategy strategy;
  return strategy.parseOn(*this, statement);
}

bool Parser::eval_cmd_on_goto_gosub(LexerLine* statement) {
  OnStatementStrategy strategy;
  return strategy.parseOnGotoGosub(*this, statement);
}

bool Parser::eval_cmd_on_error(LexerLine* statement) {
  OnStatementStrategy strategy;
  return strategy.parseOnError(*this, statement);
}

bool Parser::eval_cmd_on_interval(LexerLine* statement) {
  OnStatementStrategy strategy;
  return strategy.parseOnInterval(*this, statement);
}

bool Parser::eval_cmd_on_key(LexerLine* statement) {
  OnStatementStrategy strategy;
  return strategy.parseOnKey(*this, statement);
}

bool Parser::eval_cmd_on_sprite(LexerLine* statement) {
  OnStatementStrategy strategy;
  return strategy.parseOnSprite(*this, statement);
}

bool Parser::eval_cmd_on_stop(LexerLine* statement) {
  OnStatementStrategy strategy;
  return strategy.parseOnStop(*this, statement);
}

bool Parser::eval_cmd_on_strig(LexerLine* statement) {
  OnStatementStrategy strategy;
  return strategy.parseOnStrig(*this, statement);
}

bool Parser::eval_cmd_interval(LexerLine* statement) {
  OnStatementStrategy strategy;
  return strategy.parseInterval(*this, statement);
}

bool Parser::eval_cmd_stop(LexerLine* statement) {
  OnStatementStrategy strategy;
  return strategy.parseStop(*this, statement);
}

bool Parser::eval_cmd_call(LexerLine* statement) {
  CallStatementStrategy strategy;
  return strategy.parseCall(*this, statement);
}

bool Parser::eval_cmd_cmd(LexerLine* statement) {
  CmdStatementStrategy strategy;
  return strategy.parseStatement(*this, statement);
}

//-----------------------------------------------------------------------------------------------------------------

int Parser::gfxOperatorCode(Lexeme* lexeme) {
  int result = -1;

  if (lexeme->value == "PSET") {
    result = 0;
  } else if (lexeme->value == "AND") {
    result = 1;
  } else if (lexeme->value == "OR") {
    result = 2;
  } else if (lexeme->value == "XOR") {
    result = 3;
  } else if (lexeme->value == "PRESET") {
    result = 4;
  } else if (lexeme->value == "TPSET") {
    result = 8;
  } else if (lexeme->value == "TAND") {
    result = 9;
  } else if (lexeme->value == "TOR") {
    result = 10;
  } else if (lexeme->value == "TXOR") {
    result = 11;
  } else if (lexeme->value == "TPRESET") {
    result = 12;
  }

  return result;
}

bool Parser::loadInclude(Lexeme* lexeme) {
  FILE* file;
  char filename[255], *s;
  char line[255];
  int len = 255, tt, ii, k;
  LexerLine* lexerLine;

  tt = lexeme->value.size();
  s = (char*)lexeme->value.c_str();
  k = 0;

  for (ii = 0; ii < tt; ii++) {
    if (s[ii] != '"') {
      filename[k] = s[ii];
      k++;
    }
  }
  filename[k] = 0;

  /***
   * @remark
   * 1. instanciate a new LexerLine;
   * 2. read the file, line by line;
   * 3. evaluate it lexing tokens;
   * 4. and process it by calling evalLine(LexerLine).
   */

  if ((file = fopen(filename, "r"))) {
    bool result = true;

    lexerLine = new LexerLine();

    while (fgets(line, len, file)) {
      lexerLine->line = line;
      if (lexerLine->evaluate())
        eval_line(lexerLine);
      else {
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

void Parser::pushActionRoot(ActionNode* action) {
  if (ctx.actionRoot) {
    ctx.actionRoot->actions.push_back(action);
    ctx.actionStack.push(ctx.actionRoot);
  } else {
    while (!ctx.actionStack.empty()) ctx.actionStack.pop();
    ctx.tag->actions.push_back(action);
  }
  ctx.actionRoot = action;
}

void Parser::popActionRoot() {
  if (ctx.actionStack.empty()) {
    ctx.actionRoot = 0;
  } else {
    ctx.actionRoot = ctx.actionStack.top();
    ctx.actionStack.pop();
  }
}

string Parser::toString() {
  string out;
  for (unsigned int i = 0; i < ctx.tags.size(); i++) {
    if (ctx.tags[i]) out += ctx.tags[i]->toString();
  }
  return out;
}

string Parser::errorToString() {
  string out;
  if (ctx.error_line) out += ctx.error_line->toString();
  if (ctx.error_message.size() > 0) {
    out += ctx.error_message;
    out += "\n";
  }
  return out;
}
