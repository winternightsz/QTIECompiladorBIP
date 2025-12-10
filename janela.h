#ifndef JANELA_H
#define JANELA_H

#include <QMainWindow>

#include "Semantico.h"

namespace Ui {
class Janela;
}

class Janela : public QMainWindow
{
    Q_OBJECT

public:
    explicit Janela(QWidget *parent = 0);
    ~Janela();

private slots:
    void tratarCliqueBotao();

private:
    Ui::Janela *ui;

    //helpers da UI
    void configurarTabelaSimbolos();
    void preencherTabela(const QList<Simbolo>& simbolos);  //aqui preenche as linhas
};

#endif // JANELA_H
