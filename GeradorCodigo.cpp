#include "GeradorCodigo.h"

GeradorCodigo::GeradorCodigo()
    : contadorTemp(0),
      contadorRotulo(0)
{
}

void GeradorCodigo::limpa() {
    secaoData.clear();
    secaoText.clear();

    contadorTemp = 0; //reset contador de temporários
    contadorRotulo = 0;

    secaoData << ".data";
    secaoText << ".text";
}

void GeradorCodigo::gera(QString instrucao, QString operando) {
    if (operando.isEmpty())
        secaoText << instrucao;
    else
        secaoText << instrucao + " " + operando;
}

void GeradorCodigo::geraData(QString variavel, QString valor) {
    secaoData << QString("%1: %2").arg(variavel, valor);
}

QString GeradorCodigo::novoTemp() {
    QString nome = QString("_t%1").arg(contadorTemp++);
    secaoData << QString("%1: 0").arg(nome);
    return nome;
}

QString GeradorCodigo::novoRotulo() {
    return QString("L%1").arg(contadorRotulo++);
}

void GeradorCodigo::geraRotulo(const QString &rotulo) {
    // linha do tipo: L0:
    secaoText << rotulo + ":";
}

QString GeradorCodigo::getCodigo() const {
    return secaoData.join("\n") + "\n" + secaoText.join("\n") + "\n";
}

void GeradorCodigo::geraInicioText(QString instrucao, QString operando) {
    QString linha;
    if (operando.isEmpty())
        linha = instrucao;
    else
        linha = instrucao + " " + operando;

    //procura a linha ".text"
    int posText = secaoText.indexOf(".text");
    if (posText < 0) {
        //se por algum motivo .text não existir ainda,
        //cria e insere no início.
        secaoText.prepend(".text");
        posText = 0;
    }

    //insere logo após .text
    secaoText.insert(posText + 1, linha);
}

