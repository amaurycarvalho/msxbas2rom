# Tasks – Current Sprint Backlog

## To do list

- [x] Write user stories in separate files at user-stories/ for lexer (src/lex.cpp), parser (src/parse.cpp), compiler (src/compiler.cpp), builder (rom.cpp and resources.cpp) and cli (src/main.cpp) writing test scenarios for each one (see ADR-001);
- [x] Optimize the project Makefile to consider src/ and include/ dependencies;
- [x] Rename test/ to tests/, adjusting the related .github/workflows/test.yml and .vscode/tasks.json (and any other reference on any file to the old folder) according the changes;
- [x] Write unit tests on tests/unit/ for lexer, parser, compiler and builder refactoring test.cpp and test_resources, adjusting the related Makefile and .github/workflows/test.yml according the changes;
- [x] Move all .cpp and .h files from src/ and include/ to src/cli/, src/application/, src/domain/ and src/infrastructure/ according by each file code objective (dont delete any file or modify its contents in this step, ask me if any doubt). Next, adjust the related Makefile, asm/Makefile and unit testings (tests/) according the changes. Finally, delete include/ folder if empty;
- [x] At src/application/lexer, rename lex.cpp to lexer.cpp and lex.h to lexer.h. Next, at src/application/parser, rename parse.cpp to parser.cpp and parse.h to parser.h. Finally, adjust the related Makefile and unit testings according the changes;
- [x] Create tests/unit/src/ folder. Next, move tests/unit/\*.cpp files to tests/unit/src/. Finally, adjust the related Makefile according the changes;
- [x] Refactor src/domain/symbols/ files to one file per class practice (do not change any code), adjusting the related Makefile and unit testings according the changes;
- [x] Move src/domain/symbols/symbol_manager.\* to src/application/symbols/. Next, refactor it implementing a Strategy pattern (tell me if another pattern applies better) with minimum code change. The individual strategy files must be saved to src/application/symbols/strategies/ folder. Finally, adjust the related Makefile and unit testings according the changes;
- [x] Move asm/ folder to src/infrastructure/kernel/asm/, adjusting the related Makefile and .vscode/tasks.json according the changes. Finally, check for more related dependencies;
- [x] Refactor src/application/lexer/ files to one file per class practice (do not change any code), adjusting the related Makefile and unit testings according the changes;
- [x] Refactor src/application/lexer/lexer_line implementing a State pattern instanciated by a factory with minimum code change. The individual states files must be saved to src/application/lexer/states/ folder. Finally, adjust the related Makefile and unit testings according the changes;
- [x] Refactor src/application/parser/ files to one file per class practice (do not change any code), adjusting the related Makefile and unit testings according the changes;
- [x] Refactor src/application/parser/ implementing a Strategy pattern instanciated by a factory with minimum code change. The individual strategy files must be saved to src/application/parser/strategies/ folder. Finally, adjust the related Makefile and unit testings according the changes;
- [x] Refactor src/application/compiler/ files to one file per class practice (do not change any code), adjusting the related Makefile and unit testings according the changes;
- [x] Propose a plan to refactor step by step src/application/compiler/ implementing a Strategy pattern in a similar way to src/application/parser (helpers and strategies folders) and ask for approval;
- [x] Refactor src/application/builder/ files to one file per class practice (do not change any code), adjusting the related Makefile and unit testings according the changes;
- [ ] Propose a plan to refactor step by step evalFunction method of CompilerExpressionEvaluator implementing a Strategy pattern at src/application/compiler/functions folder and ask for approval;
- [ ] Inspect src/ and tests/ searching for opportunities to apply `forward declaration` technique and housekeepings. If found, propose a plan to refactor it and ask for approval;
- [ ] Inspect application/symbols searching for unit testing opportunities, propose a plan to implement them step by step and ask for approval;
- [ ] Inspect application/lexer searching for unit testing opportunities, propose a plan to implement them step by step and ask for approval;
- [ ] Inspect application/parser searching for unit testing opportunities, propose a plan to implement them step by step and ask for approval;
- [ ] Inspect application/compiler searching for unit testing opportunities, propose a plan to implement them step by step and ask for approval;
- [ ] Inspect application/builder searching for unit testing opportunities, propose a plan to implement them step by step and ask for approval;
- [ ] Propose a plan to refactor step by step application/ implementing `unique_ptr` (or similar solutions for C++11) on object allocations to avoid memory leaks and ask for approval;
- [ ] Inspect application, domain and infrastructure searching for `printf`, debug, info, warnings and error messages. Next, propose a plan to refactor it step by step applying an appropriate logging engine to them (logging to be showed on console by cli only) and ask for approval;
- [ ] Inspect application, cli and infrastructure searching for any code access to the file system. Next, propose a plan to refactor it step by step applying an appropriate file system helper respecting clean archictecture and paying attention to the file access pecularities used by each original code in the codebase. Finally, ask for approval;
- [ ] Inspect src/application/lexer searching for opportunities to apply or implement the Iterator pattern. If found, propose a plan to refactor it step by step and ask for approval;
- [ ] Inspect src/application/parser searching for opportunities to apply or implement the Iterator pattern. If found, propose a plan to refactor it step by step and ask for approval;
- [ ] Inspect src/application/compiler searching for opportunities to apply or implement the Iterator pattern. If found, propose a plan to refactor it step by step and ask for approval;
- [ ] Inspect src/application/builder searching for opportunities to apply or implement the Iterator pattern. If found, propose a plan to refactor it step by step and ask for approval;
- [ ] Inspect src/application/symbols searching for opportunities to apply or implement the Iterator pattern. If found, propose a plan to refactor it step by step and ask for approval.
