#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "cJSON.h"
#include "cJSON.c"
#include "lexer.h"
#include "parser.h"



Node* handlePrint(cJSON *tokens, int index, int end);

int findTokenIndex(cJSON *tokens, int start, int end, enum TokenType targetTokenType);

Node* handleIdentifier(cJSON *tokens, int index);

Node* parseTokens(cJSON *tokens, int start, int end, const char* lexeme);

Node* parse(cJSON *json_data_parsed) {
    cJSON *tokens = cJSON_GetObjectItemCaseSensitive(json_data_parsed, "tokens");
    if (cJSON_IsArray(tokens)) {
        int array_size = cJSON_GetArraySize(tokens);
        if (array_size > 0) {
            return parseTokens(tokens, 0, array_size - 1, NULL);
        } else {
            fprintf(stderr, "Error: Empty array of tokens.\n");
            return NULL;
        }
    } else {
        fprintf(stderr, "Error: field 'tokens' isn't an array.\n");
        return NULL;
    }
}

enum TokenType getTokenTypeFromString(const char* typeString) {
    if (strcmp(typeString, "TOKEN_INT_DECL") == 0) {
        return TOKEN_INT_DECL;
    } else if (strcmp(typeString, "TOKEN_DOUBLE_DECL") == 0) {
        return TOKEN_DOUBLE_DECL;
    } else if (strcmp(typeString, "TOKEN_INT_LITERAL") == 0) {
        return TOKEN_INT_LITERAL;
    } else if (strcmp(typeString, "TOKEN_DOUBLE_LITERAL") == 0) {
        return TOKEN_DOUBLE_LITERAL;
    } else if (strcmp(typeString, "TOKEN_IDENTIFIER") == 0) {
        return TOKEN_IDENTIFIER;
    } else if (strcmp(typeString, "TOKEN_PLUS") == 0) {
        return TOKEN_PLUS;
    } else if (strcmp(typeString, "TOKEN_MINUS") == 0) {
        return TOKEN_MINUS;
    } else if (strcmp(typeString, "TOKEN_MULTI") == 0) {
        return TOKEN_MULTI;
    } else if (strcmp(typeString, "TOKEN_DIVISION") == 0) {
        return TOKEN_DIVISION;
    } else if (strcmp(typeString, "TOKEN_ASSIGN") == 0) {
        return TOKEN_ASSIGN;
    } else if (strcmp(typeString, "TOKEN_LESS") == 0) {
        return TOKEN_LESS;
    } else if (strcmp(typeString, "TOKEN_GREATER") == 0) {
        return TOKEN_GREATER;
    } else if (strcmp(typeString, "TOKEN_EQUAL") == 0) {
        return TOKEN_EQUAL;
    } else if (strcmp(typeString, "TOKEN_LESS_OR_EQUAL") == 0) {
        return TOKEN_LESS_OR_EQUAL;
    } else if (strcmp(typeString, "TOKEN_GREATER_OR_EQUAL") == 0) {
        return TOKEN_GREATER_OR_EQUAL;
    } else if (strcmp(typeString, "TOKEN_OPEN_PAREN") == 0) {
        return TOKEN_OPEN_PAREN;
    } else if (strcmp(typeString, "TOKEN_CLOSE_PAREN") == 0) {
        return TOKEN_CLOSE_PAREN;
    } else if (strcmp(typeString, "TOKEN_OPEN_BRACE") == 0) {
        return TOKEN_OPEN_BRACE;
    } else if (strcmp(typeString, "TOKEN_CLOSE_BRACE") == 0) {
        return TOKEN_CLOSE_BRACE;
    } else if (strcmp(typeString, "TOKEN_PRINT") == 0) {
        return TOKEN_PRINT;
    } else if (strcmp(typeString, "TOKEN_INPUT") == 0) {
        return TOKEN_INPUT;
    } else if (strcmp(typeString, "TOKEN_WHILE") == 0) {
        return TOKEN_WHILE;
    } else if (strcmp(typeString, "TOKEN_CONDITION") == 0) {
        return TOKEN_CONDITION;
    } else if (strcmp(typeString, "TOKEN_THEN") == 0) {
        return TOKEN_THEN;
    } else if (strcmp(typeString, "TOKEN_ELSE") == 0) {
        return TOKEN_ELSE;
    } else if (strcmp(typeString, "TOKEN_IF") == 0) {
        return TOKEN_IF;
    } else if (strcmp(typeString, "TOKEN_NEW_LINE") == 0) {
        return TOKEN_NEW_LINE;
    } else if (strcmp(typeString, "TOKEN_EOF") == 0) {
        return TOKEN_EOF;
    } else {
        fprintf(stderr, "Error: Unknown token type: %s\n", typeString);
        return TOKEN_ERROR;
    }
}

Node* parseexpressions(cJSON *tokens, int start, int end) {
    int relationalOperatorIndex = -1;
    int plusMinusIndex = -1;
    for (int i = start; i <= end; i++) {
        cJSON *token = cJSON_GetArrayItem(tokens, i);
        if (cJSON_IsObject(token)) {
            cJSON *type = cJSON_GetObjectItemCaseSensitive(token, "type");
            if (cJSON_IsString(type)) {
                enum TokenType tokenType = getTokenTypeFromString(type->valuestring);
                if (tokenType == TOKEN_OPEN_PAREN) {
                    int openParenCount = 1;
                    int closeParenIndex = i + 1;
                    while (closeParenIndex <= end && openParenCount > 0) {
                        cJSON *innerToken = cJSON_GetArrayItem(tokens, closeParenIndex);
                        if (cJSON_IsObject(innerToken)) {
                            cJSON *innerType = cJSON_GetObjectItemCaseSensitive(innerToken, "type");
                            if (cJSON_IsString(innerType)) {
                                enum TokenType innerTokenType = getTokenTypeFromString(innerType->valuestring);
                                if (innerTokenType == TOKEN_OPEN_PAREN) {
                                    openParenCount++;
                                } else if (innerTokenType == TOKEN_CLOSE_PAREN) {
                                    openParenCount--;
                                }
                            }
                        }
                        closeParenIndex++;
                    }
                    i = closeParenIndex - 1;
                    continue;
                } else if (tokenType == TOKEN_EQUAL || tokenType == TOKEN_GREATER || tokenType == TOKEN_LESS) {
                    relationalOperatorIndex = i;
                    break;
                }
            }
        }
    }
    if (relationalOperatorIndex != -1) {
        cJSON *relationalOperatorToken = cJSON_GetArrayItem(tokens, relationalOperatorIndex);
        Node* relationalOperatorNode = (Node*)malloc(sizeof(Node));
        relationalOperatorNode->type = getTokenTypeFromString(cJSON_GetObjectItemCaseSensitive(relationalOperatorToken, "type")->valuestring);
        const char* relationalOperatorLexeme = cJSON_GetObjectItemCaseSensitive(relationalOperatorToken, "lexeme")->valuestring;
        snprintf(relationalOperatorNode->lexeme, sizeof(relationalOperatorNode->lexeme), "%s", relationalOperatorLexeme);
        relationalOperatorNode->intValue = 0;
        relationalOperatorNode->doubleValue = 0;
        relationalOperatorNode->left = parseexpressions(tokens, start, relationalOperatorIndex - 1);
        relationalOperatorNode->right = parseexpressions(tokens, relationalOperatorIndex + 1, end);
        return relationalOperatorNode;
    }

    int firstNewLineIndex = -1;
    int endbrace = -1;
    for (int i = end; i >= start; i--) {
        cJSON *token = cJSON_GetArrayItem(tokens, i);
        if (cJSON_IsObject(token)) {
            cJSON *type = cJSON_GetObjectItemCaseSensitive(token, "type");
            if (cJSON_IsString(type)) {
                enum TokenType tokenType = getTokenTypeFromString(type->valuestring);
                if (tokenType == TOKEN_NEW_LINE) {
                    firstNewLineIndex = i;
                }
                if (tokenType == TOKEN_CLOSE_BRACE) {
                    endbrace = i;
                }
            }
        }
    }
    if (firstNewLineIndex != -1) {
        Node* right = parseexpressions(tokens, start, firstNewLineIndex - 1);
        Node* left = parseexpressions(tokens, firstNewLineIndex + 1, end);
        Node* newLineNode = (Node*)malloc(sizeof(Node));
        newLineNode->type = TOKEN_NEW_LINE;
        snprintf(newLineNode->lexeme, sizeof(newLineNode->lexeme), "%s", "\n");

        newLineNode->left = left;
        newLineNode->right = right;
        newLineNode->intValue = 0;
        newLineNode->doubleValue = 0;
        return newLineNode;
    } else if (endbrace != -1){
        return NULL;
    }

    for (int i = start; i <= end; i++) {
        cJSON *token = cJSON_GetArrayItem(tokens, i);
        if (cJSON_IsObject(token)) {
            cJSON *type = cJSON_GetObjectItemCaseSensitive(token, "type");
            if (cJSON_IsString(type)) {
                enum TokenType tokenType = getTokenTypeFromString(type->valuestring);
                if (tokenType == TOKEN_PRINT) {
                    Node* printNode = (Node*)malloc(sizeof(Node));
                    printNode->type = TOKEN_PRINT;
                    const char* printLexeme = cJSON_GetObjectItemCaseSensitive(cJSON_GetArrayItem(tokens, i), "lexeme")->valuestring;
                    snprintf(printNode->lexeme, sizeof(printNode->lexeme), "%s", printLexeme);
                    printNode->left = NULL;
                    printNode->right = parseexpressions(tokens, i + 1, end);
                    printNode->intValue = 0;
                    printNode->doubleValue = 0;
                    return printNode;
                }
                if (tokenType == TOKEN_ASSIGN){
                    Node* assignNode = (Node*)malloc(sizeof(Node));
                    assignNode->type = getTokenTypeFromString(cJSON_GetObjectItemCaseSensitive(cJSON_GetArrayItem(tokens, i), "type")->valuestring);
                    const char* assignLexeme = cJSON_GetObjectItemCaseSensitive(cJSON_GetArrayItem(tokens, i), "lexeme")->valuestring;
                    snprintf(assignNode->lexeme, sizeof(assignNode->lexeme), "%s", assignLexeme);
                    assignNode->intValue = 0;
                    assignNode->doubleValue = 0;
                    assignNode->left = handleIdentifier(tokens, i - 1);
                    assignNode->right = parseexpressions(tokens, i + 1, end);
                    return assignNode;
                }
                if (tokenType == TOKEN_OPEN_PAREN) {
                    int openParenCount = 1;
                    int closeParenIndex = i + 1;
                    while (closeParenIndex <= end && openParenCount > 0) {
                        cJSON *innerToken = cJSON_GetArrayItem(tokens, closeParenIndex);
                        if (cJSON_IsObject(innerToken)) {
                            cJSON *innerType = cJSON_GetObjectItemCaseSensitive(innerToken, "type");
                            if (cJSON_IsString(innerType)) {
                                enum TokenType innerTokenType = getTokenTypeFromString(innerType->valuestring);
                                if (innerTokenType == TOKEN_OPEN_PAREN) {
                                    openParenCount++;
                                } else if (innerTokenType == TOKEN_CLOSE_PAREN) {
                                    openParenCount--;
                                }
                            }
                        }
                        closeParenIndex++;
                    }
                    i = closeParenIndex - 1;
                    continue;
                } else if (tokenType == TOKEN_MINUS || tokenType == TOKEN_PLUS) {
                    plusMinusIndex = i;
                }
            }
        }
    }
    if (plusMinusIndex != -1) {
        cJSON *plusMinusToken = cJSON_GetArrayItem(tokens, plusMinusIndex);
        Node* plusMinusNode = (Node*)malloc(sizeof(Node));
        plusMinusNode->type = getTokenTypeFromString(cJSON_GetObjectItemCaseSensitive(plusMinusToken, "type")->valuestring);
        const char* plusMinusLexeme = cJSON_GetObjectItemCaseSensitive(plusMinusToken, "lexeme")->valuestring;
        snprintf(plusMinusNode->lexeme, sizeof(plusMinusNode->lexeme), "%s", plusMinusLexeme);
        plusMinusNode->intValue = 0;
        plusMinusNode->doubleValue = 0;
        if (plusMinusIndex + 1 > end || start > plusMinusIndex - 1){
                Node *errorNode = (Node *)malloc(sizeof(Node));
                errorNode->type = TOKEN_ERROR;
                const char *errorLexeme = cJSON_GetObjectItemCaseSensitive(cJSON_GetArrayItem(tokens, plusMinusIndex), "lexeme")->valuestring;
                snprintf(errorNode->lexeme, sizeof(errorNode->lexeme), "%s", errorLexeme);
                errorNode->left = NULL;
                errorNode->right = NULL;
                errorNode->intValue = 0;
                errorNode->doubleValue = 0;
                fprintf(stderr, "Error: Incorrect use of  '%s'\n", errorLexeme);
                return errorNode;
        }
        plusMinusNode->left = parseexpressions(tokens, start, plusMinusIndex - 1);
        plusMinusNode->right = parseexpressions(tokens, plusMinusIndex + 1, end);
        return plusMinusNode;
    }
    int multDivIndex = -1;
    for (int i = start; i <= end; i++) {
        cJSON *token = cJSON_GetArrayItem(tokens, i);
        if (cJSON_IsObject(token)) {
            cJSON *type = cJSON_GetObjectItemCaseSensitive(token, "type");
            if (cJSON_IsString(type)) {
                enum TokenType tokenType = getTokenTypeFromString(type->valuestring);
                if (tokenType == TOKEN_OPEN_PAREN) {
                    int openParenCount = 1;
                    int closeParenIndex = i + 1;
                    while (closeParenIndex <= end && openParenCount > 0) {
                        cJSON *innerToken = cJSON_GetArrayItem(tokens, closeParenIndex);
                        if (cJSON_IsObject(innerToken)) {
                            cJSON *innerType = cJSON_GetObjectItemCaseSensitive(innerToken, "type");
                            if (cJSON_IsString(innerType)) {
                                enum TokenType innerTokenType = getTokenTypeFromString(innerType->valuestring);

                                if (innerTokenType == TOKEN_OPEN_PAREN) {
                                    openParenCount++;
                                } else if (innerTokenType == TOKEN_CLOSE_PAREN) {
                                    openParenCount--;
                                }
                            }
                        }
                        closeParenIndex++;
                    }
                    i = closeParenIndex - 1;
                    continue;
                } else if (tokenType == TOKEN_DIVISION || tokenType == TOKEN_MULTI) {
                    multDivIndex = i;
                }
            }
        }
    }
    if (multDivIndex != -1) {
        cJSON *multDivToken = cJSON_GetArrayItem(tokens, multDivIndex);
        Node* multDivNode = (Node*)malloc(sizeof(Node));
        multDivNode->type = getTokenTypeFromString(cJSON_GetObjectItemCaseSensitive(multDivToken, "type")->valuestring);
        const char* multDivLexeme = cJSON_GetObjectItemCaseSensitive(multDivToken, "lexeme")->valuestring;
        snprintf(multDivNode->lexeme, sizeof(multDivNode->lexeme), "%s", multDivLexeme);
        multDivNode->intValue = 0;
        multDivNode->doubleValue = 0;
        if (multDivIndex + 1 > end || start > multDivIndex - 1){
                Node *errorNode = (Node *)malloc(sizeof(Node));
                errorNode->type = TOKEN_ERROR;
                const char *errorLexeme = cJSON_GetObjectItemCaseSensitive(cJSON_GetArrayItem(tokens, multDivIndex), "lexeme")->valuestring;
                snprintf(errorNode->lexeme, sizeof(errorNode->lexeme), "%s", errorLexeme);
                errorNode->left = NULL;
                errorNode->right = NULL;
                errorNode->intValue = 0;
                errorNode->doubleValue = 0;
                fprintf(stderr, "Error: Incorrect use of  '%s'\n", errorLexeme);
                return errorNode;
        }
        multDivNode->left = parseexpressions(tokens, start, multDivIndex - 1);
        multDivNode->right = parseexpressions(tokens, multDivIndex + 1, end);
        return multDivNode;
    }
    if (start == end) {
        cJSON *leafToken = cJSON_GetArrayItem(tokens, start);
        Node* leafNode = (Node*)malloc(sizeof(Node));
        leafNode->type = getTokenTypeFromString(cJSON_GetObjectItemCaseSensitive(leafToken, "type")->valuestring);
        const char* leafLexeme = cJSON_GetObjectItemCaseSensitive(leafToken, "lexeme")->valuestring;
        snprintf(leafNode->lexeme, sizeof(leafNode->lexeme), "%s", leafLexeme);
        leafNode->left = NULL;
        leafNode->right = NULL;
        if (leafNode->type == TOKEN_INT_LITERAL) {
            leafNode->intValue = atoi(leafLexeme);
            leafNode->doubleValue = 0.0;
        } else if (leafNode->type == TOKEN_DOUBLE_LITERAL) {
            leafNode->intValue = 0;
            leafNode->doubleValue = atof(leafLexeme);
        } else {
            leafNode->intValue = 0;
            leafNode->doubleValue = 0.0;
        }
        return leafNode;
    }

    if (start < end && cJSON_GetArrayItem(tokens, start)->type == cJSON_Object &&  cJSON_GetArrayItem(tokens, end)->type == cJSON_Object) {
        cJSON *startType = cJSON_GetObjectItemCaseSensitive(cJSON_GetArrayItem(tokens, start), "type");
        cJSON *endType = cJSON_GetObjectItemCaseSensitive(cJSON_GetArrayItem(tokens, end), "type");
        if (cJSON_IsString(startType) && cJSON_IsString(endType) &&
            getTokenTypeFromString(startType->valuestring) == TOKEN_OPEN_PAREN &&
            getTokenTypeFromString(endType->valuestring) == TOKEN_CLOSE_PAREN) {
            return parseexpressions(tokens, start + 1, end - 1);
        }
    }
    return NULL;
}

Node* handleVariableDeclaration(cJSON *tokens, int index, int end) {
    Node* declarationNode = (Node*)malloc(sizeof(Node));
    declarationNode->type = getTokenTypeFromString(cJSON_GetObjectItemCaseSensitive(cJSON_GetArrayItem(tokens, index), "type")->valuestring);
    const char* declarationLexeme = cJSON_GetObjectItemCaseSensitive(cJSON_GetArrayItem(tokens, index), "lexeme")->valuestring;
    snprintf(declarationNode->lexeme, sizeof(declarationNode->lexeme), "%s", declarationLexeme);
    if (index + 1 <= end) {
        cJSON *identifierToken = cJSON_GetArrayItem(tokens, index + 1);
        if (cJSON_IsObject(identifierToken)) {
            cJSON *identifierType = cJSON_GetObjectItemCaseSensitive(identifierToken, "type");
            if (cJSON_IsString(identifierType) && getTokenTypeFromString(identifierType->valuestring) == TOKEN_IDENTIFIER) {
                Node* currentTokenNode = (Node*)malloc(sizeof(Node));
                currentTokenNode->type = getTokenTypeFromString(cJSON_GetObjectItemCaseSensitive(cJSON_GetArrayItem(tokens, index), "type")->valuestring);
                const char* currentLexeme = cJSON_GetObjectItemCaseSensitive(cJSON_GetArrayItem(tokens, index), "lexeme")->valuestring;
                snprintf(currentTokenNode->lexeme, sizeof(currentTokenNode->lexeme), "%s", currentLexeme);
                currentTokenNode->left = NULL;
                currentTokenNode->intValue = 0;
                currentTokenNode->doubleValue = 0;
                Node* nextNode = (Node*)malloc(sizeof(Node));
                nextNode->type = getTokenTypeFromString(identifierType->valuestring);
                const char* nextLexeme = cJSON_GetObjectItemCaseSensitive(identifierToken, "lexeme")->valuestring;
                snprintf(nextNode->lexeme, sizeof(nextNode->lexeme), "%s", nextLexeme);
                nextNode->intValue = 0;
                nextNode->doubleValue = 0;
                nextNode->left = NULL;
                nextNode->right = NULL;
                currentTokenNode->right = nextNode;
                if (index + 2 <= end) {
                    cJSON *nextnextToken = cJSON_GetArrayItem(tokens, index + 2);
                    if (cJSON_IsObject(nextnextToken)) {
                        cJSON *nextnextTokenType = cJSON_GetObjectItemCaseSensitive(nextnextToken, "type");
                        if (cJSON_IsString(nextnextTokenType)) {
                            Node* nextnextNode = (Node*)malloc(sizeof(Node));
                            nextnextNode->type = getTokenTypeFromString(nextnextTokenType->valuestring);
                            const char* nextnextLexeme = cJSON_GetObjectItemCaseSensitive(nextnextToken, "lexeme")->valuestring;
                            snprintf(nextnextNode->lexeme, sizeof(nextnextNode->lexeme), "%s", nextnextLexeme);
                            nextnextNode->intValue = 0;
                            nextnextNode->doubleValue = 0;
                            nextnextNode->left = NULL;
                            nextnextNode->right = NULL;
                            nextNode->right = nextnextNode;
                            if (index + 3 <= end) {
                                cJSON *lastToken = cJSON_GetArrayItem(tokens, index + 3);
                                if (cJSON_IsObject(lastToken)) {
                                    cJSON *lastTokenType = cJSON_GetObjectItemCaseSensitive(lastToken, "type");
                                    if (cJSON_IsString(lastTokenType)) {
                                        Node* lastNode = (Node*)malloc(sizeof(Node));
                                        lastNode->type = getTokenTypeFromString(lastTokenType->valuestring);
                                        const char* lastLexeme = cJSON_GetObjectItemCaseSensitive(lastToken, "lexeme")->valuestring;
                                        snprintf(lastNode->lexeme, sizeof(lastNode->lexeme), "%s", lastLexeme);
                                        lastNode->left = NULL;
                                        lastNode->right = NULL;
                                        if (lastNode->type == TOKEN_INT_LITERAL) {
                                            lastNode->intValue = atoi(lastLexeme);
                                            lastNode->doubleValue = 0.0;
                                        } else if (lastNode->type == TOKEN_DOUBLE_LITERAL) {
                                            lastNode->intValue = 0;
                                            lastNode->doubleValue = strtod(lastLexeme, NULL);
                                        }
                                        nextnextNode->right = lastNode;
                                    }
                                }
                            }
                        }
                    }
                }
                return currentTokenNode;
            } else{
                Node *errorNode = (Node *)malloc(sizeof(Node));
                errorNode->type = TOKEN_ERROR;
                const char *errorLexeme = cJSON_GetObjectItemCaseSensitive(cJSON_GetArrayItem(tokens, index), "lexeme")->valuestring;
                snprintf(errorNode->lexeme, sizeof(errorNode->lexeme), "%s", errorLexeme);
                errorNode->left = NULL;
                errorNode->right = NULL;
                errorNode->intValue = 0;
                errorNode->doubleValue = 0;
                fprintf(stderr, "Error: Unknown token '%s'\n", errorLexeme);
                return errorNode;
            }
        }
    }
    return NULL;
}

int findTokenIndex(cJSON *tokens, int start, int end, enum TokenType targetTokenType) {
    for (int i = start; i <= end; i++) {
        cJSON *token = cJSON_GetArrayItem(tokens, i);
        if (cJSON_IsObject(token)) {
            cJSON *type = cJSON_GetObjectItemCaseSensitive(token, "type");
            if (cJSON_IsString(type)) {
                enum TokenType tokenType = getTokenTypeFromString(type->valuestring);
                if (tokenType == targetTokenType) {
                    return i;
                }
            }
        }
    }
    return -1;
}

Node* createLeafNode(enum TokenType type, const char* lexeme) {
    Node* leafNode = (Node*)malloc(sizeof(Node));
    leafNode->type = type;
    snprintf(leafNode->lexeme, sizeof(leafNode->lexeme), "%s", lexeme);
    leafNode->left = NULL;
    leafNode->right = NULL;
    leafNode->intValue = 0;
    leafNode->doubleValue = 0;
    return leafNode;
}

Node* handleIdentifier(cJSON *tokens, int index) {
    cJSON *token = cJSON_GetArrayItem(tokens, index);
    Node* identifierNode = (Node*)malloc(sizeof(Node));
    identifierNode->type = getTokenTypeFromString(cJSON_GetObjectItemCaseSensitive(token, "type")->valuestring);
    const char* identifierLexeme = cJSON_GetObjectItemCaseSensitive(token, "lexeme")->valuestring;
    snprintf(identifierNode->lexeme, sizeof(identifierNode->lexeme), "%s", identifierLexeme);
    identifierNode->left = NULL;
    identifierNode->right = NULL;
    identifierNode->intValue = 0;
    identifierNode->doubleValue = 0;
    return identifierNode;
}

Node* handleAssignment(cJSON *tokens, int index, int end) {
    Node* assignNode = (Node*)malloc(sizeof(Node));
    assignNode->type = getTokenTypeFromString(cJSON_GetObjectItemCaseSensitive(cJSON_GetArrayItem(tokens, index), "type")->valuestring);
    const char* assignLexeme = cJSON_GetObjectItemCaseSensitive(cJSON_GetArrayItem(tokens, index), "lexeme")->valuestring;
    snprintf(assignNode->lexeme, sizeof(assignNode->lexeme), "%s", assignLexeme);
    assignNode->left = handleIdentifier(tokens, index - 1);
    assignNode->right = parseexpressions(tokens, index + 1, end);
    assignNode->intValue = 0;
    assignNode->doubleValue = 0;
    return assignNode;
}

Node* handlePrint(cJSON *tokens, int index, int end) {
    Node* printNode = (Node*)malloc(sizeof(Node));
    printNode->type = TOKEN_PRINT;
    const char* printLexeme = cJSON_GetObjectItemCaseSensitive(cJSON_GetArrayItem(tokens, index), "lexeme")->valuestring;
    snprintf(printNode->lexeme, sizeof(printNode->lexeme), "%s", printLexeme);
    printNode->left = NULL;
    printNode->right = parseexpressions(tokens, index + 1, end);
    printNode->intValue = 0;
    printNode->doubleValue = 0;
    return printNode;
}

Node* handleIf(cJSON *tokens, int index, int end) {
    Node* ifNode = (Node*)malloc(sizeof(Node));
    ifNode->type = TOKEN_IF;
    snprintf(ifNode->lexeme, sizeof(ifNode->lexeme), "%s", "");
    int openParIndex = findTokenIndex(tokens, index + 1, end, TOKEN_OPEN_PAREN);
    int closeParIndex = findTokenIndex(tokens, openParIndex + 1, end, TOKEN_OPEN_BRACE);
    const char* ifLexeme = cJSON_GetObjectItemCaseSensitive(cJSON_GetArrayItem(tokens, index), "lexeme")->valuestring;
    snprintf(ifNode->lexeme, sizeof(ifNode->lexeme), "%s", ifLexeme);
    ifNode->left = parseexpressions(tokens, openParIndex + 1, closeParIndex - 2);
    ifNode->right = parseexpressions(tokens, closeParIndex + 2, end);
    ifNode->intValue = 0;
    ifNode->doubleValue = 0;
    return ifNode;
}

Node* handleWhile(cJSON *tokens, int index, int end) {
    Node* whileNode = (Node*)malloc(sizeof(Node));
    whileNode->type = TOKEN_WHILE;
    snprintf(whileNode->lexeme, sizeof(whileNode->lexeme), "%s", "");
    int openParIndex = findTokenIndex(tokens, index + 1, end, TOKEN_OPEN_PAREN);
    int closeParIndex = findTokenIndex(tokens, openParIndex + 1, end, TOKEN_OPEN_BRACE);
    const char* whileLexeme = cJSON_GetObjectItemCaseSensitive(cJSON_GetArrayItem(tokens, index), "lexeme")->valuestring;
    snprintf(whileNode->lexeme, sizeof(whileNode->lexeme), "%s", whileLexeme);
    whileNode->left = parseexpressions(tokens, openParIndex + 1, closeParIndex - 2);
    whileNode->right = parseexpressions(tokens, closeParIndex + 2, end);
    whileNode->intValue = 0;
    whileNode->doubleValue = 0;
    return whileNode;
}


Node* handleId(cJSON *tokens, int index, int end) {
    Node* idNode = (Node*)malloc(sizeof(Node));
    idNode->type = TOKEN_IDENTIFIER;
    const char* idLexeme = cJSON_GetObjectItemCaseSensitive(cJSON_GetArrayItem(tokens, index), "lexeme")->valuestring;
    snprintf(idNode->lexeme, sizeof(idNode->lexeme), "%s", idLexeme);
    idNode->left = NULL;
    idNode->right = NULL;
    if (index + 1 <= end) {
        cJSON *nextToken = cJSON_GetArrayItem(tokens, index + 1);
        if (cJSON_IsObject(nextToken)) {
            cJSON *nextTokenType = cJSON_GetObjectItemCaseSensitive(nextToken, "type");
            if (cJSON_IsString(nextTokenType)) {
                if (getTokenTypeFromString(nextTokenType->valuestring) == TOKEN_ASSIGN) {
                    return handleAssignment(tokens, index+1, end);
                } else {
                    Node *errorNode = (Node *)malloc(sizeof(Node));
                    errorNode->type = TOKEN_ERROR;
                    const char *errorLexeme = cJSON_GetObjectItemCaseSensitive(cJSON_GetArrayItem(tokens, index), "lexeme")->valuestring;
                    snprintf(errorNode->lexeme, sizeof(errorNode->lexeme), "%s", errorLexeme);
                    errorNode->left = NULL;
                    errorNode->right = NULL;
                    errorNode->intValue = 0;
                    errorNode->doubleValue = 0;
                    fprintf(stderr, "Error: Something wrong with token '%s'\n", errorLexeme);
                    return errorNode;
                }
            }
        }
    }
    else{
        Node *errorNode = (Node *)malloc(sizeof(Node));
                    errorNode->type = TOKEN_ERROR;
                    const char *errorLexeme = cJSON_GetObjectItemCaseSensitive(cJSON_GetArrayItem(tokens, index), "lexeme")->valuestring;
                    snprintf(errorNode->lexeme, sizeof(errorNode->lexeme), "%s", errorLexeme);
                    errorNode->left = NULL;
                    errorNode->right = NULL;
                    errorNode->intValue = 0;
                    errorNode->doubleValue = 0;
                    fprintf(stderr, "The variable was expected to be equated to some value. Variable name:  '%s'\n", errorLexeme);
                    return errorNode;
    }
}



Node* checkthatitis(cJSON *tokens, int start, int end, const char* lexeme) {
    int i = start;
    cJSON *token = cJSON_GetArrayItem(tokens, i);
    if (cJSON_IsObject(token)) {
        cJSON *type = cJSON_GetObjectItemCaseSensitive(token, "type");
        if (cJSON_IsString(type)) {
            enum TokenType tokenType = getTokenTypeFromString(type->valuestring);
            switch (tokenType) {
                case TOKEN_EOF:
                    return createLeafNode(TOKEN_EOF, "");
                case TOKEN_INT_DECL:
                case TOKEN_DOUBLE_DECL:
                    return handleVariableDeclaration(tokens, i, end);
                case TOKEN_IDENTIFIER:
                    return handleId(tokens, i, end);
                case TOKEN_PRINT:
                    return handlePrint(tokens, i, end);
                case TOKEN_IF:
                    return handleIf(tokens, i, end);
                case TOKEN_WHILE:
                    return handleWhile(tokens, i, end);
            }
        }
    }
    return NULL;
}



Node* parseTokens(cJSON *tokens, int start, int end, const char* lexeme) {
    if (start > end) {
        return NULL;
    }

    int j = -1;
    int firstNewLineIndex = -1;
    int closeBraceIndex = -1;
    int errorTokenIndex = -1;
    for (int i = start; i <= end; i++) {
        cJSON *token = cJSON_GetArrayItem(tokens, i);
        if (cJSON_IsObject(token)) {
            cJSON *type = cJSON_GetObjectItemCaseSensitive(token, "type");
            if (cJSON_IsString(type)) {
                enum TokenType tokenType = getTokenTypeFromString(type->valuestring);
                if (tokenType == TOKEN_ERROR){
                    errorTokenIndex = i;
                    break;
                }
                else if (tokenType == TOKEN_OPEN_BRACE){
                    j = i;
                    closeBraceIndex = findTokenIndex(tokens, j + 1, end, TOKEN_CLOSE_BRACE);
                }
                if (j != -1){
                    if (i>closeBraceIndex){
                        if (tokenType == TOKEN_NEW_LINE) {
                            firstNewLineIndex = i;
                            break;

                        }
                    }
                } else{
                    if (tokenType == TOKEN_NEW_LINE) {
                        firstNewLineIndex = i;
                        break;
                    }
                }
            }
        }
    }


    if (errorTokenIndex != -1) {
        cJSON *errorToken = cJSON_GetArrayItem(tokens, errorTokenIndex);
        Node *errorNode = (Node *)malloc(sizeof(Node));
        errorNode->type = TOKEN_ERROR;
        const char *errorLexeme = cJSON_GetObjectItemCaseSensitive(errorToken, "lexeme")->valuestring;
        snprintf(errorNode->lexeme, sizeof(errorNode->lexeme), "%s", errorLexeme);
        errorNode->left = NULL;
        errorNode->right = NULL;
        errorNode->intValue = 0;
        errorNode->doubleValue = 0;
        return errorNode;
    } else if (firstNewLineIndex != -1) {
        Node* left = parseTokens(tokens, firstNewLineIndex + 1, end, lexeme);
        Node* right = checkthatitis(tokens, start, firstNewLineIndex - 1, lexeme);
        Node* newLineNode = (Node*)malloc(sizeof(Node));
        newLineNode->type = TOKEN_NEW_LINE;
        if (lexeme != NULL) {
            snprintf(newLineNode->lexeme, sizeof(newLineNode->lexeme), "%s", lexeme);
        } else {
            cJSON *lexemeJson = cJSON_GetObjectItemCaseSensitive(cJSON_GetArrayItem(tokens, firstNewLineIndex), "lexeme");
            if (cJSON_IsString(lexemeJson)) {
                snprintf(newLineNode->lexeme, sizeof(newLineNode->lexeme), "%s", lexemeJson->valuestring);
            } else {
                snprintf(newLineNode->lexeme, sizeof(newLineNode->lexeme), "%s", " ");
            }
        }
        newLineNode->left = left;
        newLineNode->right = right;
        newLineNode->intValue = 0;
        newLineNode->doubleValue = 0;
        return newLineNode;
    } else {
        Node* eofNode = (Node*)malloc(sizeof(Node));
        eofNode->type = TOKEN_EOF;
        if (lexeme != NULL) {
            snprintf(eofNode->lexeme, sizeof(eofNode->lexeme), "%s", lexeme);
        } else {
            cJSON *lastToken = cJSON_GetArrayItem(tokens, end);
            cJSON *lastLexemeJson = cJSON_GetObjectItemCaseSensitive(lastToken, "lexeme");
            if (cJSON_IsString(lastLexemeJson)) {
                snprintf(eofNode->lexeme, sizeof(eofNode->lexeme), "%s", lastLexemeJson->valuestring);
            } else {
                snprintf(eofNode->lexeme, sizeof(eofNode->lexeme), "%s", " ");
            }
        }
        eofNode->left = NULL;
        eofNode->right = NULL;
        eofNode->intValue = 0;
        eofNode->doubleValue = 0;
        return eofNode;
    }
}


Node* Parser() {
    FILE *file = fopen("./output.json", "r");
    if (!file) {
        printf("Error during opening the file output.json\n");
        exit(EXIT_FAILURE);
    }
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    char *json_data = (char *)malloc(file_size + 1);
    fread(json_data, 1, file_size, file);
    fclose(file);
    json_data[file_size] = '\0';
    cJSON *json_data_parsed = cJSON_Parse(json_data);
    if (!json_data_parsed) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            fprintf(stderr, "Error printing JSON: %s\n", error_ptr);
        }
        free(json_data);
        return 1;
    }
    Node* ast = parse(json_data_parsed);
    printf("finished");
    return ast;
}