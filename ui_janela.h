/********************************************************************************
** Form generated from reading UI file 'janela.ui'
**
** Created by: Qt User Interface Compiler version 6.9.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_JANELA_H
#define UI_JANELA_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDockWidget>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Janela
{
public:
    QWidget *centralWidget;
    QVBoxLayout *verticalLayout;
    QPlainTextEdit *entrada;
    QSpacerItem *verticalSpacer;
    QHBoxLayout *horizontalLayout;
    QTextEdit *console;
    QPushButton *botaoCompilar;
    QPushButton *pushTabela;
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;
    QDockWidget *dockWidget;
    QWidget *dockWidgetContents_3;
    QWidget *verticalLayoutWidget;
    QVBoxLayout *verticalLayout_3;
    QTableWidget *tabelaSimbolosUI;
    QTextEdit *saidaAsmUI;

    void setupUi(QMainWindow *Janela)
    {
        if (Janela->objectName().isEmpty())
            Janela->setObjectName("Janela");
        Janela->setEnabled(true);
        Janela->resize(1203, 800);
        QFont font;
        font.setPointSize(26);
        Janela->setFont(font);
        QIcon icon(QIcon::fromTheme(QIcon::ThemeIcon::FormatTextItalic));
        Janela->setWindowIcon(icon);
        Janela->setStyleSheet(QString::fromUtf8("QMainWindow { background-image: url(:/images/skyNight.png); }\n"
"\n"
" QMainWindow::separator {\n"
"     background: rgba(85, 88, 255, 100);\n"
"     width: 10px; /* when vertical */\n"
"     height: 10px; /* when horizontal */\n"
" }\n"
"\n"
" QMainWindow::separator:hover {\n"
"     background:  rgba(85, 88, 255, 170);\n"
" }\n"
""));
        Janela->setDockOptions(QMainWindow::DockOption::AllowNestedDocks|QMainWindow::DockOption::AllowTabbedDocks|QMainWindow::DockOption::AnimatedDocks);
        centralWidget = new QWidget(Janela);
        centralWidget->setObjectName("centralWidget");
        verticalLayout = new QVBoxLayout(centralWidget);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName("verticalLayout");
        entrada = new QPlainTextEdit(centralWidget);
        entrada->setObjectName("entrada");
        QFont font1;
        font1.setFamilies({QString::fromUtf8("Roboto Condensed")});
        font1.setPointSize(20);
        font1.setBold(false);
        font1.setItalic(false);
        entrada->setFont(font1);
        entrada->setStyleSheet(QString::fromUtf8("QPlainTextEdit{\n"
"background: rgba(255, 255, 255, 30);\n"
"color: white;\n"
"	font: 20pt \"Roboto Condensed\";\n"
"}"));

        verticalLayout->addWidget(entrada);

        verticalSpacer = new QSpacerItem(20, 4, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        verticalLayout->addItem(verticalSpacer);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName("horizontalLayout");
        console = new QTextEdit(centralWidget);
        console->setObjectName("console");
        console->setFont(font1);
        console->setStyleSheet(QString::fromUtf8("QTextEdit{\n"
"background: rgba(255, 255, 255, 30);\n"
"color: white;\n"
"font: 20pt \"Roboto Condensed\";\n"
"}"));
        console->setReadOnly(true);

        horizontalLayout->addWidget(console);

        botaoCompilar = new QPushButton(centralWidget);
        botaoCompilar->setObjectName("botaoCompilar");
        QFont font2;
        font2.setFamilies({QString::fromUtf8("Pixellari")});
        font2.setPointSize(26);
        botaoCompilar->setFont(font2);
        botaoCompilar->setStyleSheet(QString::fromUtf8("QPushButton {color: rgb(255, 255, 255)}"));

        horizontalLayout->addWidget(botaoCompilar);


        verticalLayout->addLayout(horizontalLayout);

        pushTabela = new QPushButton(centralWidget);
        pushTabela->setObjectName("pushTabela");
        QSizePolicy sizePolicy(QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(pushTabela->sizePolicy().hasHeightForWidth());
        pushTabela->setSizePolicy(sizePolicy);
        pushTabela->setMaximumSize(QSize(16777215, 16777203));
        QFont font3;
        font3.setFamilies({QString::fromUtf8("Pixellari")});
        font3.setPointSize(14);
        pushTabela->setFont(font3);
        pushTabela->setStyleSheet(QString::fromUtf8("QPushButton {color: rgb(255, 255, 255)}"));
        pushTabela->setIconSize(QSize(12, 12));
        pushTabela->setAutoDefault(false);
        pushTabela->setFlat(false);

        verticalLayout->addWidget(pushTabela, 0, Qt::AlignmentFlag::AlignRight|Qt::AlignmentFlag::AlignVCenter);

        verticalLayout->setStretch(0, 1);
        Janela->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(Janela);
        menuBar->setObjectName("menuBar");
        menuBar->setEnabled(false);
        menuBar->setGeometry(QRect(0, 0, 1203, 18));
        menuBar->setNativeMenuBar(true);
        Janela->setMenuBar(menuBar);
        mainToolBar = new QToolBar(Janela);
        mainToolBar->setObjectName("mainToolBar");
        mainToolBar->setEnabled(false);
        Janela->addToolBar(Qt::ToolBarArea::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(Janela);
        statusBar->setObjectName("statusBar");
        statusBar->setEnabled(false);
        Janela->setStatusBar(statusBar);
        dockWidget = new QDockWidget(Janela);
        dockWidget->setObjectName("dockWidget");
        dockWidget->setEnabled(true);
        dockWidget->setMinimumSize(QSize(541, 727));
        dockWidget->setMaximumSize(QSize(800, 524287));
        dockWidget->setStyleSheet(QString::fromUtf8(" QDockWidget {\n"
"     border: 1px solid pink;\n"
"	background: rgb(97, 97, 255);\n"
"	font: 500 26pt \"Pixellari\";\n"
"	color: white;\n"
" }\n"
"\n"
" QDockWidget::title {\n"
"     text-align: left;     \n"
"     padding: 10px;\n"
" }\n"
"\n"
" QDockWidget::close-button, QDockWidget::float-button {\n"
"    background: white;\n"
"     \n"
"     \n"
" }\n"
"\n"
" QDockWidget::close-button:hover, QDockWidget::float-button:hover {\n"
"     background: gray;\n"
" }\n"
"\n"
" QDockWidget::close-button:pressed, QDockWidget::float-button:pressed {\n"
"     padding: 1px -1px -1px 1px;\n"
" }\n"
""));
        dockWidget->setFloating(false);
        dockWidget->setFeatures(QDockWidget::DockWidgetFeature::DockWidgetClosable|QDockWidget::DockWidgetFeature::DockWidgetFloatable|QDockWidget::DockWidgetFeature::DockWidgetMovable);
        dockWidget->setAllowedAreas(Qt::DockWidgetArea::AllDockWidgetAreas);
        dockWidgetContents_3 = new QWidget();
        dockWidgetContents_3->setObjectName("dockWidgetContents_3");
        dockWidgetContents_3->setEnabled(true);
        dockWidgetContents_3->setMinimumSize(QSize(500, 670));
        verticalLayoutWidget = new QWidget(dockWidgetContents_3);
        verticalLayoutWidget->setObjectName("verticalLayoutWidget");
        verticalLayoutWidget->setGeometry(QRect(0, 0, 931, 661));
        verticalLayout_3 = new QVBoxLayout(verticalLayoutWidget);
        verticalLayout_3->setSpacing(20);
        verticalLayout_3->setContentsMargins(11, 11, 11, 11);
        verticalLayout_3->setObjectName("verticalLayout_3");
        verticalLayout_3->setSizeConstraint(QLayout::SizeConstraint::SetDefaultConstraint);
        verticalLayout_3->setContentsMargins(0, 0, 0, 0);
        tabelaSimbolosUI = new QTableWidget(verticalLayoutWidget);
        tabelaSimbolosUI->setObjectName("tabelaSimbolosUI");
        tabelaSimbolosUI->setEnabled(true);
        QFont font4;
        font4.setFamilies({QString::fromUtf8("Pixellari")});
        font4.setPointSize(9);
        font4.setWeight(QFont::Medium);
        font4.setItalic(false);
        font4.setUnderline(true);
        tabelaSimbolosUI->setFont(font4);
        tabelaSimbolosUI->setStyleSheet(QString::fromUtf8("QTableWidget{\n"
"font: 500 9pt \"Pixellari\";\n"
"background: rgba(170, 170, 255, 200)rgba(108, 128, 255, 200);\n"
"}\n"
"\n"
"QHeaderView::section {\n"
"           background-color: rgb(84, 92, 254);\n"
"           color: white;\n"
"        }"));

        verticalLayout_3->addWidget(tabelaSimbolosUI);

        saidaAsmUI = new QTextEdit(verticalLayoutWidget);
        saidaAsmUI->setObjectName("saidaAsmUI");
        QFont font5;
        font5.setFamilies({QString::fromUtf8("Pixellari")});
        font5.setPointSize(14);
        font5.setWeight(QFont::Medium);
        font5.setItalic(false);
        saidaAsmUI->setFont(font5);
        saidaAsmUI->setStyleSheet(QString::fromUtf8("QTextEdit{\n"
"background: rgba(255, 255, 255, 30);\n"
"color: white;\n"
"}"));
        saidaAsmUI->setReadOnly(true);

        verticalLayout_3->addWidget(saidaAsmUI);

        dockWidget->setWidget(dockWidgetContents_3);
        Janela->addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, dockWidget);
        dockWidget->raise();

        retranslateUi(Janela);

        pushTabela->setDefault(false);


        QMetaObject::connectSlotsByName(Janela);
    } // setupUi

    void retranslateUi(QMainWindow *Janela)
    {
        Janela->setWindowTitle(QCoreApplication::translate("Janela", "IDE QTIE", nullptr));
        botaoCompilar->setText(QCoreApplication::translate("Janela", "Compilar", nullptr));
        pushTabela->setText(QCoreApplication::translate("Janela", "Tabela e Assembly", nullptr));
        dockWidget->setWindowTitle(QCoreApplication::translate("Janela", "Tabela", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Janela: public Ui_Janela {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_JANELA_H
