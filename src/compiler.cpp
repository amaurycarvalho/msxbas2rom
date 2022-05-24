//-------------------------------------------------------------------
// Compiler class specialized as a Z80 code builder for MSX system
// created by Amaury Carvalho, 2020
//-------------------------------------------------------------------

#include "compiler.h"
#include "compiler_hooks.h"

Compiler::Compiler() {

    code = (unsigned char *) malloc(COMPILE_CODE_SIZE);
    ram = (unsigned char *) malloc(COMPILE_RAM_SIZE);

    memset(code, 0x00, COMPILE_CODE_SIZE);
    memset(ram, 0x00, COMPILE_RAM_SIZE);

    code_size = 0;
    code_start = 0;
    code_pointer = code_start;

    ram_size = 0;
    ram_start = code_start + 0x4020;
    ram_pointer = ram_start;
    ramMemoryPerc = 0;
    segm_last = 0;
    segm_total = 0;

    error_message = "";
    current_tag = 0;
    debug = false;
    megaROM = false;
    has_line_number = false;
    compiled = false;

}

Compiler::~Compiler() {
    free(code);
    free(ram);
}

bool Compiler::build(Parser *parser) {
    TagNode *tag;
    SymbolNode *symbol;
    CodeNode *codeItem;
    unsigned int i, t;

    this->parser = parser;

    clear_symbols();

    t = parser->tags.size();
    compiled = (t > 0);

    code_pointer = code_start;
    code_size = 0;
    ram_pointer = ram_start;
    ram_size = 0;
    for(i=0; i < 5; i++)
        last_code[i] = &code[code_pointer];

    if(debug)
        printf("Registering start of program...");

    codeItem = new CodeNode();
    codeItem->name = "START_PGM";
    codeItem->start = code_pointer;
    cmd_start();
    codeItem->length = code_pointer - codeItem->start;
    codeItem->is_code = true;
    codeItem->debug = true;
    codeList.push_back(codeItem);
    if(debug)
        printf(" %i byte(s)\n", codeItem->length);
    if(codeItem->length >= 0x4000) {
        syntax_error("Maximum of start of program code per ROM reached (16k)");
        return false;
    }


    if(debug)
        printf("Registering compiled code (line/bytes): ");

    for(i = 0; i < t; i++) {

        tag = parser->tags[i];

        if(tag) {

            if(debug) {
                if(i)
                    printf(", %s", tag->name.c_str());
                else
                    printf("%s", tag->name.c_str());
            }

            if(tag->name != "DIRECTIVE") {

                // Trim leading zeros
                while (tag->name.find("0") == 0 && tag->name.size() > 1) {
                    tag->name.erase(0, 1);
                }

                symbol = getSymbol(tag);
                if(symbol) {
                    if(symbol->address) {
                        if(debug)
                            printf(" error\n");
                        current_tag = tag;
                        syntax_error("Line number already declared");
                        break;
                    }
                } else
                    symbol = addSymbol(tag);

                symbol->address = code_pointer;

            }

            codeItem = new CodeNode();
            codeItem->start = code_pointer;
            codeItem->name = "LIN_"+tag->name;

            if(!evaluate(tag)) {
                compiled = false;
                break;
            }

            codeItem->length = code_pointer - codeItem->start;
            codeItem->is_code = true;
            codeItem->debug = true;
            codeList.push_back(codeItem);

            if(debug)
                printf("/%i", codeItem->length);

            if(codeItem->length >= 0x4000) {
                if(debug)
                    printf(" error\n");
                syntax_error("Maximum of code per line per ROM reached (16k)");
                return false;
            }

        }

    }

    if(debug)
        printf("\n");

    if(compiled) {
        if(forNextStack.size()) {
            current_tag = forNextStack.top()->tag;
            syntax_error("FOR without a NEXT");
        }
    }

    if(compiled) {

        current_tag = 0;

        if(debug)
            printf("Registering end of program...");

        codeItem = new CodeNode();
        codeItem->name = "END_PGM";
        codeItem->start = code_pointer;
        cmd_end(true);
        codeItem->length = code_pointer - codeItem->start;
        codeItem->is_code = true;
        codeItem->debug = true;
        codeList.push_back(codeItem);
        if(debug)
            printf(" %i byte(s)\n", codeItem->length);
        if(codeItem->length >= 0x4000) {
            syntax_error("Maximum of end of program code per ROM reached (16k)");
            return false;
        }

        if(debug)
            printf("Registering support code...");

        codeItem = new CodeNode();
        codeItem->start = code_pointer;
        func_symbols();
        codeItem->length = code_pointer - codeItem->start;
        codeItem->is_code = true;
        codeItem->debug = false;
        codeList.push_back(codeItem);
        if(debug)
            printf(" %i byte(s)\n", codeItem->length);
        if(codeItem->length >= 0x4000) {
            syntax_error("Maximum of support code per ROM reached (16k)");
            return false;
        }

        if(debug)
            printf("Registering symbols..");

        data_symbols();

        if(debug)
            printf(".");

        i = save_symbols();

        if(debug)
            printf(" %i byte(s)\n", i);

        if(debug)
            printf("Adjusting code/data memory address...\n");

        do_fix();

        ramMemoryPerc = (ram_size * 100.0) / def_RAM_SIZE;
    }

    return compiled;
}

void Compiler::clear_symbols() {

    code_size = 0;
    code_start = 0;
    code_pointer = code_start;

    ram_size = 0;
    ram_start = code_start + 0x4000 + def_RAM_INIT;
    ram_pointer = ram_start;
    ramMemoryPerc = 0;

    error_message = "";
    current_tag = 0;

    mark_count = 0;
    for_count = 0;
    data_start = 0;
    pt3 = false;
    akm = false;
    font = false;
    file_support = false;
    has_defusr = false;
    has_open_grp = false;
    has_tiny_sprite = false;
    compiled = false;

    end_mark = 0;
    enable_basic_mark = 0;
    disable_basic_mark = 0;
    draw_mark = 0;
    io_redirect_mark = 0;
    io_screen_mark = 0;

    symbols.clear();
    fixes.clear();

    resourceList.clear();
    codeList.clear();
    fileList.clear();
    dataList.clear();

    while(!forNextStack.empty())
        forNextStack.pop();

    temp_str_mark = new SymbolNode();
    temp_str_mark->lexeme = new Lexeme(Lexeme::type_identifier, Lexeme::subtype_numeric, "_TEMPSTR_START_", "0");
    temp_str_mark->lexeme->isAbstract = true;

    heap_mark = new SymbolNode();
    heap_mark->lexeme = new Lexeme(Lexeme::type_identifier, Lexeme::subtype_numeric, "_HEAP_", "0");
    heap_mark->lexeme->isAbstract = true;

}

void Compiler::data_symbols() {
    Lexeme *lexeme;
    int i, t = parser->datas.size();

    if(megaROM) {

        data_start = symbols.size();

        for(i = 0; i < t; i++) {
            lexeme = parser->datas[i];
            addSymbol(lexeme);
        }

    } else {

        if(parser->has_data) {
            lexeme = new Lexeme();
            lexeme->name = "_DATA_";
            lexeme->value = lexeme->name;
            resourceList.push_back(lexeme);
        }

    }

}

int Compiler::save_symbols() {
    unsigned int i, t = symbols.size();
    SymbolNode *symbol;
    CodeNode *codeItem;
    Lexeme *lexeme;
    char *s;
    int length = 0, var_size = 0, literal_count=0;

    for(i = 0; i < t; i++) {

        if((symbol = symbols[i])) {
            if((lexeme = symbol->lexeme)) {

                if(lexeme->isAbstract)
                    continue;

                if(megaROM && parser->has_data) {
                    if(i == (unsigned int) data_start) {
                        if(data_mark) {
                            data_mark->symbol->address = code_pointer;
                        } else {
                            syntax_error("Internal error: DATA MARK");
                        }
                    }
                }

                if(lexeme->type == Lexeme::type_literal ) {

                    symbol->address = code_pointer;

                    // string constant or binary data
                    if(lexeme->subtype == Lexeme::subtype_string ||
                            lexeme->subtype == Lexeme::subtype_binary_data ||
                            lexeme->subtype == Lexeme::subtype_integer_data) {
                        int k, tt = lexeme->value.size();

                        codeItem = new CodeNode();
                        codeItem->name = "LIT_" + to_string(literal_count);
                        codeItem->start = code_pointer;

                        literal_count ++;

                        s = (char *) lexeme->value.c_str();

                        if(lexeme->subtype == Lexeme::subtype_string) {
                            if(tt) {
                                // strip quotes

                                if(s[tt-1] == '"')
                                    tt--;
                                if(s[0] == '"') {
                                    s++;
                                    tt --;
                                }

                                if(tt < 0)
                                    tt = 0;
                            }
                        }

                        if(lexeme->subtype == Lexeme::subtype_integer_data) {
                            k = atoi(s);
                            addWord(k);
                        } else {
                            addByte(tt);

                            for(k = 0; k < tt; k++) {
                                addByte(s[k]);
                            }
                        }

                        codeItem->length = code_pointer - codeItem->start;
                        codeItem->is_code = false;
                        codeItem->debug = true;
                        codeList.push_back(codeItem);

                        length += codeItem->length;

                    }

                } else if(lexeme->type == Lexeme::type_identifier ) {

                    codeItem = new CodeNode();
                    codeItem->name = "VAR_"+lexeme->value;
                    codeItem->start = ram_pointer;
                    codeItem->is_code = false;
                    codeItem->debug = true;
                    dataList.push_back(codeItem);

                    var_size = 0;

                    symbol->address = ram_pointer;

                    // string variable
                    if(lexeme->subtype == Lexeme::subtype_string) {

                        var_size = 256;

                        // integer variable
                    } else if(lexeme->subtype == Lexeme::subtype_numeric) {

                        var_size = 2;

                        // double variable
                    } else if(lexeme->subtype == Lexeme::subtype_double_decimal) {

                        var_size = 3;

                        // single variable
                    } else if(lexeme->subtype == Lexeme::subtype_single_decimal) {

                        var_size = 3;

                    }

                    if(lexeme->isArray) {
                        var_size = lexeme->array_size;
                        if(!var_size) {
                            syntax_error("Array [" + lexeme->value + "] declaration is missing");
                        }
                    }

                    codeItem->length = var_size;

                    ram_size += var_size;
                    ram_pointer += var_size;

                    if(ram_size > def_RAM_SIZE) {
                        syntax_error("Not enough memory to variables [" + to_string(ram_size) + " bytes occupied from RAM]");
                    }

                }
            }
        }
    }

    temp_str_mark->address = ram_pointer;

    var_size = (256 * 5);  // temporary strings
    ram_size += var_size;
    ram_pointer += var_size;

    if(parser->has_font) {
        ram_size += def_RAM_BUFSIZ;
        ram_pointer += def_RAM_BUFSIZ;
    }

    heap_mark->address = ram_pointer;

    return length;
}

void Compiler::do_fix() {
    unsigned int i, t = fixes.size(), address;
    FixNode *fix;
    SymbolNode *symbol;

    for(i = 0; i < t; i++) {

        fix = fixes[i];
        symbol = fix->symbol;
        address = symbol->address;

        if(!address) {
            if(symbol->lexeme) {
                symbol->lexeme->print();
                syntax_error("Symbol reference not found: variable or constant");
            } else if(symbol->tag) {
                current_tag = symbol->tag;
                syntax_error("Symbol reference not found: line number in GOTO/GOSUB/RETURN");
            } else
                syntax_error("Symbol reference not found");
            break;
        }

        address += fix->step;

        code[fix->address] = address & 0xFF;
        code[fix->address+1] = (address >> 8) & 0xFF;

    }
}

void Compiler::pushLastCode() {
    int i;
    for(i = 4; i > 0; i--) {
        last_code[i] = last_code[i-1];
    }
    last_code[0] = &code[code_pointer];
}

void Compiler::popLastCode() {
    int i;
    for(i = 1; i < 5; i++) {
        last_code[i-1] = last_code[i];
    }
}

void Compiler::addCodeByte(unsigned char byte) {

    code[code_pointer++] = byte;
    code_size ++;

}

void Compiler::addByteOptimized(unsigned char byte) {
    bool optimized = false;
    int code_pointer_saved = code_pointer;
    int code_reduced = 0;
    unsigned char *s;

    if(! megaROM) {

        switch(byte) {

            // pop de
            case 0xD1 : {

                    // ld hl,x | push hl | ld hl,x | pop de
                    if(*last_code[0] == 0x21 && *last_code[1] == 0xE5 && *last_code[2] == 0x21 && *last_code[3] != 0xDD && *last_code[3] != 0xFD) {
                        // optimize to ld de,x | ld hl,x
                        s = last_code[2];
                        s[0] = 0x11;
                        s[3] = 0x21;
                        s[4] = s[5];
                        s[5] = s[6];
                        popLastCode();
                        last_code[1] = s;
                        last_code[0] = &s[3];
                        code_pointer_saved = (last_code[0] - &code[0]);
                        code_reduced = 1;
                        code_pointer -= code_reduced;
                        code_size -= code_reduced;
                        optimized = true;

                        // push hl | ld hl,x | pop de
                    } else if(*last_code[0] == 0x21 && *last_code[1] == 0xE5) {
                        // optimize to ex de,hl | ld hl,x
                        s = last_code[1];
                        s[0] = 0xEB;
                        optimized = true;

                        // ld hl,(x) | push hl | ld hl,(x) | pop de
                    } else if(*last_code[0] == 0x2A && *last_code[1] == 0xE5 && *last_code[2] == 0x2A && *last_code[3] != 0xDD && *last_code[3] != 0xFD) {
                        // optimize to ld hl,(x) | ex de,hl | ld hl,(x)
                        s = last_code[1];
                        s[0] = 0xEB;
                        optimized = true;

                        // push hl | ld hl,(x) | pop de
                    } else if(*last_code[0] == 0x2A && *last_code[1] == 0xE5) {
                        // optimize to ex de,hl | ld hl,(x)
                        s = last_code[1];
                        s[0] = 0xEB;
                        optimized = true;

                    }

                }
                break;

            // add hl,de
            case 0x19 : {

                    // ex de,hl | ld hl,x | add hl,de
                    if(*last_code[0] == 0x21 && *last_code[1] == 0xEB) {
                        s = last_code[0];
                        // optimize to inc hl if 1, 2 or 3
                        if(s[2] == 0x00 && s[1] == 1) {
                            s = last_code[1];
                            s[0] = 0x23;
                            code_pointer -= 3;
                            code_size -= 3;
                            popLastCode();
                            optimized = true;
                        } else if(s[2] == 0x00 && s[1] == 2) {
                            s = last_code[1];
                            s[0] = 0x23;
                            s[1] = 0x23;
                            code_pointer -= 2;
                            code_size -= 2;
                            popLastCode();
                            optimized = true;
                        } else if(s[2] == 0x00 && s[1] == 3) {
                            s = last_code[1];
                            s[0] = 0x23;
                            s[1] = 0x23;
                            s[2] = 0x23;
                            code_pointer --;
                            code_size --;
                            popLastCode();
                            optimized = true;
                        } else {
                            // optimize to ld de,x | add hl,de
                            s = last_code[1];
                            s[0] = 0x11;
                            s[1] = s[2];
                            s[2] = s[3];
                            s[3] = byte;
                            last_code[0] = &s[3];
                            optimized = true;
                        }

                        // ld hl,(x) | ld de,x | add hl,de
                    } else if(*last_code[0] == 11 && *last_code[1] == 0x2A) {
                        // optimize to ld hl,(x) | inc hl if 1, 2 or 3
                        s = last_code[0];
                        if(s[2] == 0x00) {
                            switch(s[1]) {
                                case 1: {
                                        s[0] = 0x23;
                                        code_pointer -= 2;
                                        code_size -= 2;
                                        optimized = true;
                                    }
                                    break;

                                case 2: {
                                        s[0] = 0x23;
                                        s[1] = 0x23;
                                        code_pointer --;
                                        code_size --;
                                        optimized = true;
                                    }
                                    break;

                                case 3: {
                                        s[0] = 0x23;
                                        s[1] = 0x23;
                                        s[2] = 0x23;
                                        optimized = true;
                                    }
                                    break;
                            }
                        }

                    }

                }
                break;

            // ex de,hl
            case 0xEB : {

                    // ex de,hl | ld hl,x | ex de,hl
                    if(*last_code[0] == 0x21 && *last_code[1] == 0xEB) {
                        // optimize to ld de,x
                        s = last_code[1];
                        s[0] = 0x11;
                        s[1] = s[2];
                        s[2] = s[3];
                        popLastCode();
                        last_code[0] = s;
                        code_pointer_saved = (s - &code[0]);
                        code_reduced = 1;
                        code_pointer--;
                        code_size--;
                        optimized = true;

                        // ld de,x | ld hl,x | ex de,hl
                    } else if(*last_code[0] == 0x21 && *last_code[1] == 0x11 && *last_code[2] != 0xDD && *last_code[2] != 0xFD) {
                        // optimize to ld hl,x | ld de,x
                        s = last_code[1];
                        s[0] = 0x21;
                        s[3] = 0x11;
                        optimized = true;

                        // ld hl,x | ld de,x | ex de,hl
                    } else if(*last_code[0] == 0x11 && *last_code[1] == 0x21 && *last_code[2] != 0xDD && *last_code[2] != 0xFD) {
                        // optimize to ld de,x | ld hl,x
                        s = last_code[1];
                        s[0] = 0x11;
                        s[3] = 0x21;
                        optimized = true;

                    }

                }
                break;

        }

    }

    if(optimized) {

        if(code_reduced) {
            unsigned int i, t = fixes.size();
            FixNode *fix;

            for(i = 0; i < t; i++) {

                fix = fixes[i];
                if(fix) {
                    if(fix->address >= code_pointer_saved) {
                        fix->address -= code_reduced;
                    }
                }

            }

        }

    } else {
        pushLastCode();
        addCodeByte(byte);
    }

}

void Compiler::addByte(unsigned char byte) {

    pushLastCode();
    addCodeByte(byte);

}

void Compiler::addWord(unsigned int word) {
    pushLastCode();
    addCodeByte( word & 0xff );
    addCodeByte( (word >> 8) & 0xff );
}

void Compiler::addWord(unsigned char byte1, unsigned char byte2) {
    pushLastCode();
    addCodeByte( byte1 );
    addCodeByte( byte2 );
}

void Compiler::addCmd(unsigned char byte, unsigned int word) {
    pushLastCode();
    addCodeByte( byte );
    addCodeByte( word & 0xff );
    addCodeByte( (word >> 8) & 0xff );
}

SymbolNode* Compiler::getSymbol(Lexeme *lexeme) {
    unsigned int i, t = symbols.size();
    bool found = false;
    SymbolNode *symbol;

    for(i = 0; i < t; i++) {
        symbol = symbols[i];
        if(symbol->lexeme) {
            if(symbol->lexeme->type == lexeme->type &&
                    symbol->lexeme->subtype == lexeme->subtype &&
                    symbol->lexeme->name == lexeme->name &&
                    symbol->lexeme->value == lexeme->value ) {
                found = true;
                break;
            }
        }
    }

    if(!found)
        symbol = 0;

    return symbol;
}

SymbolNode* Compiler::addSymbol(Lexeme *lexeme) {
    SymbolNode *symbol = getSymbol(lexeme);

    if(!symbol) {
        symbol = new SymbolNode();
        symbol->lexeme = lexeme;
        symbol->tag = 0;
        symbol->address = 0;
        symbols.push_back(symbol);
    }

    return symbol;
}

SymbolNode* Compiler::getSymbol(TagNode *tag) {
    unsigned int i, t = symbols.size();
    bool found = false;
    SymbolNode *symbol;

    for(i = 0; i < t; i++) {
        symbol = symbols[i];
        if(symbol->tag) {
            if(symbol->tag->name == tag->name) {
                found = true;
                break;
            }
        }
    }

    if(!found)
        symbol = 0;

    return symbol;
}

SymbolNode* Compiler::addSymbol(TagNode *tag) {
    SymbolNode *symbol = getSymbol(tag);

    if(!symbol) {
        symbol = new SymbolNode();
        symbol->lexeme = 0;
        symbol->tag = tag;
        symbol->address = 0;
        symbols.push_back(symbol);
    }

    return symbol;
}

SymbolNode* Compiler::addSymbol(string line) {
    unsigned int i, t = symbols.size();
    bool found = false;
    SymbolNode *symbol;
    TagNode *tag;

    for(i = 0; i < t; i++) {
        symbol = symbols[i];
        if(symbol->tag) {
            if(symbol->tag->name == line) {
                found = true;
                break;
            }
        }
    }

    if(!found) {
        tag = new TagNode();
        tag->name = line;
        symbol = addSymbol(tag);
    }

    return symbol;
}

FixNode* Compiler::addFix(SymbolNode *symbol) {
    FixNode *fix = new FixNode();
    bool is_id = false;

    if(symbol->lexeme) {
        is_id = (symbol->lexeme->type == Lexeme::type_identifier);
    }

    if(megaROM && !is_id) {

        // nop, nop      ; reserved to "jr ?, ??" when "call ?, ??" or "jp ?, ??"
        addByte(0x00);
        addByte(0x00);
        // ex AF, AF'    ; save registers (will be restored by MR_ function)
        addByte(0x08);
        // exx
        addByte(0xD9);
        // ld A, <segm>
        addWord(0x3E, 0x00);
        // ld HL, <address>
        addCmd(0x21, 0x0000);
        // CALL MR_????

    }

    fix->symbol = symbol;
    fix->address = code_pointer + 1;
    fix->step = 0;
    fixes.push_back(fix);

    return fix;
}

FixNode* Compiler::addFix(Lexeme *lexeme) {
    SymbolNode *symbol = addSymbol(lexeme);
    return addFix(symbol);
}

FixNode* Compiler::addFix(string line) {
    return addFix(addSymbol(line));
}

SymbolNode* Compiler::addPreMark() {
    string mark_name = "MARK_" + to_string(mark_count);
    mark_count ++;
    return addSymbol(mark_name);
}

FixNode* Compiler::addMark() {
    return addFix(addPreMark());
}

int Compiler::write(unsigned char *dest, int start_address) {
    unsigned int i, t;
    unsigned int k, tt = 0, step;
    unsigned char *d, *s;
    int address, new_address, length, segm_from, segm_to;
    FixNode *fix, *skip;
    CodeNode *codeItem;
    vector<FixNode*> skips;
    bool is_id, is_jump, is_load;

    // copy compiled code to final destination

    if(megaROM) {

        skips.clear();

        t = codeList.size();
        segm_last = 2;         // last ROM segment starts at segment 2
        segm_total = 4;        // 4 segments of 8kb (0, 1, 2, 3)
        length = (start_address - 0x8000);
        code_size = 0;
        d = dest;

        for(i = 0; i < t; i++) {

            codeItem = codeList[i];

            //printf("%i address %i size %i\n", i, codeItem->start, codeItem->length);

            if(codeItem->length) {

                s = &code[codeItem->start];

                if(codeItem->is_code)
                    step = 8;
                else
                    step = 3;

                tt = (length + codeItem->length + step);

                if( tt >= 0x4000 ) {

                    segm_last += 2;     // konami segments size are 8kb (0/1, 2/3...)

                    if(codeItem->is_code) {
                        // code to skip a segment to another

                        // ld a, segmt
                        d[0] = 0x3E;
                        d[1] = segm_last;   // extra code will start after segment 3
                        // ld hl, 0x8000
                        d[2] = 0x21;
                        d[3] = 0x00;
                        d[4] = 0x80;
                        // jp MR_JUMP
                        d[5] = 0xC3;
                        d[6] = (def_MR_JUMP & 0xFF);
                        d[7] = ((def_MR_JUMP >> 8) & 0xFF);

                    } else {
                        // data mark to skip a segment to another
                        d[0] = 0xFF;
                        d[1] = 0xFF;
                        d[2] = 0xFF;
                    }

                    d += step;
                    length += step;

                    while(length < 0x4000) {
                        d[0] = 0;
                        d++;
                        length++;
                        step++;
                    }

                    skip = new FixNode();
                    skip->address = codeItem->start;
                    skip->step = step;
                    skips.push_back(skip);

                    code_size += length;

                    length = 0;
                }

                memcpy(d, s, codeItem->length);

                d += codeItem->length;
                length += codeItem->length;

            }

        }

        code_size += length;

        segm_total = ((segm_last + 1) / 16 + 1) * 16;

        tt = skips.size();

    } else {

        memcpy(dest, code, code_size);

    }

    // reallocate code pointers to new start address

    t = fixes.size();

    for(i = 0; i < t; i++) {

        fix = fixes[i];
        if(fix) {
            if(fix->symbol) {

                if(fix->symbol->lexeme) {
                    is_id = (fix->symbol->lexeme->type == Lexeme::type_identifier);
                } else {
                    is_id = false;
                }

                address = fix->address;

                if(megaROM) {

                    step = 0;
                    for(k = 0; k < tt; k++) {
                        skip = skips[k];
                        if(address >= skip->address) {
                            step += skip->step;
                        }
                    }
                    address += step;

                    new_address = fix->symbol->address + fix->step;

                    if(is_id) {

                        new_address += 0x8000;

                        dest[address] = new_address & 0xFF;
                        dest[address+1] = (new_address >> 8) & 0xFF;

                    } else {

                        step = 0;
                        for(k = 0; k < tt; k++) {
                            skip = skips[k];
                            if(new_address >= skip->address) {
                                step += skip->step;
                            }
                        }
                        new_address += start_address + step;

                        segm_from = ( (address + start_address - 0x8000) / 0x4000)*2 + 2;  // segments (8k/8k): 0/1, 2/3, ....
                        segm_to = ( (new_address - 0x8000) / 0x4000)*2 + 2;  // segments (8k/8k): 0/1, 2/3, ....

                        new_address = (new_address % 0x4000) + 0x8000;

                        switch(dest[address - 1]) {
                            case 0xFA: // jp m,
                            case 0xEA: // jp pe,
                            case 0xF2: // jp p,
                            case 0xE2: // jp po,
                            case 0xD2: // jp nc,
                            case 0xC2: // jp nz,
                            case 0xDA: // jp c,
                            case 0xCA: // jp z,
                            case 0xC3: // jp
                                is_jump = true;
                                is_load = false;
                                break;
                            case 0xF4: // call p,
                            case 0xE4: // call po,
                            case 0xFC: // call m,
                            case 0xEC: // call pe,
                            case 0xD4: // call nc,
                            case 0xC4: // call nz,
                            case 0xDC: // call c,
                            case 0xCC: // call z,
                            case 0xCD: // call
                            case 0xFF: // special load
                                is_jump = false;
                                is_load = false;
                                break;
                            default:
                                is_jump = false;
                                is_load = true;
                        }

                        if(segm_from == segm_to && (is_jump || is_load)) {

                            dest[address] = new_address & 0xFF;
                            dest[address+1] = (new_address >> 8) & 0xFF;

                            for(k = 2; k < 11; k++) {
                                dest[address-k] = 0;  // 9 nops at previous bytes
                            }

                            dest[address-10] = 0x18;  // jr $+8
                            dest[address- 9] = 0x07;

                        } else {

                            // ld a, segm        ; 2 bytes
                            dest[address - 5] = segm_to & 0xFF;
                            // ld hl, address    ; 3 bytes
                            dest[address - 3] = new_address & 0xFF;
                            dest[address - 2] = (new_address >> 8) & 0xFF;

                            // call MR_....      ; 3 bytes
                            switch(dest[address - 1]) {
                                // CALL
                                case 0xF4: // call p,
                                case 0xE4: // call po,
                                case 0xFC: // call m,
                                case 0xEC: { // call pe,
                                        dest[address - 10] = 0xF5;  // push af
                                        dest[address -  9] = 0x08;  // ex af, af'
                                        dest[address -  8] = 0xD9;  // exx
                                        dest[address -  7] = 0xF1;  // pop af
                                        dest[address] = def_MR_CALL & 0xFF;
                                        dest[address+1] = (def_MR_CALL >> 8) & 0xFF;
                                    }
                                    break;
                                case 0xD4: { // call nc,
                                        dest[address - 10] = 0x38;  // jr c, $+11
                                        dest[address -  9] = 0x0A;
                                        dest[address -  1] = 0xCD;  // change to call
                                        dest[address] = def_MR_CALL & 0xFF;
                                        dest[address+1] = (def_MR_CALL >> 8) & 0xFF;
                                    }
                                    break;
                                case 0xC4: { // call nz,
                                        dest[address - 10] = 0x28;  // jr z, $+11
                                        dest[address -  9] = 0x0A;
                                        dest[address -  1] = 0xCD;  // change to call
                                        dest[address] = def_MR_CALL & 0xFF;
                                        dest[address+1] = (def_MR_CALL >> 8) & 0xFF;
                                    }
                                    break;
                                case 0xDC: { // call c,
                                        dest[address - 10] = 0x30;  // jr nc, $+11
                                        dest[address -  9] = 0x0A;
                                        dest[address -  1] = 0xCD;  // change to call
                                        dest[address] = def_MR_CALL & 0xFF;
                                        dest[address+1] = (def_MR_CALL >> 8) & 0xFF;
                                    }
                                    break;
                                case 0xCC: { // call z,
                                        dest[address - 10] = 0x20;  // jr nz, $+11
                                        dest[address -  9] = 0x0A;
                                        dest[address -  1] = 0xCD;  // change to call
                                        dest[address] = def_MR_CALL & 0xFF;
                                        dest[address+1] = (def_MR_CALL >> 8) & 0xFF;
                                    }
                                    break;
                                case 0xCD: { // call
                                        dest[address] = def_MR_CALL & 0xFF;
                                        dest[address+1] = (def_MR_CALL >> 8) & 0xFF;
                                    }
                                    break;
                                // JUMP
                                case 0xFA: // jp m,
                                case 0xEA: // jp pe,
                                case 0xF2: // jp p,
                                case 0xE2: { // jp po,
                                        dest[address - 10] = 0xF5;  // push af
                                        dest[address -  9] = 0x08;  // ex af, af'
                                        dest[address -  8] = 0xD9;  // exx
                                        dest[address -  7] = 0xF1;  // pop af
                                        dest[address] = def_MR_JUMP & 0xFF;
                                        dest[address+1] = (def_MR_JUMP >> 8) & 0xFF;
                                    }
                                    break;
                                case 0xD2: { // jp nc,
                                        dest[address - 10] = 0x38;  // jr c, $+11
                                        dest[address -  9] = 0x0A;
                                        dest[address -  1] = 0xC3;  // change to jp
                                        dest[address] = def_MR_JUMP & 0xFF;
                                        dest[address+1] = (def_MR_JUMP >> 8) & 0xFF;
                                    }
                                    break;
                                case 0xC2: { // jp nz,
                                        dest[address - 10] = 0x28;  // jr z, $+11
                                        dest[address -  9] = 0x0A;
                                        dest[address -  1] = 0xC3;  // change to jp
                                        dest[address] = def_MR_JUMP & 0xFF;
                                        dest[address+1] = (def_MR_JUMP >> 8) & 0xFF;
                                    }
                                    break;
                                case 0xDA: { // jp c,
                                        dest[address - 10] = 0x30;  // jr nc, $+11
                                        dest[address -  9] = 0x0A;
                                        dest[address -  1] = 0xC3;  // change to jp
                                        dest[address] = def_MR_JUMP & 0xFF;
                                        dest[address+1] = (def_MR_JUMP >> 8) & 0xFF;
                                    }
                                    break;
                                case 0xCA: { // jp z,
                                        dest[address - 10] = 0x20;  // jr nz, $+11
                                        dest[address -  9] = 0x0A;
                                        dest[address -  1] = 0xC3;  // change to jp
                                        dest[address] = def_MR_JUMP & 0xFF;
                                        dest[address+1] = (def_MR_JUMP >> 8) & 0xFF;
                                    }
                                    break;
                                case 0xC3: { // jp
                                        dest[address] = def_MR_JUMP & 0xFF;
                                        dest[address+1] = (def_MR_JUMP >> 8) & 0xFF;
                                    }
                                    break;
                                // special LOAD (segment address into a:hl)
                                case 0xFF: {
                                        dest[address - 10] = dest[address - 6];
                                        dest[address -  9] = dest[address - 5];
                                        dest[address -  8] = dest[address - 4];
                                        dest[address -  7] = dest[address - 3];
                                        dest[address -  6] = dest[address - 2];
                                        dest[address -  5] = 0x18;  // jr $+6
                                        dest[address -  4] = 0x05;  // nop
                                        dest[address -  3] = 0x00;  // nop
                                        dest[address -  2] = 0x00;  // nop
                                        dest[address -  1] = 0x00;  // nop
                                        dest[address]      = 0x00;  // nop
                                        dest[address +  1] = 0x00;  // nop
                                    }
                                    break;
                                // LOAD
                                default: {
                                        dest[address - 1] = 0xCD;
                                        dest[address] = def_MR_GET_DATA & 0xFF;
                                        dest[address+1] = (def_MR_GET_DATA >> 8) & 0xFF;
                                    }
                            }

                        }

                    }

                } else {

                    new_address = (dest[address+1] << 8) | dest[address];

                    if(is_id) {
                        new_address += 0x8000;
                    } else {
                        new_address += start_address;
                    }

                    dest[address] = new_address & 0xFF;
                    dest[address+1] = (new_address >> 8) & 0xFF;

                }

            } else
                printf("Error fixing symbol (null)\n");
        } else
            printf("Error in fix object (null)\n");

    }

    return code_size;
}

void Compiler::double2FloatLib(double value, int *words) {
    float value2 = value;
    float2FloatLib(value2, words);
}

void Compiler::float2FloatLib(float value, int *words) {
    unsigned short *pi = (unsigned short *) &value;
    unsigned char *pc = (unsigned char *) &value;
    unsigned char sign = 0;

    if(value == 0.0) {
        words[0] = 0;
        words[1] = 0;
        words[2] = 0;
        words[3] = 0;
        return;
    }

    if(pc[3] & 0x80)
        sign = 0x80;
    pc[3] <<= 1;

    if(pc[2] & 0x80)
        pc[3] |= 1;

    pc[2] &= 0x7F;
    pc[2] |= sign;

    pc[3] ++;

    words[0] = pi[0];
    words[1] = pi[1];
    words[2] = 0;
    words[3] = 0;

}

int Compiler::str2FloatLib(string text) {
    int words[4];
    float value;
    try {
        value = stof(text);
    } catch(exception &e) {
        printf("Warning: error while converting numeric constant %s\n", text.c_str());
        value = 0;
    }
    float2FloatLib(value, (int *) &words);
    return ((words[1] + 0x0100) << 8) | (words[0] >> 8 & 0xff);
}

int Compiler::getUsingFormat(string text) {
    int c = 0x80;           // format style (7=1 6=, 5=* 4=$ 3=+ 2=- 1=0  0=^)
    int d = 0, e = 0;       // d=thousand digits, e=decimal digits
    int b = text.size();    // format string size
    int i;
    char *s = (char *) text.c_str();

    for(i = 0; i < b; i++) {
        switch(s[i]) {
            // 0, set zeros formating
            case '0' : {
                    c |= 1<<1; // set bit 1
                    if(e)
                        e++;
                    else
                        d++;
                }
                break;

            // # numeric format char, handle
            case '#' : {
                    if(e)
                        e++;
                    else
                        d++;
                }
                break;

            // +, set sign flag and continue
            case '+' : {
                    c |= 1<<3;  // set bit 3
                    if(d)
                        c |= 1<<2; // set bit 2
                    else {
                        if(e)
                            e++;
                        else
                            d++;
                    }
                }
                break;

            // -, set sign after number
            case '-' : {
                    c |= 1<<2; // set bit 2
                }
                break;

            // decimal point
            case '.' : {
                    e++;
                }
                break;

            // mark symbol
            case '*' : {
                    c |= 1 << 5;  // set bit 5
                    if(e)
                        e++;
                    else
                        d++;
                }
                break;

            // currency symbol
            case '$' : {
                    c |= 1 << 4;  // set bit 4
                    if(e)
                        e++;
                    else
                        d++;
                }
                break;

            // thousand separator
            case ',' : {
                    c |= 1 << 6;  // set bit 6
                    if(e)
                        e++;
                    else
                        d++;
                }
                break;

            // exponential representation
            case '^' : {
                    c |= 1;  // set bit 0
                    if(e)
                        e++;
                    else
                        d++;
                }
                break;

        }
    }

    return ( (e&0xF) | ((d&0xF) << 4) | (c << 8) );
}

//-------------------------------------------------------------------------------------------

bool Compiler::evaluate(TagNode *tag) {
    ActionNode *action;
    unsigned int i, t = tag->actions.size(), lin;

    current_tag = tag;

    if(has_line_number) {
        try {
            lin = stoi(tag->name);
        } catch (exception &e) {
            printf("Warning: error while converting numeric constant %s\n", tag->name.c_str());
            lin = 0;
        }
        // ld hl, line number
        addCmd(0x21, lin);
        // ld (CURLIN), hl
        addCmd(0x22, def_CURLIN);
    }

    for(i = 0; i < t && compiled; i++) {

        action = tag->actions[i];

        if(!evalAction(action))
            return false;

    }

    return compiled;
}

bool Compiler::evalActions(ActionNode *action) {
    ActionNode *sub_action;
    unsigned int i, t = action->actions.size();

    for(i = 0; i < t && compiled; i++) {

        sub_action = action->actions[i];

        if(!evalAction(sub_action))
            return false;

    }

    return compiled;
}

bool Compiler::evalAction(ActionNode *action) {
    Lexeme *lexeme;
    bool traps_checked = false;

    if(action) {

        current_action = action;
        lexeme = action->lexeme;

        if(lexeme->type == Lexeme::type_keyword) {

            if(lexeme->name == "END") {
                traps_checked = addCheckTraps();
                cmd_end(false);
            } else if(lexeme->name == "REM" || lexeme->name == "'") {
                return true;
            } else if(lexeme->name == "CLEAR") {
                return true;
            } else if(lexeme->name == "DEF" || lexeme->name == "DEFINT" ||
                      lexeme->name == "DEFSNG" || lexeme->name == "DEFDBL" || lexeme->name == "DEFSTR") {
                cmd_def();
            } else if(lexeme->name == "CLS") {
                cmd_cls();
            } else if(lexeme->name == "BEEP") {
                cmd_beep();
            } else if(lexeme->name == "PRINT") {
                cmd_print();
            } else if(lexeme->name == "INPUT") {
                cmd_input(true);
            } else if(lexeme->name == "GOTO") {
                traps_checked = addCheckTraps();
                cmd_goto();
            } else if(lexeme->name == "GOSUB") {
                traps_checked = addCheckTraps();
                cmd_gosub();
            } else if(lexeme->name == "RETURN") {
                traps_checked = addCheckTraps();
                cmd_return();
            } else if(lexeme->name == "SOUND") {
                cmd_sound();
            } else if(lexeme->name == "OUT") {
                cmd_out();
            } else if(lexeme->name == "POKE") {
                cmd_poke();
            } else if(lexeme->name == "VPOKE") {
                cmd_vpoke();
            } else if(lexeme->name == "PLAY") {
                cmd_play();
            } else if(lexeme->name == "DRAW") {
                cmd_draw();
            } else if(lexeme->name == "LET") {
                cmd_let();
            } else if(lexeme->name == "IF") {
                traps_checked = addCheckTraps();
                cmd_if();
            } else if(lexeme->name == "FOR") {
                cmd_for();
            } else if(lexeme->name == "NEXT") {
                traps_checked = addCheckTraps();
                cmd_next();
            } else if(lexeme->name == "TIME") {
                cmd_let();
            } else if(lexeme->name == "DIM") {
                cmd_dim();
            } else if(lexeme->name == "REDIM") {
                cmd_redim();
            } else if(lexeme->name == "RANDOMIZE") {
                cmd_randomize();
            } else if(lexeme->name == "LOCATE") {
                cmd_locate();
            } else if(lexeme->name == "SCREEN") {
                cmd_screen();
            } else if(lexeme->name == "WIDTH") {
                cmd_width();
            } else if(lexeme->name == "COLOR") {
                cmd_color();
            } else if(lexeme->name == "PSET") {
                cmd_pset(true);
            } else if(lexeme->name == "PRESET") {
                cmd_pset(false);
            } else if(lexeme->name == "LINE") {
                cmd_line();
            } else if(lexeme->name == "PAINT") {
                cmd_paint();
            } else if(lexeme->name == "CIRCLE") {
                cmd_circle();
            } else if(lexeme->name == "COPY") {
                cmd_copy();
            } else if(lexeme->name == "PUT") {
                cmd_put();
            } else if(lexeme->name == "DATA") {
                cmd_data();
            } else if(lexeme->name == "IDATA") {
                cmd_idata();
            } else if(lexeme->name == "READ") {
                cmd_read();
            } else if(lexeme->name == "IREAD") {
                cmd_iread();
            } else if(lexeme->name == "RESTORE") {
                cmd_restore();
            } else if(lexeme->name == "IRESTORE") {
                cmd_irestore();
            } else if(lexeme->name == "RESUME") {
                cmd_resume();
            } else if(lexeme->name == "SET") {
                cmd_set();
            } else if(lexeme->name == "ON") {
                traps_checked = addCheckTraps();
                cmd_on();
            } else if(lexeme->name == "INTERVAL") {
                cmd_interval();
            } else if(lexeme->name == "KEY") {
                cmd_key();
            } else if(lexeme->name == "STRIG") {
                cmd_strig();
            } else if(lexeme->name == "SPRITE") {
                cmd_sprite();
            } else if(lexeme->name == "STOP") {
                traps_checked = addCheckTraps();
                cmd_stop();
            } else if(lexeme->name == "WAIT") {
                cmd_wait();
            } else if(lexeme->name == "SWAP") {
                cmd_swap();
            } else if(lexeme->name == "CALL") {
                cmd_call();
            } else if(lexeme->name == "CMD") {
                cmd_cmd();
            } else if(lexeme->name == "MAXFILES") {
                cmd_maxfiles();
            } else if(lexeme->name == "OPEN") {
                cmd_open();
            } else if(lexeme->name == "OPEN_GRP") {
                has_open_grp = true;
            } else if(lexeme->name == "CLOSE") {
                cmd_close();
            } else if(lexeme->name == "FILE") {
                cmd_file();
            } else if(lexeme->name == "TEXT") {
                cmd_text();
            } else if(lexeme->name == "BLOAD") {
                cmd_bload();
            } else {
                syntax_error();
                return false;
            }

            if(!traps_checked) {
                addCheckTraps();
            }

        } else {
            syntax_error();
            return false;
        }

    } else {
        syntax_error();
        return false;
    }

    return compiled;
}

int Compiler::evalExpression(ActionNode *action) {
    int result = Lexeme::subtype_unknown;
    Lexeme *lexeme;

    lexeme = action->lexeme;

    if(lexeme) {

        if(lexeme->type == Lexeme::type_identifier) {

            result = lexeme->subtype;

            if(lexeme->isArray || result == Lexeme::subtype_string) {

                if(!addVarAddress(action)) {
                    if(!lexeme->isArray && action->actions.size()) {
                        syntax_error("Undeclared array or unknown function");
                    } else
                        result = Lexeme::subtype_unknown;
                } else {
                    if(lexeme->subtype == Lexeme::subtype_numeric) {
                        // ld e, (hl)
                        addByte(0x5E);
                        // inc hl
                        addByte(0x23);
                        // ld d, (hl)
                        addByte(0x56);
                        // ex de, hl
                        addByte(0xEB);
                    } else if(lexeme->subtype == Lexeme::subtype_single_decimal || lexeme->subtype == Lexeme::subtype_double_decimal) {
                        // ld b, (hl)
                        addByte(0x46);
                        // inc hl
                        addByte(0x23);
                        // ld e, (hl)
                        addByte(0x5E);
                        // inc hl
                        addByte(0x23);
                        // ld d, (hl)
                        addByte(0x56);
                        // ex de, hl
                        addByte(0xEB);
                    }
                }
            } else {

                if(action->actions.size()) {
                    syntax_error("Undeclared array or unknown function");
                    return result;
                }

                // get numeric variable data optimization

                if(lexeme->subtype == Lexeme::subtype_numeric) {
                    // ld hl, (variable)
                    addFix( lexeme );
                    addCmd(0x2A, 0x0000);
                } else if(lexeme->subtype == Lexeme::subtype_single_decimal || lexeme->subtype == Lexeme::subtype_double_decimal) {
                    // ld a, (variable)
                    addFix( lexeme );
                    addCmd(0x3A, 0x0000);
                    // ld b, a
                    addByte(0x47);
                    // ld hl, (variable+1)
                    addFix( lexeme )->step = 1;
                    addCmd(0x2A, 0x0000);
                }

            }

        } else if(lexeme->type == Lexeme::type_keyword) {

            result = evalFunction(action);

        } else if(lexeme->type == Lexeme::type_operator) {

            result = evalOperator(action);

        } else if(lexeme->type == Lexeme::type_separator) {

            syntax_error("Invalid separator on expression");
            result = Lexeme::subtype_numeric;

        } else if(lexeme->type == Lexeme::type_literal) {

            result = lexeme->subtype;

            if(lexeme->subtype == Lexeme::subtype_string) {

                addFix( lexeme );

                addCmd( 0x21, 0x0000);  // ld hl, string

            } else if(lexeme->subtype == Lexeme::subtype_numeric) {
                char *s = (char*) lexeme->value.c_str();
                try {
                    if(s[0] == '&') {
                        if(s[1] == 'h' || s[1] == 'H')
                            addCmd( 0x21, stoi(lexeme->value.substr(2), 0, 16));  // ld hl, value
                        else if(s[1] == 'o' || s[1] == 'O')
                            addCmd( 0x21, stoi(lexeme->value.substr(2), 0, 8));  // ld hl, value
                        else if(s[1] == 'b' || s[1] == 'B')
                            addCmd( 0x21, stoi(lexeme->value.substr(2), 0, 2));  // ld hl, value
                        else
                            result = Lexeme::subtype_unknown;
                    } else
                        addCmd( 0x21, stoi(lexeme->value));  // ld hl, value
                } catch(exception &e) {
                    printf("Warning: error while converting numeric constant %s\n", lexeme->value.c_str());
                    addCmd( 0x21, 0 );  // ld hl, value
                }

            } else if(lexeme->subtype == Lexeme::subtype_single_decimal || lexeme->subtype == Lexeme::subtype_double_decimal) {
                int value = str2FloatLib(lexeme->value);

                addByte( 0x06 );
                addByte( (value >> 16) & 0xff ); // ld b, value
                addCmd( 0x21, value & 0xffff);  // ld hl, value

            } else if(lexeme->subtype == Lexeme::subtype_null) {

            } else {

                result = Lexeme::subtype_unknown;

            }
        } else
            result = Lexeme::subtype_unknown;


    }

    return result;
}

int Compiler::evalOperator(ActionNode *action) {
    int result = Lexeme::subtype_unknown;
    Lexeme *lexeme;
    ActionNode *next_action;
    unsigned int t = action->actions.size();
    unsigned char *s;

    if(!t)
        return result;

    lexeme = action->lexeme;

    if(lexeme) {

        if(t == 1)  {

            if(!evalOperatorParms(action, 1))
                return result;

            next_action = action->actions[0];
            result = next_action->subtype;

            if(lexeme->value == "NOT") {

                if( result == Lexeme::subtype_numeric ) {

                    // call intCompareNOT
                    addCmd(0xCD, def_intCompareNOT);

                } else
                    result = Lexeme::subtype_unknown;

            } else if(lexeme->value == "-") {

                if( result == Lexeme::subtype_numeric ) {

                    // call intNEG
                    addCmd(0xCD, def_intNEG);

                } else if( result == Lexeme::subtype_single_decimal || result == Lexeme::subtype_double_decimal ) {

                    // call floatNeg
                    addCmd(0xCD, def_floatNEG);

                } else
                    result = Lexeme::subtype_unknown;

            } else if(lexeme->value == "+") {
                return result;
            } else
                result = Lexeme::subtype_unknown;

        } else if(t == 2) {

            if(!evalOperatorParms(action, 2))
                return result;

            result = evalOperatorCast(action);
            if(result == Lexeme::subtype_unknown)
                return result;

            if(lexeme->value == "AND") {

                if( result == Lexeme::subtype_numeric ) {

                    // pop de
                    addByteOptimized(0xD1);

                    // call intCompareAND
                    addCmd(0xCD, def_intCompareAND);

                } else
                    result = Lexeme::subtype_unknown;

            } else if(lexeme->value == "OR") {

                if( result == Lexeme::subtype_numeric ) {

                    // pop de
                    addByteOptimized(0xD1);

                    // call intCompareOR
                    addCmd(0xCD, def_intCompareOR);

                } else
                    result = Lexeme::subtype_unknown;

            } else if(lexeme->value == "XOR") {

                if( result == Lexeme::subtype_numeric ) {

                    // pop de
                    addByteOptimized(0xD1);

                    // call intCompareXOR
                    addCmd(0xCD, def_intCompareXOR);

                } else
                    result = Lexeme::subtype_unknown;

            } else if(lexeme->value == "EQV") {

                // same as: not a xor b

                if( result == Lexeme::subtype_numeric ) {

                    // pop de
                    addByteOptimized(0xD1);

                    // ld a, l
                    addByte(0x7D);
                    // xor e
                    addByte(0xAB);
                    // cpl
                    addByte(0x2F);
                    // ld l, a
                    addByte(0x6F);
                    // ld a, h
                    addByte(0x7C);
                    // xor d
                    addByte(0xAA);
                    // cpl
                    addByte(0x2F);
                    // ld h, a
                    addByte(0x67);

                } else
                    result = Lexeme::subtype_unknown;

            } else if(lexeme->value == "IMP") {

                if( result == Lexeme::subtype_numeric ) {

                    // same as: not a or b

                    // pop de
                    addByteOptimized(0xD1);

                    // ld a, e
                    addByte(0x7B);
                    // cpl
                    addByte(0x2F);
                    // or l
                    addByte(0xB5);
                    // ld l, a
                    addByte(0x6F);
                    // ld a, d
                    addByte(0x7A);
                    // cpl
                    addByte(0x2F);
                    // or h
                    addByte(0xB4);
                    // ld h, a
                    addByte(0x67);

                } else
                    result = Lexeme::subtype_unknown;

            } else if(lexeme->value == "=") {

                if( result == Lexeme::subtype_numeric ) {

                    // pop de
                    addByteOptimized(0xD1);

                    // call intCompareEQ
                    addCmd(0xCD, def_intCompareEQ);

                } else if( result == Lexeme::subtype_single_decimal || result == Lexeme::subtype_double_decimal ) {

                    // pop af
                    addByte(0xF1);
                    // pop de
                    addByte(0xD1);
                    // ld c, a
                    addByte(0x4F);

                    // 7876 xbasic compare floats (=)
                    addCmd(0xCD, def_XBASIC_COMPARE_FLOATS_EQ);

                    result = Lexeme::subtype_numeric;

                } else if( result == Lexeme::subtype_string ) {

                    // 7e99 xbasic copy string to NULBUF
                    addCmd(0xCD, def_XBASIC_COPY_STRING_TO_NULBUF);
                    // pop hl
                    addByte(0xE1);

                    // 7eae xbasic compare string (NULBUF = string)
                    addCmd(0xCD, def_XBASIC_COMPARE_STRING_WITH_NULBUF_EQ);

                    result = Lexeme::subtype_numeric;

                } else
                    result = Lexeme::subtype_unknown;

            } else if(lexeme->value == "<>") {

                if( result == Lexeme::subtype_numeric ) {

                    // pop de
                    addByteOptimized(0xD1);

                    // call intCompareNE
                    addCmd(0xCD, def_intCompareNE);

                } else if( result == Lexeme::subtype_single_decimal || result == Lexeme::subtype_double_decimal ) {

                    // pop af
                    addByte(0xF1);
                    // pop de
                    addByte(0xD1);
                    // ld c, a
                    addByte(0x4F);

                    // 787f xbasic compare floats (<>)
                    addCmd(0xCD, def_XBASIC_COMPARE_FLOATS_NE);

                    result = Lexeme::subtype_numeric;

                } else if( result == Lexeme::subtype_string ) {

                    // 7e99 xbasic copy string to NULBUF
                    addCmd(0xCD, def_XBASIC_COPY_STRING_TO_NULBUF);
                    // pop hl
                    addByte(0xE1);

                    // 7ec9 xbasic compare string (NULBUF <> string)
                    addCmd(0xCD, def_XBASIC_COMPARE_STRING_WITH_NULBUF_NE);

                    result = Lexeme::subtype_numeric;

                } else
                    result = Lexeme::subtype_unknown;

            } else if(lexeme->value == "<") {

                if( result == Lexeme::subtype_numeric ) {

                    // pop de
                    addByteOptimized(0xD1);

                    // call intCompareLT
                    addCmd(0xCD, def_intCompareLT);

                } else if( result == Lexeme::subtype_single_decimal || result == Lexeme::subtype_double_decimal ) {

                    // pop af
                    addByte(0xF1);
                    // pop de
                    addByte(0xD1);
                    // ld c, a
                    addByte(0x4F);

                    // 7888 xbasic compare floats (>)
                    addCmd(0xCD, def_XBASIC_COMPARE_FLOATS_GT);

                    result = Lexeme::subtype_numeric;

                } else if( result == Lexeme::subtype_string ) {

                    // 7e99 xbasic copy string to NULBUF
                    addCmd(0xCD, def_XBASIC_COPY_STRING_TO_NULBUF);
                    // pop hl
                    addByte(0xE1);

                    // 7ea4 xbasic compare string (NULBUF > string)
                    addCmd(0xCD, def_XBASIC_COMPARE_STRING_WITH_NULBUF_GT);

                    result = Lexeme::subtype_numeric;

                } else
                    result = Lexeme::subtype_unknown;

            } else if(lexeme->value == "<=") {

                if( result == Lexeme::subtype_numeric ) {

                    // pop de
                    addByteOptimized(0xD1);

                    // call intCompareLE
                    addCmd(0xCD, def_intCompareLE);

                } else if( result == Lexeme::subtype_single_decimal || result == Lexeme::subtype_double_decimal ) {

                    // pop af
                    addByte(0xF1);
                    // pop de
                    addByte(0xD1);
                    // ld c, a
                    addByte(0x4F);

                    // 7892 xbasic compare floats (>=)
                    addCmd(0xCD, def_XBASIC_COMPARE_FLOATS_GE);

                    result = Lexeme::subtype_numeric;

                } else if( result == Lexeme::subtype_string ) {

                    // 7e99 xbasic copy string to NULBUF
                    addCmd(0xCD, def_XBASIC_COPY_STRING_TO_NULBUF);
                    // pop hl
                    addByte(0xE1);

                    // 7eb7 xbasic compare string (NULBUF >= string)
                    addCmd(0xCD, def_XBASIC_COMPARE_STRING_WITH_NULBUF_GE);

                    result = Lexeme::subtype_numeric;

                } else
                    result = Lexeme::subtype_unknown;

            } else if(lexeme->value == ">") {

                if( result == Lexeme::subtype_numeric ) {

                    // pop de
                    addByteOptimized(0xD1);

                    // call intCompareGT
                    addCmd(0xCD, def_intCompareGT);

                } else if( result == Lexeme::subtype_single_decimal || result == Lexeme::subtype_double_decimal ) {

                    // pop af
                    addByte(0xF1);
                    // pop de
                    addByte(0xD1);
                    // ld c, a
                    addByte(0x4F);

                    // 789b xbasic compare floats (<)
                    addCmd(0xCD, def_XBASIC_COMPARE_FLOATS_LT);

                    result = Lexeme::subtype_numeric;

                } else if( result == Lexeme::subtype_string ) {

                    // 7e99 xbasic copy string to NULBUF
                    addCmd(0xCD, def_XBASIC_COPY_STRING_TO_NULBUF);
                    // pop hl
                    addByte(0xE1);

                    // 7ec0 xbasic compare string (NULBUF < string)
                    addCmd(0xCD, def_XBASIC_COMPARE_STRING_WITH_NULBUF_LT);

                    result = Lexeme::subtype_numeric;

                } else
                    result = Lexeme::subtype_unknown;

            } else if(lexeme->value == ">=") {

                if( result == Lexeme::subtype_numeric ) {

                    // pop de
                    addByteOptimized(0xD1);

                    // call intCompareGE
                    addCmd(0xCD, def_intCompareGE);

                } else if( result == Lexeme::subtype_single_decimal || result == Lexeme::subtype_double_decimal ) {

                    // pop af
                    addByte(0xF1);
                    // pop de
                    addByte(0xD1);
                    // ld c, a
                    addByte(0x4F);

                    // 78a4 xbasic compare floats (<=)
                    addCmd(0xCD, def_XBASIC_COMPARE_FLOATS_LE);

                    result = Lexeme::subtype_numeric;

                } else if( result == Lexeme::subtype_string ) {

                    // 7e99 xbasic copy string to NULBUF
                    addCmd(0xCD, def_XBASIC_COPY_STRING_TO_NULBUF);
                    // pop hl
                    addByte(0xE1);

                    // 7ed2 xbasic compare string (NULBUF <= string)
                    addCmd(0xCD, def_XBASIC_COMPARE_STRING_WITH_NULBUF_LE);

                    result = Lexeme::subtype_numeric;

                } else
                    result = Lexeme::subtype_unknown;

            } else if(lexeme->value == "+") {

                if( result == Lexeme::subtype_numeric ) {

                    // pop de
                    addByteOptimized(0xD1);
                    // add hl, de      ; add integers
                    addByteOptimized(0x19);

                } else if( result == Lexeme::subtype_single_decimal || result == Lexeme::subtype_double_decimal ) {

                    // pop af
                    addByte(0xF1);
                    // pop de
                    addByte(0xD1);
                    // ld c, a
                    addByte(0x4F);
                    // call 0x76c1     ; add floats (b:hl + c:de = b:hl)
                    addCmd(0xCD, def_XBASIC_ADD_FLOATS);

                } else if( result == Lexeme::subtype_string ) {

                    // pop bc                      ; bc=string 1, hl=string 2
                    addByte(0xC1);
                    // ld de, temporary variable   ; de=string destination
                    addTempStr(false);
                    // push de
                    addByte(0xD5);
                    //   call 0x7f05               ; xbasic concat strings (in: bc=str1, hl=str2, de=strdest; out: hl fake)
                    addCmd(0xCD, def_XBASIC_CONCAT_STRINGS);
                    // pop hl                      ; correct destination
                    addByte(0xE1);

                } else
                    result = Lexeme::subtype_unknown;

            } else if(lexeme->value == "-") {

                if( result == Lexeme::subtype_numeric ) {

                    // pop de
                    addByteOptimized(0xD1);
                    // ex de,hl
                    addByteOptimized(0xEB);

                    s = last_code[0];
                    if(s[0] == 0x11 && s[1] == 0x01 && s[2] == 0x00) {
                        // dec hl
                        s[0] = 0x2B;
                        code_pointer -= 2;
                        code_size -= 2;
                    } else if(s[0] == 0x11 && s[1] == 0x02 && s[2] == 0x00) {
                        // dec hl | dec hl
                        s[0] = 0x2B;
                        s[1] = 0x2B;
                        code_pointer --;
                        code_size --;
                    } else if(s[0] == 0x11 && s[1] == 0x03 && s[2] == 0x00) {
                        // dec hl | dec hl | dec hl
                        s[0] = 0x2B;
                        s[1] = 0x2B;
                        s[2] = 0x2B;
                    } else if(s[0] == 0x11 && s[1] == 0x04 && s[2] == 0x00) {
                        // dec hl | dec hl | dec hl | dec hl
                        s[0] = 0x2B;
                        s[1] = 0x2B;
                        s[2] = 0x2B;
                        addByte(0x2B);
                        /*
                        } else if(s[0] == 0x11 && s[1] == 0x05 && s[2] == 0x00) {
                        // dec hl | dec hl | dec hl | dec hl
                        s[0] = 0x2B;
                        s[1] = 0x2B;
                        s[2] = 0x2B;
                        addByte(0x2B);
                        addByte(0x2B);
                        } else if(s[0] == 0x11 && s[1] == 0x06 && s[2] == 0x00) {
                        // dec hl | dec hl | dec hl | dec hl
                        s[0] = 0x2B;
                        s[1] = 0x2B;
                        s[2] = 0x2B;
                        addByte(0x2B);
                        addByte(0x2B);
                        addByte(0x2B);
                        */
                    } else {
                        // and a
                        addByte(0xA7);
                        // sbc hl, de      ; subtract integers
                        addWord(0xED, 0x52);
                    }

                } else if( result == Lexeme::subtype_single_decimal || result == Lexeme::subtype_double_decimal ) {

                    // pop af
                    addByte(0xF1);
                    // pop de
                    addByte(0xD1);
                    // ex de,hl
                    addByte(0xEB);
                    // ld c, b
                    addByte(0x48);
                    // ld b, a
                    addByte(0x47);
                    // call 0x76bd     ; subtract floats (b:hl - c:de = b:hl)
                    addCmd(0xCD, def_XBASIC_SUBTRACT_FLOATS);

                } else
                    result = Lexeme::subtype_unknown;

            } else if(lexeme->value == "*") {

                if( result == Lexeme::subtype_numeric ) {

                    // pop de
                    addByteOptimized(0xD1);

                    /*
                    s = last_code[0];
                    if(s[0] == 0x11 && s[1] == 0x02 && s[2] == 0x00) {
                        // add hl,hl
                        s[0] = 0x29;
                        code_pointer -= 2;
                        code_size -= 2;
                    } else if(s[0] == 0x11 && s[1] == 0x04 && s[2] == 0x00) {
                        // add hl,hl | add hl,hl
                        s[0] = 0x29;
                        s[1] = 0x29;
                        code_pointer --;
                        code_size --;
                    } else if(s[0] == 0x11 && s[1] == 0x08 && s[2] == 0x00) {
                        // add hl,hl | add hl,hl | add hl,hl
                        s[0] = 0x29;
                        s[1] = 0x29;
                        s[3] = 0x29;
                    } else if(s[0] == 0x11 && s[1] == 16 && s[2] == 0x00) {
                        // add hl,hl | add hl,hl | add hl,hl | add hl,hl
                        s[0] = 0x29;
                        s[1] = 0x29;
                        s[3] = 0x29;
                        addByte(0x29);
                    } else if(s[0] == 0x11 && s[1] == 32 && s[2] == 0x00) {
                        // add hl,hl | add hl,hl | add hl,hl | add hl,hl | add hl,hl
                        s[0] = 0x29;
                        s[1] = 0x29;
                        s[3] = 0x29;
                        addByte(0x29);
                        addByte(0x29);
                    } else if(s[0] == 0x11 && s[1] == 64 && s[2] == 0x00) {
                        // add hl,hl | add hl,hl | add hl,hl | add hl,hl | add hl,hl | add hl,hl
                        s[0] = 0x29;
                        s[1] = 0x29;
                        s[3] = 0x29;
                        addByte(0x29);
                        addByte(0x29);
                        addByte(0x29);
                    } else if(s[0] == 0x11 && s[1] == 128 && s[2] == 0x00) {
                        // XOR A | SRL H | RR L | RRA | LD H, L | LD L, A
                    } else if(s[0] == 0x11 && s[1] == 256 && s[2] == 0x00) {
                        //  LD H, L | LD L, 0
                    } else {
                    */
                    // call 0x761b     ; multiply integers (hl = hl * de)
                    addCmd(0xCD, def_XBASIC_MULTIPLY_INTEGERS);
                    //}

                } else if( result == Lexeme::subtype_single_decimal || result == Lexeme::subtype_double_decimal ) {

                    // pop af
                    addByte(0xF1);
                    // pop de
                    addByte(0xD1);
                    // ld c, a
                    addByte(0x4F);
                    // call 0x7732     ; multiply floats
                    addCmd(0xCD, def_XBASIC_MULTIPLY_FLOATS);

                } else
                    result = Lexeme::subtype_unknown;

            } else if(lexeme->value == "/") {

                if( result == Lexeme::subtype_numeric ) {

                    // pop de
                    addByteOptimized(0xD1);
                    // ex de,hl
                    addByteOptimized(0xEB);

                    /*
                    s = last_code[0];
                    if(s[0] == 0x11 && s[1] == 0x02 && s[2] == 0x00) {
                        // srl h | rr l
                        s[0] = 0xCB;
                        s[1] = 0x3C;
                        s[2] = 0xCB;
                        addByte(0x1D);
                    } else if(s[0] == 0x11 && s[1] == 0x04 && s[2] == 0x00) {
                        // srl h | rr l | srl h | rr l
                        s[0] = 0xCB;
                        s[1] = 0x3C;
                        s[2] = 0xCB;
                        addByte(0x1D);
                        addWord(0xCB, 0x3C);
                        addWord(0xCB, 0x1D);
                    } else if(s[0] == 0x11 && s[1] == 0x08 && s[2] == 0x00) {
                        // LD A, L | SRL H | RRA | SRL H | RRA | SRL H | RRA | LD L, A
                    } else if(s[0] == 0x11 && s[1] == 16 && s[2] == 0x00) {
                        // XOR A | ADD HL, HL | RLA | ADD HL, HL | RLA | ADD HL, HL | RLA | ADD HL, HL | RLA | LD L, H | LD H, A
                    } else if(s[0] == 0x11 && s[1] == 32 && s[2] == 0x00) {
                        // XOR A | ADD HL, HL | RLA | ADD HL, HL | RLA | ADD HL, HL | RLA | LD L, H | LD H, A
                    } else if(s[0] == 0x11 && s[1] == 64 && s[2] == 0x00) {
                        // XOR A | ADD HL, HL | RLA | ADD HL, HL | RLA | LD L, H | LD H, A
                    } else if(s[0] == 0x11 && s[1] == 128 && s[2] == 0x00) {
                        // XOR A | ADD HL, HL | RLA | LD L, H | LD H, A
                    } else {
                    */
                    // call 0x762d     ; divide integers
                    addCmd(0xCD, def_XBASIC_DIVIDE_INTEGERS);
                    //}

                } else if( result == Lexeme::subtype_single_decimal || result == Lexeme::subtype_double_decimal ) {

                    // pop af
                    addByte(0xF1);
                    // pop de
                    addByte(0xD1);
                    // ex de,hl
                    addByte(0xEB);
                    // ld c, b
                    addByte(0x48);
                    // ld b, a
                    addByte(0x47);
                    // call 0x7775     ; divide floats
                    addCmd(0xCD, def_XBASIC_DIVIDE_FLOATS);

                } else
                    result = Lexeme::subtype_unknown;

            } else if(lexeme->value == "\\") {

                if( result == Lexeme::subtype_numeric ) {

                    // pop de
                    addByteOptimized(0xD1);
                    // ex de,hl
                    addByteOptimized(0xEB);
                    // call 0x762d     ; divide integers
                    addCmd(0xCD, def_XBASIC_DIVIDE_INTEGERS);

                } else if( result == Lexeme::subtype_single_decimal || result == Lexeme::subtype_double_decimal ) {

                    // cast
                    addCast(result, Lexeme::subtype_numeric);

                    // pop bc
                    addByte(0xC1);
                    // pop de
                    addByte(0xD1);
                    // push hl
                    addByte(0xE5);
                    // ex de, hl
                    addByte(0xEB);

                    // cast
                    addCast(result, Lexeme::subtype_numeric);

                    // pop de
                    addByte(0xD1);

                    // call 0x762d     ; divide integers
                    addCmd(0xCD, def_XBASIC_DIVIDE_INTEGERS);

                    result = Lexeme::subtype_numeric;

                } else
                    result = Lexeme::subtype_unknown;

            } else if(lexeme->value == "^") {

                if( result == Lexeme::subtype_numeric ) {

                    // ex de,hl
                    addByteOptimized(0xEB);

                    // pop hl
                    addByte(0xE1);

                    //   call 0x782D     ; integer to float
                    addCmd(0xCD, def_XBASIC_CAST_INTEGER_TO_FLOAT);

                    // call 0x77C1       ; power float ^ integer
                    addCmd(0xCD, def_XBASIC_POWER_FLOAT_TO_INTEGER);

                    result = Lexeme::subtype_single_decimal;

                } else if( result == Lexeme::subtype_single_decimal || result == Lexeme::subtype_double_decimal ) {

                    // pop af
                    addByte(0xF1);
                    // pop de
                    addByte(0xD1);
                    // ex de,hl
                    addByte(0xEB);
                    // ld c, b
                    addByte(0x48);
                    // ld b, a
                    addByte(0x47);
                    // call 0x780d      ; power float ^ float
                    addCmd(0xCD, def_XBASIC_POWER_FLOAT_TO_FLOAT);

                } else
                    result = Lexeme::subtype_unknown;

            } else if(lexeme->value == "MOD") {

                if( result == Lexeme::subtype_numeric ) {

                    // pop de
                    addByteOptimized(0xD1);
                    // ex de,hl
                    addByteOptimized(0xEB);
                    // call 0x762d     ; divide integers
                    addCmd(0xCD, def_XBASIC_DIVIDE_INTEGERS);
                    // ex de, hl       ; remainder
                    addByte(0xEB);

                } else if( result == Lexeme::subtype_single_decimal || result == Lexeme::subtype_double_decimal ) {

                    // cast
                    addCast(result, Lexeme::subtype_numeric);

                    // pop bc
                    addByte(0xC1);
                    // pop de
                    addByte(0xD1);
                    // push hl
                    addByte(0xE5);
                    // ex de, hl
                    addByte(0xEB);

                    // cast
                    addCast(result, Lexeme::subtype_numeric);

                    // pop de
                    addByte(0xD1);

                    // call 0x762d     ; divide integers
                    addCmd(0xCD, def_XBASIC_DIVIDE_INTEGERS);

                    // ex de, hl       ; remainder
                    addByte(0xEB);

                    result = Lexeme::subtype_numeric;

                } else
                    result = Lexeme::subtype_unknown;

            } else if(lexeme->value == "SHR") {

                if( result == Lexeme::subtype_numeric ) {

                    // pop de
                    addByteOptimized(0xD1);

                    // call intSHR
                    addCmd(0xCD, def_intSHR);

                } else
                    result = Lexeme::subtype_unknown;

            } else if(lexeme->value == "SHL") {

                if( result == Lexeme::subtype_numeric ) {

                    // pop de
                    addByteOptimized(0xD1);

                    // call intSHL
                    addCmd(0xCD, def_intSHL);

                } else
                    result = Lexeme::subtype_unknown;

            } else
                result = Lexeme::subtype_unknown;

        } else
            result = Lexeme::subtype_unknown;


    }

    return result;
}

bool Compiler::evalOperatorParms(ActionNode *action, int parmCount) {
    bool result = false;
    int subtype;
    ActionNode *next_action;
    int i, t = action->actions.size();

    if(t == parmCount) {
        result = true;
        for(i = t - 1; i >= 0 && result; i--) {
            next_action = action->actions[i];

            subtype = evalExpression(next_action);
            result &= (subtype != Lexeme::subtype_unknown);

            next_action->subtype = subtype;

            if(i) {
                // push hl
                addByte(0xE5);
                if(subtype == Lexeme::subtype_single_decimal || subtype == Lexeme::subtype_double_decimal) {
                    // push bc
                    addByte(0xC5);
                }
            }
        }
    }

    return result;
}

int Compiler::evalOperatorCast(ActionNode *action) {
    int result = Lexeme::subtype_unknown;
    ActionNode *next_action1, *next_action2;

    next_action1 = action->actions[0];
    next_action2 = action->actions[1];

    if(next_action1->subtype == next_action2->subtype) {

        result = next_action1->subtype;

    } else if( next_action2->subtype == Lexeme::subtype_numeric &&
               (next_action1->subtype == Lexeme::subtype_single_decimal || next_action1->subtype == Lexeme::subtype_double_decimal) ) {

        // ex: (float) + (int)
        //     200.0 + 100

        // call castParamFloatInt
        addCmd(0xCD, def_castParamFloatInt);

        result = next_action1->subtype;

    } else if( next_action1->subtype == Lexeme::subtype_numeric &&
               (next_action2->subtype == Lexeme::subtype_single_decimal || next_action2->subtype == Lexeme::subtype_double_decimal) ) {

        // ex: (int) + (float)
        // ex: 200 + 100.0

        // cast
        addCast(next_action1->subtype, next_action2->subtype);

        result = next_action2->subtype;

    } else if( next_action2->subtype == Lexeme::subtype_numeric && next_action1->subtype == Lexeme::subtype_string) {

        // pop de   ; swap parameters code
        addByte(0xD1);
        // push hl
        addByte(0xE5);
        // ex de,hl
        addByte(0xEB);

        // cast
        addCast(next_action1->subtype, next_action2->subtype);

        // pop de   ; swap again
        addByte(0xD1);
        // push hl
        addByte(0xE5);
        // ex de,hl
        addByte(0xEB);

        result = next_action2->subtype;

    } else if( next_action1->subtype == Lexeme::subtype_numeric && next_action2->subtype == Lexeme::subtype_string) {

        // cast
        addCast(next_action1->subtype, next_action2->subtype);

        result = next_action2->subtype;

    } else if( (next_action2->subtype == Lexeme::subtype_single_decimal || next_action2->subtype == Lexeme::subtype_double_decimal) &&
               next_action1->subtype == Lexeme::subtype_string) {

        result = Lexeme::subtype_unknown;

    } else if( (next_action1->subtype == Lexeme::subtype_single_decimal || next_action1->subtype == Lexeme::subtype_double_decimal) &&
               next_action2->subtype == Lexeme::subtype_string) {

        // cast
        addCast(next_action1->subtype, next_action2->subtype);

        result = next_action2->subtype;

    } else if( (next_action1->subtype == Lexeme::subtype_single_decimal && next_action2->subtype == Lexeme::subtype_double_decimal) ||
               (next_action2->subtype == Lexeme::subtype_single_decimal && next_action1->subtype == Lexeme::subtype_double_decimal) ) {

        result = Lexeme::subtype_double_decimal;

    } else
        result = Lexeme::subtype_unknown;

    return result;
}

int Compiler::evalFunction(ActionNode *action) {
    int result[4];
    Lexeme *lexeme, *lexeme2;
    ActionNode *next_action;
    unsigned int i, t = action->actions.size();

    for(i = 0; i < 4; i++)
        result[i] = Lexeme::subtype_unknown;

    lexeme = action->lexeme;

    if(lexeme) {

        if(t) {

            if(lexeme->value == "VARPTR") {
                next_action = action->actions[0];
                if(next_action->lexeme->type == Lexeme::type_identifier) {
                    addVarAddress(next_action);
                    return Lexeme::subtype_numeric;
                } else
                    return Lexeme::subtype_unknown;
            } else if(lexeme->value == "USING$") {
                if(t>=2) {
                    next_action = action->actions[1];
                    lexeme2 = next_action->lexeme;
                    if(lexeme2) {
                        if(lexeme2->type == Lexeme::type_literal && lexeme2->subtype == Lexeme::subtype_string) {
                            int r = getUsingFormat(lexeme2->value);
                            lexeme2->subtype = Lexeme::subtype_numeric;
                            lexeme2->value = to_string(r);
                        }
                    }
                }
            }

            if(!evalOperatorParms(action, t))
                return result[0];

            for(i = 0; i<t; i++) {
                next_action = action->actions[i];
                result[i] = next_action->subtype;

                if(result[i] == Lexeme::subtype_unknown)
                    return result[i];

            }
        }

        switch(t) {
            case 0: {

                    if(lexeme->value == "TIME") {

                        // ld hl, (0xFC9E)    ; JIFFY
                        addCmd(0x2A, 0xFC9E);
                        result[0] = Lexeme::subtype_numeric;

                    } else if(lexeme->value == "POS") {

                        // ld hl, (0xF661)  ; TTYPOS
                        addCmd(0x2A, 0xF661);
                        // ld h, 0
                        addWord(0x26, 0x00);
                        result[0] = Lexeme::subtype_numeric;

                    } else if(lexeme->value == "LPOS") {

                        // ld hl, (0xF415)  ; LPTPOS
                        addCmd(0x2A, 0xF415);
                        // ld h, 0
                        addWord(0x26, 0x00);
                        result[0] = Lexeme::subtype_numeric;

                    } else if(lexeme->value == "CSRLIN") {

                        // ld hl, (0xF3DC)  ; CSRY
                        addCmd(0x2A, def_CSRY);
                        // ld h, 0
                        addWord(0x26, 0x00);
                        result[0] = Lexeme::subtype_numeric;

                    } else if(lexeme->value == "INKEY") {

                        // ld hl, 0
                        addCmd(0x21, 0x0000);
                        // call 0x009C        ; CHSNS
                        addCmd(0xCD, 0x009C);
                        // jr z,$+5
                        addWord(0x28,0x04);
                        //   call 0x009F        ; CHGET
                        addCmd(0xCD, 0x009F);
                        //   ld l, a
                        addByte(0x6F);

                        result[0] = Lexeme::subtype_numeric;

                    } else if(lexeme->value == "INKEY$") {

                        // call 0x009C        ; CHSNS
                        addCmd(0xCD, 0x009C);
                        // ld hl, temporary string
                        addTempStr(true);
                        // call 0x7e5e   ; xbasic INKEY$ (in: hl=dest; out: hl=result)
                        addCmd(0xCD, def_XBASIC_INKEY);

                        result[0] = Lexeme::subtype_string;

                    } else if(lexeme->value == "MAXFILES") {

                        // ld hl, (MAXFIL)
                        addCmd(0x2A, 0xF85F);

                        result[0] = Lexeme::subtype_numeric;

                    } else if(lexeme->value == "FRE") {

                        // ld hl, (HEAPSIZ)
                        addCmd(0x2A, def_HEAPSIZ);

                        result[0] = Lexeme::subtype_numeric;

                    } else if(lexeme->value == "HEAP") {

                        // ld hl, (HEAPSTR)
                        addCmd(0x2A, def_HEAPSTR);

                        result[0] = Lexeme::subtype_numeric;

                    } else if(lexeme->value == "MSX") {

                        // ld hl, (VERSION)                    ; 0 = MSX1, 1 = MSX2, 2 = MSX2+, 3 = MSXturboR
                        addCmd(0x2A, def_VERSION);
                        // ld h, 0
                        addWord(0x26, 0x00);

                        result[0] = Lexeme::subtype_numeric;

                    } else if(lexeme->value == "NTSC") {

                        // ld hl, 0
                        addCmd(0x21, 0x0000);
                        // ld a, (NTSC)
                        addCmd(0x3A, def_NTSC);
                        // and 128   ; bit 7 on?
                        addWord(0xE6, 0x80);
                        // jr nz, $+1
                        addWord(0x20, 0x01);
                        //    dec hl
                        addByte(0x2B);

                        result[0] = Lexeme::subtype_numeric;

                    } else if(lexeme->value == "VDP") {

                        // ld a, 4
                        addWord(0x3e, 4);
                        // CALL USR2
                        addCmd(0xCD, def_usr2+1);

                        result[0] = Lexeme::subtype_numeric;

                    } else if(lexeme->value == "TURBO") {

                        // ld a, 5
                        addWord(0x3e, 5);
                        // CALL USR2
                        addCmd(0xCD, def_usr2+1);

                        result[0] = Lexeme::subtype_numeric;

                    } else if(lexeme->value == "COLLISION") {

                        // CALL SUB_SPRCOL_ALL
                        addCmd(0xCD, def_usr3_COLLISION_ALL);

                        result[0] = Lexeme::subtype_numeric;


                    } else if(lexeme->value == "MAKER") {

                        // ld a, 6
                        addWord(0x3e, 6);
                        // CALL USR2
                        addCmd(0xCD, def_usr2+1);

                        result[0] = Lexeme::subtype_numeric;

                    } else if(lexeme->value == "PLYSTATUS") {

                        // CALL usr2_player_status
                        addCmd(0xCD, def_usr2_player_status);

                        result[0] = Lexeme::subtype_numeric;

                    } else
                        result[0] = Lexeme::subtype_unknown;

                }
                break;

            case 1 : {

                    if(lexeme->value == "INT") {

                        if( result[0] == Lexeme::subtype_numeric ) {

                            // its ok, return same parameter value
                            return result[0];

                        } else if( result[0] == Lexeme::subtype_single_decimal || result[0] == Lexeme::subtype_double_decimal ) {

                            // call 0x78e5         ; xbasic INT
                            addCmd(0xCD, def_XBASIC_INT);

                        } else
                            result[0] = Lexeme::subtype_unknown;

                    } else if(lexeme->value == "FIX") {

                        if( result[0] == Lexeme::subtype_numeric ) {

                            // cast
                            addCast(result[0], Lexeme::subtype_single_decimal);
                            result[0] = Lexeme::subtype_single_decimal;

                        }

                        if( result[0] == Lexeme::subtype_single_decimal || result[0] == Lexeme::subtype_double_decimal ) {

                            // call 0x78d8         ; xbasic FIX (in b:hl, out b:hl)
                            addCmd(0xCD, def_XBASIC_FIX);

                        } else
                            result[0] = Lexeme::subtype_unknown;

                    } else if(lexeme->value == "RND") {

                        if( result[0] == Lexeme::subtype_numeric ) {

                            // cast
                            addCast(result[0], Lexeme::subtype_single_decimal);
                            result[0] = Lexeme::subtype_single_decimal;

                        }

                        if( result[0] == Lexeme::subtype_single_decimal || result[0] == Lexeme::subtype_double_decimal ) {

                            // call 0x7678         ; xbasic RND (in b:hl, out b:hl)
                            addCmd(0xCD, def_XBASIC_RND);

                        } else
                            result[0] = Lexeme::subtype_unknown;

                    } else if(lexeme->value == "SIN") {

                        if( result[0] == Lexeme::subtype_numeric ) {

                            // cast
                            addCast(result[0], Lexeme::subtype_single_decimal);
                            result[0] = Lexeme::subtype_single_decimal;

                        }

                        if( result[0] == Lexeme::subtype_single_decimal || result[0] == Lexeme::subtype_double_decimal ) {

                            // call 0x7936         ; xbasic SIN (in b:hl, out b:hl)
                            addCmd(0xCD, def_XBASIC_SIN);

                        } else
                            result[0] = Lexeme::subtype_unknown;

                    } else if(lexeme->value == "COS") {

                        if( result[0] == Lexeme::subtype_numeric ) {

                            // cast
                            addCast(result[0], Lexeme::subtype_single_decimal);
                            result[0] = Lexeme::subtype_single_decimal;

                        }

                        if( result[0] == Lexeme::subtype_single_decimal || result[0] == Lexeme::subtype_double_decimal ) {

                            // call 0x792e         ; xbasic COS (in b:hl, out b:hl)
                            addCmd(0xCD, def_XBASIC_COS);

                        } else
                            result[0] = Lexeme::subtype_unknown;

                    } else if(lexeme->value == "TAN") {

                        if( result[0] == Lexeme::subtype_numeric ) {

                            // cast
                            addCast(result[0], Lexeme::subtype_single_decimal);
                            result[0] = Lexeme::subtype_single_decimal;

                        }

                        if( result[0] == Lexeme::subtype_single_decimal || result[0] == Lexeme::subtype_double_decimal ) {

                            // call 0x7990         ; xbasic TAN (in b:hl, out b:hl)
                            addCmd(0xCD, def_XBASIC_TAN);

                        } else
                            result[0] = Lexeme::subtype_unknown;

                    } else if(lexeme->value == "ATN") {

                        if( result[0] == Lexeme::subtype_numeric ) {

                            // cast
                            addCast(result[0], Lexeme::subtype_single_decimal);
                            result[0] = Lexeme::subtype_single_decimal;

                        }

                        if( result[0] == Lexeme::subtype_single_decimal || result[0] == Lexeme::subtype_double_decimal ) {

                            // call 0x79b2         ; xbasic ATN (in b:hl, out b:hl)
                            addCmd(0xCD, def_XBASIC_ATN);

                        } else
                            result[0] = Lexeme::subtype_unknown;

                    } else if(lexeme->value == "EXP") {

                        if( result[0] == Lexeme::subtype_numeric ) {

                            // cast
                            addCast(result[0], Lexeme::subtype_single_decimal);
                            result[0] = Lexeme::subtype_single_decimal;

                        }

                        if( result[0] == Lexeme::subtype_single_decimal || result[0] == Lexeme::subtype_double_decimal ) {

                            // call 0x79fa         ; xbasic EXP (in b:hl, out b:hl)
                            addCmd(0xCD, def_XBASIC_EXP);

                        } else
                            result[0] = Lexeme::subtype_unknown;

                    } else if(lexeme->value == "LOG") {

                        if( result[0] == Lexeme::subtype_numeric ) {

                            // cast
                            addCast(result[0], Lexeme::subtype_single_decimal);
                            result[0] = Lexeme::subtype_single_decimal;

                        }

                        if( result[0] == Lexeme::subtype_single_decimal || result[0] == Lexeme::subtype_double_decimal ) {

                            // call 0x7a53         ; xbasic LOG (in b:hl, out b:hl)
                            addCmd(0xCD, def_XBASIC_LOG);

                        } else
                            result[0] = Lexeme::subtype_unknown;

                    } else if(lexeme->value == "SQR") {

                        if( result[0] == Lexeme::subtype_numeric ) {

                            // cast
                            addCast(result[0], Lexeme::subtype_single_decimal);
                            result[0] = Lexeme::subtype_single_decimal;

                        }

                        if( result[0] == Lexeme::subtype_single_decimal || result[0] == Lexeme::subtype_double_decimal ) {

                            // call 0x7ab5         ; xbasic SQR (in b:hl, out b:hl)
                            addCmd(0xCD, def_XBASIC_SQR);

                        } else
                            result[0] = Lexeme::subtype_unknown;

                    } else if(lexeme->value == "SGN") {

                        if( result[0] == Lexeme::subtype_numeric ) {

                            // call 0x5b5d         ; xbasic SGN (in hl, out hl)
                            addCmd(0xCD, def_XBASIC_SGN_INT);

                        } else if( result[0] == Lexeme::subtype_single_decimal || result[0] == Lexeme::subtype_double_decimal ) {

                            // call 0x5b72         ; xbasic SGN (in b:hl, out b:hl)
                            addCmd(0xCD, def_XBASIC_SGN_FLOAT);

                        } else
                            result[0] = Lexeme::subtype_unknown;

                    } else if(lexeme->value == "ABS") {

                        if( result[0] == Lexeme::subtype_numeric ) {

                            // call 0x5b36         ; xbasic ABS (in hl, out hl)
                            addCmd(0xCD, def_XBASIC_ABS_INT);

                        } else if( result[0] == Lexeme::subtype_single_decimal || result[0] == Lexeme::subtype_double_decimal ) {

                            // xbasic ABS (in b:hl, out b:hl)
                            // res 7,h
                            addWord(0xCB, 0xBC);

                        } else
                            result[0] = Lexeme::subtype_unknown;

                    } else if(lexeme->value == "VAL") {

                        if( result[0] == Lexeme::subtype_numeric ) {

                            // its ok, return same parameter value
                            return result[0];

                        } else if( result[0] == Lexeme::subtype_single_decimal || result[0] == Lexeme::subtype_double_decimal ) {

                            // its ok, return same parameter value
                            return result[0];

                        } else if( result[0] == Lexeme::subtype_string ) {

                            // call 0x7e07   ; VAL function - xbasic string to float (in hl, out b:hl)
                            addCmd(0xCD, def_XBASIC_VAL);
                            result[0] = Lexeme::subtype_single_decimal;

                        } else
                            result[0] = Lexeme::subtype_unknown;

                    } else if(lexeme->value == "PEEK") {

                        if( result[0] == Lexeme::subtype_single_decimal || result[0] == Lexeme::subtype_double_decimal ) {
                            // cast
                            addCast( result[0], Lexeme::subtype_numeric );
                            result[0] = Lexeme::subtype_numeric;
                        }

                        if( result[0] == Lexeme::subtype_numeric ) {

                            // ld l,(hl)
                            addByte(0x6E);
                            // ld h, 0
                            addWord(0x26, 0x00);

                        } else
                            result[0] = Lexeme::subtype_unknown;

                    } else if(lexeme->value == "VPEEK") {

                        if( result[0] == Lexeme::subtype_single_decimal || result[0] == Lexeme::subtype_double_decimal ) {
                            // cast
                            addCast( result[0], Lexeme::subtype_numeric );
                            result[0] = Lexeme::subtype_numeric;
                        }

                        if( result[0] == Lexeme::subtype_numeric ) {

                            // call 0x70a1    ; xbasic VPEEK (in:hl, out:hl)
                            addCmd(0xCD, def_XBASIC_VPEEK);

                        } else
                            result[0] = Lexeme::subtype_unknown;

                    } else if(lexeme->value == "INP") {

                        if( result[0] == Lexeme::subtype_single_decimal || result[0] == Lexeme::subtype_double_decimal ) {
                            // cast
                            addCast( result[0], Lexeme::subtype_numeric );
                            result[0] = Lexeme::subtype_numeric;
                        }

                        if( result[0] == Lexeme::subtype_numeric ) {

                            // ld c, l
                            addByte(0x4D);
                            // in a, (c)
                            addWord(0xED, 0x78);
                            // ld l, a
                            addByte(0x6F);
                            // ld h, 0
                            addWord(0x26, 0x00);

                        } else
                            result[0] = Lexeme::subtype_unknown;

                    } else if(lexeme->value == "EOF") {

                        if( result[0] == Lexeme::subtype_single_decimal || result[0] == Lexeme::subtype_double_decimal ) {
                            // cast
                            addCast( result[0], Lexeme::subtype_numeric );
                            result[0] = Lexeme::subtype_numeric;
                        }

                        if( result[0] == Lexeme::subtype_numeric ) {

                            // ld (DAC+2), hl
                            addCmd(0x22, def_DAC+2);

                            addEnableBasicSlot();

                            // call HSAVD          ; alloc disk
                            addCmd(0xCD, 0xFE94);
                            // call GETIOBLK       ; get io channel control block from DAC
                            addCmd(0xCD, 0x6A6A);
                            // jr z, $+6           ; file not open
                            addWord(0x28, 0x05);
                            // jr c, $+4           ; not a disk drive device
                            addWord(0x38, 0x03);
                            // call HEOF           ; put in DAC end of file status
                            addCmd(0xCD, 0xFEA3);

                            addDisableBasicSlot();

                            // ld hl, (DAC+2)
                            addCmd(0x2A, def_DAC+2);

                        } else
                            result[0] = Lexeme::subtype_unknown;

                    } else if(lexeme->value == "VDP") {

                        if( result[0] == Lexeme::subtype_single_decimal || result[0] == Lexeme::subtype_double_decimal ) {
                            // cast
                            addCast( result[0], Lexeme::subtype_numeric );
                            result[0] = Lexeme::subtype_numeric;
                        }

                        if( result[0] == Lexeme::subtype_numeric ) {

                            // call 0x7337         ; xbasic VDP (in: hl, out: hl)
                            addCmd(0xCD, def_XBASIC_VDP);

                        } else
                            result[0] = Lexeme::subtype_unknown;

                    } else if(lexeme->value == "PSG") {

                        if( result[0] == Lexeme::subtype_single_decimal || result[0] == Lexeme::subtype_double_decimal ) {
                            // cast
                            addCast( result[0], Lexeme::subtype_numeric );
                            result[0] = Lexeme::subtype_numeric;
                        }

                        if( result[0] == Lexeme::subtype_numeric ) {

                            // ld a, l
                            addByte(0x7D);
                            // cp 16
                            addWord(0xFE, 0x10);
                            // jr nc, $+4
                            addWord(0x30, 0x03);
                            //   call 0x0096         ; RDPSG (in: a = PSG register)
                            addCmd(0xCD, 0x0096);
                            //   ld l, a
                            addByte(0x6F);
                            //   ld h, 0
                            addWord(0x26, 0x00);

                        } else
                            result[0] = Lexeme::subtype_unknown;

                    } else if(lexeme->value == "PLAY") {

                        if( result[0] == Lexeme::subtype_single_decimal || result[0] == Lexeme::subtype_double_decimal ) {
                            // cast
                            addCast( result[0], Lexeme::subtype_numeric );
                            result[0] = Lexeme::subtype_numeric;
                        }

                        if( result[0] == Lexeme::subtype_numeric ) {

                            // call usr2_play
                            addCmd(0xCD, def_usr2_play);

                        } else
                            result[0] = Lexeme::subtype_unknown;

                    } else if(lexeme->value == "STICK") {

                        if( result[0] == Lexeme::subtype_single_decimal || result[0] == Lexeme::subtype_double_decimal ) {
                            // cast
                            addCast( result[0], Lexeme::subtype_numeric );
                            result[0] = Lexeme::subtype_numeric;
                        }

                        if( result[0] == Lexeme::subtype_numeric ) {

                            // ld a, l
                            addByte(0x7D);
                            // call 0x00D5      ; GTSTCK
                            addCmd(0xCD, 0x00D5);
                            // ld h, 0
                            addWord(0x26,00);
                            // ld l, a
                            addByte(0x6F);

                        } else
                            result[0] = Lexeme::subtype_unknown;

                    } else if(lexeme->value == "STRIG") {

                        if( result[0] == Lexeme::subtype_single_decimal || result[0] == Lexeme::subtype_double_decimal ) {
                            // cast
                            addCast( result[0], Lexeme::subtype_numeric );
                            result[0] = Lexeme::subtype_numeric;
                        }

                        if( result[0] == Lexeme::subtype_numeric ) {

                            // ld a, l
                            addByte(0x7D);
                            // call 0x00D8      ; GTTRIG
                            addCmd(0xCD, 0x00D8);
                            // ld h, a
                            addByte(0x67);
                            // ld l, a
                            addByte(0x6F);

                        } else
                            result[0] = Lexeme::subtype_unknown;

                    } else if(lexeme->value == "PAD") {

                        if( result[0] == Lexeme::subtype_single_decimal || result[0] == Lexeme::subtype_double_decimal ) {
                            // cast
                            addCast( result[0], Lexeme::subtype_numeric );
                            result[0] = Lexeme::subtype_numeric;
                        }

                        if( result[0] == Lexeme::subtype_numeric ) {

                            // ld a, l
                            addByte(0x7D);
                            // call 0x00DB      ; GTPAD
                            addCmd(0xCD, 0x00DB);
                            // ld h, 0
                            addWord(0x26,00);
                            // ld l, a
                            addByte(0x6F);

                        } else
                            result[0] = Lexeme::subtype_unknown;

                    } else if(lexeme->value == "PDL") {

                        if( result[0] == Lexeme::subtype_single_decimal || result[0] == Lexeme::subtype_double_decimal ) {
                            // cast
                            addCast( result[0], Lexeme::subtype_numeric );
                            result[0] = Lexeme::subtype_numeric;
                        }

                        if( result[0] == Lexeme::subtype_numeric ) {

                            // ld a, l
                            addByte(0x7D);
                            // call 0x00DE      ; GTPDL
                            addCmd(0xCD, 0x00DE);
                            // ld h, 0
                            addWord(0x26,00);
                            // ld l, a
                            addByte(0x6F);

                        } else
                            result[0] = Lexeme::subtype_unknown;

                    } else if(lexeme->value == "BASE") {

                        if( result[0] == Lexeme::subtype_single_decimal || result[0] == Lexeme::subtype_double_decimal ) {
                            // cast
                            addCast( result[0], Lexeme::subtype_numeric );
                            result[0] = Lexeme::subtype_numeric;
                        }

                        if( result[0] == Lexeme::subtype_numeric ) {

                            // call base function
                            addCmd(0xCD, def_XBASIC_BASE);
                            addCmd(0xCD, def_XBASIC_ABS_INT);  // abs()

                        } else
                            result[0] = Lexeme::subtype_unknown;

                    } else if(lexeme->value == "ASC") {

                        if( result[0] == Lexeme::subtype_string ) {

                            // ex de, hl
                            addByteOptimized(0xEB);
                            // ld hl, 0
                            addCmd(0x21, 0x0000);
                            // ld a, (de)
                            addByte(0x1A);
                            // and a
                            addByte(0xA7);
                            // jr z,$+4
                            addWord(0x28, 0x03);
                            //   inc de
                            addByte(0x13);
                            //   ld a, (de)
                            addByte(0x1A);
                            //   ld l, a
                            addByte(0x6f);

                            result[0] = Lexeme::subtype_numeric;

                        } else
                            result[0] = Lexeme::subtype_unknown;

                    } else if(lexeme->value == "LEN") {

                        if( result[0] == Lexeme::subtype_string ) {

                            // ld l, (hl)
                            addByte(0x6e);
                            // ld h, 0
                            addWord(0x26, 0x00);

                            result[0] = Lexeme::subtype_numeric;

                        } else
                            result[0] = Lexeme::subtype_unknown;

                    } else if(lexeme->value == "CSNG") {

                        // cast
                        addCast(result[0], Lexeme::subtype_single_decimal);

                        result[0] = Lexeme::subtype_single_decimal;

                    } else if(lexeme->value == "CDBL") {

                        // cast
                        addCast(result[0], Lexeme::subtype_double_decimal);

                        result[0] = Lexeme::subtype_double_decimal;

                    } else if(lexeme->value == "CINT") {

                        // cast
                        addCast(result[0], Lexeme::subtype_numeric);

                        result[0] = Lexeme::subtype_numeric;

                    } else if(lexeme->value == "POS") {

                        // ld hl, (0xF661)  ; TTYPOS
                        addCmd(0x2A, 0xF661);
                        // ld h, 0
                        addWord(0x26, 0x00);
                        result[0] = Lexeme::subtype_numeric;

                    } else if(lexeme->value == "LPOS") {

                        // ld hl, (0xF415)  ; LPTPOS
                        addCmd(0x2A, 0xF415);
                        // ld h, 0
                        addWord(0x26, 0x00);
                        result[0] = Lexeme::subtype_numeric;

                    } else if(lexeme->value == "CSRLIN") {

                        // ld hl, (0xF3DC)  ; CSRY
                        addCmd(0x2A, 0xF3DC);
                        // ld h, 0
                        addWord(0x26, 0x00);
                        result[0] = Lexeme::subtype_numeric;

                    } else if(lexeme->value == "CHR$") {

                        if( result[0] == Lexeme::subtype_single_decimal || result[0] == Lexeme::subtype_double_decimal ) {
                            // cast
                            addCast( result[0], Lexeme::subtype_numeric );
                            result[0] = Lexeme::subtype_numeric;
                        }

                        if( result[0] == Lexeme::subtype_numeric ) {

                            // ex de, hl
                            addByteOptimized(0xEB);
                            // ld hl, temporary string
                            addTempStr(true);
                            // inc d
                            addByte(0x14);
                            // ld (hl), d
                            addByte(0x72);
                            // inc hl
                            addByte(0x23);
                            // ld (hl), e
                            addByte(0x73);
                            // dec hl
                            addByte(0x2B);

                            result[0] = Lexeme::subtype_string;

                        } else
                            result[0] = Lexeme::subtype_unknown;

                    } else if(lexeme->value == "SPACE$" || lexeme->value == "SPC") {

                        if( result[0] == Lexeme::subtype_single_decimal || result[0] == Lexeme::subtype_double_decimal ) {
                            // cast
                            addCast( result[0], Lexeme::subtype_numeric );
                            result[0] = Lexeme::subtype_numeric;
                        }

                        if( result[0] == Lexeme::subtype_numeric ) {

                            // ld a, 0x20      ; space
                            addWord(0x3e, 0x20);
                            // ld b, l
                            addByte(0x45);
                            // ld hl, temporary string
                            addTempStr(true);
                            // call 0x7e4c    ; STRING$ (hl=destination, b=number of chars, a=char)
                            addCmd(0xCD, def_XBASIC_STRING);

                            result[0] = Lexeme::subtype_string;

                        } else
                            result[0] = Lexeme::subtype_unknown;

                    } else if(lexeme->value == "TAB") {

                        if( result[0] == Lexeme::subtype_single_decimal || result[0] == Lexeme::subtype_double_decimal ) {
                            // cast
                            addCast( result[0], Lexeme::subtype_numeric );
                            result[0] = Lexeme::subtype_numeric;
                        }

                        if( result[0] == Lexeme::subtype_numeric ) {

                            // call tab function
                            addCmd(0xCD, def_XBASIC_TAB);

                            result[0] = Lexeme::subtype_string;

                        } else
                            result[0] = Lexeme::subtype_unknown;

                    } else if(lexeme->value == "STR$") {

                        // cast
                        addCast(result[0], Lexeme::subtype_string);

                        result[0] = Lexeme::subtype_string;

                    } else if(lexeme->value == "INPUT$") {

                        if( result[0] == Lexeme::subtype_single_decimal || result[0] == Lexeme::subtype_double_decimal ) {
                            addCast( result[0], Lexeme::subtype_numeric );
                            result[0] = Lexeme::subtype_numeric;
                        }

                        if( result[0] == Lexeme::subtype_numeric ) {

                            // ex de, hl
                            addByteOptimized(0xEB);
                            // ld hl, temporary string
                            addTempStr(true);
                            // push hl
                            addByte(0xE5);
                            //   ld (hl), e
                            addByte(0x73);
                            //   inc hl
                            addByte(0x23);
                            //     call 0x009F        ; CHGET
                            addCmd(0xCD, 0x009F);
                            //     ld (hl), a
                            addByte(0x77);
                            //     inc hl
                            addByte(0x23);
                            //     dec e
                            addByte(0x1D);
                            //   jr nz,$-8
                            addWord(0x20,0xFF-7);
                            // pop hl
                            addByte(0xE1);

                            result[0] = Lexeme::subtype_string;

                        } else
                            result[0] = Lexeme::subtype_unknown;

                    } else if(lexeme->value == "BIN$") {

                        if( result[0] == Lexeme::subtype_single_decimal || result[0] == Lexeme::subtype_double_decimal ) {
                            // cast
                            addCast( result[0], Lexeme::subtype_numeric );
                            result[0] = Lexeme::subtype_numeric;
                        }

                        if( result[0] == Lexeme::subtype_numeric ) {

                            // ld c, 1
                            addWord(0x0E, 0x01);
                            // call 0x7e22    ; xbasic OCT$/HEX$/BIN$ (in: hl=integer, de=BUF, c=mode [1=bin, 3=oct, 4=hex]; out: hl destination corrected)
                            addCmd(0xCD, def_XBASIC_OCT_HEX_BIN);
                            // ld de, temporary string
                            addTempStr(false);
                            // push de
                            addByte(0xD5);
                            //   call 0x7e9d   ; xbasic copy string (in: hl=source, de=dest; out: hl end of string)
                            addCmd(0xCD, def_XBASIC_COPY_STRING);
                            // pop hl
                            addByte(0xE1);

                            result[0] = Lexeme::subtype_string;

                        } else
                            result[0] = Lexeme::subtype_unknown;

                    } else if(lexeme->value == "OCT$") {

                        if( result[0] == Lexeme::subtype_single_decimal || result[0] == Lexeme::subtype_double_decimal ) {
                            // cast
                            addCast( result[0], Lexeme::subtype_numeric );
                            result[0] = Lexeme::subtype_numeric;
                        }

                        if( result[0] == Lexeme::subtype_numeric ) {

                            // ld c, 3
                            addWord(0x0E, 0x03);
                            // call 0x7e22    ; xbasic OCT$/HEX$/BIN$ (in: hl=integer, de=BUF, c=mode [1=bin, 3=oct, 4=hex]; out: hl destination corrected)
                            addCmd(0xCD, def_XBASIC_OCT_HEX_BIN);
                            // ld de, temporary string
                            addTempStr(false);
                            // push de
                            addByte(0xD5);
                            //   call 0x7e9d   ; xbasic copy string (in: hl=source, de=dest; out: hl end of string)
                            addCmd(0xCD, def_XBASIC_COPY_STRING);
                            // pop hl
                            addByte(0xE1);

                            result[0] = Lexeme::subtype_string;

                        } else
                            result[0] = Lexeme::subtype_unknown;

                    } else if(lexeme->value == "HEX$") {

                        if( result[0] == Lexeme::subtype_single_decimal || result[0] == Lexeme::subtype_double_decimal ) {
                            // cast
                            addCast( result[0], Lexeme::subtype_numeric );
                            result[0] = Lexeme::subtype_numeric;
                        }

                        if( result[0] == Lexeme::subtype_numeric ) {

                            // ld c, 4
                            addWord(0x0E, 0x04);
                            // call 0x7e22    ; xbasic OCT$/HEX$/BIN$ (in: hl=integer, de=BUF, c=mode [1=bin, 3=oct, 4=hex]; out: hl destination corrected)
                            addCmd(0xCD, def_XBASIC_OCT_HEX_BIN);
                            // ld de, temporary string
                            addTempStr(false);
                            // push de
                            addByte(0xD5);
                            //   call 0x7e9d   ; xbasic copy string (in: hl=source, de=dest; out: hl end of string)
                            addCmd(0xCD, def_XBASIC_COPY_STRING);
                            // pop hl
                            addByte(0xE1);

                            result[0] = Lexeme::subtype_string;

                        } else
                            result[0] = Lexeme::subtype_unknown;

                    } else if(lexeme->value == "RESOURCE") {

                        // cast
                        addCast(result[0], Lexeme::subtype_numeric);
                        result[0] = Lexeme::subtype_numeric;

                        // call usr0
                        addCmd(0xCD, def_usr0);

                    } else if(lexeme->value == "RESOURCESIZE") {

                        // cast
                        addCast(result[0], Lexeme::subtype_numeric);
                        result[0] = Lexeme::subtype_numeric;

                        // call usr1
                        addCmd(0xCD, def_usr1);

                    } else if(lexeme->value == "COLLISION") {

                        // cast
                        addCast(result[0], Lexeme::subtype_numeric);
                        result[0] = Lexeme::subtype_numeric;

                        // call SUB_SPRCOL_ONE
                        addCmd(0xCD, def_usr3_COLLISION_ONE);

                    } else if(lexeme->value == "USR" || lexeme->value == "USR0") {

                        // cast
                        addCast(result[0], Lexeme::subtype_numeric);
                        result[0] = Lexeme::subtype_numeric;

                        if(has_defusr) {
                            // xor a
                            addByte(0xAF);
                            // call XBASIC_USR
                            addCmd(0xCD, def_XBASIC_USR);
                        } else {
                            // call usr0
                            addCmd(0xCD, def_usr0);
                        }

                    } else if(lexeme->value == "USR1") {

                        // cast
                        addCast(result[0], Lexeme::subtype_numeric);
                        result[0] = Lexeme::subtype_numeric;

                        if(has_defusr) {
                            // ld a, 1
                            addWord(0x3E, 0x01);
                            // call XBASIC_USR
                            addCmd(0xCD, def_XBASIC_USR);
                        } else {
                            // call usr1
                            addCmd(0xCD, def_usr1);
                        }

                    } else if(lexeme->value == "USR2") {

                        // cast
                        addCast(result[0], Lexeme::subtype_numeric);
                        result[0] = Lexeme::subtype_numeric;

                        if(has_defusr) {
                            // ld a, 2
                            addWord(0x3E, 0x02);
                            // call XBASIC_USR
                            addCmd(0xCD, def_XBASIC_USR);
                        } else {
                            // call usr2
                            addCmd(0xCD, def_usr2);
                        }

                    } else if(lexeme->value == "USR3") {

                        // cast
                        addCast(result[0], Lexeme::subtype_numeric);
                        result[0] = Lexeme::subtype_numeric;

                        if(has_defusr) {
                            // ld a, 3
                            addWord(0x3E, 0x03);
                            // call XBASIC_USR
                            addCmd(0xCD, def_XBASIC_USR);
                        } else {
                            // call usr3
                            addCmd(0xCD, def_usr3);
                        }

                    } else if(lexeme->value == "USR4") {

                        // cast
                        addCast(result[0], Lexeme::subtype_numeric);
                        result[0] = Lexeme::subtype_numeric;

                        if(has_defusr) {
                            // ld a, 4
                            addWord(0x3E, 0x04);
                            // call XBASIC_USR
                            addCmd(0xCD, def_XBASIC_USR);
                        }

                    } else if(lexeme->value == "USR5") {

                        // cast
                        addCast(result[0], Lexeme::subtype_numeric);
                        result[0] = Lexeme::subtype_numeric;

                        if(has_defusr) {
                            // ld a, 5
                            addWord(0x3E, 0x05);
                            // call XBASIC_USR
                            addCmd(0xCD, def_XBASIC_USR);
                        }

                    } else if(lexeme->value == "USR6") {

                        // cast
                        addCast(result[0], Lexeme::subtype_numeric);
                        result[0] = Lexeme::subtype_numeric;

                        if(has_defusr) {
                            // ld a, 6
                            addWord(0x3E, 0x06);
                            // call XBASIC_USR
                            addCmd(0xCD, def_XBASIC_USR);
                        }

                    } else if(lexeme->value == "USR7") {

                        // cast
                        addCast(result[0], Lexeme::subtype_numeric);
                        result[0] = Lexeme::subtype_numeric;

                        if(has_defusr) {
                            // ld a, 7
                            addWord(0x3E, 0x07);
                            // call XBASIC_USR
                            addCmd(0xCD, def_XBASIC_USR);
                        }

                    } else if(lexeme->value == "USR8") {

                        // cast
                        addCast(result[0], Lexeme::subtype_numeric);
                        result[0] = Lexeme::subtype_numeric;

                        if(has_defusr) {
                            // ld a, 8
                            addWord(0x3E, 0x08);
                            // call XBASIC_USR
                            addCmd(0xCD, def_XBASIC_USR);
                        }

                    } else if(lexeme->value == "USR9") {

                        // cast
                        addCast(result[0], Lexeme::subtype_numeric);
                        result[0] = Lexeme::subtype_numeric;

                        if(has_defusr) {
                            // ld a, 9
                            addWord(0x3E, 0x09);
                            // call XBASIC_USR
                            addCmd(0xCD, def_XBASIC_USR);
                        }

                    } else
                        result[0] = Lexeme::subtype_unknown;

                }
                break;

            case 2 : {

                    if(lexeme->value == "POINT") {

                        if( result[0] == Lexeme::subtype_single_decimal || result[0] == Lexeme::subtype_double_decimal ) {
                            // cast
                            addCast( result[0], Lexeme::subtype_numeric );
                            result[0] = Lexeme::subtype_numeric;
                        }

                        if( result[1] == Lexeme::subtype_single_decimal || result[1] == Lexeme::subtype_double_decimal ) {

                            // ex de,hl
                            addByteOptimized(0xEB);
                            // pop bc
                            addByte(0xC1);
                            // pop hl
                            addByte(0xE1);
                            // push de
                            addByte(0xD5);

                            // cast
                            addCast( result[1], Lexeme::subtype_numeric );
                            result[1] = Lexeme::subtype_numeric;

                            // pop de
                            addByte(0xD1);
                            // ex de,hl
                            addByte(0xEB);

                        } else {
                            // pop de
                            addByte(0xD1);
                        }

                        if( result[1] == Lexeme::subtype_numeric && result[0] == Lexeme::subtype_numeric ) {

                            // call 0x6fa7     ; xbasic POINT (in: de=x, hl=y; out: hl=color)
                            addCmd(0xCD, def_XBASIC_POINT);

                            result[0] = Lexeme::subtype_numeric;

                        } else
                            result[0] = Lexeme::subtype_unknown;

                    } else if(lexeme->value == "TILE") {

                        if( result[0] == Lexeme::subtype_single_decimal || result[0] == Lexeme::subtype_double_decimal ) {
                            // cast
                            addCast( result[0], Lexeme::subtype_numeric );
                            result[0] = Lexeme::subtype_numeric;
                        }

                        if( result[1] == Lexeme::subtype_single_decimal || result[1] == Lexeme::subtype_double_decimal ) {

                            // ex de,hl
                            addByteOptimized(0xEB);
                            // pop bc
                            addByte(0xC1);
                            // pop hl
                            addByte(0xE1);
                            // push de
                            addByte(0xD5);

                            // cast
                            addCast( result[1], Lexeme::subtype_numeric );
                            result[1] = Lexeme::subtype_numeric;

                            // pop de
                            addByte(0xD1);
                            // ex de,hl
                            addByte(0xEB);

                        } else {
                            // pop de
                            addByte(0xD1);
                        }

                        if( result[1] == Lexeme::subtype_numeric && result[0] == Lexeme::subtype_numeric ) {

                            // ld h, e
                            addByte(0x63);
                            // inc l
                            addByte(0x2C);
                            // inc h     ; bios based coord system (home=1,1)
                            addByte(0x24);

                            // call def_tileAddress (in: hl=xy; out: hl=address)
                            addCmd(0xCD, def_tileAddress);

                            // call 0x70a1    ; xbasic VPEEK (in:hl, out:hl)
                            addCmd(0xCD, def_XBASIC_VPEEK);

                            result[0] = Lexeme::subtype_numeric;

                        } else
                            result[0] = Lexeme::subtype_unknown;

                    } else if(lexeme->value == "COLLISION") {

                        if( result[0] == Lexeme::subtype_single_decimal || result[0] == Lexeme::subtype_double_decimal ) {
                            // cast
                            addCast( result[0], Lexeme::subtype_numeric );
                            result[0] = Lexeme::subtype_numeric;
                        }

                        if( result[1] == Lexeme::subtype_single_decimal || result[1] == Lexeme::subtype_double_decimal ) {

                            // ex de,hl
                            addByteOptimized(0xEB);
                            // pop bc
                            addByte(0xC1);
                            // pop hl
                            addByte(0xE1);
                            // push de
                            addByte(0xD5);

                            // cast
                            addCast( result[1], Lexeme::subtype_numeric );
                            result[1] = Lexeme::subtype_numeric;

                            // pop de
                            addByte(0xD1);
                            // ex de,hl
                            addByte(0xEB);

                        } else {
                            // pop de
                            addByte(0xD1);
                        }

                        if( result[1] == Lexeme::subtype_numeric && result[0] == Lexeme::subtype_numeric ) {

                            // call SUB_SPRCOL_COUPLE
                            addCmd(0xCD, def_usr3_COLLISION_COUPLE);

                            result[0] = Lexeme::subtype_numeric;

                        } else
                            result[0] = Lexeme::subtype_unknown;

                    } else if(lexeme->value == "STRING$") {

                        if( result[0] == Lexeme::subtype_single_decimal || result[0] == Lexeme::subtype_double_decimal ) {
                            // cast
                            addCast( result[0], Lexeme::subtype_numeric );
                            result[0] = Lexeme::subtype_numeric;
                        } else if(result[0] == Lexeme::subtype_string) {
                            // inc hl
                            addByte(0x23);
                            // ld l, (hl)
                            addByte(0x6E);
                            // ld h, 0
                            addWord(0x26, 0x00);
                            result[0] = Lexeme::subtype_numeric;
                        }

                        if( result[1] == Lexeme::subtype_numeric ) {
                            // pop de
                            addByte(0xD1);
                        } else if(result[1] == Lexeme::subtype_single_decimal || result[1] == Lexeme::subtype_double_decimal) {
                            // pop bc
                            addByte(0xF1);
                            // pop de
                            addByte(0xD1);
                            // push hl
                            addByte(0xE5);
                            //   ex de,hl
                            addByte(0xEB);
                            // cast
                            addCast( result[1], Lexeme::subtype_numeric );
                            //   ex de,hl
                            addByte(0xEB);
                            // pop hl
                            addByte(0xE1);
                            result[1] = Lexeme::subtype_numeric;
                        } else
                            result[1] = Lexeme::subtype_unknown;

                        if( result[0] == Lexeme::subtype_numeric && result[1] == Lexeme::subtype_numeric ) {

                            // ld a, l
                            addByte(0x7D);
                            // ld b, e
                            addByte(0x43);
                            // ld hl, temporary string
                            addTempStr(true);
                            // call 0x7e4c    ; STRING$ (hl=destination, b=number of chars, a=char)
                            addCmd(0xCD, def_XBASIC_STRING);

                            result[0] = Lexeme::subtype_string;

                        } else
                            result[0] = Lexeme::subtype_unknown;

                    } else if(lexeme->value == "LEFT$") {

                        if( result[0] == Lexeme::subtype_single_decimal || result[0] == Lexeme::subtype_double_decimal ) {
                            // cast
                            addCast( result[0], Lexeme::subtype_numeric );
                            result[0] = Lexeme::subtype_numeric;
                        }

                        if( result[1] == Lexeme::subtype_string && result[0] == Lexeme::subtype_numeric ) {

                            // ld a, l
                            addByte(0x7D);
                            // pop hl
                            addByte(0xE1);

                            // call 0x7d99     ; xbasic left string (in: a=size, hl=source; out: hl=BUF)
                            addCmd(0xCD, def_XBASIC_LEFT);
                            // ld de, temporary string
                            addTempStr(false);
                            // push de
                            addByte(0xD5);
                            //   call 0x7e9d   ; xbasic copy string (in: hl=source, de=dest; out: hl end of string)
                            addCmd(0xCD, def_XBASIC_COPY_STRING);
                            // pop hl
                            addByte(0xE1);

                            result[0] = Lexeme::subtype_string;

                        } else
                            result[0] = Lexeme::subtype_unknown;

                    } else if(lexeme->value == "RIGHT$") {

                        if( result[0] == Lexeme::subtype_single_decimal || result[0] == Lexeme::subtype_double_decimal ) {
                            // cast
                            addCast( result[0], Lexeme::subtype_numeric );
                            result[0] = Lexeme::subtype_numeric;
                        }

                        if( result[1] == Lexeme::subtype_string && result[0] == Lexeme::subtype_numeric ) {

                            // ld a, l
                            addByte(0x7D);
                            // pop hl
                            addByte(0xE1);

                            // call 0x7da0     ; xbasic right string (in: a=size, hl=source; out: hl=BUF)
                            addCmd(0xCD, def_XBASIC_RIGHT);
                            // ld de, temporary string
                            addTempStr(false);
                            // push de
                            addByte(0xD5);
                            //   call 0x7e9d   ; xbasic copy string (in: hl=source, de=dest; out: hl end of string)
                            addCmd(0xCD, def_XBASIC_COPY_STRING);
                            // pop hl
                            addByte(0xE1);

                            result[0] = Lexeme::subtype_string;

                        } else
                            result[0] = Lexeme::subtype_unknown;

                    } else if(lexeme->value == "MID$") {

                        if( result[0] == Lexeme::subtype_single_decimal || result[0] == Lexeme::subtype_double_decimal ) {
                            // cast
                            addCast( result[0], Lexeme::subtype_numeric );
                            result[0] = Lexeme::subtype_numeric;
                        }

                        if( result[1] == Lexeme::subtype_string && result[0] == Lexeme::subtype_numeric ) {

                            // ld b, l         ; start char
                            addByte(0x45);
                            // pop hl          ; source string
                            addByte(0xE1);

                            // ld a, 0xff      ; number of chars (all left on source string)
                            addWord(0x3E, 0xff);

                            // call 0x7db1     ; xbasic mid string (in: b=start, a=size, hl=source; out: hl=BUF)
                            addCmd(0xCD, def_XBASIC_MID);

                            // ld de, temporary string
                            addTempStr(false);
                            // push de
                            addByte(0xD5);
                            //   call 0x7e9d   ; xbasic copy string (in: hl=source, de=dest; out: hl end of string)
                            addCmd(0xCD, def_XBASIC_COPY_STRING);
                            // pop hl
                            addByte(0xE1);

                            result[0] = Lexeme::subtype_string;

                        } else
                            result[0] = Lexeme::subtype_unknown;

                    } else if(lexeme->value == "USING$") {

                        if( result[0] == Lexeme::subtype_double_decimal || result[0] == Lexeme::subtype_numeric ) {
                            // cast
                            addCast( result[0], Lexeme::subtype_single_decimal );
                            result[0] = Lexeme::subtype_single_decimal;
                        }

                        if( result[1] == Lexeme::subtype_string && result[0] == Lexeme::subtype_single_decimal ) {

                            // ld c, b
                            addByte(0x48);
                            // ex de, hl
                            addByte(0xEB);
                            // pop hl
                            addByte(0xE1);

                            // call XBASIC_USING    ; hl = item format string, c:de = float, out hl=string
                            addCmd(0xCD, def_XBASIC_USING);
                            // ld de, temporary string
                            addTempStr(false);
                            // push de
                            addByte(0xD5);
                            //   call 0x7e9d   ; xbasic copy string (in: hl=source, de=dest; out: hl end of string)
                            addCmd(0xCD, def_XBASIC_COPY_STRING);
                            // pop hl
                            addByte(0xE1);

                            result[0] = Lexeme::subtype_string;

                        } else if( result[1] == Lexeme::subtype_numeric && result[0] == Lexeme::subtype_single_decimal ) {

                            // pop de
                            addByte(0xD1);
                            // push de
                            addByte(0xD5);
                            //   ld a, e
                            addByte(0x7B);
                            //   rrca
                            addByte(0x0F);
                            //   rrca
                            addByte(0x0F);
                            //   rrca
                            addByte(0x0F);
                            //   rrca
                            addByte(0x0F);
                            //   and 0x0F
                            addWord(0xE6, 0x0F);
                            //   ld d, a
                            addByte(0x57);
                            //   ld a, e
                            addByte(0x7B);
                            //   and 0x0F
                            addWord(0xE6, 0x0F);
                            //   ld e, a
                            addByte(0x5F);
                            // pop af
                            addByte(0xF1);

                            // call XBASIC_USING_DO    ; a=format, d=thousand digits, e=decimal digits, b:hl=number, out hl=string
                            addCmd(0xCD, def_XBASIC_USING_DO);
                            // ld de, temporary string
                            addTempStr(false);
                            // push de
                            addByte(0xD5);
                            //   call 0x7e9d   ; xbasic copy string (in: hl=source, de=dest; out: hl end of string)
                            addCmd(0xCD, def_XBASIC_COPY_STRING);
                            // pop hl
                            addByte(0xE1);

                            result[0] = Lexeme::subtype_string;

                        } else
                            result[0] = Lexeme::subtype_unknown;

                    } else if(lexeme->value == "INSTR") {

                        if( result[0] == Lexeme::subtype_string && result[1] == Lexeme::subtype_string ) {

                            // ex de,hl        ; search string
                            addByte(0xEB);
                            // pop hl          ; source string
                            addByte(0xE1);

                            // ld a, 0x01      ; search start
                            addWord(0x3E, 0x01);

                            // call 0x7e6c     ; xbasic INSTR (in: a=start, hl=source, de=search; out: hl=position)
                            addCmd(0xCD, def_XBASIC_INSTR);

                            result[0] = Lexeme::subtype_numeric;

                        } else
                            result[0] = Lexeme::subtype_unknown;

                    } else
                        result[0] = Lexeme::subtype_unknown;

                }
                break;

            case 3 : {

                    if(lexeme->value == "MID$") {

                        if( result[0] == Lexeme::subtype_single_decimal || result[0] == Lexeme::subtype_double_decimal ) {
                            // cast
                            addCast( result[0], Lexeme::subtype_numeric );
                            result[0] = Lexeme::subtype_numeric;
                        }

                        // ld a, l             ; number of chars
                        addByte(0x7D);

                        if( result[1] == Lexeme::subtype_single_decimal || result[1] == Lexeme::subtype_double_decimal ) {
                            // pop bc
                            addByte(0xC1);
                            // pop hl
                            addByte(0xE1);
                            // push af
                            addByte(0xF5);
                            // cast
                            addCast( result[1], Lexeme::subtype_numeric );
                            // pop af
                            addByte(0xF1);
                            result[1] = Lexeme::subtype_numeric;
                        } else {
                            // pop hl
                            addByte(0xE1);
                        }

                        if( result[2] == Lexeme::subtype_string && result[1] == Lexeme::subtype_numeric && result[0] == Lexeme::subtype_numeric ) {

                            // ld b, l         ; start char
                            addByte(0x45);
                            // pop hl          ; source string
                            addByte(0xE1);

                            // call 0x7db1     ; xbasic mid string (in: b=start, a=size, hl=source; out: hl=BUF)
                            addCmd(0xCD, def_XBASIC_MID);

                            // ld de, temporary string
                            addTempStr(false);
                            // push de
                            addByte(0xD5);
                            //   call 0x7e9d   ; xbasic copy string (in: hl=source, de=dest; out: hl end of string)
                            addCmd(0xCD, def_XBASIC_COPY_STRING);
                            // pop hl
                            addByte(0xE1);

                            result[0] = Lexeme::subtype_string;

                        } else
                            result[0] = Lexeme::subtype_unknown;

                    } else if(lexeme->value == "INSTR") {

                        // ex de,hl        ; search string
                        addByte(0xEB);
                        // pop hl          ; source string
                        addByte(0xE1);
                        // pop bc          ; search start
                        addByte(0xC1);

                        if( result[2] == Lexeme::subtype_single_decimal || result[2] == Lexeme::subtype_double_decimal ) {
                            // ex (sp),hl
                            addByte(0xE3);
                            // push de
                            addByte(0xD5);
                            //   cast
                            addCast( result[2], Lexeme::subtype_numeric );
                            //   ld c, l
                            addByte(0x4D);
                            // pop de
                            addByte(0xD1);
                            // pop hl
                            addByte(0xE1);
                            result[2] = Lexeme::subtype_numeric;
                        }

                        // ld a, c             ; search start
                        addByte(0x79);

                        if( result[0] == Lexeme::subtype_string && result[1] == Lexeme::subtype_string && result[2] == Lexeme::subtype_numeric ) {

                            // call 0x7e6c     ; xbasic INSTR (in: a=start, hl=source, de=search; out: hl=position)
                            addCmd(0xCD, def_XBASIC_INSTR);

                            result[0] = Lexeme::subtype_numeric;

                        } else
                            result[0] = Lexeme::subtype_unknown;

                    } else
                        result[0] = Lexeme::subtype_unknown;

                }
                break;

        }

    }

    return result[0];
}

bool Compiler::addVarAddress(ActionNode *action) {
    Lexeme *lexeme, *lexeme1, *lexeme2;
    ActionNode *action1, *action2;
    unsigned int i, t;
    int factor, diff;
    int result_subtype;
    bool first;

    lexeme = action->lexeme;
    t = action->actions.size();

    if(lexeme->isArray) {

        switch(t) {
            case 0: {
                    syntax_error("Array index is missing");
                    return false;
                }
                break;

            case 1: {
                    action1 = action->actions[0];
                    lexeme1 = action1->lexeme;

                    if(lexeme1->type == Lexeme::type_literal) {

                        if(lexeme1->subtype == Lexeme::subtype_numeric ||
                                lexeme1->subtype == Lexeme::subtype_single_decimal ||
                                lexeme1->subtype == Lexeme::subtype_double_decimal) {

                            try {
                                i = stoi(lexeme1->value) * lexeme->x_factor;
                            } catch(exception &e) {
                                printf("Warning: error while converting numeric constant %s\n", lexeme1->value.c_str());
                                i = 0;
                            }

                            // ld hl, variable_address +  (x_index * x_factor)
                            addFix( lexeme )->step = i;
                            addCmd(0x21, 0x0000);

                        } else {
                            syntax_error("Invalid array index type");
                        }

                    } else {

                        // ld hl, x index
                        result_subtype = evalExpression(action1);
                        addCast(result_subtype, Lexeme::subtype_numeric);

                        if(lexeme->x_factor == 2) {
                            // add hl, hl
                            addByte(0x29);
                        } else if(lexeme->x_factor == 3) {
                            // ld d, h
                            addByte(0x54);
                            // ld e, l
                            addByte(0x5D);
                            // add hl, hl
                            addByte(0x29);
                            // add hl, de
                            addByte(0x19);
                        } else if(lexeme->x_factor == 256) {
                            // ld h, l
                            addByte(0x65);
                            // ld l, 0
                            addWord(0x2E, 0x00);
                        } else {
                            // ld de, x_factor
                            addCmd(0x11, lexeme->x_factor);

                            // call 0x761b    ; integer multiplication (hl = hl * de)
                            addCmd(0xCD, def_XBASIC_MULTIPLY_INTEGERS);
                        }

                        // ld de, variable
                        addFix( lexeme );
                        addCmd(0x11, 0x0000);

                        // add hl, de   ; hl = variable_address +  (x_index * x_factor)
                        addByte(0x19);

                    }

                }
                break;

            case 2: {
                    action1 = action->actions[1];
                    lexeme1 = action1->lexeme;

                    action2 = action->actions[0];
                    lexeme2 = action2->lexeme;

                    if(lexeme1->type == Lexeme::type_literal && lexeme2->type == Lexeme::type_literal) {

                        if(lexeme1->subtype == Lexeme::subtype_numeric ||
                                lexeme1->subtype == Lexeme::subtype_single_decimal ||
                                lexeme1->subtype == Lexeme::subtype_double_decimal) {

                            try {
                                i = stoi(lexeme1->value) * lexeme->x_factor;
                            } catch(exception &e) {
                                printf("Warning: error while converting numeric constant %s\n", lexeme1->value.c_str());
                                i = 0;
                            }

                            if(lexeme2->subtype == Lexeme::subtype_numeric ||
                                    lexeme2->subtype == Lexeme::subtype_single_decimal ||
                                    lexeme2->subtype == Lexeme::subtype_double_decimal) {

                                try {
                                    i += stoi(lexeme2->value) * lexeme->y_factor;
                                } catch(exception &e) {
                                    printf("Warning: error while converting numeric constant %s\n", lexeme2->value.c_str());
                                }

                                // ld hl, variable_address +  (x_index * x_factor) + (y_index * y_factor)
                                addFix( lexeme )->step = i;
                                addCmd(0x21, 0x0000);

                            } else {
                                syntax_error("Invalid array 2nd index type");
                            }

                        } else {
                            syntax_error("Invalid array 1st index type");
                        }

                    } else {

                        // ld hl, x index
                        result_subtype = evalExpression(action1);
                        addCast(result_subtype, Lexeme::subtype_numeric);

                        if(lexeme->x_factor == 2) {
                            // add hl, hl
                            addByte(0x29);
                        } else if(lexeme->x_factor == 3) {
                            // ld d, h
                            addByte(0x54);
                            // ld e, l
                            addByte(0x5D);
                            // add hl, hl
                            addByte(0x29);
                            // add hl, de
                            addByte(0x19);
                        } else if(lexeme->x_factor == 256) {
                            // ld h, l
                            addByte(0x65);
                            // ld l, 0
                            addWord(0x2E, 0x00);
                        } else {
                            // ld de, x_factor
                            addCmd(0x11, lexeme->x_factor);

                            // call 0x761b    ; integer multiplication (hl = hl * de)
                            addCmd(0xCD, def_XBASIC_MULTIPLY_INTEGERS);
                        }

                        // push hl
                        addByte(0xE5);

                        // ld hl, y index
                        result_subtype = evalExpression(action2);
                        addCast(result_subtype, Lexeme::subtype_numeric);

                        // calculate y factor * index
                        // ---------- old code
                        // ld de, y_factor
                        //addCmd(0x11, lexeme->y_factor);
                        // call 0x761b    ; integer multiplication (hl = hl * de)
                        //addCmd(0xCD, def_XBASIC_MULTIPLY_INTEGERS);
                        //--------------------

                        factor = lexeme->y_factor;
                        if(lexeme->x_factor == 2) {
                            factor >>= 1;
                        }

                        first = true;

                        // verify if factor is power of 2
                        diff = 1;
                        while(factor >= (diff<<1) ) {
                            diff <<= 1;   // diff *= 2;
                        }
                        if(diff != factor) {
                            // ld b, h           ; save index in bc
                            addByte(0x44);
                            // ld c, l
                            addByte(0x4D);
                        }

                        while(factor) {

                            if(!first) {
                                // ex de, hl     ; save current total
                                addByte(0xEB);
                                // ld h, b       ; restore index from bc
                                addByte(0x60);
                                // ld l, c
                                addByte(0x69);
                            }

                            diff = 1;

                            while(factor >= (diff<<1) ) {

                                // add hl, hl    ; x 2
                                addByte(0x29);

                                diff <<= 1;   // diff *= 2;
                            }

                            if(first) {
                                first = false;
                            } else {
                                // add hl, de    ; add last total
                                addByte(0x19);
                            }

                            factor -= diff;

                        }

                        if(lexeme->x_factor == 2) {
                            // add hl, hl    ; x 2
                            addByte(0x29);
                        }

                        // pop de
                        addByte(0xD1);

                        // add hl, de     ; hl = (x_index * x_factor) + (y_index * y_factor)
                        addByte(0x19);

                        // ld de, variable
                        addFix( lexeme );
                        addCmd(0x11, 0x0000);

                        // add hl, de    ; hl += variable_adress
                        addByte(0x19);

                    }

                }
                break;

            default: {
                    syntax_error("Wrong array parameters count");
                    return false;
                }

        }

    } else {

        if(t) {
            syntax_error("Undeclared array or unknown function");
            return false;
        } else {
            // ld hl, variable
            addFix( lexeme );
            addCmd(0x21, 0x0000);
        }

    }

    return true;
}

void Compiler::addTempStr(bool atHL) {

    if(atHL) {
        // call GET_NEXT_TEMP_STRING_ADDRESS
        addCmd(0xCD, def_GET_NEXT_TEMP_STRING_ADDRESS);
    } else {
        // ex de, hl
        addByte(0xEB);
        // call GET_NEXT_TEMP_STRING_ADDRESS
        addCmd(0xCD, def_GET_NEXT_TEMP_STRING_ADDRESS);
        // ex de, hl
        addByteOptimized(0xEB);
    }
}

void Compiler::addCast(int from, int to) {

    if( from != to ) {

        if(from == Lexeme::subtype_numeric) {

            if(to == Lexeme::subtype_numeric) {
                return;
            } else if(to == Lexeme::subtype_string) {
                // call 0x7b26   ; xbasic int to string (in hl, out hl)
                addCmd(0xCD, def_XBASIC_CAST_INTEGER_TO_STRING);
            } else if(to == Lexeme::subtype_single_decimal || to == Lexeme::subtype_double_decimal) {
                // call 0x782d   ; xbasic int to float (in hl, out b:hl)
                addCmd(0xCD, def_XBASIC_CAST_INTEGER_TO_FLOAT);
            }

        } else if(from == Lexeme::subtype_string) {

            if(to == Lexeme::subtype_numeric) {
                // call 0x7e07   ; VAL function - xbasic string to float (in hl, out b:hl)
                addCmd(0xCD, def_XBASIC_CAST_STRING_TO_FLOAT);
                // call 0x784f   ; xbasic float to integer (in b:hl, out hl)
                addCmd(0xCD, def_XBASIC_CAST_FLOAT_TO_INTEGER);
            } else if(to == Lexeme::subtype_string) {
                return;
            } else if(to == Lexeme::subtype_single_decimal || to == Lexeme::subtype_double_decimal) {
                // call 0x7e07   ; VAL function - xbasic string to float (in hl, out b:hl)
                addCmd(0xCD, def_XBASIC_CAST_STRING_TO_FLOAT);
            }

        } else if(from == Lexeme::subtype_single_decimal || from == Lexeme::subtype_double_decimal) {

            if(to == Lexeme::subtype_numeric) {
                // call 0x784f   ; xbasic float to integer (in b:hl, out hl)
                addCmd(0xCD, def_XBASIC_CAST_FLOAT_TO_INTEGER);
            } else if(to == Lexeme::subtype_string) {
                // call 0x7b80   ; xbasic float to string (in b:hl, out hl)
                addCmd(0xCD, def_XBASIC_CAST_FLOAT_TO_STRING);
            } else if(to == Lexeme::subtype_single_decimal || to == Lexeme::subtype_double_decimal) {
                return;
            }

        } else {
            syntax_error("Unknown type to cast");
        }
    }

}


//-------------------------------------------------------------------------------------------

void Compiler::cmd_start() {

    // ld (SAVSTK), sp
    addByte(0xED);
    addCmd(0x73, 0xf6b1);

    // ld a, (SLTSTR)     ; start slot
    addCmd(0x3A, def_SLTSTR);

    // ld h, 0x40
    addWord(0x26, 0x40);

    // call ENASLT        ; enable xbasic page
    addCmd(0xCD, def_ENASLT);

    if(parser->has_data) {
        if(megaROM) {
            data_mark = addMark();
            // special ld hl, 0x0000   ; DATA start pointer at his segment
            addCmd(0xFF, 0x0000);
            // ld c, l
            addByte(0x4D);
            // ld b, h
            addByte(0x44);
        } else {
            // ld bc, 0x0000           ; DATA start pointer
            addCmd(0x01, 0x0000);
        }
    }

    // ld hl, HEAP START ADDRESS
    addFix(heap_mark);
    addCmd(0x21, 0x0000);

    // ld de, TEMPORARY STRING START ADDRESS
    addFix(temp_str_mark);
    addCmd(0x11, 0x0000);


    if(parser->has_font) {
        // ld ix, FONT BUFFER START ADDRESS
        addByte(0xDD);
        addFix(heap_mark)->step = -def_RAM_BUFSIZ;
        addCmd(0x21, 0x0000);
    } else {
        // push hl
        addByte(0xE5);
        // pop ix
        addWord(0xDD, 0xE1);
    }

    // call XBASIC INIT                  ; hl=heap start address, de=temporary string start address, bc=data address, ix=font address, a=data segment
    addCmd(0xCD, def_XBASIC_INIT);

    if(parser->has_data && !megaROM) {
        // ld hl, data resource number
        addCmd(0x21, parser->resourceCount);
        // ld (DAC), hl
        addCmd(0x22, def_DAC);
        // call cmd_restore
        addCmd(0xCD, def_cmd_restore);
    }

    if(parser->has_traps) {
        if(megaROM) {
            // ld a, 0xFF
            addWord(0x3E, 0xFF);
        } else {
            // xor a
            addByte(0xAF);
        }
        // ld (0xFC82), a          ; start of TRPTBL reserved area (megaROM flag to traps)
        addCmd(0x32, def_MR_TRAP_FLAG);
    }

    if(megaROM) {

        // ld a, 2
        addWord(0x3E, 0x02);
        // call MR_CHANGE_SGM
        addCmd(0xCD, def_MR_CHANGE_SGM);

    }

    if(parser->has_akm) {
        // initialize AKM player
        addCmd(0xCD, def_player_initialize);
    }

    // ei
    addByte(0xFB);

}

void Compiler::cmd_end(bool last) {

    if(last) {

        if(end_mark)
            end_mark->symbol->address = code_pointer;

        if(parser->has_akm) {
            // disable AKM player
            addCmd(0xCD, def_player_unhook);
        }

        // call XBASIC_END
        addCmd(0xCD, def_XBASIC_END);

        // ld sp, (SAVSTK)
        addByte(0xED);
        addCmd(0x7b, 0xf6b1);

        // ld hl, fake empty line
        addCmd(0x21, def_ENDPRG);

        if(megaROM) {

            // ld a, 2
            addWord(0x3E, 0x02);
            // ld iy, (SLTSTR-1)
            addByte(0xFD);
            addCmd(0x2A, def_SLTSTR-1);
            // ld ix, MR_CHANGE_SGM
            addByte(0xDD);
            addCmd(0x21, def_MR_CHANGE_SGM);
            // call CALSLT
            addCmd(0xCD, def_CALSLT);
            // ei
            addByte(0xFB);
            // ret               ; return to basic
            addByte(0xC9);

        } else {
            // ret               ; return to basic
            addByte(0xC9);
        }

    } else {

        // jp end_mark
        if(end_mark)
            addFix(end_mark->symbol);
        else
            end_mark = addMark();
        addCmd(0xC3, 0x0000);

    }

}

void Compiler::cmd_cls() {

    addCmd( 0xcd, def_XBASIC_CLS );   // call cls

}

void Compiler::cmd_beep() {

    addCmd( 0xcd, 0x00c0 );   // call beep

}

void Compiler::cmd_randomize() {

    // ld hl, 0x3579      ; RANDOMIZE 1 - FIX
    addCmd(0x21, 0x3579);
    // ld (0xF7BC+0), hl  ; SWPTMP+0
    addCmd(0x22, 0xF7BC);

    // ld hl, (0xFC9E)    ; RANDOMIZE 2 - JIFFY
    addCmd(0x2A, 0xFC9E);
    // ld (0xF7BC+2), hl  ; SWPTMP+2
    addCmd(0x22, 0xF7BE);

}

void Compiler::cmd_goto() {
    Lexeme *lexeme;

    if(current_action->actions.size() == 1) {
        lexeme = current_action->actions[0]->lexeme;
        if(lexeme) {
            if(lexeme->type == Lexeme::type_literal && lexeme->subtype == Lexeme::subtype_numeric) {

                // Trim leading zeros
                while (lexeme->value.find("0") == 0 && lexeme->value.size() > 1) {
                    lexeme->value.erase(0, 1);
                }

                // jp address
                addFix( lexeme->value );
                addCmd(0xC3, 0x0000 );
                return;
            }
        }
    }

    syntax_error("Invalid GOTO parameters");
}

void Compiler::cmd_gosub() {
    Lexeme *lexeme;

    if(current_action->actions.size() == 1) {
        lexeme = current_action->actions[0]->lexeme;
        if(lexeme) {
            if(lexeme->type == Lexeme::type_literal && lexeme->subtype == Lexeme::subtype_numeric) {

                // Trim leading zeros
                while (lexeme->value.find("0") == 0 && lexeme->value.size() > 1) {
                    lexeme->value.erase(0, 1);
                }

                // call address
                addFix( lexeme->value );
                addCmd(0xcd, 0x0000 );
                return;
            }
        }
    }

    syntax_error("Invalid GOSUB parameters");
}

void Compiler::cmd_return() {
    Lexeme *lexeme;
    int t = current_action->actions.size();

    if(t == 0) {

        // ret
        addByte(0xC9);
        return;

    } else if(t == 1) {

        lexeme = current_action->actions[0]->lexeme;
        if(lexeme) {
            if(lexeme->type == Lexeme::type_literal && lexeme->subtype == Lexeme::subtype_numeric) {

                // Trim leading zeros
                while (lexeme->value.find("0") == 0 && lexeme->value.size() > 1) {
                    lexeme->value.erase(0, 1);
                }

                if(megaROM) {

                    // pop bc           ; delete old return segment/address
                    addByte(0xC1);
                    // pop de           ; delete old return segment/address
                    addByte(0xD1);
                    // ld a, 0x48       ; verify if running on trap (MR_CALL_TRAP)
                    addWord(0x3E, 0x48);
                    // cp b
                    addByte(0xB8);
                    // jp nz, address   ; if not, jump to new address and segment
                    addFix( lexeme->value );
                    addCmd(0xC2, 0x0000);

                    // pop de           ; fix trap return control
                    addByte(0xD1);
                    // pop hl
                    addByte(0xE1);
                    // exx
                    addByte(0xD9);
                    // special ld hl, address
                    addFix( lexeme->value );
                    addCmd(0xFF, 0x0000);
                    // ex (sp), hl      ; new return address
                    addByte(0xE3);
                    // exx
                    addByte(0xD9);
                    // push hl
                    addByte(0xE5);
                    // push de
                    addByte(0xD5);
                    // push af          ; new return segment
                    addByte(0xF5);
                    // push bc          ; trap return
                    addByte(0xC5);
                    // ret
                    addByte(0xC9);

                } else {

                    // pop bc           ; delete old return address
                    addByte(0xC1);
                    // ld a, 0x6C       ; verify if running on trap
                    addWord(0x3E, 0x6C);
                    // cp b
                    addByte(0xB8);
                    // jp nz, address   ; jump to new address
                    addFix( lexeme->value );
                    addCmd(0xC2, 0x0000);

                    // pop de           ; fix trap return control
                    addByte(0xD1);
                    // ld hl, address
                    addFix( lexeme->value );
                    addCmd(0x21, 0x0000);
                    // ex (sp), hl
                    addByte(0xE3);
                    // push de
                    addByte(0xD5);
                    // push bc
                    addByte(0xC5);
                    // ret
                    addByte(0xC9);

                }

                return;
            }
        }
    }

    syntax_error("Invalid RETURN parameters");
}

void Compiler::cmd_print() {
    Lexeme *lexeme, *last_lexeme=0;
    ActionNode *action, *subaction;
    unsigned int i, t = current_action->actions.size();
    int result_subtype;
    bool redirected=false;

    if(t) {

        for( i = 0; i < t; i++) {

            action = current_action->actions[i];
            lexeme = action->lexeme;
            last_lexeme = lexeme;

            if(lexeme) {

                if(lexeme->type == Lexeme::type_separator) {
                    if(lexeme->value == ",") {
                        addCmd( 0xcd, def_XBASIC_PRINT_TAB );   // call print_tab
                    } else if(lexeme->value == ";") {
                        continue;
                    } else if(lexeme->value == "#") {
                        if(has_open_grp)
                            continue;

                        redirected = true;
                        subaction = action->actions[0];
                        result_subtype = evalExpression(subaction);
                        addCast( result_subtype, Lexeme::subtype_numeric );

                        // call io redirect
                        if(io_redirect_mark)
                            addFix(io_redirect_mark->symbol);
                        else
                            io_redirect_mark = addMark();
                        addCmd(0xCD, 0x0000);

                        continue;
                    } else {
                        syntax_error("Invalid PRINT parameter separator");
                        return;
                    }
                } else {

                    result_subtype = evalExpression(action);

                    if(result_subtype == Lexeme::subtype_string) {

                        addCmd( 0xcd, def_XBASIC_PRINT_STR );   // call print_str

                    } else if(result_subtype == Lexeme::subtype_numeric) {

                        addCmd( 0xcd, def_XBASIC_PRINT_INT );   // call print_int

                    } else if(result_subtype == Lexeme::subtype_single_decimal || result_subtype == Lexeme::subtype_double_decimal) {

                        addCmd( 0xcd, def_XBASIC_PRINT_FLOAT );   // call print_float

                    } else {
                        syntax_error("Invalid PRINT parameter");
                        return;
                    }
                }

            }

        }

    } else {

        addCmd( 0xcd, def_XBASIC_PRINT_CRLF );   // call print_crlf

    }

    if(last_lexeme) {
        if(last_lexeme->type != Lexeme::type_separator ||
                (last_lexeme->value != ";" && last_lexeme->value != ",") ) {
            addCmd( 0xcd, def_XBASIC_PRINT_CRLF );   // call print_crlf
        }
    }

    if(redirected) {
        // call io screen
        if(io_screen_mark)
            addFix(io_screen_mark->symbol);
        else
            io_screen_mark = addMark();
        addCmd(0xCD, 0x0000);
    }
}

void Compiler::cmd_input(bool question) {
    Lexeme *lexeme;
    ActionNode *action, *subaction;
    unsigned int i, t = current_action->actions.size();
    int result_subtype;
    bool redirected = false;

    if(t) {

        for( i = 0; i < t; i++) {
            action = current_action->actions[i];
            lexeme = action->lexeme;
            if(lexeme) {

                if(lexeme->type == Lexeme::type_separator) {
                    if(lexeme->value == ",") {
                        addCmd( 0xcd, def_XBASIC_PRINT_TAB );   // call print_tab
                    } else if(lexeme->value == ";") {
                        continue;
                    } else if(lexeme->value == "#") {
                        redirected = true;
                        subaction = action->actions[0];
                        result_subtype = evalExpression(subaction);
                        addCast( result_subtype, Lexeme::subtype_numeric );

                        // call io redirect
                        if(io_redirect_mark)
                            addFix(io_redirect_mark->symbol);
                        else
                            io_redirect_mark = addMark();
                        addCmd(0xCD, 0x0000);

                        continue;
                    } else {
                        syntax_error("Invalid INPUT parameter separator");
                        return;
                    }
                } else {

                    if(lexeme->type == Lexeme::type_identifier) {

                        // call INPUT or LINE INPUT

                        if(question) {
                            addCmd(0xCD, def_XBASIC_INPUT_1);
                        } else {
                            addCmd(0xCD, def_XBASIC_INPUT_2);
                        }

                        // do assignment

                        addCast( Lexeme::subtype_string, lexeme->subtype );

                        if(!addAssignment(action))
                            return;

                    } else {
                        result_subtype = evalExpression(action);

                        if(result_subtype == Lexeme::subtype_string) {

                            addCmd( 0xcd, def_XBASIC_PRINT_STR );   // call print_str

                        } else if(result_subtype == Lexeme::subtype_numeric) {

                            addCmd( 0xcd, def_XBASIC_PRINT_INT );   // call print_int

                        } else if(result_subtype == Lexeme::subtype_single_decimal || result_subtype == Lexeme::subtype_double_decimal) {

                            addCmd( 0xcd, def_XBASIC_PRINT_FLOAT );   // call print_float

                        } else {
                            syntax_error("Invalid INPUT parameter");
                            return;
                        }
                    }

                }

            }
        }

        if(redirected) {
            // call io screen
            if(io_screen_mark)
                addFix(io_screen_mark->symbol);
            else
                io_screen_mark = addMark();
            addCmd(0xCD, 0x0000);
        }

    } else {
        syntax_error();
    }

}

void Compiler::cmd_line() {
    Lexeme *lexeme;
    ActionNode *action, *sub_action;
    unsigned int i, t = current_action->actions.size();
    int line_type = 0, result_subtype, state;
    bool has_x0_coord=false, has_x1_coord=false, has_y0_coord=false, has_y1_coord=false;
    bool has_color=false, has_line_type=false, has_operator=false;

    if(t) {

        action = current_action->actions[0];
        lexeme = action->lexeme;
        if(lexeme->type == Lexeme::type_keyword && lexeme->value == "INPUT") {
            current_action = action;
            return cmd_input(false);
        }

        state = 0;

        for(i = 0; i < t; i++) {

            action = current_action->actions[i];

            switch(state) {
                case 0: {
                        if(action->actions.size() != 2) {
                            syntax_error("Coordenates parameters error on LINE");
                            return;
                        } else if(action->lexeme->value == "COORD") {

                            sub_action = action->actions[0];

                            result_subtype = evalExpression(sub_action);

                            if(result_subtype == Lexeme::subtype_null) {
                                // ld hl, (0xFCB7)  ;GRPACX
                                addCmd(0x2A, 0xFCB7);
                                // push hl
                                addByte(0xE5);
                                has_x0_coord = true;
                            } else {
                                addCast(result_subtype, Lexeme::subtype_numeric);

                                // ld (0xFCB7), hl  ;GRPACX
                                addCmd(0x22, 0xFCB7);
                                // push hl
                                addByte(0xE5);
                                has_x0_coord = true;

                            }

                            sub_action = action->actions[1];

                            result_subtype = evalExpression(sub_action);

                            if(result_subtype == Lexeme::subtype_null) {
                                // ld hl, (0xFCB9)  ;GRPACY
                                addCmd(0x2A, 0xFCB9);
                                // push hl
                                addByte(0xE5);
                                has_y0_coord = true;
                            } else {
                                addCast(result_subtype, Lexeme::subtype_numeric);

                                // ld (0xFCB9), hl  ;GRPACY
                                addCmd(0x22, 0xFCB9);
                                // push hl
                                addByte(0xE5);
                                has_y0_coord = true;

                            }

                        } else if(action->lexeme->value == "STEP") {

                            sub_action = action->actions[0];

                            result_subtype = evalExpression(sub_action);

                            if(result_subtype == Lexeme::subtype_null) {
                                // ld hl, (0xFCB7)  ;GRPACX
                                addCmd(0x2A, 0xFCB7);
                                // push hl
                                addByte(0xE5);
                                has_x0_coord = true;
                            } else {
                                addCast(result_subtype, Lexeme::subtype_numeric);
                                // ld de, (0xFCB7)  ;GRPACX
                                addByte(0xED);
                                addCmd(0x5B, 0xFCB7);
                                // add hl, de
                                addByte(0x19);
                                // ld (0xFCB7), hl  ;GRPACX
                                addCmd(0x22, 0xFCB7);
                                // push hl
                                addByte(0xE5);
                                has_x0_coord = true;
                            }

                            sub_action = action->actions[1];

                            result_subtype = evalExpression(sub_action);

                            if(result_subtype == Lexeme::subtype_null) {
                                // ld hl, (0xFCB9)  ;GRPACY
                                addCmd(0x2A, 0xFCB9);
                                // push hl
                                addByte(0xE5);
                                has_y0_coord = true;
                            } else {
                                addCast(result_subtype, Lexeme::subtype_numeric);
                                // ld de, (0xFCB9)  ;GRPACY
                                addByte(0xED);
                                addCmd(0x5B, 0xFCB9);
                                // add hl, de
                                addByte(0x19);
                                // ld (0xFCB9), hl  ;GRPACY
                                addCmd(0x22, 0xFCB9);
                                // push hl
                                addByte(0xE5);
                                has_y0_coord = true;
                            }

                        } else if(action->lexeme->value == "TO_COORD") {

                            state ++;

                            if(!has_x0_coord) {
                                // ex de,hl
                                addByte(0xEB);
                                // ld hl, (0xFCB7)  ;GRPACX
                                addCmd(0x2A, 0xFCB7);
                                // push hl
                                addByte(0xE5);
                                // ld hl, (0xFCB9)  ;GRPACY
                                addCmd(0x2A, 0xFCB9);
                                // push hl
                                addByte(0xE5);
                                // ex de,hl
                                addByte(0xEB);
                                has_x0_coord = true;
                                has_y0_coord = true;
                            }

                            sub_action = action->actions[0];

                            result_subtype = evalExpression(sub_action);

                            if(result_subtype == Lexeme::subtype_null) {
                            } else {
                                addCast(result_subtype, Lexeme::subtype_numeric);

                                // ld (0xFCB7), hl  ;GRPACX
                                addCmd(0x22, 0xFCB7);
                                // push hl
                                addByte(0xE5);
                                has_x1_coord = true;

                            }

                            sub_action = action->actions[1];

                            result_subtype = evalExpression(sub_action);

                            if(result_subtype == Lexeme::subtype_null) {
                            } else {
                                addCast(result_subtype, Lexeme::subtype_numeric);

                                // ld (0xFCB9), hl  ;GRPACY
                                addCmd(0x22, 0xFCB9);
                                // push hl
                                addByte(0xE5);
                                has_y1_coord = true;

                            }

                        } else if(action->lexeme->value == "TO_STEP") {

                            state ++;

                            if(!has_x0_coord) {
                                // ex de,hl
                                addByte(0xEB);
                                // ld hl, (0xFCB7)  ;GRPACX
                                addCmd(0x2A, 0xFCB7);
                                // push hl
                                addByte(0xE5);
                                // ld hl, (0xFCB9)  ;GRPACY
                                addCmd(0x2A, 0xFCB9);
                                // push hl
                                addByte(0xE5);
                                // ex de,hl
                                addByte(0xEB);
                                has_x0_coord = true;
                                has_y0_coord = true;
                            }

                            sub_action = action->actions[0];

                            result_subtype = evalExpression(sub_action);

                            if(result_subtype == Lexeme::subtype_null) {
                            } else {
                                addCast(result_subtype, Lexeme::subtype_numeric);
                                // ld de, (0xFCB7)  ;GRPACX
                                addByte(0xED);
                                addCmd(0x5B, 0xFCB7);
                                // add hl, de
                                addByte(0x19);
                                // ld (0xFCB7), hl  ;GRPACX
                                addCmd(0x22, 0xFCB7);
                                // push hl
                                addByte(0xE5);
                                has_x1_coord = true;
                            }

                            sub_action = action->actions[1];

                            result_subtype = evalExpression(sub_action);

                            if(result_subtype == Lexeme::subtype_null) {
                            } else {
                                addCast(result_subtype, Lexeme::subtype_numeric);
                                // ld de, (0xFCB9)  ;GRPACY
                                addByte(0xED);
                                addCmd(0x5B, 0xFCB9);
                                // add hl, de
                                addByte(0x19);
                                // ld (0xFCB9), hl  ;GRPACY
                                addCmd(0x22, 0xFCB9);
                                // push hl
                                addByte(0xE5);
                                has_y1_coord = true;
                            }

                        } else {
                            syntax_error("Invalid coordenates on LINE");
                            return;
                        }

                    }
                    break;

                case 1: {

                        state ++;

                        result_subtype = evalExpression(action);

                        if(result_subtype == Lexeme::subtype_null)
                            continue;

                        addCast(result_subtype, Lexeme::subtype_numeric);

                        // ld a, l
                        addByte(0x7D);

                        has_color = true;

                    }
                    break;

                case 2: {

                        state ++;

                        lexeme = action->lexeme;
                        if(lexeme->subtype == Lexeme::subtype_null)
                            continue;

                        if(lexeme->type == Lexeme::type_literal) {
                            try {
                                line_type = stoi(lexeme->value);
                            } catch(exception &e) {
                                printf("Warning: error while converting numeric constant %s\n", lexeme->value.c_str());
                                line_type = 0;
                            }
                        } else {
                            syntax_error("Invalid shape parameter");
                        }

                        has_line_type = true;

                    }
                    break;

                case 3: {

                        state ++;

                        result_subtype = evalExpression(action);

                        if(result_subtype == Lexeme::subtype_null)
                            continue;

                        addCast(result_subtype, Lexeme::subtype_numeric);

                        // ld b, l
                        addByte(0x45);

                        has_operator = true;

                    }
                    break;

                default: {
                        syntax_error("LINE parameters not supported");
                        return;
                    }
            }

        }

        if(!has_color) {
            // ld a, (0xF3E9)       ; FORCLR
            addCmd(0x3A, 0xF3E9);
        }

        if(!has_operator) {
            // ld b, 0
            addWord(0x06, 0x00);
        }

        if(has_y1_coord) {
            // pop hl
            addByte(0xE1);
        } else {
            // ld hl, (0xFCB9)  ;GRPACY
            addCmd(0x2A, 0xFCB9);
        }

        if(has_x1_coord) {
            // pop de
            addByte(0xD1);
        } else {
            // ld de, (0xFCB7)  ;GRPACX
            addByte(0xED);
            addCmd(0x5B, 0xFCB7);
        }

        if(has_y0_coord) {
            // pop iy
            addWord(0xFD, 0xE1);
        } else {
            // ld iy, (0xFCB9)  ;GRPACY
            addByte(0xFD);
            addCmd(0x2A, 0xFCB9);
        }

        if(has_x0_coord) {
            // pop ix
            addWord(0xDD, 0xE1);
        } else {
            // ld ix, (0xFCB7)  ;GRPACX
            addByte(0xDD);
            addByte(0xED);
            addCmd(0x5B, 0xFCB7);
        }

        if(line_type == 0 || !has_line_type) {
            // call 0x6DA7   ; xbasic LINE (in: ix=x0, iy=y0, de=x1, hl=y1, a=color, b=operator)
            addCmd(0xCD, def_XBASIC_LINE);
        } else if(line_type == 1) {
            // call 0x6D49   ; xbasic BOX (in: ix=x0, iy=y0, de=x1, hl=y1, a=color, b=operator)
            addCmd(0xCD, def_XBASIC_BOX);
        } else {
            // call 0x6E27   ; xbasic BOX FILLED (in: ix=x0, iy=y0, de=x1, hl=y1, a=color, b=operator)
            addCmd(0xCD, def_XBASIC_BOXF);
        }

    } else {
        syntax_error("LINE with empty parameters");
    }

}

void Compiler::cmd_copy() {
    ActionNode *action, *sub_action;
    unsigned int i, t = current_action->actions.size();
    int result_subtype, state;
    bool has_x0_coord=false, has_x1_coord=false, has_x2_coord=false;
    bool has_y0_coord=false, has_y1_coord=false, has_y2_coord=false;
    bool has_src_page=false, has_dest_page=false, has_operator=false;
    bool has_address_from=false, has_address_to=false;

    if(t) {

        action = current_action->actions[0];

        state = 0;

        for(i = 0; i < t; i++) {

            action = current_action->actions[i];

            switch(state) {
                case 0: {
                        if(action->lexeme->value == "SCREEN") {
                            return cmd_copy_screen();
                        } else if(action->lexeme->value == "COORD") {

                            if(action->actions.size() != 2) {
                                syntax_error("Coordenates parameters error on COPY");
                                return;
                            }

                            sub_action = action->actions[0];

                            result_subtype = evalExpression(sub_action);

                            if(result_subtype == Lexeme::subtype_null) {
                                // ld hl, (0xFCB7)  ;GRPACX
                                addCmd(0x2A, 0xFCB7);
                                // push hl
                                addByte(0xE5);
                                has_x0_coord = true;
                            } else {
                                addCast(result_subtype, Lexeme::subtype_numeric);

                                // ld (0xFCB7), hl  ;GRPACX
                                addCmd(0x22, 0xFCB7);
                                // push hl
                                addByte(0xE5);
                                has_x0_coord = true;

                            }

                            sub_action = action->actions[1];

                            result_subtype = evalExpression(sub_action);

                            if(result_subtype == Lexeme::subtype_null) {
                                // ld hl, (0xFCB9)  ;GRPACY
                                addCmd(0x2A, 0xFCB9);
                                // push hl
                                addByte(0xE5);
                                has_y0_coord = true;
                            } else {
                                addCast(result_subtype, Lexeme::subtype_numeric);

                                // ld (0xFCB9), hl  ;GRPACY
                                addCmd(0x22, 0xFCB9);
                                // push hl
                                addByte(0xE5);
                                has_y0_coord = true;

                            }

                        } else if(action->lexeme->value == "STEP") {

                            sub_action = action->actions[0];

                            result_subtype = evalExpression(sub_action);

                            if(result_subtype == Lexeme::subtype_null) {
                                // ld hl, (0xFCB7)  ;GRPACX
                                addCmd(0x2A, 0xFCB7);
                                // push hl
                                addByte(0xE5);
                                has_x0_coord = true;
                            } else {
                                addCast(result_subtype, Lexeme::subtype_numeric);
                                // ld de, (0xFCB7)  ;GRPACX
                                addByte(0xED);
                                addCmd(0x5B, 0xFCB7);
                                // add hl, de
                                addByte(0x19);
                                // ld (0xFCB7), hl  ;GRPACX
                                addCmd(0x22, 0xFCB7);
                                // push hl
                                addByte(0xE5);
                                has_x0_coord = true;
                            }

                            sub_action = action->actions[1];

                            result_subtype = evalExpression(sub_action);

                            if(result_subtype == Lexeme::subtype_null) {
                                // ld hl, (0xFCB9)  ;GRPACY
                                addCmd(0x2A, 0xFCB9);
                                // push hl
                                addByte(0xE5);
                                has_y0_coord = true;
                            } else {
                                addCast(result_subtype, Lexeme::subtype_numeric);
                                // ld de, (0xFCB9)  ;GRPACY
                                addByte(0xED);
                                addCmd(0x5B, 0xFCB9);
                                // add hl, de
                                addByte(0x19);
                                // ld (0xFCB9), hl  ;GRPACY
                                addCmd(0x22, 0xFCB9);
                                // push hl
                                addByte(0xE5);
                                has_y0_coord = true;
                            }

                        } else if(action->lexeme->value == "TO_COORD") {

                            state ++;

                            sub_action = action->actions[0];

                            result_subtype = evalExpression(sub_action);

                            if(result_subtype == Lexeme::subtype_null) {
                            } else {
                                addCast(result_subtype, Lexeme::subtype_numeric);

                                // ld (0xFCB7), hl  ;GRPACX
                                addCmd(0x22, 0xFCB7);
                                // push hl
                                addByte(0xE5);
                                has_x1_coord = true;

                            }

                            sub_action = action->actions[1];

                            result_subtype = evalExpression(sub_action);

                            if(result_subtype == Lexeme::subtype_null) {
                            } else {
                                addCast(result_subtype, Lexeme::subtype_numeric);

                                // ld (0xFCB9), hl  ;GRPACY
                                addCmd(0x22, 0xFCB9);
                                // push hl
                                addByte(0xE5);
                                has_y1_coord = true;

                            }

                        } else if(action->lexeme->value == "TO_STEP") {

                            state ++;

                            sub_action = action->actions[0];

                            result_subtype = evalExpression(sub_action);

                            if(result_subtype == Lexeme::subtype_null) {
                            } else {
                                addCast(result_subtype, Lexeme::subtype_numeric);
                                // ld de, (0xFCB7)  ;GRPACX
                                addByte(0xED);
                                addCmd(0x5B, 0xFCB7);
                                // add hl, de
                                addByte(0x19);
                                // ld (0xFCB7), hl  ;GRPACX
                                addCmd(0x22, 0xFCB7);
                                // push hl
                                addByte(0xE5);
                                has_x1_coord = true;
                            }

                            sub_action = action->actions[1];

                            result_subtype = evalExpression(sub_action);

                            if(result_subtype == Lexeme::subtype_null) {
                            } else {
                                addCast(result_subtype, Lexeme::subtype_numeric);
                                // ld de, (0xFCB9)  ;GRPACY
                                addByte(0xED);
                                addCmd(0x5B, 0xFCB9);
                                // add hl, de
                                addByte(0x19);
                                // ld (0xFCB9), hl  ;GRPACY
                                addCmd(0x22, 0xFCB9);
                                // push hl
                                addByte(0xE5);
                                has_y1_coord = true;
                            }

                        } else {
                            has_address_from = true;

                            result_subtype = evalExpression(action);

                            if(result_subtype != Lexeme::subtype_numeric &&
                                    result_subtype != Lexeme::subtype_single_decimal &&
                                    result_subtype != Lexeme::subtype_double_decimal) {
                                syntax_error("Invalid address in COPY");
                                return;
                            }

                            addCast(result_subtype, Lexeme::subtype_numeric);

                            // push hl
                            addByte(0xE5);

                            state ++;

                        }

                    }
                    break;

                case 1: {

                        state ++;

                        result_subtype = evalExpression(action);

                        if(result_subtype == Lexeme::subtype_null)
                            continue;

                        addCast(result_subtype, Lexeme::subtype_numeric);

                        // ld a, l
                        addByte(0x7D);
                        // push af
                        addByte(0xF5);

                        has_src_page = true;

                    }
                    break;

                case 2: {

                        if(action->lexeme->value == "TO_DEST") {

                            state ++;

                            sub_action = action->actions[0];

                            result_subtype = evalExpression(sub_action);

                            if(result_subtype == Lexeme::subtype_null) {
                            } else {
                                addCast(result_subtype, Lexeme::subtype_numeric);

                                // ld (0xFCB7), hl  ;GRPACX
                                addCmd(0x22, 0xFCB7);
                                // push hl
                                addByte(0xE5);
                                has_x2_coord = true;

                            }

                            sub_action = action->actions[1];

                            result_subtype = evalExpression(sub_action);

                            if(result_subtype == Lexeme::subtype_null) {
                            } else {
                                addCast(result_subtype, Lexeme::subtype_numeric);

                                // ld (0xFCB9), hl  ;GRPACY
                                addCmd(0x22, 0xFCB9);
                                // push hl
                                addByte(0xE5);
                                has_y2_coord = true;

                            }

                        } else {

                            has_address_to = true;

                            result_subtype = evalExpression(action);

                            if(result_subtype == Lexeme::subtype_null)
                                continue;

                            addCast(result_subtype, Lexeme::subtype_numeric);

                            state = 99;   // exit loop

                        }

                    }
                    break;

                case 3: {

                        state ++;

                        result_subtype = evalExpression(action);

                        if(result_subtype == Lexeme::subtype_null)
                            continue;

                        addCast(result_subtype, Lexeme::subtype_numeric);

                        // ld a, l
                        addByte(0x7D);
                        // push af
                        addByte(0xF5);

                        has_dest_page = true;

                    }
                    break;

                case 4: {

                        state ++;

                        result_subtype = evalExpression(action);

                        if(result_subtype == Lexeme::subtype_null)
                            continue;

                        addCast(result_subtype, Lexeme::subtype_numeric);

                        // ld a, l
                        addByte(0x7D);

                        has_operator = true;

                    }
                    break;

                default: {
                        syntax_error("COPY parameters not supported");
                        return;
                    }
            }

        }

        if(has_address_from) {

            if(!has_operator) {
                // xor a
                addByte(0xAF);
            }
            // ld (LOGOP), a
            addCmd(0x32, def_LOGOP);

            if(has_dest_page) {
                // pop af
                addByte(0xF1);
                // ld (ACPAGE), a
                addCmd(0x32, def_ACPAGE);
            }

            if(has_y2_coord) {
                // pop hl
                addByte(0xE1);
            } else {
                // ld hl, (0xFCB9)  ;GRPACY
                addCmd(0x2A, 0xFCB9);
            }
            // ld (DY), hl
            addCmd(0x22, def_DY);

            if(has_x2_coord) {
                // pop hl
                addByte(0xE1);
            } else {
                // ld hl, (0xFCB7)  ;GRPACX
                addCmd(0x2A, 0xFCB7);
            }
            // ld (DX), hl
            addCmd(0x22, def_DX);

            if(has_src_page) {
                // pop af
                addByte(0xF1);
                // add a,a
                addByte(0x87);
                // add a,a
                addByte(0x87);
                // ld (ARGT), a    ; direction/expansion (0000DDEE)
                addCmd(0x32, def_ARGT);
            }

            // pop hl
            addByte(0xE1);

            // call XBASIC_COPY_FROM
            addCmd(0xCD, def_XBASIC_COPY_FROM);


        } else if(has_address_to) {

            // ex de,hl      ; address to (hl to de)
            addByte(0xEB);

            if(has_src_page) {
                // pop af
                addByte(0xF1);
                // ld (ACPAGE), a
                addCmd(0x32, def_ACPAGE);
            }

            if(has_y1_coord) {
                // pop hl
                addByte(0xE1);
            } else {
                // ld hl, (0xFCB9)  ;GRPACY
                addCmd(0x2A, 0xFCB9);
            }
            // ld (NY), hl
            addCmd(0x22, def_NY);

            if(has_x1_coord) {
                // pop hl
                addByte(0xE1);
            } else {
                // ld hl, (0xFCB7)  ;GRPACX
                addCmd(0x2A, 0xFCB7);
            }
            // ld (NX), hl
            addCmd(0x22, def_NX);

            if(has_y0_coord) {
                // pop hl
                addByte(0xE1);
            } else {
                // ld hl, (0xFCB9)  ;GRPACY
                addCmd(0x2A, 0xFCB9);
            }
            // ld (SY), hl
            addCmd(0x22, def_SY);

            if(has_x0_coord) {
                // pop hl
                addByte(0xE1);
            } else {
                // ld hl, (0xFCB7)  ;GRPACX
                addCmd(0x2A, 0xFCB7);
            }
            // ld (SX), hl
            addCmd(0x22, def_SX);

            // ex de,hl
            addByte(0xEB);

            // call XBASIC_COPY_TO
            addCmd(0xCD, def_XBASIC_COPY_TO);

        } else {

            if(!has_operator) {
                // ld b, 0
                addWord(0x06, 0x00);
            }

            if(has_dest_page) {
                // pop af
                addByte(0xF1);
            } else {
                // ld a, (ACPAGE)
                addCmd(0x3A, def_ACPAGE);
            }
            // ld (0xFC19), a
            addCmd(0x32, 0xFC19);

            if(has_y2_coord) {
                // pop hl
                addByte(0xE1);
                // ld (0xFCB9), hl  ;GRPACY
                addCmd(0x22, 0xFCB9);
            }

            if(has_x2_coord) {
                // pop hl
                addByte(0xE1);
                // ld (0xFCB7), hl  ;GRPACX
                addCmd(0x22, 0xFCB7);
            }

            if(has_src_page) {
                // pop af
                addByte(0xF1);
            } else {
                // ld a, (ACPAGE)
                addCmd(0x3A, def_ACPAGE);
            }
            // ld (0xFC18), a
            addCmd(0x32, 0xFC18);

            if(has_y1_coord) {
                // pop iy
                addWord(0xFD, 0xE1);
            } else {
                // ld iy, (0xFCB9)  ;GRPACY
                addByte(0xFD);
                addCmd(0x2A, 0xFCB9);
            }

            if(has_x1_coord) {
                // pop ix
                addWord(0xDD, 0xE1);
            } else {
                // ld ix, (0xFCB7)  ;GRPACX
                addByte(0xDD);
                addByte(0xED);
                addCmd(0x5B, 0xFCB7);
            }

            if(has_y0_coord) {
                // pop hl
                addByte(0xE1);
            } else {
                // ld hl, (0xFCB9)  ;GRPACY
                addCmd(0x2A, 0xFCB9);
            }

            if(has_x0_coord) {
                // pop de
                addByte(0xD1);
            } else {
                // ld de, (0xFCB7)  ;GRPACX
                addByte(0xED);
                addCmd(0x5B, 0xFCB7);
            }

            // call COPY    ; in: de=x0, hl=y0, ix=x1, iy=y1, 0xFC18=srcpg, 0xFCB7=x2, 0xFCB9=y2, 0xFC19=destpg, b=operator
            addCmd(0xCD, def_XBASIC_COPY);

        }

    } else {
        syntax_error("COPY with empty parameters");
    }

}

void Compiler::cmd_copy_screen() {
    int t = current_action->actions.size();

    if(t > 1) {
        syntax_error("Invalid COPY SCREEN parameters");
    } else {

        if(t == 0) {
            // xor a
        } else if(t == 1) {
            ActionNode *action = current_action->actions[0], *sub_action;
            int result_subtype;

            sub_action = action->actions[0];
            result_subtype = evalExpression(sub_action);

            addCast(result_subtype, Lexeme::subtype_numeric);

            // ld a, l
            addByte(0x7D);
        }

        // and 1
        addWord(0xE6, 0x01);
        // inc a
        addByte(0x3C);
        // or 0x10
        addWord(0xF6, 0x10);
        // ld hl, BUF
        addCmd(0x21, def_BUF);
        // push hl
        addByte(0xE5);
        //   ld (hl), 0xC5  ; SCREEN token
        addWord(0x36, 0xC5);
        //   inc hl
        addByte(0x23);
        //   ld (hl), a
        addByte(0x77);
        //   inc hl
        addByte(0x23);
        //   xor a
        addByte(0xAF);
        //   ld (hl), a
        addByte(0x77);
        //   inc hl
        addByte(0x23);
        //   ld (hl), a
        addByte(0x77);
        // pop hl
        addByte(0xE1);

        // ld a, (VERSION)
        addCmd(0x3A, def_VERSION);
        // and a
        addByte(0xA7);
        // jr z, skip
        addWord(0x28, 9);

        //   ld a, (hl)       ; first character
        addByte(0x7E);
        //   ld ix, (COPY)    ; COPY
        addWord(0xDD, 0x2A);
        addWord(def_COPY_STMT);
        //   call CALBAS
        addCmd( 0xcd, def_CALBAS );
        //   ei
        addByte(0xFB);

        // skip:
    }

    return;
}


void Compiler::cmd_sound() {
    Lexeme *lexeme;
    ActionNode *action;
    unsigned int i, t = current_action->actions.size();
    int result_subtype;

    if(t == 2) {

        for( i = 0; i < t; i++) {
            action = current_action->actions[i];
            lexeme = action->lexeme;
            if(lexeme) {

                result_subtype = evalExpression(action);

                // cast
                addCast(result_subtype, Lexeme::subtype_numeric);

                if(i == 0) {
                    // ld a, l
                    addByte( 0x7D );
                    // push af
                    addByte( 0xF5 );
                } else {
                    // ld e, l
                    addByte( 0x5D );
                    // pop af
                    addByte( 0xF1 );

                    // call sound function
                    addCmd(0xCD, def_XBASIC_SOUND);
                }

            }

        }

    } else {
        syntax_error("Invalid SOUND parameters");
    }

}

void Compiler::cmd_bload() {
    Lexeme *lexeme;
    ActionNode *action;
    unsigned int t = current_action->actions.size();
    FileNode *file;
    string fileExt;
    bool isTinySprite;
    int bytes;

    if(t == 2) {

        action = current_action->actions[1];
        lexeme = action->lexeme;
        if(lexeme) {

            if(lexeme->name == "S") {

                action = current_action->actions[0];
                lexeme = action->lexeme;
                if(lexeme) {

                    if(lexeme->type == Lexeme::type_literal && lexeme->subtype == Lexeme::subtype_string) {

                        file = new FileNode();
                        file->name = lexeme->value;
                        file->name.resize(file->name.size()-1);
                        file->name.erase(0, 1);
                        file->packed = true;
                        fileExt = file->getFileExt();

                        isTinySprite = (strcasecmp(fileExt.c_str(), ".SPR")==0);

                        if (file->open()) {

                            fileList.push_back(file);

                            if(isTinySprite) {
                                unsigned char *spr_data = (unsigned char *) malloc(0x4000);
                                int i=0, total_bytes;
                                file->close();
                                if(!spr_data) {
                                    syntax_error("Tiny Sprite file processing memory allocation error");
                                    return;
                                }
                                total_bytes = file->ParseTinySpriteFile((char *)file->name.c_str(), spr_data, 0x4000);
                                if(total_bytes <= 0) {
                                    syntax_error("Invalid Tiny Sprite file");
                                    free(spr_data);
                                    return;
                                }

                                file->length = total_bytes;
                                while(total_bytes) {
                                    if(total_bytes > 200) {
                                        bytes = file->readAsLexeme(&spr_data[i], 200);
                                        total_bytes -= 200;
                                        i += 200;
                                    } else {
                                        bytes = file->readAsLexeme(&spr_data[i], total_bytes);
                                        total_bytes = 0;
                                    }
                                    if(bytes == 0) {
                                        syntax_error("Error packing Tiny Sprite file in BLOAD (empty block size)");
                                        free(spr_data);
                                        return;
                                    }
                                    if(bytes > 255) {
                                        syntax_error("Error packing Tiny Sprite file in BLOAD (block size > 255 bytes)");
                                        free(spr_data);
                                        return;
                                    }
                                    addSymbol(file->current_lexeme);
                                }
                                free(spr_data);

                                // call CLRSPR    ; clear sprites
                                addCmd(0xCD, def_CLRSPR);
                            } else {
                                while(!file->eof()) {
                                    bytes = file->readAsLexeme();
                                    if(bytes == 0) {
                                        file->close();
                                        syntax_error("Error packing image file in BLOAD (empty block size)");
                                        return;
                                    }
                                    if(bytes > 255) {
                                        file->close();
                                        syntax_error("Error packing image file in BLOAD (block size > 255 bytes)");
                                        return;
                                    }
                                    addSymbol(file->current_lexeme);
                                }
                                file->close();
                            }

                            if(file->first_lexeme) {

                                if( file->file_header[0] == 0xFE || isTinySprite ) {

                                    if(megaROM) {
                                        // special ld hl, first file block address
                                        addFix(file->first_lexeme);
                                        addCmd(0xFF, 0x0000);
                                    } else {
                                        // ld hl, first file block address
                                        addFix(file->first_lexeme);
                                        addCmd(0x21, 0x0000);
                                    }

                                } else {
                                    syntax_error("Invalid screen file format in BLOAD");
                                    return;
                                }

                            } else {
                                if(isTinySprite) {
                                    syntax_error("Empty Tiny Sprite file in BLOAD");
                                } else {
                                    syntax_error("Empty screen file in BLOAD");
                                }
                                return;
                            }

                            // ld bc, blocks count
                            addCmd(0x01, file->blocks);

                            if(isTinySprite) {
                                if(megaROM) {
                                    // exx
                                    addByte(0xD9);
                                    // ld hl, SUB_BLOAD_SPRITE
                                    addCmd(0x21, def_XBASIC_BLOAD_SPRITE);
                                    // call MR_CALL
                                    addCmd(0xCD, def_MR_CALL);
                                } else {
                                    // call bload function
                                    addCmd(0xCD, def_XBASIC_BLOAD_SPRITE);
                                }
                            } else {
                                if(megaROM) {
                                    // exx
                                    addByte(0xD9);
                                    // ld hl, SUB_LOAD
                                    addCmd(0x21, def_XBASIC_BLOAD);
                                    // call MR_CALL
                                    addCmd(0xCD, def_MR_CALL);
                                } else {
                                    // call bload function
                                    addCmd(0xCD, def_XBASIC_BLOAD);
                                }
                            }

                        } else {
                            syntax_error("BLOAD file not found");
                        }

                    } else {
                        syntax_error("BLOAD file name is missing to load into ROM");
                    }

                } else {
                    syntax_error("BLOAD 1st parameter error");
                }

            } else {
                syntax_error("BLOAD valid only to screen");
            }

        } else {
            syntax_error("BLOAD 2nd parameter error");
        }

    } else {
        syntax_error("Invalid BLOAD parameters count");
    }

}

void Compiler::cmd_play() {
    ActionNode *action;
    unsigned int i, t = current_action->actions.size();
    int result_subtype;
    bool xor_a = true;

    if(t >= 1 && t <= 3) {

        for( i = 0; i < 3; i++) {
            if( i < t ) {
                action = current_action->actions[i];
                result_subtype = evalExpression(action);
                if(result_subtype != Lexeme::subtype_string) {
                    syntax_error("Invalid PLAY parameter");
                    return;
                }
                // push hl
                addByte(0xE5);
            } else {
                if(xor_a) {
                    // xor a
                    addByte(0xAF);
                    xor_a = false;
                }
                // push af
                addByte(0xF5);
            }
        }

        // pop bc
        addByte(0xC1);
        // pop de
        addByte(0xD1);
        // pop hl
        addByte(0xE1);

        addCmd(0xCD, def_XBASIC_PLAY);

    } else {
        syntax_error("Invalid PLAY parameters");
    }

}

void Compiler::cmd_draw() {
    Lexeme *lexeme;
    ActionNode *action;
    unsigned int i, t = current_action->actions.size();
    int result_subtype;

    if(t == 1) {

        i = 0;
        action = current_action->actions[i];
        lexeme = action->lexeme;
        if(lexeme) {

            result_subtype = evalExpression(action);

            if(result_subtype == Lexeme::subtype_string) {

                // call draw function
                if(draw_mark)
                    addFix(draw_mark->symbol);
                else
                    draw_mark = addMark();
                addCmd(0xCD, 0x0000);

            } else {
                syntax_error("Invalid DRAW parameter");
                return;
            }

        }

    } else {
        syntax_error("Invalid DRAW parameters");
    }

}

void Compiler::cmd_dim() {
    Lexeme *lexeme, *parm_lexeme;
    ActionNode *action;
    unsigned int i, k, w, tt, t = current_action->actions.size();
    int new_size;

    if(!t) {
        syntax_error("DIM parameters is missing");
    } else {

        for(i = 0; i < t; i++) {

            action = current_action->actions[i];
            lexeme = action->lexeme;

            if(lexeme->type == Lexeme::type_identifier) {

                if(lexeme->subtype == Lexeme::subtype_string)
                    lexeme->x_factor = 256;
                else if(lexeme->subtype == Lexeme::subtype_numeric)
                    lexeme->x_factor = 2;
                else if(lexeme->subtype == Lexeme::subtype_single_decimal || lexeme->subtype == Lexeme::subtype_double_decimal)
                    lexeme->x_factor = 3;
                else
                    lexeme->x_factor = 0;

                tt = action->actions.size();
                if(tt >= 1 && tt <= 2) {

                    lexeme->isArray = true;

                    if(tt == 1) {
                        k = 0;
                        w = 1;
                    } else {
                        k = 1;
                        w = 0;
                    }

                    parm_lexeme = action->actions[k]->lexeme;
                    if(parm_lexeme->type == Lexeme::type_literal && parm_lexeme->subtype == Lexeme::subtype_numeric) {
                        try {
                            lexeme->x_size = stoi(parm_lexeme->value) + 1;
                        } catch(exception &e) {
                            printf("Warning: error while converting numeric constant %s\n", parm_lexeme->value.c_str());
                            lexeme->x_size = 0;
                        }
                        if(!lexeme->x_size) {
                            syntax_error("Array 1st dimension index cannot be zero");
                            break;
                        }
                    } else {
                        syntax_error("Array 1st dimension index must be a integer constant");
                        break;
                    }

                    if(tt == 2) {
                        parm_lexeme = action->actions[w]->lexeme;
                        if(parm_lexeme->type == Lexeme::type_literal && parm_lexeme->subtype == Lexeme::subtype_numeric) {
                            try {
                                lexeme->y_size = stoi(parm_lexeme->value) + 1;
                            } catch(exception &e) {
                                printf("Warning: error while converting numeric constant %s\n", parm_lexeme->value.c_str());
                                lexeme->y_size = 0;
                            }
                            if(!lexeme->y_size) {
                                syntax_error("Array 2nd dimension index cannot be zero");
                                break;
                            }
                        } else {
                            syntax_error("Array 2nd dimension index must be a integer constant");
                            break;
                        }
                    } else
                        lexeme->y_size = 1;

                    lexeme->y_factor = lexeme->x_factor * lexeme->x_size;
                    new_size = lexeme->y_factor * lexeme->y_size;

                    if(lexeme->array_size < new_size)
                        lexeme->array_size = new_size;
                } else {
                    syntax_error("Arrays with more than 2 dimensions isn't supported");
                    break;
                }

            } else {
                syntax_error("Invalid DIM parameter");
                break;
            }

        }

    }

}

void Compiler::cmd_redim() {

    cmd_dim();

}

void Compiler::cmd_let() {
    Lexeme *lexeme;
    ActionNode *action, *lex_action;
    unsigned int t = current_action->actions.size();
    int result_subtype, result[3];

    if(t != 2) {
        syntax_error("Invalid LET parameters count");
        return;
    }

    lex_action = current_action->actions[0];
    lexeme = lex_action->lexeme;

    // ld hl, data parameter

    action = current_action->actions[1];
    result_subtype = evalExpression(action);

    // do assignment

    if(lexeme->value == "MID$") {

        // cast
        addCast(result_subtype, Lexeme::subtype_string);

        // push hl
        addByte(0xE5);

        t = lex_action->actions.size();

        if(t < 2 || t > 3) {
            syntax_error("Invalid MID$ assignment parameters count");
            return;
        }

        if(!evalOperatorParms(lex_action, t)) {
            syntax_error("Invalid MID$ assignment parameters");
            return;
        }

        result[0] = lex_action->actions[0]->subtype;
        result[1] = lex_action->actions[1]->subtype;

        if( t == 2 ) {

            if( result[0] == Lexeme::subtype_single_decimal || result[0] == Lexeme::subtype_double_decimal ) {
                // cast
                addCast( result[0], Lexeme::subtype_numeric );
                result[0] = Lexeme::subtype_numeric;
            }

            if( result[1] == Lexeme::subtype_string && result[0] == Lexeme::subtype_numeric ) {

                // ld a, l         ; start char
                addByte(0x7D);
                // pop de          ; de=destination string
                addByte(0xD1);
                // pop hl          ; hl=source string
                addByte(0xE1);

                // ld b, (hl)      ; number of chars (all from source)
                addWord(0x46);

                // call 0x7dd8    ; mid assignment (in: hl=source string, b=size, a=start, de=destination string)
                addCmd(0xCD, def_XBASIC_MID_ASSIGN);

                return;

            }

        } else {

            result[2] = lex_action->actions[2]->subtype;

            if( result[0] == Lexeme::subtype_single_decimal || result[0] == Lexeme::subtype_double_decimal ) {
                // cast
                addCast( result[0], Lexeme::subtype_numeric );
                result[0] = Lexeme::subtype_numeric;
            }

            // ld b, l             ; number of chars
            addByte(0x45);

            if( result[1] == Lexeme::subtype_single_decimal || result[1] == Lexeme::subtype_double_decimal ) {
                // ld a, b
                addByte(0x78);
                // pop bc
                addByte(0xC1);
                // pop hl
                addByte(0xE1);
                // push af
                addByte(0xF5);
                // cast
                addCast( result[1], Lexeme::subtype_numeric );
                // pop bc
                addByte(0xC1);
                result[1] = Lexeme::subtype_numeric;
            } else {
                // pop hl
                addByte(0xE1);
            }

            // ld a, l         ; start char
            addByte(0x7D);

            if( result[2] == Lexeme::subtype_string && result[1] == Lexeme::subtype_numeric && result[0] == Lexeme::subtype_numeric ) {

                // pop de          ; de=destination string
                addByte(0xD1);
                // pop hl          ; hl=source string
                addByte(0xE1);

                // call 0x7dd8    ; mid assignment (in: hl=source string, b=size, a=start, de=destination string)
                addCmd(0xCD, def_XBASIC_MID_ASSIGN);

                return;
            }

        }

        syntax_error("Invalid MID$ assignment type");

    } else if (lexeme->value == "VDP") {

        // cast
        addCast(result_subtype, Lexeme::subtype_numeric);

        // ld a, l
        addByte(0x7D);
        // push af
        addByte(0xF5);

        t = lex_action->actions.size();

        if(t != 1) {
            syntax_error("Invalid VDP assignment parameters count");
            return;
        }

        if(!evalOperatorParms(lex_action, t)) {
            syntax_error("Invalid VDP assignment parameters");
            return;
        }

        result[0] = lex_action->actions[0]->subtype;

        // cast
        addCast( result[0], Lexeme::subtype_numeric );
        result[0] = Lexeme::subtype_numeric;

        // pop bc
        addByte(0xC1);
        // ld a, l
        addByte(0x7D);
        // cp 0x08
        addWord(0xFE, 0x08);
        // adc a, 0xff
        addWord(0xCE, 0xFF);
        // ld c, a
        addByte(0x4F);

        // call 0x0047     ; VDP assignment (in: b=data, c=port)
        addCmd(0xCD, 0x0047);

    } else if (lexeme->value == "SPRITE$") {

        // cast
        addCast(result_subtype, Lexeme::subtype_string);

        // push hl
        addByte(0xE5);

        t = lex_action->actions.size();

        if(t != 1) {
            syntax_error("Invalid SPRITE$ assignment parameters count");
            return;
        }

        if(!evalOperatorParms(lex_action, t)) {
            syntax_error("Invalid SPRITE$ assignment parameters");
            return;
        }

        result[0] = lex_action->actions[0]->subtype;

        // cast
        addCast( result[0], Lexeme::subtype_numeric );
        result[0] = Lexeme::subtype_numeric;

        // ld a, l
        addByte(0x7D);
        // pop hl
        addByte(0xE1);

        // call 0x7143     ; xbasic SPRITE assignment (in: a=sprite pattern, hl=string)
        addCmd(0xCD, def_XBASIC_SPRITE_ASSIGN);

    } else {

        // cast

        addCast(result_subtype, lexeme->subtype);

        // do assignment

        addAssignment(lex_action);

    }

}

bool Compiler::addAssignment(ActionNode *action) {

    if(action->lexeme->type == Lexeme::type_keyword) {

        if(action->lexeme->value == "TIME") {

            // ld (0xFC9E), hl    ; JIFFY
            addCmd(0x22, 0xFC9E);

        } else if(action->lexeme->value == "MAXFILES") {

            // ld a, l
            addByte(0x7D);
            // ld ix, MAXFILES
            addByte(0xDD);
            addCmd(0x21, def_MAXFILES);
            // call CALBAS
            addCmd(0xCD, def_CALBAS);
            // ei
            addByte(0xFB);

        } else {
            syntax_error("Invalid KEYWORD/FUNCTION assignment");
        }

    } else if(action->lexeme->type == Lexeme::type_identifier) {

        if(action->lexeme->isArray || action->lexeme->subtype == Lexeme::subtype_string) {

            // push hl
            addByte(0xE5);

            if(action->lexeme->subtype == Lexeme::subtype_single_decimal || action->lexeme->subtype == Lexeme::subtype_double_decimal) {
                // push bc
                addByte(0xC5);
            }

            if(!addVarAddress(action))
                return false;

            if(action->lexeme->subtype == Lexeme::subtype_string) {

                // pop de
                addByte(0xD1);
                // ex de,hl
                addByte(0xEB);

                // call 0x7e9d   ; xbasic copy string (in: hl=source, de=dest; out: hl end of string)
                addCmd(0xCD, def_XBASIC_COPY_STRING);

            } else if(action->lexeme->subtype == Lexeme::subtype_numeric) {

                // pop de
                addByte(0xD1);
                // ld (hl),e
                addByte(0x73);
                // inc hl
                addByte(0x23);
                // ld (hl),d
                addByte(0x72);

            } else if(action->lexeme->subtype == Lexeme::subtype_single_decimal || action->lexeme->subtype == Lexeme::subtype_double_decimal) {

                // pop bc
                addByte(0xC1);
                // pop de
                addByte(0xD1);
                // ld (hl),b
                addByte(0x70);
                // inc hl
                addByte(0x23);
                // ld (hl),e
                addByte(0x73);
                // inc hl
                addByte(0x23);
                // ld (hl),d
                addByte(0x72);

            } else {
                syntax_error("Invalid assignment");
                return false;
            }

        } else {

            // assignment optimization

            if(action->lexeme->subtype == Lexeme::subtype_numeric) {

                // ld (var), hl
                addFix(action->lexeme);
                addCmd(0x22, 0x0000);

            } else if(action->lexeme->subtype == Lexeme::subtype_single_decimal || action->lexeme->subtype == Lexeme::subtype_double_decimal) {

                // ld a, b
                addByte(0x78);
                // ld (var), a
                addFix(action->lexeme);
                addCmd(0x32, 0x0000);
                // ld (var+1), hl
                addFix(action->lexeme)->step = 1;
                addCmd(0x22, 0x0000);

            } else {
                syntax_error("Invalid assignment");
                return false;
            }

        }

    } else {
        syntax_error("Invalid constant/expression assignment");
        return false;
    }

    return true;
}

void Compiler::cmd_if() {
    Lexeme *lexeme, *last_lexeme;
    ActionNode *action, *saved_action = current_action, *last_action;
    unsigned int i, t = saved_action->actions.size(), tt;
    int result_subtype;
    FixNode *mark_else=0, *mark_endif=0;
    bool isLastActionGoto=false, isElseLikeEndif = true;

    if(!t) {
        syntax_error("IF parameters is missing");
    } else {

        for(i = 0; i < t; i++) {

            action = saved_action->actions[i];
            lexeme = action->lexeme;

            if(lexeme->type == Lexeme::type_keyword) {

                if(lexeme->value == "COND") {

                    // ld hl, data parameter

                    result_subtype = evalExpression(action->actions[0]);

                    if(result_subtype == Lexeme::subtype_numeric) {

                        // ld a, l
                        addByte(0x7D);
                        // or h
                        addByte(0xB4);
                        // jp z, ELSE or ENDIF
                        mark_else = addMark();
                        addCmd(0xCA, 0x0000);

                    } else {
                        syntax_error("Invalid condition expression");
                        break;
                    }

                } else if(lexeme->value == "THEN") {

                    tt = action->actions.size();
                    if(tt) {
                        last_action = action->actions[tt-1];
                        last_lexeme = last_action->lexeme;
                        isLastActionGoto = (last_lexeme->type == Lexeme::type_keyword && last_lexeme->value == "GOTO");
                    }

                    if(!evalActions(action))
                        break;

                } else if(lexeme->value == "GOTO") {

                    isLastActionGoto = true;

                    if(!evalAction(action))
                        break;

                } else if(lexeme->value == "GOSUB") {

                    if(!evalAction(action))
                        break;

                } else if(lexeme->value == "ELSE") {

                    if(!isLastActionGoto) {
                        // jp ENDIF
                        mark_endif = addMark();
                        addCmd(0xC3, 0x0000);
                    }

                    isLastActionGoto = false;
                    isElseLikeEndif = false;

                    tt = action->actions.size();
                    if(tt==1) {
                        last_action = action->actions[0];
                        last_lexeme = last_action->lexeme;
                        isLastActionGoto = (last_lexeme->type == Lexeme::type_keyword && last_lexeme->value == "GOTO");
                    }

                    if(isLastActionGoto) {
                        if(last_action->actions.size()) {
                            last_lexeme = last_action->actions[0]->lexeme;
                        }

                        if(last_lexeme->type == Lexeme::type_literal && last_lexeme->subtype == Lexeme::subtype_numeric) {
                            // mark ELSE position
                            if(mark_else) {
                                mark_else->symbol = addSymbol(last_lexeme->value);
                            } else {
                                syntax_error("ELSE parameter is missing");
                            }
                        } else {
                            syntax_error("Invalid GOTO parameter");
                        }

                    } else {
                        // mark ELSE position
                        if(mark_else)
                            mark_else->symbol->address = code_pointer;

                        if(!evalActions(action))
                            break;
                    }

                } else {
                    syntax_error("Invalid IF syntax");
                    break;
                }

            } else {
                syntax_error("Invalid IF parameter type");
                break;
            }

        }

        // mark ENDIF position
        if(mark_endif)
            mark_endif->symbol->address = code_pointer;
        else if(mark_else)
            if(isElseLikeEndif)
                mark_else->symbol->address = code_pointer;

    }

}

void Compiler::cmd_for() {
    Lexeme *lexeme, *lex_var=0;
    ActionNode *action, *var_action, *saved_action = current_action;
    unsigned int i, t = saved_action->actions.size();
    int result_subtype;
    ForNextNode *forNext;
    bool has_let=false, has_to=false, has_step=false;

    if(!t) {
        syntax_error("FOR parameters is missing");
    } else {

        for_count ++;

        forNext = new ForNextNode();
        forNextStack.push(forNext);

        forNext->index = for_count;
        forNext->tag = current_tag;
        forNext->for_to = new Lexeme(Lexeme::type_identifier, Lexeme::subtype_numeric, "FOR_TO_" + to_string(for_count));
        forNext->for_to_action = new ActionNode();
        forNext->for_to_action->lexeme = forNext->for_to;
        addSymbol(forNext->for_to);

        forNext->for_step = new Lexeme(Lexeme::type_identifier, Lexeme::subtype_numeric, "FOR_STEP_" + to_string(for_count));
        forNext->for_step_action = new ActionNode();
        forNext->for_step_action->lexeme = forNext->for_step;
        addSymbol(forNext->for_step);

        for(i = 0; i < t; i++) {

            action = saved_action->actions[i];
            lexeme = action->lexeme;

            if(lexeme->type == Lexeme::type_keyword) {

                if(lexeme->value == "LET") {

                    if(action->actions.size()) {
                        var_action = action->actions[0];
                        lex_var = var_action->lexeme;
                        forNext->for_var = lex_var;
                        forNext->for_to->subtype = lex_var->subtype;
                        forNext->for_step->subtype = lex_var->subtype;
                    } else {
                        syntax_error("Invalid FOR expression (variable assignment)");
                        return;
                    }

                    if(lex_var->type != Lexeme::type_identifier) {
                        syntax_error("Invalid FOR expression (variable is missing)");
                        return;
                    } else {
                        if(lex_var->subtype != Lexeme::subtype_numeric &&
                                lex_var->subtype != Lexeme::subtype_single_decimal &&
                                lex_var->subtype != Lexeme::subtype_double_decimal) {
                            syntax_error("Invalid FOR expression (wrong data type)");
                            return;
                        }
                    }

                    if(!evalAction(action))
                        return;

                    has_let = true;

                } else if(lexeme->value == "TO") {

                    // ld hl, data parameter

                    result_subtype = evalExpression(action->actions[0]);

                    if(result_subtype == Lexeme::subtype_numeric ||
                            result_subtype == Lexeme::subtype_single_decimal ||
                            result_subtype == Lexeme::subtype_double_decimal) {

                        addCast(result_subtype, lex_var->subtype);

                        addAssignment(forNext->for_to_action);

                        has_to = true;

                    } else {
                        syntax_error("Invalid TO expression (wrong data type)");
                        return;
                    }

                } else if(lexeme->value == "STEP") {

                    // ld hl, data parameter

                    result_subtype = evalExpression(action->actions[0]);

                    if(result_subtype == Lexeme::subtype_numeric ||
                            result_subtype == Lexeme::subtype_single_decimal ||
                            result_subtype == Lexeme::subtype_double_decimal) {

                        addCast(result_subtype, lex_var->subtype);

                        addAssignment(forNext->for_step_action);

                        has_step = true;

                    } else {
                        syntax_error("Invalid STEP expression (wrong data type)");
                        return;
                    }

                } else {
                    syntax_error("Invalid FOR syntax");
                    return;
                }

            } else {
                syntax_error("Invalid FOR parameter type");
                return;
            }

        }

        if(has_let && has_to) {

            if(!has_step && lex_var->subtype != Lexeme::subtype_numeric) {
                // ld hl, 1
                addCmd(0x21, 0x0001);

                addCast(Lexeme::subtype_numeric, forNext->for_step->subtype);

                addAssignment(forNext->for_step_action);
            }

            if(lex_var->subtype == Lexeme::subtype_numeric) {
                // ld hl, (variable)
                addFix( forNext->for_var );
                addCmd(0x2A, 0x0000);

                // jr $+12      ; jump to check code
                if(has_step) {
                    addWord(0x18, 0x0B);
                } else {
                    addWord(0x18, 0x07);
                }

                // ;step code
                forNext->for_step_mark = addPreMark();
                forNext->for_step_mark->address = code_pointer;

                // ld hl, (variable)
                addFix( forNext->for_var );
                addCmd(0x2A, 0x0000);

                if(has_step) {
                    // ld de, (step)
                    addByte(0xED);
                    addFix( forNext->for_step );
                    addCmd(0x5B, 0x0000);
                    // add hl,de
                    addByte(0x19);
                } else {
                    // inc hl
                    addByte(0x23);
                }

                // ld (variable), hl
                addFix( forNext->for_var );
                addCmd(0x22, 0x0000);

                // ;check code

                // ex de, hl         ; after, de = (variable)
                addByte(0xEB);

                // ld hl, (to)
                addFix( forNext->for_to );
                addCmd(0x2A, 0x0000);

                if(has_step) {
                    // ld a, (step+1)
                    addFix( forNext->for_step )->step = 1;
                    addCmd(0x3A, 0x0000);

                    // bit 7, a
                    addWord(0xCB, 0x7F);
                    // jr z, $+2
                    addWord(0x28, 0x01);
                    //   ex de,hl
                    addByte(0xEB);
                }

                // ;var > to? goto end for

                // call intCompareGT
                addCmd(0xCD, def_intCompareGT);

                // jp nz, end_for
                forNext->for_end_mark = addMark();
                addCmd(0xC2, 0x0000);

                // body start

            } else {

                // jr $+26      ; jump to check code
                addWord(0x18, 0x19);

                // ;step code
                forNext->for_step_mark = addPreMark();
                forNext->for_step_mark->address = code_pointer;

                // ld a, (variable)
                addFix( forNext->for_var );
                addCmd(0x3A, 0x0000);
                // ld b, a
                addByte(0x47);
                // ld hl, (variable+1)
                addFix( forNext->for_var )->step = 1;
                addCmd(0x2A, 0x0000);

                // ld a, (step)
                addFix( forNext->for_step );
                addCmd(0x3A, 0x0000);
                // ld c, a
                addByte(0x4F);
                // ld de, (step)
                addByte(0xED);
                addFix( forNext->for_step )->step = 1;
                addCmd(0x5B, 0x0000);
                // call 0x76c1     ; add floats (b:hl + c:de = b:hl)
                addCmd(0xCD, def_XBASIC_ADD_FLOATS);

                // ld a, b
                addByte(0x78);
                // ld (variable), a
                addFix( forNext->for_var );
                addCmd(0x32, 0x0000);
                // ld (variable+1), hl
                addFix( forNext->for_var )->step = 1;
                addCmd(0x22, 0x0000);

                // ;check code

                // ld a, (to)
                addFix( forNext->for_to );
                addCmd(0x3A, 0x0000);
                // ld b, a
                addByte(0x47);
                // ld hl, (to+1)
                addFix( forNext->for_to )->step = 1;
                addCmd(0x2A, 0x0000);

                // ld a, (variable)
                addFix( forNext->for_var );
                addCmd(0x3A, 0x0000);
                // ld c, a
                addByte(0x4F);
                // ld de, (variable+1)
                addByte(0xED);
                addFix( forNext->for_var )->step = 1;
                addCmd(0x5B, 0x0000);

                // ld a, (step+2)
                addFix( forNext->for_step )->step = 2;
                addCmd(0x3A, 0x0000);

                // bit 7, a
                addWord(0xCB, 0x7F);
                // jr nz, $+5
                addWord(0x20, 0x04);
                //   ex de,hl
                addByte(0xEB);
                //   ld a, c
                addByte(0x79);
                //   ld c, b
                addByte(0x48);
                //   ld b, a
                addByte(0x47);

                // ;var > to? goto end for

                // 78a4 xbasic compare floats (<=)
                addCmd(0xCD, def_XBASIC_COMPARE_FLOATS_LE);

                // ld a, l
                addByte(0x7D);
                // or h
                addByte(0xB4);

                // jp z, end_for
                forNext->for_end_mark = addMark();
                addCmd(0xCA, 0x0000);

                // body start
            }


        } else {
            syntax_error("Incomplete FOR syntax");
        }

    }

}

void Compiler::cmd_next() {
    ForNextNode *forNext;

    if(forNextStack.size()) {

        forNext = forNextStack.top();
        forNextStack.pop();

        // jp step
        addFix(forNext->for_step_mark);
        addCmd(0xC3, 0x0000);

        if(forNext->for_end_mark)
            forNext->for_end_mark->symbol->address = code_pointer;

    } else {
        syntax_error("NEXT without a FOR");
    }

}

void Compiler::cmd_locate() {
    ActionNode *action;
    unsigned int t = current_action->actions.size();
    int result_subtype;

    if(t != 2) {
        syntax_error("LOCATE without enough parameters");
    } else {

        action = current_action->actions[0];
        result_subtype = evalExpression(action);

        addCast(result_subtype, Lexeme::subtype_numeric);

        // push hl
        addByte(0xE5);

        action = current_action->actions[1];
        result_subtype = evalExpression(action);

        addCast(result_subtype, Lexeme::subtype_numeric);

        // pop de
        addByteOptimized(0xD1);
        // call XBASIC_LOCATE    ; hl = y, de = x
        addCmd(0xCD, def_XBASIC_LOCATE);

    }

}

void Compiler::cmd_screen() {
    ActionNode *action;
    Lexeme *lexeme;
    unsigned int i, t = current_action->actions.size();
    int result_subtype;

    if(t) {

        action = current_action->actions[0];
        lexeme = action->lexeme;
        if(lexeme->value == "COPY") {
            current_action = action;
            cmd_screen_copy();
            return;
        } else if(lexeme->value == "PASTE") {
            current_action = action;
            cmd_screen_paste();
            return;
        } else if(lexeme->value == "SCROLL") {
            current_action = action;
            cmd_screen_scroll();
            return;
        } else if(lexeme->value == "LOAD") {
            current_action = action;
            cmd_screen_load();
            return;
        }

        for(i = 0; i < t; i++) {

            action = current_action->actions[i];
            result_subtype = evalExpression(action);

            if(result_subtype == Lexeme::subtype_null)
                continue;

            addCast(result_subtype, Lexeme::subtype_numeric);

            if(i != 5) {
                // ld a, l
                addByte(0x7D);
            }

            switch(i) {
                // display mode
                case 0: {

                        // cp 4
                        addWord(0xFE, 0x04);
                        // jr c, $+10                  ; skip if screen mode <= 3 (its safe for msx1)
                        addWord(0x38, 0x09);
                        //   ld a, (BIOS VERSION)
                        addCmd(0x3A, def_VERSION);
                        //   and a
                        addByte(0xA7);
                        //   jr nz, $+3                ; skip if not MSX1 (screen mode its safe for msx2 and above)
                        addWord(0x20, 0x02);
                        //     ld l, 2                 ; else, force screen mode 2
                        addWord(0x2E,0x02);
                        //   ld a, l
                        addByte(0x7D);

                        // call 0x7367    ; xbasic SCREEN mode (in: a = screen mode)
                        addCmd(0xCD, def_XBASIC_SCREEN);
                    }
                    break;

                // sprite size
                case 1: {
                        // call 0x70bc    ; xbasic SCREEN sprite (in: a = sprite mode)
                        addCmd(0xCD, def_XBASIC_SCREEN_SPRITE);
                    }
                    break;

                // key click
                case 2: {
                        // ld (CLIKSW), a   ; 0=keyboard click off, 1=keyboard click on
                        addCmd(0x32, def_CLIKSW);
                    }
                    break;

                // baud rate
                case 3: {
                        // ; original code: C1F63 on subrom of TurboR and A7A2D on main rom of the rest
                        // ld bc,5
                        addCmd(0x01, 0x0005);
                        // and a
                        addByte(0xA7);
                        // ld hl,CS1200
                        addCmd(0x21, def_CS1200);
                        // jr z,skip
                        addWord(0x28, 0x01);
                        //   add hl,bc
                        addByte(0x09);
                        // skip:
                        // ld de,LOW
                        addCmd(0x11, def_LOW);
                        // ldir
                        addWord(0xED, 0xB0);
                    }
                    break;

                // printer type
                case 4: {
                        // ld (NTMSXP), a   ; printer type (0=default)
                        addCmd(0x32, def_NTMSXP);
                    }
                    break;

                // interlace mode
                case 5: {

                        // ld a, (VERSION)
                        addCmd(0x3A, def_VERSION);
                        // and a
                        addByte(0xA7);
                        // jr z, skip1
                        addWord(0x28, 25);

                        //   ld a, l
                        addByte(0x7D);

                        //   ; original code: J1F45 on subrom of MSX2 and above
                        //   and 3     ; must be 0 to 3
                        addWord(0xE6, 0x03);
                        //   add a, a
                        addByte(0x87);
                        //   bit 1, a
                        addWord(0xCB, 0x4F);
                        //   jr z, skip2
                        addWord(0x28, 0x02);
                        //     set 3, a
                        addWord(0xCB, 0xDF);
                        //   skip2:
                        //   and 0x0C
                        addWord(0xE6, 0x0C);
                        //   ld b, a
                        addByte(0x47);
                        //   ld a, (RG9SAV)
                        addCmd(0x3A, def_RG9SAV);
                        //   and 0xF3
                        addWord(0xE6, 0xF3);
                        //   or b
                        addByte(0xB0);
                        //   ld c, 9
                        addWord(0x0E, 0x09);
                        //   ld b, a
                        addByte(0x47);
                        //   call WRTVDP
                        addCmd(0xCD, def_WRTVDP);

                        // skip1:
                    }
                    break;

                default: {
                        syntax_error("SCREEN parameters not supported");
                        return;
                    }
            }

        }

    } else {
        syntax_error("SCREEN with empty parameters");
    }

}

void Compiler::cmd_screen_copy() {
    Lexeme *lexeme;
    ActionNode *action;
    unsigned int i, t = current_action->actions.size();
    int result_subtype;

    if(t) {

        if(t > 2) {
            syntax_error("SCREEN COPY with excess of parameters");
            return;
        }

        for(i = 0; i < t; i++) {

            action = current_action->actions[i];
            lexeme = action->lexeme;

            if(i) {
                // push hl
                addByte(0xE5);

                result_subtype = evalExpression(action);
                addCast(result_subtype, Lexeme::subtype_numeric);

                // ld a, l                 ; copy parameter to A
                addByte(0x7D);

                // pop hl
                addByte(0xE1);
            } else {
                if(lexeme->type == Lexeme::type_identifier) {
                    // ld hl, variable
                    addFix( lexeme );
                    addCmd(0x21, 0x0000);
                    result_subtype = Lexeme::subtype_numeric;
                } else {
                    result_subtype = evalExpression(action);
                }
                addCast(result_subtype, Lexeme::subtype_numeric);
                if(t == 1) {
                    // xor a
                    addByte(0xAF);
                }
            }

        }

        // call screen_copy
        addCmd(0xCD, def_cmd_screen_copy);

    } else {
        syntax_error("SCREEN COPY with empty parameters");
    }

}

void Compiler::cmd_screen_paste() {
    Lexeme *lexeme;
    ActionNode *action;
    unsigned int i, t = current_action->actions.size();
    int result_subtype;

    if(t) {

        if(t > 1) {
            syntax_error("SCREEN PASTE with excess of parameters");
            return;
        }

        for(i = 0; i < t; i++) {

            action = current_action->actions[i];
            lexeme = action->lexeme;

                if(lexeme->type == Lexeme::type_identifier) {
                    // ld hl, variable
                    addFix( lexeme );
                    addCmd(0x21, 0x0000);
                    result_subtype = Lexeme::subtype_numeric;
                } else {
                    result_subtype = evalExpression(action);
                }
                addCast(result_subtype, Lexeme::subtype_numeric);

        }

        // call screen_paste
        addCmd(0xCD, def_cmd_screen_paste);

    } else {
        syntax_error("SCREEN PASTE with empty parameters");
    }

}

void Compiler::cmd_screen_scroll() {
    ActionNode *action;
    unsigned int i, t = current_action->actions.size();
    int result_subtype;

    if(t) {

        if(t > 1) {
            syntax_error("SCREEN SCROLL with excess of parameters");
            return;
        }

        for(i = 0; i < t; i++) {

            action = current_action->actions[i];
            result_subtype = evalExpression(action);
            addCast(result_subtype, Lexeme::subtype_numeric);

            // ld a, l                 ; copy parameter to A
            addByte(0x7D);

            // ld hl, (HEAPSTR)
            addCmd(0x2A, def_HEAPSTR);

            // push hl
            addByte(0xE5);

            // call screen_copy
            addCmd(0xCD, def_cmd_screen_copy);

            // pop hl
            addByte(0xE1);

            // call screen_paste
            addCmd(0xCD, def_cmd_screen_paste);

        }

    } else {
        syntax_error("SCREEN SCROLL with empty parameters");
    }

}

void Compiler::cmd_screen_load() {
    ActionNode *action;
    unsigned int i, t = current_action->actions.size();
    int result_subtype;

    if(t) {

        for(i = 0; i < t; i++) {

            action = current_action->actions[i];
            result_subtype = evalExpression(action);
            addCast(result_subtype, Lexeme::subtype_numeric);

            // call screen_load
            addCmd(0xCD, def_cmd_screen_load);

        }

    } else {
        syntax_error("SCREEN LOAD with empty parameters");
    }

}

void Compiler::cmd_width() {
    ActionNode *action;
    unsigned int t = current_action->actions.size();
    int result_subtype;

    if(t == 1) {

        action = current_action->actions[0];
        result_subtype = evalExpression(action);

        addCast(result_subtype, Lexeme::subtype_numeric);

        // ld a, (SCRMOD)          ; SCRMOD (current screen mode), OLDSCR (last text screen mode)
        addCmd(0x3A, def_SCRMOD);
        // cp 2
        addWord(0xFE, 0x02);
        // jr nc, $+FIM            ; skip if not text mode
        addWord(0x30, 0x22);       // 34 bytes

        // and a                   ; test if zero
        addByte(0xA7);
        // ld a, l                 ; copy parameter to A
        addByte(0x7D);
        // jr z, $+6               ; if zero then its 40 columns, goto LINL40
        addWord(0x28, 0x05);
        //   ld (0xF3AF), a        ; LINL32
        addCmd(0x32, 0xF3AF);
        //   jr $+4                ; goto LINLEN
        addWord(0x18, 0x03);
        //     ld (0xF3AE), a      ; LINL40
        addCmd(0x32, 0xF3AE);
        // ld (0xF3B0), a          ; LINLEN
        addCmd(0x32, 0xF3B0);
        // sub 0x0E
        addWord(0xD6, 0x0E);
        // add a, 0x1C
        addWord(0xC6, 0x1C);
        // cpl
        addByte(0x2F);
        // inc a
        addByte(0x3C);
        // add a, l
        addByte(0x85);
        // ld (0xF3B2), a          ; CLMLST
        addCmd(0x32, 0xF3B2);
        // ld a, 0x0C              ; new page (clear the screen)
        //addWord(0x3E, 0x0C);
        // rst 0x18                ; OUTDO - output to screen
        //addByte(0xDF);

        // ld a, (SCRMOD)          ; SCRMOD (current screen mode), OLDSCR (last text screen mode)
        addCmd(0x3A, def_SCRMOD);
        // call 0x7367    ; xbasic SCREEN mode (in: a = screen mode)
        addCmd(0xCD, def_XBASIC_SCREEN);
        // call cls
        addCmd( 0xcd, def_XBASIC_CLS);

    } else {
        syntax_error("WIDTH syntax error");
    }

}

void Compiler::cmd_color() {
    ActionNode *action, *subaction;
    Lexeme *lexeme;
    unsigned int i, t = current_action->actions.size();
    int result_subtype;

    if(t) {

        action = current_action->actions[0];
        lexeme = action->lexeme;

        if(lexeme->type == Lexeme::type_keyword) {

            if(lexeme->value == "NEW") {
                // ld ix, 0x0141
                addByte(0xDD);
                addCmd(0x21, 0x0141);
                // call EXTROM
                addCmd(0xCD, def_EXTROM);
                // ei
                addByte(0xFB);

            } else if(lexeme->value == "RESTORE") {
                // ld ix, 0x0145
                addByte(0xDD);
                addCmd(0x21, 0x0145);
                // call EXTROM
                addCmd(0xCD, def_EXTROM);
                // ei
                addByte(0xFB);

            } else if(lexeme->value == "SPRITE") {

                t = action->actions.size();

                if(t != 2) {
                    syntax_error("Invalid COLOR SPRITE parameters count");
                    return;
                }

                for(i = 0; i < t; i++) {

                    subaction = action->actions[i];
                    result_subtype = evalExpression(subaction);

                    if(result_subtype != Lexeme::subtype_null) {
                        addCast(result_subtype, Lexeme::subtype_numeric);

                        // ld a, l
                        addByte(0x7D);
                    }

                    if( i < 1) {
                        // push af
                        addByte(0xF5);
                    }
                }

                // pop bc
                addByte(0xC1);
                // call COLOR_SPRITE   ; in: b, a
                addCmd(0xCD, def_XBASIC_COLOR_SPRITE);

            } else if(lexeme->value == "SPRITE$") {

                t = action->actions.size();

                if(t != 2) {
                    syntax_error("Invalid COLOR SPRITE$ parameters count");
                    return;
                }

                for(i = 0; i < t; i++) {

                    subaction = action->actions[i];
                    result_subtype = evalExpression(subaction);

                    if(i < 1) {
                        addCast(result_subtype, Lexeme::subtype_numeric);

                        // ld a, l
                        addByte(0x7D);
                        // push af
                        addByte(0xF5);
                    }
                }

                // pop bc
                addByte(0xC1);
                // call COLOR_SPRSTR   ; in: b, hl
                addCmd(0xCD, def_XBASIC_COLOR_SPRSTR);

            } else if(lexeme->value == "RGB") {

                t = action->actions.size();

                if(t < 2 || t > 4) {
                    syntax_error("Invalid COLOR RGB parameters count");
                    return;
                }

                for(i = 0; i < t; i++) {
                    subaction = action->actions[i];
                    result_subtype = evalExpression(subaction);

                    if(result_subtype != Lexeme::subtype_null) {
                        addCast(result_subtype, Lexeme::subtype_numeric);

                        // ld a, l
                        addByte(0x7D);
                    }

                    if( i < 3) {
                        // push af
                        addByte(0xF5);
                    }
                }

                if(t < 4) {
                    // xor a
                    addByte(0xAF);
                }
                // ld b, a
                addByte(0x47);

                if(t < 3) {
                    // ld h, a
                    addByte(0x67);
                } else {
                    // pop hl
                    addByte(0xE1);
                }

                // pop de
                addByte(0xD1);
                // pop af
                addByte(0xF1);

                // call COLOR_RGB   ; in: a, d, h, b
                addCmd(0xCD, def_XBASIC_COLOR_RGB);

            } else {
                syntax_error("Invalid COLOR parameters");
            }


        } else {

            for(i = 0; i < t; i++) {

                action = current_action->actions[i];
                result_subtype = evalExpression(action);

                if(result_subtype == Lexeme::subtype_null)
                    continue;

                addCast(result_subtype, Lexeme::subtype_numeric);

                // ld a, l
                addByte(0x7D);

                switch(i) {
                    case 0: {
                            // ld (0xF3E9), a   ; FORCLR
                            addCmd(0x32, 0xF3E9);
                            // ld (ATRBYT), a   ; ATRBYT
                            addCmd(0x32, def_ATRBYT);
                        }
                        break;

                    case 1: {
                            // ld (0xF3EA), a   ; BAKCLR
                            addCmd(0x32, 0xF3EA);
                        }
                        break;

                    case 2: {
                            // ld (0xF3EB), a   ; BDRCLR
                            addCmd(0x32, 0xF3EB);
                        }
                        break;

                    default: {
                            syntax_error("COLOR parameters not supported");
                            return;
                        }
                }

            }

            // ld a, (SCRMOD)
            addCmd(0x3A, def_SCRMOD);

            // call 0x0062   ; CHGCLR
            addCmd(0xCD, 0x0062);

        }

    } else {
        syntax_error("COLOR with empty parameters");
    }

}

void Compiler::cmd_pset(bool forecolor) {
    ActionNode *action, *sub_action;
    unsigned int i, t = current_action->actions.size();
    int result_subtype;
    bool has_x_coord=false, has_y_coord=false, has_color=false, has_operator=false;

    if(t) {

        for(i = 0; i < t; i++) {

            action = current_action->actions[i];

            switch(i) {
                case 0: {
                        if(action->actions.size() != 2) {
                            if(forecolor)
                                syntax_error("Coordenates parameters error on PSET");
                            else
                                syntax_error("Coordenates parameters error on PRESET");
                            return;
                        } else if(action->lexeme->value == "COORD") {

                            sub_action = action->actions[0];

                            result_subtype = evalExpression(sub_action);

                            if(result_subtype == Lexeme::subtype_null) {
                            } else {
                                addCast(result_subtype, Lexeme::subtype_numeric);

                                // ld (0xFCB7), hl  ;GRPACX
                                addCmd(0x22, 0xFCB7);
                                // push hl
                                addByte(0xE5);

                                has_x_coord = true;
                            }

                            sub_action = action->actions[1];

                            result_subtype = evalExpression(sub_action);

                            if(result_subtype == Lexeme::subtype_null) {
                            } else {
                                addCast(result_subtype, Lexeme::subtype_numeric);

                                // ld (0xFCB9), hl  ;GRPACY
                                addCmd(0x22, 0xFCB9);

                                // push hl
                                addByte(0xE5);

                                has_y_coord = true;
                            }

                        } else if(action->lexeme->value == "STEP") {

                            sub_action = action->actions[0];

                            result_subtype = evalExpression(sub_action);

                            if(result_subtype == Lexeme::subtype_null) {
                            } else {
                                addCast(result_subtype, Lexeme::subtype_numeric);
                                // ld de, (0xFCB7)  ;GRPACX
                                addByte(0xED);
                                addCmd(0x5B, 0xFCB7);
                                // add hl, de
                                addByte(0x19);
                                // ld (0xFCB7), hl  ;GRPACX
                                addCmd(0x22, 0xFCB7);
                                // push hl
                                addByte(0xE5);

                                has_x_coord = true;
                            }

                            sub_action = action->actions[1];

                            result_subtype = evalExpression(sub_action);

                            if(result_subtype == Lexeme::subtype_null) {
                            } else {
                                addCast(result_subtype, Lexeme::subtype_numeric);
                                // ld de, (0xFCB9)  ;GRPACY
                                addByte(0xED);
                                addCmd(0x5B, 0xFCB9);
                                // add hl, de
                                addByte(0x19);
                                // ld (0xFCB9), hl  ;GRPACY
                                addCmd(0x22, 0xFCB9);
                                // push hl
                                addByte(0xE5);

                                has_y_coord = true;
                            }

                        } else {
                            if(forecolor)
                                syntax_error("Invalid coordenates on PSET");
                            else
                                syntax_error("Invalid coordenates on PRESET");
                            return;
                        }

                    }
                    break;

                case 1: {

                        result_subtype = evalExpression(action);

                        if(result_subtype == Lexeme::subtype_null)
                            continue;

                        addCast(result_subtype, Lexeme::subtype_numeric);

                        // ld a, l
                        addByte(0x7D);

                        // push af       ; save color
                        addByte(0xF5);

                        has_color = true;

                    }
                    break;

                case 2: {

                        result_subtype = evalExpression(action);

                        if(result_subtype == Lexeme::subtype_null)
                            continue;

                        addCast(result_subtype, Lexeme::subtype_numeric);

                        // ld a, l
                        addByte(0x7D);

                        // ld (LOGOPR), a          ; save new logical operator to basic interpreter
                        addCmd(0x32, def_LOGOPR);

                        has_operator = true;

                    }
                    break;


                default: {
                        if(forecolor)
                            syntax_error("PSET parameters not supported");
                        else
                            syntax_error("PRESET parameters not supported");
                        return;
                    }
            }

        }

        if(!has_operator) {
            // ld a, (LOGOPR)      ; get default operator from basic interpreter
            addCmd(0x3A, def_LOGOPR);
        }

        // ld b, a      ; get operator
        addByte(0x47);

        if(has_color) {
            // pop af       ; color
            addByte(0xF1);
        } else {
            if(forecolor) {
                // ld a, (0xF3E9)       ; FORCLR
                addCmd(0x3A, def_FORCLR);
            } else {
                // ld a, (0xF3EA)       ; BAKCLR
                addCmd(0x3A, def_BAKCLR);
            }
        }

        if(has_y_coord) {
            // pop hl
            addByte(0xE1);
        } else {
            // ld hl, (0xFCB9)  ;GRPACY
            addCmd(0x2A, 0xFCB9);
        }

        if(has_x_coord) {
            // pop de
            addByte(0xD1);
        } else {
            // ld de, (0xFCB7)  ;GRPACX
            addByte(0xED);
            addCmd(0x5B, 0xFCB7);
        }

        // call 0x6F71   ; xbasic PSET (in: hl=y, de=x, a=color, b=operator)
        addCmd(0xCD, def_XBASIC_PSET);

    } else {
        if(forecolor)
            syntax_error("PSET with empty parameters");
        else
            syntax_error("PRESET with empty parameters");
    }

}

void Compiler::cmd_paint() {
    ActionNode *action, *sub_action;
    unsigned int i, t = current_action->actions.size();
    int result_subtype;
    bool has_x_coord=false, has_y_coord=false, has_color=false, has_border=false;

    if(t) {

        for(i = 0; i < t; i++) {

            action = current_action->actions[i];

            switch(i) {
                case 0: {
                        if(action->actions.size() != 2) {
                            syntax_error("Coordenates parameters error on PAINT");
                            return;
                        } else if(action->lexeme->value == "COORD") {

                            sub_action = action->actions[0];

                            result_subtype = evalExpression(sub_action);

                            if(result_subtype == Lexeme::subtype_null) {
                            } else {
                                addCast(result_subtype, Lexeme::subtype_numeric);

                                // ld (0xFCB7), hl  ;GRPACX
                                addCmd(0x22, 0xFCB7);
                                // push hl
                                addByte(0xE5);

                                has_x_coord = true;
                            }

                            sub_action = action->actions[1];

                            result_subtype = evalExpression(sub_action);

                            if(result_subtype == Lexeme::subtype_null) {
                            } else {
                                addCast(result_subtype, Lexeme::subtype_numeric);

                                // ld (0xFCB9), hl  ;GRPACY
                                addCmd(0x22, 0xFCB9);

                                // push hl
                                addByte(0xE5);

                                has_y_coord = true;
                            }

                        } else if(action->lexeme->value == "STEP") {

                            sub_action = action->actions[0];

                            result_subtype = evalExpression(sub_action);

                            if(result_subtype == Lexeme::subtype_null) {
                            } else {
                                addCast(result_subtype, Lexeme::subtype_numeric);
                                // ld de, (0xFCB7)  ;GRPACX
                                addByte(0xED);
                                addCmd(0x5B, 0xFCB7);
                                // add hl, de
                                addByte(0x19);
                                // ld (0xFCB7), hl  ;GRPACX
                                addCmd(0x22, 0xFCB7);
                                // push hl
                                addByte(0xE5);

                                has_x_coord = true;
                            }

                            sub_action = action->actions[1];

                            result_subtype = evalExpression(sub_action);

                            if(result_subtype == Lexeme::subtype_null) {
                            } else {
                                addCast(result_subtype, Lexeme::subtype_numeric);
                                // ld de, (0xFCB9)  ;GRPACY
                                addByte(0xED);
                                addCmd(0x5B, 0xFCB9);
                                // add hl, de
                                addByte(0x19);
                                // ld (0xFCB9), hl  ;GRPACY
                                addCmd(0x22, 0xFCB9);
                                // push hl
                                addByte(0xE5);

                                has_y_coord = true;
                            }

                        } else {
                            syntax_error("Invalid coordenates on PAINT");
                            return;
                        }

                    }
                    break;

                case 1: {

                        result_subtype = evalExpression(action);

                        if(result_subtype == Lexeme::subtype_null)
                            continue;

                        addCast(result_subtype, Lexeme::subtype_numeric);

                        // ld b, l       ; paint color
                        addByte(0x45);

                        has_color = true;

                    }
                    break;

                case 2: {

                        if(has_color) {
                            // push bc    ; save paint color
                            addByte(0xC5);
                        }

                        result_subtype = evalExpression(action);

                        if(result_subtype == Lexeme::subtype_null)
                            continue;

                        addCast(result_subtype, Lexeme::subtype_numeric);

                        has_border = true;

                    }
                    break;


                default: {
                        syntax_error("PAINT parameters not supported");
                        return;
                    }
            }

        }

        if(has_border) {
            if(has_color) {
                // pop bc               ; restore paint color
                addByte(0xC1);
            } else {
                // ld a, (ATRBYT)
                addCmd(0x3A, def_ATRBYT);
                // ld b, a              ; paint color = default color
                addByte(0x47);
            }
            //   ld a, l                ; border color
            addByte(0x7D);

        } else {
            if(has_color) {
                // ld a, l              ; border color = paint color
                addByte(0x7D);
            } else {
                // ld a, (ATRBYT)       ; border color = default color
                addCmd(0x3A, def_ATRBYT);
                // ld b, a              ; paint color = default color
                addByte(0x47);
            }
        }

        if(has_y_coord) {
            // pop hl
            addByte(0xE1);
        } else {
            // ld hl, (0xFCB9)  ;GRPACY
            addCmd(0x2A, 0xFCB9);
        }

        if(has_x_coord) {
            // pop de
            addByte(0xD1);
        } else {
            // ld de, (0xFCB7)  ;GRPACX
            addByte(0xED);
            addCmd(0x5B, 0xFCB7);
        }

        // call 0x74B3   ; xbasic PAINT (in: hl=y, de=x, b=filling color, a=border color)
        addCmd(0xCD, def_XBASIC_PAINT);

    } else {
        syntax_error("PAINT with empty parameters");
    }

}

void Compiler::cmd_circle() {
    ActionNode *action, *sub_action;
    unsigned int i, t = current_action->actions.size();
    int result_subtype;
    bool has_x_coord=false, has_y_coord=false, has_radius=false, has_color=false;
    bool has_trace1=false, has_trace2=false, has_aspect=false;

    if(t) {

        for(i = 0; i < t; i++) {

            action = current_action->actions[i];

            switch(i) {
                // coord
                case 0: {
                        if(action->actions.size() != 2) {
                            syntax_error("Coordenates parameters error on CIRCLE");
                            return;
                        } else if(action->lexeme->value == "COORD") {

                            sub_action = action->actions[0];

                            result_subtype = evalExpression(sub_action);

                            if(result_subtype == Lexeme::subtype_null) {
                            } else {
                                addCast(result_subtype, Lexeme::subtype_numeric);

                                // ld (0xFCB7), hl  ;GRPACX
                                addCmd(0x22, 0xFCB7);

                                has_x_coord = true;
                            }

                            sub_action = action->actions[1];

                            result_subtype = evalExpression(sub_action);

                            if(result_subtype == Lexeme::subtype_null) {
                            } else {
                                addCast(result_subtype, Lexeme::subtype_numeric);

                                // ld (0xFCB9), hl  ;GRPACY
                                addCmd(0x22, 0xFCB9);

                                has_y_coord = true;
                            }

                        } else if(action->lexeme->value == "STEP") {

                            sub_action = action->actions[0];

                            result_subtype = evalExpression(sub_action);

                            if(result_subtype == Lexeme::subtype_null) {
                            } else {
                                addCast(result_subtype, Lexeme::subtype_numeric);
                                // ld de, (0xFCB7)  ;GRPACX
                                addByte(0xED);
                                addCmd(0x5B, 0xFCB7);
                                // add hl, de
                                addByte(0x19);
                                // ld (0xFCB7), hl  ;GRPACX
                                addCmd(0x22, 0xFCB7);

                                has_x_coord = true;
                            }

                            sub_action = action->actions[1];

                            result_subtype = evalExpression(sub_action);

                            if(result_subtype == Lexeme::subtype_null) {
                            } else {
                                addCast(result_subtype, Lexeme::subtype_numeric);
                                // ld de, (0xFCB9)  ;GRPACY
                                addByte(0xED);
                                addCmd(0x5B, 0xFCB9);
                                // add hl, de
                                addByte(0x19);
                                // ld (0xFCB9), hl  ;GRPACY
                                addCmd(0x22, 0xFCB9);

                                has_y_coord = true;
                            }

                        } else {
                            syntax_error("Invalid coordenates on CIRCLE");
                            return;
                        }

                    }
                    break;

                // radius
                case 1: {

                        result_subtype = evalExpression(action);

                        if(result_subtype == Lexeme::subtype_null)
                            continue;

                        addCast(result_subtype, Lexeme::subtype_numeric);

                        has_radius = true;

                    }
                    break;

                // color
                case 2: {

                        if(has_radius) {
                            // push hl
                            addByte(0xE5);
                        }

                        result_subtype = evalExpression(action);

                        if(result_subtype == Lexeme::subtype_null) {
                            if(has_radius) {
                                code_pointer --;
                                code_size --;
                            }
                            continue;
                        }

                        addCast(result_subtype, Lexeme::subtype_numeric);

                        // ld a, l
                        addByte(0x7D);

                        has_color = true;

                    }
                    break;

                // tracing start
                case 3: {

                        if(has_radius) {
                            if(has_color) {
                                // pop hl
                                addByte(0xE1);
                            }
                            // ld c, l
                            addByte(0x4D);
                        }

                        // ld hl, BUF
                        addCmd(0x21, def_BUF);
                        // push hl
                        addByte(0xE5);
                        //   ld (hl), 0x2C  ; comma
                        //addWord(0x36, 0x2C);
                        //   inc hl
                        //addByte(0x23);

                        if(has_radius) {
                            //   ld (hl), 0x0F  ; short interger marker
                            addWord(0x36, 0x0F);
                            //   inc hl
                            addByte(0x23);
                            //   ld (hl), c     ; radius
                            addByte(0x71);
                            //   inc hl
                            addByte(0x23);
                        }

                        //   ld (hl), 0x2C  ; comma
                        addWord(0x36, 0x2C);
                        //   inc hl
                        addByte(0x23);

                        if(has_color) {
                            //   ld (hl), 0x0F  ; short interger marker
                            addWord(0x36, 0x0F);
                            //   inc hl
                            addByte(0x23);
                            //   ld (hl), a     ; color
                            addByte(0x77);
                            //   inc hl
                            addByte(0x23);
                        }

                        //   ld (hl), 0x2C  ; comma
                        addWord(0x36, 0x2C);
                        //   inc hl
                        addByte(0x23);

                        // push hl
                        addByte(0xE5);

                        result_subtype = evalExpression(action);

                        if(result_subtype == Lexeme::subtype_null) {
                            code_pointer --;
                            code_size --;
                            continue;
                        }

                        addCast(result_subtype, Lexeme::subtype_single_decimal);

                        // pop de
                        addByteOptimized(0xD1);

                        // call WriteParamBCD             ; b:hl, de -> hl
                        addCmd(0xCD, def_WriteParamBCD);

                        has_trace1 = true;

                    }
                    break;

                // tracing end
                case 4: {
                        //   ld (hl), 0x2C  ; comma
                        addWord(0x36, 0x2C);
                        //   inc hl
                        addByte(0x23);

                        // push hl
                        addByte(0xE5);

                        result_subtype = evalExpression(action);

                        if(result_subtype == Lexeme::subtype_null) {
                            code_pointer --;
                            code_size --;
                            continue;
                        }

                        addCast(result_subtype, Lexeme::subtype_single_decimal);

                        // pop de
                        addByteOptimized(0xD1);

                        // call WriteParamBCD             ; b:hl, de -> hl
                        addCmd(0xCD, def_WriteParamBCD);

                        has_trace2 = true;
                    }
                    break;

                // aspect ratio
                case 5: {
                        //   ld (hl), 0x2C  ; comma
                        addWord(0x36, 0x2C);
                        //   inc hl
                        addByte(0x23);

                        // push hl
                        addByte(0xE5);

                        result_subtype = evalExpression(action);

                        if(result_subtype == Lexeme::subtype_null) {
                            code_pointer -= 3;
                            code_size -= 3;
                            continue;
                        }

                        addCast(result_subtype, Lexeme::subtype_single_decimal);

                        // pop de
                        addByteOptimized(0xD1);

                        // call WriteParamBCD             ; b:hl, de -> hl
                        addCmd(0xCD, def_WriteParamBCD);

                        has_aspect = true;
                    }
                    break;

                default: {
                        syntax_error("Invalid CIRCLE parameters");
                        return;
                    }
            }

        }

        if(has_trace1 || has_trace2 || has_aspect) {

            //   ld (hl), 0x00
            addWord(0x36, 0x00);
            //   inc hl
            addByte(0x23);

            // pop hl
            addByte(0xE1);

            // ld a, (hl)         ; first character
            addByte(0x7E);
            // ld ix, M5B16       ; rom basic circle without coords
            addByte(0xDD);
            addCmd(0x21, 0x5B16);
            // call xbasic CIRCLE2 (in: hl = basic line starting on radius parameter)
            addCmd(0xCD, def_XBASIC_CIRCLE2);

        } else {
            if(has_color && has_radius) {
                // pop hl
                addByte(0xE1);
            } else if(!has_color) {
                // ld a, (0xF3E9)       ; FORCLR
                addCmd(0x3A, 0xF3E9);
            }

            if(!has_radius) {
                // ld hl, 0
                addCmd(0x21, 0x0000);
            }

            if(!has_y_coord) {
            }

            if(!has_x_coord) {
            }

            // call xbasic CIRCLE (in: GRPACX/GRPACY, hl=radius, a=color)
            addCmd(0xCD, def_XBASIC_CIRCLE);
        }

    } else {
        syntax_error("CIRCLE with empty parameters");
    }

}

void Compiler::cmd_put() {
    ActionNode *action;
    Lexeme *lexeme;
    unsigned int t = current_action->actions.size();

    if(t) {

        action = current_action->actions[0];
        lexeme = action->lexeme;

        if(lexeme->type == Lexeme::type_keyword && lexeme->value == "SPRITE") {
            current_action = action;
            cmd_put_sprite();
        } else if(lexeme->type == Lexeme::type_keyword && lexeme->value == "TILE") {
            current_action = action;
            cmd_put_tile();
        } else {
            syntax_error("Invalid PUT statement");
        }

    } else {
        syntax_error("Empty PUT statement");
    }

}

void Compiler::cmd_put_sprite() {
    ActionNode *action, *sub_action;
    unsigned int i, t = current_action->actions.size();
    int result_subtype, parm_flag = 0;
    bool has_sprite=false, has_x_coord=false, has_y_coord=false, has_color=false, has_pattern=false;

    if(t) {

        for(i = 0; i < t; i++) {

            action = current_action->actions[i];

            switch(i) {
                case 0: {

                        result_subtype = evalExpression(action);

                        if(result_subtype == Lexeme::subtype_null)
                            continue;

                        addCast(result_subtype, Lexeme::subtype_numeric);

                        // ld a, l
                        addByte(0x7D);
                        // push af
                        addByte(0xF5);

                        has_sprite = true;

                    }
                    break;

                case 1: {
                        if(action->actions.size() != 2) {
                            syntax_error("Coordenates parameters error on PUT SPRITE");
                            return;
                        } else if(action->lexeme->value == "COORD") {

                            sub_action = action->actions[0];

                            result_subtype = evalExpression(sub_action);

                            if(result_subtype == Lexeme::subtype_null) {
                            } else {
                                addCast(result_subtype, Lexeme::subtype_numeric);

                                // ld (0xFCB7), hl  ;GRPACX
                                addCmd(0x22, 0xFCB7);

                                // push hl
                                addByte(0xE5);

                                has_x_coord = true;
                            }

                            sub_action = action->actions[1];

                            result_subtype = evalExpression(sub_action);

                            if(result_subtype == Lexeme::subtype_null) {
                            } else {
                                addCast(result_subtype, Lexeme::subtype_numeric);

                                // ld (0xFCB9), hl  ;GRPACY
                                addCmd(0x22, 0xFCB9);

                                // push hl
                                addByte(0xE5);

                                has_y_coord = true;
                            }

                        } else if(action->lexeme->value == "STEP") {

                            sub_action = action->actions[0];

                            result_subtype = evalExpression(sub_action);

                            if(result_subtype == Lexeme::subtype_null) {
                            } else {
                                addCast(result_subtype, Lexeme::subtype_numeric);
                                // ld de, (0xFCB7)  ;GRPACX
                                addByte(0xED);
                                addCmd(0x5B, 0xFCB7);
                                // add hl, de
                                addByte(0x19);
                                // ld (0xFCB7), hl  ;GRPACX
                                addCmd(0x22, 0xFCB7);

                                // push hl
                                addByte(0xE5);

                                has_x_coord = true;
                            }

                            sub_action = action->actions[1];

                            result_subtype = evalExpression(sub_action);

                            if(result_subtype == Lexeme::subtype_null) {
                            } else {
                                addCast(result_subtype, Lexeme::subtype_numeric);
                                // ld de, (0xFCB9)  ;GRPACY
                                addByte(0xED);
                                addCmd(0x5B, 0xFCB9);
                                // add hl, de
                                addByte(0x19);
                                // ld (0xFCB9), hl  ;GRPACY
                                addCmd(0x22, 0xFCB9);

                                // push hl
                                addByte(0xE5);

                                has_y_coord = true;
                            }

                        } else {
                            syntax_error("Invalid coordenates on PUT SPRITE");
                            return;
                        }

                    }
                    break;

                case 2: {

                        result_subtype = evalExpression(action);

                        if(result_subtype == Lexeme::subtype_null)
                            continue;

                        addCast(result_subtype, Lexeme::subtype_numeric);

                        // ld a, l
                        addByte(0x7D);
                        // push af
                        addByte(0xF5);

                        has_color = true;

                    }
                    break;

                case 3: {

                        result_subtype = evalExpression(action);

                        if(result_subtype == Lexeme::subtype_null)
                            continue;

                        addCast(result_subtype, Lexeme::subtype_numeric);

                        // ld a, l
                        addByte(0x7D);
                        // push af
                        addByte(0xF5);

                        has_pattern = true;

                    }
                    break;


                default: {
                        syntax_error("PUT SPRITE parameters not supported");
                        return;
                    }
            }

        }

        if(has_pattern) {
            // pop hl
            addByte(0xE1);
            parm_flag |= 0x20;
        } else {
            // ld h, 0
            //addWord(0x26, 0x00);
        }

        if(has_color) {
            // pop de
            addByte(0xD1);
            parm_flag |= 0x40;
        } else {
            // ld d, 0
            //addWord(0x16, 0x00);
        }

        if(has_y_coord) {
            // pop iy
            addWord(0xFD, 0xE1);
            parm_flag |= 0x80;
        } else {
            // ld iy, 0x0000
            //addByte(0xFD);
            //addCmd(0x21, 0x0000);
        }

        if(has_x_coord) {
            // pop ix
            addWord(0xDD, 0xE1);
            parm_flag |= 0x80;
        } else {
            // ld ix, 0x0000
            //addByte(0xDD);
            //addCmd(0x21, 0x0000);
        }

        if(has_sprite) {
            // pop af
            addByte(0xF1);
        } else {
            // xor a
            addByte(0xAF);
        }

        // ld b, parameters flag
        addWord(0x06, parm_flag);

        // call xbasic PUT SPRITE (in: ix=x, iy=y, d=color, a=sprite number, h=pattern number, b=parameters flag (b11100000)
        addCmd(0xCD, def_XBASIC_PUT_SPRITE);

    } else {
        syntax_error("PUT SPRITE with empty parameters");
    }

}

void Compiler::cmd_put_tile() {
    ActionNode *action, *sub_action;
    unsigned int i, t = current_action->actions.size();
    int result_subtype;
    bool has_tile=false, has_x_coord=false, has_y_coord=false;

    if(t) {

        for(i = 0; i < t; i++) {

            action = current_action->actions[i];

            switch(i) {
                case 0: {

                        result_subtype = evalExpression(action);

                        if(result_subtype == Lexeme::subtype_null)
                            continue;

                        addCast(result_subtype, Lexeme::subtype_numeric);

                        // ld a, l
                        addByte(0x7D);
                        // push af
                        addByte(0xF5);

                        has_tile = true;

                    }
                    break;

                case 1: {
                        if(action->actions.size() != 2) {
                            syntax_error("Coordenates parameters error on PUT TILE");
                            return;
                        } else if(action->lexeme->value == "COORD") {

                            sub_action = action->actions[0];

                            result_subtype = evalExpression(sub_action);

                            if(result_subtype == Lexeme::subtype_null) {
                            } else {
                                addCast(result_subtype, Lexeme::subtype_numeric);

                                // ld (0xFCB7), hl  ;GRPACX
                                addCmd(0x22, 0xFCB7);

                                // push hl
                                addByte(0xE5);

                                has_x_coord = true;
                            }

                            sub_action = action->actions[1];

                            result_subtype = evalExpression(sub_action);

                            if(result_subtype == Lexeme::subtype_null) {
                            } else {
                                addCast(result_subtype, Lexeme::subtype_numeric);

                                // ld (0xFCB9), hl  ;GRPACY
                                addCmd(0x22, 0xFCB9);

                                // push hl
                                addByte(0xE5);

                                has_y_coord = true;
                            }

                        } else if(action->lexeme->value == "STEP") {

                            sub_action = action->actions[0];

                            result_subtype = evalExpression(sub_action);

                            if(result_subtype == Lexeme::subtype_null) {
                            } else {
                                addCast(result_subtype, Lexeme::subtype_numeric);
                                // ld de, (0xFCB7)  ;GRPACX
                                addByte(0xED);
                                addCmd(0x5B, 0xFCB7);
                                // add hl, de
                                addByte(0x19);
                                // ld (0xFCB7), hl  ;GRPACX
                                addCmd(0x22, 0xFCB7);

                                // push hl
                                addByte(0xE5);

                                has_x_coord = true;
                            }

                            sub_action = action->actions[1];

                            result_subtype = evalExpression(sub_action);

                            if(result_subtype == Lexeme::subtype_null) {
                            } else {
                                addCast(result_subtype, Lexeme::subtype_numeric);
                                // ld de, (0xFCB9)  ;GRPACY
                                addByte(0xED);
                                addCmd(0x5B, 0xFCB9);
                                // add hl, de
                                addByte(0x19);
                                // ld (0xFCB9), hl  ;GRPACY
                                addCmd(0x22, 0xFCB9);

                                // push hl
                                addByte(0xE5);

                                has_y_coord = true;
                            }

                        } else {
                            syntax_error("Invalid coordenates on PUT TILE");
                            return;
                        }

                    }
                    break;

                default: {
                        syntax_error("PUT TILE parameters not supported");
                        return;
                    }
            }

        }

        if(has_y_coord) {
            // pop hl
            addByte(0xE1);
            // inc l        ; y coord (1 based for bios)
            addByte(0x2C);
        } else {
            // ld a, (CSRY)
            addCmd(0x3A, def_CSRY);
            // ld l, a
            addByte(0x6F);
        }

        if(has_x_coord) {
            // pop bc
            addByte(0xC1);
            // ld h, c
            addByte(0x61);
            // inc h        ; x coord (1 based for bios)
            addByte(0x24);
        } else {
            // ld a, (CSRX)
            addCmd(0x3A, def_CSRX);
            // ld h, a
            addByte(0x67);
        }

        //   call TileAddress   ; in hl=xy, out: hl
        addCmd(0xCD, def_tileAddress);

        if(has_tile) {
            // pop af
            addByte(0xF1);
        } else {
            // xor a
            addByte(0xAF);
        }

        // call 0x70b5                  ; xbasic VPOKE (in: hl=address, a=byte)
        addCmd(0xCD, def_XBASIC_VPOKE);

    } else {
        syntax_error("PUT TILE with empty parameters");
    }

}

void Compiler::cmd_set() {
    ActionNode *action;
    Lexeme *next_lexeme;
    unsigned int t = current_action->actions.size();
    FixNode* mark;

    if(t == 1) {

        action = current_action->actions[0];
        next_lexeme = action->lexeme;

        if(next_lexeme->type == Lexeme::type_keyword) {
            if(next_lexeme->value == "TILE") {
                cmd_set_tile();
                return;
            } else if(next_lexeme->value == "SPRITE") {
                cmd_set_sprite();
                return;
            }
        }

        // ld a, (BIOS VERSION)
        addCmd(0x3A, def_VERSION);
        // and a
        addByte(0xA7);
        // jp z, $                ; skip if MSX1
        mark = addMark();
        addCmd(0xCA, 0x0000);

        if (next_lexeme->type == Lexeme::type_keyword && next_lexeme->value == "ADJUST") {
            syntax_error("Not supported yet");
        } else if(next_lexeme->type == Lexeme::type_keyword && next_lexeme->value == "BEEP") {
            syntax_error("Not supported yet");
        } else if(next_lexeme->type == Lexeme::type_keyword && next_lexeme->value == "DATE") {
            syntax_error("Not supported yet");
        } else if(next_lexeme->type == Lexeme::type_keyword && next_lexeme->value == "PAGE") {
            cmd_set_page();
        } else if(next_lexeme->type == Lexeme::type_keyword && next_lexeme->value == "PASSWORD") {
            syntax_error("Not supported yet");
        } else if(next_lexeme->type == Lexeme::type_keyword && next_lexeme->value == "PROMPT") {
            syntax_error("Not supported yet");
        } else if(next_lexeme->type == Lexeme::type_keyword && next_lexeme->value == "SCREEN") {
            syntax_error("Not supported yet");
        } else if(next_lexeme->type == Lexeme::type_keyword && next_lexeme->value == "SCROLL") {
            cmd_set_scroll();
        } else if(next_lexeme->type == Lexeme::type_keyword && next_lexeme->value == "TIME") {
            syntax_error("Not supported yet");
        } else if(next_lexeme->type == Lexeme::type_keyword && next_lexeme->value == "TITLE") {
            syntax_error("Not supported yet");
        } else if(next_lexeme->type == Lexeme::type_keyword && next_lexeme->value == "VIDEO") {
            cmd_set_video();
        } else {
            syntax_error("Invalid SET statement");
        }

        mark->symbol->address = code_pointer;

    } else {
        syntax_error("Wrong SET parameters count");
    }

}

void Compiler::cmd_set_video() {
    ActionNode *action = current_action->actions[0], *sub_action;
    unsigned int i, t = action->actions.size();
    int result_subtype;

    if(t) {

        if(t > 7) {
            syntax_error("Invalid SET VIDEO parameters");
            return;
        }

        // ld hl, BUF
        addCmd(0x21, def_BUF);
        // push hl
        addByte(0xE5);
        //   ld (hl), 0x56  ; VIDEO tokens
        addWord(0x36, 0x56);
        //   inc hl
        addByte(0x23);
        //   ld (hl), 0x49
        addWord(0x36, 0x49);
        //   inc hl
        addByte(0x23);
        //   ld (hl), 0x44
        addWord(0x36, 0x44);
        //   inc hl
        addByte(0x23);
        //   ld (hl), 0x45
        addWord(0x36, 0x45);
        //   inc hl
        addByte(0x23);
        //   ld (hl), 0x4F
        addWord(0x36, 0x4F);
        //   inc hl
        addByte(0x23);

        for(i = 0; i < t; i++) {

            if( i ) {
                // ld (hl), 0x2C   ; comma
                addWord(0x36, 0x2C);
                // inc hl
                addByte(0x23);
            }

            // push hl
            addByte(0xE5);

            sub_action = action->actions[i];
            result_subtype = evalExpression(sub_action);

            if(result_subtype == Lexeme::subtype_null) {
                code_pointer --;
                code_size --;
                continue;
            }

            addCast(result_subtype, Lexeme::subtype_numeric);

            //   ld a, l
            addByte(0x7D);
            //   and 3
            addWord(0xE6, 0x03);
            //   inc a
            addByte(0x3C);
            //   or 0x10
            addWord(0xF6, 0x10);

            // pop hl
            addByte(0xE1);

            // ld (hl), a
            addByte(0x77);
            // inc hl
            addByte(0x23);

        }

        //   xor a
        addByte(0xAF);
        //   ld (hl), a
        addByte(0x77);
        //   inc hl
        addByte(0x23);
        //   ld (hl), a
        addByte(0x77);

        // pop hl
        addByte(0xE1);

        // ld a, (VERSION)
        addCmd(0x3A, def_VERSION);
        // and a
        addByte(0xA7);
        // jr z, skip
        addWord(0x28, 9);

        //   ld a, (hl)      ; first character
        addByte(0x7E);
        //   ld ix, (SET)    ; SET
        addWord(0xDD, 0x2A);
        addWord(def_SET_STMT);
        //   call CALBAS
        addCmd( 0xcd, def_CALBAS );
        //   ei
        addByte(0xFB);

        // skip:

    } else {
        syntax_error("SET VIDEO with empty parameters");
    }

}

void Compiler::cmd_set_page() {
    ActionNode *action = current_action->actions[0], *sub_action;
    Lexeme *lexeme;
    unsigned int t = action->actions.size();
    int result_subtype;

    if(t >= 1 && t <= 2) {

        // ld hl, parameter value
        sub_action = action->actions[0];
        lexeme = sub_action->lexeme;

        if(! (lexeme->type == Lexeme::type_literal && lexeme->subtype == Lexeme::subtype_null) ) {

            result_subtype = evalExpression(sub_action);
            addCast(result_subtype, Lexeme::subtype_numeric);

            // ld a,l
            addByte(0x7D);

            // call SET_PAGE       ; in: a = display page
            addCmd(0xCD, def_XBASIC_SET_PAGE);

        }

        if(t == 2) {
            sub_action = action->actions[1];
            result_subtype = evalExpression(sub_action);
            addCast(result_subtype, Lexeme::subtype_numeric);

            //   ld a,l
            addByte(0x7D);
            //   ld (ACPAGE), a    ; in: a = active page (write and read)
            addCmd(0x32, def_ACPAGE);
        }

    } else {
        syntax_error("Wrong parameters count on SET PAGE statement");
    }

}

void Compiler::cmd_set_scroll() {
    ActionNode *action = current_action->actions[0], *sub_action;
    Lexeme *lexeme;
    unsigned int i, t = action->actions.size();
    int result_subtype;

    if(t > 0 && t <= 4) {

        for(i = 0; i < t; i++) {

            sub_action = action->actions[i];
            lexeme = sub_action->lexeme;

            if(lexeme->type == Lexeme::type_literal && lexeme->subtype == Lexeme::subtype_null) {
                // ld hl, 0xffff
                addCmd(0x21, 0xFFFF);
            } else {
                // ld hl, parameter value
                result_subtype = evalExpression(sub_action);
                addCast(result_subtype, Lexeme::subtype_numeric);
            }

            if( i > 1 ) {
                // ld h, l
                addByte(0x65);
            }

            // push hl
            addByte(0xE5);
        }

        if(t == 4) {
            // pop af
            addByte(0xF1);
        } else {
            // ld a, 0xff
            addWord(0x3E, 0xFF);
        }

        if(t >= 3) {
            // pop bc
            addByte(0xC1);
        } else {
            // ld b, 0xff
            addWord(0x06, 0xFF);
        }

        if(t >= 2) {
            // pop hl
            addByte(0xE1);
        } else {
            // ld hl, 0xffff
            addCmd(0x21, 0xFFFF);
        }

        // pop de
        addByte(0xD1);

        // call SET_SCROLL      ; in: de=x, hl=y, b=mask mode, a=page mode
        addCmd(0xCD, def_XBASIC_SET_SCROLL);

    } else {
        syntax_error("Wrong parameters count on SET SCROLL statement");
    }

}

void Compiler::cmd_set_tile() {
    ActionNode *action = current_action->actions[0], *sub_action, *sub_sub_action;
    Lexeme *lexeme;
    unsigned int i, t, tt;
    int result_subtype;

    t = action->actions.size();
    if(t) {

        action = action->actions[0];
        lexeme = action->lexeme;
        t = action->actions.size();

        if(lexeme->value == "ON") {

            // ld a, 2                   ; tiled mode
            addWord(0x3E, 2);
            // ld (SOMODE), a
            addCmd(0x32, def_SOMODE);
            // clear screen, set font to default and put cursor on home
            cmd_cls();
            // ld hl, 0
            addCmd(0x21, 0x0000);
            // ld (DAC), hl
            addCmd(0x22, def_DAC);
            // ld d, h
            addByte(0x54);
            // ld e, l
            addByte(0x5D);
            // call XBASIC_LOCATE    ; hl = y, de = x
            addCmd(0xCD, def_XBASIC_LOCATE);
            // ld a, 0xff                ; it means all screen banks
            addWord(0x3E, 0xFF);
            // ld (ARG), a
            addCmd(0x32, def_ARG);
            // call cmd_setfnt
            addCmd(0xCD, def_cmd_setfnt);

        } else if(lexeme->value == "OFF") {

            // ld a, 1      ; graphical mode
            addWord(0x3E, 1);
            // ld (SOMODE), a
            addCmd(0x32, def_SOMODE);

        } else if(lexeme->value == "PATTERN") {

            if(t >= 2 && t <= 3) {

                // tile number
                sub_action = action->actions[0];
                // ld hl, parameter value    ; tile number
                result_subtype = evalExpression(sub_action);
                addCast(result_subtype, Lexeme::subtype_numeric);
                // ld (ARG), hl
                addCmd(0x22, def_ARG);

                // bank number
                if(t == 3) {
                    sub_action = action->actions[2];
                    // ld hl, parameter value    ; tile number
                    result_subtype = evalExpression(sub_action);
                    addCast(result_subtype, Lexeme::subtype_numeric);
                    // ld h, l
                    addByte(0x65);
                } else {
                    // ld h, 0x03
                    addWord(0x26, 0x03);
                }
                // ld (ARG2), hl
                addCmd(0x22, def_ARG2);

                // pattern data
                sub_action = action->actions[1];
                lexeme = sub_action->lexeme;

                if(lexeme->value == "ARRAY") {

                    tt = sub_action->actions.size();

                    for(i = 0; i < tt; i++) {

                        sub_sub_action = sub_action->actions[i];
                        lexeme = sub_sub_action->lexeme;

                        if(lexeme->type == Lexeme::type_literal && lexeme->subtype == Lexeme::subtype_null) {
                            continue;

                        } else {

                            // ld hl, parameter value    ; pattern data parameter
                            result_subtype = evalExpression(sub_sub_action);
                            addCast(result_subtype, Lexeme::subtype_numeric);

                            // ld bc, (ARG2)
                            addByte(0xED);
                            addCmd(0x4B, def_ARG2);
                            // ld c, l
                            addByte(0x4D);
                            // ld de, *i*
                            addCmd(0x11, i);
                            // ld hl, (ARG)
                            addCmd(0x2A, def_ARG);

                            // call set_tile_pattern ; hl = tile number, de = line number, b = bank number (3=all), c = pattern data
                            addCmd(0xCD, def_set_tile_pattern);

                        }

                    }


                } else {
                    syntax_error("Wrong pattern parameter on SET TILE PATTERN statement");
                }

            } else {
                syntax_error("Wrong parameters count on SET TILE PATTERN statement");
            }

        } else if(lexeme->value == "COLOR") {

            if(t >= 2 && t <= 4) {

                // tile number
                sub_action = action->actions[0];
                // ld hl, parameter value    ; tile number
                result_subtype = evalExpression(sub_action);
                addCast(result_subtype, Lexeme::subtype_numeric);
                // ld (ARG), hl
                addCmd(0x22, def_ARG);

                // bank number
                if(t == 4) {
                    sub_action = action->actions[3];
                    // ld hl, parameter value    ; tile number
                    result_subtype = evalExpression(sub_action);
                    addCast(result_subtype, Lexeme::subtype_numeric);
                    // ld h, l
                    addByte(0x65);
                } else {
                    // ld h, 0x03
                    addWord(0x26, 0x03);
                }
                // ld (ARG2), hl
                addCmd(0x22, def_ARG2);

                // color data
                sub_action = action->actions[1];
                lexeme = sub_action->lexeme;

                if(lexeme->value == "ARRAY") {

                    tt = sub_action->actions.size();

                    for(i = 0; i < tt; i++) {

                        sub_sub_action = sub_action->actions[i];
                        lexeme = sub_sub_action->lexeme;

                        if(lexeme->type == Lexeme::type_literal && lexeme->subtype == Lexeme::subtype_null) {
                            continue;

                        } else {

                            // ld hl, parameter value    ; color FC data parameter
                            result_subtype = evalExpression(sub_sub_action);
                            addCast(result_subtype, Lexeme::subtype_numeric);
                            // ld a, l
                            addByte(0x7D);
                            // rla
                            addByte(0x17);
                            // rla
                            addByte(0x17);
                            // rla
                            addByte(0x17);
                            // rla
                            addByte(0x17);
                            // and 0xF0
                            addWord(0xE6, 0xF0);

                            if(t >= 3) {
                                // color data
                                sub_sub_action = action->actions[2];
                                lexeme = sub_sub_action->lexeme;
                                if(lexeme->value != "ARRAY") {
                                    syntax_error("Syntax not supported on SET TILE COLOR statement");
                                    return;
                                }
                                if(i < sub_sub_action->actions.size()) {
                                    sub_sub_action = sub_sub_action->actions[i];
                                    lexeme = sub_sub_action->lexeme;
                                    if(!(lexeme->type == Lexeme::type_literal && lexeme->subtype == Lexeme::subtype_null)) {
                                        // push af
                                        addByte(0xF5);
                                        // ld hl, parameter value    ; color BC data parameter
                                        result_subtype = evalExpression(sub_sub_action);
                                        addCast(result_subtype, Lexeme::subtype_numeric);
                                        // pop af
                                        addByte(0xF1);
                                        // or l
                                        addByte(0xB5);
                                    }
                                }
                            }

                            // ld bc, (ARG2)
                            addByte(0xED);
                            addCmd(0x4B, def_ARG2);
                            // ld c, a
                            addByte(0x4F);
                            // ld de, *i*
                            addCmd(0x11, i);
                            // ld hl, (ARG)        ; tile number
                            addCmd(0x2A, def_ARG);

                            // call set_tile_color ; hl = tile number, de = line number (15=all), b = bank number (3=all), c = color data (FC,BC)
                            addCmd(0xCD, def_set_tile_color);

                        }

                    }

                } else {

                    // ld hl, parameter value    ; color FC data parameter
                    result_subtype = evalExpression(sub_action);
                    addCast(result_subtype, Lexeme::subtype_numeric);
                    // ld a, l
                    addByte(0x7D);
                    // rla
                    addByte(0x17);
                    // rla
                    addByte(0x17);
                    // rla
                    addByte(0x17);
                    // rla
                    addByte(0x17);
                    // and 0xF0
                    addWord(0xE6, 0xF0);

                    if(t >= 3) {
                        // color data
                        sub_sub_action = action->actions[2];
                        lexeme = sub_sub_action->lexeme;
                        if(lexeme->value == "ARRAY") {
                            syntax_error("Syntax not supported on SET TILE COLOR statement");
                            return;
                        }
                        // push af
                        addByte(0xF5);
                        // ld hl, parameter value    ; color BC data parameter
                        result_subtype = evalExpression(sub_sub_action);
                        addCast(result_subtype, Lexeme::subtype_numeric);
                        // pop af
                        addByte(0xF1);
                        // or l
                        addByte(0xB5);
                    }

                    // ld bc, (ARG2)
                    addByte(0xED);
                    addCmd(0x4B, def_ARG2);
                    // ld c, a
                    addByte(0x4F);
                    // ld de, 0x000F         ; all lines
                    addCmd(0x11, 0x000F);
                    // ld hl, (ARG)        ; tile number
                    addCmd(0x2A, def_ARG);

                    // call set_tile_color ; hl = tile number, de = line number (15=all), b = bank number (3=all), c = color data (FC,BC)
                    addCmd(0xCD, def_set_tile_color);

                }

            } else {
                syntax_error("Wrong parameters count on SET TILE COLOR statement");
            }

        } else {

            syntax_error("Invalid syntax on SET TILE statement");

        }

    } else {
        syntax_error("Missing parameters on SET TILE statement");
    }

}

void Compiler::cmd_set_sprite() {
    ActionNode *action = current_action->actions[0], *sub_action, *sub_sub_action;
    Lexeme *lexeme;
    unsigned int i, t, tt;
    int result_subtype;

    t = action->actions.size();
    if(t) {

        action = action->actions[0];
        lexeme = action->lexeme;
        t = action->actions.size();

        if(lexeme->value == "TRANSPOSE") {



        } else if(lexeme->value == "PATTERN") {

            if(t >= 2 && t <= 3) {

                // tile number
                sub_action = action->actions[0];
                // ld hl, parameter value    ; tile number
                result_subtype = evalExpression(sub_action);
                addCast(result_subtype, Lexeme::subtype_numeric);
                // ld (ARG), hl
                addCmd(0x22, def_ARG);

                // bank number
                if(t == 3) {
                    sub_action = action->actions[2];
                    // ld hl, parameter value    ; tile number
                    result_subtype = evalExpression(sub_action);
                    addCast(result_subtype, Lexeme::subtype_numeric);
                    // ld h, l
                    addByte(0x65);
                } else {
                    // ld h, 0x03
                    addWord(0x26, 0x03);
                }
                // ld (ARG2), hl
                addCmd(0x22, def_ARG2);

                // pattern data
                sub_action = action->actions[1];
                lexeme = sub_action->lexeme;

                if(lexeme->value == "ARRAY") {

                    tt = sub_action->actions.size();

                    for(i = 0; i < tt; i++) {

                        sub_sub_action = sub_action->actions[i];
                        lexeme = sub_sub_action->lexeme;

                        if(lexeme->type == Lexeme::type_literal && lexeme->subtype == Lexeme::subtype_null) {
                            continue;

                        } else {

                            // ld hl, parameter value    ; pattern data parameter
                            result_subtype = evalExpression(sub_sub_action);
                            addCast(result_subtype, Lexeme::subtype_numeric);

                            // ld bc, (ARG2)
                            addByte(0xED);
                            addCmd(0x4B, def_ARG2);
                            // ld c, l
                            addByte(0x4D);
                            // ld de, *i*
                            addCmd(0x11, i);
                            // ld hl, (ARG)
                            addCmd(0x2A, def_ARG);

                            // call set_tile_pattern ; hl = tile number, de = line number, b = bank number (3=all), c = pattern data
                            addCmd(0xCD, def_set_tile_pattern);

                        }

                    }


                } else {
                    syntax_error("Wrong pattern parameter on SET SPRITE PATTERN statement");
                }

            } else {
                syntax_error("Wrong parameters count on SET SPRITE PATTERN statement");
            }

        } else if(lexeme->value == "COLOR") {

            if(t >= 2 && t <= 4) {

                // tile number
                sub_action = action->actions[0];
                // ld hl, parameter value    ; tile number
                result_subtype = evalExpression(sub_action);
                addCast(result_subtype, Lexeme::subtype_numeric);
                // ld (ARG), hl
                addCmd(0x22, def_ARG);

                // bank number
                if(t == 4) {
                    sub_action = action->actions[3];
                    // ld hl, parameter value    ; tile number
                    result_subtype = evalExpression(sub_action);
                    addCast(result_subtype, Lexeme::subtype_numeric);
                    // ld h, l
                    addByte(0x65);
                } else {
                    // ld h, 0x03
                    addWord(0x26, 0x03);
                }
                // ld (ARG2), hl
                addCmd(0x22, def_ARG2);

                // color data
                sub_action = action->actions[1];
                lexeme = sub_action->lexeme;

                if(lexeme->value == "ARRAY") {

                    tt = sub_action->actions.size();

                    for(i = 0; i < tt; i++) {

                        sub_sub_action = sub_action->actions[i];
                        lexeme = sub_sub_action->lexeme;

                        if(lexeme->type == Lexeme::type_literal && lexeme->subtype == Lexeme::subtype_null) {
                            continue;

                        } else {

                            // ld hl, parameter value    ; color FC data parameter
                            result_subtype = evalExpression(sub_sub_action);
                            addCast(result_subtype, Lexeme::subtype_numeric);
                            // ld a, l
                            addByte(0x7D);
                            // rla
                            addByte(0x17);
                            // rla
                            addByte(0x17);
                            // rla
                            addByte(0x17);
                            // rla
                            addByte(0x17);
                            // and 0xF0
                            addWord(0xE6, 0xF0);

                            if(t >= 3) {
                                // color data
                                sub_sub_action = action->actions[2];
                                lexeme = sub_sub_action->lexeme;
                                if(lexeme->value != "ARRAY") {
                                    syntax_error("Syntax not supported on SET SPRITE COLOR statement");
                                    return;
                                }
                                if(i < sub_sub_action->actions.size()) {
                                    sub_sub_action = sub_sub_action->actions[i];
                                    lexeme = sub_sub_action->lexeme;
                                    if(!(lexeme->type == Lexeme::type_literal && lexeme->subtype == Lexeme::subtype_null)) {
                                        // push af
                                        addByte(0xF5);
                                        // ld hl, parameter value    ; color BC data parameter
                                        result_subtype = evalExpression(sub_sub_action);
                                        addCast(result_subtype, Lexeme::subtype_numeric);
                                        // pop af
                                        addByte(0xF1);
                                        // or l
                                        addByte(0xB5);
                                    }
                                }
                            }

                            // ld bc, (ARG2)
                            addByte(0xED);
                            addCmd(0x4B, def_ARG2);
                            // ld c, a
                            addByte(0x4F);
                            // ld de, *i*
                            addCmd(0x11, i);
                            // ld hl, (ARG)        ; tile number
                            addCmd(0x2A, def_ARG);

                            // call set_tile_color ; hl = tile number, de = line number (15=all), b = bank number (3=all), c = color data (FC,BC)
                            addCmd(0xCD, def_set_tile_color);

                        }

                    }

                } else {

                    // ld hl, parameter value    ; color FC data parameter
                    result_subtype = evalExpression(sub_action);
                    addCast(result_subtype, Lexeme::subtype_numeric);
                    // ld a, l
                    addByte(0x7D);
                    // rla
                    addByte(0x17);
                    // rla
                    addByte(0x17);
                    // rla
                    addByte(0x17);
                    // rla
                    addByte(0x17);
                    // and 0xF0
                    addWord(0xE6, 0xF0);

                    if(t >= 3) {
                        // color data
                        sub_sub_action = action->actions[2];
                        lexeme = sub_sub_action->lexeme;
                        if(lexeme->value == "ARRAY") {
                            syntax_error("Syntax not supported on SET SPRITE COLOR statement");
                            return;
                        }
                        // push af
                        addByte(0xF5);
                        // ld hl, parameter value    ; color BC data parameter
                        result_subtype = evalExpression(sub_sub_action);
                        addCast(result_subtype, Lexeme::subtype_numeric);
                        // pop af
                        addByte(0xF1);
                        // or l
                        addByte(0xB5);
                    }

                    // ld bc, (ARG2)
                    addByte(0xED);
                    addCmd(0x4B, def_ARG2);
                    // ld c, a
                    addByte(0x4F);
                    // ld de, 0x000F         ; all lines
                    addCmd(0x11, 0x000F);
                    // ld hl, (ARG)        ; tile number
                    addCmd(0x2A, def_ARG);

                    // call set_tile_color ; hl = tile number, de = line number (15=all), b = bank number (3=all), c = color data (FC,BC)
                    addCmd(0xCD, def_set_tile_color);

                }

            } else {
                syntax_error("Wrong parameters count on SET SPRITE COLOR statement");
            }

        } else {

            syntax_error("Invalid syntax on SET SPRITE statement");

        }

    } else {
        syntax_error("Missing parameters on SET SPRITE statement");
    }

}

void Compiler::cmd_on() {
    ActionNode *action;
    Lexeme *next_lexeme;
    unsigned int t = current_action->actions.size();

    if(t) {

        action = current_action->actions[0];
        next_lexeme = action->lexeme;

        if (next_lexeme->type == Lexeme::type_keyword && next_lexeme->value == "ERROR") {
            cmd_on_error();
        } else if(next_lexeme->type == Lexeme::type_keyword && next_lexeme->value == "INTERVAL") {
            cmd_on_interval();
        } else if(next_lexeme->type == Lexeme::type_keyword && next_lexeme->value == "KEY") {
            cmd_on_key();
        } else if(next_lexeme->type == Lexeme::type_keyword && next_lexeme->value == "SPRITE") {
            cmd_on_sprite();
        } else if(next_lexeme->type == Lexeme::type_keyword && next_lexeme->value == "STOP") {
            cmd_on_stop();
        } else if(next_lexeme->type == Lexeme::type_keyword && next_lexeme->value == "STRIG") {
            cmd_on_strig();
        } else if(next_lexeme->type == Lexeme::type_keyword && next_lexeme->value == "INDEX") {
            cmd_on_goto_gosub();
        } else {
            syntax_error("Invalid ON statement");
        }

    } else {
        syntax_error("Empty ON statement");
    }

}

void Compiler::cmd_on_error() {
    syntax_error("Not implemented yet");
}

void Compiler::cmd_on_interval() {
    ActionNode *action, *sub_action, *parm_action;
    Lexeme *lexeme, *parm_lexeme;
    unsigned int t;
    int result_subtype;

    action = current_action->actions[0];
    t = action->actions.size();

    if(t == 2) {

        // INDEX VARIABLE

        sub_action = action->actions[0];
        lexeme = sub_action->lexeme;
        if(lexeme->value != "INDEX") {
            syntax_error("Interval index is missing in ON INTERVAL");
            return;
        }
        if(sub_action->actions.size()!=1) {
            syntax_error("Wrong parameter count in interval index from ON INTERVAL");
            return;
        }

        parm_action = sub_action->actions[0];

        // ld hl, variable
        result_subtype = evalExpression(parm_action);

        addCast(result_subtype, Lexeme::subtype_numeric);

        // di
        addByte(0xF3);
        //   ld (0xFCA0), hl   ; INTVAL
        addCmd(0x22, 0xFCA0);
        //   xor a
        addByte(0xAF);
        //   ld (0xFC7F), a    ; ON INTERVAL STATE (0=off, 1=on)
        addCmd(0x32, 0xFC7F);
        //   ld (0xFCA3), a    ; INTCNT - initialize with zero (2 bytes)
        addCmd(0x32, 0xFCA3);
        addCmd(0x32, 0xFCA4);
        // ei
        addByte(0xFB);

        // GOSUB

        sub_action = action->actions[1];
        lexeme = sub_action->lexeme;
        if(lexeme->value != "GOSUB") {
            syntax_error("GOSUB is missing in ON INTERVAL");
            return;
        }
        if(sub_action->actions.size()!=1) {
            syntax_error("Wrong parameter count in GOSUB from ON INTERVAL");
            return;
        }

        parm_action = sub_action->actions[0];
        parm_lexeme = parm_action->lexeme;

        if(parm_lexeme->type == Lexeme::type_literal && parm_lexeme->subtype == Lexeme::subtype_numeric) {

            if(megaROM) {

                // ld hl, GOSUB ADDRESS
                addFix( parm_lexeme->value );
                addCmd(0xFF, 0x0000);
                // ld (0xFC80), hl                ; INTERVAL ADDRESS
                addCmd(0x22, 0xFC80);
                // ld (MR_TRAP_SEGMS+17), a       ; INTERVAL segment
                addCmd(0x32, def_MR_TRAP_SEGMS + 17);

            } else {

                // ld hl, GOSUB ADDRESS
                addFix( parm_lexeme->value );
                addCmd(0x21, 0x0000);
                // ld (0xFC80), hl   ; GOSUB ADDRESS
                addCmd(0x22, 0xFC80);

            }

        } else {

            syntax_error("Invalid GOSUB parameter in ON INTERVAL");
            return;

        }

    } else {
        syntax_error("ON INTERVAL with empty parameters");
    }

}

void Compiler::cmd_interval() {
    ActionNode *action;
    Lexeme *next_lexeme;
    unsigned int t = current_action->actions.size();

    if(t == 1) {

        action = current_action->actions[0];
        next_lexeme = action->lexeme;

        // ld hl, 0xFC7F    ; INTERVAL state
        addCmd(0x21, 0xfc7f);

        if (next_lexeme->type == Lexeme::type_keyword && next_lexeme->value == "ON") {
            // call 0x6c89   ; xbasic turn on trap (hl=trap state address)
            addCmd(0xCD, def_XBASIC_TRAP_ON);
        } else if(next_lexeme->type == Lexeme::type_keyword && next_lexeme->value == "OFF") {
            // call 0x6c9c   ; xbasic turn off trap (hl=trap state address)
            addCmd(0xCD, def_XBASIC_TRAP_OFF);
        } else if(next_lexeme->type == Lexeme::type_keyword && next_lexeme->value == "STOP") {
            // call 0x6ca5   ; xbasic turn stop trap (hl=trap state address)
            addCmd(0xCD, def_XBASIC_TRAP_STOP);
        } else {
            syntax_error("Invalid INTERVAL statement");
        }

    } else {
        syntax_error("Empty INTERVAL statement");
    }

}

void Compiler::cmd_stop() {
    ActionNode *action;
    Lexeme *next_lexeme;
    unsigned int t = current_action->actions.size();

    if(t == 1) {

        action = current_action->actions[0];
        next_lexeme = action->lexeme;

        // ld hl, 0xFC6A    ; STOP state
        addCmd(0x21, 0xFC6A);

        if (next_lexeme->type == Lexeme::type_keyword && next_lexeme->value == "ON") {
            // call 0x6c89   ; xbasic turn on trap (hl=trap state address)
            addCmd(0xCD, def_XBASIC_TRAP_ON);
        } else if(next_lexeme->type == Lexeme::type_keyword && next_lexeme->value == "OFF") {
            // call 0x6c9c   ; xbasic turn off trap (hl=trap state address)
            addCmd(0xCD, def_XBASIC_TRAP_OFF);
        } else if(next_lexeme->type == Lexeme::type_keyword && next_lexeme->value == "STOP") {
            // call 0x6ca5   ; xbasic turn stop trap (hl=trap state address)
            addCmd(0xCD, def_XBASIC_TRAP_STOP);
        } else {
            syntax_error("Invalid STOP statement");
        }

    } else if(t==0) {
        cmd_end(false);
    } else {
        syntax_error("Wrong number of parameters in STOP");
    }

}

void Compiler::cmd_sprite() {
    ActionNode *action;
    Lexeme *next_lexeme;
    unsigned int t = current_action->actions.size();

    if(t == 1) {

        action = current_action->actions[0];
        next_lexeme = action->lexeme;

        if(next_lexeme->value == "LOAD") {
            current_action = action;
            cmd_sprite_load();
            return;
        }

        // ld hl, 0xFC6D   ; SPRITE state
        addCmd(0x21, 0xFC6D);

        if (next_lexeme->type == Lexeme::type_keyword && next_lexeme->value == "ON") {
            // call 0x6c89   ; xbasic turn on trap (hl=trap state address)
            addCmd(0xCD, def_XBASIC_TRAP_ON);
        } else if(next_lexeme->type == Lexeme::type_keyword && next_lexeme->value == "OFF") {
            // call 0x6c9c   ; xbasic turn off trap (hl=trap state address)
            addCmd(0xCD, def_XBASIC_TRAP_OFF);
        } else if(next_lexeme->type == Lexeme::type_keyword && next_lexeme->value == "STOP") {
            // call 0x6ca5   ; xbasic turn stop trap (hl=trap state address)
            addCmd(0xCD, def_XBASIC_TRAP_STOP);
        } else {
            syntax_error("Invalid SPRITE statement");
        }

    } else {
        syntax_error("Empty SPRITE statement");
    }

}

void Compiler::cmd_sprite_load() {
    ActionNode *action;
    unsigned int i, t = current_action->actions.size();
    int result_subtype;

    if(t) {

        if(t > 1) {
            syntax_error("SPRITE LOAD with excess of parameters");
            return;
        }

        for(i = 0; i < t; i++) {

            action = current_action->actions[i];
            result_subtype = evalExpression(action);
            addCast(result_subtype, Lexeme::subtype_numeric);

            // ld (DAC), hl
            addCmd(0x22, def_DAC);
            // call cmd_wrtspr                    ; tiny sprite loader
            addCmd(0xCD, def_cmd_wrtspr);

        }

    } else {
        syntax_error("SPRITE LOAD with empty parameters");
    }

}

void Compiler::cmd_key() {
    ActionNode *action1, *action2;
    Lexeme *next_lexeme;
    unsigned int t = current_action->actions.size();
    int result_subtype;

    if(t == 1) {

        action1 = current_action->actions[0];
        next_lexeme = action1->lexeme;

        if (next_lexeme->type == Lexeme::type_keyword && next_lexeme->value == "ON") {
            // call 0x00CF   ; DSPFNK - (0xF3DE = CNSDFG: function keys presentation)
            addCmd(0xCD, 0x00CF);
        } else if(next_lexeme->type == Lexeme::type_keyword && next_lexeme->value == "OFF") {
            // call 0x00CC   ; ERAFNK
            addCmd(0xCD, 0x00CC);
        } else {
            syntax_error("Invalid KEY statement");
        }

    } else if(t == 2) {

        action2 = current_action->actions[1];
        next_lexeme = action2->lexeme;

        // ld hl, key number
        action1 = current_action->actions[0];
        result_subtype = evalExpression(action1);
        addCast(result_subtype, Lexeme::subtype_numeric);

        // dec hl
        addByte(0x2B);

        if( (next_lexeme->type == Lexeme::type_keyword && next_lexeme->subtype == Lexeme::subtype_function) ||
                (next_lexeme->type == Lexeme::type_operator && next_lexeme->value == "+") ||
                (next_lexeme->subtype == Lexeme::subtype_string) ) {

            // add hl, hl
            addByte(0x29);
            // add hl, hl
            addByte(0x29);
            // add hl, hl
            addByte(0x29);
            // add hl, hl      ; key number * 16
            addByte(0x29);
            // ld de, 0xF87F   ; FNKSTR (10 x 16 bytes)
            addCmd(0x11, 0xf87f);
            // add hl, de
            addByte(0x19);

            // push hl
            addByte(0xE5);
            //   ld a, 32        ; space
            addWord(0x3E, 0x20);
            //   ld (hl), a
            addByte(0x77);
            //   ld e, l
            addByte(0x5d);
            //   ld d, h
            addByte(0x54);
            //   inc de
            addByte(0x13);
            //   ld bc, 15
            addCmd(0x01, 0x000F);
            //   ldir
            addWord(0xED, 0xB0);

            //   ld hl, variable address
            result_subtype = evalExpression(action2);
            addCast(result_subtype, Lexeme::subtype_string);

            //   xor a
            addByte(0xAF);
            //   ld c, (hl)
            addByte(0x4E);
            //   ld b, a
            addWord(0x47);
            //   inc hl
            addByte(0x23);

            // pop de
            addByte(0xD1);

            // or c
            addByte(0xB1);
            // jr z, $+3
            addWord(0x28, 0x02);
            //   ldir
            addWord(0xED, 0xB0);
            // ld (de), a
            addByte(0x12);
            // call 0x00C9    ; FNKSB
            addCmd(0xCD, 0x00C9);

        } else {

            // ld e, l
            addByte(0x5D);
            // ld d, h
            addByte(0x54);
            // add hl, de
            addByte(0x19);
            // add hl, de       ; key number * 3
            addByte(0x19);

            // ld de, 0xFC4C    ; KEY state position = key number * 3 + 0xFC4C
            addCmd(0x11, 0xfc4c);

            // add hl, de
            addByte(0x19);

            if (next_lexeme->type == Lexeme::type_keyword && next_lexeme->value == "ON") {
                // call 0x6c89   ; xbasic turn on trap (hl=trap state address)
                addCmd(0xCD, def_XBASIC_TRAP_ON);
            } else if(next_lexeme->type == Lexeme::type_keyword && next_lexeme->value == "OFF") {
                // call 0x6c9c   ; xbasic turn off trap (hl=trap state address)
                addCmd(0xCD, def_XBASIC_TRAP_OFF);
            } else if(next_lexeme->type == Lexeme::type_keyword && next_lexeme->value == "STOP") {
                // call 0x6ca5   ; xbasic turn stop trap (hl=trap state address)
                addCmd(0xCD, def_XBASIC_TRAP_STOP);
            } else {
                syntax_error("Invalid KEY statement");
            }

        }

    } else {
        syntax_error("Empty KEY statement");
    }

}

void Compiler::cmd_strig() {
    ActionNode *action1, *action2;
    Lexeme *next_lexeme;
    unsigned int t = current_action->actions.size();
    int result_subtype;

    if(t == 2) {

        action2 = current_action->actions[1];
        next_lexeme = action2->lexeme;

        // ld hl, strig number
        action1 = current_action->actions[0];
        result_subtype = evalExpression(action1);
        addCast(result_subtype, Lexeme::subtype_numeric);

        // dec hl
        //addByte(0x2B);

        // ld e, l
        addByte(0x5D);
        // ld d, h
        addByte(0x54);
        // add hl, de
        addByte(0x19);
        // add hl, de       ; strig number * 3
        addByte(0x19);

        // ld de, 0xFC70    ; STRIG state position = key number * 3 + 0xFC70
        addCmd(0x11, 0xFC70);

        // add hl, de
        addByte(0x19);

        if (next_lexeme->type == Lexeme::type_keyword && next_lexeme->value == "ON") {
            // call 0x6c89   ; xbasic turn on trap (hl=trap state address)
            addCmd(0xCD, def_XBASIC_TRAP_ON);
        } else if(next_lexeme->type == Lexeme::type_keyword && next_lexeme->value == "OFF") {
            // call 0x6c9c   ; xbasic turn off trap (hl=trap state address)
            addCmd(0xCD, def_XBASIC_TRAP_OFF);
        } else if(next_lexeme->type == Lexeme::type_keyword && next_lexeme->value == "STOP") {
            // call 0x6ca5   ; xbasic turn stop trap (hl=trap state address)
            addCmd(0xCD, def_XBASIC_TRAP_STOP);
        } else {
            syntax_error("Invalid STRIG statement");
        }


    } else {
        syntax_error("Wrong number of parameters in STRIG statement");
    }

}

bool Compiler::addCheckTraps() {

    if(parser->has_traps) {
        // call 0x6c25   ; xbasic check traps
        addCmd(0xCD, def_XBASIC_TRAP_CHECK);
    }

    return parser->has_traps;
}


void Compiler::cmd_on_key() {
    ActionNode *action, *sub_action;
    Lexeme *lexeme, *sub_lexeme;
    unsigned int i, t;

    action = current_action->actions[0];
    if(action->actions.size() != 1) {
        syntax_error("Wrong parameters in ON KEY");
        return;
    }

    action = action->actions[0];
    lexeme = action->lexeme;
    if(lexeme->value != "GOSUB") {
        syntax_error("GOSUB parameters is missing in ON KEY");
        return;
    }
    t = action->actions.size();

    if(t) {

        // GOSUB LIST

        // ld hl, 0xFC4D    ; KEY first GOSUB position = 0xFC4C+1
        addCmd(0x21, 0xfc4D);

        for(i = 0; i < t; i++) {

            sub_action = action->actions[i];
            sub_lexeme = sub_action->lexeme;

            if(sub_lexeme->type == Lexeme::type_literal && sub_lexeme->subtype == Lexeme::subtype_numeric) {

                if(megaROM) {

                    // push hl
                    addByte(0xE5);
                    //   ld hl, GOSUB ADDRESS
                    addFix( sub_lexeme->value );
                    addCmd(0xFF, 0x0000);
                    //   ld (MR_TRAP_SEGMS), a       ; KEY segment
                    addCmd(0x32, def_MR_TRAP_SEGMS + i);
                    //   ex de, hl
                    addByte(0xEB);
                    // pop hl
                    addByte(0xE1);

                } else {

                    // ld de, call address
                    addFix( sub_lexeme->value );
                    addCmd(0x11, 0x0000);

                }

            } else {

                // ld hl, 0x368D   ; dummy bios RET address
                addCmd(0x21, def_XBASIC_DUMMY_RET);

            }

            // ld (hl), e
            addByte(0x73);
            // inc hl
            addByte(0x23);
            // ld (hl), d
            addByte(0x72);
            // inc hl
            addByte(0x23);
            // inc hl
            addByte(0x23);

        }

    } else {
        syntax_error("ON KEY with empty parameters");
    }

}

void Compiler::cmd_on_sprite() {
    ActionNode *action, *sub_action;
    Lexeme *lexeme, *sub_lexeme;
    unsigned int t, i = 0;

    action = current_action->actions[0];
    if(action->actions.size() != 1) {
        syntax_error("Wrong parameters in ON SPRITE");
        return;
    }

    action = action->actions[0];
    lexeme = action->lexeme;
    if(lexeme->value != "GOSUB") {
        syntax_error("GOSUB parameters is missing in ON SPRITE");
        return;
    }
    t = action->actions.size();

    if(t == 1) {

        // GOSUB address

        sub_action = action->actions[0];
        sub_lexeme = sub_action->lexeme;

        if(sub_lexeme->type == Lexeme::type_literal && sub_lexeme->subtype == Lexeme::subtype_numeric) {

            if(megaROM) {

                // push hl
                addByte(0xE5);
                //   ld hl, GOSUB ADDRESS
                addFix( sub_lexeme->value );
                addCmd(0xFF, 0x0000);
                //   ld (MR_TRAP_SEGMS+11), a       ; SPRITE segment
                addCmd(0x32, def_MR_TRAP_SEGMS + 11 + i);
                //   ex de, hl
                addByte(0xEB);
                // pop hl
                addByte(0xE1);

            } else {

                // ld hl, call address
                addFix( sub_lexeme->value );
                addCmd(0x21, 0x0000);

            }

        } else {

            // ld hl, 0x368D   ; dummy bios RET address
            addCmd(0x21, def_XBASIC_DUMMY_RET);

        }

        // ld (0xFC6E), hl     ; STOP GOSUB position = 0xFC6D+1
        addCmd(0x22, 0xFC6E);

    } else {
        syntax_error("ON SPRITE with wrong count of parameters");
    }
}

void Compiler::cmd_on_stop() {
    ActionNode *action, *sub_action;
    Lexeme *lexeme, *sub_lexeme;
    unsigned int t, i=0;

    action = current_action->actions[0];
    if(action->actions.size() != 1) {
        syntax_error("Wrong parameters in ON STOP");
        return;
    }

    action = action->actions[0];
    lexeme = action->lexeme;
    if(lexeme->value != "GOSUB") {
        syntax_error("GOSUB parameters is missing in ON STOP");
        return;
    }
    t = action->actions.size();

    if(t == 1) {

        // GOSUB address

        sub_action = action->actions[0];
        sub_lexeme = sub_action->lexeme;

        if(sub_lexeme->type == Lexeme::type_literal && sub_lexeme->subtype == Lexeme::subtype_numeric) {

            if(megaROM) {

                // push hl
                addByte(0xE5);
                //   ld hl, GOSUB ADDRESS
                addFix( sub_lexeme->value );
                addCmd(0xFF, 0x0000);
                //   ld (MR_TRAP_SEGMS+10), a       ; STOP segment
                addCmd(0x32, def_MR_TRAP_SEGMS + 10 + i);
                //   ex de, hl
                addByte(0xEB);
                // pop hl
                addByte(0xE1);

            } else {

                // ld hl, call address
                addFix( sub_lexeme->value );
                addCmd(0x21, 0x0000);

            }

        } else {

            // ld hl, 0x368D   ; dummy bios RET address
            addCmd(0x21, def_XBASIC_DUMMY_RET);

        }

        // ld (0xFC6B), hl     ; STOP GOSUB position = 0xFC6A+1
        addCmd(0x22, 0xfc6B);

    } else {
        syntax_error("ON STOP with wrong count of parameters");
    }
}

void Compiler::cmd_on_strig() {
    ActionNode *action, *sub_action;
    Lexeme *lexeme, *sub_lexeme;
    unsigned int i, t;

    action = current_action->actions[0];
    if(action->actions.size() != 1) {
        syntax_error("Wrong parameters in ON STRIG");
        return;
    }

    action = action->actions[0];
    lexeme = action->lexeme;
    if(lexeme->value != "GOSUB") {
        syntax_error("GOSUB parameters is missing in ON STRIG");
        return;
    }
    t = action->actions.size();

    if(t) {

        // GOSUB LIST

        // ld hl, 0xFC71    ; STRIG first GOSUB position = 0xFC70+1
        addCmd(0x21, 0xFC71);

        for(i = 0; i < t; i++) {

            sub_action = action->actions[i];
            sub_lexeme = sub_action->lexeme;

            if(sub_lexeme->type == Lexeme::type_literal && sub_lexeme->subtype == Lexeme::subtype_numeric) {

                if(megaROM) {

                    // push hl
                    addByte(0xE5);
                    //   ld hl, GOSUB ADDRESS
                    addFix( sub_lexeme->value );
                    addCmd(0xFF, 0x0000);
                    //   ld (MR_TRAP_SEGMS+12), a       ; STRIG segment
                    addCmd(0x32, def_MR_TRAP_SEGMS + 12 + i);
                    //   ex de, hl
                    addByte(0xEB);
                    // pop hl
                    addByte(0xE1);

                } else {

                    // ld de, call address
                    addFix( sub_lexeme->value );
                    addCmd(0x11, 0x0000);

                }

            } else {

                // ld de, 0x368D   ; dummy bios RET address
                addCmd(0x11, def_XBASIC_DUMMY_RET);

            }

            // ld (hl), e
            addByte(0x73);
            // inc hl
            addByte(0x23);
            // ld (hl), d
            addByte(0x72);
            // inc hl
            addByte(0x23);
            // inc hl
            addByte(0x23);

        }

    } else {
        syntax_error("ON STRIG with empty parameters");
    }
}

void Compiler::cmd_on_goto_gosub() {
    ActionNode *action, *sub_action;
    Lexeme *lexeme, *sub_lexeme;
    FixNode *mark;
    unsigned int i, t = current_action->actions.size();
    int result_subtype;

    if(t == 2) {

        // INDEX VARIABLE
        // ld hl, variable

        action = current_action->actions[0];
        if(action->actions.size()==0) {
            syntax_error("Empty parameter in ON GOTO/GOSUB");
            return;
        }

        sub_action = action->actions[0];

        result_subtype = evalExpression(sub_action);

        addCast(result_subtype, Lexeme::subtype_numeric);

        // GOTO / GOSUB LIST

        action = current_action->actions[1];
        lexeme = action->lexeme;
        t = action->actions.size();

        // ld a, l
        addByte(0x7D);

        if(megaROM) {
            // ld (TEMP), a
            addCmd(0x32, def_TEMP);
        }

        // and a
        addByte(0xA7);
        // jp z, address
        mark = addMark();
        addCmd(0xCA, 0x0000);

        for(i = 0; i < t; i++) {

            sub_action = action->actions[i];
            sub_lexeme = sub_action->lexeme;

            if(sub_lexeme->type == Lexeme::type_literal && sub_lexeme->subtype == Lexeme::subtype_numeric) {

                // Trim leading zeros
                while (sub_lexeme->value.find("0") == 0 && sub_lexeme->value.size() > 1) {
                    sub_lexeme->value.erase(0, 1);
                }

                if(lexeme->value == "GOTO") {

                    if(megaROM) {
                        // ld a, (TEMP)
                        addCmd(0x3A, def_TEMP);
                    }

                    // dec a
                    addByte(0x3D);

                    if(megaROM) {
                        // ld (TEMP), a
                        addCmd(0x32, def_TEMP);
                    }

                    // jp z, address
                    addFix( sub_lexeme->value );
                    addCmd(0xCA, 0x0000);

                } else {

                    if(megaROM) {
                        // ld a, (TEMP)
                        addCmd(0x3A, def_TEMP);
                    }

                    // dec a
                    addByte(0x3D);

                    if(megaROM) {
                        // ld (TEMP), a
                        addCmd(0x32, def_TEMP);
                    }

                    if(megaROM) {
                        // jr nz, $+25
                        addWord(0x20, 24);
                    } else {
                        // jr nz, $+7
                        addWord(0x20, 0x06);
                    }
                    //   call address
                    addFix( sub_lexeme->value );
                    addCmd(0xCD, 0x0000);
                    //   jp address
                    addFix(mark->symbol);
                    addCmd(0xC3, 0x0000);

                }

            } else {

                if(megaROM) {
                    // ld a, (TEMP)
                    addCmd(0x3A, def_TEMP);
                }

                // dec a
                addByte(0x3D);

                if(megaROM) {
                    // ld (TEMP), a
                    addCmd(0x32, def_TEMP);
                }

            }

        }

        mark->symbol->address = code_pointer;

    } else {
        syntax_error("ON GOTO/GOSUB with empty parameters");
    }

}

void Compiler::cmd_swap() {
    Lexeme *lexeme1, *lexeme2;
    ActionNode *action1, *action2;
    unsigned int t = current_action->actions.size();

    if(t == 2) {

        action1 = current_action->actions[0];
        lexeme1 = action1->lexeme;
        if(lexeme1->type != Lexeme::type_identifier) {
            syntax_error("Invalid parameter type in SWAP (1st)");
            return;
        }

        action2 = current_action->actions[1];
        lexeme2 = action2->lexeme;
        if(lexeme2->type != Lexeme::type_identifier) {
            syntax_error("Invalid parameter type in SWAP (2nd)");
            return;
        }

        if(lexeme1->subtype == lexeme2->subtype) {

            // ld hl, variable
            addVarAddress(action1);
            // push hl
            addByte(0xE5);
            // ld hl, variable
            addVarAddress(action2);
            // pop de
            addByteOptimized(0xD1);

            if(lexeme1->subtype == Lexeme::subtype_numeric) {
                // call 0x6bf5    ; xbasic SWAP integers (in: hl=var1, de=var2)
                addCmd(0xCD, def_XBASIC_SWAP_INTEGER);
            } else if(lexeme1->subtype == Lexeme::subtype_string) {
                // call 0x6bf9    ; xbasic SWAP strings (in: hl=var1, de=var2)
                addCmd(0xCD, def_XBASIC_SWAP_STRING);
            } else {
                // call 0x6bfd    ; xbasic SWAP floats (in: hl=var1, de=var2)
                addCmd(0xCD, def_XBASIC_SWAP_FLOAT);
            }

        } else {
            syntax_error("Parameters type mismatch in SWAP");
        }

    } else {
        syntax_error("Invalid SWAP parameters");
    }

}

void Compiler::cmd_wait() {
    Lexeme *lexeme;
    ActionNode *action;
    unsigned int i, t = current_action->actions.size();
    int result_subtype;

    if(t == 2 || t == 3) {

        for( i = 0; i < t; i++) {

            action = current_action->actions[i];
            lexeme = action->lexeme;

            if(lexeme) {

                result_subtype = evalExpression(action);

                // cast
                addCast(result_subtype, Lexeme::subtype_numeric);

                switch(i) {

                    case 0 : {
                            // push hl
                            addByte(0xE5);
                        }
                        break;

                    case 1 : {
                            if(t == 3) {
                                // ld a, l
                                addByte(0x7D);
                                // push af
                                addByte(0xF5);
                            } else {
                                // ld d, l
                                addByte(0x55);
                                // pop bc
                                addByte(0xC1);
                                //   in a,(c)
                                addWord(0xED, 0x78);
                                //   and d
                                addByte(0xA2);
                                // jr z, $-4
                                addWord(0x28, 0xFB);
                            }
                        }
                        break;

                    case 2 : {
                            // ld h, l
                            addByte(0x65);
                            // pop de
                            addByte(0xD1);
                            // pop bc
                            addByte(0xC1);
                            //   in a,(c)
                            addWord(0xED, 0x78);
                            //   xor h
                            addByte(0xAC);
                            //   and d
                            addByte(0xA2);
                            // jr z, $-5
                            addWord(0x28, 0xFA);

                        }
                        break;

                }

            }

        }

    } else {
        syntax_error("Invalid WAIT parameters");
    }

}

void Compiler::cmd_data() {

    if(! current_action->actions.size()) {
        syntax_error("DATA with empty parameters");
    }

}

void Compiler::cmd_idata() {

    if(! current_action->actions.size()) {
        syntax_error("IDATA with empty parameters");
    }

}

void Compiler::cmd_read() {
    ActionNode *action;
    Lexeme *lexeme;
    unsigned int i, t = current_action->actions.size();

    if(t) {

        for(i = 0; i < t; i++) {

            action = current_action->actions[i];
            lexeme = action->lexeme;

            if(lexeme->type != Lexeme::type_identifier) {
                syntax_error("Invalid READ parameter type");
                return;
            }

            // call read
            if(megaROM) {
                addCmd(0xCD, def_XBASIC_READ_MR);
            } else {
                addCmd(0xCD, def_XBASIC_READ);
            }

            addCast(Lexeme::subtype_string, lexeme->subtype);

            // do assignment

            addAssignment(action);

        }

    } else {
        syntax_error("READ with empty parameters");
    }

}

void Compiler::cmd_iread() {
    ActionNode *action;
    Lexeme *lexeme;
    unsigned int i, t = current_action->actions.size();

    if(t) {

        for(i = 0; i < t; i++) {

            action = current_action->actions[i];
            lexeme = action->lexeme;

            if(lexeme->type != Lexeme::type_identifier) {
                syntax_error("Invalid IREAD parameter type");
                return;
            }

            // call XBASIC_IREAD
            addCmd(0xCD, def_XBASIC_IREAD);

            addCast(Lexeme::subtype_numeric, lexeme->subtype);

            // do assignment

            addAssignment(action);

        }

    } else {
        syntax_error("IREAD with empty parameters");
    }

}

void Compiler::cmd_resume() {
    syntax_error("RESUME statement not supported in compiled mode");
}

void Compiler::cmd_restore() {
    ActionNode *action;
    Lexeme *lexeme;
    int i, t = current_action->actions.size(), tt;
    int result_subtype;
    bool found;

    if(t == 0) {

        if(megaROM) {
            // ld de, 0
            addCmd(0x11, 0x0000);
            // call xbasic_restore_mr
            addCmd(0xCD, def_XBASIC_RESTORE_MR);
        } else {
            // ld hl, 0
            addCmd(0x21, 0x0000);
            // call xbasic_restore
            addCmd(0xCD, def_XBASIC_RESTORE);
        }

    } else if(t == 1) {

        action = current_action->actions[0];

        if(megaROM) {

            if(action->lexeme->type != Lexeme::type_literal) {
                syntax_error("Invalid RESTORE parameter type");
                return;
            }

            // Trim leading zeros
            while (action->lexeme->value.find("0") == 0 && action->lexeme->value.size() > 1) {
                action->lexeme->value.erase(0, 1);
            }

            t = parser->datas.size();
            tt = 0;
            found = false;

            for(i = 0; i < t; i++) {
                lexeme = parser->datas[i];
                if(lexeme->tag == action->lexeme->value) {
                    found = true;
                    break;
                }
                tt += lexeme->value.size() + 1;
                if(lexeme->value.c_str()[0] == '"')
                    tt -= 2;
            }

            if(!found) {
                syntax_error("Line number not found for RESTORE (Is it after current line?)");
                return;
            }

            // ld de, data count
            addCmd(0x11, i);
            // call restore
            addCmd(0xCD, def_XBASIC_RESTORE_MR);

        } else {

            lexeme = action->lexeme;
            if(lexeme) {
                result_subtype = evalExpression(action);

                // cast
                addCast(result_subtype, Lexeme::subtype_numeric);

                // call restore
                addCmd(0xCD, def_XBASIC_RESTORE);
            }

        }

    } else {
        syntax_error("RESTORE with wrong number of parameters");
    }

}

void Compiler::cmd_irestore() {
    ActionNode *action;
    Lexeme *lexeme;
    int t = current_action->actions.size();
    int result_subtype;

    if(t == 0) {

        // ld hl, (DATLIN)
        addCmd(0x2A, def_DATLIN);
        // ld (DATPTR), hl
        addCmd(0x22, def_DATPTR);

    } else if(t == 1) {

        action = current_action->actions[0];

        lexeme = action->lexeme;
        if(lexeme) {
            result_subtype = evalExpression(action);

            // cast
            addCast(result_subtype, Lexeme::subtype_numeric);

            // ex de, hl
            addByte(0xEB);
            // ld hl, (DATLIN)
            addCmd(0x2A, def_DATLIN);
            // add hl, de
            addByte(0x19);
            // ld (DATPTR), hl
            addCmd(0x22, def_DATPTR);
        }


    } else {
        syntax_error("IRESTORE with wrong number of parameters");
    }

}

void Compiler::cmd_out() {
    Lexeme *lexeme;
    ActionNode *action;
    unsigned int i, t = current_action->actions.size();
    int result_subtype;

    if(t == 2) {

        for( i = 0; i < t; i++) {
            action = current_action->actions[i];
            lexeme = action->lexeme;
            if(lexeme) {

                result_subtype = evalExpression(action);

                // cast
                addCast(result_subtype, Lexeme::subtype_numeric);

                if(i == 0) {
                    // push hl
                    addByte(0xE5);
                } else {
                    // pop bc
                    addByte(0xC1);
                    // out (c), l
                    addWord(0xED, 0x69);
                }

            }

        }

    } else {
        syntax_error("Invalid OUT parameters");
    }

}

void Compiler::cmd_poke() {
    Lexeme *lexeme;
    ActionNode *action;
    unsigned int i, t = current_action->actions.size();
    int result_subtype;

    if(t == 2) {

        for( i = 0; i < t; i++) {
            action = current_action->actions[i];
            lexeme = action->lexeme;
            if(lexeme) {

                result_subtype = evalExpression(action);

                // cast
                addCast(result_subtype, Lexeme::subtype_numeric);

                if(i == 0) {
                    // push hl
                    addByte(0xE5);
                } else {
                    // ex de, hl
                    addByte(0xEB);
                    // pop hl
                    addByte(0xE1);
                    // ld (hl), e
                    addByte(0x73);
                }

            }

        }

    } else {
        syntax_error("Invalid POKE parameters");
    }

}

void Compiler::cmd_vpoke() {
    Lexeme *lexeme;
    ActionNode *action;
    unsigned int i, t = current_action->actions.size();
    int result_subtype;

    if(t == 2) {

        for( i = 0; i < t; i++) {
            action = current_action->actions[i];
            lexeme = action->lexeme;
            if(lexeme) {

                result_subtype = evalExpression(action);

                // cast
                addCast(result_subtype, Lexeme::subtype_numeric);

                if(i == 0) {
                    // push hl
                    addByte(0xE5);
                } else {
                    // ld a, l
                    addByte(0x7D);
                    // pop hl
                    addByte(0xE1);
                    // call 0x70b5    ; xbasic VPOKE (in: hl=address, a=byte)
                    addCmd(0xCD, def_XBASIC_VPOKE);
                }

            }

        }

    } else {
        syntax_error("Invalid VPOKE parameters");
    }

}

void Compiler::cmd_file() {
    Lexeme *lexeme;
    ActionNode *action;
    unsigned int t = current_action->actions.size();

    if(t == 1) {
        action = current_action->actions[0];
        lexeme = action->lexeme;

        if(lexeme->type == Lexeme::type_literal && lexeme->subtype == Lexeme::subtype_string) {
            lexeme->name = "FILE";
            resourceList.push_back(lexeme);
        } else {
            error_message = "Invalid parameter in FILE keyword";
        }

    } else {
        syntax_error("Wrong FILE parameters count");
    }

}

void Compiler::cmd_text() {
    Lexeme *lexeme;
    ActionNode *action;
    unsigned int t = current_action->actions.size();

    if(t == 1) {
        action = current_action->actions[0];
        lexeme = action->lexeme;

        if(lexeme->type == Lexeme::type_literal && lexeme->subtype == Lexeme::subtype_string) {
            lexeme->name = "TEXT";
            resourceList.push_back(lexeme);
        } else {
            error_message = "Invalid parameter in TEXT keyword";
        }

    } else {
        syntax_error("Wrong TEXT parameters count");
    }

}

void Compiler::cmd_call() {
    ActionNode *action;
    Lexeme *lexeme;
    unsigned int i, t = current_action->actions.size();

    if(t) {

        for(i = 0; i < t; i++) {

            action = current_action->actions[i];
            lexeme = action->lexeme;

            if(lexeme->value == "TURBO") {

                // not need anymore... he he he...

            } else if(lexeme->value == "SYSTEM") {

                // ld hl, data address
                addVarAddress(action);
                // ld a, (hl)      ; first character
                addByte(0x7E);
                // ld ix, (CALL)    ; CALL
                addWord(0xDD, 0x2A);
                addWord(def_CALL_STMT);
                // call CALBAS
                addCmd( 0xcd, def_CALBAS );
                // ei
                addByte(0xFB);

            } else {
                // todo: concatenate CALL statement  into a literal string
                //       and run it with BASIC like above
                syntax_error("CALL statement invalid");
                return;

            }

        }

    } else {
        syntax_error("CALL with empty parameters");
    }

}

void Compiler::cmd_maxfiles() {
    ActionNode *action;
    unsigned int t = current_action->actions.size();
    int result_subtype;
    FixNode* mark;

    if(t) {

        // xor a
        //addByte(0xAF);
        // ld (NLONLY), a
        //addCmd(0x32, def_NLONLY);
        // ld (DSKDIS), a
        //addCmd(0x32, 0xFD99);

        action = current_action->actions[0];
        result_subtype = evalExpression(action);
        addCast(result_subtype, Lexeme::subtype_numeric);

        // ld a, (MAXFIL)
        addCmd(0x3A, def_MAXFIL);
        // cp l
        addByte(0xBD);
        // jp z, $                ; skip if equal
        mark = addMark();
        addCmd(0xCA, 0x0000);

        // ld a, l
        addByte(0x7D);

        // ld (MAXFIL), a
        addCmd(0x32, def_MAXFIL);

        // push af
        addByte(0xF5);

        addEnableBasicSlot();

        // ld hl, fake empty line
        addCmd(0x21, def_ENDPRG);

        // call CLOSE_ALL
        addCmd(0xCD, def_CLOSE_ALL);

        // pop af
        addByte(0xF1);

        // call MAXFILES
        addCmd(0xCD, def_MAXFILES);

        // call 0x7304			; end printeroutput (basic interpreter function)
        addCmd(0xCD, 0x7304);
        // call 0x4AFF			; return interpreter output to screen (basic interpreter function)
        addCmd(0xCD, 0x4AFF);

        addDisableBasicSlot();

        // ; restore stack state
        // ld bc, 16
        addCmd(0x01, 0x0010);
        // ld (TEMP), sp
        addByte(0xED);
        addCmd(0x73, def_TEMP);
        // ld hl, (TEMP)
        addCmd(0x2A, def_TEMP);
        // xor a
        addByte(0xAF);
        // sbc hl, bc
        addWord(0xED, 0x42);
        // ld sp,hl
        addByte(0xF9);
        // ex de,hl
        addByte(0xEB);
        // ld hl, (SAVSTK)
        addCmd(0x2A, def_SAVSTK);
        // ldir
        addWord(0xED, 0xB0);
        // ld (SAVSTK), sp
        addByte(0xED);
        addCmd(0x73, def_SAVSTK);

        mark->symbol->address = code_pointer;

    } else {

        syntax_error("Empty MAXFILES assignment");

    }

}

void Compiler::cmd_open() {
    ActionNode *action;
    Lexeme *lexeme;
    unsigned int i, t = current_action->actions.size();
    int result_subtype, state=0;
    FixNode *mark;
    bool has[4];

    for(i=0; i<4; i++)
        has[i] = false;

    file_support = true;

    if(t) {

        for(i = 0; i < t; i++) {

            action = current_action->actions[i];
            lexeme = action->lexeme;

            switch(state) {
                // file name
                case 0: {
                        has[state] = true;
                        state = 1;
                        result_subtype = evalExpression(action);
                        addCast(result_subtype, Lexeme::subtype_string);

                        // push hl
                        addByte(0xE5);
                    }
                    break;

                // FOR
                case 1: {
                        has[state] = true;
                        state = 2;
                        if(lexeme->value == "OUT") {
                            // ld a, 2     ; output mode
                            addWord(0x3E, 0x02);
                        } else if(lexeme->value == "APP") {
                            // ld a, 8     ; append mode
                            addWord(0x3E, 0x08);
                        } else if(lexeme->value == "INPUT") {
                            // ld a, 1     ; input mode
                            addWord(0x3E, 0x01);
                        } else {
                            // ld a, 4     ; random mode
                            addWord(0x3E, 0x04);
                            i --;
                        }
                        // push af
                        addByte(0xF5);
                    }
                    break;

                // AS
                case 2: {
                        has[state] = true;
                        state = 3;
                        result_subtype = evalExpression(action);
                        addCast(result_subtype, Lexeme::subtype_numeric);
                        // push hl
                        addByte(0xE5);
                    }
                    break;

                // LEN
                case 3: {
                        has[state] = true;
                        state = 4;

                        result_subtype = evalExpression(action);
                        addCast(result_subtype, Lexeme::subtype_numeric);
                    }
                    break;
            }

        }

        addEnableBasicSlot();

        // LEN
        if(!has[3]) {
            // ld hl, 256  ; default record size
            addCmd(0x21, 0x0100);
        }
        // ld (RECSIZ), hl
        addCmd(0x22, def_RECSIZ);

        // AS
        if(has[2]) {
            // pop hl
            addByte(0xE1);
            // ld a, l
            addByte(0x7D);
        } else {
            // xor a
            addByte(0xAF);
        }
        // ld (TEMP), a
        addCmd(0x32, def_TEMP);

        // FOR
        if(has[1]) {
            // pop af
            addByte(0xF1);
        } else {
            // xor a
            addByte(0xAF);
        }
        // ld (TEMP+1), a           ; file mode
        addCmd(0x32, def_TEMP+1);

        if(megaROM) {
            // ld hl, 0x0000             ; get return point address
            mark = addMark();
            addCmd(0xFF, 0x0000);
            // ld c, l
            addByte(0x4d);
            // ld b, h
            addByte(0x44);
        } else {
            // ld bc, 0x0000             ; get return point address
            mark = addMark();
            addCmd(0x01, 0x0000);
        }

        // FILE
        if(has[0]) {
            // pop hl
            addByte(0xE1);
        } else {
            // ld hl, NULL STR
            addCmd(0x21, 0x8000);
        }

        // push bc            ; save return point to run after FILESPEC
        addByte(0xC5);

        // ld e, (hl)
        addByte(0x5E);

        // inc hl
        addByte(0x23);

        // push hl
        addByte(0xE5);

        // jp FILESPEC        ; in e=string size, hl=string start; out d = device code, FILNAM
        addCmd(0xC3, def_FILESPEC);

        mark->symbol->address = code_pointer;   // return point after FILESPEC

        // ld a, (TEMP+1)
        addCmd(0x3A, def_TEMP+1);
        // ld e, a                ; file mode
        addByte(0x5F);

        // ld a, (TEMP)           ; io number
        addCmd(0x3A, def_TEMP);

        // ld hl, fake empty line
        addCmd(0x21, def_ENDPRG);

        // call OPEN     ; in: a = i/o number, e = filemode, d = devicecode
        addCmd(0xCD, def_OPEN);

        addDisableBasicSlot();

    } else {

        syntax_error("Empty OPEN statement");

    }

}

void Compiler::cmd_close() {
    ActionNode *action;
    unsigned int i, t = current_action->actions.size();
    int result_subtype;

    if(t) {

        for(i = 0; i < t; i++) {

            action = current_action->actions[i];
            result_subtype = evalExpression(action);
            addCast(result_subtype, Lexeme::subtype_numeric);

            // ld a, l
            addByte(0x7D);
            // push af
            addByte(0xF5);

            addEnableBasicSlot();

            // pop af
            addByte(0xF1);
            // ld hl, fake empty line
            addCmd(0x21, def_ENDPRG);

            // call CLOSE    ; in: a = i/o number
            addCmd(0xCD, def_CLOSE);

            addDisableBasicSlot();
        }

    } else {
        addEnableBasicSlot();

        // ld hl, fake empty line
        addCmd(0x21, def_ENDPRG);

        // call ClOSE ALL
        addCmd(0xCD, def_CLOSE_ALL);

        addDisableBasicSlot();
    }

}

void Compiler::cmd_def() {
    ActionNode *action, *subaction;
    Lexeme *lexeme;
    unsigned int t = current_action->actions.size();
    int result_subtype;

    if(t) {

        action = current_action->actions[0];
        lexeme = action->lexeme;

        if(lexeme->value == "USR") {

            t = action->actions.size();

            if(t == 2) {

                has_defusr = true;

                subaction = action->actions[0];
                result_subtype = evalExpression(subaction);
                addCast(result_subtype, Lexeme::subtype_numeric);

                // push hl
                addByte(0xE5);

                subaction = action->actions[1];
                result_subtype = evalExpression(subaction);
                addCast(result_subtype, Lexeme::subtype_numeric);

                // ex de,hl
                addByte(0xEB);

                // pop hl
                addByte(0xE1);
                // ld bc, USRTAB
                addCmd(0x01, def_USRTAB);
                // add hl,hl
                addByte(0x29);
                // add hl,bc
                addByte(0x09);
                // ld (hl), e
                addByte(0x73);
                // inc hl
                addByte(0x23);
                // ld (hl), d
                addByte(0x72);

            } else {
                syntax_error("Wrong DEF USR parameters count");
            }

        }

    }

}

void Compiler::cmd_cmd() {
    ActionNode *action, *sub_action1, *sub_action2, *sub_action3;
    Lexeme *lexeme;
    unsigned int i, t = current_action->actions.size();
    int result_subtype;

    if(t) {

        for(i = 0; i < t; i++) {

            action = current_action->actions[i];
            lexeme = action->lexeme;

            if(lexeme->value == "RUNASM") {

                if(action->actions.size() == 1) {

                    sub_action1 = action->actions[0];
                    result_subtype = evalExpression(sub_action1);
                    addCast(result_subtype, Lexeme::subtype_numeric);

                    // ld (DAC), hl
                    addCmd(0x22, def_DAC);
                    // call cmd_runasm
                    addCmd(0xCD, def_cmd_runasm);

                } else {
                    syntax_error("CMD RUNASM syntax error");
                }

            } else if(lexeme->value == "RUNBAS") {

                syntax_error("CMD RUNBAS not implemented yet");

            } else if(lexeme->value == "WRTVRAM") {

                if(action->actions.size() == 2) {

                    sub_action1 = action->actions[0];
                    result_subtype = evalExpression(sub_action1);
                    addCast(result_subtype, Lexeme::subtype_numeric);

                    // ld (DAC), hl
                    addCmd(0x22, def_DAC);

                    sub_action2 = action->actions[1];
                    result_subtype = evalExpression(sub_action2);
                    addCast(result_subtype, Lexeme::subtype_numeric);

                    // ld (ARG), hl
                    addCmd(0x22, def_ARG);

                    // call cmd_runasm
                    addCmd(0xCD, def_cmd_wrtvram);

                } else {
                    syntax_error("CMD WRTVRAM syntax error");
                }

            } else if(lexeme->value == "WRTFNT") {

                if(action->actions.size() == 1) {

                    sub_action1 = action->actions[0];
                    result_subtype = evalExpression(sub_action1);
                    addCast(result_subtype, Lexeme::subtype_numeric);

                    // ld (DAC), hl
                    addCmd(0x22, def_DAC);
                    // call cmd_wrtfnt
                    addCmd(0xCD, def_cmd_wrtfnt);

                } else {
                    syntax_error("CMD WRTFNT syntax error");
                }

            } else if(lexeme->value == "WRTCHR") {

                if(action->actions.size() == 1) {

                    sub_action1 = action->actions[0];
                    result_subtype = evalExpression(sub_action1);
                    addCast(result_subtype, Lexeme::subtype_numeric);

                    // ld (DAC), hl
                    addCmd(0x22, def_DAC);
                    // call cmd_wrtchr
                    addCmd(0xCD, def_cmd_wrtchr);

                } else {
                    syntax_error("CMD WRTCHR syntax error");
                }

            } else if(lexeme->value == "WRTCLR") {

                if(action->actions.size() == 1) {

                    sub_action1 = action->actions[0];
                    result_subtype = evalExpression(sub_action1);
                    addCast(result_subtype, Lexeme::subtype_numeric);

                    // ld (DAC), hl
                    addCmd(0x22, def_DAC);
                    // call cmd_wrtclr
                    addCmd(0xCD, def_cmd_wrtclr);

                } else {
                    syntax_error("CMD WRTCLR syntax error");
                }

            } else if(lexeme->value == "WRTSCR") {

                if(action->actions.size() == 1) {

                    sub_action1 = action->actions[0];
                    result_subtype = evalExpression(sub_action1);
                    addCast(result_subtype, Lexeme::subtype_numeric);

                    // ld (DAC), hl
                    addCmd(0x22, def_DAC);
                    // call cmd_wrtscr
                    addCmd(0xCD, def_cmd_wrtscr);

                } else {
                    syntax_error("CMD WRTSCR syntax error");
                }

            } else if(lexeme->value == "WRTSPR") {

                if(action->actions.size() == 1) {

                    sub_action1 = action->actions[0];
                    result_subtype = evalExpression(sub_action1);
                    addCast(result_subtype, Lexeme::subtype_numeric);

                    // ld (DAC), hl
                    addCmd(0x22, def_DAC);
                    // call cmd_wrtspr                    ; tiny sprite loader
                    addCmd(0xCD, def_cmd_wrtspr);

                } else {
                    syntax_error("CMD WRTSPR syntax error");
                }

            } else if(lexeme->value == "WRTSPRPAT") {

                if(action->actions.size() == 1) {

                    sub_action1 = action->actions[0];
                    result_subtype = evalExpression(sub_action1);
                    addCast(result_subtype, Lexeme::subtype_numeric);

                    // ld (DAC), hl
                    addCmd(0x22, def_DAC);
                    // call cmd_wrtsprpat
                    addCmd(0xCD, def_cmd_wrtsprpat);

                } else {
                    syntax_error("CMD WRTSPRPAT syntax error");
                }

            } else if(lexeme->value == "WRTSPRCLR") {

                if(action->actions.size() == 1) {

                    sub_action1 = action->actions[0];
                    result_subtype = evalExpression(sub_action1);
                    addCast(result_subtype, Lexeme::subtype_numeric);

                    // ld (DAC), hl
                    addCmd(0x22, def_DAC);
                    // call cmd_wrtsprclr
                    addCmd(0xCD, def_cmd_wrtsprclr);

                } else {
                    syntax_error("CMD WRTSPRCLR syntax error");
                }

            } else if(lexeme->value == "WRTSPRATR") {

                if(action->actions.size() == 1) {

                    sub_action1 = action->actions[0];
                    result_subtype = evalExpression(sub_action1);
                    addCast(result_subtype, Lexeme::subtype_numeric);

                    // ld (DAC), hl
                    addCmd(0x22, def_DAC);
                    // call cmd_wrtspratr
                    addCmd(0xCD, def_cmd_wrtspratr);

                } else {
                    syntax_error("CMD WRTSPRATR syntax error");
                }

            } else if(lexeme->value == "RAMTOVRAM") {

                if(action->actions.size() == 3) {

                    sub_action1 = action->actions[0];
                    result_subtype = evalExpression(sub_action1);
                    addCast(result_subtype, Lexeme::subtype_numeric);

                    // ld (DAC), hl
                    addCmd(0x22, def_DAC);

                    sub_action2 = action->actions[1];
                    result_subtype = evalExpression(sub_action2);
                    addCast(result_subtype, Lexeme::subtype_numeric);

                    // ld (ARG), hl
                    addCmd(0x22, def_ARG);

                    sub_action3 = action->actions[2];
                    result_subtype = evalExpression(sub_action3);
                    addCast(result_subtype, Lexeme::subtype_numeric);

                    // ld (ARG2), hl
                    addCmd(0x22, def_ARG2);

                    // call cmd_ramtovram
                    addCmd(0xCD, def_cmd_ramtovram);

                } else {
                    syntax_error("CMD RAMTOVRAM syntax error");
                }

            } else if(lexeme->value == "VRAMTORAM") {

                if(action->actions.size() == 3) {

                    sub_action1 = action->actions[0];
                    result_subtype = evalExpression(sub_action1);
                    addCast(result_subtype, Lexeme::subtype_numeric);

                    // ld (DAC), hl
                    addCmd(0x22, def_DAC);

                    sub_action2 = action->actions[1];
                    result_subtype = evalExpression(sub_action2);
                    addCast(result_subtype, Lexeme::subtype_numeric);

                    // ld (ARG), hl
                    addCmd(0x22, def_ARG);

                    sub_action3 = action->actions[2];
                    result_subtype = evalExpression(sub_action3);
                    addCast(result_subtype, Lexeme::subtype_numeric);

                    // ld (ARG2), hl
                    addCmd(0x22, def_ARG2);

                    // call cmd_vramtoram
                    addCmd(0xCD, def_cmd_vramtoram);

                } else {
                    syntax_error("CMD VRAMTORAM syntax error");
                }

            } else if(lexeme->value == "DISSCR") {

                // call cmd_disscr
                addCmd(0xCD, def_cmd_disscr);

            } else if(lexeme->value == "ENASCR") {

                // call cmd_enascr
                addCmd(0xCD, def_cmd_enascr);

            } else if(lexeme->value == "KEYCLKOFF") {

                // call cmd_keyclkoff
                addCmd(0xCD, def_cmd_keyclkoff);

            } else if(lexeme->value == "MUTE") {

                // call cmd_mute
                addCmd(0xCD, def_cmd_mute);

            } else if(lexeme->value == "PLAY") {

                if(action->actions.size() == 1) {

                    sub_action1 = action->actions[0];
                    result_subtype = evalExpression(sub_action1);
                    addCast(result_subtype, Lexeme::subtype_numeric);

                    // ld (DAC), hl
                    addCmd(0x22, def_DAC);

                    // xor a
                    addByte(0xAF);
                    // ld (ARG), a
                    addCmd(0x32, def_ARG);

                    // call cmd_play
                    addCmd(0xCD, def_cmd_play);

                } else if(action->actions.size() == 2) {

                    sub_action1 = action->actions[0];
                    result_subtype = evalExpression(sub_action1);
                    addCast(result_subtype, Lexeme::subtype_numeric);

                    // ld (DAC), hl
                    addCmd(0x22, def_DAC);

                    sub_action2 = action->actions[1];
                    result_subtype = evalExpression(sub_action2);
                    addCast(result_subtype, Lexeme::subtype_numeric);

                    // ld (ARG), hl
                    addCmd(0x22, def_ARG);

                    // call cmd_play
                    addCmd(0xCD, def_cmd_play);

                } else {
                    syntax_error("CMD PLAY syntax error");
                }

            } else if(lexeme->value == "DRAW") {

                if(action->actions.size() == 1) {

                    sub_action1 = action->actions[0];
                    result_subtype = evalExpression(sub_action1);
                    addCast(result_subtype, Lexeme::subtype_numeric);

                    // ld (DAC), hl
                    addCmd(0x22, def_DAC);
                    // call cmd_draw
                    addCmd(0xCD, def_cmd_draw);

                } else {
                    syntax_error("CMD DRAW syntax error");
                }

            } else if(lexeme->value == "PT3LOAD") {

                pt3 = true;
                syntax_error("Arkos Tracker not permitted with PT3 player");

            } else if(lexeme->value == "PT3PLAY") {

                pt3 = true;
                syntax_error("Arkos Tracker not permitted with PT3 player");

            } else if(lexeme->value == "PT3MUTE") {

                pt3 = true;
                syntax_error("Arkos Tracker not permitted with PT3 player");

            } else if(lexeme->value == "PT3LOOP") {

                pt3 = true;
                syntax_error("Arkos Tracker not permitted with PT3 player");

            } else if(lexeme->value == "PT3REPLAY") {

                pt3 = true;
                syntax_error("Arkos Tracker not permitted with PT3 player");

            } else if(lexeme->value == "PLYLOAD") {

                akm = true;

                if(action->actions.size()) {

                    sub_action1 = action->actions[0];
                    result_subtype = evalExpression(sub_action1);
                    addCast(result_subtype, Lexeme::subtype_numeric);

                    // ld (DAC), hl
                    addCmd(0x22, def_DAC);

                    if(action->actions.size() > 1) {
                        sub_action2 = action->actions[1];
                        result_subtype = evalExpression(sub_action2);
                        addCast(result_subtype, Lexeme::subtype_numeric);
                    }

                    // ld (ARG), hl
                    addCmd(0x22, def_ARG);

                } else {
                    syntax_error("CMD PLYLOAD syntax error");
                }

                // call cmd_plyload
                addCmd(0xCD, def_cmd_plyload);

            } else if(lexeme->value == "PLYSONG") {

                akm = true;

                if(action->actions.size() == 1) {

                    sub_action1 = action->actions[0];
                    result_subtype = evalExpression(sub_action1);
                    addCast(result_subtype, Lexeme::subtype_numeric);

                    // ld (DAC), hl
                    addCmd(0x22, def_DAC);

                } else {
                    syntax_error("CMD PLYSONG syntax error");
                }

                // call cmd_plysong
                addCmd(0xCD, def_cmd_plysong);

            } else if(lexeme->value == "PLYPLAY") {

                akm = true;

                // call cmd_plyplay
                addCmd(0xCD, def_cmd_plyplay);

            } else if(lexeme->value == "PLYMUTE") {

                akm = true;

                // call cmd_akmmute
                addCmd(0xCD, def_cmd_plymute);

            } else if(lexeme->value == "PLYLOOP") {

                akm = true;

                if(action->actions.size() == 1) {

                    sub_action1 = action->actions[0];
                    result_subtype = evalExpression(sub_action1);
                    addCast(result_subtype, Lexeme::subtype_numeric);

                    // ld (DAC), hl
                    addCmd(0x22, def_DAC);
                    // call cmd_plyloop
                    addCmd(0xCD, def_cmd_plyloop);

                } else {
                    syntax_error("CMD PLYLOOP syntax error");
                }

            } else if(lexeme->value == "PLYREPLAY") {

                akm = true;

                // call cmd_plyreplay
                addCmd(0xCD, def_cmd_plyreplay);

            } else if(lexeme->value == "PLYSOUND") {

                if(action->actions.size()) {

                    sub_action1 = action->actions[0];
                    result_subtype = evalExpression(sub_action1);
                    addCast(result_subtype, Lexeme::subtype_numeric);

                    // ld (DAC), hl
                    addCmd(0x22, def_DAC);

                    if(action->actions.size() > 1) {
                        sub_action2 = action->actions[1];
                        result_subtype = evalExpression(sub_action2);
                        addCast(result_subtype, Lexeme::subtype_numeric);
                    } else {
                        // ld hl, 0
                        addCmd(0x21, 0x0000);
                    }

                    // ld (ARG), hl
                    addCmd(0x22, def_ARG);

                    if(action->actions.size() > 2) {
                        sub_action2 = action->actions[1];
                        result_subtype = evalExpression(sub_action2);
                        addCast(result_subtype, Lexeme::subtype_numeric);
                    } else if(action->actions.size() > 1) {
                        // ld hl, 0
                        addCmd(0x21, 0x0000);
                    }

                    // ld (ARG+2), hl
                    addCmd(0x22, def_ARG+2);

                } else {
                    syntax_error("CMD PLYSOUND syntax error");
                }

                // call cmd_plysound
                addCmd(0xCD, def_cmd_plysound);

            } else if(lexeme->value == "SETFNT") {

                font = true;

                if(action->actions.size() == 1) {

                    sub_action1 = action->actions[0];
                    result_subtype = evalExpression(sub_action1);
                    addCast(result_subtype, Lexeme::subtype_numeric);

                    // ld (DAC), hl
                    addCmd(0x22, def_DAC);

                    // ld a, 0xff                ; it means all screen banks
                    addWord(0x3E, 0xFF);
                    // ld (ARG), a
                    addCmd(0x32, def_ARG);

                    // call cmd_setfnt
                    addCmd(0xCD, def_cmd_setfnt);

                } else if(action->actions.size() == 2) {

                    sub_action1 = action->actions[0];
                    result_subtype = evalExpression(sub_action1);
                    addCast(result_subtype, Lexeme::subtype_numeric);

                    // ld (DAC), hl
                    addCmd(0x22, def_DAC);

                    sub_action2 = action->actions[1];
                    result_subtype = evalExpression(sub_action2);
                    addCast(result_subtype, Lexeme::subtype_numeric);

                    // ld (ARG), hl             ; screen font bank number
                    addCmd(0x22, def_ARG);

                    // call cmd_setfnt
                    addCmd(0xCD, def_cmd_setfnt);

                } else {
                    syntax_error("CMD SETFNT syntax error");
                }

            } else if(lexeme->value == "UPDFNTCLR") {

                // call cmd_disscr
                addCmd(0xCD, def_cmd_updfntclr);


            } else if(lexeme->value == "CLRSCR") {

                // call cmd_clrscr
                addCmd(0xCD, def_cmd_clrscr);

            } else if(lexeme->value == "RAMTORAM") {

                if(action->actions.size() == 3) {

                    sub_action1 = action->actions[0];
                    result_subtype = evalExpression(sub_action1);
                    addCast(result_subtype, Lexeme::subtype_numeric);

                    // ld (DAC), hl
                    addCmd(0x22, def_DAC);

                    sub_action2 = action->actions[1];
                    result_subtype = evalExpression(sub_action2);
                    addCast(result_subtype, Lexeme::subtype_numeric);

                    // ld (ARG), hl
                    addCmd(0x22, def_ARG);

                    sub_action3 = action->actions[2];
                    result_subtype = evalExpression(sub_action3);
                    addCast(result_subtype, Lexeme::subtype_numeric);

                    // ld (ARG2), hl
                    addCmd(0x22, def_ARG2);

                    // call cmd_ramtoram
                    addCmd(0xCD, def_cmd_ramtoram);

                } else {
                    syntax_error("CMD RAMTORAM syntax error");
                }

            } else if(lexeme->value == "CLRKEY") {

                // call cmd_clrkey
                addCmd(0xCD, def_cmd_clrkey);

            } else if(lexeme->value == "CLIP") {

                if(action->actions.size() == 1) {

                    sub_action1 = action->actions[0];
                    result_subtype = evalExpression(sub_action1);
                    addCast(result_subtype, Lexeme::subtype_numeric);

                    // ld a, l
                    addByte(0x7D);
                    // ld hl, 0xfafc
                    addCmd(0x21, 0xfafc);
                    // res 4, (hl)
                    addWord(0xCB, 0xA6);
                    // bit 0, a
                    addWord(0xCB, 0x47);
                    // jr z, +3
                    addWord(0x28, 0x02);
                    //   set 4, (hl)
                    addWord(0xCB, 0xE6);

                } else {
                    syntax_error("CMD CLIP syntax error");
                }

            } else if(lexeme->value == "TURBO") {

                if(action->actions.size() == 1) {

                    sub_action1 = action->actions[0];
                    result_subtype = evalExpression(sub_action1);
                    addCast(result_subtype, Lexeme::subtype_numeric);

                    // ld (DAC), hl
                    addCmd(0x22, def_DAC);
                    // call cmd_turbo
                    addCmd(0xCD, def_cmd_turbo);

                } else {
                    syntax_error("CMD TURBO syntax error");
                }

            } else if(lexeme->value == "RESTORE") {

                if(megaROM) {
                    syntax_error("CMD RESTORE not supported yet with MegaROM option");
                } else {
                    if(action->actions.size() == 1) {

                        sub_action1 = action->actions[0];
                        result_subtype = evalExpression(sub_action1);
                        addCast(result_subtype, Lexeme::subtype_numeric);

                        // ld (DAC), hl
                        addCmd(0x22, def_DAC);

                        // call cmd_restore
                        addCmd(0xCD, def_cmd_restore);

                    } else {
                        syntax_error("CMD RESTORE syntax error");
                    }
                }

            } else {
                syntax_error("CMD statement invalid");
                return;

            }

        }

    } else {
        syntax_error("CMD with empty parameters");
    }

}

void Compiler::addEnableBasicSlot() {
    if(megaROM) {
        // ld a, (EXPTBL)
        addCmd(0x3A, def_EXPTBL);
        // ld h,040h        ; <--- enable jump to here
        addWord(0x26, 0x40);
        // call ENASLT		; Select the ROM on page 4000h
        addCmd(0xCD, def_ENASLT);
        // ei
        addByte(0xFB);
    } else {
        // call enable basic slot function
        if(enable_basic_mark)
            addFix(enable_basic_mark->symbol);
        else
            enable_basic_mark = addMark();
        addCmd(0xCD, 0x0000);
    }
}

void Compiler::addDisableBasicSlot() {
    if(megaROM) {
        // ld a, (SLTSTR)
        addCmd(0x3A, def_SLTSTR);
        // ld h,040h        ; <--- enable jump to here
        addWord(0x26, 0x40);
        // call ENASLT		; Select the ROM on page 4000h
        addCmd(0xCD, def_ENASLT);
        // ei
        addByte(0xFB);
    } else {
        // call disable basic slot function
        if(disable_basic_mark)
            addFix(disable_basic_mark->symbol);
        else
            disable_basic_mark = addMark();
        addCmd(0xCD, 0x0000);
    }
}

void Compiler::func_symbols() {

    // IO REDIRECT FUNCTION
    if(io_redirect_mark) {
        io_redirect_mark->symbol->address = code_pointer;
        // ld a, l  ; io number
        addByte(0x7D);
        // ld hl, fake empty line
        addCmd(0x21, def_ENDPRG);
        // ld ix, IOREDIRECT     ; in: a = i/o number
        addByte(0xDD);
        addCmd(0x21, def_IOREDIRECT);
        // call CALBAS
        addCmd(0xCD, def_CALBAS);
        // ei
        addByte(0xFB);
        // ret
        addByte(0xC9);
    }

    // IO SCREEN FUNCTION
    if(io_screen_mark) {
        io_screen_mark->symbol->address = code_pointer;
        // ld hl, fake empty line
        addCmd(0x21, def_ENDPRG);
        // ld ix, IOTOSCREEN
        addByte(0xDD);
        addCmd(0x21, def_IOTOSCREEN);
        // call CALBAS
        addCmd(0xCD, def_CALBAS);
        // ei
        addByte(0xFB);
        // ret
        addByte(0xC9);
    }

    // DRAW STATEMENT - in: hl (pointer to string)
    if(draw_mark) {
        draw_mark->symbol->address = code_pointer;
        // ld a, (SCRMOD)
        addCmd(0x3A, def_SCRMOD);
        // cp 2
        addWord(0xFE, 0x02);
        // ret c                    ; return if text mode
        addByte(0xD8);

        // push hl
        addByte(0xE5);

        addEnableBasicSlot();

        // pop hl
        addByte(0xE1);

        // xor a
        addByte(0xAF);
        // ld bc, disable basic slot
        if(disable_basic_mark)
            addFix(disable_basic_mark->symbol);
        else
            disable_basic_mark = addMark();
        addCmd(0x01, 0x0000);
        // push bc
        addByte(0xC5);
        // ld bc, 0
        addCmd(0x01, 0x0000);
        // push bc
        addByte(0xC5);
        // push bc
        addByte(0xC5);
        // push bc
        addByte(0xC5);
        // ld de, 0x5D83
        addCmd(0x11, 0x5D83);
        // ld (0xFCBB), a   ; DRWFLG
        addCmd(0x32, 0xFCBB);
        // ld (0xF958), a   ; MCLFLG
        addCmd(0x32, 0xF958);
        // ld (0xF956), de  ; MCLTAB
        addByte(0xED);
        addCmd(0x53, 0xF956);
        // ld a, (hl)
        addByte(0x7E);
        // inc hl
        addByte(0x23);
        // ld (0xFB3B), a   ; MCLLEN
        addCmd(0x32, 0xFB3B);
        // jp 0x5691    ; DRAW subroutine = 0x568C+5  (main routine hook address = 0x39A8)
        addCmd(0xC3, 0x5691);
    }

    // ENABLE BASIC SLOT FUNCTION
    if(enable_basic_mark) {
        enable_basic_mark->symbol->address = code_pointer;
        // ld a, (EXPTBL)
        addCmd(0x3A, def_EXPTBL);
        // jr $+4            ; jump to disable code
        addWord(0x18, 0x03);
    }

    // DISABLE BASIC SLOT FUNCTION
    if(disable_basic_mark) {
        disable_basic_mark->symbol->address = code_pointer;
        // ld a, (SLTSTR)
        addCmd(0x3A, def_SLTSTR);
        // ld h,040h        ; <--- enable jump to here
        addWord(0x26, 0x40);
        // call ENASLT		; Select the ROM on page 4000h
        addCmd(0xCD, def_ENASLT);
        // ei
        addByte(0xFB);
        // ret
        addByte(0xC9);
    }

}

//-------------------------------------------------------------------------------------------

void Compiler::syntax_error() {
    syntax_error("Syntax error");
}

void Compiler::syntax_error(string msg) {
    compiled = false;
    error_message = msg;
    if(current_tag)
        error_message += " (line=" + current_tag->name + ")";
}

//-------------------------------------------------------------------------------------------

FileNode::~FileNode() {
    close();
}

bool FileNode::open() {
    this->clear();
    handle = fopen(name.c_str(), "rb");
    return handle;
}

bool FileNode::eof() {
    if(handle) {
        return feof(handle);
    } else
        return true;
}

int FileNode::read(unsigned char *data, int max_length) {
    bytes = 0;
    memset(data, 0, max_length);
    if(handle) {
        bytes = fread(data, 1, max_length, handle);
        length += bytes;
    }
    return bytes;
}

int FileNode::read() {
    buffer = buf_plain;
    if(read(buffer, 200)) {
        if(packed) {
            memset(buf_packed, 0, 1024);
            bytes = pletter.pack(buffer, 200, &buf_packed[0]);
            buffer = buf_packed;

            packed_length += bytes;
        }
        blocks ++;
    }
    return bytes;
}

int FileNode::readAsLexeme() {

    if(read(buf_plain, 200)) {

        readAsLexeme(&buf_plain[0], bytes);

    }

    return bytes;
}

int FileNode::readAsLexeme(unsigned char *data, int data_length) {
    if(packed) {
        memset(buf_packed, 0, 1024);
        bytes = pletter.pack(data, data_length, &buf_packed[0]);

        packed_length += bytes;

        if(bytes == 0 || bytes > 255)
            return bytes;
    }

    current_lexeme = new Lexeme(Lexeme::type_literal, Lexeme::subtype_binary_data, "FB_" + name + "_" + to_string(blocks), "");
    current_lexeme->value.resize(bytes);

    s = (unsigned char*) current_lexeme->value.data();

    if(packed) {
        memcpy(s, &buf_packed[0], bytes);
    } else {
        memcpy(s, data, data_length);
    }

    if(first) {
        first_lexeme = current_lexeme;
        memcpy(&file_header[0], data, 255);
        first = false;
    }

    blocks ++;

    return bytes;
}

void FileNode::close() {
    if(handle) {
        fclose(handle);
        handle = 0;
    }
}

void FileNode::clear() {
    handle = 0;
    length = 0;
    first = true;
    packed_length = 0;
    blocks = 0;
    first_lexeme = 0;
    current_lexeme = 0;
}

bool FileNode::create() {
    clear();
    handle = fopen(name.c_str(), "wb");
    return handle;
}

void FileNode::write(unsigned char *data, int data_length) {
    fwrite(data, 1, data_length, handle);
    length += data_length;
}

//----------------------------------------------------------------------------------------------

void FileNode::stripQuotes(string text, char *buf) {
    int tt;
    char *s;

    // strips quotes from text
    tt = text.size();
    s = (char *) text.c_str();
    if(s[0] == '"') {
        s++;
        tt --;
    }
    strcpy(buf, s);
    if(buf[tt-1] == '"') {
        buf[tt-1] = 0;
        tt --;
    }
}

string FileNode::getFileExt() {
    char buf[255];
    getFileExt((char *)name.c_str(), &buf[0]);
    return string(buf);
}

void FileNode::getFileExt(char *filename, char *buf) {
    char *s;
    int i,t;
    t = strlen(filename);
    s = filename;
    s += t;
    buf[0] = 0;
    while(t) {
        if(s[0] == '.') {
            strcpy(buf, s);
            t = strlen(buf);
            for(i=0; i<t; i++)
                buf[i] = toupper(buf[i]);
            break;
        }
        s--;
        t--;
    }
}

void FileNode::fixAKM(unsigned char *data, int address, int length) {
}

void FileNode::fixAKX(unsigned char *data, int address, int length) {
    int t = length;
    int i = 0, current, start = 0x0100;
    bool first = true;

    // loop the effects list

    while( i < t ) {

        current = data[i] | (data[i+1]<<8);
        if(current) {
            if(first) {
                t = current - start;
                first = false;
            }

            current = current - start + address;

            data[i] = current & 0xFF;
            data[i+1] = (current >> 8) & 0xFF;
        }

        i += 2;

    }


}

//----------------------------------------------------------------------------------------------

bool FileNode::writeToFile(char *filename, unsigned char *data, int data_length) {

    name = string(filename);

    if(!create()) {
        return false;
    }

    write(data, data_length);

    close();

    return true;
}

int FileNode::readFromFile(char *filename, unsigned char *data, int maxlen) {
    FILE *file;
    int total_bytes = 0;

    if ((file = fopen(filename, "rb"))) {
        total_bytes = fread(data, 1, maxlen, file);
        fclose(file);
    }

    length = total_bytes;

    return total_bytes;
}

int FileNode::ParseTinySpriteFile(char *filename, unsigned char *data, int maxlen) {
    FILE *file;
    char line[255];
    int len, size_read, state;
    int sprite_type=0;
    int sprite_x=0, sprite_y=0, sprite_c=0;
    int sprite_k, sprite_w, sprite_n;
    unsigned char sprite_data[16][4][8];
    unsigned char sprite_attr[16*255];
    unsigned char sprite_color[16];
    unsigned char *sprite_count = &sprite_color[0];
    bool ok = false, found;

    *sprite_count = 0;

    // for msx1 tiny sprite format file:
    //   [type=0][sprite count][sprites patterns=collection of 32 bytes][sprites colors=1 byte per sprite]
    // for msx2 format file:
    //   [type=1][sprite count][sprites patterns=collection of 32 bytes][sprites colors=16 bytes per sprite]

    if ((file = fopen(filename, "r"))) {

        size_read = 0;
        state = 0;
        memset(sprite_attr, 0, 64);
        ok = true;

        while ( fgets(line, sizeof(line), file) ) {

            len = strlen(line);

            // strip CR LF of the line
            while(len) {
                if(line[len - 1] < 0x20) {
                    line[len - 1] = 0;
                    len --;
                } else
                    break;
            }

            if(len) {
                switch(state) {
                    // file header
                    case 0 : {
                            if(strcasecmp(line, "!type")==0) {
                                state ++;
                            } else {
                                ok = false;
                            }
                        }
                        break;

                    // sprite type
                    case 1 : {
                            if(strcasecmp(line, "msx1")==0) {
                                sprite_type = 0;  // msx 1 (screen mode <= 3)
                            } else if(strcasecmp(line, "msx2")==0) {
                                sprite_type = 1;  // msx 2 (screen mode >= 4)
                            } else {
                                ok = false;
                                break;
                            }
                            data[0] = sprite_type;    // MSX 1
                            data[1] = 0;
                            sprite_count = &data[1];
                            data += 2;
                            size_read += 2;
                            state ++;
                        }
                        break;

                    // slot number
                    case 2 : {
                            if(line[0]=='#') {
                                sprite_y = 0;
                                memset(sprite_data, 0, 16*4*8);
                                memset(sprite_color, 0, 16);
                                state++;
                            } else {
                                ok = false;
                            }
                        }
                        break;

                    // sprite data
                    case 3 : {
                            for(sprite_x = 0; sprite_x < 16; sprite_x++) {

                                sprite_c = line[sprite_x];

                                if(sprite_c >= '0' && sprite_c <= '9') {
                                    sprite_c -= '0';
                                } else if(sprite_c >= 'A' && sprite_c <= 'F') {
                                    sprite_c -= 'A';
                                    sprite_c += 10;
                                } else if(sprite_c >= 'a' && sprite_c <= 'f') {
                                    sprite_c -= 'a';
                                    sprite_c += 10;
                                } else {
                                    sprite_c = 0;
                                }

                                if(sprite_c) {
                                    if(sprite_y < 8) {
                                        if(sprite_x < 8) {
                                            sprite_k = 0;
                                        } else {
                                            sprite_k = 2;
                                        }
                                    } else {
                                        if(sprite_x < 8) {
                                            sprite_k = 1;
                                        } else {
                                            sprite_k = 3;
                                        }
                                    }
                                    sprite_w = sprite_y % 8;
                                    sprite_n = (1 << (7 - (sprite_x % 8)));
                                    sprite_data[sprite_c][sprite_k][sprite_w] |= sprite_n;
                                    sprite_color[sprite_c] = 1;
                                }

                            }

                            sprite_y ++;

                            if(sprite_y >= 16) {
                                if(sprite_type) {
                                    // msx 2
                                    do {
                                        found = false;
                                        for(sprite_y = 0; sprite_y < 16; sprite_y++) {
                                            if(sprite_y < 8) {
                                                sprite_k = 0;
                                            } else {
                                                sprite_k = 1;
                                            }
                                            sprite_w = sprite_y % 8;
                                            sprite_n = sprite_k + 2;
                                            sprite_data[0][sprite_k][sprite_w] = 0;
                                            sprite_data[0][sprite_n][sprite_w] = 0;
                                            sprite_color[sprite_y] = 0;
                                            for(sprite_c = 1; sprite_c < 16; sprite_c++) {
                                                if(sprite_data[sprite_c][sprite_k][sprite_w] | sprite_data[sprite_c][sprite_n][sprite_w]) {
                                                    sprite_data[0][sprite_k][sprite_w] = sprite_data[sprite_c][sprite_k][sprite_w];
                                                    sprite_data[0][sprite_n][sprite_w] = sprite_data[sprite_c][sprite_n][sprite_w];
                                                    sprite_data[sprite_c][sprite_k][sprite_w] = 0;
                                                    sprite_data[sprite_c][sprite_n][sprite_w] = 0;
                                                    sprite_color[sprite_y] = sprite_c;
                                                    found = true;
                                                    break;
                                                }
                                            }
                                        }
                                        if(found) {
                                            sprite_n = *sprite_count * 16;
                                            memcpy(&sprite_attr[sprite_n], &sprite_color[0], 16);
                                            memcpy(data, sprite_data[0], 32);
                                            *sprite_count += 1;
                                            data += 32;
                                            size_read += 32;
                                        }
                                    } while(found);
                                } else {
                                    // msx 1
                                    for(sprite_c = 0; sprite_c < 16; sprite_c++) {
                                        if(sprite_color[sprite_c]) {
                                            sprite_attr[*sprite_count] = sprite_c;
                                            memcpy(data, sprite_data[sprite_c], 32);
                                            *sprite_count += 1;
                                            data += 32;
                                            size_read += 32;
                                        }
                                    }
                                }
                                state = 2;
                            }
                        }
                        break;

                }

                if(!ok)
                    break;

            }

        }

        fclose(file);

        if(ok) {
            if(*sprite_count) {
                if(sprite_type) {
                    // msx 2
                    sprite_n = *sprite_count * 16;
                    memcpy(data, sprite_attr, sprite_n);
                    size_read += sprite_n;
                } else {
                    // msx 1
                    memcpy(data, sprite_attr, *sprite_count);
                    size_read += *sprite_count;
                }
            }
            return size_read;
        } else
            return 0;

    } else {

        return -1;

    }

}

#ifdef Win
int _strcasecmp(const char * str1, const char * str2) {
    int d = 0;
    while(1) {
        int c1 = tolower(*(str1++));
        int c2 = tolower(*(str2++));
        if(((d = c1 - c2) != 0) || (c2 == '\0')) {
            break;
        }
    }
    return d;
}
#endif
