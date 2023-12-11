// lexer.h
#ifndef LEXER_H
#define LEXER_H


typedef enum TokenType{
    TOKEN_INT_DECL,
    TOKEN_DOUBLE_DECL,
    TOKEN_INT_LITERAL,
    TOKEN_DOUBLE_LITERAL,
    TOKEN_IDENTIFIER,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_MULTI,
    TOKEN_DIVISION,
    TOKEN_ASSIGN,
    TOKEN_LESS,
    TOKEN_GREATER,
    TOKEN_EQUAL,
    TOKEN_LESS_OR_EQUAL,
    TOKEN_GREATER_OR_EQUAL,
    TOKEN_OPEN_PAREN,
    TOKEN_CLOSE_PAREN,
    TOKEN_OPEN_BRACE,
    TOKEN_CLOSE_BRACE,
    TOKEN_PRINT,
    TOKEN_INPUT,
    TOKEN_WHILE,
    TOKEN_CONDITION,
    TOKEN_THEN,
    TOKEN_ELSE,
    TOKEN_IF,
    TOKEN_EOF,
    TOKEN_NEW_LINE,
    TOKEN_ERROR
} TokenType;

// Declare the function that you want to make available to other files
void performLexicalAnalysis(const char *inputFilename, const char *outputFilename);

#endif // LEXER_H