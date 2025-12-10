#ifndef CONSTANTS_H
#define CONSTANTS_H

enum TokenId 
{
    EPSILON  = 0,
    DOLLAR   = 1,
    t_LIT_INTEIRO = 2,
    t_LIT_DECIMAIS = 3,
    t_LIT_BINARIO = 4,
    t_LIT_HEX = 5,
    t_LIT_STRING = 6,
    t_LIT_CHAR = 7,
    t_ID = 8,
    t_KEY_INT = 9,
    t_KEY_FLOAT = 10,
    t_KEY_END = 11,
    t_KEY_DOUBLE = 12,
    t_KEY_CHAR = 13,
    t_KEY_STRING = 14,
    t_KEY_BOOL = 15,
    t_KEY_RETURN = 16,
    t_KEY_VOID = 17,
    t_KEY_WHILE = 18,
    t_KEY_FOR = 19,
    t_KEY_IF = 20,
    t_KEY_ELSE = 21,
    t_KEY_SWITCH = 22,
    t_KEY_CASE = 23,
    t_KEY_BREAK = 24,
    t_KEY_CLASS = 25,
    t_KEY_NAMESPACE = 26,
    t_KEY_CONTINUE = 27,
    t_KEY_DO = 28,
    t_KEY_TRUE = 29,
    t_KEY_FALSE = 30,
    t_KEY_COUT = 31,
    t_KEY_PRINT = 32,
    t_KEY_CIN = 33,
    t_OP_SUM = 34,
    t_OP_SUB = 35,
    t_OP_MUL = 36,
    t_OP_DIV = 37,
    t_OP_EQU = 38,
    t_OP_MOD = 39,
    t_OP_MAIORIGUAL = 40,
    t_OP_MENORIGUAL = 41,
    t_OP_IGUALDADE = 42,
    t_OP_DIFERENTE = 43,
    t_OP_MAIOR = 44,
    t_OP_MENOR = 45,
    t_OP_AND = 46,
    t_OP_OR = 47,
    t_OP_NOT = 48,
    t_OP_SHR = 49,
    t_OP_SHL = 50,
    t_OP_BIT_AND = 51,
    t_OP_BIT_OR = 52,
    t_OP_BIT_NOT = 53,
    t_OP_BIT_XOR = 54,
    t_DELIM_PONTO = 55,
    t_DELIM_VIRGULA = 56,
    t_DELIM_PONTOVIRGULA = 57,
    t_DELIM_DOIS_PONTOS = 58,
    t_DELIM_PAREN_ESQ = 59,
    t_DELIM_PAREN_DIR = 60,
    t_DELIM_CHAVE_ESQ = 61,
    t_DELIM_CHAVE_DIR = 62,
    t_DELIM_COLCH_ESQ = 63,
    t_DELIM_COLCH_DIR = 64,
    t_COMENTARIO = 65,
    t_COMENTARIO_MULTILINHA = 66
};

const int STATES_COUNT = 54;

extern int SCANNER_TABLE[STATES_COUNT][256];

extern int TOKEN_STATE[STATES_COUNT];

extern int SPECIAL_CASES_INDEXES[68];

extern const char *SPECIAL_CASES_KEYS[25];

extern int SPECIAL_CASES_VALUES[25];

extern const char *SCANNER_ERROR[STATES_COUNT];

const int FIRST_SEMANTIC_ACTION = 124;

const int SHIFT  = 0;
const int REDUCE = 1;
const int ACTION = 2;
const int ACCEPT = 3;
const int GO_TO  = 4;
const int ERROR  = 5;

extern const int PARSER_TABLE[288][826][2];

extern const int PRODUCTIONS[122][2];

extern const char *PARSER_ERROR[288];

#endif
