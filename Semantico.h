#ifndef SEMANTICO_H
#define SEMANTICO_H

#include "Token.h"
#include "SemanticError.h"

#include <QString>
#include <QList>
#include <QStack>

#include "GeradorCodigo.h"
using namespace std;

class Simbolo {
public:
    QString tipo;
    QString nome;
    bool vetor;
    bool funcao;
    bool param;
    int escopo;
    bool inicializado;
    bool usado;

    int tam;

    Simbolo() {
        this->vetor = false;
        this->funcao = false;
        this->param = false;
        this->inicializado = false;
        this->usado = false;

        this->tam = 1;
    }
};

class Semantico
{
private:
    QString tipo;
    QString nome;

    QString lhsPendente;   //nome do LHS em andamento (a ser inicializado no #12)
    bool avisosGerados = false;

    QStack<int> pilhaTipos;
    QStack<int> pilhaOps;

    //suporte a relacionais
    QStack<QString> pilhaRelLhs;      //guarda temporários com o LHS
    QStack<QString> pilhaRelOper;     //guarda os operadores como string ("<", ">=", "==", ...)

    QStack<QString> pilhaRotulosIf;
    QStack<QString> pilhaRotulosElse; //guarda L_else

    //para controlar rótulos de laços while
    QStack<QString> pilhaWhileInicio;
    QStack<QString> pilhaWhileFim;

    QStack<QString> pilhaDoWhileInicio;

    QStack<QString> pilhaForInicio;  //rótulo onde começa a checagem da condição
    QStack<QString> pilhaForInc;     //rótulo do incremento
    QStack<QString> pilhaForCorpo;   //rótulo do corpo
    QStack<QString> pilhaForFim;     //rótulo de saída do laço

    //ionformação sobre o último relacional avaliado em uma expressão
    QString ultimoOperadorRelacional;   //"<", ">", "<=", ">=", "==", "!="
    bool exprAtualTemRelacional = false; //true se a expressão corrente usou relacional

    bool nomeExisteNoEscopoAtual(const QString& nome) const; //#9
    bool nomeExisteEmEscopoVisivel(const QString& nome) const; //#9

    void marcarUsadoMaisProximo(const QString& nome); //marca como usado o símbolo resolvido no #9

    void marcarInicializadoMaisProximo(const QString& nome);   //#12
    int  resolverIndexMaisProximo(const QString& nome) const;  //#12 retorna índice em escopo visível, ou -1

    //mapeamentos
    int tipoFromLiteralAction(int action) const;  //#20..#24 -> SemanticTable::Types

    void reduzirBinaria();      //#70: aplica tabela expTable
    void pushOper(int opCode);  //41..#64: empurra operador

    //assembly BIP
    GeradorCodigo gerador;   //buffer .data + .text
    QString operAtual;       //guarda "+" ou "-"
    bool temOperador = false;
    bool lhsEhVetor = false; //setado por #121

    QString idBaseVetAtual;

    int tamanhoVetorPend = 0;      //último tamanho literal lido por #13
    long long produtoDimPend = 0;  //pra se quiser tratar multi-dim: produto das dimensões

    QString idExprAtual;  //para #120 (RHS vetor)
    QString tempIdxLhs;   //índice do LHS de vetor (v[expr] = ...)

    QString vetorRhsNome;
    QString vetorRhsIdxTemp;
    bool vetorRhsPendente = false;

    //aqui pro suporte a operações com vetores em expressões:  a + v[i], v[i] + a, etc.
    bool lhsVetAtivo = false;   //indica se tem um LHS salvo para um acesso de vetor como segundo operando
    QString lhsVetTemp;         //temp onde guarda o ACC antes de começar a avaliar o índice
    bool opVetPendente = false; //diz se existe um operador "externo" pendente ( +, -, &, |, ^, <<, >> )
    QString operVetPend;        //operador pendente associado ao lhsVetTemp

    //“macros” de desvio condicional usando so instruções do BIP
    void geraBranchSeZero(const QString& rotulo);     //ACC == 0  → desvia
    void geraBranchSeNaoZero(const QString& rotulo);  //ACC != 0  → desvia

     void geraCodigoRelacional(int lhsType, int rhsType);

     //converte resultado de relacional (ACC = LHS - RHS) em 0/1
     //e opcionalmente já faz STO no destino (variável ou $out_port).
     //destino != ""  → atribuição/inicialização (r = (a<b);)
     //isOutput=true → cout/print (print(a<b);)
     //retorna true se já gerou todos os STO necessários.
     bool boolifyRelacionalParaAtribOuSaida(const QString &destino, bool isOutput);

     //aqui tem o suporte a rotinas / sub-rotinas
     QString funcaoAtual;        //nome da função/rotina cujo corpo está sendo gerado
     bool dentroFuncao = false;  //se esta dentro do corpo de uma função/rotina
     bool jmpPrincipalInserido = false; //se já inseriu "JMP _PRINCIPAL" no início do .text

     //chamadas de função com parâmetros por cópia
     QString funcaoChamadaAtual;   //nome da função sendo chamada (ex.: "DOBRO")
     int contParChamada = 0;       //índice do parâmetro atual (0,1,2,...)

     //suporte pro comando 'return'
     bool retornoTemExpr = false;  //setado por #611 / #612


     QString getNomeParametro(const QString& nomeFunc, int indice) const;
     int contarParametros(const QString& nomeFunc) const;
     int tipoParametro(const QString& nomeFunc, int indice) const;

     int tipoFromString(const QString& tipoStr) const;
     int tipoFuncaoAtual() const;



public:
    Semantico();

    QList<Simbolo> tabelaSimbolos;

    int contador;
    QStack<int> pilha;

    void executeAction(int action, const Token *token);

    void insereTabela();//#2

    QList<QString> avisos; //pra expor os avisos à IDE
    void gerarAvisosFinais();

    GeradorCodigo& getGeradorCodigo() { return gerador; }
    QString getAssembly() const { return gerador.getCodigo(); }

};

#endif

