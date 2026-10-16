# Guia Completo de Comandos do Wolfpack (In-Game, Console e Web)

Bem-vindo à documentação oficial de comandos do **Wolfpack Server Emulator** (Qt 6 & Python 3).

Este guia foi elaborado para administradores, desenvolvedores e jogadores que desejam conhecer todos os recursos de gerenciamento in-game, moderação, navegação, construção e controle de spawns do emulador.

---

## 📋 Sumário

1. [Como Executar Comandos](#-como-executar-comandos)
2. [Níveis de Acesso (PLevel / Ranks)](#-níveis-de-acesso-plevel--ranks)
3. [Menu Visual de Ajuda: `'commands`](#-menu-visual-de-ajuda-commands)
4. [Administração do Servidor](#-administração-do-servidor)
5. [Spawns, NPCs e População do Mundo](#-spawns-npcs-e-população-do-mundo)
6. [Navegação e Teleporte: Gump `'go`](#-navegação-e-teleporte-gump-go)
7. [Construção, Decoração e Edição de Itens](#-construção-decoração-e-edição-de-itens)
8. [Custom Housing (AOS / High Seas)](#-custom-housing-aos--high-seas)
9. [Moderação e Gestão de Jogadores](#-moderação-e-gestão-de-jogadores)
10. [Comandos de Jogador (PLevel 0)](#-comandos-de-jogador-plevel-0)
11. [Portal Web e Console Remoto (WebAdmin)](#-portal-web-e-console-remoto-webadmin)
12. [Como Criar Novos Comandos em Python 3](#-como-criar-novos-comandos-em-python-3)

---

## ⚡ Como Executar Comandos

Os comandos no Wolfpack podem ser executados de três formas:

1. **In-Game (Cliente Ultima Online)**:
   * No chat do cliente, digite o apóstrofo/aspas simples `'` antes do comando.
   * Exemplo: `'go`, `'respawntowns`, `'commands`, `'save`.
2. **Console Local do Servidor (`wolfpack-console`)**:
   * Digite o comando diretamente no terminal sem o prefixo `'`.
   * Pressione `S` para habilitar ou desabilitar o *Secure Mode* se necessário.
3. **Painel WebAdmin (`http://127.0.0.1:2594/console.py`)**:
   * Execute qualquer comando administrativo diretamente pelo navegador na aba **Console**.

---

## 🛡️ Níveis de Acesso (PLevel / Ranks)

O Wolfpack utiliza um sistema de Lista de Controle de Acesso (ACL) definido em `definitions/system/access.xml`. Cada conta possui um nível de privilégio que determina quais comandos podem ser executados:

| Rank | ID ACL | Papel | Descrição |
|:---:|:---:|:---|:---|
| **0** | `player` | **Player** | Jogador padrão. Acesso a comandos utilitários (`where`, `who`, `help`, `afk`, etc.). |
| **25** | `counselor` | **Counselor** | Assistente de suporte a novatos, atendimento a páginas e teleporte básico. |
| **50** | `seer` | **Seer** | Organizador de eventos e construtor. Acesso a `'go'`, `'add'`, `'respawntowns'`, etc. |
| **75** | `gm` | **GameMaster** | Administrador de mundo, moderação, prisão, restock e controle de spawns. |
| **99** | `coadmin` | **Co-Admin** | Acesso quase irrestrito exceto recarregamento bruto ou shutdown. |
| **100** | `admin` | **Administrator** | Acesso total e absoluto a todas as diretivas do servidor. |

---

## 📖 Menu Visual de Ajuda: `'commands`

Para consultar a lista de comandos permitidos diretamente dentro do jogo, utilize:

```text
'commands
'cmd
```

### Recursos do Gump `'commands`:
* **Filtro Automático de Permissão**: Mostra apenas os comandos que o seu personagem tem direito de usar com base no seu PLevel.
* **Navegação por Categorias**:
  * ⚙️ **Admin**: Salvamento, recarregamento e controle do servidor.
  * 🗺️ **Teleport**: Deslocamento entre mapas, coordenadas e busca de jogadores.
  * 🐉 **NPC/Spawns**: Respawn de cidades, masmorras e lojistas.
  * 🔨 **Building**: Construção, decoração, manipulação de tiles e estáticos.
  * ⚖️ **Moderation**: Punição, congelamento, cura e supervisão de jogadores.
  * 📜 **Properties**: Inspeção de tags, scripts e propriedades dos objetos.
  * 👤 **Player**: Comandos públicos do jogador comum.
* **Barra de Pesquisa**: Digite o nome ou função do comando para filtrá-lo instantaneamente.
* **Execução Rápida**: Clique no botão ao lado do comando para executá-lo imediatamente ou abrir seu gump de configuração.

---

## ⚙️ Administração do Servidor

Comandos centrais para gerenciar a estabilidade e integridade do Wolfpack:

| Comando | Rank Mínimo | Descrição e Sintaxe |
|:---|:---:|:---|
| `'save` | GM (75) | Salva o estado completo do mundo e das contas no banco de dados (SQLite/MySQL com `REPLACE INTO`). |
| `'reload <alvo>` | Admin (100) | Recarrega componentes em tempo real sem desligar o servidor.<br>• `'reload scripts` (recarrega todo o ecossistema Python 3)<br>• `'reload definitions` (recarrega XMLs de itens e NPCs)<br>• `'reload accounts` (recarrega contas) |
| `'shutdown [segundos]` | Admin (100) | Encerra o servidor com segurança, realizando um ciclo prévio de autosave. |
| `'broadcast <mensagem>` | Seer (50) | Envia um comunicado no centro da tela para todos os jogadores online. |
| `'gmtalk <mensagem>` | Seer (50) | Canal de comunicação seguro e exclusivo entre os membros da staff. |
| `'staff [0\|1]` | Seer (50) | Alterna o status de Staff do seu personagem (concede invulnerabilidade e invisibilidade de monitoramento). |
| `'servertime` | Counselor (25) | Exibe o horário real do sistema e o tempo decorrido desde o boot do servidor. |
| `'showserials` | GM (75) | Ativa a visualização do serial hexadecimal flutuando sobre a cabeça de itens e NPCs. |

---

## 🐉 Spawns, NPCs e População do Mundo

O Wolfpack conta com um motor híbrido de spawn regions em C++ e scripts de Champion Spawns em Python.

### 1. Novo Comando de Respawn de Cidades: `'respawntowns`

Permite repovoar instantaneamente vendedores, guardas, cidadãos e animais urbanos em todos os mapas:

```text
'respawntowns
```
> **Efeito:** Varre as **417 spawnregions de cidades** do mundo e preenche todos os lojistas, guardas e cidadãos até o limite máximo.

#### Subcomandos Úteis:
* **Respawn de uma cidade específica:**
  ```text
  'respawntowns britain
  'respawntowns luna
  'respawntowns zento
  'respawntowns cove
  ```
* **Listar todas as cidades suportadas:**
  ```text
  'respawntowns list
  ```
* **Limpar/remover spawns de cidades:**
  ```text
  'respawntowns clear
  'respawntowns clear britain
  ```

---

### 2. Controle de Alta Performance com `'spawnregion`

Comando nativo da engine C++ para controle granular de regiões:

| Comando | Descrição |
|:---|:---|
| `'spawnregion fill towns` | Preenche todas as cidades do jogo até o limite máximo. |
| `'spawnregion respawn towns` | Dispara um ciclo padrão de respawn nas cidades. |
| `'spawnregion clear towns` | Limpa todos os NPCs/itens gerados nas cidades. |
| `'spawnregion fill vendors` | Preenche todos os vendedores e lojistas de todas as facetas. |
| `'spawnregion fill all` | Preenche o mundo inteiro (cidades, dungeons, mares e florestas). |
| `'spawnregion fill <nome>` | Preenche uma região ou grupo específico (ex: `'spawnregion fill destard`). |
| `'spawnregion info <nome>` | Abre um gump detalhado com estatísticas de população da região. |

---

### 3. Ferramentas Adicionais de Spawns

* `'spawn`: Cria uma **SpawnGem** móvel para gerar criaturas personalizadas por raio, tempo e quantidade.
* `'restock`: Força o reabastecimento imediato de estoque e ouro de todos os lojistas/vendors.
* `'champ`: Abre o painel de controle e ativação de **Champion Spawns**.
* `'peerless`: Gerencia chefes e altares Peerless de Mondain's Legacy.
* `'paragon`: Converte a criatura alvo em uma versão **Paragon** com status aprimorados e loot especial.

---

## 🗺️ Navegação e Teleporte: Gump `'go`

O comando `'go` abre uma interface moderna e visual para navegar rapidamente por todo o universo de Ultima Online:

```text
'go
```

### Mapas e Facetas Suportadas (100% integradas com as Expans�es Oficiais):
1. **Trammel**: Cidades clássicas (Britain, Moonglow, Trinsic, Yew, Vesper, Haven, etc.), Dungeons, Santuários e Locais de Interesse.
2. **Felucca**: Todas as cidades em modo PvP, Dungeons (Despise, Destard, Shame, Deceit, Covetous, etc.), Altares de Champion Spawns e Covis de Piratas.
3. **Ilshenar**: Santuários dos Princípios, Cidades Místicas (Lakeshire, Mistas, Montor, Gargoyle City) e Masmorras (Blood, Sorcerer, Exodus).
4. **Malas**: Luna (A Cidade Sagrada), Umbra (A Cidade dos Necromantes) e a Masmorra Doom.
5. **Tokuno Islands**: Isamu-Jima, Makoto-Jima (Zento) e Homare-Jima com seus templos e santuários.
6. **Ter Mur**: Royal City, Holy City, Underworld, Tomb of Kings e a Stygian Abyss.

### Outros Comandos de Movimentação:
* `'goname <nome>`: Teleporta o seu personagem para junto de um jogador ou NPC pelo nome.
* `'gouid <serial>`: Teleporta para a posição exata de um item ou personagem pelo seu Serial Hexadecimal.
* `'send <jogador> <destino>`: Teleporta outro jogador para uma cidade ou coordenada.
* `'where`: Exibe suas coordenadas exatas (`X, Y, Z, Mapa`).
* `'who`: Lista todos os jogadores conectados com atalhos de teleporte rápido.

---

## 🔨 Construção, Decoração e Edição de Itens

Conjunto essencial de ferramentas para construtores, arquitetos e GameMasters:

| Comando | Descrição |
|:---|:---|
| `'add <item\|npc>` | Abre o catálogo visual de criação ou instancia diretamente um ID/Definição. |
| `'remove` | Apaga permanentemente do mundo o item ou criatura alvo. |
| `'dupe [quant]` | Clona o item selecionado e o coloca diretamente na sua mochila. |
| `'edit` ou `'props` | Abre o editor visual de propriedades (Weight, Hue, Movable, Name, Script, Layer). |
| `'tile <id>` | Ferramenta de ladrilhamento automático entre dois pontos para pisos, muros e calçadas. |
| `'makestatic` | Converte um item dinâmico em parte estática permanente do mapa. |
| `'allmove` | Habilita modo livre de movimentação (permite arrastar itens travados ou estáticos). |
| `'allshow` | Revela itens ocultos, portas secretas e armadilhas invisíveis. |
| `'nudgeup [altura]` | Eleva a altura Z do item alvo. |
| `'nudgedown [altura]` | Reduz a altura Z do item alvo. |
| `'dye [hue]` | Abre a paleta de cores ou tinge o objeto alvo com um Hue hexadecimal direto. |
| `'settag <chave> <valor>` | Adiciona uma tag customizada de script ao objeto alvo. |
| `'deltag <chave>` | Remove uma tag de script do objeto alvo. |

---

## 🏠 Custom Housing (AOS / High Seas)

O Wolfpack possui um sistema completo e moderno de construção customizada de residências (Custom Housing Designer):

* **Comando In-Game:**
  ```text
  'customhouse
  ```
* **Funcionalidades do Designer:**
  * **Categorias de Peças**: Paredes (Walls), Portas (Doors), Pisos (Floors), Escadas (Stairs) e Telhados (Roofs).
  * **Regras Estruturais (Física de Construção)**: Validação em tempo real baseada nas regras de `suppinfo.txt` para impedir paredes e pisos flutuantes.
  * **Controle de Andares**: Navegação por até 4 andares e cobertura.
  * **Backup e Restauração**: Opção de salvar rascunhos, reverter alterações e restaurar o estado original da fundação.
  * **Cálculo de Custos e Commit**: Deduz o valor das alterações da conta bancária do proprietário e publica a estrutura diretamente no arquivo comprimido de rede (`0xD8`).

---

## ⚖️ Moderação e Gestão de Jogadores

Ferramentas para supervisão e aplicação de regras:

* `'jail <minutos>`: Envia o jogador selecionado para a prisão pelo tempo estipulado.
* `'forgive`: Libera o jogador da prisão antes do término da pena.
* `'freeze` / `'unfreeze`: Paralisa ou desconcongela o personagem alvo.
* `'mute`: Impede o jogador de falar no chat público ou canais globais.
* `'disconnect`: Desconecta a conexão do socket do jogador.
* `'kill`: Mata instantaneamente a criatura ou jogador alvo.
* `'resurrect`: Ressuscita o fantasma do jogador alvo.
* `'heal`: Restaura 100% dos pontos de vida (HP), mana e vigor (Stamina).
* `'invul`: Torna o personagem imune a danos físicos e mágicos.
* `'invis`: Torna o personagem completamente invisível para outros jogadores.
* `'allskills [valor]`: Define todas as habilidades do personagem para o valor especificado (ex: `'allskills 100.0`).
* `'nightsight`: Concede visão noturna permanente ao jogador.

---

## 👤 Comandos de Jogador (PLevel 0)

Comandos públicos disponíveis para qualquer jogador comum:

* `'help`: Abre o menu de suporte e ajuda ao novato.
* `'where`: Informa suas coordenadas e o nome da região onde você está.
* `'who`: Lista a quantidade de jogadores conectados no servidor.
* `'motd`: Reexibe a mensagem do dia configurada no servidor.
* `'afk [motivo]`: Marca o personagem como ausente e avisa quem enviar mensagens privadas.
* `'password <nova_senha>`: Altera a senha da conta com criptografia segura SHA-512.
* `'stuck`: Sistema de socorro para personagens presos em paredes ou terrenos acidentados.
* `'bank`: Abre o cofre do banco (quando próximo a um Banqueiro em cidades guardadas).
* `'guards`: Chama a guarda urbana se houver criminosos na tela dentro de zonas protegidas.

---

## 🌐 Portal Web e Console Remoto (WebAdmin)

O Wolfpack conta com um servidor web HTTP embutido que pode ser acessado em qualquer navegador moderno:

📍 **URL Padrão:** `http://127.0.0.1:2594/`

### 1. Área Pública e Portal do Jogador:
* **Login de Conta**: Jogadores podem autenticar com seu login e senha do jogo.
* **Ficha de Personagens**:
  * Estatísticas completas: Força (Str), Destreza (Dex), Inteligência (Int), Vida, Mana e Stamina.
  * Árvore completa de 55+ habilidades (Skills) com pontuação atual e base.
  * Posição no mundo e mapa onde o personagem está deslogado.
* **Cofre do Banco**: Consulta do saldo de ouro e inventário guardado com segurança.
* **Propriedades e Casas**: Listagem de todas as residências que o jogador possui, localização no mapa e status de decay.

### 2. Painel Administrativo (Staff):
* **Gerenciamento de Contas**: Criar contas, banir, suspender, alterar senhas e níveis de privilégio (PLevel).
* **Console Interativo em Tempo Real**: Envie qualquer comando listado neste guia diretamente do navegador e veja a saída instantânea.
* **Mapas por Satélite**: Visualizador cartográfico interativo de todas as 6 facetas do jogo.
* **Logs do Python e do Servidor**: Diagnóstico em tempo real de erros, avisos e execuções de scripts.

---

## 🐍 Como Criar Novos Comandos em Python 3

Criar comandos no Wolfpack é extremamente simples e modular graças ao ecossistema Python 3.

### Exemplo Prático: Criando o comando `'bemvindo`

1. Crie um arquivo Python na pasta:
   ```text
   server/release/scripts/commands/bemvindo.py
   ```

2. Adicione o seguinte código:
   ```python
   # -*- coding: utf-8 -*-
   import wolfpack

   def commandBemVindo(socket, command, args):
       """
       Comando de boas-vindas para novos jogadores.
       Uso: 'bemvindo
       """
       char = socket.player
       if not char:
           return True

       socket.sysmessage("Bem-vindo às terras lendárias do Wolfpack!")
       socket.sysmessage(f"Seu personagem atual é: {char.name}")
       socket.sysmessage("Digite 'commands para visualizar todos os comandos disponíveis.")
       
       # Cria um pequeno efeito visual de partículas no personagem
       char.effect(0x376A, 10, 15)
       char.sound(0x1F2)
       return True

   def onLoad():
       """
       Registra o comando durante o carregamento dos scripts.
       """
       wolfpack.registercommand("bemvindo", commandBemVindo)
   ```

3. Registre o script em `server/release/definitions/scripts.xml` e defina as permissões de acesso em `access.xml`.
4. No jogo ou no console, execute:
   ```text
   'reload scripts
   ```
5. Pronto! O comando `'bemvindo` estará funcionando imediatamente sem necessidade de reiniciar o servidor.

---

*Documentação mantida pela comunidade do Wolfpack Server Emulator.*  
*Para dúvidas e contribuições, acesse o repositório oficial no GitHub.*
