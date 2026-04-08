# Glossary

## General

- **SDD**: Spec Driven Development.

## Class Summary

- **Lexer**: Responsible for lexical analysis of MSX BASIC source code. It reads raw program text and converts it into a sequence of Lexeme objects organized per line (LexerLine), identifying tokens such as keywords, identifiers, operators, literals, and separators;
- **Parser**: Implements a syntax analyzer and AST (Abstract Syntax Tree) builder for MSX BASIC. Processes lexemes into structured actions/statements, validates syntax, manages program structure (lines, statements, expressions), and prepares data for the compiler stage;
- **Compiler**: Responsible for semantic analysis and code generation, acting as a Z80 code builder for the MSX platform. It transforms the parsed syntax tree into executable machine code, manages memory (RAM/code), resolves symbols, applies optimizations, and prepares the final binary structure;
- **Rom**: Handles the construction of the final ROM image. Combines compiled code, kernel, and resources into a valid MSX cartridge format, supporting both standard ROM and MegaROM layouts;
- **SymbolManager**: Manages symbol tables and debug symbol generation. Tracks code and data symbols, and exports them into multiple formats (e.g., assembler, NoICE, OMDS) for debugging and tooling integration;
- **ResourceManager**: Responsible for handling external and embedded resources (e.g., DATA, IDATA, assets). Builds resource maps and integrates them into the final ROM layout;
- **IZ80/Z80**: Acts as a low-level Z80 instruction abstraction layer. Provides helper methods to emit machine instructions, enabling the Compiler to focus on higher-level code generation logic.

## Supporting Data Structures

- **Lexeme**: Represents a token produced during lexical analysis. Encapsulates information such as type, subtype, value, and attributes (e.g., arrays, literals), serving as the fundamental unit consumed by the parser;
- **LexerLine**: Represents a single line of tokenized code, containing an ordered list of Lexeme objects. Provides navigation utilities (e.g., get first/next lexeme), enabling the Parser to iterate through tokens efficiently;
- **SymbolNode**: Represents a symbol entry (variable, label, or reference) with associated address and metadata;
- **CodeNode**: Represents a block of generated code or data, including its address, size, and debug relevance;
- **FixNode**: Handles deferred address resolution (fix-ups) for symbols that are resolved after code generation;
- **TagNode**: Represents line labels or program tags, typically corresponding to BASIC line numbers;
- **ActionNode**: Represents a node in the syntax tree, used by the parser to model statements and expressions.
- **BuildOptions**: Encapsulates compiler and build configuration.

