#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"

char* my_strndup(const char *s, size_t n) {
    size_t len = strnlen(s, n);
    char *new_str = (char*)malloc(len + 1);
    if (new_str == NULL) {
        return NULL;
    }
    new_str[len] = '\0';
    return (char*)memcpy(new_str, s, len);
}

// Token types


const char *tokenTypeToString(TokenType type) {
    switch (type) {
        case TOKEN_INT_DECL: return "TOKEN_INT_DECL";
        case TOKEN_DOUBLE_DECL: return "TOKEN_DOUBLE_DECL";
        case TOKEN_INT_LITERAL: return "TOKEN_INT_LITERAL";
        case TOKEN_DOUBLE_LITERAL: return "TOKEN_DOUBLE_LITERAL";
        case TOKEN_IDENTIFIER: return "TOKEN_IDENTIFIER";
        case TOKEN_PLUS: return "TOKEN_PLUS";
        case TOKEN_MINUS: return "TOKEN_MINUS";
        case TOKEN_MULTI: return "TOKEN_MULTI";
        case TOKEN_DIVISION: return "TOKEN_DIVISION";
        case TOKEN_ASSIGN: return "TOKEN_ASSIGN";
        case TOKEN_LESS: return "TOKEN_LESS";
        case TOKEN_GREATER: return "TOKEN_GREATER";
        case TOKEN_EQUAL: return "TOKEN_EQUAL";
        case TOKEN_LESS_OR_EQUAL: return "TOKEN_LESS_OR_EQUAL";
        case TOKEN_GREATER_OR_EQUAL: return "TOKEN_GREATER_OR_EQUAL";
        case TOKEN_OPEN_PAREN: return "TOKEN_OPEN_PAREN";
        case TOKEN_CLOSE_PAREN: return "TOKEN_CLOSE_PAREN";
        case TOKEN_OPEN_BRACE: return "TOKEN_OPEN_BRACE";
        case TOKEN_CLOSE_BRACE: return "TOKEN_CLOSE_BRACE";
        case TOKEN_PRINT: return "TOKEN_PRINT";
        case TOKEN_INPUT: return "TOKEN_INPUT";
        case TOKEN_WHILE: return "TOKEN_WHILE";
        case TOKEN_ELSE: return "TOKEN_ELSE";
        case TOKEN_IF: return "TOKEN_IF";
        case TOKEN_EOF: return "TOKEN_EOF";
        case TOKEN_ERROR: return "TOKEN_ERROR";
        case TOKEN_NEW_LINE: return "TOKEN_NEW_LINE";
        default: return "UNKNOWN_TOKEN";
    }
}


// Token structure
typedef struct {
    TokenType type;
    char *lexeme;
} Token;

// TokenList structure
typedef struct {
    Token *tokens;
    size_t size;
    size_t capacity;
} TokenList;

typedef struct {
    const char *keyword;
    TokenType type;
} Keyword;

const Keyword keywords[] = {
        {"print", TOKEN_PRINT},
        {"input", TOKEN_INPUT},
        {"while", TOKEN_WHILE},
        {"if", TOKEN_IF},
        {"or", TOKEN_ELSE}
};
const int numKeywords = sizeof(keywords) / sizeof(Keyword);

TokenType keywordLookup(const char *lexeme, size_t length) {
    for (int i = 0; i < numKeywords; i++) {
        if (strncmp(lexeme, keywords[i].keyword, length) == 0 && length == strlen(keywords[i].keyword)) {
            return keywords[i].type;
        }
    }
    return TOKEN_IDENTIFIER; // Not a keyword, so it's an identifier
}

// Add new token to the list
void addToken(TokenList *tokenList, TokenType type, const char *lexeme) {
    if (tokenList->size >= tokenList->capacity) {
        tokenList->capacity = tokenList->capacity < 1 ? 1 : tokenList->capacity * 2;
        tokenList->tokens = (Token *)realloc(tokenList->tokens, tokenList->capacity * sizeof(Token));

    }
    tokenList->tokens[tokenList->size].type = type;
    tokenList->tokens[tokenList->size].lexeme = strdup(lexeme);
    tokenList->size++;
}

void freeTokenList(TokenList *tokenList) {
    for (size_t i = 0; i < tokenList->size; i++) {
        free(tokenList->tokens[i].lexeme);
    }
    free(tokenList->tokens);
}

char *readFileIntoString(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Could not open source file");
        exit(EXIT_FAILURE);
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buffer = (char *)malloc(length + 1);
    if (!buffer) {
        perror("Memory allocation failed");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    fread(buffer, 1, length, file);
    buffer[length] = '\0'; // Null-terminate the string

    fclose(file);
    return buffer;
}

TokenList tokenize(const char *source) {
    TokenList tokenList = {NULL, 0, 0};
    const char *start = source;
    const char *current = source;
    size_t sourceLength = strlen(source);
    int line = 1;

    while (*current != '\0' && (current - source) < sourceLength) {
        if (*current == '\n') {
            addToken(&tokenList, TOKEN_NEW_LINE, "\\n");
            current++;
            continue;
        }

        if (isspace(*current)) {
            current++;
            continue;

        }
        switch (*current) {
            case '#': {
                const char next = *(current + 1);
                if (next == 'i') {
                    addToken(&tokenList, TOKEN_INT_DECL, "#i");
                    current += 2; // Move past the "#i"
                } else if (next == 'd') {
                    addToken(&tokenList, TOKEN_DOUBLE_DECL, "#d");
                    current += 2; // Move past the "#d"
                } else {
                    char errorLexeme[3] = {'#', next, '\0'};
                    addToken(&tokenList, TOKEN_ERROR, errorLexeme);
                    current += 2; // Move past the "#" and the unrecognized character
                    return tokenList;
                }
                break;
            }
            case '=':
                if (*(current + 1) == '=') {
                    addToken(&tokenList, TOKEN_EQUAL, "==");
                    current += 2; // Advance past both '=' characters
                } else {
                    // Check if the next character is a number
                    if (((isdigit(*(current + 1))) || (isdigit(*(current + 2)))) || ((isalpha(*(current + 1))) || (isalpha(*(current + 2))))) {
                        addToken(&tokenList, TOKEN_ASSIGN, "=");
                        current++;
                    } else {
                        // Handle the error case where the character following '=' is not a number
                        fprintf(stderr, "Error: Unexpected character '%c' after '='\n", *(current + 1));
                        // Add additional error handling logic as needed
                        return tokenList;
                    }
                }
                break;

            case '(':
                addToken(&tokenList, TOKEN_OPEN_PAREN, "(");
                current++;
                break;
            case ')':
                addToken(&tokenList, TOKEN_CLOSE_PAREN, ")");
                current++;
                break;
            case '{':
                addToken(&tokenList, TOKEN_OPEN_BRACE, "{");
                current++;
                break;
            case '}':
                addToken(&tokenList, TOKEN_CLOSE_BRACE, "}");
                current++;
                break;
            case '+':
                addToken(&tokenList, TOKEN_PLUS, "+");
                current++;
                break;
            case '-':
                addToken(&tokenList, TOKEN_MINUS, "-");
                current++;
                break;
            case '*':
                addToken(&tokenList, TOKEN_MULTI, "*");
                current++;
                break;
            case '/':
                addToken(&tokenList, TOKEN_DIVISION, "/");
                current++;
                break;
            case '>':
                addToken(&tokenList, TOKEN_GREATER, ">");
                current++;
                break;
            case '<':
                addToken(&tokenList, TOKEN_LESS, "<");
                current++;
                break;
            default:
                if (isdigit(*current)) {
                    // Integer or double literal
                    start = current;
                    while (isdigit(*current)) {
                        current++; // Continue with the integer part
                    }

                    if (*current == '.') {
                        const char* dot = current;
                        current++; // Skip the dot

                        if (!isdigit(*current)) {
                            // If there's a dot but no digits after it, it's an error
                            addToken(&tokenList, TOKEN_ERROR, my_strndup(dot, 1));
                            return tokenList;
                        } else {
                            // There are digits after the dot, it's a double literal
                            while (isdigit(*current)) current++; // Continue with the fractional part

                            // Check if there is another dot following which would indicate an error
                            if (*current == '.') {
                                addToken(&tokenList, TOKEN_ERROR, my_strndup(start, current - start));
                                current++; // Skip the erroneous dot
                                return tokenList;
                            } else {
                                // It's a valid double literal
                                addToken(&tokenList, TOKEN_DOUBLE_LITERAL, my_strndup(start, current - start));
                            }
                        }
                    } else {
                        // It was just an integer literal
                        addToken(&tokenList, TOKEN_INT_LITERAL, my_strndup(start, current - start));
                    }


                } else if (isalpha(*current) || *current == '_') {
                    // Identifier or keyword
                    start = current;
                    while (isalnum(*current) || *current == '_') current++;
                    size_t length = current - start;
                    char *lexeme = my_strndup(start, length);

                    if (strncmp(lexeme, "end", length) == 0 && length == 3) {
                        free(lexeme);
                        addToken(&tokenList, TOKEN_EOF, "end");
                        return tokenList; // Stop reading further and return
                    }

                    TokenType type = keywordLookup(lexeme, length);
                    addToken(&tokenList, type, lexeme);

                    free(lexeme);
                }
                else {
                    // Unrecognized character
                    char unknown[2] = {*current, '\0'};

                    // Print the unknown token
                    fprintf(stderr, "Error: Unknown token '%s'\n", unknown);

                    addToken(&tokenList, TOKEN_ERROR, unknown);
                    current++;
                    return tokenList;
                }

                break;
        }
    }

    addToken(&tokenList, TOKEN_EOF, "EOF");
    return tokenList;
}


// Function to write the list of tokens to a JSON file
void writeTokensToJson(const TokenList *tokenList, const char *filename) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        perror("Could not open JSON output file");
        return;
    }

    fprintf(file, "{\n  \"tokens\": [\n");
    for (size_t i = 0; i < tokenList->size; i++) {
        fprintf(file, "    {\"type\": \"%s\", \"lexeme\": \"%s\"}",
                tokenTypeToString(tokenList->tokens[i].type),
                tokenList->tokens[i].lexeme);
        if (i < tokenList->size - 1) fprintf(file, ",\n");
    }
    fprintf(file, "\n  ]\n}");

    fclose(file);
}



// Main function where the lexer starts execution
void performLexicalAnalysis(const char *inputFilename, const char *outputFilename) {
    char *source = readFileIntoString(inputFilename);
    TokenList tokenList = tokenize(source);
    writeTokensToJson(&tokenList, outputFilename);
    freeTokenList(&tokenList);
    free(source);
}

