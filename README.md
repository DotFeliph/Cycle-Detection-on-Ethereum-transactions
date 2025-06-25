# Cycle Detection in Ethereum Transactions

![Language](https://img.shields.io/badge/Language-C-blue.svg)![License](https://img.shields.io/badge/License-MIT-yellow.svg)![Docs](https://img.shields.io/badge/Docs-Doxygen-green.svg)

Projeto desenvolvido para a disciplina de Teoria dos Grafos, focado na detecção de ciclos em um conjunto de dados de transações da blockchain Ethereum.

## Tabela de Conteúdos

1.  [Sobre o Projeto](#sobre-o-projeto)
2.  [Conceitos Principais](#conceitos-principais)
3.  [Funcionalidades](#funcionalidades)
4.  [Estrutura do Projeto](#estrutura-do-projeto)
5.  [Pré-requisitos](#pré-requisitos)
6.  [Como Compilar e Usar](#compilação)
7.  [Coleta e Pré-processamento de Dados](#pipeline-de-Dados)
8.  [Documentação](#documentação)
9.  [Licença](#licença)
10. [Autor](#autor)

## Sobre o Projeto

Este projeto implementa um programa em C para modelar transações financeiras da rede Ethereum como um grafo direcionado e, em seguida, aplicar um algoritmo de Busca em Profundidade (DFS) para identificar a existência de ciclos.

A detecção de ciclos em grafos de transação é um problema fundamental com aplicações práticas, como a identificação de possíveis rotas de arbitragem cíclica ou a análise de fluxos monetários complexos. Este trabalho serve como uma implementação prática dos conceitos teóricos de grafos para resolver um problema do mundo real.

##  Conceitos Principais

*   **Grafo Direcionado:** Cada carteira Ethereum é representada como um vértice (nó), e cada transação é uma aresta direcionada do remetente para o destinatário. O peso da aresta corresponde ao valor da transação.

*   **Detecção de Ciclos com DFS:** O algoritmo de Busca em Profundidade (DFS) atravessa o grafo mantendo o controle dos vértices no caminho da recursão atual (a "pilha de recursão"). Se um vértice adjacente já visitado é encontrado novamente *nesta mesma pilha*, significa que um ciclo foi detectado. O caminho do início do ciclo até o vértice atual é então reportado como um ciclo.

*   **Biblioteca GMP:** Essencial para manipular os valores das transações (em Wei), que são números inteiros muito grandes e excedem a capacidade dos tipos de dados padrão de 64 bits (como `long long`). A GMP permite realizar aritmética de precisão arbitrária, garantindo que nenhum dado de valor seja perdido.

---

##  Pré-requisitos

Para compilar e executar este projeto, você precisará das seguintes ferramentas instaladas:

- **GCC (Compilador C):** `sudo apt-get install build-essential`
- **Make:** Geralmente incluído no `build-essential`.
- **Biblioteca GMP:** `sudo apt-get install libgmp-dev`
- **Doxygen (para gerar a documentação):** `sudo apt-get install doxygen`
- **Graphviz (para gerar diagramas na documentação):** `sudo apt-get install graphviz`

Para a coleta de dados, você também precisará:
- **Python 3 e Pip**
- **Ethereum ETL:** `pip3 install ethereum-etl`

---

##  Compilação

O projeto utiliza um `Makefile` que automatiza todo o processo de compilação.

1.  **Clone o repositório:**
    ```bash
    git clone https://github.com/DotFeliph/Cycle-Detection-on-Ethereum-transactions.git
    cd Cycle-Detection-on-Ethereum-transactions
    ```

2.  **Compile o projeto:**
    ```bash
    make
    ```
    Este comando criará o diretório `build/` para os arquivos de objeto e o executável `main` no diretório raiz.

3.  **Para limpar os arquivos gerados:**
    ```bash
    make clean
    ```
    Este comando removerá o executável e as pastas `build/` e `docs/`.

---

##  Uso

Para executar o programa, utilize o seguinte comando, fornecendo um arquivo de dados como argumento:

```bash
./main [OPÇÕES] <arquivo_de_dados>
```

**Argumentos e Opções:**
- `<arquivo_de_dados>`: (Obrigatório) Caminho para o arquivo de texto contendo as transações formatadas (`remetente destinatario valor`).
- `-o, --output <arquivo>`: Define um nome para o arquivo de saída. Se não for especificado, um nome único com timestamp será gerado (ex: `output--2025-06-25_22-10-00.txt`).
- `-v, --verbose`: Ativa o modo verboso, exibindo informações de progresso e tempo de execução no terminal.
- `-h, --help`: Exibe a mensagem de ajuda detalhada.

**Exemplo:**
```bash
./main -v filtered.txt
```

---

##  Pipeline de Dados

Para gerar um arquivo de dados compatível com o programa, siga estes passos:

### 1. Coleta de Dados

Use o `ethereum-etl` para extrair transações de um intervalo de blocos da Ethereum. Você precisará de uma URL de um provedor RPC (como [Alchemy](https://www.alchemy.com/) ou Infura).

```bash
ethereumetl export_blocks_and_transactions \
--start-block 13924750 \
--end-block 13924800 \
--provider-uri  \
--transactions-output transactions.csv
```

### 2. Pré-processamento

Use este script `awk` para filtrar o arquivo `transactions.csv` e formatá-lo para a entrada do nosso programa. Ele remove transações de valor zero, auto-transações e formata a saída.

```bash
awk -F',' '
  NR==1 { next }
  NR==FNR {
    if ($8 != 0 && $6 != $7) { receivers[$7] = 1 }
    next
  }
  {
    if ($8 != 0 && $6 != $7 && ($6 in receivers)) {
      print $6, $7, $8 ".0"
    }
  }
' transactions.csv transactions.csv > filtered.txt
```
O arquivo `filtered.txt` está agora pronto para ser usado como entrada.

---

##  Documentação

A documentação técnica completa do código-fonte pode ser gerada usando Doxygen.

1.  **Gere a documentação:**
    ```bash
    make docs
    ```

2.  **Visualize a documentação:**
    Abra o seguinte arquivo no seu navegador:
    `docs/html/index.html`

A documentação inclui descrições de todas as funções e estruturas, diagramas de chamadas e páginas dedicadas à análise do projeto.

---

##  Licença

Distribuído sob a licença MIT. Veja o arquivo `LICENSE` para mais informações.

---

##  Contato

Amanda "AmandamatosB" 
Antônio "anponiomss1" 
Feliph "DotFeliph" 

Link do Projeto: [https://github.com/DotFeliph/Cycle-Detection-on-Ethereum-transactions](https://github.com/DotFeliph/Cycle-Detection-on-Ethereum-transactions)