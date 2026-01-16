/***
 * @file parse.cpp
 * @brief Parser class code specialized as a MSX BASIC syntax tree builder
 * @author Amaury Carvalho (2019-2025)
 * @note
 *   https://en.wikipedia.org/wiki/Parsing
 *   https://en.wikipedia.org/wiki/Recursive_descent_parser
 *   https://www.strchr.com/expression_evaluator
 *   https://en.wikipedia.org/wiki/Shunting-yard_algorithm
 */

#include "parse.h"

/***
 * @name Parser class code
 */

Parser::Parser() {
  int i;

  tag = 0;
  actionRoot = 0;
  error_line = 0;
  resourceCount = 0;

  lineNo = 0;
  eval_expr_error = false;
  line_comment = false;
  has_traps = false;
  has_defusr = false;
  has_data = false;
  has_idata = false;
  has_play = false;
  has_input = false;
  has_font = false;
  has_pt3 = false;
  has_akm = false;
  has_mtf = false;
  has_resource_restore = false;
  error_message = "";

  for (i = 0; i < 26; i++) deftbl[i] = 0;

  tags.clear();
  symbolList.clear();
  datas.clear();

  while (!actionStack.empty()) actionStack.pop();

  lex_null = new Lexeme(Lexeme::type_literal, Lexeme::subtype_null, "NULL");

  lex_empty_string =
      new Lexeme(Lexeme::type_literal, Lexeme::subtype_string, "");

  lex_index =
      new Lexeme(Lexeme::type_keyword, Lexeme::subtype_numeric, "INDEX");
}

Parser::~Parser() {
  if (lex_null) delete lex_null;
  if (lex_empty_string) delete lex_empty_string;
  if (lex_index) delete lex_index;
}

bool Parser::evaluate(Lexer* lexer) {
  int i, t = lexer->lines.size();
  LexerLine* lexerLine;

  this->opts = lexer->opts;

  Parser();

  for (i = 0, lineNo = 1; i < t; i++, lineNo++) {
    lexerLine = lexer->lines[i];
    // if(opts->debug) {
    // printf("%s\n",lexerLine->line.c_str());
    // }
    if (lexerLine->getLexemeCount() > 0) {
      line_comment = false;
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

  error_line = lexerLine;
  actionRoot = 0;

  if (lexeme) {
    lexeme = coalesceSymbols(lexeme);

    if (lexeme->isLiteralNumeric()) {
      tag = new TagNode();  // register line number tag
      tag->name = lexeme->value;
      tag->value = tag->name;
      tags.push_back(tag);

      while ((lexeme = lexerLine->getNextLexeme())) {
        lexeme = coalesceSymbols(lexeme);

        if (lexeme->isKeyword("IF")) {
          if_count++;
        }

        if (lexeme->isSeparator(":") && if_count == 0) {
          actionRoot = 0;
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
        actionRoot = 0;
        if (!eval_phrase(&phrase)) return false;
      }

    } else if (lexeme->type == Lexeme::type_keyword) {
      if (lexeme->value == "FILE" || lexeme->value == "TEXT") {
        resourceCount++;

        tag = new TagNode();  // register line number tag
        tag->name = "DIRECTIVE";
        tag->value = tag->name;
        tags.push_back(tag);

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
              error_message =
                  "INCLUDE file not found or with content syntax error";
              return false;
            }

          } else {
            error_message = "Invalid parameter in INCLUDE keyword";
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

  error_message = "Invalid keyword/identifier";
  return false;
}

bool Parser::eval_statement(LexerLine* statement) {
  Lexeme* lexeme;
  ActionNode* action;
  ActionNode* actionSaved = actionRoot;
  unsigned int actionCount = actionStack.size();
  bool result = true;

  lexeme = statement->getFirstLexeme();

  if (lexeme) {
    lexeme = coalesceSymbols(lexeme);

    action = new ActionNode(lexeme);
    pushActionRoot(action);

    if (lexeme->value == "REM" || lexeme->value == "CLS" ||
        lexeme->value == "END" || lexeme->value == "BEEP" ||
        lexeme->value == "RANDOMIZE") {
      result = true;
    } else if (lexeme->value == "'") {
      lexeme->type = Lexeme::type_keyword;
      lexeme->name = "REM";
      lexeme->value = lexeme->name;
      result = true;
    } else if (lexeme->value == "DEF") {
      result = eval_cmd_def(statement, 0);
    } else if (lexeme->value == "DEFINT") {
      result = eval_cmd_def(statement, 2);
    } else if (lexeme->value == "DEFSTR") {
      result = eval_cmd_def(statement, 3);
    } else if (lexeme->value == "DEFSNG") {
      result = eval_cmd_def(statement, 4);
    } else if (lexeme->value == "DEFDBL") {
      result = eval_cmd_def(statement, 8);
    } else if (lexeme->value == "WIDTH" || lexeme->value == "CLEAR" ||
               lexeme->value == "ERASE" || lexeme->value == "LOCATE" ||
               lexeme->value == "DRAW" || lexeme->value == "GOTO" ||
               lexeme->value == "GOSUB" || lexeme->value == "RETURN" ||
               lexeme->value == "SOUND" || lexeme->value == "RESTORE" ||
               lexeme->value == "RESUME" || lexeme->value == "READ" ||
               lexeme->value == "IREAD" || lexeme->value == "IRESTORE" ||
               lexeme->value == "POKE" || lexeme->value == "IPOKE" ||
               lexeme->value == "VPOKE" || lexeme->value == "OUT" ||
               lexeme->value == "SWAP" || lexeme->value == "WAIT" ||
               lexeme->value == "SEED" || lexeme->value == "BLOAD") {
      if (lexeme->value == "BLOAD") resourceCount++;
      result = eval_cmd_generic(statement);
    } else if (lexeme->value == "SCREEN") {
      result = eval_cmd_screen(statement);
    } else if (lexeme->value == "PLAY") {
      has_play = true;
      result = eval_cmd_generic(statement);
    } else if (lexeme->value == "LET") {
      result = eval_cmd_let(statement);
    } else if (lexeme->value == "DIM" || lexeme->value == "REDIM") {
      result = eval_cmd_dim(statement);
    } else if (lexeme->value == "PRINT") {
      result = eval_cmd_print(statement);
    } else if (lexeme->value == "?") {
      lexeme->value = "PRINT";
      lexeme->name = lexeme->value;
      result = eval_cmd_print(statement);
    } else if (lexeme->value == "INPUT") {
      has_input = true;
      result = eval_cmd_input(statement);
    } else if (lexeme->value == "SPRITE") {
      result = eval_cmd_sprite(statement);
    } else if (lexeme->value == "BASE") {
      result = eval_cmd_base(statement);
    } else if (lexeme->value == "VDP") {
      result = eval_cmd_vdp(statement);
    } else if (lexeme->value == "PUT") {
      result = eval_cmd_put(statement);
    } else if (lexeme->value == "TIME") {
      result = eval_cmd_time(statement);
    } else if (lexeme->value == "SET") {
      result = eval_cmd_set(statement);
    } else if (lexeme->value == "GET") {
      result = eval_cmd_get(statement);
    } else if (lexeme->value == "ON") {
      result = eval_cmd_on(statement);
    } else if (lexeme->value == "INTERVAL") {
      result = eval_cmd_interval(statement);
    } else if (lexeme->value == "STOP") {
      result = eval_cmd_stop(statement);
    } else if (lexeme->value == "KEY") {
      result = eval_cmd_key(statement);
    } else if (lexeme->value == "STRIG") {
      result = eval_cmd_strig(statement);
    } else if (lexeme->value == "COLOR") {
      result = eval_cmd_color(statement);
    } else if (lexeme->value == "DATA") {
      result = eval_cmd_data(statement, Lexeme::subtype_string);
    } else if (lexeme->value == "IDATA") {
      result = eval_cmd_data(statement, Lexeme::subtype_integer_data);
    } else if (lexeme->value == "IF") {
      return eval_cmd_if(statement, 0);
    } else if (lexeme->value == "FOR") {
      result = eval_cmd_for(statement);
    } else if (lexeme->value == "NEXT") {
      result = eval_cmd_next(statement);
    } else if (lexeme->value == "PSET" || lexeme->value == "PRESET") {
      result = eval_cmd_pset(statement);
    } else if (lexeme->value == "LINE") {
      result = eval_cmd_line(statement);
    } else if (lexeme->value == "CIRCLE") {
      result = eval_cmd_circle(statement);
    } else if (lexeme->value == "PAINT") {
      result = eval_cmd_paint(statement);
    } else if (lexeme->value == "COPY") {
      result = eval_cmd_copy(statement);
    } else if (lexeme->value == "_") {
      lexeme->value = "CALL";
      lexeme->name = lexeme->value;
      result = eval_cmd_call(statement);
    } else if (lexeme->value == "CALL") {
      result = eval_cmd_call(statement);
    } else if (lexeme->value == "CMD") {
      result = eval_cmd_cmd(statement);
    } else if (lexeme->value == "OPEN") {
      result = eval_cmd_open(statement);
    } else if (lexeme->value == "CLOSE") {
      result = eval_cmd_close(statement);
    } else if (lexeme->value == "MAX") {
      result = eval_cmd_maxfiles(statement);
    } else {
      result = false;
    }

    popActionRoot();
  }

  while (actionStack.size() > actionCount) actionStack.pop();

  actionRoot = actionSaved;

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
      if (actionRoot) {
        lexLet = actionRoot->lexeme;
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

  error_message = "Invalid LET statement";
  return false;
}

bool Parser::eval_expression(LexerLine* parm) {
  ActionNode* actionSaved = actionRoot;
  unsigned int actionCount = actionStack.size();

  while (!expressionList.empty()) expressionList.pop();

  if (!eval_expression_push(parm)) return false;

  while (!expressionList.empty()) {
    eval_expression_pop(1);
  }

  while (actionStack.size() > actionCount) actionStack.pop();

  actionRoot = actionSaved;

  return true;
}

void Parser::eval_expression_pop(int n) {
  Lexeme* lexeme;
  int k;

  while (n && !expressionList.empty()) {
    lexeme = expressionList.top();
    expressionList.pop();

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
          eval_expr_error = true;
          error_message = "Mismatched parentheses error";
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
            eval_expr_error = true;
            error_message = "Invalid = symbol in expression";
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
            eval_expr_error = true;
            error_message = "Invalid > symbol in expression";
            return false;
          }
        } else if (lexeme->value != "+" && lexeme->value != "-" &&
                   lexeme->value != "(" && lexeme->value != ")" &&
                   lexeme->value != "NOT") {
          eval_expr_error = true;
          error_message = "Invalid expression unary symbol";
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
          if (expressionList.empty()) {
            eval_expr_error = true;
            error_message =
                "Invalid FUNCTION or ARRAY declaration in expression";
            return false;
          }

          next_lexeme = expressionList.top();
          expressionList.pop();

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
            eval_expr_error = true;
            error_message = "Mismatched parentheses error in function or array";
            return false;
          }

          if (!parmcount) {
            eval_expr_error = true;
            error_message =
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
          eval_expr_error = true;
          error_message = "Mismatched parentheses error";
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
  expressionList.push(lexeme);
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
    actionRoot->actions.push_back(actionExpr);

  return actionExpr;
}

//-----------------------------------------------------------------------------------------------------------------

bool Parser::eval_cmd_generic(LexerLine* statement) {
  Lexeme* next_lexeme;
  LexerLine parm;
  int sepcount = 0;

  // get keyword parameters

  while ((next_lexeme = statement->getNextLexeme())) {
    next_lexeme = coalesceSymbols(next_lexeme);

    if (next_lexeme->isSeparator("(")) {
      sepcount++;
    } else if (next_lexeme->isSeparator(")") && sepcount > 0) {
      sepcount--;
    } else if (next_lexeme->type == Lexeme::type_separator &&
               (next_lexeme->value == "," || next_lexeme->value == ";") &&
               sepcount == 0) {
      if (parm.getLexemeCount()) {
        parm.setLexemeBOF();
        if (!eval_expression(&parm)) {
          return false;
        }
        parm.clearLexemes();
      } else {
        next_lexeme = lex_null;
        pushActionFromLexeme(next_lexeme);
      }

      continue;
    } else if (next_lexeme->isOperator("'")) {
      break;
    }

    parm.addLexeme(next_lexeme);
  }

  if (parm.getLexemeCount()) {
    parm.setLexemeBOF();
    if (!eval_expression(&parm)) {
      return false;
    }
  }

  return true;
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
  ActionNode *action = actionRoot, *subaction;
  Lexeme* lexeme;
  unsigned int i, t;

  if (!eval_cmd_generic(statement)) return false;

  t = action->actions.size();
  if (!t) return false;

  for (i = 0; i < t; i++) {
    subaction = action->actions[i];
    lexeme = subaction->lexeme;
    lexeme->isArray = true;
    lexeme->parm_count = subaction->actions.size();
    if (!lexeme->parm_count) {
      error_message =
          "Invalid array declaration: DIM size parameter is missing";
      return false;
    }
  }

  return true;
}

bool Parser::eval_cmd_let(LexerLine* statement) {
  return eval_assignment(statement);
}

bool Parser::eval_cmd_print(LexerLine* statement) {
  Lexeme* next_lexeme;
  LexerLine parm;
  ActionNode* action;
  int sepcount = 0, state = 0, i;
  bool print_using = false;
  Lexeme* lex_using[5] = {0, 0, 0, 0, 0};

  // get keyword parameters
  while ((next_lexeme = statement->getNextLexeme())) {
    next_lexeme = coalesceSymbols(next_lexeme);

    switch (state) {
      case 0: {
        if (next_lexeme->isSeparator("#") && sepcount == 0) {
          state = 1;
          pushActionFromLexeme(next_lexeme);
          continue;
        } else if (next_lexeme->isKeyword("USING")) {
          print_using = true;
          state = 3;
          continue;
        } else if (next_lexeme->isSeparator("(")) {
          sepcount++;
        } else if (next_lexeme->isSeparator(")") && sepcount > 0) {
          sepcount--;
        } else if (next_lexeme->type == Lexeme::type_separator &&
                   (next_lexeme->value == "," || next_lexeme->value == ";") &&
                   sepcount == 0) {
          if (parm.getLexemeCount()) {
            if (print_using) {
              parm.addLexeme(lex_using[4]);
            }

            parm.setLexemeBOF();
            if (!eval_expression(&parm)) {
              return false;
            }

            parm.clearLexemes();
          }

          action = new ActionNode(next_lexeme);
          actionRoot->actions.push_back(action);

          continue;
        }
      } break;

      case 1: {
        state = 2;
        pushActionFromLexeme(next_lexeme);
        popActionRoot();
        continue;
      } break;

      case 2: {
        if (next_lexeme->isSeparator(",")) {
          state = 0;
          continue;
        }
      } break;

      case 3: {
        if (next_lexeme->type == Lexeme::type_identifier ||
            next_lexeme->type == Lexeme::type_literal) {
          lex_using[0] = new Lexeme(Lexeme::type_keyword,
                                    Lexeme::subtype_function, "USING$");
          lex_using[1] =
              new Lexeme(Lexeme::type_separator, Lexeme::subtype_string, "(");
          lex_using[2] = next_lexeme;
          lex_using[3] =
              new Lexeme(Lexeme::type_separator, Lexeme::subtype_string, ",");
          lex_using[4] =
              new Lexeme(Lexeme::type_separator, Lexeme::subtype_string, ")");

          for (i = 0; i <= 3; i++) parm.addLexeme(lex_using[i]);

          state = 4;

          continue;
        } else {
          return false;
        }
      } break;

      case 4: {
        if (next_lexeme->type == Lexeme::type_separator &&
            (next_lexeme->value == "," || next_lexeme->value == ";")) {
          state = 0;
          continue;
        } else {
          return false;
        }
      } break;
    }

    if (parm.getLexemeCount() == 0 && print_using) {
      for (i = 0; i <= 3; i++) parm.addLexeme(lex_using[i]);
    }

    parm.addLexeme(next_lexeme);
  }

  if (parm.getLexemeCount()) {
    if (print_using) {
      parm.addLexeme(lex_using[4]);
    }
    parm.setLexemeBOF();
    if (!eval_expression(&parm)) {
      return false;
    }
  }

  return true;
}

bool Parser::eval_cmd_input(LexerLine* statement) {
  return eval_cmd_print(statement);
}

bool Parser::eval_cmd_line_input(LexerLine* statement) {
  return eval_cmd_print(statement);
}

bool Parser::eval_cmd_color(LexerLine* statement) {
  Lexeme* next_lexeme = statement->getNextLexeme();

  if (next_lexeme) {
    if (next_lexeme->isOperator("=")) {
      return eval_cmd_color_rgb(statement);
    } else if (next_lexeme->isKeyword("SPRITE")) {
      return eval_cmd_color_sprite(statement);
    } else if (next_lexeme->isKeyword("SPRITE$")) {
      return eval_cmd_color_sprite(statement);
    } else {
      statement->getPreviousLexeme();
      return eval_cmd_generic(statement);
    }
  } else {
    error_message = "Invalid COLOR statement";
  }

  return false;
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
  Lexeme* next_lexeme;
  int state = 0, c[2], i;

  if (vartype == 0) {
    if ((next_lexeme = statement->getNextLexeme())) {
      next_lexeme = coalesceSymbols(next_lexeme);
      if (next_lexeme->name == "INT")
        return eval_cmd_def(statement, 2);
      else if (next_lexeme->name == "STR")
        return eval_cmd_def(statement, 3);
      else if (next_lexeme->name == "SNG")
        return eval_cmd_def(statement, 4);
      else if (next_lexeme->name == "DBL")
        return eval_cmd_def(statement, 8);
      else if (next_lexeme->name == "USR")
        return eval_cmd_def_usr(statement);
      else if (next_lexeme->name == "USR0")
        return eval_cmd_def_usr(statement);
      else
        return false;
    } else
      return false;
  }

  c[0] = -1;
  c[1] = -1;

  while ((next_lexeme = statement->getNextLexeme())) {
    next_lexeme = coalesceSymbols(next_lexeme);

    if (next_lexeme->isSeparator(",")) {
      if (state) {
        if (c[0] >= 0 && c[1] >= 0)
          for (i = c[0]; i <= c[1]; i++) deftbl[i] = vartype;
      } else {
        if (c[0] >= 0) deftbl[c[0]] = vartype;
      }

      state = 0;
      c[0] = -1;
      c[1] = -1;

    } else if (next_lexeme->isOperator("-")) {
      state = 1;

    } else if (next_lexeme->type == Lexeme::type_identifier) {
      c[state] = next_lexeme->name.c_str()[0] - 'A';
      if (!(c[state] >= 0 && c[state] < 26)) c[state] = -1;
    }
  }

  if (state) {
    if (c[0] >= 0 && c[1] >= 0)
      for (i = c[0]; i <= c[1]; i++) deftbl[i] = vartype;
  } else {
    if (c[0] >= 0) deftbl[c[0]] = vartype;
  }

  return true;
}

bool Parser::eval_cmd_def_usr(LexerLine* statement) {
  Lexeme *next_lexeme, *lex_zero;
  LexerLine parm;
  int state = 0;
  bool lex_zero_used = false;

  has_defusr = true;

  next_lexeme = statement->getCurrentLexeme();
  if (!next_lexeme) {
    return false;
  }

  lex_zero = new Lexeme(Lexeme::type_literal, Lexeme::subtype_numeric, "0");

  if (next_lexeme->value == "USR0") {
    next_lexeme->name = "USR";
    next_lexeme->value = next_lexeme->name;
    pushActionFromLexeme(lex_zero);
    lex_zero_used = true;
    state = 1;
  }

  pushActionFromLexeme(next_lexeme);

  while ((next_lexeme = statement->getNextLexeme())) {
    switch (state) {
      case 0: {
        if (next_lexeme->isLiteralNumeric()) {
          state = 1;
          pushActionFromLexeme(next_lexeme);
          continue;
        } else if (next_lexeme->isOperator("=")) {
          state = 2;
          pushActionFromLexeme(lex_zero);
          lex_zero_used = true;
        } else {
          error_message = "Invalid DEF USR assignment";
          if (!lex_zero_used) delete lex_zero;
          return false;
        }
      } break;

      case 1: {
        if (next_lexeme->isOperator("=")) {
          state = 2;
        } else {
          error_message = "DEF USR assignment is missing";
          if (!lex_zero_used) delete lex_zero;
          return false;
        }
      } break;

      case 2: {
        parm.addLexeme(next_lexeme);
      }
    }
  }

  if (parm.getLexemeCount()) {
    parm.setLexemeBOF();
    if (!eval_expression(&parm)) {
      if (!lex_zero_used) delete lex_zero;
      return false;
    }
    parm.clearLexemes();
  }

  popActionRoot();

  if (!lex_zero_used) delete lex_zero;

  return true;
}

bool Parser::eval_cmd_put(LexerLine* statement) {
  Lexeme* next_lexeme;
  ActionNode* action;
  bool result = false;

  if ((next_lexeme = statement->getNextLexeme())) {
    next_lexeme = coalesceSymbols(next_lexeme);

    if (next_lexeme->type == Lexeme::type_keyword) {
      action = new ActionNode(next_lexeme);
      pushActionRoot(action);

      if (next_lexeme->value == "SPRITE") {
        result = eval_cmd_put_sprite(statement);
      } else if (next_lexeme->value == "TILE") {
        result = eval_cmd_put_tile(statement);
      }

      popActionRoot();
    }
  }

  return result;
}

bool Parser::eval_cmd_put_sprite(LexerLine* statement) {
  Lexeme* next_lexeme;
  ActionNode *action, *act_coord;
  LexerLine parm;
  int state = 0, sepCount = 0;

  act_coord = new ActionNode("COORD");

  parm.clearLexemes();

  while ((next_lexeme = statement->getNextLexeme())) {
    next_lexeme = coalesceSymbols(next_lexeme);

    switch (state) {
      case 0: {
        if (next_lexeme->isSeparator(",")) {
          if (parm.getLexemeCount()) {
            parm.setLexemeBOF();
            if (!eval_expression(&parm)) {
              return false;
            }
            parm.clearLexemes();
          } else {
            pushActionFromLexeme(lex_null);
          }
          state = 1;
          continue;
        }
      } break;

      case 1: {
        if (next_lexeme->isKeyword("STEP")) {
          action = new ActionNode(next_lexeme);
          pushActionRoot(action);
          continue;
        } else if (next_lexeme->isSeparator("(")) {
          state = 2;
          if (actionRoot) {
            if (actionRoot->lexeme) {
              if (actionRoot->lexeme->value != "STEP") {
                pushActionRoot(act_coord);
              }
            }
          }
          continue;
        } else if (next_lexeme->isSeparator(",")) {
          state = 3;
          pushActionRoot(act_coord);
          pushActionFromLexeme(lex_null);
          pushActionFromLexeme(lex_null);
          popActionRoot();
          continue;
        } else {
          error_message = "PUT SPRITE without a valid complement.";
          eval_expr_error = true;
          return false;
        }
      } break;

      case 2: {
        if (next_lexeme->isSeparator("(")) {
          sepCount++;
        } else if (next_lexeme->isSeparator(")")) {
          if (sepCount) {
            sepCount--;
          } else {
            state = 3;
            if (parm.getLexemeCount()) {
              parm.setLexemeBOF();
              if (!eval_expression(&parm)) {
                return false;
              }
              parm.clearLexemes();
            } else {
              if (actionRoot) {
                pushActionFromLexeme(lex_null);
                if (actionRoot->actions.size() == 1) {
                  pushActionFromLexeme(lex_null);
                }
              }
            }
            popActionRoot();
            next_lexeme = statement->getNextLexeme();
            if (next_lexeme) {
              if (next_lexeme->type != Lexeme::type_separator ||
                  next_lexeme->value != ",") {
                return false;
              }
            }
            continue;
          }
        } else if (next_lexeme->isSeparator(",") && sepCount == 0) {
          if (parm.getLexemeCount()) {
            parm.setLexemeBOF();
            if (!eval_expression(&parm)) {
              return false;
            }
            parm.clearLexemes();
          } else {
            pushActionFromLexeme(lex_null);
          }
          continue;
        }

      } break;

      case 3: {
        if (next_lexeme->isSeparator("(")) {
          sepCount++;
        } else if (next_lexeme->isSeparator(")")) {
          if (sepCount) sepCount--;
        } else if (next_lexeme->isSeparator(",") && sepCount == 0) {
          if (parm.getLexemeCount()) {
            parm.setLexemeBOF();
            if (!eval_expression(&parm)) {
              return false;
            }
            parm.clearLexemes();
          } else {
            next_lexeme = lex_null;
            pushActionFromLexeme(next_lexeme);
          }
          continue;
        }

      } break;
    }

    parm.addLexeme(next_lexeme);
  }

  if (parm.getLexemeCount()) {
    parm.setLexemeBOF();
    if (!eval_expression(&parm)) {
      return false;
    }

    parm.clearLexemes();
  }

  return true;
}

bool Parser::eval_cmd_put_tile(LexerLine* statement) {
  Lexeme* next_lexeme;
  ActionNode *action, *act_coord;
  LexerLine parm;
  int state = 0, sepCount = 0;

  act_coord = new ActionNode("COORD");

  parm.clearLexemes();

  while ((next_lexeme = statement->getNextLexeme())) {
    next_lexeme = coalesceSymbols(next_lexeme);

    switch (state) {
      case 0: {
        if (next_lexeme->isSeparator(",")) {
          if (parm.getLexemeCount()) {
            parm.setLexemeBOF();
            if (!eval_expression(&parm)) {
              return false;
            }
            parm.clearLexemes();
          } else {
            pushActionFromLexeme(lex_null);
          }
          state = 1;
          continue;
        }
      } break;

      case 1: {
        if (next_lexeme->isKeyword("STEP")) {
          action = new ActionNode(next_lexeme);
          pushActionRoot(action);
          continue;
        } else if (next_lexeme->isSeparator("(")) {
          state = 2;
          if (actionRoot) {
            if (actionRoot->lexeme) {
              if (actionRoot->lexeme->value != "STEP") {
                pushActionRoot(act_coord);
              }
            }
          }
          continue;
        } else if (next_lexeme->isSeparator(",")) {
          state = 3;
          pushActionRoot(act_coord);
          pushActionFromLexeme(lex_null);
          pushActionFromLexeme(lex_null);
          popActionRoot();
          continue;
        } else {
          error_message = "PUT TILE without a valid complement.";
          eval_expr_error = true;
          return false;
        }
      } break;

      case 2: {
        if (next_lexeme->isSeparator("(")) {
          sepCount++;
        } else if (next_lexeme->isSeparator(")")) {
          if (sepCount) {
            sepCount--;
          } else {
            state = 3;
            if (parm.getLexemeCount()) {
              parm.setLexemeBOF();
              if (!eval_expression(&parm)) {
                return false;
              }
              parm.clearLexemes();
            } else {
              if (actionRoot) {
                pushActionFromLexeme(lex_null);
                if (actionRoot->actions.size() == 1) {
                  pushActionFromLexeme(lex_null);
                }
              }
            }
            popActionRoot();
            next_lexeme = statement->getNextLexeme();
            if (next_lexeme) {
              if (next_lexeme->type != Lexeme::type_separator ||
                  next_lexeme->value != ",") {
                return false;
              }
            }
            continue;
          }
        } else if (next_lexeme->isSeparator(",") && sepCount == 0) {
          if (parm.getLexemeCount()) {
            parm.setLexemeBOF();
            if (!eval_expression(&parm)) {
              return false;
            }
            parm.clearLexemes();
          } else {
            pushActionFromLexeme(lex_null);
          }
          continue;
        }

      } break;

      case 3: {
        if (next_lexeme->isSeparator("(")) {
          sepCount++;
        } else if (next_lexeme->isSeparator(")")) {
          if (sepCount) sepCount--;
        } else if (next_lexeme->isSeparator(",") && sepCount == 0) {
          if (parm.getLexemeCount()) {
            parm.setLexemeBOF();
            if (!eval_expression(&parm)) {
              return false;
            }
            parm.clearLexemes();
          } else {
            next_lexeme = lex_null;
            pushActionFromLexeme(next_lexeme);
          }
          continue;
        }

      } break;
    }

    parm.addLexeme(next_lexeme);
  }

  if (parm.getLexemeCount()) {
    parm.setLexemeBOF();
    if (!eval_expression(&parm)) {
      return false;
    }

    parm.clearLexemes();
  }

  return true;
}

bool Parser::eval_cmd_sprite(LexerLine* statement) {
  Lexeme* next_lexeme;

  if ((next_lexeme = statement->getNextLexeme())) {
    if (next_lexeme->type == Lexeme::type_keyword) {
      if (next_lexeme->value == "ON" || next_lexeme->value == "OFF" ||
          next_lexeme->value == "STOP") {
        pushActionFromLexeme(next_lexeme);

        return true;

      } else if (next_lexeme->value == "LOAD") {
        return eval_cmd_sprite_load(statement);
      }
    }
  }

  return false;
}

bool Parser::eval_cmd_sprite_load(LexerLine* statement) {
  Lexeme* next_lexeme = statement->getCurrentLexeme();
  bool result;
  pushActionFromLexeme(next_lexeme);
  result = eval_cmd_generic(statement);
  popActionRoot();
  return result;
}

bool Parser::eval_cmd_key(LexerLine* statement) {
  Lexeme* next_lexeme;
  LexerLine parm;
  int sepCount = 0;

  while ((next_lexeme = statement->getNextLexeme())) {
    if (next_lexeme->type == Lexeme::type_keyword) {
      if (next_lexeme->value == "ON" || next_lexeme->value == "OFF" ||
          next_lexeme->value == "STOP") {
        if (parm.getLexemeCount()) {
          parm.setLexemeBOF();
          if (!eval_expression(&parm)) {
            return false;
          }
          parm.clearLexemes();
        }

        pushActionFromLexeme(next_lexeme);

        continue;
      }

    } else if (next_lexeme->isSeparator("(")) {
      sepCount++;

    } else if (next_lexeme->isSeparator(")")) {
      if (sepCount) sepCount--;

    } else if (next_lexeme->isSeparator(",") && sepCount == 0) {
      if (parm.getLexemeCount()) {
        parm.setLexemeBOF();
        if (!eval_expression(&parm)) {
          return false;
        }
        parm.clearLexemes();
      }

      continue;
    }

    parm.addLexeme(next_lexeme);
  }

  if (parm.getLexemeCount()) {
    parm.setLexemeBOF();
    if (!eval_expression(&parm)) {
      return false;
    }
    parm.clearLexemes();
  }

  return true;
}

bool Parser::eval_cmd_strig(LexerLine* statement) {
  Lexeme* next_lexeme;
  LexerLine parm;
  int sepCount = 0;

  while ((next_lexeme = statement->getNextLexeme())) {
    if (next_lexeme->type == Lexeme::type_keyword) {
      if (next_lexeme->value == "ON" || next_lexeme->value == "OFF" ||
          next_lexeme->value == "STOP") {
        if (parm.getLexemeCount()) {
          parm.setLexemeBOF();
          if (!eval_expression(&parm)) {
            return false;
          }
          parm.clearLexemes();
        }

        pushActionFromLexeme(next_lexeme);

        continue;
      }

    } else if (next_lexeme->isSeparator("(")) {
      sepCount++;

    } else if (next_lexeme->isSeparator(")")) {
      if (sepCount) sepCount--;

    } else if (next_lexeme->isSeparator(",") && sepCount == 0) {
      if (parm.getLexemeCount()) {
        parm.setLexemeBOF();
        if (!eval_expression(&parm)) {
          return false;
        }
        parm.clearLexemes();
      }

      continue;
    }

    parm.addLexeme(next_lexeme);
  }

  if (parm.getLexemeCount()) {
    parm.setLexemeBOF();
    if (!eval_expression(&parm)) {
      return false;
    }
    parm.clearLexemes();
  }

  return true;
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
  Lexeme *next_lexeme, *last_lexeme = statement->getCurrentLexeme();
  LexerLine parm;
  ActionNode* action;
  int state = 0;
  bool testGotoGosub = false, testIf = false, skipEmptyStmtCheck = false;

  action = new ActionNode("COND");
  pushActionRoot(action);

  parm.clearLexemes();

  while ((next_lexeme = statement->getNextLexeme())) {
    switch (state) {
      // CONDITION parse
      case 0: {
        if (next_lexeme->isKeyword("THEN") || next_lexeme->isKeyword("GOTO") ||
            next_lexeme->isKeyword("GOSUB")) {
          if (parm.getLexemeCount()) {
            parm.setLexemeBOF();
            if (!eval_expression(&parm)) {
              error_message = "IF command without a valid condition";
              eval_expr_error = true;
              return false;
            }
            parm.clearLexemes();
          } else {
            error_message = "IF command with a empty condition";
            eval_expr_error = true;
            return false;
          }

          popActionRoot();

          action = new ActionNode(next_lexeme);
          pushActionRoot(action);

          last_lexeme = next_lexeme;

          state = 1;
          testGotoGosub = true;
          testIf = true;

          continue;

        } else if (next_lexeme->isKeyword("ELSE")) {
          error_message = "ELSE without a THEN/GOTO/GOSUB";
          eval_expr_error = true;
          return false;

        } else if (next_lexeme->isSeparator(":")) {
          error_message = "Invalid separator on IF statement";
          eval_expr_error = true;
          return false;
        }

      } break;

      // THEN/GOTO/GOSUB parse
      case 1:
      // ELSE parse
      case 2: {
        if (testIf) {
          testIf = false;
          if (next_lexeme->isKeyword("IF")) {
            if (parm.getLexemeCount()) {
              parm.setLexemeBOF();
              if (!eval_expression(&parm)) {
                error_message = "Invalid expression before IF command";
                eval_expr_error = true;
                return false;
              }
              parm.clearLexemes();
            }

            next_lexeme = coalesceSymbols(next_lexeme);
            action = new ActionNode(next_lexeme);
            pushActionRoot(action);
            if (!eval_cmd_if(statement, level + 1)) {
              return false;
            }
            skipEmptyStmtCheck = true;
            testGotoGosub = false;
            continue;
          } else if (next_lexeme->isKeyword("THEN")) {
            error_message = "Duplicated THEN in an IF command";
            eval_expr_error = true;
            return false;
          }
        }

        if (testGotoGosub) {
          testGotoGosub = false;
          if (next_lexeme->isLiteralNumeric()) {
            if (last_lexeme->isKeyword("THEN") ||
                last_lexeme->isKeyword("ELSE")) {
              action = new ActionNode("GOTO");
              pushActionRoot(action);
              pushActionFromLexeme(next_lexeme);
              popActionRoot();
            } else if (last_lexeme->isKeyword("GOTO")) {
              if (state == 1)
                last_lexeme->value = "THEN";
              else
                last_lexeme->value = "ELSE";
              last_lexeme->name = last_lexeme->value;
              action = new ActionNode("GOTO");
              pushActionRoot(action);
              pushActionFromLexeme(next_lexeme);
              popActionRoot();
            } else if (last_lexeme->isKeyword("GOSUB")) {
              if (state == 1)
                last_lexeme->value = "THEN";
              else
                last_lexeme->value = "ELSE";
              last_lexeme->name = last_lexeme->value;
              action = new ActionNode("GOSUB");
              pushActionRoot(action);
              pushActionFromLexeme(next_lexeme);
              popActionRoot();
            } else {
              error_message = "IF with invalid GOTO/GOSUB parameter";
              eval_expr_error = true;
              return false;
            }

            parm.clearLexemes();
            skipEmptyStmtCheck = true;

            continue;
          }
        }

        if (next_lexeme->isSeparator(":") || next_lexeme->isKeyword("ELSE")) {
          testIf = true;

          if (parm.getLexemeCount()) {
            parm.setLexemeBOF();
            if (!eval_phrase(&parm)) return false;
            parm.clearLexemes();
          } else {
            if (skipEmptyStmtCheck) {
              skipEmptyStmtCheck = false;
            } else {
              error_message = "IF with an empty statement";
              eval_expr_error = true;
              return false;
            }
          }

          if (next_lexeme->isKeyword("ELSE")) {
            popActionRoot();

            if (state == 1) {
              action = new ActionNode(next_lexeme);
              pushActionRoot(action);
              last_lexeme = next_lexeme;
              testGotoGosub = true;
              state = 2;
            } else {
              if (level) {
                popActionRoot();
                statement->getPreviousLexeme();
                return true;
              }
              error_message = "Duplicated ELSE in an IF command";
              eval_expr_error = true;
              return false;
            }
          }

          continue;
        }

      } break;
    }

    parm.addLexeme(next_lexeme);
  }

  if (state == 0) {
    error_message = "IF without a THEN/GOTO/GOSUB/ELSE complement";
    eval_expr_error = true;
    return false;
  }

  if (parm.getLexemeCount()) {
    if (state > 2) {
      error_message = "Code detected after end of an IF statement";
      eval_expr_error = true;
      return false;
    }

    if (testGotoGosub) {
      next_lexeme = parm.getFirstLexeme();
      if (next_lexeme->isLiteralNumeric()) {
        action = new ActionNode("GOTO");
        pushActionRoot(action);
        pushActionFromLexeme(next_lexeme);
        popActionRoot();

        parm.clearLexemes();
      }
    } else {
      if (parm.getLexemeCount()) {
        parm.setLexemeBOF();
        if (!eval_phrase(&parm)) return false;
        parm.clearLexemes();
      }
    }

    popActionRoot();
  }

  popActionRoot();

  return true;
}

bool Parser::eval_cmd_for(LexerLine* statement) {
  Lexeme *next_lexeme, *last_lexeme = 0;
  LexerLine parm;
  ActionNode* action;
  int state = 0;

  parm.clearLexemes();

  while ((next_lexeme = statement->getNextLexeme())) {
    switch (state) {
      case 0: {
        if (next_lexeme->isKeyword("TO")) {
          parm.setLexemeBOF();
          if (!eval_assignment(&parm)) {
            error_message = "FOR command without a valid assignment";
            eval_expr_error = true;
            return false;
          }

          parm.clearLexemes();

          action = new ActionNode(next_lexeme);
          pushActionRoot(action);

          last_lexeme = next_lexeme;

          state = 1;

          continue;
        }

      } break;

      case 1: {
        if (next_lexeme->isKeyword("STEP")) {
          if (last_lexeme->value != "TO") {
            error_message = "STEP without a TO clausule";
            eval_expr_error = true;
            return false;
          }

          parm.setLexemeBOF();
          if (!eval_expression(&parm)) {
            error_message = "FOR with an invalid TO/STEP";
            eval_expr_error = true;
            return false;
          }

          parm.clearLexemes();
          popActionRoot();

          action = new ActionNode(next_lexeme);
          pushActionRoot(action);

          last_lexeme = next_lexeme;

          state = 2;

          continue;
        }

      } break;
    }

    parm.addLexeme(next_lexeme);
  }

  if (parm.getLexemeCount() && last_lexeme) {
    if (last_lexeme->value != "TO" && last_lexeme->value != "STEP") {
      error_message = "FOR command without a TO/STEP complement.";
      eval_expr_error = true;
      return false;
    }

    parm.setLexemeBOF();
    if (!eval_expression(&parm)) {
      error_message = "FOR with an invalid TO/STEP";
      eval_expr_error = true;
      return false;
    }

    popActionRoot();
    parm.clearLexemes();

  } else {
    error_message = "Invalid FOR statement (empty)";
    return false;
  }

  return true;
}

bool Parser::eval_cmd_next(LexerLine* statement) {
  Lexeme *next_lexeme, *current_lexeme = actionRoot->lexeme;
  ActionNode* action;
  int sepCount = 0;

  // test more next parameters

  while ((next_lexeme = statement->getNextLexeme())) {
    if (next_lexeme->isSeparator("(")) {
      sepCount++;
    } else if (next_lexeme->isSeparator(")")) {
      if (sepCount) sepCount--;
    } else if (next_lexeme->isSeparator(",") && sepCount == 0) {
      popActionRoot();
      action = new ActionNode(current_lexeme);
      pushActionRoot(action);
    }
  }

  return true;
}

bool Parser::eval_cmd_open(LexerLine* statement) {
  Lexeme* next_lexeme;
  char* s;
  string stext;
  int state = 0;

  while ((next_lexeme = statement->getNextLexeme())) {
    switch (state) {
      case 0: {
        if (next_lexeme->type == Lexeme::type_identifier ||
            next_lexeme->type == Lexeme::type_literal) {
          string str = next_lexeme->value;
          size_t pos;

          // strip quotes
          if (str.size()) str.erase(str.size() - 1, 1);
          if (str.size()) str.erase(0, 1);
          if (str.size()) {
            // trim spaces of string
            while ((pos = str.find(" ")) != string::npos) {
              str.erase(pos, 1);
            }
            // turn string to uppercase
            for (auto& c : str) c = (char)toupper(c);
          }

          if (next_lexeme->type == Lexeme::type_literal && str == "GRP:") {
            actionRoot->lexeme->name = "OPEN_GRP";
            actionRoot->lexeme->value = actionRoot->lexeme->name;
            return true;
          }
          pushActionFromLexeme(next_lexeme);
          state = 1;
          continue;
        } else {
          error_message = "File name is missing in OPEN statement";
          return false;
        }
      } break;

      case 1: {
        s = (char*)next_lexeme->value.c_str();

        if (next_lexeme->isKeyword("FOR")) {
          state = 2;
          continue;
        } else if (next_lexeme->value == "AS") {
          state = 4;
          continue;
        } else if (s[0] == 'A' && s[1] == 'S' && s[2] == '#') {
          s += 3;
          stext = *s;
          next_lexeme =
              new Lexeme(Lexeme::type_literal, Lexeme::subtype_numeric, stext);
          pushActionFromLexeme(next_lexeme);
          state = 5;
          continue;
        } else {
          error_message = "FOR/AS is missing in OPEN statement";
          return false;
        }
      } break;

      case 2: {
        s = (char*)next_lexeme->value.c_str();

        if (next_lexeme->value == "INPUT" || next_lexeme->value == "OUT" ||
            next_lexeme->value == "APP") {
          pushActionFromLexeme(next_lexeme);
          popActionRoot();
          continue;
        } else if (next_lexeme->type == Lexeme::type_keyword &&
                   (next_lexeme->value == "PUT" ||
                    next_lexeme->value == "END")) {
          continue;
        } else if (next_lexeme->value == "AS") {
          state = 4;
          continue;
        } else if (s[0] == 'A' && s[1] == 'S' && s[2] == '#') {
          s += 3;
          stext = *s;
          next_lexeme =
              new Lexeme(Lexeme::type_literal, Lexeme::subtype_numeric, stext);
          pushActionFromLexeme(next_lexeme);
          state = 5;
          continue;
        } else {
          error_message = "Invalid FOR parameter in OPEN statement";
          return false;
        }
      } break;

      case 3: {
        s = (char*)next_lexeme->value.c_str();

        if (next_lexeme->value == "AS") {
          state = 4;
          continue;
        } else if (s[0] == 'A' && s[1] == 'S' && s[2] == '#') {
          s += 3;
          stext = *s;
          next_lexeme =
              new Lexeme(Lexeme::type_literal, Lexeme::subtype_numeric, stext);
          pushActionFromLexeme(next_lexeme);
          state = 5;
          continue;
        } else {
          error_message = "AS is missing in OPEN statement";
          return false;
        }
      } break;

      case 4: {
        if (next_lexeme->isSeparator("#")) continue;

        if (next_lexeme->type == Lexeme::type_identifier ||
            next_lexeme->type == Lexeme::type_literal) {
          pushActionFromLexeme(next_lexeme);
          state = 5;
          continue;
        } else {
          error_message = "File number is missing in OPEN statement";
          return false;
        }
      } break;

      case 5: {
        if (next_lexeme->isKeyword("LEN")) {
          state = 6;
          continue;
        } else {
          error_message = "LEN is missing in OPEN statement";
          return false;
        }
      } break;

      case 6: {
        if (next_lexeme->isOperator("=")) continue;

        if (next_lexeme->type == Lexeme::type_identifier ||
            next_lexeme->type == Lexeme::type_literal) {
          pushActionFromLexeme(next_lexeme);
          state = 7;
          continue;
        } else {
          error_message = "Record length is missing in OPEN statement";
          return false;
        }
      } break;
    }
  }

  return true;
}

bool Parser::eval_cmd_close(LexerLine* statement) {
  Lexeme* next_lexeme;
  int state = 0;

  while ((next_lexeme = statement->getNextLexeme())) {
    switch (state) {
      case 0: {
        if (next_lexeme->isSeparator("#")) {
          state = 1;
        } else {
          error_message = "# is missing in CLOSE statement";
          return false;
        }

      } break;

      case 1: {
        if (next_lexeme->isLiteralNumeric()) {
          pushActionFromLexeme(next_lexeme);
        } else {
          error_message = "Invalid parameter in CLOSE statement";
          return false;
        }

      } break;

      case 2: {
        if (next_lexeme->isSeparator(",")) {
          state = 0;
        } else {
          error_message = "Comma is missing in CLOSE statement";
          return false;
        }
      } break;
    }
  }

  return true;
}

bool Parser::eval_cmd_maxfiles(LexerLine* statement) {
  Lexeme* next_lexeme;
  LexerLine parm;
  int state = 0;

  next_lexeme = statement->getCurrentLexeme();
  if (!next_lexeme) return false;

  next_lexeme->name = "MAXFILES";
  next_lexeme->value = next_lexeme->name;

  while ((next_lexeme = statement->getNextLexeme())) {
    switch (state) {
      case 0: {
        if (next_lexeme->isKeyword("FILES")) {
          state = 1;
          continue;
        } else {
          error_message = "Invalid MAXFILES assignment";
          return false;
        }
      } break;

      case 1: {
        if (next_lexeme->isOperator("=")) {
          state = 2;
        } else {
          error_message = "MAXFILES assignment is missing";
          return false;
        }
      } break;

      case 2: {
        parm.addLexeme(next_lexeme);
      }
    }
  }

  if (parm.getLexemeCount()) {
    parm.setLexemeBOF();
    if (!eval_expression(&parm)) {
      return false;
    }
    parm.clearLexemes();
  }

  return true;
}

bool Parser::eval_cmd_pset(LexerLine* statement) {
  Lexeme* next_lexeme;
  ActionNode* action;
  LexerLine parm;
  int state = 0, sepCount = 0, parmCount = 0;
  bool mustPopAction = false, isKeyword = false;
  string parmValue;

  parm.clearLexemes();

  while ((next_lexeme = statement->getNextLexeme())) {
    next_lexeme = coalesceSymbols(next_lexeme);

    if (state == 0) {
      if (next_lexeme->isKeyword("STEP")) {
        action = new ActionNode(next_lexeme);
        pushActionRoot(action);
        continue;
      } else if (next_lexeme->isSeparator("(")) {
        state++;
        parmCount++;
        if (actionRoot->lexeme->value != "STEP") {
          action = new ActionNode("COORD");
          pushActionRoot(action);
        }
        continue;
      } else {
        error_message = "PSET without a valid complement.";
        eval_expr_error = true;
        return false;
      }
    } else if (state == 1) {
      if (next_lexeme->isSeparator("(")) {
        sepCount++;
      } else if (next_lexeme->isSeparator(")")) {
        if (sepCount) {
          sepCount--;
        } else {
          mustPopAction = true;
          parmCount++;
          continue;
        }
      } else if (next_lexeme->isSeparator(",") && sepCount == 0) {
        if (parm.getLexemeCount()) {
          parm.setLexemeBOF();
          if (!eval_expression(&parm)) {
            return false;
          }
          parm.clearLexemes();
          if (mustPopAction) {
            popActionRoot();
            mustPopAction = false;
          } else
            parmCount++;
        } else {
          parmCount++;
          next_lexeme = lex_null;
          pushActionFromLexeme(next_lexeme);
        }
        continue;
      }
      parm.addLexeme(next_lexeme);
    }
  }

  if (parm.getLexemeCount()) {
    next_lexeme = parm.getFirstLexeme();

    if (parmCount > 3) {
      if (next_lexeme->type == Lexeme::type_keyword ||
          next_lexeme->type == Lexeme::type_operator) {
        int operatorCode = gfxOperatorCode(next_lexeme);

        isKeyword = true;

        if (operatorCode >= 0) {
          parmValue = to_string(operatorCode);
        } else {
          error_message = "Invalid operator parameter in PSET statement.";
          eval_expr_error = true;
          return false;
        }
      }
    }

    if (isKeyword) {
      next_lexeme =
          new Lexeme(Lexeme::type_literal, Lexeme::subtype_numeric, parmValue);
      pushActionFromLexeme(next_lexeme);
    } else {
      parm.setLexemeBOF();
      if (!eval_expression(&parm)) {
        return false;
      }
    }

    parm.clearLexemes();
  }

  return true;
}

bool Parser::eval_cmd_line(LexerLine* statement) {
  Lexeme* next_lexeme;
  ActionNode* action;
  LexerLine parm;
  int state = 0, sepCount = 0, parmCount = 0;
  bool startAsParm2 = false, mustPopAction = false;
  bool sepTime = false, isKeyword = false;
  string parmValue;

  parm.clearLexemes();

  while ((next_lexeme = statement->getNextLexeme())) {
    next_lexeme = coalesceSymbols(next_lexeme);

    switch (state) {
      case 0: {
        if (next_lexeme->isKeyword("INPUT")) {
          has_input = true;
          pushActionFromLexeme(next_lexeme);
          return eval_cmd_line_input(statement);
        } else if (next_lexeme->isKeyword("STEP")) {
          if (startAsParm2) {
            action = new ActionNode("TO_STEP");
            pushActionRoot(action);
          } else {
            action = new ActionNode(next_lexeme);
            pushActionRoot(action);
          }
          continue;
        } else if (next_lexeme->isSeparator("(")) {
          parmCount++;
          if (startAsParm2)
            state = 3;
          else
            state++;
          if (actionRoot->lexeme->value != "STEP" &&
              actionRoot->lexeme->value != "TO_STEP") {
            action = new ActionNode("COORD");
            if (startAsParm2) action->lexeme->name = "TO_COORD";
            action->lexeme->value = action->lexeme->name;
            pushActionRoot(action);
          }
          continue;
        } else if (next_lexeme->isOperator("-")) {
          startAsParm2 = true;
          continue;
        } else {
          error_message = "LINE without a valid complement.";
          eval_expr_error = true;
          return false;
        }
      } break;

      case 1: {
        if (next_lexeme->isSeparator("(")) {
          sepCount++;
        } else if (next_lexeme->isSeparator(")")) {
          if (sepCount)
            sepCount--;
          else {
            if (sepTime) {
              error_message = "Invalid parentheses syntax in LINE statement.";
              eval_expr_error = true;
              return false;
            }
            if (parm.getLexemeCount()) {
              parm.setLexemeBOF();
              if (!eval_expression(&parm)) {
                return false;
              }
              parm.clearLexemes();
              popActionRoot();
            }
            sepTime = true;
            continue;
          }
        } else if (next_lexeme->isSeparator(",") && sepCount == 0) {
          if (parm.getLexemeCount()) {
            parm.setLexemeBOF();
            if (!eval_expression(&parm)) {
              return false;
            }
            parm.clearLexemes();
          }
          continue;
        } else if (next_lexeme->isOperator("-") && sepTime) {
          state++;
          continue;
        }

      } break;

      case 2: {
        if (next_lexeme->isKeyword("STEP")) {
          action = new ActionNode("TO_STEP");
          pushActionRoot(action);
          continue;
        } else if (next_lexeme->isSeparator("(")) {
          state++;
          if (actionRoot) {
            if (actionRoot->lexeme->value != "TO_STEP") {
              action = new ActionNode("TO_COORD");
              pushActionRoot(action);
            }
          } else {
            error_message = "Invalid parentheses syntax in LINE statement.";
            eval_expr_error = true;
            return false;
          }
          continue;
        }

      } break;

      case 3: {
        if (next_lexeme->isSeparator("(")) {
          sepCount++;
        } else if (next_lexeme->isSeparator(")")) {
          if (sepCount)
            sepCount--;
          else {
            if (parmCount != 2 || mustPopAction) {
              error_message = "Invalid parentheses syntax in LINE statement.";
              eval_expr_error = true;
              return false;
            }
            mustPopAction = true;
            continue;
          }
        } else if (next_lexeme->isSeparator(",") && sepCount == 0) {
          parmCount++;

          if (parm.getLexemeCount()) {
            next_lexeme = parm.getFirstLexeme();

            isKeyword = false;

            if (parmCount >= 3 && !mustPopAction) {
              if (next_lexeme->type == Lexeme::type_identifier) {
                if (next_lexeme->value == "B") {
                  parmValue = "1";
                  isKeyword = true;
                } else if (next_lexeme->value == "BF") {
                  parmValue = "2";
                  isKeyword = true;
                }
              }
            }

            if (isKeyword) {
              next_lexeme = new Lexeme(Lexeme::type_literal,
                                       Lexeme::subtype_numeric, parmValue);
              pushActionFromLexeme(next_lexeme);
            } else {
              parm.setLexemeBOF();
              if (!eval_expression(&parm)) {
                return false;
              }
              if (mustPopAction) {
                popActionRoot();
                mustPopAction = false;
              }
            }

            parm.clearLexemes();

          } else {
            next_lexeme = lex_null;
            pushActionFromLexeme(next_lexeme);
          }

          continue;

        } else if (mustPopAction) {
          error_message = "Invalid syntax in LINE statement.";
          eval_expr_error = true;
          return false;
        }

      } break;
    }

    parm.addLexeme(next_lexeme);
  }

  if (parm.getLexemeCount()) {
    next_lexeme = parm.getFirstLexeme();

    isKeyword = false;

    if (parmCount >= 3) {
      if (next_lexeme->type == Lexeme::type_identifier) {
        if (next_lexeme->value == "B") {
          parmValue = "1";
          isKeyword = true;
        } else if (next_lexeme->value == "BF") {
          parmValue = "2";
          isKeyword = true;
        }
      } else if (next_lexeme->type == Lexeme::type_keyword ||
                 (next_lexeme->type == Lexeme::type_operator &&
                  next_lexeme->value != "+" && next_lexeme->value != "-")) {
        int operatorCode = gfxOperatorCode(next_lexeme);

        isKeyword = true;

        if (operatorCode >= 0) {
          parmValue = to_string(operatorCode);
        } else {
          error_message = "Invalid operator parameter in LINE statement.";
          eval_expr_error = true;
          return false;
        }
      }
    }

    if (isKeyword) {
      next_lexeme =
          new Lexeme(Lexeme::type_literal, Lexeme::subtype_numeric, parmValue);
      pushActionFromLexeme(next_lexeme);
    } else {
      parm.setLexemeBOF();
      if (!eval_expression(&parm)) {
        return false;
      }
    }

    parm.clearLexemes();
  }

  return true;
}

bool Parser::eval_cmd_circle(LexerLine* statement) {
  Lexeme* next_lexeme;
  ActionNode* action;
  LexerLine parm;
  int state = 0, sepCount = 0, parmCount = 0;
  bool mustPopAction = false;

  parm.clearLexemes();

  while ((next_lexeme = statement->getNextLexeme())) {
    next_lexeme = coalesceSymbols(next_lexeme);

    if (state == 0) {
      if (next_lexeme->isKeyword("STEP")) {
        action = new ActionNode(next_lexeme);
        pushActionRoot(action);
        continue;
      } else if (next_lexeme->isSeparator("(")) {
        state++;
        parmCount++;
        if (actionRoot->lexeme->value != "STEP") {
          action = new ActionNode("COORD");
          pushActionRoot(action);
        }
        continue;
      } else {
        error_message = "CIRCLE without a valid complement.";
        eval_expr_error = true;
        return false;
      }
    } else if (state == 1) {
      if (next_lexeme->isSeparator("(")) {
        sepCount++;
      } else if (next_lexeme->isSeparator(")")) {
        if (sepCount) {
          sepCount--;
        } else {
          mustPopAction = true;
          parmCount++;
          continue;
        }
      } else if (next_lexeme->isSeparator(",") && sepCount == 0) {
        if (parm.getLexemeCount()) {
          parm.setLexemeBOF();
          if (!eval_expression(&parm)) {
            return false;
          }
          parm.clearLexemes();
          if (mustPopAction) {
            popActionRoot();
            mustPopAction = false;
          } else
            parmCount++;
        } else {
          parmCount++;
          next_lexeme = lex_null;
          pushActionFromLexeme(next_lexeme);
        }
        continue;
      }
      parm.addLexeme(next_lexeme);
    }
  }

  if (parm.getLexemeCount()) {
    parm.setLexemeBOF();
    if (!eval_expression(&parm)) {
      return false;
    }

    parm.clearLexemes();
  }

  return true;
}

bool Parser::eval_cmd_paint(LexerLine* statement) {
  Lexeme* next_lexeme;
  ActionNode* action;
  LexerLine parm;
  int state = 0, sepCount = 0, parmCount = 0;
  bool mustPopAction = false;

  parm.clearLexemes();

  while ((next_lexeme = statement->getNextLexeme())) {
    next_lexeme = coalesceSymbols(next_lexeme);

    if (state == 0) {
      if (next_lexeme->isKeyword("STEP")) {
        action = new ActionNode(next_lexeme);
        pushActionRoot(action);
        continue;
      } else if (next_lexeme->isSeparator("(")) {
        state++;
        parmCount++;
        if (actionRoot->lexeme->value != "STEP") {
          action = new ActionNode("COORD");
          pushActionRoot(action);
        }
        continue;
      } else {
        error_message = "PAINT without a valid complement.";
        eval_expr_error = true;
        return false;
      }
    } else if (state == 1) {
      if (next_lexeme->isSeparator("(")) {
        sepCount++;
      } else if (next_lexeme->isSeparator(")")) {
        if (sepCount) {
          sepCount--;
        } else {
          mustPopAction = true;
          parmCount++;
          continue;
        }
      } else if (next_lexeme->isSeparator(",") && sepCount == 0) {
        if (parm.getLexemeCount()) {
          parm.setLexemeBOF();
          if (!eval_expression(&parm)) {
            return false;
          }
          parm.clearLexemes();
          if (mustPopAction) {
            popActionRoot();
            mustPopAction = false;
          } else
            parmCount++;
        } else {
          next_lexeme = lex_null;
          pushActionFromLexeme(next_lexeme);
        }
        continue;
      }
      parm.addLexeme(next_lexeme);
    }
  }

  if (parm.getLexemeCount()) {
    parm.setLexemeBOF();
    if (!eval_expression(&parm)) {
      return false;
    }

    parm.clearLexemes();
  }

  return true;
}

bool Parser::eval_cmd_copy(LexerLine* statement) {
  Lexeme* next_lexeme;
  ActionNode* action;
  LexerLine parm;
  int state = 0, sepCount = 0, parmCount = 0;
  bool isKeyword = false;
  string parmValue;

  parm.clearLexemes();

  while ((next_lexeme = statement->getNextLexeme())) {
    next_lexeme = coalesceSymbols(next_lexeme);

    switch (state) {
      case 0: {
        if (next_lexeme->isKeyword("SCREEN")) {
          pushActionFromLexeme(next_lexeme);
          return eval_cmd_generic(statement);
        }
        if (next_lexeme->isSeparator("(")) {
          parmCount++;
          state = 1;

          action = new ActionNode("COORD");
          pushActionRoot(action);

          continue;

        } else {
          state = 9;
        }
      } break;

      case 1: {
        if (next_lexeme->isSeparator("(")) {
          sepCount++;
        } else if (next_lexeme->isSeparator(")")) {
          if (sepCount)
            sepCount--;
          else {
            if (parm.getLexemeCount()) {
              parm.setLexemeBOF();
              if (!eval_expression(&parm)) {
                return false;
              }
              parm.clearLexemes();
            } else {
              next_lexeme = lex_null;
              pushActionFromLexeme(next_lexeme);
            }
            popActionRoot();
            state = 10;
            continue;
          }
        } else if (next_lexeme->isSeparator(",") && sepCount == 0) {
          if (parm.getLexemeCount()) {
            parm.setLexemeBOF();
            if (!eval_expression(&parm)) {
              return false;
            }
            parm.clearLexemes();
          } else {
            next_lexeme = lex_null;
            pushActionFromLexeme(next_lexeme);
          }
          continue;
        }

      } break;

      case 2: {
        if (next_lexeme->isKeyword("STEP")) {
          action = new ActionNode("TO_STEP");
          pushActionRoot(action);
          continue;
        } else if (next_lexeme->isSeparator("(")) {
          parmCount++;
          state = 3;
          if (actionRoot->lexeme->value != "TO_STEP") {
            action = new ActionNode("TO_COORD");
            pushActionRoot(action);
          }
          continue;
        }

      } break;

      case 3: {
        if (next_lexeme->isSeparator("(")) {
          sepCount++;
        } else if (next_lexeme->isSeparator(")")) {
          if (sepCount)
            sepCount--;
          else {
            if (parm.getLexemeCount()) {
              parm.setLexemeBOF();
              if (!eval_expression(&parm)) {
                return false;
              }
              parm.clearLexemes();
            } else {
              next_lexeme = lex_null;
              pushActionFromLexeme(next_lexeme);
            }

            popActionRoot();

            parmCount++;
            state = 4;

            continue;
          }
        } else if (next_lexeme->isSeparator(",") && sepCount == 0) {
          if (parm.getLexemeCount()) {
            parm.setLexemeBOF();
            if (!eval_expression(&parm)) {
              return false;
            }
            parm.clearLexemes();
          } else {
            next_lexeme = lex_null;
            pushActionFromLexeme(next_lexeme);
          }

          continue;
        }

      } break;

      case 4: {
        if (next_lexeme->isKeyword("TO")) {
          if (parm.getLexemeCount()) {
            parm.setLexemeBOF();
            if (!eval_expression(&parm)) {
              return false;
            }
            parm.clearLexemes();
          } else {
            next_lexeme = lex_null;
            pushActionFromLexeme(next_lexeme);
          }

          parmCount++;
          state = 5;

          continue;

        } else if (next_lexeme->isSeparator(",")) {
          continue;
        }

      } break;

      case 5: {
        parmCount++;
        if (next_lexeme->isSeparator("(")) {
          state = 6;

          action = new ActionNode("TO_DEST");
          pushActionRoot(action);

          continue;
        } else {
          state = 8;
        }

      } break;

      case 6: {
        if (next_lexeme->isSeparator("(")) {
          sepCount++;
        } else if (next_lexeme->isSeparator(")")) {
          if (sepCount)
            sepCount--;
          else {
            if (parm.getLexemeCount()) {
              parm.setLexemeBOF();
              if (!eval_expression(&parm)) {
                return false;
              }
              parm.clearLexemes();
            } else {
              next_lexeme = lex_null;
              pushActionFromLexeme(next_lexeme);
            }

            popActionRoot();

            state = 7;

            continue;
          }
        } else if (next_lexeme->isSeparator(",") && sepCount == 0) {
          if (parm.getLexemeCount()) {
            parm.setLexemeBOF();
            if (!eval_expression(&parm)) {
              return false;
            }
            parm.clearLexemes();
          } else {
            next_lexeme = lex_null;
            pushActionFromLexeme(next_lexeme);
          }

          continue;
        }

      } break;

      case 7: {
        if (next_lexeme->isSeparator(",")) {
          state = 8;

          continue;
        }

      } break;

      case 8: {
        if (next_lexeme->isSeparator(",")) {
          if (parm.getLexemeCount()) {
            parm.setLexemeBOF();
            if (!eval_expression(&parm)) {
              return false;
            }
            parm.clearLexemes();
          } else {
            next_lexeme = lex_null;
            pushActionFromLexeme(next_lexeme);
          }

          parmCount++;

          continue;
        }

      } break;

      case 9: {
        if (next_lexeme->isKeyword("TO")) {
          if (parm.getLexemeCount()) {
            parm.setLexemeBOF();
            if (!eval_expression(&parm)) {
              return false;
            }
            parm.clearLexemes();
          } else {
            next_lexeme = lex_null;
            pushActionFromLexeme(next_lexeme);
          }

          if (parmCount == 0) {
            pushActionFromLexeme(lex_null);
          }

          parmCount++;
          state = 5;

          continue;

        } else if (next_lexeme->isSeparator(",")) {
          if (parm.getLexemeCount()) {
            parm.setLexemeBOF();
            if (!eval_expression(&parm)) {
              return false;
            }
            parm.clearLexemes();
          } else {
            next_lexeme = lex_null;
            pushActionFromLexeme(next_lexeme);
          }

          continue;
        }

      } break;

      case 10: {
        if (next_lexeme->isOperator("-")) {
          state = 2;
          continue;
        } else {
          error_message = "Invalid syntax on COPY statement";
          return false;
        }
      } break;
    }

    parm.addLexeme(next_lexeme);
  }

  if (parm.getLexemeCount()) {
    next_lexeme = parm.getFirstLexeme();

    if (parmCount >= 5) {
      if ((next_lexeme->type == Lexeme::type_keyword &&
           next_lexeme->subtype != Lexeme::subtype_function) ||
          next_lexeme->type == Lexeme::type_operator) {
        int operatorCode = gfxOperatorCode(next_lexeme);

        isKeyword = true;

        if (operatorCode >= 0) {
          parmValue = to_string(operatorCode);
        } else {
          error_message = "Invalid operator parameter in COPY statement.";
          eval_expr_error = true;
          return false;
        }
      }
    }

    if (isKeyword) {
      next_lexeme =
          new Lexeme(Lexeme::type_literal, Lexeme::subtype_numeric, parmValue);
      pushActionFromLexeme(next_lexeme);
    } else {
      parm.setLexemeBOF();
      if (!eval_expression(&parm)) {
        return false;
      }
    }

    parm.clearLexemes();
  }

  return true;
}

bool Parser::eval_cmd_set(LexerLine* statement) {
  Lexeme* next_lexeme;
  ActionNode* action;
  bool result = false;

  if ((next_lexeme = statement->getNextLexeme())) {
    coalesceSymbols(next_lexeme);

    next_lexeme = statement->getCurrentLexeme();
    action = new ActionNode(next_lexeme);
    pushActionRoot(action);

    if (next_lexeme->type == Lexeme::type_keyword) {
      if (next_lexeme->value == "BEEP" || next_lexeme->value == "DATE" ||
          next_lexeme->value == "PAGE" || next_lexeme->value == "PASSWORD" ||
          next_lexeme->value == "PROMPT" || next_lexeme->value == "SCREEN" ||
          next_lexeme->value == "SCROLL" || next_lexeme->value == "TIME" ||
          next_lexeme->value == "TITLE" || next_lexeme->value == "VIDEO" ||
          next_lexeme->value == "FONT") {
        result = eval_cmd_generic(statement);
      } else if (next_lexeme->value == "ADJUST") {
        result = eval_cmd_set_adjust(statement);
      } else if (next_lexeme->value == "TILE") {
        result = eval_cmd_set_tile(statement);
      } else if (next_lexeme->value == "SPRITE") {
        result = eval_cmd_set_sprite(statement);
      }
    }

    popActionRoot();
  }

  return result;
}

bool Parser::eval_cmd_set_adjust(LexerLine* statement) {
  Lexeme* next_lexeme;
  // ActionNode *action;
  LexerLine parm;
  int state = 0, sepCount = 0, parmCount = 0;
  bool mustPopAction = false;
  string parmValue;

  parm.clearLexemes();

  while ((next_lexeme = statement->getNextLexeme())) {
    next_lexeme = coalesceSymbols(next_lexeme);

    if (state == 0) {
      if (next_lexeme->isSeparator("(")) {
        state++;
        parmCount++;
        continue;
      } else {
        error_message = "SET ADJUST without a valid complement.";
        eval_expr_error = true;
        return false;
      }
    } else if (state == 1) {
      if (next_lexeme->isSeparator("(")) {
        sepCount++;
      } else if (next_lexeme->isSeparator(")")) {
        if (sepCount) {
          sepCount--;
        } else {
          mustPopAction = true;
          parmCount++;
          continue;
        }
      } else if (next_lexeme->isSeparator(",") && sepCount == 0) {
        if (parm.getLexemeCount()) {
          parm.setLexemeBOF();
          if (!eval_expression(&parm)) {
            return false;
          }
          parm.clearLexemes();
          if (mustPopAction) {
            popActionRoot();
            mustPopAction = false;
          } else
            parmCount++;
        } else {
          parmCount++;
          next_lexeme = lex_null;
          pushActionFromLexeme(next_lexeme);
        }
        continue;
      }
      parm.addLexeme(next_lexeme);
    }
  }

  if (parm.getLexemeCount()) {
    parm.getFirstLexeme();

    parm.setLexemeBOF();
    if (!eval_expression(&parm)) {
      return false;
    }

    parm.clearLexemes();
  }

  return true;
}

bool Parser::eval_cmd_set_tile(LexerLine* statement) {
  Lexeme* next_lexeme;
  ActionNode* action;
  bool result = false;

  if ((next_lexeme = statement->getNextLexeme())) {
    coalesceSymbols(next_lexeme);

    next_lexeme = statement->getCurrentLexeme();
    action = new ActionNode(next_lexeme);
    pushActionRoot(action);

    if (next_lexeme->type == Lexeme::type_keyword) {
      if (next_lexeme->value == "COLOR" || next_lexeme->value == "PATTERN" ||
          next_lexeme->value == "FLIP" || next_lexeme->value == "ROTATE") {
        result = eval_cmd_set_tile_colpat(statement);
      } else if (next_lexeme->value == "ON" || next_lexeme->value == "OFF") {
        result = true;
      }
    }

    popActionRoot();
  }

  return result;
}

bool Parser::eval_cmd_set_tile_colpat(LexerLine* statement) {
  Lexeme* next_lexeme;
  ActionNode* act_coord;
  LexerLine parm;
  int state = 1, sepCount = 0;
  bool hasArrayParm = false;

  parm.clearLexemes();

  while ((next_lexeme = statement->getNextLexeme())) {
    next_lexeme = coalesceSymbols(next_lexeme);

    switch (state) {
      case 0: {
        if (next_lexeme->isSeparator("(")) {
          state = 2;
          sepCount = 0;
          act_coord = new ActionNode("ARRAY");
          pushActionRoot(act_coord);
          hasArrayParm = true;
          continue;
        } else {
          state = 1;
        }
      }

      case 1: {
        if (next_lexeme->isSeparator(",")) {
          state = 0;
          if (parm.getLexemeCount()) {
            parm.setLexemeBOF();
            if (!eval_expression(&parm)) {
              return false;
            }
            parm.clearLexemes();
          } else {
            pushActionFromLexeme(lex_null);
          }
          if (hasArrayParm) {
            popActionRoot();
            hasArrayParm = false;
          }
          continue;
        }
      } break;

      case 2: {
        if (next_lexeme->isSeparator("(")) {
          sepCount++;
        } else if (next_lexeme->isSeparator(")")) {
          if (sepCount) {
            sepCount--;
          } else {
            state = 1;
            continue;
          }
        } else if (next_lexeme->isSeparator(",") && sepCount == 0) {
          if (parm.getLexemeCount()) {
            parm.setLexemeBOF();
            if (!eval_expression(&parm)) {
              return false;
            }
            parm.clearLexemes();
          } else {
            pushActionFromLexeme(lex_null);
          }
          continue;
        }

      } break;
    }

    parm.addLexeme(next_lexeme);
  }

  if (parm.getLexemeCount()) {
    parm.setLexemeBOF();
    if (!eval_expression(&parm)) {
      return false;
    }

    parm.clearLexemes();

    if (hasArrayParm) {
      popActionRoot();
    }
  }

  return true;
}

bool Parser::eval_cmd_set_sprite(LexerLine* statement) {
  Lexeme* next_lexeme;
  ActionNode* action;
  bool result = false;

  if ((next_lexeme = statement->getNextLexeme())) {
    coalesceSymbols(next_lexeme);

    next_lexeme = statement->getCurrentLexeme();
    action = new ActionNode(next_lexeme);
    pushActionRoot(action);

    if (next_lexeme->type == Lexeme::type_keyword) {
      if (next_lexeme->value == "COLOR" || next_lexeme->value == "PATTERN" ||
          next_lexeme->value == "FLIP" || next_lexeme->value == "ROTATE") {
        result = eval_cmd_set_sprite_colpattra(statement);
      }
    }

    popActionRoot();
  }

  return result;
}

bool Parser::eval_cmd_set_sprite_colpattra(LexerLine* statement) {
  Lexeme* next_lexeme;
  ActionNode* act_coord;
  LexerLine parm;
  int state = 1, sepCount = 0;
  bool hasArrayParm = false;

  parm.clearLexemes();

  while ((next_lexeme = statement->getNextLexeme())) {
    next_lexeme = coalesceSymbols(next_lexeme);

    switch (state) {
      case 0: {
        if (next_lexeme->isSeparator("(")) {
          state = 2;
          sepCount = 0;
          act_coord = new ActionNode("ARRAY");
          pushActionRoot(act_coord);
          hasArrayParm = true;
          continue;
        } else {
          state = 1;
        }
      }

      case 1: {
        if (next_lexeme->isSeparator(",")) {
          state = 0;
          if (parm.getLexemeCount()) {
            parm.setLexemeBOF();
            if (!eval_expression(&parm)) {
              return false;
            }
            parm.clearLexemes();
          } else {
            pushActionFromLexeme(lex_null);
          }
          if (hasArrayParm) {
            popActionRoot();
            hasArrayParm = false;
          }
          continue;
        }
      } break;

      case 2: {
        if (next_lexeme->isSeparator("(")) {
          sepCount++;
        } else if (next_lexeme->isSeparator(")")) {
          if (sepCount) {
            sepCount--;
          } else {
            state = 1;
            continue;
          }
        } else if (next_lexeme->isSeparator(",") && sepCount == 0) {
          if (parm.getLexemeCount()) {
            parm.setLexemeBOF();
            if (!eval_expression(&parm)) {
              return false;
            }
            parm.clearLexemes();
          } else {
            pushActionFromLexeme(lex_null);
          }
          continue;
        }

      } break;
    }

    parm.addLexeme(next_lexeme);
  }

  if (parm.getLexemeCount()) {
    parm.setLexemeBOF();
    if (!eval_expression(&parm)) {
      return false;
    }

    parm.clearLexemes();

    if (hasArrayParm) {
      popActionRoot();
    }
  }

  return true;
}

bool Parser::eval_cmd_get(LexerLine* statement) {
  Lexeme* next_lexeme;
  ActionNode* action;
  bool result = false;

  if ((next_lexeme = statement->getNextLexeme())) {
    coalesceSymbols(next_lexeme);

    next_lexeme = statement->getCurrentLexeme();
    action = new ActionNode(next_lexeme);
    pushActionRoot(action);

    if (next_lexeme->type == Lexeme::type_keyword) {
      if (next_lexeme->value == "DATE" || next_lexeme->value == "TIME") {
        result = eval_cmd_generic(statement);
      } else if (next_lexeme->value == "TILE") {
        result = eval_cmd_get_tile(statement);
      } else if (next_lexeme->value == "SPRITE") {
        result = eval_cmd_get_sprite(statement);
      }
    }

    popActionRoot();
  }

  return result;
}

bool Parser::eval_cmd_get_tile(LexerLine* statement) {
  return eval_cmd_get_sprite(statement);
}

bool Parser::eval_cmd_get_sprite(LexerLine* statement) {
  Lexeme* next_lexeme;
  ActionNode* action;
  bool result = false;

  if ((next_lexeme = statement->getNextLexeme())) {
    coalesceSymbols(next_lexeme);

    next_lexeme = statement->getCurrentLexeme();
    action = new ActionNode(next_lexeme);
    pushActionRoot(action);

    if (next_lexeme->type == Lexeme::type_keyword) {
      if (next_lexeme->value == "COLOR" || next_lexeme->value == "PATTERN") {
        result = eval_cmd_set_sprite_colpattra(statement);
      }
    }

    popActionRoot();
  }

  return result;
}

bool Parser::eval_cmd_screen(LexerLine* statement) {
  Lexeme* next_lexeme;

  if ((next_lexeme = statement->getNextLexeme())) {
    if (next_lexeme->type == Lexeme::type_keyword) {
      if (next_lexeme->value == "COPY") {
        return eval_cmd_screen_copy(statement);
      } else if (next_lexeme->value == "PASTE") {
        return eval_cmd_screen_paste(statement);
      } else if (next_lexeme->value == "SCROLL") {
        return eval_cmd_screen_scroll(statement);
      } else if (next_lexeme->value == "LOAD") {
        return eval_cmd_screen_load(statement);
      } else if (next_lexeme->value == "ON") {
        return eval_cmd_screen_on(statement);
      } else if (next_lexeme->value == "OFF") {
        return eval_cmd_screen_off(statement);
      }
    }

    statement->getPreviousLexeme();

    return eval_cmd_generic(statement);
  }

  return false;
}

bool Parser::eval_cmd_screen_copy(LexerLine* statement) {
  Lexeme* next_lexeme = statement->getCurrentLexeme();
  LexerLine parm;
  int state = 0;
  bool result = false;

  pushActionFromLexeme(next_lexeme);

  parm.clearLexemes();

  while ((next_lexeme = statement->getNextLexeme())) {
    next_lexeme = coalesceSymbols(next_lexeme);

    switch (state) {
      case 0: {
        if (next_lexeme->isKeyword("TO")) {
          state = 1;
          continue;
        } else {
          return false;
        }
      } break;

      case 1: {
        if (next_lexeme->isKeyword("SCROLL")) {
          if (parm.getLexemeCount()) {
            parm.setLexemeBOF();
            if (!eval_expression(&parm)) {
              return false;
            }
            parm.clearLexemes();
          } else {
            return false;
          }
          state = 2;
          continue;
        }
      } break;
    }

    parm.addLexeme(next_lexeme);
  }

  if (parm.getLexemeCount()) {
    parm.setLexemeBOF();
    if (!eval_expression(&parm)) {
      return false;
    }

    result = true;
  }

  popActionRoot();

  return result;
}

bool Parser::eval_cmd_screen_paste(LexerLine* statement) {
  Lexeme* next_lexeme = statement->getCurrentLexeme();
  bool result = false;

  pushActionFromLexeme(next_lexeme);

  if ((next_lexeme = statement->getNextLexeme())) {
    if (next_lexeme->isKeyword("FROM")) {
      result = eval_cmd_generic(statement);
    }
  }

  popActionRoot();

  return result;
}

bool Parser::eval_cmd_screen_scroll(LexerLine* statement) {
  Lexeme* next_lexeme = statement->getCurrentLexeme();
  bool result;
  pushActionFromLexeme(next_lexeme);
  result = eval_cmd_generic(statement);
  popActionRoot();
  return result;
}

bool Parser::eval_cmd_screen_load(LexerLine* statement) {
  Lexeme* next_lexeme = statement->getCurrentLexeme();
  bool result;
  pushActionFromLexeme(next_lexeme);
  result = eval_cmd_generic(statement);
  popActionRoot();
  return result;
}

bool Parser::eval_cmd_screen_on(LexerLine* statement) {
  Lexeme* next_lexeme = statement->getCurrentLexeme();
  pushActionFromLexeme(next_lexeme);
  popActionRoot();
  return true;
}

bool Parser::eval_cmd_screen_off(LexerLine* statement) {
  return eval_cmd_screen_on(statement);
}

bool Parser::eval_cmd_on(LexerLine* statement) {
  Lexeme* next_lexeme;

  if ((next_lexeme = statement->getNextLexeme())) {
    next_lexeme = coalesceSymbols(next_lexeme);

    if (next_lexeme->isKeyword("ERROR")) {
      return eval_cmd_on_error(statement);
    } else if (next_lexeme->isKeyword("INTERVAL")) {
      has_traps = true;
      return eval_cmd_on_interval(statement);
    } else if (next_lexeme->isKeyword("KEY")) {
      has_traps = true;
      return eval_cmd_on_key(statement);
    } else if (next_lexeme->isKeyword("SPRITE")) {
      has_traps = true;
      return eval_cmd_on_sprite(statement);
    } else if (next_lexeme->isKeyword("STOP")) {
      has_traps = true;
      return eval_cmd_on_stop(statement);
    } else if (next_lexeme->isKeyword("STRIG")) {
      has_traps = true;
      return eval_cmd_on_strig(statement);
    } else {
      return eval_cmd_on_goto_gosub(statement);
    }
  }

  return false;
}

bool Parser::eval_cmd_on_goto_gosub(LexerLine* statement) {
  Lexeme* next_lexeme;
  ActionNode *action, *action_index;
  LexerLine parm;
  int state = 0;
  bool next_is_sep = false;

  next_lexeme = statement->getPreviousLexeme();
  if (!next_lexeme) return false;

  action_index = new ActionNode(lex_index);

  while ((next_lexeme = statement->getNextLexeme())) {
    switch (state) {
      case 0: {
        if (next_lexeme->isKeyword("GOSUB") || next_lexeme->isKeyword("GOTO")) {
          if (!parm.getLexemeCount()) return false;

          pushActionRoot(action_index);

          parm.setLexemeBOF();
          if (!eval_expression(&parm)) {
            return false;
          }
          parm.clearLexemes();

          popActionRoot();

          action = new ActionNode(next_lexeme);
          pushActionRoot(action);

          state++;

          continue;
        }

      } break;

      case 1: {
        if (next_lexeme->type == Lexeme::type_literal) {
          if (next_is_sep) return false;

          pushActionFromLexeme(next_lexeme);

          next_is_sep = true;

          continue;

        } else if (next_lexeme->isSeparator(",")) {
          if (!next_is_sep) {
            pushActionFromLexeme(lex_null);
          }

          next_is_sep = false;

          continue;

        } else
          return false;

      } break;
    }

    parm.addLexeme(next_lexeme);
  }

  if (state) {
    popActionRoot();
  }

  return state;
}

bool Parser::eval_cmd_on_error(LexerLine* statement) {
  return false;
}

bool Parser::eval_cmd_on_interval(LexerLine* statement) {
  Lexeme* next_lexeme;
  ActionNode *action, *action_index;
  LexerLine parm;
  int state = 0;
  bool first = true;

  next_lexeme = statement->getCurrentLexeme();
  if (!next_lexeme) return false;

  action = new ActionNode(next_lexeme);
  pushActionRoot(action);

  action_index = new ActionNode(lex_index);

  while ((next_lexeme = statement->getNextLexeme())) {
    switch (state) {
      case 0: {
        if (first) {
          first = false;
          if (next_lexeme->isOperator("=")) {
            continue;
          }
        }

        if (next_lexeme->isKeyword("GOSUB")) {
          if (!parm.getLexemeCount()) return false;

          pushActionRoot(action_index);

          parm.setLexemeBOF();
          if (!eval_expression(&parm)) {
            return false;
          }
          parm.clearLexemes();

          popActionRoot();

          action = new ActionNode(next_lexeme);
          pushActionRoot(action);

          state++;

          continue;
        }

      } break;

      case 1: {
        if (next_lexeme->type == Lexeme::type_literal) {
          pushActionFromLexeme(next_lexeme);

          state++;

          continue;

        } else
          return false;

      } break;

      case 2: {
        return false;
      } break;
    }

    parm.addLexeme(next_lexeme);
  }

  if (state) {
    popActionRoot();
  }

  popActionRoot();

  return state;
}

bool Parser::eval_cmd_on_key(LexerLine* statement) {
  Lexeme* next_lexeme;
  ActionNode* action;
  LexerLine parm;
  int state = 0;
  bool next_is_sep = false;

  next_lexeme = statement->getCurrentLexeme();
  if (!next_lexeme) return false;
  action = new ActionNode(next_lexeme);
  pushActionRoot(action);

  while ((next_lexeme = statement->getNextLexeme())) {
    switch (state) {
      case 0: {
        if (next_lexeme->isKeyword("GOSUB")) {
          action = new ActionNode(next_lexeme);
          pushActionRoot(action);

          state++;

          continue;

        } else
          return false;

      } break;

      case 1: {
        if (next_lexeme->type == Lexeme::type_literal) {
          if (next_is_sep) return false;

          pushActionFromLexeme(next_lexeme);

          next_is_sep = true;

          continue;

        } else if (next_lexeme->isSeparator(",")) {
          if (!next_is_sep) {
            pushActionFromLexeme(lex_null);
          }

          next_is_sep = false;

          continue;

        } else
          return false;

      } break;
    }

    parm.addLexeme(next_lexeme);
  }

  if (state) {
    popActionRoot();
  }

  popActionRoot();

  return state;
}

bool Parser::eval_cmd_on_sprite(LexerLine* statement) {
  Lexeme* next_lexeme;
  ActionNode* action;
  LexerLine parm;
  int state = 0;
  bool next_is_sep = false;

  next_lexeme = statement->getCurrentLexeme();
  if (!next_lexeme) return false;
  action = new ActionNode(next_lexeme);
  pushActionRoot(action);

  while ((next_lexeme = statement->getNextLexeme())) {
    switch (state) {
      case 0: {
        if (next_lexeme->isKeyword("GOSUB")) {
          action = new ActionNode(next_lexeme);
          pushActionRoot(action);

          state++;

          continue;

        } else
          return false;

      } break;

      case 1: {
        if (next_lexeme->type == Lexeme::type_literal) {
          if (next_is_sep) return false;

          pushActionFromLexeme(next_lexeme);

          next_is_sep = true;

          continue;

        } else if (next_lexeme->isSeparator(",")) {
          if (!next_is_sep) {
            pushActionFromLexeme(lex_null);
          }

          next_is_sep = false;

          continue;

        } else
          return false;

      } break;
    }

    parm.addLexeme(next_lexeme);
  }

  if (state) {
    popActionRoot();
  }

  popActionRoot();

  return state;
}

bool Parser::eval_cmd_on_stop(LexerLine* statement) {
  Lexeme* next_lexeme;
  ActionNode* action;
  LexerLine parm;
  int state = 0;
  bool next_is_sep = false;

  next_lexeme = statement->getCurrentLexeme();
  if (!next_lexeme) return false;
  action = new ActionNode(next_lexeme);
  pushActionRoot(action);

  while ((next_lexeme = statement->getNextLexeme())) {
    switch (state) {
      case 0: {
        if (next_lexeme->isKeyword("GOSUB")) {
          action = new ActionNode(next_lexeme);
          pushActionRoot(action);

          state++;

          continue;

        } else
          return false;

      } break;

      case 1: {
        if (next_lexeme->type == Lexeme::type_literal) {
          if (next_is_sep) return false;

          pushActionFromLexeme(next_lexeme);

          next_is_sep = true;

          continue;

        } else if (next_lexeme->isSeparator(",")) {
          if (!next_is_sep) {
            pushActionFromLexeme(lex_null);
          }

          next_is_sep = false;

          continue;

        } else
          return false;

      } break;
    }

    parm.addLexeme(next_lexeme);
  }

  if (state) {
    popActionRoot();
  }

  popActionRoot();

  return state;
}

bool Parser::eval_cmd_on_strig(LexerLine* statement) {
  Lexeme* next_lexeme;
  ActionNode* action;
  LexerLine parm;
  int state = 0;
  bool next_is_sep = false;

  next_lexeme = statement->getCurrentLexeme();
  if (!next_lexeme) return false;
  action = new ActionNode(next_lexeme);
  pushActionRoot(action);

  while ((next_lexeme = statement->getNextLexeme())) {
    switch (state) {
      case 0: {
        if (next_lexeme->isKeyword("GOSUB")) {
          action = new ActionNode(next_lexeme);
          pushActionRoot(action);

          state++;

          continue;

        } else
          return false;

      } break;

      case 1: {
        if (next_lexeme->type == Lexeme::type_literal) {
          if (next_is_sep) return false;

          pushActionFromLexeme(next_lexeme);

          next_is_sep = true;

          continue;

        } else if (next_lexeme->isSeparator(",")) {
          if (!next_is_sep) {
            pushActionFromLexeme(lex_null);
          }

          next_is_sep = false;

          continue;

        } else
          return false;

      } break;
    }

    parm.addLexeme(next_lexeme);
  }

  if (state) {
    popActionRoot();
  }

  popActionRoot();

  return state;
}

bool Parser::eval_cmd_interval(LexerLine* statement) {
  Lexeme* next_lexeme;

  if ((next_lexeme = statement->getNextLexeme())) {
    if (next_lexeme->type == Lexeme::type_keyword) {
      if (next_lexeme->value == "ON" || next_lexeme->value == "OFF" ||
          next_lexeme->value == "STOP") {
        pushActionFromLexeme(next_lexeme);

        return true;
      }
    }
  }

  return false;
}

bool Parser::eval_cmd_stop(LexerLine* statement) {
  Lexeme* next_lexeme;

  if ((next_lexeme = statement->getNextLexeme())) {
    if (next_lexeme->type == Lexeme::type_keyword) {
      if (next_lexeme->value == "ON" || next_lexeme->value == "OFF" ||
          next_lexeme->value == "STOP") {
        pushActionFromLexeme(next_lexeme);

      } else
        return false;

    } else
      return false;
  }

  return true;
}

bool Parser::eval_cmd_call(LexerLine* statement) {
  Lexeme* next_lexeme;

  while ((next_lexeme = statement->getNextLexeme())) {
    pushActionFromLexeme(next_lexeme);
  }

  return true;
}

bool Parser::eval_cmd_cmd(LexerLine* statement) {
  ActionNode* action;
  Lexeme* lexeme = statement->getNextLexeme();

  if (!lexeme) return false;

  action = new ActionNode(lexeme);
  pushActionRoot(action);

  if (lexeme->value == "WRTFNT" || lexeme->value == "SETFNT" ||
      lexeme->value == "WRTCHR")
    has_font = true;

  if (lexeme->value == "PT3LOAD" || lexeme->value == "PT3PLAY") has_pt3 = true;

  if (lexeme->value == "PLYLOAD" || lexeme->value == "PLYPLAY" ||
      lexeme->value == "PLYSONG")
    has_akm = true;

  if (lexeme->value == "MTF") has_mtf = true;

  if (lexeme->value == "RESTORE") has_resource_restore = true;

  if (!eval_cmd_generic(statement)) return false;

  popActionRoot();

  return true;
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
  if (actionRoot) {
    actionRoot->actions.push_back(action);
    actionStack.push(actionRoot);
  } else {
    while (!actionStack.empty()) actionStack.pop();
    tag->actions.push_back(action);
  }
  actionRoot = action;
}

void Parser::popActionRoot() {
  if (actionStack.empty()) {
    actionRoot = 0;
  } else {
    actionRoot = actionStack.top();
    actionStack.pop();
  }
}

void ActionNode::print() {
  ActionNode* action;
  unsigned int i;

  if (actions.size()) {
    indent += 2;

    printf("%*s(\n", indent, "");

    for (i = 0; i < actions.size(); i++) {
      action = actions[i];
      if (action) {
        action->indent = indent;
        action->print();
      }
    }

    printf("%*s) ", indent, "");
    printf("Action %s\n", lexeme->value.c_str());

    indent -= 2;

  } else {
    if (lexeme->type == Lexeme::type_keyword) {
      printf("%*s", indent + 2, "");
      printf("Action %s\n", lexeme->value.c_str());
    } else {
      lexeme->indent = indent;
      lexeme->print();
    }
  }
}

void TagNode::print() {
  ActionNode* action;
  unsigned int i;

  printf("Tag %s\n", name.c_str());

  for (i = 0; i < actions.size(); i++) {
    action = actions[i];
    if (action) action->print();
  }
}

void Parser::print() {
  TagNode* tag;
  for (unsigned int i = 0; i < tags.size(); i++) {
    tag = tags[i];
    if (tag) tag->print();
  }
}

void Parser::error() {
  if (error_line) error_line->print();
  if (error_message.size() > 0) printf("%s\n", error_message.c_str());
}

/***
 * @name ActionNode class code
 */

ActionNode::ActionNode() {
  create((Lexeme*)0);
}

ActionNode::ActionNode(string name) {
  create(new Lexeme(Lexeme::type_keyword, Lexeme::subtype_any, name));
}

ActionNode::ActionNode(Lexeme* plexeme) {
  create(plexeme);
}

void ActionNode::create(Lexeme* plexeme) {
  lexeme = plexeme;
  indent = 0;
  subtype = Lexeme::subtype_unknown;
  actions.clear();
}
