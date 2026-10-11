# Arquitetura e Formatos de Dados: Definições XML vs. Componentes (.txt)

Este documento detalha o propósito e a convivência de dois formatos de dados fundamentais no Wolfpack:
1. **Definições em XML** (`definitions/**/*.xml`)
2. **Tabelas de Componentes em texto** (`data/components/*.txt`)

---

## 1. Definições de Itens e Mundo em XML (`definitions/`)

Os arquivos XML presentes em `definitions/` (como `definitions/items/buildings/walls.xml`, `doors.xml`, etc.) definem **entidades individuais de jogo**:

- **O que representam**: Objetos físicos instanciados no mundo do Ultima Online (`cItem`).
- **Características**:
  - Possuem um identificador e modelo gráfico (`<id>0x6</id>`).
  - Possuem propriedades de gameplay (`<nodecay/>`, `<weight>`, `<value>`, `<script>`).
  - Possuem metadados de categorização nos menus de Game Master (`<category>Buildings\Walls\...</category>`).
- **Ciclo de vida**: Criados por comandos como `.add 6`, vendidos por vendors, dropados como loot ou colocados estaticamente no mapa. Cada um tem seu próprio `serial` de objeto.

---

## 2. Componentes de Custom Housing em .txt (`data/components/`)

Os arquivos em `data/components/` (`walls.txt`, `floors.txt`, `doors.txt`, `roof.txt`, `stairs.txt`, `suppinfo.txt`, `teleprts.txt`) vêm do sistema oficial de **Custom Housing (AOS / High Seas)** e são mantidos como padrão de mercado (compatíveis com ServUO e RunUO):

- **O que representam**: O **catálogo dimensional e as matrizes de regras estruturais** utilizadas pelo cliente do UO na ferramenta de arquitetura (*Custom Housing Designer*).
- **Por que são tabelas relacionais em texto**:
  - No designer de casas, paredes e pisos **não são itens do mundo (`cItem`)**. São números inteiros gravados diretamente em blocos de bytes comprimidos dentro do arquivo de fundação da casa (`multi.idx`/`multi.mul` e pacotes de rede `0xD8`).
  - Uma parede não é apenas um ID de tile. Ela é composta por relações geométricas complexas entre peças:
    ```tsv
    Category  Style  TID       South1  South2  South3  Corner  East1  East2  East3  Post  WindowS  WindowE ...
    0         0      1060054   10      7       12      6       13     8      11     9     14       15      ...
    ```
    Isso ensina ao cliente e ao servidor que:
    - O canto é o tile `6`.
    - As paredes retas viradas para o sul são `10, 7, 12`.
    - As paredes viradas para o leste são `13, 8, 11`.
    - A coluna/poste de sustentação é `9`.
    - As janelas associadas a esse estilo são `14` e `15`.
  - O arquivo `suppinfo.txt` contém milhares de regras de física estrutural (suporte de peso de andares superiores, onde telhados podem se apoiar, escadas válidas, etc.).

---

## 3. Por que manter em `.txt` em vez de converter para XML?

Embora seja tecnicamente viável converter essas matrizes para tags XML do Wolfpack, a decisão de mantê-las em `.txt` traz três grandes vantagens técnicas:

1. **Interoperabilidade e Atualização Imediata com Expansões do UO**:
   - A comunidade do ServUO / RunUO mantém essas matrizes rigorosamente atualizadas com cada nova expansão oficial da Broadsword / EA (como *Time of Legends*, novos temas de casas, *High Seas*).
   - Quando novos tiles são lançados, basta atualizar os arquivos `.txt` diretamente a partir do repositório de dados oficial, sem precisar reescrever e validar manualmente milhares de linhas de tags XML proprietárias.

2. **Desempenho e Consumo de Memória no Boot**:
   - Carregar matrizes tabulares delimitadas por tabulação diretamente para vetores em memória (`int[]`) leva **menos de 2 milissegundos**.
   - Fazer parsing de milhares de nós de um XML dimensional complexo na inicialização aumentaria o tempo de boot e o consumo de heap sem nenhum ganho real.

3. **Inexistência de Duplicidade Real de Dados**:
   - Os XMLs definem o *item* (atributos, pesos, comportamentos).
   - Os `.txt` definem a *matriz de encaixe arquitetônico* (geometria de construção).
   - Modificar atributos do item `0x6` no XML (como peso ou script) não afeta a geometria da parede na casa do jogador, e vice-versa.
