# QTIE Compilador BIP 

Este projeto contém um **compilador desenvolvido em C++ utilizando Qt Creator** que gera código BIP a partir de uma linguagem própria semelhante a C.  
O repositório inclui lexer, parser, ações semânticas, gerador de código, arquivos .gals e exemplos de entrada/saída.

## Sobre o Projeto
- Completo para geração de código 1, 2 e 3.
- Projeto para disciplina de Compiladores sob orientação do professor: Eduardo Alves da Silva.
- Alunos: Helena Becker Piazera, Izabela Andreazza e Logan Santana Fernandes.
- Universidade do Vale do Itajaí 
---

## **1. Requisitos para Compilar ou Executar o Compilador**

Você pode compilar o compilador de **três maneiras diferentes**:

### **Opção 1 — Usando Qt Creator (recomendado)**  
Essa é a forma utilizada no desenvolvimento original.

**Requisitos:**
- Qt Creator 6 ou superior  
- Qt 6.x ou 5.x  
- Compilador MinGW (Windows) ou GCC/Clang (Linux)  
- Kits configurados no Qt Creator

**Passos:**
1. Abra o Qt Creator  
2. Clique em **File > Open Project**  
3. Selecione o arquivo `.pro` do compilador  
4. Configure o kit (MinGW, GCC, MSVC, etc.)  
5. Clique em **Build > Build Project**  
6. Clique em **Run** para executar o compilador por interface gráfica

---

## Opção 2 — Compilando via terminal com `qmake` + `make`

Se você tiver Qt instalado no sistema, pode compilar diretamente pelo terminal:

### **Linux/macOS**
```bash
qmake
make
./QTIECompiladorBIP
```

### **Windows (Qt + MinGW)**
```bash
qmake
mingw32-make
QTIECompiladorBIP.exe
```

---

## Opção 3 — Compilando manualmente com CMake (para quem não quer instalar Qt Creator)

O projeto pode ser portado para CMake caso o usuário prefira compilar manualmente.

Exemplo de `CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 3.16)
project(QTIECompiladorBIP)

set(CMAKE_CXX_STANDARD 17)

find_package(Qt6 COMPONENTS Core Widgets REQUIRED)

set(SOURCES
    main.cpp
    lexer.cpp
    parser.cpp
    semantico.cpp
    gerador.cpp
)

qt6_add_executable(QTIECompiladorBIP ${SOURCES})
target_link_libraries(QTIECompiladorBIP Qt6::Core Qt6::Widgets)
```

E então:

```bash
mkdir build
cd build
cmake ..
make
./QTIECompiladorBIP
```

---

## **2. Como Executar o Compilador**

Depois de compilado, o compilador pode ser executado de três formas:

---

### **1. Interface gráfica (Qt Creator / Qt)**

Se o projeto inclui uma interface:
- Abra o executável gerado (`QTIECompiladorBIP.exe` ou `./QTIECompiladorBIP`)
- Carregue o arquivo fonte `.txt` ou `.c` pelo menu
- Clique em "Compilar"
- O BIP gerado aparecerá na janela de saída

---

### **2. Linha de comando (CLI)**

Se o compilador foi configurado para rodar via argumentos:

```bash
./QTIECompiladorBIP entrada.txt saida.bip
```

Ou:

```bash
QTIECompiladorBIP.exe minha_entrada.txt meu_codigo.bip
```

Isso fará:

- Ler o arquivo de entrada  
- Rodar lexer → parser → semântico → gerador  
- Criar um arquivo `.bip` com o código final

---

### **3. Usando arquivos arrastados (Windows)**  
Se configurado:  
Basta arrastar o arquivo de entrada acima do executável.

---

## **3. Estrutura do Projeto**

```
/src

Léxico
- Lexico.cpp / Lexico.h — Implementação do analisador léxico.
- LexicalError.h — Tratamento de erros léxicos.

Sintático
- Sintatico.cpp / Sintatico.h — Parser que chama ações semânticas conforme a gramática.
- SyntacticError.h — Tratamento de erros sintáticos.

Semântico
- Semantico.cpp / Semantico.h — Verificação semântica e geração de código BIP.
- semantictable.cpp / semantictable.h — Tabelas de compatibilidade (tipos, operações).
- tabelaSemantica.cpp / tabelaSemantica.h — Versão auxiliar/antiga da tabela semântica.
- SemanticError.h — Tratamento de erros semânticos.

Gerador de Código
- GeradorCodigo.cpp / GeradorCodigo.h — Constrói seções `.data` e `.text`, gera instruções BIP.
- Constants.cpp / Constants.h — Constantes globais relacionadas ao código.

Interface Gráfica (IDE)
- janela.cpp / janela.h — Comportamento da janela principal.
- janela.ui — Layout Qt Designer.
- ui_janela.h — Gerado automaticamente pelo Qt; mapeia widgets para C++.
- skyNight.png — Asset visual da interface.
- resources.qrc — Arquivo de recursos Qt.

Infraestrutura
- Token.h — Estrutura básica do Token.
- AnalysisError.h — Classe de erro genérica para extensões.
- Teste.pro — Arquivo de projeto Qt Creator.
- Teste.pro.user — Configurações específicas do ambiente.

Execução
- main.cpp — Entrada da aplicação compilador/IDE.

```

---

## **4. Funcionalidades Implementadas no Compilador**

### ✔ Análise léxica  
### ✔ Análise sintática (GALS)  
### ✔ Ações semânticas completas  
### ✔ Geração de código BIP:

- Atribuições  
- If, else, while, for  
- Acesso a vetores (`LDV`, `STOV`)  
- Sub-rotinas (`CALL`, `RETURN`)  
- Passagem de parâmetros por cópia  
- Retorno de funções  
- Verificação de compatibilidade:
  - quantidade de parâmetros  
  - tipos  
  - ordem  

---

## **5. Exemplos de Uso**

### Exemplo – Função com parâmetro, retorno e chamada de função

Entrada:

```c
int soma2(int x) {
    return x + 2;
}

int main() {
    int a;
    int r;

    a = 5;
    r = soma2(a);
}
```
Saída BIP:
```asm
.data
x: 0      ; parâmetro da função soma2
a: 0      ; variável local em main
r: 0      ; variável local em main

.text
    ; pula a definição das funções — execução começa em main
    JMP _MAIN

_soma2:
    ; função: int soma2(int x) { return x + 2; }

    LD  x          ; ACC = parâmetro x
    ADDI 2         ; ACC = x + 2
    RETURN 0       ; devolve o valor em ACC ao chamador


_MAIN:
    ; código do main

    ; a = 5;
    LDI 5
    STO a

    ; r = soma2(a);

    ; (1) passa argumento por cópia
    LD  a          ; ACC = a
    STO x          ; parâmetro x = a

    ; (2) chama a função
    CALL _soma2    ; ACC recebe o retorno

    ; (3) armazena o retorno em r
    STO r

    HLT 0
```

Entrada:

```c
int soma(int a, int b) {
    return a + b;
}

int main() {
    int x;
    x = soma(5, 7);
}
```

Saída BIP:

```asm
.data
a: 0
b: 0
x: 0

.text
JMP _MAIN

_soma:
    LD a
    ADD b
    RETURN 0

_MAIN:
    LDI 5
    STO a
    LDI 7
    STO b
    CALL _soma
    STO x
    HLT 0
```
### Exemplo – Retorno de Função (sem parâmetros)

Entrada:

```c
int dez() {
    return 10;
}

int main() {
    int r;
    r = dez();
}
```

Saída BIP:

```asm
.data
r: 0

.text
    JMP _MAIN

_dez:
    LDI 10
    RETURN 0         ; devolve 10 ao chamador

_MAIN:
    CALL _dez        ; ACC = 10
    STO r            ; r = 10
    HLT 0

```

### Exemplo – Parâmetro por Cópia

Entrada:

```c
int ident(int x) {
    return x;
}

int main() {
    int a;
    int r;

    a = 5;
    r = ident(a);
}
```

Saída BIP:

```asm
.data
x: 0
a: 0
r: 0

.text
    JMP _MAIN

_ident:
    LD  x
    RETURN 0

_MAIN:
    LDI 5
    STO a

    LD  a
    STO x          ; passa parâmetro por cópia
    CALL _ident
    STO r

    HLT 0
```
### Exemplo – Chamada de função (sem retorno usado)

Entrada:

```c
void show5() {
    print(5);
}

int main() {
    show5();
}
```

Saída BIP:

```asm
.data

.text
    JMP _MAIN

_show5:
    LDI 5
    STO $out_port   ; print(5)
    RETURN 0

_MAIN:
    CALL _show5
    HLT 0

```
### Exemplo – Chamada de função (sem retorno usado)

Entrada:

```c
void show5() {
    print(5);
}

int main() {
    show5();
}
```

Saída BIP:

```asm
.data

.text
    JMP _MAIN

_show5:
    LDI 5
    STO $out_port   ; print(5)
    RETURN 0

_MAIN:
    CALL _show5
    HLT 0

```
### Caso Incorreto (erro semântico)

```c
int soma(int a, int b) {
    return a + b;
}

int main() {
    int r;
    r = soma(1);    // ERRO → falta argumento b
}
```


