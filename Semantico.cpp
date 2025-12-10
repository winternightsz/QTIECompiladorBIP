#include "Semantico.h"
#include "Constants.h"
#include "SemanticTable.h"
#include <iostream>

using namespace std;

#include <QDebug>
#include <QStack>

Semantico::Semantico() : contador(0) {
    this->pilha.push(0);
    gerador.limpa();
    tempIdxLhs.clear();

    exprAtualTemRelacional = false;
    ultimoOperadorRelacional.clear();

    funcaoAtual.clear();
    dentroFuncao = false;
    jmpPrincipalInserido = false;
}

void Semantico::executeAction(int action, const Token *token)
{
    //    string lexema = token->getLexeme();
    QString lexema = QString::fromStdString( token->getLexeme() );

    switch (action) {
    case 1: // TIPO
        this->tipo = lexema;
        break;
    case 2:{ // NOME do simbolo quando verificar o nome verificar unicidade.
        this->nome = lexema;
        this->insereTabela();

        produtoDimPend = 0;
        tamanhoVetorPend = 0;

        // Simbolo s;
        // s.tipo = this->tipo;
        // s.nome = this->nome;
        // s.escopo = this->pilha.top();
        // this->tabelaSimbolos.append(s);

        qDebug() << QString("Inserindo o simbolo: %1 do tipo: %2")
                        .arg(this->nome)
                        .arg(this->tipo);
        break;
    }
    case 13: { //tamanho literal do vetor durante a declaracao  int v[ 10 ] ...
        //guarda ultimo tamanho lido e pra multi-dim tem qwue acumular o produto
        bool ok = false;
        int tam = QString::fromStdString(token->getLexeme()).toInt(&ok);
        if (!ok || tam <= 0) {
            throw SemanticError("Tamanho de vetor inválido (#13)");
        }
        //para 1D:
        tamanhoVetorPend = tam;

        //pra multi-dim produtoDimPend = (produtoDimPend==0?1:produtoDimPend)*tam;
        break;
    }

    case 3: { //confirma que o símbolo atual é vetor
        if (tabelaSimbolos.isEmpty()) break;
        Simbolo& s = this->tabelaSimbolos.back();
        s.vetor = true;

        //aplica tamanho (1D) e pra multi-dim seria produtoDimPend (>=1)
        if (tamanhoVetorPend <= 0  && produtoDimPend==0 ) {
            throw SemanticError("Vetor sem tamanho válido (#3)");
        }

        //criou um campo 'tam' no Simbolo
        s.tam = (produtoDimPend>0 ? (int)produtoDimPend : tamanhoVetorPend);

        //zera pendências pra não “vazar” p/ próximo símbolo
         produtoDimPend = 0;
         tamanhoVetorPend = 0;
        break;
    }

    case 4:{ //aqui sei que é função.
        qDebug() << "Passando por aqui função!";
        Simbolo& s = this->tabelaSimbolos.back();
        s.funcao = true;
        break;
    }
    case 5:{ //aqui sei que é parametro.
        qDebug() << "Passando por aqui parametro!";
        Simbolo& s = this->tabelaSimbolos.back();
        s.param = true;
        s.inicializado = true; //<-parâmetros chegam inicializados pelo chamador senão pode ver avisos de “usado sem estar inicializado”
        s.escopo = this->contador + 1; //atribui escopo correto para o parametro
        break;
    }
    case 6:{ //inicialização em declaração e checa compatibilidade
        qDebug() << "Passando por aqui inicializado!";
        if (tabelaSimbolos.isEmpty()) break;
        Simbolo& s = this->tabelaSimbolos.back();

        if (pilhaTipos.isEmpty()) {
            throw SemanticError(QString("#6 Expressão sem tipo na inicialização de '%1'").arg(s.nome).toStdString());
        }
        int tipoRhs = pilhaTipos.pop();
        int tipoLhs =
            (s.tipo == "int")    ? SemanticTable::INT :
                (s.tipo == "float")  ? SemanticTable::FLO :
                (s.tipo == "double") ? SemanticTable::FLO :
                (s.tipo == "char")   ? SemanticTable::CHA :
                (s.tipo == "string") ? SemanticTable::STR :
                SemanticTable::BOO;

        int status = SemanticTable::atribType(tipoLhs, tipoRhs);
        if (status == SemanticTable::ERR) {
            auto toStr = [](int t)->QString{
                using T = SemanticTable::Types;
                switch(t){ case T::INT: return "int"; case T::FLO: return "float";
                case T::CHA: return "char"; case T::STR: return "string";
                case T::BOO: return "bool";  default: return "<?>";
                }
            };
            QString msg = QString("Tipos incompatíveis na inicialização: %1 = %2").arg(s.nome, toStr(tipoRhs));
            throw SemanticError(msg.toStdString());
        } else if (status == SemanticTable::WAR) {
            QString w = QString("Aviso: conversão implícita na inicialização de '%1'").arg(s.nome);
            if (!avisos.contains(w)) avisos.push_back(w);
        }
        bool handled = boolifyRelacionalParaAtribOuSaida(s.nome, /*isOutput=*/false);

        if (!handled) {
            //expressão NÃO era relacional → só grava o ACC normalmente
            //gerador.gera("STO", s.nome); //tirei pois tava duplicando
        }
        //no fim da expressão de init ACC tem o valor -> STO var
        gerador.gera("STO", s.nome);

        s.inicializado = true;
        break;
    }
    case 7: { //entra bloco: cria novo escopo
        this->contador += 1;
        this->pilha.push(this->contador);
        qDebug() << ">> ENTER escopo =" << this->contador;
        break;
    }
    case 8: {
        if (!this->pilha.isEmpty()) this->pilha.pop();
        qDebug() << "<< LEAVE escopo";

        //se voltou ao global (só resta o 0 na pilha) finaliza avisos
        if (this->pilha.size() == 1 && this->pilha.top() == 0) {
            if (!avisosGerados) {
                gerarAvisosFinais();     //<-- gera os dois tipos de aviso
                //avisosGerados = true; //precisei comentar pois não mostrava lista toda de avisos
                qDebug() << ">> Avisos gerados ao sair para o escopo global.";
            }
        }
        break;
    }
    case 9: { //USO (faz leitura): verifica declarado, marca usado
        QString idUsado = lexema;
        idExprAtual = lexema; //pra que #120 saiba qual é o vetor base

        //verifica se está declarado em escopo visível
        if (!nomeExisteEmEscopoVisivel(idUsado)) {
            QString msg = "Identificador '" + idUsado + "' não declarado no escopo visível";
            throw SemanticError(msg.toStdString());
        }

        //empurra tipo do id
        int idx = resolverIndexMaisProximo(idUsado);
        if (idx >= 0) {
            pilhaTipos.push(/*mapear QString tipo -> SemanticTable::Types*/
                            (tabelaSimbolos[idx].tipo == "int"    ? SemanticTable::INT :
                                 tabelaSimbolos[idx].tipo == "float"  ? SemanticTable::FLO :
                                 tabelaSimbolos[idx].tipo == "double" ? SemanticTable::FLO :  // pode mapear double para FLO por simplicidade
                                 tabelaSimbolos[idx].tipo == "char"   ? SemanticTable::CHA :
                                 tabelaSimbolos[idx].tipo == "string" ? SemanticTable::STR :
                                 SemanticTable::BOO));
        }

        //carrega/acumula no ACC conforme operador pendente
        if (!temOperador) {
            gerador.gera("LD", idUsado);
        } else {
            if      (operAtual == "+") gerador.gera("ADD", idUsado);
            else if (operAtual == "-") gerador.gera("SUB", idUsado);
            else if (operAtual == "&") gerador.gera("AND", idUsado);
            else if (operAtual == "|") gerador.gera("OR",  idUsado);
            else if (operAtual == "^") gerador.gera("XOR", idUsado);
            else if (operAtual == "<<") gerador.gera("SHL", idUsado);
            else if (operAtual == ">>") gerador.gera("SHR", idUsado);
            else if (operAtual == "*")  gerador.gera("MUL", idUsado);
            else if (operAtual == "/")  gerador.gera("DIV", idUsado);
            else if (operAtual == "%")  gerador.gera("MOD", idUsado);
            temOperador = false;
            operAtual.clear();
        }


        marcarUsadoMaisProximo(idUsado);
        break;
    }
    case 202: { //base de vetor ou função: v[...] ou f(...)
        QString idUsado = lexema;
        idExprAtual = idUsado;  // usado por #120
        idBaseVetAtual = idUsado;
        //se houver um operador pendente (ex.: a + v[...]),
        //salva o ACC (LHS) e o operador, e limpa operAtual/temOperador
        if (temOperador) {
            //guarda o valor atual do ACC (LHS da expressão externa)
            lhsVetTemp = gerador.novoTemp();
            gerador.gera("STO", lhsVetTemp);
            lhsVetAtivo = true;

            //guarda o operador externo ( +, -, &, |, ^, <<, >> )
            operVetPend   = operAtual;
            opVetPendente = true;

            //limpa contexto de operador global pra não interferir
            //na expressão do índice dentro dos colchetes
            temOperador = false;
            operAtual.clear();
        } else {
            lhsVetAtivo   = false;
            opVetPendente = false;
            operVetPend.clear();
        }

        //verifica se está declarado
        if (!nomeExisteEmEscopoVisivel(idUsado)) {
            QString msg = "Identificador '" + idUsado + "' não declarado no escopo visível";
            throw SemanticError(msg.toStdString());
        }

        //empurra tipo do id (tipo do elemento do vetor/retorno da função)
        int idx = resolverIndexMaisProximo(idUsado);
        if (idx >= 0) {
            pilhaTipos.push(
                (tabelaSimbolos[idx].tipo == "int")    ? SemanticTable::INT :
                    (tabelaSimbolos[idx].tipo == "float")  ? SemanticTable::FLO :
                    (tabelaSimbolos[idx].tipo == "double") ? SemanticTable::FLO :
                    (tabelaSimbolos[idx].tipo == "char")   ? SemanticTable::CHA :
                    (tabelaSimbolos[idx].tipo == "string") ? SemanticTable::STR :
                    SemanticTable::BOO
                );
        }

        //não gera LD aqui, quem vai carregar é o #120 (LDV base[$indr])
        marcarUsadoMaisProximo(idUsado);
        break;
    }

    case 10: {
        break;
    }
    case 11: { //LHS: verifica declarado, não marca uso, não inicializa ainda
        QString idEscrito = lexema;
        if (!nomeExisteEmEscopoVisivel(idEscrito)) {
            QString msg = "Identificador '" + idEscrito + "' não declarado no escopo visível";
            throw SemanticError(msg.toStdString());
        }
        //guarda para inicializar só depois da RHS (#12):
        lhsPendente = idEscrito;
        break;
    }
    case 12: { // final da atribuição OU cin >> id
        if (!lhsPendente.isEmpty()) {
            qDebug() << "#12 entrando, pilhaTipos.size() =" << pilhaTipos.size();
            //RHS DEVE ter tipo na pilha
            if (pilhaTipos.isEmpty()) {
                throw SemanticError("Expressão da atribuição sem tipo (RHS vazio) #12");
            }

            int tipoRhs = pilhaTipos.pop();

            int idx = resolverIndexMaisProximo(lhsPendente);
            if (idx >= 0) {
                int tipoLhs =
                    (tabelaSimbolos[idx].tipo == "int")    ? SemanticTable::INT :
                        (tabelaSimbolos[idx].tipo == "float")  ? SemanticTable::FLO :
                        (tabelaSimbolos[idx].tipo == "double") ? SemanticTable::FLO :
                        (tabelaSimbolos[idx].tipo == "char")   ? SemanticTable::CHA :
                        (tabelaSimbolos[idx].tipo == "string") ? SemanticTable::STR :
                        SemanticTable::BOO;

                int status = SemanticTable::atribType(tipoLhs, tipoRhs);
                if (status == SemanticTable::ERR) {
                    auto toStr = [](int t)->QString{
                        using T = SemanticTable::Types;
                        switch(t){ case T::INT: return "int"; case T::FLO: return "float";
                        case T::CHA: return "char"; case T::STR: return "string";
                        case T::BOO: return "bool";  default: return "<?>"; }
                    };
                    QString msg = QString("Tipos incompatíveis na atribuição: %1 = %2")
                                      .arg(tabelaSimbolos[idx].tipo, toStr(tipoRhs));
                    throw SemanticError(msg.toStdString());
                } else if (status == SemanticTable::WAR) {
                    QString w = QString("Aviso: conversão implícita na atribuição para '%1'")
                                    .arg(lhsPendente);
                    if (!avisos.contains(w)) avisos.push_back(w);
                }
            }
            bool handled = boolifyRelacionalParaAtribOuSaida(lhsPendente, /*isOutput=*/false);

            if (!handled) {
                //nao era relacional → segue lógica normal (vetor/escalar)

                //vetor
                if (lhsEhVetor) {
                    //ACC contém o valor da expressão do lado direito (RHS).
                    if (tempIdxLhs.isEmpty()) {
                        throw SemanticError("Índice de vetor não encontrado para LHS (#12/#115)");
                    }

                    //guarda RHS em um temporário
                    QString tempVal = gerador.novoTemp();
                    gerador.gera("STO", tempVal);

                    //reconfigura $indr com o índice do LHS (calculado em #115)
                    gerador.gera("LD", tempIdxLhs);
                    gerador.gera("STO", "$indr");

                    //recarrega RHS e escreve no vetor
                    gerador.gera("LD", tempVal);
                    gerador.gera("STOV", lhsPendente);

                    lhsEhVetor = false;
                    tempIdxLhs.clear();
                } else {
                    gerador.gera("STO", lhsPendente);
                }

            } else {
                //se handled == true, o helper já fez os STO lhsPendente
                //e não precisa mexer em lhsEhVetor aqui (não faz sentido r = v[i] < x para vetor LHS)
                lhsEhVetor = false;
                tempIdxLhs.clear();
            }
            //fim vetor

            marcarInicializadoMaisProximo(lhsPendente);
            lhsPendente.clear();
        } else {
            //CIN >> id  gramática dispara #12 ANTES de <caso_vetor>,
            //então aqui trata somente escalar — cin >> v[i] exigiria #12 após as dimensões
            QString idCin = lexema;
            gerador.gera("LD", "$in_port");
            gerador.gera("STO", idCin);
            marcarInicializadoMaisProximo(idCin);
        }
        break;
    }

    //literais:
    case 20:{
        qDebug() << "#20 LIT_INTEIRO -> push INT";
        int t = tipoFromLiteralAction(action);
        pilhaTipos.push(t);

        if (!temOperador) {
            //começo de expressão: só carrega imediato
            gerador.gera("LDI", lexema);
        } else {
            if (operAtual == "+" )      gerador.gera("ADDI", lexema);
            else if (operAtual == "-" ) gerador.gera("SUBI", lexema);
            else if (operAtual == "&" || operAtual == "|" || operAtual == "^") {
                //Não tem ANDI/ORI/XORI, então:
                QString bitTmp = gerador.novoTemp();
                gerador.gera("STO", bitTmp);          //guarda ACC (lado esquerdo)
                gerador.gera("LDI", lexema);            //carrega o literal
                if      (operAtual == "&") gerador.gera("AND", bitTmp);
                else if (operAtual == "|") gerador.gera("OR",  bitTmp);
                else                        gerador.gera("XOR", bitTmp);
            }
            //multiplicação por literal mas teria que mudar aqui pra ADD porque nao tem MUL no BIP
            else if (operAtual == "*") {
                QString tLhs = gerador.novoTemp();
                gerador.gera("STO", tLhs);       //guarda LHS (já estava em ACC)
                gerador.gera("LDI", lexema);     //carrega RHS literal
                gerador.gera("MUL", tLhs);       //ACC = RHS * LHS  (comutativo)
            }
            temOperador = false;
            operAtual.clear();
        }

        break;
    }
    case 21:{
        qDebug() << "#21 LIT_DECIMAIS -> push FLO";
        int t = tipoFromLiteralAction(action);
        pilhaTipos.push(t);

        //simples: trunca para inteiro no BIP
        QString inteiro = lexema.section('.',0,0);
        if (!temOperador) gerador.gera("LDI", inteiro);
        else {
            if (operAtual == "+")      gerador.gera("ADDI", inteiro);
            else if (operAtual == "-") gerador.gera("SUBI", inteiro);
            temOperador = false;
            operAtual.clear();
        }

        break;
    }
    case 22:{
        qDebug() << "#21 LIT_CHAR -> push CHAR";
        int t = tipoFromLiteralAction(action);
        pilhaTipos.push(t);
        break;
    }
    case 23:{
        qDebug() << "#23 LIT_STRING -> push STR";
        int t = tipoFromLiteralAction(action);
        pilhaTipos.push(t);
        break;
    }
    case 24: {
        int t = tipoFromLiteralAction(action);
        pilhaTipos.push(t);
        QString v = (lexema.toLower() == "true") ? "1" : "0";
        if (!temOperador) {
            gerador.gera("LDI", v);
        } else {
            if      (operAtual == "+") gerador.gera("ADDI", v);
            else if (operAtual == "-") gerador.gera("SUBI", v);
            else if (operAtual == "&" || operAtual == "|" || operAtual == "^") {
                //fallback sem ANDI/ORI/XORI
                QString bitTmp = gerador.novoTemp();
                gerador.gera("STO", bitTmp); // guarda ACC (LHS)
                gerador.gera("LDI", v);        // carrega o literal (RHS)
                if      (operAtual == "&") gerador.gera("AND", bitTmp);
                else if (operAtual == "|") gerador.gera("OR",  bitTmp);
                else                       gerador.gera("XOR", bitTmp);
            }
            temOperador = false;
            operAtual.clear();
        }
        break;
    }
    //operadores
    // operadores + e -
    case 41: operAtual = "+"; temOperador = true; break; // +
    case 42: operAtual = "-"; temOperador = true; break; // -
    // operadores de alta precedência:
    case 43: //*
        operAtual = "*";
        temOperador = true;
        pushOper(SemanticTable::MUL);
        break;

    case 44: ///
        operAtual = "/";
        temOperador = true;
        pushOper(SemanticTable::DIV);
        break;

    case 45: //%
        operAtual = "%";
        temOperador = true;
        pushOper(SemanticTable::MOD);
        break;

    case 46: //OP_SHR >>
        operAtual = ">>"; temOperador = true;
        pushOper(SemanticTable::ROO);
        break;

    case 47: //OP_SHL <<
        operAtual = "<<"; temOperador = true;
        pushOper(SemanticTable::POT);
        break;

    case 48: { //OP_MAIOR '>'
        //ACC contém o resultado da expressão à esquerda
        QString tLhs = gerador.novoTemp();
        gerador.gera("STO", tLhs);

        pilhaRelLhs.push(tLhs);
        pilhaRelOper.push(">");

        pushOper(SemanticTable::REL);
        break;
    }
    case 49: { //OP_MENOR '<'
        QString tLhs = gerador.novoTemp();
        gerador.gera("STO", tLhs);

        pilhaRelLhs.push(tLhs);
        pilhaRelOper.push("<");

        pushOper(SemanticTable::REL);
        break;
    }
    case 50: { //OP_MAIORIGUAL '>='
        QString tLhs = gerador.novoTemp();
        gerador.gera("STO", tLhs);

        pilhaRelLhs.push(tLhs);
        pilhaRelOper.push(">=");

        pushOper(SemanticTable::REL);
        break;
    }
    case 51: { //OP_MENORIGUAL '<='
        QString tLhs = gerador.novoTemp();
        gerador.gera("STO", tLhs);

        pilhaRelLhs.push(tLhs);
        pilhaRelOper.push("<=");

        pushOper(SemanticTable::REL);
        break;
    }
    case 52: { //OP_IGUALDADE '=='
        QString tLhs = gerador.novoTemp();
        gerador.gera("STO", tLhs);

        pilhaRelLhs.push(tLhs);
        pilhaRelOper.push("==");

        pushOper(SemanticTable::REL);
        break;
    }
    case 53: { //OP_DIFERENTE '!='
        QString tLhs = gerador.novoTemp();
        gerador.gera("STO", tLhs);

        pilhaRelLhs.push(tLhs);
        pilhaRelOper.push("!=");

        pushOper(SemanticTable::REL);
        break;
    }

    case 60: pushOper(SemanticTable::AND); break;
    case 61: pushOper(SemanticTable::OR_); break;
    // bit_or/bit_xor/bit_and:
    case 64: //OP_BIT_AND
        operAtual = "&";  temOperador = true;
        //salvaLHSSePreciso();
        pushOper(SemanticTable::BIT_AND);
        break;

    case 62: //OP_BIT_OR
        operAtual = "|";  temOperador = true;
        //salvaLHSSePreciso();
        pushOper(SemanticTable::BIT_OR);
        break;

    case 63: //OP_BIT_XOR
        operAtual = "^";  temOperador = true;
        //salvaLHSSePreciso();
        pushOper(SemanticTable::BIT_XOR);
        break;



    case 70: { //reduzir binária no nível atual
        reduzirBinaria();
        break;
    }

    case 200: { //após fechar <comando_saida>
        qDebug() << "#200";
        //assume que ACC tem o valor da expressão
        bool handled = boolifyRelacionalParaAtribOuSaida(/*destino=*/QString(), /*isOutput=*/true);

        if (!handled) {
            //não era relacional → só manda ACC para a porta de saída
            gerador.gera("STO", "$out_port");
        }
        break;
    }
    case 201: {

        //pra cada símbolo não-função, emite a linha apropriada
        for (const Simbolo& s : tabelaSimbolos) {
            if (s.funcao) continue; // função não vai pra .data

            if (s.vetor) {
                //vtor: usa o tamanho coletado no #13 (s.tam)
                //ex.:  v: 0,0,0,0
                QStringList zeros;
                for (int k = 0; k < s.tam; k++)
                    zeros << "0";

                QString lista = zeros.join(", ");
                gerador.geraData(s.nome, lista);

            } else {
                //escalar: inicializa com 0
                //ex.:  x: 0
                gerador.geraData(s.nome, "0");
            }
        }
        qDebug() << "#201";
        gerador.gera("HLT", "0");
        break;
    }
    case 115: { //índice computado dentro de LHS: v[ expr ] = ...
        //nesse momento, o valor da <expressao> (índice) está no ACC.
        tempIdxLhs = gerador.novoTemp(); // ex.: _t3
        gerador.gera("STO", tempIdxLhs);

        //já configura o registrador de indireção: tirei pois duplicando
        //gerador.gera("LD",  tempIdxLhs);
        //gerador.gera("STO", "$indr");
        break;
    }

    case 121: { //confirmou que o LHS é vetor (já fechou ']')
        lhsEhVetor = true;
        break;
    }
    case 120: {
        //base *obrigatoriamente* veio do #202 (ID do vetor ou função)
        if (idBaseVetAtual.isEmpty()) {
            throw SemanticError("Acesso a vetor/função sem base registrada em #120");
        }
        QString base = idBaseVetAtual;
        //ACC contém o resultado da expressão do índice:  expr_idx
        //idExprAtual é o nome do vetor (ex.: "v")
        //se opVetPendente == true, tem:
        //      lhsVetTemp  = LHS externo salvo em #202 (ex.: valor de 'x' em x + v[i])
        //      operVetPend = operador externo ('+', '-', '&', '|', '^', '<<', '>>')

        //guarda o índice em um temporário
        //vetorRhsIdxTemp = gerador.novoTemp();
        //gerador.gera("STO", vetorRhsIdxTemp);

        //configura o registrador de indireção com o índice
        //gerador.gera("LD",  vetorRhsIdxTemp);

        //tirei pois nao faz mais aqui
        gerador.gera("STO", "$indr");

        if (!opVetPendente) {
            //caso normal: o acesso a vetor é o primeiro (ou único) operando da expressão.
            //só carrega o elemento em ACC e pronto.
            gerador.gera("LDV", base);   //ACC = idExprAtual[$indr]
        } else {
            //tem um operador externo (ex.: a + v[i]) e um LHS salvo em lhsVetTemp.

            //vai carregar o elemento do vetor como RHS em ACC
            gerador.gera("LDV", base);   // ACC = RHS = vetor[idr]

            //vai combinra ACC (RHS) com o LHS salvo em lhsVetTemp usando o operador operVetPend
            if (operVetPend == "+") {
                //pra soma, tanto LHS + RHS quanto RHS + LHS são equivalentes
                gerador.gera("ADD", lhsVetTemp);   // ACC = RHS + LHS
            }
            else if (operVetPend == "-") {
                //pra subtração, precisa de ACC = LHS - RHS
                QString tRhs = gerador.novoTemp();
                gerador.gera("STO", tRhs);       //guarda RHS
                gerador.gera("LD", lhsVetTemp);  // ACC = LHS
                gerador.gera("SUB", tRhs);       //ACC = LHS - RHS
            }
            else if (operVetPend == "&") {
                QString tRhs = gerador.novoTemp();
                gerador.gera("STO", tRhs);       //RHS
                gerador.gera("LD", lhsVetTemp);  //LHS
                gerador.gera("AND", tRhs);       //ACC = LHS & RHS
            }
            else if (operVetPend == "|") {
                QString tRhs = gerador.novoTemp();
                gerador.gera("STO", tRhs);
                gerador.gera("LD", lhsVetTemp);
                gerador.gera("OR", tRhs);        //ACC = LHS | RHS
            }
            else if (operVetPend == "^") {
                QString tRhs = gerador.novoTemp();
                gerador.gera("STO", tRhs);
                gerador.gera("LD", lhsVetTemp);
                gerador.gera("XOR", tRhs);       //ACC = LHS ^ RHS
            }
            else if (operVetPend == "<<") {
                QString tRhs = gerador.novoTemp();
                gerador.gera("STO", tRhs);       //RHS = quantidade de shift
                gerador.gera("LD", lhsVetTemp);  //LHS
                gerador.gera("SHL", tRhs);       //ACC = LHS << RHS
            }
            else if (operVetPend == ">>") {
                QString tRhs = gerador.novoTemp();
                gerador.gera("STO", tRhs);       //RHS
                gerador.gera("LD", lhsVetTemp);  //LHS
                gerador.gera("SHR", tRhs);       //ACC = LHS >> RHS
            }
            else if (operVetPend == "*") {
                QString tRhs = gerador.novoTemp();
                gerador.gera("STO", tRhs);       //RHS = vetor[$indr]
                gerador.gera("LD", lhsVetTemp);  //LHS
                gerador.gera("MUL", tRhs);       //ACC = LHS * RHS
            }
            else if (operVetPend == "/") {
                QString tRhs = gerador.novoTemp();
                gerador.gera("STO", tRhs);       //RHS
                gerador.gera("LD", lhsVetTemp);  //LHS
                gerador.gera("DIV", tRhs);       //ACC = LHS / RHS
            }
            else if (operVetPend == "%") {
                QString tRhs = gerador.novoTemp();
                gerador.gera("STO", tRhs);       //RHS
                gerador.gera("LD", lhsVetTemp);  //LHS
                gerador.gera("MOD", tRhs);       //ACC = LHS % RHS
            }

            //limpa o estado pendente de vetor + operador
            opVetPendente = false;
            operVetPend.clear();
            lhsVetAtivo = false;

            idBaseVetAtual.clear();
        }

        break;
    }

    case 300: {
        //início de um IF:
        //ACC contém o valor da condição:
        //se houve relacional: ACC = LHS - RHS
        //se não houve: ACC = expr (true se != 0)

        //cria um rótulo para o caso FALSO (ou fim do IF se não houver ELSE)
        QString rotFalse = gerador.novoRotulo();

        //empilha:
        //pilhaRotulosElse: rótulo do caso falso
        //pilhaRotulosIf:   rótulo de fim (por enquanto string vazia = "não sei se tem else")
        pilhaRotulosElse.push(rotFalse);
        pilhaRotulosIf.push(QString()); //ainda não sabe se vai ter ELSE

        if (exprAtualTemRelacional) {
            //tem ACC = LHS - RHS, usa operador guardado
            if      (ultimoOperadorRelacional == ">")
                gerador.gera("BLE", rotFalse);  //falso quando LHS <= RHS
            else if (ultimoOperadorRelacional == "<")
                gerador.gera("BGE", rotFalse);  //falso quando LHS >= RHS
            else if (ultimoOperadorRelacional == ">=")
                gerador.gera("BLT", rotFalse);  //falso quando LHS < RHS
            else if (ultimoOperadorRelacional == "<=")
                gerador.gera("BGT", rotFalse);  //falso quando LHS > RHS
            else if (ultimoOperadorRelacional == "==")
                gerador.gera("BNE", rotFalse);  //falso quando LHS != RHS
            else if (ultimoOperadorRelacional == "!=")
                gerador.gera("BEQ", rotFalse);  //falso quando LHS == RHS
            else
                geraBranchSeZero(rotFalse);     //fallback de segurança
        } else {
            //if (expr) sem relacional → falso se expr == 0
            geraBranchSeZero(rotFalse);
        }

        //limpa estado de relacional pra a próxima expressão
        exprAtualTemRelacional = false;
        ultimoOperadorRelacional.clear();

        break;
    }



    case 301: {
        //fim do bloco THEN; a decisão sobre ELSE/sem-ELSE
        //será tomada em #303 (se houver KEY_ELSE) e em #302.
        break;
    }

    case 302: {
        //final do IF (após o bloco ELSE ou após "else vazio")
        if (pilhaRotulosIf.isEmpty() || pilhaRotulosElse.isEmpty()) {
            throw SemanticError("Erro interno: pilhas vazias em #302 (if)");
        }

        QString rotEnd   = pilhaRotulosIf.pop();
        QString rotFalse = pilhaRotulosElse.pop();

        if (rotEnd.isEmpty()) {
            //IF sem ELSE:
            //o rótulo rotFalse é o próprio fim do IF
            gerador.geraRotulo(rotFalse);
        } else {
            //IF com ELSE:
            //rotFalse já foi rotulado em #303 (início do else)
            //aqui marca só o fim do IF
            gerador.geraRotulo(rotEnd);
        }

        break;
    }


    case 303: {
        //encontra um ELSE para o IF corrente.
        //pilhaRotulosElse.top() = rótulo do caso falso (L_false)
        //pilhaRotulosIf.top()   = "" (pois ainda não sabia se teria ELSE)

        if (pilhaRotulosIf.isEmpty() || pilhaRotulosElse.isEmpty()) {
            throw SemanticError("Erro interno: pilhas vazias em #303 (else)");
        }

        QString rotFalse = pilhaRotulosElse.top(); //não desempilha ainda
        QString rotEnd   = gerador.novoRotulo();   //rótulo de fim do IF

        //substitui o sentinel "" do topo de pilhaRotulosIf por rotEnd
        pilhaRotulosIf.pop();
        pilhaRotulosIf.push(rotEnd);

        //quando terminar o THEN, deve pular o bloco ELSE
        gerador.gera("JMP", rotEnd);
        //e aqui começa o bloco ELSE
        gerador.geraRotulo(rotFalse);

        break;
    }


    case 400: {
        //início do laço while:
        //cria dois rótulos: um para o começo do laço e um para o fim.
        QString rotInicio = gerador.novoRotulo();
        QString rotFim    = gerador.novoRotulo();

        //empilha para suportar laços aninhados
        pilhaWhileInicio.push(rotInicio);
        pilhaWhileFim.push(rotFim);

        //marca o rótulo de início ANTES de avaliar a condição
        gerador.geraRotulo(rotInicio);
        break;
    }
    case 401: {
        //após avaliar a condição do while: ACC contém 0/!=0 ou (LHS - RHS) se for relacional
        if (pilhaWhileFim.isEmpty()) {
            throw SemanticError("Erro interno: pilhaWhileFim vazia em #401 (while)");
        }

        QString rotFim = pilhaWhileFim.top();  // ainda não desempilha

        if (exprAtualTemRelacional) {
            //while (A op B) { ... }
            //ffalso → sair do laço

            if      (ultimoOperadorRelacional == ">")
                gerador.gera("BLE", rotFim);  // falso: A <= B
            else if (ultimoOperadorRelacional == "<")
                gerador.gera("BGE", rotFim);  // falso: A >= B
            else if (ultimoOperadorRelacional == ">=")
                gerador.gera("BLT", rotFim);  // falso: A < B
            else if (ultimoOperadorRelacional == "<=")
                gerador.gera("BGT", rotFim);  // falso: A > B
            else if (ultimoOperadorRelacional == "==")
                gerador.gera("BNE", rotFim);  // falso: A != B
            else if (ultimoOperadorRelacional == "!=")
                gerador.gera("BEQ", rotFim);  // falso: A == B
            else
                geraBranchSeZero(rotFim);
        } else {
            //while (expr) → sai se expr == 0
            geraBranchSeZero(rotFim);
        }

        exprAtualTemRelacional = false;
        ultimoOperadorRelacional.clear();

        break;
    }

    case 402: {
        //fim do corpo do while
        if (pilhaWhileInicio.isEmpty() || pilhaWhileFim.isEmpty()) {
            throw SemanticError("Erro interno: pilhas de while vazias em #402 (while)");
        }

        QString rotInicio = pilhaWhileInicio.pop();
        QString rotFim    = pilhaWhileFim.pop();

        //volta para o início do laço
        gerador.gera("JMP", rotInicio);

        //marca o rótulo de saída do laço
        gerador.geraRotulo(rotFim);

        break;
    }

    case 410: {
        //após <inicializacao_for>
        //cria os rótulos do for:
        //rotCond  : ponto onde avalia a condição
        //rotInc   : ponto do incremento
        //rotCorpo : início do corpo do laço
        //rotFim   : saída do laço
        QString rotCond  = gerador.novoRotulo();
        QString rotInc   = gerador.novoRotulo();
        QString rotCorpo = gerador.novoRotulo();
        QString rotFim   = gerador.novoRotulo();

        //empilha para suportar fors aninhados:
        pilhaForInicio.push(rotCond);    // aqui "Inicio" = condição
        pilhaForInc.push(rotInc);
        pilhaForCorpo.push(rotCorpo);
        pilhaForFim.push(rotFim);

        //logo após a inicialização, começa avaliando a condição
        gerador.geraRotulo(rotCond);
        break;
    }

    case 411: {
        //após a expressão de condição do for (ACC = 0/!=0 ou LHS - RHS)
        if (pilhaForFim.isEmpty() || pilhaForCorpo.isEmpty()) {
            throw SemanticError("Erro interno: pilhas do for vazias em #411");
        }

        QString rotFim   = pilhaForFim.top();
        QString rotCorpo = pilhaForCorpo.top();

        if (exprAtualTemRelacional) {
            // for (...; A op B; ...) { ... }
            // condição falsa → vai para rotFim
            if      (ultimoOperadorRelacional == ">")
                gerador.gera("BLE", rotFim);
            else if (ultimoOperadorRelacional == "<")
                gerador.gera("BGE", rotFim);
            else if (ultimoOperadorRelacional == ">=")
                gerador.gera("BLT", rotFim);
            else if (ultimoOperadorRelacional == "<=")
                gerador.gera("BGT", rotFim);
            else if (ultimoOperadorRelacional == "==")
                gerador.gera("BNE", rotFim);
            else if (ultimoOperadorRelacional == "!=")
                gerador.gera("BEQ", rotFim);
            else
                geraBranchSeZero(rotFim);
        } else {
            //for (...; expr; ...) → condição falsa se expr == 0
            geraBranchSeZero(rotFim);
        }

        //se passou do desvio, condição era verdadeira → vai para o corpo
        gerador.gera("JMP", rotCorpo);

        exprAtualTemRelacional = false;
        ultimoOperadorRelacional.clear();

        break;
    }


    case 412: {
        //marca o início do código de incremento
        if (pilhaForInc.isEmpty()) {
            throw SemanticError("Erro interno: pilhaForInc vazia em #412");
        }

        QString rotInc = pilhaForInc.top();
        gerador.geraRotulo(rotInc);

        //a seguir vem o código normal da <atribuicao> de incremento
        break;
    }

    case 413: {
        //terminou o incremento: agora volta para a condição e
        //logo depois marca o corpo do laço
        if (pilhaForInicio.isEmpty() || pilhaForCorpo.isEmpty()) {
            throw SemanticError("Erro interno: pilhas do for vazias em #413");
        }

        QString rotCond  = pilhaForInicio.top();
        QString rotCorpo = pilhaForCorpo.top();

        //após o incremento, volta para o teste de condição
        gerador.gera("JMP", rotCond);

        //agora marca o início do corpo do laço
        gerador.geraRotulo(rotCorpo);

        //em seguida virá o <bloco_comandos>
        break;
    }

    case 414: {
        //fim do corpo: desvia para o incremento, e depois sai em rotFim
        if (pilhaForInicio.isEmpty() || pilhaForInc.isEmpty() ||
            pilhaForFim.isEmpty()    || pilhaForCorpo.isEmpty()) {
            throw SemanticError("Erro interno: pilhas do for vazias em #414");
        }

        QString rotCond  = pilhaForInicio.pop();
        QString rotInc   = pilhaForInc.pop();
        QString rotCorpo = pilhaForCorpo.pop();
        QString rotFim   = pilhaForFim.pop();

        Q_UNUSED(rotCond);
        Q_UNUSED(rotCorpo);

        //ao final do corpo, desvia para o incremento
        gerador.gera("JMP", rotInc);

        //marca o fim do laço
        gerador.geraRotulo(rotFim);

        break;
    }



    case 500: {
        //marca o início do bloco do-while
        QString rotInicio = gerador.novoRotulo();
        pilhaDoWhileInicio.push(rotInicio);
        gerador.geraRotulo(rotInicio);
        break;
    }

    case 502: {
        if (pilhaDoWhileInicio.isEmpty()) {
            throw SemanticError("Erro interno: pilhaDoWhileInicio vazia em #502 (do-while)");
        }

        QString rotInicio = pilhaDoWhileInicio.pop();

        //após a expressão, ACC contém 0/!=0 ou (LHS - RHS) se for relacional
        if (exprAtualTemRelacional) {
            //do { ... } while (A op B);
            //quer VOLTAR ao início quando A op B é VERDADEIRO.

            if      (ultimoOperadorRelacional == ">")
                gerador.gera("BGT", rotInicio);  // verdadeiro: A > B
            else if (ultimoOperadorRelacional == "<")
                gerador.gera("BLT", rotInicio);  // verdadeiro: A < B
            else if (ultimoOperadorRelacional == ">=")
                gerador.gera("BGE", rotInicio);  // verdadeiro: A >= B
            else if (ultimoOperadorRelacional == "<=")
                gerador.gera("BLE", rotInicio);  // verdadeiro: A <= B
            else if (ultimoOperadorRelacional == "==")
                gerador.gera("BEQ", rotInicio);  // verdadeiro: A == B
            else if (ultimoOperadorRelacional == "!=")
                gerador.gera("BNE", rotInicio);  // verdadeiro: A != B
            else
                geraBranchSeNaoZero(rotInicio);  // fallback
        } else {
            //do { ... } while (expr); → repete se expr != 0
            geraBranchSeNaoZero(rotInicio);
        }

        exprAtualTemRelacional = false;
        ultimoOperadorRelacional.clear();

        break;
    }


    case 600: {
        //iinício do corpo de uma função/rotina
        //precisa descobrir qual é a função dona deste bloco.
        int idxFunc = -1;
        for (int i = tabelaSimbolos.size() - 1; i >= 0; --i) {
            if (tabelaSimbolos[i].funcao) { // marcado em #4
                idxFunc = i;
                break;
            }
        }
        if (idxFunc < 0) {
            qDebug() << "Aviso interno: #600 chamado sem função marcada";
            break;
        }

        funcaoAtual = tabelaSimbolos[idxFunc].nome;
        dentroFuncao = true;

        //função main() é o ponto de entrada
        bool ehPrincipal = (funcaoAtual == "main");

        //quando saber que existe principal, coloca:
        //.text
        //JMP _MAIN
        // ...
        if (ehPrincipal && !jmpPrincipalInserido) {
            gerador.geraInicioText("JMP", "_MAIN");
            jmpPrincipalInserido = true;
        }

        //define o rótulo de entrada da rotina
        if (ehPrincipal) {
            gerador.geraRotulo("_MAIN");
        } else {
            QString rotuloFunc = "_" + funcaoAtual;
            gerador.geraRotulo(rotuloFunc);
        }

        break;
    }

    case 601: {
        //fim do corpo de uma função/rotina
        if (!dentroFuncao)
            break;

        bool ehPrincipal = (funcaoAtual == "main");

        //rotinas "normais" terminam com RETURN 0
        if (!ehPrincipal) {
            gerador.gera("RETURN", "0");
        }
        //a main NÃO dá RETURN — o HLT final continua vindo do #201.

        dentroFuncao = false;
        funcaoAtual.clear();
        break;
    }

    case 602: {
        //início de uma chamada de função/procedimento: ID(...)
        QString nomeFunc = lexema;

        int idxFunc = resolverIndexMaisProximo(nomeFunc);
        if (idxFunc < 0) {
            QString msg = "Rotina/função não declarada: " + nomeFunc;
            throw SemanticError(msg.toStdString());
        }

        if (!tabelaSimbolos[idxFunc].funcao) {
            QString msg = "Identificador não é rotina/função: " + nomeFunc;
            throw SemanticError(msg.toStdString());
        }

        //guarda o nome da função para os argumentos (#603) e para o CALL (#605)
        funcaoChamadaAtual = nomeFunc;
        contParChamada = 0;

        //IMPORTANTE: não gera CALL aqui.
        //O CALL será emitido em #605, depois dos argumentos.
        break;
    }

    case 603: {
        //fim de uma <expressao> usada como argumento de chamada de função.

        if (funcaoChamadaAtual.isEmpty()) {
            //se acontecer, é bug de gramática/ações.
            qDebug() << "Aviso interno: #603 chamado sem funcaoChamadaAtual";
            break;
        }

        //nome do parâmetro de índice contParChamada
        QString nomeParam = getNomeParametro(funcaoChamadaAtual, contParChamada);
        if (nomeParam.isEmpty()) {
            QString msg = QString("Número de argumentos maior que parâmetros na chamada de '%1'")
                              .arg(funcaoChamadaAtual);
            throw SemanticError(msg.toStdString());
        }

        //tipo da expressão do argumento (RHS)
        if (pilhaTipos.isEmpty()) {
            throw SemanticError("Expressão de argumento sem tipo na pilha (#603)");
        }
        int tipoArg = pilhaTipos.pop();

        //tipo do parâmetro (LHS)
        int tipoParam = tipoParametro(funcaoChamadaAtual, contParChamada);

        using ST = SemanticTable;
        int status = ST::atribType(tipoParam, tipoArg);

        auto toStr = [](int t)->QString{
            using T = SemanticTable::Types;
            switch (t) {
            case T::INT: return "int";
            case T::FLO: return "float";
            case T::CHA: return "char";
            case T::STR: return "string";
            case T::BOO: return "bool";
            default:     return "<?>"; }
        };

        if (status == ST::ERR) {
            QString msg = QString(
                              "Tipo incompatível no argumento %1 da chamada de '%2': esperado %3, encontrado %4")
                              .arg(contParChamada + 1)
                              .arg(funcaoChamadaAtual)
                              .arg(toStr(tipoParam))
                              .arg(toStr(tipoArg));
            throw SemanticError(msg.toStdString());
        } else if (status == ST::WAR) {
            QString w = QString(
                            "Aviso: conversão implícita no argumento %1 da chamada de '%2'")
                            .arg(contParChamada + 1)
                            .arg(funcaoChamadaAtual);
            if (!avisos.contains(w)) avisos.push_back(w);
        }

        //ACC já contém o valor do argumento -> cópia por valor para o parâmetro
        gerador.gera("STO", nomeParam);

        // Próximo parâmetro
        contParChamada++;
        break;
    }

    case 605: {
        if (funcaoChamadaAtual.isEmpty()) {
            qDebug() << "Aviso interno: #605 sem funcaoChamadaAtual";
            break;
        }

        int totalParams = contarParametros(funcaoChamadaAtual);
        if (contParChamada < totalParams) {
            QString msg = QString(
                              "Faltam argumentos na chamada de '%1': esperados %2, recebidos %3")
                              .arg(funcaoChamadaAtual)
                              .arg(totalParams)
                              .arg(contParChamada);
            throw SemanticError(msg.toStdString());
        }
        //se contParChamada > totalParams, já teria estourado em #603

        QString rotuloDestino;
        if (funcaoChamadaAtual == "main") {
            rotuloDestino = "_MAIN";
        } else {
            rotuloDestino = "_" + funcaoChamadaAtual;
        }

        gerador.gera("CALL", rotuloDestino);

        funcaoChamadaAtual.clear();
        contParChamada = 0;
        break;
    }


    case 611: {
        //expressao_opcional -> <expressao>
        //quer dizer: houve uma expressão depois do 'return'
        retornoTemExpr = true;
        break;
    }

    case 612: {
        //expressao_opcional -> vazio
        //quer dizer: 'return;' sem expressão
        retornoTemExpr = false;
        break;
    }

    case 610: {
        //comando_return: KEY_RETURN <expressao_opcional> #610 ';'

        if (!dentroFuncao) {
            throw SemanticError("Uso de 'return' fora de uma função");
        }

        //descobre símbolo e tipo da função atual
        QString tipoFuncStr("void");
        const Simbolo* funcSym = nullptr;
        for (int i = tabelaSimbolos.size() - 1; i >= 0; --i) {
            const Simbolo& s = tabelaSimbolos.at(i);
            if (s.nome == funcaoAtual && s.funcao) {
                funcSym = &s;
                tipoFuncStr = s.tipo;
                break;
            }
        }

        bool ehVoid = (tipoFuncStr == "void");
        int tFunc   = ehVoid ? SemanticTable::ERR : tipoFromString(tipoFuncStr);

        if (!retornoTemExpr) {
            //caso: return;
            if (!ehVoid) {
                QString msg = QString(
                                  "Função '%1' não é void: 'return;' exige função void")
                                  .arg(funcaoAtual);
                throw SemanticError(msg.toStdString());
            }

            //não precisa mexer no ACC, apenas voltar
            gerador.gera("RETURN", "0");
        } else {
            //caso: return <expr>;
            if (ehVoid) {
                QString msg = QString(
                                  "Função '%1' é void: não pode retornar expressão")
                                  .arg(funcaoAtual);
                throw SemanticError(msg.toStdString());
            }

            if (pilhaTipos.isEmpty()) {
                throw SemanticError(
                    "Expressão de retorno sem tipo na pilha (#610)");
            }

            int tExpr = pilhaTipos.pop();

            int status = SemanticTable::atribType(tFunc, tExpr);
            if (status == SemanticTable::ERR) {
                auto toStr = [](int t)->QString{
                    using T = SemanticTable::Types;
                    switch (t) {
                    case T::INT: return "int";
                    case T::FLO: return "float";
                    case T::CHA: return "char";
                    case T::STR: return "string";
                    case T::BOO: return "bool";
                    default:     return "<?>"; }
                };
                QString msg = QString(
                                  "Tipo incompatível em 'return' da função '%1': esperado %2, recebido %3")
                                  .arg(funcaoAtual,
                                       tipoFuncStr,
                                       toStr(tExpr));
                throw SemanticError(msg.toStdString());
            } else if (status == SemanticTable::WAR) {
                QString w = QString(
                                "Aviso: conversão implícita no 'return' da função '%1'")
                                .arg(funcaoAtual);
                if (!avisos.contains(w)) avisos.push_back(w);
            }

            //a essa altura:
            //a expressão de retorno já foi avaliada.
            //ACC contém o valor a ser retornado.
            //basta executar RETURN 0: o chamador verá o valor no ACC.
            //gerador.gera("RETURN", "0"); tirei
        }

        break;
    }

    case 700: {
        //início de chamada de função em expressão: ID #202 já rodou,
        //então o nome da função está em idBaseVetAtual.
        if (idBaseVetAtual.isEmpty()) {
            throw SemanticError("Chamada de função sem base registrada (#700)");
        }

        QString nomeFunc = idBaseVetAtual;

        int idxFunc = resolverIndexMaisProximo(nomeFunc);
        if (idxFunc < 0) {
            QString msg = QString("Rotina/função '%1' não declarada").arg(nomeFunc);
            throw SemanticError(msg.toStdString());
        }
        if (!tabelaSimbolos[idxFunc].funcao) {
            QString msg = QString("Identificador '%1' não é função/rotina (#700)").arg(nomeFunc);
            throw SemanticError(msg.toStdString());
        }

        // Prepara contexto de chamada (igual ao #602, mas para expressão)
        funcaoChamadaAtual = nomeFunc;
        contParChamada = 0;

        break;
    }

    case 701: {
        if (funcaoChamadaAtual.isEmpty()) {
            throw SemanticError("Fim de chamada de função sem início (#701)");
        }

        int totalParams = contarParametros(funcaoChamadaAtual);
        if (contParChamada < totalParams) {
            QString msg = QString(
                              "Faltam argumentos na chamada de '%1': esperados %2, recebidos %3")
                              .arg(funcaoChamadaAtual)
                              .arg(totalParams)
                              .arg(contParChamada);
            throw SemanticError(msg.toStdString());
        }

        QString rotuloDestino;
        if (funcaoChamadaAtual == "main") {
            rotuloDestino = "_MAIN";
        } else {
            rotuloDestino = "_" + funcaoChamadaAtual;
        }

        gerador.gera("CALL", rotuloDestino);

        //tipo de retorno da função (para expressão)
        int idxFunc = -1;
        for (int i = tabelaSimbolos.size() - 1; i >= 0; --i) {
            const Simbolo& s = tabelaSimbolos.at(i);
            if (s.nome == funcaoChamadaAtual && s.funcao) {
                idxFunc = i;
                break;
            }
        }

        if (idxFunc < 0) {
            pilhaTipos.push(SemanticTable::ERR);
        } else {
            int tFunc = tipoFromString(tabelaSimbolos[idxFunc].tipo);

            if (tFunc == SemanticTable::ERR) {
                QString msg = QString(
                                  "Função '%1' do tipo '%2' não pode ser usada em expressão")
                                  .arg(funcaoChamadaAtual,
                                       tabelaSimbolos[idxFunc].tipo);
                throw SemanticError(msg.toStdString());
            }

            pilhaTipos.push(tFunc);
        }

        funcaoChamadaAtual.clear();
        contParChamada = 0;
        break;
    }



    default:
        break;
    }
}




/*

//////////////////// /HELPERS/ //////////////////////////////

*/



//#2
//usa o escopo do topo e verifica duplicado no mesmo escopo
void Semantico::insereTabela() {
    Simbolo novoSimbolo;
    novoSimbolo.tipo   = this->tipo;
    novoSimbolo.nome   = this->nome;
    novoSimbolo.escopo = this->pilha.top();   //escopo atual

    //verificar unicidade SOMENTE no escopo atual
    if (nomeExisteNoEscopoAtual(novoSimbolo.nome)) {
        QString msgErro = "O símbolo '" + novoSimbolo.nome + "' já existe neste escopo";
        throw SemanticError(msgErro.toStdString());
    }

    this->tabelaSimbolos.push_back(novoSimbolo);
}

//^^
//está declarado com o MESMO nome no escopo do topo da pilha?
bool Semantico::nomeExisteNoEscopoAtual(const QString& nome) const {
    if (pilha.isEmpty()) return false;
    int escopoAtual = pilha.top();
    for (const Simbolo& s : tabelaSimbolos) {
        if (s.nome == nome && s.escopo == escopoAtual) {
            return true;
        }
    }
    return false;
}

//#9 e #11
//está declarado em ALGUM escopo visível (algum valor dentro da pilha)?
bool Semantico::nomeExisteEmEscopoVisivel(const QString& nome) const {
    // percorre de trás pra frente (mais recente primeiro)
    for (int i = tabelaSimbolos.size()-1; i >= 0; --i) {
        const Simbolo& s = tabelaSimbolos.at(i);
        // o escopo é visível se estiver presente na pilha (escopo atual ou algum pai)
        for (int p = pilha.size()-1; p >= 0; --p) {
            if (s.escopo == pilha.at(p) && s.nome == nome) {
                return true;
            }
        }
    }
    return false;
}

//#9
//marca como usado
void Semantico::marcarUsadoMaisProximo(const QString& nome) {
    // percorre a tabela de trás pra frente (declaração mais recente primeiro)
    for (int i = tabelaSimbolos.size() - 1; i >= 0; --i) {
        Simbolo &s = tabelaSimbolos[i];

        if (s.nome != nome) continue;

        // está em algum escopo visível?
        for (int p = pilha.size() - 1; p >= 0; --p) {
            if (s.escopo == pilha.at(p)) {
                s.usado = true;  // marcou
                return;
            }
        }
    }
}

//#11 e #12
int Semantico::resolverIndexMaisProximo(const QString& nome) const {
    for (int i = tabelaSimbolos.size()-1; i >= 0; --i) {
        const Simbolo& s = tabelaSimbolos.at(i);
        if (s.nome != nome) continue;
        // escopo visível?
        for (int p = pilha.size()-1; p >= 0; --p) {
            if (s.escopo == pilha.at(p)) return i;
        }
    }
    return -1;
}

//#12
//Marca Inicializado
void Semantico::marcarInicializadoMaisProximo(const QString& nome) {
    int idx = resolverIndexMaisProximo(nome);
    if (idx >= 0) {
        tabelaSimbolos[idx].inicializado = true;
    }
}

//#8
//vgera avisos para a IDE somente os gera depois de acabar a pilha do escopo
void Semantico::gerarAvisosFinais() {
    //avisos.clear(); //precisei comentar pois não mostrava lista toda de avisos

    for (const Simbolo &s : tabelaSimbolos) {
        // (opcional) ignore funções: geralmente não fazem sentido aqui
        if (s.funcao) continue;

        // 1) Declarado e NÃO usado
        if (!s.usado) {
            QString msg = "Aviso: identificador declarado e não usado: " + s.nome;
            if (!avisos.contains(msg)) avisos.push_back(msg);
        }

        // 2) Usado e NÃO inicializado
        if (s.usado && !s.inicializado) {
            QString msg = "Aviso: identificador usado sem estar inicializado: " + s.nome;
            if (!avisos.contains(msg)) avisos.push_back(msg);
        }
    }
}




//transforma o nome em enum(numero) da classe SemanticTable
int Semantico::tipoFromLiteralAction(int action) const {
    using T = SemanticTable::Types;
    switch (action) {
    case 20: return T::INT;  // LIT_INTEIRO
    case 21: return T::FLO;  // LIT_DECIMAIS
    case 22: return T::CHA;  // LIT_CHAR
    case 23: return T::STR;  // LIT_STRING
    case 24: return T::BOO;  // KEY_TRUE/FALSE
    default: return SemanticTable::ERR;
    }
}

int Semantico::tipoFromString(const QString& tipoStr) const {
    using T = SemanticTable::Types;
    if (tipoStr == "int")    return T::INT;
    if (tipoStr == "float")  return T::FLO;
    if (tipoStr == "double") return T::FLO;
    if (tipoStr == "char")   return T::CHA;
    if (tipoStr == "string") return T::STR;
    if (tipoStr == "bool")   return T::BOO;
    return SemanticTable::ERR;  // inclui "void" e desconhecidos
}

int Semantico::tipoFuncaoAtual() const {
    if (funcaoAtual.isEmpty()) return SemanticTable::ERR;

    for (int i = tabelaSimbolos.size() - 1; i >= 0; --i) {
        const Simbolo& s = tabelaSimbolos.at(i);
        if (s.nome == funcaoAtual && s.funcao) {
            return tipoFromString(s.tipo);
        }
    }
    return SemanticTable::ERR;
}


void Semantico::pushOper(int opCode) {
    pilhaOps.push(opCode);
}

void Semantico::reduzirBinaria() {
    using ST = SemanticTable;

    if (pilhaTipos.size() < 2 || pilhaOps.isEmpty())
        return;

    int rhsType = pilhaTipos.pop();
    int lhsType = pilhaTipos.pop();
    int op      = pilhaOps.pop();

    if (op == ST::REL) {
        //operador relacional: gera código BIP específico
        geraCodigoRelacional(lhsType, rhsType);
    } else {
        //demais operações: apenas verificação de tipos (código já foi gerado
        //durante a leitura de literais/IDs usando operAtual/temOperador)
        int res = ST::resultType(lhsType, rhsType, op);
        if (res == ST::ERR) {
            QString msg = QString("Incompatibilidade de tipos na expressão (operador %1)")
                              .arg(op);
            throw SemanticError(msg.toStdString());
        }
        qDebug() << "Passando por aqui final da funcao reduzir binaria (op != REL)";
        pilhaTipos.push(res);
    }
}

void Semantico::geraCodigoRelacional(int lhsType, int rhsType)
{
    using ST = SemanticTable;

    //berificação de tipos usando a tabela semântica
    int res = ST::resultType(lhsType, rhsType, ST::REL);
    if (res == ST::ERR) {
        QString msg = "Incompatibilidade de tipos em operação relacional";
        throw SemanticError(msg.toStdString());
    }

    //recupera o temporário do LHS e o operador relacional
    if (pilhaRelLhs.isEmpty() || pilhaRelOper.isEmpty()) {
        QString msg = "Erro interno: pilhas de relacionais vazias em geraCodigoRelacional";
        throw SemanticError(msg.toStdString());
    }

    QString tLhs  = pilhaRelLhs.pop();   //TEMP_ESQ (valor do lado esquerdo que foi salvo no #48..#53)
    QString opLex = pilhaRelOper.pop();  //operador: "<", ">", "<=", ">=", "==", "!="

    //marca que a expressão atual contém um relacional e qual é ele
    ultimoOperadorRelacional = opLex;
    exprAtualTemRelacional   = true;

    //RHS está no ACC neste momento → guarda em TEMP_DIR
    QString tRhs = gerador.novoTemp();   // TEMP_DIR
    gerador.gera("STO", tRhs);

    //faz LHS - RHS e deixa no ACC
    gerador.gera("LD", tLhs);
    gerador.gera("SUB", tRhs);
    //ACC agora contém LHS - RHS
    //as flags da ULA estão coerentes com essa subtração e serão
    //usadas pelos desvios em if/while/for/do-while.

    //tipo resultante é booleano (para a análise de tipos)
    pilhaTipos.push(res);
}
bool Semantico::boolifyRelacionalParaAtribOuSaida(const QString &destino, bool isOutput)
{
    if (!exprAtualTemRelacional) {
        return false; // nada a fazer
    }

    //ACC contém LHS - RHS aqui.
    //se verdadeiro: destino = 1
    //se falso    : destino = 0

    QString L_true = gerador.novoRotulo();
    QString L_end  = gerador.novoRotulo();

    //desvio para o CASO VERDADEIRO
    if      (ultimoOperadorRelacional == ">") {
        //verdadeiro se LHS > RHS  → LHS - RHS > 0
        gerador.gera("BGT", L_true);
    } else if (ultimoOperadorRelacional == "<") {
        //verdadeiro se LHS < RHS  → LHS - RHS < 0
        gerador.gera("BLT", L_true);
    } else if (ultimoOperadorRelacional == ">=") {
        //verdadeiro se LHS >= RHS → LHS - RHS >= 0
        gerador.gera("BGE", L_true);
    } else if (ultimoOperadorRelacional == "<=") {
        //verdadeiro se LHS <= RHS → LHS - RHS <= 0
        gerador.gera("BLE", L_true);
    } else if (ultimoOperadorRelacional == "==") {
        //verdadeiro se LHS == RHS → LHS - RHS == 0
        gerador.gera("BEQ", L_true);
    } else if (ultimoOperadorRelacional == "!=") {
        //verdadeiro se LHS != RHS → LHS - RHS != 0
        gerador.gera("BNE", L_true);
    } else {
        //operador inesperado → não mexe
        exprAtualTemRelacional = false;
        ultimoOperadorRelacional.clear();
        return false;
    }

    //pro caso FASO
    //se não saltou, é falso.
    //precisa gerar r = 0  (ou $out_port = 0)

    gerador.gera("LDI", "0");
    if (!destino.isEmpty() && !isOutput) {
        //atribuição/inicialização
        gerador.gera("STO", destino);
    } else if (isOutput) {
        //saída (cout/print)
        gerador.gera("STO", "$out_port");
    }
    gerador.gera("JMP", L_end);

    //Caso VERDADEIRO
    gerador.geraRotulo(L_true);
    gerador.gera("LDI", "1");
    if (!destino.isEmpty() && !isOutput) {
        gerador.gera("STO", destino);
    } else if (isOutput) {
        gerador.gera("STO", "$out_port");
    }

    //FIM
    gerador.geraRotulo(L_end);

    //limpa o estado do relacional (já foi totalmente utilizado)
    exprAtualTemRelacional = false;
    ultimoOperadorRelacional.clear();

    //já gera STO do destino (ou $out_port), quem chamou NÃO deve gravar de novo
    return true;
}


QString Semantico::getNomeParametro(const QString& nomeFunc, int indice) const {
    //acha a função
    int idxFunc = -1;
    for (int i = 0; i < tabelaSimbolos.size(); ++i) {
        const Simbolo& s = tabelaSimbolos[i];
        if (s.nome == nomeFunc && s.funcao) {
            idxFunc = i;
            break;
        }
    }
    if (idxFunc < 0) return QString();

    //percorre símbolos subsequentes, pegando apenas parâmetros
    int count = 0;
    for (int j = idxFunc + 1; j < tabelaSimbolos.size(); ++j) {
        const Simbolo& s = tabelaSimbolos[j];

        if (!s.param) {
            if (count > 0) break;   //já terminou a sequência de parâmetros
            continue;
        }

        if (count == indice) {
            return s.nome;
        }
        ++count;
    }

    return QString();
}

int Semantico::contarParametros(const QString& nomeFunc) const {
    //acha a função
    int idxFunc = -1;
    for (int i = 0; i < tabelaSimbolos.size(); ++i) {
        const Simbolo& s = tabelaSimbolos[i];
        if (s.nome == nomeFunc && s.funcao) {
            idxFunc = i;
            break;
        }
    }
    if (idxFunc < 0) return 0;

    //conta parâmetros logo após a função
    int count = 0;
    for (int j = idxFunc + 1; j < tabelaSimbolos.size(); ++j) {
        const Simbolo& s = tabelaSimbolos[j];

        if (!s.param) {
            if (count > 0) break;
            continue;
        }
        ++count;
    }

    return count;
}

int Semantico::tipoParametro(const QString& nomeFunc, int indice) const {
    //acha a função
    int idxFunc = -1;
    for (int i = 0; i < tabelaSimbolos.size(); ++i) {
        const Simbolo& s = tabelaSimbolos[i];
        if (s.nome == nomeFunc && s.funcao) {
            idxFunc = i;
            break;
        }
    }
    if (idxFunc < 0) return SemanticTable::ERR;

    //percorre parâmetros
    int count = 0;
    for (int j = idxFunc + 1; j < tabelaSimbolos.size(); ++j) {
        const Simbolo& s = tabelaSimbolos[j];

        if (!s.param) {
            if (count > 0) break;
            continue;
        }

        if (count == indice) {
            return tipoFromString(s.tipo);
        }
        ++count;
    }

    return SemanticTable::ERR;
}




void Semantico::geraBranchSeZero(const QString& rotulo)
{
    //usa SUBI 0 para atualizar os flags da ULA com base em ACC
    gerador.gera("SUBI", "0");
    //se o resultado for zero → desvia
    gerador.gera("BEQ", rotulo);
}

void Semantico::geraBranchSeNaoZero(const QString& rotulo)
{
    //usa SUBI 0 para atualizar os flags da ULA com base em ACC
    gerador.gera("SUBI", "0");
    //se o resultado for diferente de zero → desvia
    gerador.gera("BNE", rotulo);
}


