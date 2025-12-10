#include "janela.h"
#include "qabstractitemview.h"
#include "ui_janela.h"
#include <QDebug>

#include "Lexico.h"
#include "Sintatico.h"
#include "Semantico.h"

Janela::Janela(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Janela)
{
    ui->setupUi(this);
    configurarTabelaSimbolos();

    connect(ui->botaoCompilar, //Objeto de Origem
            SIGNAL(clicked(bool)), //Acao do objeto dfonte
            this, //Destino: quem vai tratar
            SLOT(tratarCliqueBotao()) //"Tratador"
            );
}
Janela::~Janela()
{
    delete ui;
}

//configura as colunas e opções visuais
void Janela::configurarTabelaSimbolos() {
    auto t = ui->tabelaSimbolosUI;   //objectName que aparece no .ui
    t->setColumnCount(8);
    t->setHorizontalHeaderLabels(
        {"Tipo","Nome","Vetor","Função","Parâmetro","Inicializado", "Usado", "Escopo"}
        );
    t->setEditTriggers(QAbstractItemView::NoEditTriggers);
    t->setSelectionBehavior(QAbstractItemView::SelectRows);
    t->setAlternatingRowColors(true);
    t->setSortingEnabled(true);
}

//preenche as linhas
void Janela::preencherTabela(const QList<Simbolo>& simbolos) {
    auto t = ui->tabelaSimbolosUI;

    t->setRowCount(simbolos.size());
    t->clearContents(); //não apaga cabeçalho

    int row = 0;
    for (const Simbolo& s : simbolos) {
        t->setItem(row, 0, new QTableWidgetItem(s.tipo));
        t->setItem(row, 1, new QTableWidgetItem(s.nome));
        t->setItem(row, 2, new QTableWidgetItem(s.vetor  ? "Sim" : "Não"));
        t->setItem(row, 3, new QTableWidgetItem(s.funcao ? "Sim" : "Não"));
        t->setItem(row, 4, new QTableWidgetItem(s.param  ? "Sim" : "Não"));
        t->setItem(row, 5, new QTableWidgetItem(s.inicializado  ? "Sim" : "Não"));
        t->setItem(row, 6, new QTableWidgetItem(s.usado  ? "Sim" : "Não"));
        t->setItem(row, 7, new QTableWidgetItem(QString::number(s.escopo)));
        ++row;
    }
    t->resizeColumnsToContents();
}

#include <QDebug>
void Janela::tratarCliqueBotao() {
    Lexico lex;
    Sintatico sint;
    Semantico sem;

    lex.setInput(ui->entrada->toPlainText().toStdString().c_str());

    try {
        sint.parse(&lex, &sem);

        //mostra símbolos (debug)
        for (Simbolo s : sem.tabelaSimbolos) {
            qDebug() << s.tipo << " | " << s.nome << " | "
                     << s.vetor << " | " << s.funcao << " | "
                     << s.param << " | " << s.inicializado << " | "
                     << s.usado << " | " << s.escopo;
        }
        preencherTabela(sem.tabelaSimbolos);

        ui->console->setText("Compilado com sucesso!");

        if (sem.avisos.isEmpty()) {
            ui->console->append("Nenhum aviso encontrado.");
        } else {
            ui->console->append("\nAvisos:");
            for (const QString &w : sem.avisos) {
                ui->console->append(" - " + w);
            }
        }

        ui->saidaAsmUI->setPlainText(sem.getAssembly());

    } catch (LexicalError err) {
        ui->console->setText(QString("Erro léxico: %1").arg(err.getMessage()));
    } catch (SyntacticError err) {
        ui->console->setText(QString("Erro Sintático: %1").arg(err.getMessage()));
    } catch (SemanticError err) {
        ui->console->setText(QString("Erro Semântico: %1").arg(err.getMessage()));
    }

}


