# Changelog

Todas as alterações notáveis, correções e novos recursos deste projeto serão documentados neste arquivo.

O formato segue as diretrizes do [Keep a Changelog](https://keepachangelog.com/pt-BR/1.0.0/).

---

## [13.1.0] - Outubro de 2026

### ⚓ Sistema Naval & Expansão High Seas
- **Roda do Leme & Mouse Steering (`boats.wheel`)**:
  - Implementada pilotagem interativa por timão (`shipwheel`, gráfico `0x1E23`).
  - Gump de navegação contínua com rosa dos ventos (avançar, recuar, deriva lateral, giros de 90° e manobra de 180° *Come About*).
  - Controle instantâneo de velocidade da vela (*Full Sail* vs. *Half Sail*) e liberação/lançamento de âncora.
- **Linhas de Amarração (`boats.mooring`)**:
  - Implementado item `mooring_line` (gráfico `0x14F8`) para embarque rápido de docas/terreno em até 8 tiles sem necessidade de pranchas.
  - Desembarque automático para a terra firme ou píer mais próximo ao clicar duas vezes a bordo.
- **Artilharia Naval & Combate de Canhões (`boats.cannons`)**:
  - Canhões navais leves e pesados (`ship_cannon_heavy`, `ship_cannon_light`) implantáveis via escrituras no convés.
  - Estação de artilharia completa com ciclo realista de disparo: Limpeza com escovilhão (`swab`), carga de pólvora (`powder_charge`), inserção de projétil com soquete (`cannonball`, `grapeshot`, `ramrod`) e escorva com estopim (`fuse_match`).
  - Animação de projétil em voo (`0x0E73`), estrondo de pólvora (`0x11C`), fumaça, explosão no impacto (`0x207`, `0x36B0`) e cálculo de dano estrutural e em criaturas.
- **Integridade Estrutural & Reparos de Emergência (`boats.damage`)**:
  - Sistema de pontos de vida (HP) estruturais para todas as embarcações (1.000 HP para Rowboats até 20.000 HP para o Britannian Ship).
  - 4 estados de avaria (*Pristine*, *Damaged*, *Heavily Damaged* e *Scuttled*). Navios com 0 HP ficam incapacitados até serem reparados.
  - Reparos de emergência com madeira (`boards`/`logs`) e `ship_repair_kit`.
- **Galleon Pilot NPC (`boats.pilot`)**:
  - NPC navegador e sentinela gerado automaticamente em todos os galeões High Seas.
  - Responde a comandos táticos por voz e abre o menu de comando do galeão (leme, segurança, avarias, âncora).
- **Sistema de Segurança em 5 Níveis (`boats.security`)**:
  - Hierarquia de tripulação vinculada ao casco: *Captain*, *Officer*, *Crewman*, *Passenger* e *Denied*.
  - Gump de manifesto de bordo para nomeação e revogação de patentes sem depender de chaves físicas.
  - Alternância entre acesso público e privado. Pranchas e cordas de amarração integradas à checagem de patentes.
- **Radar Marítimo & Runas Navais (`boats.tracking`, `boats.runes`)**:
  - Varredura de radar marítimo via luneta (`spyglass`) com alcance de 200 tiles, informando distância, rumo cardinal e fixação de rastreio.
  - Runas de teleporte naval (`ship_rune`, tom `0x47F`), sintonizáveis por oficiais para *Recall* diretamente ao convés em alto mar.

### 🏰 Construções do Contest Broadsword & MultiCollection.uop
- **Suporte Nativo a MultiCollection.uop**:
  - Leitor de multis em C++ (`cMultiCache::load`) aprimorado para suplementar `multi.idx`/`multi.mul` lendo arquivos modernos de coleções `.uop` (872 multis carregados sem duplicatas).
- **20 Novos Castelos e Fortalezas Broadsword**:
  - Adicionados multis e escrituras de castelos contestados: *Trinsic Keep*, *Gothic Rose Castle*, *Feudal Castle*, *Castle of Oceania*, *The Sandstone Castle*, *Grimswind Sisters*, *Robin's Roost*, etc.
- **Atualização da Ferramenta de Construção ("A Luneta" - ID `0x14f6`)**:
  - Gump redesenhado com paginação dinâmica para as 20 novas construções contestadas e menu de compra direta de escrituras de embarcações clássicas e High Seas.

### 📦 Migração de Dados do Expans�es Oficiais
- **Itens e Equipamentos**:
  - Migrados 48 itens concretos ausentes: espadas e cajados de caveira Gargish, capuzes de mago e assassino, instrumentos musicais raros (Aud-Char, Cowbell, Cello, Trumpet), manto de humildade e consumíveis da expansão.
- **Decoração e Portas de Ter Mur**:
  - Geradas decorações estáticas e portas automáticas para o mapa 5 (Ter Mur).
  - Correção de carregamento de itens com IDs não scriptados e mapeamento de portas gargoyle.

---

## [13.0.0] - Modernização de Core (Qt 6 & Python 3)

### 🚀 Engine C++ e Plataforma
- **Migração para Qt 6**:
  - Atualização completa do núcleo de sockets, rede, eventos e interface de usuário para **Qt 6.10+**.
- **Python 3 Embutido**:
  - Suporte completo a **Python 3.10 até Python 3.14+** via CPython C-API e Boost.Python moderno.
  - Migração de todos os scripts de jogo (`server/release/scripts/`) eliminando funções obsoletas do Python 2.
  - Slots de comparação rica (`tp_richcompare`) e registros unificados de submódulos C++.
- **Sistema de Build com CMake**:
  - Substituição do qmake por CMake moderno (`CMakeLists.txt`), com suporte a cabeçalhos pré-compilados e compilação paralela rápida.
  - Geração de binários dedicados para GUI (`wolfpack`) e console (`wolfpack-console`).

### 🗄️ Banco de Dados & Salvamento de Mundo
- **Unificação de Banco de Dados (`Database Driver = "same"`)**:
  - Contas e dados do mundo compartilham o mesmo schema no MariaDB/MySQL.
- **SQLite 3 Moderno**:
  - Ativação de PRAGMAs de alta velocidade (`WAL`, `synchronous=NORMAL`, `foreign_keys=ON`).
- **MariaDB 11.8+ / MySQL 8.x**:
  - DDL atualizado para `ENGINE=InnoDB DEFAULT CHARSET=utf8mb4`.
  - Salvamento incremental com upsert (`REPLACE INTO`), reduzindo tempo de autosave de 50s para menos de 800ms.
- **Segurança de Senhas (SHA-512)**:
  - Substituição do MD5 legado por SHA-512 (128 hex), com migração transparente e silenciosa no login dos jogadores.

### 🛡️ Rede, Criptografia e Estabilidade
- **Criptografia Twofish para 64-bit**:
  - Correção de overflow de pilha (`__stack_chk_fail`) no algoritmo Twofish, tipando `DWORD` como `quint32` (4 bytes exatos).
- **Proteção contra Tráfego Inválido**:
  - Desconexão graciosa em sockets de login e game contra requisições web/scanners sem derrubar o servidor.
  - Desacoplamento do servidor web administrativo (*Remote Admin*) para a porta dedicada `2594`.
- **Compatibilidade de Spellbooks com Clientes Modernos**:
  - Pacote adaptativo `0x24` (9 bytes para High Seas / 7.0.9+, 7 bytes clássico) e correção de offsets no pacote `0xBF 0x1B`.
- **Custom Housing AoS / High Seas**:
  - Leitor de multi em cache de memória (<5ms) e protocolo de pacotes `0xBF 0x1D/0x1E`, `0xD7` e `0xD8`.
- **Modo Staff & Proteção de Cidades**:
  - Comando `'staff` com invulnerabilidade e trégua de combate.
  - Bloqueio nativo de spawn agressivo em regiões guardadas, ajustado 1:1 com os dados do Expans�es Oficiais.
