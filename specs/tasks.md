# Tasks – Current Sprint Backlog

## To do list

- [x] Write user stories in separate files at user-stories/ for lexer (src/lex.cpp), parser (src/parse.cpp), compiler (src/compiler.cpp), builder (rom.cpp and resources.cpp) and cli (src/main.cpp) writing test scenarios for each one (see ADR-001);
- [x] Optimize the project Makefile to consider src/ and include/ dependencies;
- [x] Rename test/ to tests/, adjusting the related .github/workflows/test.yml and .vscode/tasks.json (and any other reference on any file to the old folder) according the changes;
- [x] Write unit tests on tests/unit/ for lexer, parser, compiler and builder refactoring test.cpp and test_resources, adjusting the related Makefile and .github/workflows/test.yml according the changes;
- [ ] Move src/_.cpp and include/_.h files to src/cli/, src/application/, src/domain/ and src/infrastructure/ according with its main objective, adjusting the related Makefile, asm/Makefile and unit testings (tests/) according the changes;
- [ ] Refactor src/application/lexer/ files to one file per class practice (do not change any code), adjusting the related Makefile and unit testings according the changes;
- [ ] Refactor src/application/parser/ files to one file per class practice (do not change any code), adjusting the related Makefile and unit testings according the changes;
- [ ] Refactor src/application/compiler/ files to one file per class practice (do not change any code), adjusting the related Makefile and unit testings according the changes;
- [ ] Refactor src/application/builder/ files to one file per class practice (do not change any code), adjusting the related Makefile and unit testings according the changes;
- [ ] Move asm/ folder to src/infrastructure/external/, adjusting the related Makefile and .vscode/tasks.json according the changes.
