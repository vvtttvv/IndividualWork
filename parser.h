#ifndef PARSER_H
#define PARSER_H
#include "cJSON.h"
#include "lexer.h"
//#include "cJSON.c"



typedef struct Node {
    TokenType type;
    char lexeme[50];
    int intValue;
    double doubleValue;
    struct Node* left;
    struct Node* right;
} Node;

Node* Parser();

#endif
