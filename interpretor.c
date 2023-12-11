#include "parser.h"
#include "lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

int errorOccurred = 0;

typedef struct variable{
    char* name;
    char* type;
    int initialized;
    union {
        float f_val;
        int i_val;
    }value ;
    struct variable* next;
}variable ;

variable *table = NULL;

variable *find_or_add_variable(const char* name, int add_if_not_found, TokenType type){
    variable *entry = table;
    while (entry){
        if (strcmp(entry->name, name) == 0){


            return entry;
        }
        entry = entry->next;

    }
    if (add_if_not_found){
        variable *new_entry = malloc(sizeof(variable));
        new_entry->name = strdup(name);

        new_entry->initialized = 0;

        if (type == TOKEN_INT_DECL){
            new_entry->value.i_val = 0;
            new_entry->type = "int";
        }
        else if (type == TOKEN_DOUBLE_DECL){
            new_entry->value.f_val = 0;
            new_entry->type = "float";
        }
        new_entry->next = table;
        table = new_entry;
        return new_entry;
    }
    return NULL;
}

int is_whole_number(double value){
    return value == floor(value);
}

void report_error(const char* message) {
    fprintf(stderr, "Error: %s\n", message);
}

float interpret(Node* ast) {
variable *entry;
double left, right;
if (!ast){
    return 0;
}
    switch (ast->type) {
        case TOKEN_NEW_LINE:
            interpret(ast->right);
//            printf("\n new line");
            interpret(ast->left);
            break;
        case TOKEN_INT_DECL:
        case TOKEN_DOUBLE_DECL:
            entry = find_or_add_variable(ast->right->lexeme, 0, ast->type);
            if (entry){
                entry->initialized = 0;
                char error_message[256];
                snprintf(error_message, sizeof(error_message), "Variable '%s' already declared", ast->right->lexeme);
                report_error(error_message);
                exit(EXIT_FAILURE);
            }
            else {
                entry = find_or_add_variable(ast->right->lexeme, 1, ast->type);
                interpret(ast->left);
                return 0;
            }
            break;
        case TOKEN_IDENTIFIER:
            entry = find_or_add_variable(ast->lexeme, 0, ast->type);
            if (entry){
                if (entry->type == "int"){
                    return entry->value.i_val;
                } else if (entry->type == "float"){
                    return entry->value.f_val;
                }
            }
            else{
                char error_message[256];
                snprintf(error_message, sizeof(error_message), "Variable '%s' not declared", ast->lexeme);
                report_error(error_message);
                exit(EXIT_FAILURE);
                exit(EXIT_FAILURE);
            }
            break;

        case TOKEN_INT_LITERAL:
            return ast->intValue;
        case TOKEN_DOUBLE_LITERAL:
            return ast->doubleValue;

        case TOKEN_PLUS:
            return interpret(ast->left) + interpret(ast->right);
        case TOKEN_MINUS:
            return interpret(ast->left) - interpret(ast->right);
        case TOKEN_MULTI:
            return interpret(ast->left) * interpret(ast->right);
        case TOKEN_DIVISION:
            if (interpret(ast->right) == 0) {
                report_error("Division by zero error");
                exit(EXIT_FAILURE);
            }
            return interpret(ast->left) / interpret(ast->right);
        case TOKEN_GREATER:
            return interpret(ast->left) > interpret(ast->right);

        case TOKEN_LESS:
            return interpret(ast->left) < interpret(ast->right);

        case TOKEN_EQUAL:
            return interpret(ast->left) == interpret(ast->right);

        case TOKEN_PRINT:
            right = interpret(ast->right);
            if (is_whole_number(right)) {
                printf("%i \n", (int)right);
            } else{
                printf("%f \n", right);
            }
            interpret(ast->left);
            break;

        case TOKEN_ASSIGN:
            entry = find_or_add_variable(ast->left->lexeme, 0, ast->type);
            if (!entry) {
                report_error("Variable not declared");
                exit(EXIT_FAILURE); // Return an error code
            }

            errorOccurred = 0;  // Reset the error flag before interpretation
            float right_value = interpret(ast->right);

            // Check the type of the variable and assign the value
            if (strcmp(entry->type, "int") == 0) {
                if (is_whole_number(right_value)) {
                    entry->value.i_val = (int)right_value;
                    entry->initialized = 1;  // Mark as initialized
                } else {
                    // Type mismatch error
                    report_error("Type mismatch: Cannot assign a non-integer value to integer variable");
                    exit(EXIT_FAILURE);  // Return an error code
                }
            } else if (strcmp(entry->type, "float") == 0) {
                entry->value.f_val = right_value;
                entry->initialized = 1;  // Mark as initialized
            } else {
                // Unknown type error
                report_error("Unknown type for variable");
                exit(EXIT_FAILURE);  // Return an error code
            }
            break;

        case TOKEN_IF:
        if ((int) interpret(ast->left)){
            interpret(ast->right);
        }
            interpret(ast->left);
            break;
        case TOKEN_WHILE:
            while ((int) interpret(ast->left)){
                interpret(ast->right);
        }

            interpret(ast->left);
            break;

}

}


int main() {
    performLexicalAnalysis("./input.txt", "./output.json");
    Node* root = Parser();  // Parse your language and get the AST
    printf("\n");
    interpret(root);
    return 0;
}



