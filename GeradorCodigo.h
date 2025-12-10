#ifndef GERADORCODIGO_H
#define GERADORCODIGO_H

#include <QString>
#include <QStringList>

class GeradorCodigo {
public:
    GeradorCodigo();

    void limpa();
    void gera(QString instrucao, QString operando = "");
    void geraData(QString variavel, QString valor = "0");

    QString novoTemp();//gera um nome de temporário único e já declara em .data

    QString novoRotulo();
    void geraRotulo(const QString &rotulo);

    QString getCodigo() const;

    //insere uma instrução logo após a diretiva .text
    void geraInicioText(QString instrucao, QString operando = "");

private:
    QStringList secaoData;
    QStringList secaoText;

    int contadorTemp;  //contador pra temporarios

    int contadorRotulo;
};

#endif
