# Wolfpack Server Emulator (Qt 6 & Python 3)

[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B20)
[![Qt](https://img.shields.io/badge/Qt-6.10-green.svg)](https://www.qt.io/)
[![Python](https://img.shields.io/badge/Python-3.14-yellow.svg)](https://www.python.org/)
[![CMake](https://img.shields.io/badge/Build-CMake-red.svg)](https://cmake.org/)
[![License](https://img.shields.io/badge/License-GPLv2-lightgrey.svg)](LICENSE)

**Wolfpack** é um emulador de servidor do **Ultima Online™** de alto desempenho, altamente customizável e extensível, construído em C++ com rede e interface sobre o framework **Qt** e motor de scripts dinâmicos embutido em **Python**.

Esta versão representa a modernização completa do projeto a partir de sua base legada (Qt 5.5 / Python 2.7 / qmake) para as tecnologias contemporâneas: **Qt 6**, **Python 3**, **C++17/C++20**, **Boost.Python moderno** e sistema de build automatizado com **CMake**.

---

## 📋 Sumário

- [Destaques da Modernização](#-destaques-da-modernização)
- [Requisitos do Sistema](#-requisitos-do-sistema)
- [Instalação de Dependências](#-instalação-de-dependências)
- [Como Compilar](#-como-compilar)
- [Como Executar](#-como-executar)
- [Configuração do Servidor](#-configuração-do-servidor)
- [Estrutura do Repositório](#-estrutura-do-repositório)
- [Scripts e Extensibilidade (Python 3)](#-scripts-e-extensibilidade-python-3)
- [Wiki e Documentação](#-wiki-e-documentação)

---

## 🚀 Destaques da Modernização

- **Migração para Qt 6**: Atualização do núcleo de rede, sockets, eventos e interface gráfica para Qt 6.10+, eliminando dependências obsoletas de Qt 4/5.
- **Python 3 Embutido (CPython C-API + Boost.Python)**:
  - Suporte completo a Python 3.10 até Python 3.14+.
  - Modernização das definições de `PyTypeObject` com slots de comparação rica (`tp_richcompare`) e métodos dinâmicos (`tp_methods`).
  - Registro unificado de submódulos C++ (`_wolfpack.console`, `_wolfpack.time`, `_wolfpack.settings`, `_wolfpack.database`, `_wolfpack.sockets`, `_wolfpack.accounts`).
  - Compatibilidade transparente para construções clássicas de scripts (`unicode = str`, `xrange = range`, comparadores, tipos de dados).
- **Correção do Carregador de Mundo Binário (`world.bin`)**: Ajuste na desserialização de nomes de tipos (`QString`) que continham terminação nula residual (`\0`), garantindo 100% de integridade no carregamento de todos os **32.998+ objetos salvos**.
- **Novo Sistema de Build com CMake**:
  - Geração nativa de build com `CMakeLists.txt`.
  - Cabeçalhos pré-compilados (`wolfpack_pch.h`) para compilação rápida.
  - Geração simultânea do executável com interface gráfica (`wolfpack`) e do servidor dedicado para terminal/servidores headless (`wolfpack-console`).

---

## 💻 Requisitos do Sistema

- **Sistema Operacional**: Linux x86_64 (Ubuntu 22.04 / 24.04+, Debian 12+ ou derivados)
- **Compilador C++**: GCC 13+ / GCC 15+ ou Clang 16+ (com suporte a C++17 e C++20)
- **CMake**: Versão 3.16 ou superior
- **Qt**: Qt 6.5+ (módulos Core, Gui, Widgets, Network, Sql)
- **Python**: Python 3.10 a 3.14 (cabeçalhos de desenvolvimento `python3-dev`)
- **Boost**: `libboost-python-dev` compilada para Python 3

---

## 📦 Instalação de Dependências

No Ubuntu / Debian e derivados, instale todas as bibliotecas e ferramentas necessárias com um único comando:

```bash
sudo apt update
sudo apt install -y \
    build-essential \
    cmake \
    ninja-build \
    qt6-base-dev \
    qt6-tools-dev \
    libboost-python-dev \
    python3-dev \
    zlib1g-dev
```

---

## 🔨 Como Compilar

### 1. Clonar o Repositório

```bash
git clone -b feature/qt6-python3 https://github.com/mbnunes/Wolfpack-Qt5.git
cd Wolfpack-Qt5/server
```

### 2. Configurar o Projeto com CMake

Gere os arquivos de build dentro de um diretório isolado:

```bash
cmake -B build_cmake -S . -DCMAKE_BUILD_TYPE=RelWithDebInfo
```

> **Dica**: Para compilação final otimizada para produção, utilize `-DCMAKE_BUILD_TYPE=Release`.

### 3. Compilar os Executáveis

Compile utilizando 4 processos concorrentes:

```bash
cmake --build build_cmake -j4
```

Ao término, dois executáveis serão gerados dentro do diretório `server/release/`:
1. `wolfpack`: Servidor com interface gráfica Qt (janela com abas de status, logs, console de comando e monitoramento).
2. `wolfpack-console`: Servidor em modo console direto no terminal (ideal para servidores dedicados, scripts de inicialização systemd ou containers Docker).

---

## 🎮 Como Executar

Acesse a pasta de execução `server/release`:

```bash
cd release
```

### Executando em Modo Console (Recomendado para servidores)

```bash
./wolfpack-console
```

### Executando em Modo Gráfico (GUI)

```bash
./wolfpack
```

### Portas de Rede Padrão

O servidor iniciará ouvindo nos seguintes serviços:

| Serviço | Porta | Descrição |
| :--- | :--- | :--- |
| **LoginServer** | `2593` | Autenticação inicial de login e senha do cliente UO |
| **GameServer** | `2592` | Comunicação principal do mundo de jogo após seleção do Shard |
| **RemoteAdmin** | `2594` | Painel de administração remota via socket |

---

## ⚙️ Configuração do Servidor

O arquivo principal de configuração do servidor é o `server/release/wolfpack.xml`.

Algumas opções frequentes:

- **Caminho dos Arquivos do Cliente UO (`MulPath`)**:
  ```xml
  <option key="MulPath" value="/caminho/para/arquivos/do/uo/" />
  ```
- **Criação Automática de Contas**:
  ```xml
  <option key="Auto Create" value="true" />
  ```
- **Configuração de Shard / IP do Servidor**:
  ```xml
  <option key="Shard 1" value="Nome Do Seu Shard=127.0.0.1,2592" />
  ```

---

## 📁 Estrutura do Repositório

```text
Wolfpack-Qt5/
├── README.md                 # Documentação principal
├── server/                   # Código-fonte e runtime do servidor
│   ├── CMakeLists.txt        # Configuração de compilação CMake
│   ├── src/                  # Código-fonte C++ (Core, Network, Python C-API)
│   ├── release/              # Diretório de execução do servidor
│   │   ├── wolfpack.xml      # Arquivo de configurações
│   │   ├── world.bin         # Base de dados binária do mundo
│   │   ├── definitions/      # Definições XML de itens, NPCs e menus
│   │   └── scripts/          # Scripts dinâmicos do jogo em Python 3
│   └── docs/                 # Documentação técnica e esquemas
├── client/                   # Ferramentas complementares do cliente
└── gumps/                    # Recursos de gumps e interface
```

---

## 🐍 Scripts e Extensibilidade (Python 3)

Todos os scripts de jogo localizados em `server/release/scripts/` operam nativamente em **Python 3**:

- **Perícias e Magias**: Implementação completa dos 8 círculos de magia, necromancia, arquearia, ferraria, alquimia, etc.
- **Sistemas de Jogo**: Agricultura/botânica (`plants/`), inteligência artificial de NPCs (`npc/`), vendedores de jogadores (`playervendor/`), barcos (`boats/`), multideeds e casas (`deeds/`).
- **WebAdmin & Status**: Servidor HTTP embutido para monitoramento web e relatórios de status.

Para criar ou modificar um script de comando, basta adicionar um arquivo Python na pasta `server/release/scripts/commands/` e registrar o gatilho com o decorador ou gancho do Wolfpack.

---

## 📖 Wiki e Documentação

Para tutoriais complementares, criação de scripts customizados, novos comandos e guias passo a passo, consulte a [Wiki Oficial do Projeto no GitHub](https://github.com/mbnunes/Wolfpack-Qt5/wiki):

- [Como Compilar o Wolfpack (Wiki)](https://github.com/mbnunes/Wolfpack-Qt5/wiki/Como-compilar-o-Wolfpack)
- [How to Compile Wolfpack (English)](https://github.com/mbnunes/Wolfpack-Qt5/wiki/Compile-Instructions)
- [Começando com o Wolfpack](https://github.com/mbnunes/Wolfpack-Qt5/wiki/Come%C3%A7ando-com-o-Wolfpack)
- [Adicionando um Novo Comando](https://github.com/mbnunes/Wolfpack-Qt5/wiki/Adicionando-um-novo-comando)

---

## 📜 Licença

O Wolfpack é um software livre de código aberto distribuído sob os termos da **GNU General Public License (GPL)**.
