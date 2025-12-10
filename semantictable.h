#ifndef SEMANTIC_TABLE_H
#define SEMANTIC_TABLE_H

class SemanticTable {
public:
    //tipos suportados
    enum Types { INT = 0, FLO, CHA, STR, BOO };

    //operações (ordem fixa usada nas tabelas)
    //SUM, SUB, MUL, DIV, REL(>,<,==,...), MOD, POT (^), ROO (sqrt/raiz), AND (&&), OR (||)
    enum Operations { SUM=0, SUB, MUL, DIV, REL, MOD, POT, ROO, AND, OR_, BIT_AND, BIT_OR, BIT_XOR };


    //dtatus de atribuição
    enum Status { ERR = -1, WAR, OK_ }; // ERR=-1, WAR=0, OK_=1

    //TABELAS
    //expTable[TP1][TP2][OP] => tipo resultante da expressão TP1 (op) TP2
    static const int expTable[5][5][13];

    //atribTable[LHS][RHS] => OK_, WAR, ERR pra compatibilidade de atribuição
    static const int atribTable[5][5];

    //consultas utilitárias
    static int resultType(int TP1, int TP2, int OP) {
        if (TP1 < 0 || TP1 >= 5 || TP2 < 0 || TP2 >= 5 || OP < 0 || OP >= 13) {
            return ERR;
        }
        return expTable[TP1][TP2][OP];
    }

    static int atribType(int TP1, int TP2) {
        if (TP1 < 0 || TP1 >= 5 || TP2 < 0 || TP2 >= 5) {
            return ERR;
        }
        return atribTable[TP1][TP2];
    }
};

#endif // SEMANTIC_TABLE_H
