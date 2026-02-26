# Tasks – Current Sprint Backlog

## To do list

- [x] Write user stories in separate files at user-stories/ for lexer (src/lex.cpp), parser (src/parse.cpp), compiler (src/compiler.cpp), builder (rom.cpp and resources.cpp) and cli (src/main.cpp) writing test scenarios for each one (see ADR-001);
- [x] Optimize the project Makefile to consider src/ and include/ dependencies;
- [x] Rename test/ to tests/, adjusting the related .github/workflows/test.yml and .vscode/tasks.json (and any other reference on any file to the old folder) according the changes;
- [x] Write unit tests on tests/unit/ for lexer, parser, compiler and builder refactoring test.cpp and test_resources, adjusting the related Makefile and .github/workflows/test.yml according the changes;
- [x] Move all .cpp and .h files from src/ and include/ to src/cli/, src/application/, src/domain/ and src/infrastructure/ according by each file code objective (dont delete any file or modify its contents in this step, ask me if any doubt). Next, adjust the related Makefile, asm/Makefile and unit testings (tests/) according the changes. Finally, delete include/ folder if empty;
- [x] At src/application/lexer, rename lex.cpp to lexer.cpp and lex.h to lexer.h. Next, at src/application/parser, rename parse.cpp to parser.cpp and parse.h to parser.h. Finally, adjust the related Makefile and unit testings according the changes;
- [ ] Create tests/unit/src/ folder. Next, move tests/unit/*.cpp files to tests/unit/src/. Finally, adjust the related Makefile according the changes, making it  
- [ ] Refactor src/domain/symbols/ files to one file per class practice (do not change any code), adjusting the related Makefile and unit testings according the changes;
- [ ] Refactor src/application/lexer/ files to one file per class practice (do not change any code), adjusting the related Makefile and unit testings according the changes;
- [ ] Refactor src/application/parser/ files to one file per class practice (do not change any code), adjusting the related Makefile and unit testings according the changes;
- [ ] Refactor src/application/compiler/ files to one file per class practice (do not change any code), adjusting the related Makefile and unit testings according the changes;
- [ ] Refactor src/application/builder/ files to one file per class practice (do not change any code), adjusting the related Makefile and unit testings according the changes;
- [ ] Move asm/ folder to src/infrastructure/external/, adjusting the related Makefile and .vscode/tasks.json according the changes;
- [ ] Refactor src/application/lexer/ implementing a State pattern with minimum code change. The individual states files must be saved to src/application/lexer/states/ folder. Finally, adjust the related Makefile and unit testings according the changes;
- [ ] Refactor src/application/parser/ implementing a Strategy pattern (tell me if another pattern applies better) with minimum code change. The individual strategy files must be saved to src/application/parser/strategies/ folder. Finally, adjust the related Makefile and unit testings according the changes;
- [ ] Refactor src/application/compiler/ implementing a Strategy pattern with minimum code change. The individual strategy files must be saved to src/application/compiler/strategies/ folder. Finally, adjust the related Makefile and unit testings according the changes;
- [ ] Refactor src/application/builder/ implementing a Strategy pattern with minimum code change. The individual strategy files must be saved to src/application/builder/strategies/ folder. Finally, adjust the related Makefile and unit testings according the changes;
- [ ] Search the codebase for any code access to the file system and refactory it to src/infrastructure/fs, adjusting the related Makefile and unit testings according the changes.
