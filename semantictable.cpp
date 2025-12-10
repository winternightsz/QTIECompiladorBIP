#include "SemanticTable.h"

// -----------------------------------------------------------------------------
//Tabela de EXPRESSÕES: expTable[TP1][TP2][OP]
//Ordem dos OPs: SUM, SUB, MUL, DIV, REL, MOD, POT, ROO, AND, OR_
//Regra geral:
//Numérico (INT/FLO/CHA) com numérico: aritmética OK; se houver FLO -> resultado FLO.
//DIV sempre resulta FLO.
//MOD apenas entre inteiros/char -> INT; caso contrário ERR.
//POT numérico -> INT se ambos INT/CHA; FLO se houver FLO.
//ROO numérico -> FLO (raiz).
//REL entre numéricos/char -> BOO; entre STR×STR -> BOO; demais -> ERR.
//AND/OR_ apenas entre BOO -> BOO; demais -> ERR.
//STR em aritmética -> ERR.
// -----------------------------------------------------------------------------

// SemanticTable.cpp
#define T_INT SemanticTable::INT
#define T_FLO SemanticTable::FLO
#define T_CHA SemanticTable::CHA
#define T_STR SemanticTable::STR
#define T_BOO SemanticTable::BOO
#define T_ERR SemanticTable::ERR

//            SUM     SUB     MUL     DIV     REL     MOD     POT     ROO     AND     OR_     BIT_AND BIT_OR  BIT_XOR
const int SemanticTable::expTable[5][5][13] =
    {
        /* TP1 = INT */
        {
            /* TP2=INT */ { T_INT, T_INT, T_INT, T_FLO, T_BOO, T_INT, T_INT, T_FLO, T_ERR, T_ERR, T_INT,  T_INT,  T_INT  },
            /* TP2=FLO */ { T_FLO, T_FLO, T_FLO, T_FLO, T_BOO, T_ERR, T_FLO, T_FLO, T_ERR, T_ERR, T_ERR,  T_ERR,  T_ERR  },
            /* TP2=CHA */ { T_INT, T_INT, T_INT, T_FLO, T_BOO, T_INT, T_INT, T_FLO, T_ERR, T_ERR, T_INT,  T_INT,  T_INT  },
            /* TP2=STR */ { T_ERR, T_ERR, T_ERR, T_ERR, T_ERR, T_ERR, T_ERR, T_ERR, T_ERR, T_ERR, T_ERR,  T_ERR,  T_ERR  },
            /* TP2=BOO */ { T_ERR, T_ERR, T_ERR, T_ERR, T_ERR, T_ERR, T_ERR, T_ERR, T_ERR, T_ERR, T_ERR,  T_ERR,  T_ERR  }
        },

        /* TP1 = FLO */
        {
            /* TP2=INT */ { T_FLO, T_FLO, T_FLO, T_FLO, T_BOO, T_ERR, T_FLO, T_FLO, T_ERR, T_ERR, T_ERR,  T_ERR,  T_ERR  },
            /* TP2=FLO */ { T_FLO, T_FLO, T_FLO, T_FLO, T_BOO, T_ERR, T_FLO, T_FLO, T_ERR, T_ERR, T_ERR,  T_ERR,  T_ERR  },
            /* TP2=CHA */ { T_FLO, T_FLO, T_FLO, T_FLO, T_BOO, T_ERR, T_FLO, T_FLO, T_ERR, T_ERR, T_ERR,  T_ERR,  T_ERR  },
            /* TP2=STR */ { T_ERR, T_ERR, T_ERR, T_ERR, T_ERR, T_ERR, T_ERR, T_ERR, T_ERR, T_ERR, T_ERR,  T_ERR,  T_ERR  },
            /* TP2=BOO */ { T_ERR, T_ERR, T_ERR, T_ERR, T_ERR, T_ERR, T_ERR, T_ERR, T_ERR, T_ERR, T_ERR,  T_ERR,  T_ERR  }
        },

        /* TP1 = CHA (char como inteiro) */
        {
            /* TP2=INT */ { T_INT, T_INT, T_INT, T_FLO, T_BOO, T_INT, T_INT, T_FLO, T_ERR, T_ERR, T_INT,  T_INT,  T_INT  },
            /* TP2=FLO */ { T_FLO, T_FLO, T_FLO, T_FLO, T_BOO, T_ERR, T_FLO, T_FLO, T_ERR, T_ERR, T_ERR,  T_ERR,  T_ERR  },
            /* TP2=CHA */ { T_INT, T_INT, T_INT, T_FLO, T_BOO, T_INT, T_INT, T_FLO, T_ERR, T_ERR, T_INT,  T_INT,  T_INT  },
            /* TP2=STR */ { T_ERR, T_ERR, T_ERR, T_ERR, T_ERR, T_ERR, T_ERR, T_ERR, T_ERR, T_ERR, T_ERR,  T_ERR,  T_ERR  },
            /* TP2=BOO */ { T_ERR, T_ERR, T_ERR, T_ERR, T_ERR, T_ERR, T_ERR, T_ERR, T_ERR, T_ERR, T_ERR,  T_ERR,  T_ERR  }
        },

        /* TP1 = STR */
        {
            /* TP2=INT */ { T_ERR, T_ERR, T_ERR, T_ERR, T_ERR, T_ERR, T_ERR, T_ERR, T_ERR, T_ERR, T_ERR,  T_ERR,  T_ERR  },
            /* TP2=FLO */ { T_ERR, T_ERR, T_ERR, T_ERR, T_ERR, T_ERR, T_ERR, T_ERR, T_ERR, T_ERR, T_ERR,  T_ERR,  T_ERR  },
            /* TP2=CHA */ { T_ERR, T_ERR, T_ERR, T_ERR, T_ERR, T_ERR, T_ERR, T_ERR, T_ERR, T_ERR, T_ERR,  T_ERR,  T_ERR  },
            /* TP2=STR */ { T_ERR, T_ERR, T_ERR, T_ERR, T_BOO, T_ERR, T_ERR, T_ERR, T_ERR, T_ERR, T_ERR,  T_ERR,  T_ERR  },
            /* TP2=BOO */ { T_ERR, T_ERR, T_ERR, T_ERR, T_ERR, T_ERR, T_ERR, T_ERR, T_ERR, T_ERR, T_ERR,  T_ERR,  T_ERR  }
        },

        /* TP1 = BOO */
        {
            /* TP2=INT */ { T_ERR, T_ERR, T_ERR, T_ERR, T_ERR, T_ERR, T_ERR, T_ERR, T_ERR, T_ERR, T_ERR,  T_ERR,  T_ERR  },
            /* TP2=FLO */ { T_ERR, T_ERR, T_ERR, T_ERR, T_ERR, T_ERR, T_ERR, T_ERR, T_ERR, T_ERR, T_ERR,  T_ERR,  T_ERR  },
            /* TP2=CHA */ { T_ERR, T_ERR, T_ERR, T_ERR, T_ERR, T_ERR, T_ERR, T_ERR, T_ERR, T_ERR, T_ERR,  T_ERR,  T_ERR  },
            /* TP2=STR */ { T_ERR, T_ERR, T_ERR, T_ERR, T_ERR, T_ERR, T_ERR, T_ERR, T_ERR, T_ERR, T_ERR,  T_ERR,  T_ERR  },
            /* TP2=BOO */ { T_ERR, T_ERR, T_ERR, T_ERR, T_BOO, T_ERR, T_ERR, T_ERR, T_BOO, T_BOO, T_ERR,  T_ERR,  T_ERR  }
        }
};


#undef T_INT
#undef T_FLO
#undef T_CHA
#undef T_STR
#undef T_BOO
#undef T_ERR
#undef OPs

// -----------------------------------------------------------------------------
//Tabela de ATRIBUIÇÃO: atribTable[LHS][RHS] => OK_, WAR, ERR
//Regras usadas:
//int <- int/char => ERR; int <- float => WAR; int <- bool => ERR.
//float <- float => OK_; float <- int => WAR; float <- bool/str/char => ERR.
//char <- char => OK_; char <- todos => ERR.
//string <- string => OK_; demais => ERR.
//bool <- bool => OK_; todos => ERR.
// -----------------------------------------------------------------------------
const int SemanticTable::atribTable[5][5] =
    {
        /* RHS:      INT,         FLO,         CHA,         STR,         BOO       */
            /* LHS INT*/ { SemanticTable::OK_,  SemanticTable::WAR,  SemanticTable::ERR,  SemanticTable::ERR,  SemanticTable::ERR  },
            /* LHS FLO*/ { SemanticTable::WAR,  SemanticTable::OK_,  SemanticTable::ERR,  SemanticTable::ERR,  SemanticTable::ERR  },
            /* LHS CHA*/ { SemanticTable::ERR,  SemanticTable::ERR,  SemanticTable::OK_,  SemanticTable::ERR,  SemanticTable::ERR  },
            /* LHS STR*/ { SemanticTable::ERR,  SemanticTable::ERR,  SemanticTable::ERR,  SemanticTable::OK_,  SemanticTable::ERR  },
            /* LHS BOO*/ { SemanticTable::ERR,  SemanticTable::ERR,  SemanticTable::ERR,  SemanticTable::ERR,  SemanticTable::OK_  }
};
