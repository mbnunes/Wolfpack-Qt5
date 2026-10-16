# ⚓ Guia Completo do Sistema de Navegação e Combate Naval (High Seas)

Este guia ensina passo a passo como utilizar todas as novas funcionalidades do **Sistema Naval e Combate Marítimo** implementado no Wolfpack, trazendo toda a experiência da expansão *High Seas* do Ultima Online.

---

## 📋 Sumário

1. [Visão Geral](#-visão-geral)
2. [Obtendo e Posicionando sua Embarcação](#-obtendo-e-posicionando-sua-embarcação)
3. [Pilotagem e Controle do Navio](#-pilotagem-e-controle-do-navio)
   - [Roda do Leme (Mouse Steering / Helm Gump)](#roda-do-leme-mouse-steering--helm-gump)
   - [Comandos de Voz Clássicos](#comandos-de-voz-clássicos)
4. [Embarque e Desembarque](#-embarque-e-desembarque)
   - [Cabos de Amarração (Mooring Lines)](#cabos-de-amarração-mooring-lines)
   - [Pranchas de Acesso (Gangplanks)](#pranchas-de-acesso-gangplanks)
5. [Sistema de Segurança e Tripulação](#-sistema-de-segurança-e-tripulação)
   - [Níveis de Patente (Manifesto de Bordo)](#níveis-de-patente-manifesto-de-bordo)
   - [Acesso Público vs. Privado](#acesso-público-vs-privado)
6. [Artilharia Naval (Canhões e Munições)](#-artilharia-naval-canhões-e-munições)
   - [Instalando Canhões](#instalando-canhões)
   - [Suprimentos Necessários](#suprimentos-necessários)
   - [Ciclo de Disparo (Passo a Passo)](#ciclo-de-disparo-passo-a-passo)
7. [Integridade do Casco e Reparos](#-integridade-do-casco-e-reparos)
   - [Estados de Avaria (Pontos de Vida)](#estados-de-avaria-pontos-de-vida)
   - [Conduzindo Reparos de Emergência](#conduzindo-reparos-de-emergência)
8. [O Piloto do Galeão (Galleon Pilot NPC)](#-o-piloto-do-galeão-galleon-pilot-npc)
9. [Radar Marítimo e Runas Navais](#-radar-marítimo-e-runas-navais)
   - [Rastreamento com Luneta (Sea Radar)](#rastreamento-com-luneta-sea-radar)
   - [Sintonizando e Usando Runas Navais](#sintonizando-e-usando-runas-navais)

---

## 🌊 Visão Geral

O sistema naval do Wolfpack suporta desde botes leves até massivos galeões de guerra:
- **Embarcações Clássicas**: Small Ship, Dragon Small Ship, Medium Ship, Dragon Medium Ship, Large Ship, Large Dragon Ship.
- **Embarcações High Seas**: Rowboat (Bote de Remos), Tokuno Galleon, Gargish Galleon, Orcish Galleon e Britannian Ship.

Todos os navios possuem integridade física (HP), suporte a canhões navais, controle por timão com mouse e sistema hierárquico de tripulação.

---

## 🚢 Obtendo e Posicionando sua Embarcação

### Onde Comprar:
1. **No Vendedor Shipwright**:
   - Visite qualquer carpinteiro naval nas docas das cidades (Britain, Vesper, Trinsic, Jhelom, Zento, etc.).
   - Todas as escrituras de barcos clássicos e galeões High Seas estão disponíveis para compra direta com ouro.
2. **Pela Ferramenta "A Luneta" (House Placement Tool - ID `0x14f6`)**:
   - Dê duplo-clique na luneta de construção.
   - Clique no botão **"Ships & Galleons"**.
   - Escolha o barco desejado e adquira a escritura instantaneamente direto do saldo do seu banco.

### Como Posicionar no Mar:
1. Fique próximo à margem ou doca de frente para a água.
2. Dê duplo-clique na escritura do barco em sua mochila.
3. Um cursor de mira azul aparecerá mostrando o contorno do navio.
4. Clique em uma área livre de água profunda.
5. O barco será criado na água, com sua âncora lançada e pronto para embarque. Em galeões, a roda do leme, cabo de amarração e piloto já são gerados no convés!

---

## 🧭 Pilotagem e Controle do Navio

### Roda do Leme (Mouse Steering / Helm Gump)
Galeões contam com uma roda de timão (**Ship's Wheel**) no convés superior de popa.
1. Dê duplo-clique na **Roda do Leme**.
2. A janela interativa de navegação (**Helm & Steering Controls**) será exibida:
   - **Rosa dos Ventos**: Botões direcionais para avançar (**Forward**), recuar (**Backward**), derivar lateralmente (**Left/Right**) ou navegar em diagonal (**Forward-Left/Right**).
   - **Girar (Turn Port / Turn Starboard)**: Gira o navio 90° para bombordo ou estibordo.
   - **Come About**: Executa uma manobra de 180° instantânea.
   - **STOP**: Cancela o movimento imediatamente.
   - **Velocidade (Toggle Speed)**: Alterna entre *Full Sail* (3 passos rápidos) e *Half Sail* (1 passo moderado).
   - **Âncora (Anchor)**: Lança ou recolhe a âncora com um clique.
   - **Inspecionar (Inspect Damage)**: Abre o painel de integridade do casco.

> [!TIP]
> O gump de navegação permanece aberto enquanto você navega, permitindo desviar de recifes, ilhas e embarcações inimigas com cliques rápidos sem precisar abrir menus novamente.

### Comandos de Voz Clássicos
Você também pode pilotar falando em voz alta no convés para o Tillerman ou Piloto:
- `"forward"` / `"avançar"`
- `"backward"` / `"recuar"`
- `"turn left"` / `"turn right"` (virar 90°)
- `"left"` / `"right"` (deriva lateral)
- `"stop"` / `"parar"`
- `"lower anchor"` / `"raise anchor"`
- `"come about"` (virar 180°)

---

## ⚓ Embarque e Desembarque

### Cabos de Amarração (Mooring Lines)
Em vez de depender de pranchas lentas ou arriscar cair no mar, utilize o cabo de amarração (`mooring_line`):
- **Para Subir a Bordo**: Da doca ou margem (em até 8 passos de distância), dê duplo-clique no rolo de corda visível na borda do navio. Você será içado diretamente para o convés.
- **Para Desembarcar**: Estando no navio, dê duplo-clique na corda. O sistema localizará o ponto de terra firme ou doca mais próximo em até 8 passos e você desembarcará com segurança.

### Pranchas de Acesso (Gangplanks)
- Dê duplo-clique na prancha lateral para estendê-la ou recolhê-la.
- Se o navio for trancado, membros registrados da tripulação conseguem destravar e embarcar sem necessidade de carregar chaves físicas.

---

## 🛡️ Sistema de Segurança e Tripulação

Diga adeus à perda de barcos por roubo de chaves! Cada navio possui uma lista de controle de acesso vinculada ao casco.

### Níveis de Patente (Manifesto de Bordo)
- **Captain (Proprietário)**: Acesso total. Pode dry-dockar o navio, renomear, alterar permissões e conceder cargos.
- **Officer (Oficial)**: Pode pilotar o timão, disparar canhões, abrir o porão de carga e realizar reparos.
- **Crewman (Tripulante)**: Pode embarcar a qualquer momento, disparar canhões e realizar reparos.
- **Passenger (Passageiro)**: Permissão garantida para embarque e transporte seguro.
- **Denied**: Acesso negado.

### Como Gerenciar a Tripulação:
1. Abra o menu do piloto ou acesse o **Manifesto de Segurança**.
2. Clique em **"Grant Access to Player"** e selecione o jogador a bordo.
3. Escolha a patente desejada (*Officer*, *Crewman* ou *Passenger*).
4. O jogador passará a constar no rol de tripulantes oficiais da embarcação.
5. Para remover alguém, basta clicar em **"Revoke"** ao lado do nome.

### Acesso Público vs. Privado:
- No topo do manifesto, clique em **"Toggle"** para alternar:
  - **PRIVATE**: Apenas capitão, oficiais, tripulantes, passageiros e membros da sua guilda/party podem subir a bordo.
  - **PUBLIC**: Qualquer aventureiro pode subir a bordo usando as pranchas ou cabos de amarração.

---

## 💣 Artilharia Naval (Canhões e Munições)

### Instalando Canhões:
1. Adquira uma escritura de canhão (**Light Ship Cannon Deed** ou **Heavy Ship Cannon Deed**).
2. Estando a bordo do navio, use a escritura e clique no piso do convés onde deseja posicioná-lo.
3. O canhão será montado na posição exata e acompanhará o movimento e rotações do navio.

### Suprimentos Necessários:
Mantenha em sua mochila ou no porão do navio:
- **Cannon Swab** (`swab`): Escovilhão para limpar o interior do tubo.
- **Powder Charge** (`powder_charge`): Carga de pólvora propulsora.
- **Ramrod** (`ramrod`): Soquete de compactação da pólvora e bala.
- **Cannonball** (`cannonball`) ou **Grapeshot** (`grapeshot`): Munição (bala sólida de ferro ou metralha dispersa).
- **Fuse Matches** (`fuse_match`): Fósforos de escorva para acender o ouvido do canhão.

### Ciclo de Disparo (Passo a Passo):
Dê duplo-clique no canhão para abrir a janela da **Estação de Artilharia**:
1. **Passo 1 (Limpeza)**: Clique em `1. Swab & Clean Barrel`. O cano será limpo de resíduos.
2. **Passo 2 (Carga de Pólvora)**: Clique em `2. Load Powder Charge`. Consome 1 carga de pólvora da sua mochila.
3. **Passo 3 (Munição)**: Clique em `3. Ram Ammunition`. Insere e soca a bala de canhão no tubo.
4. **Passo 4 (Escorva)**: Clique em `4. Prime Fuse Cord`. Prepara o estopim de ignição.
5. **Passo 5 (FOGO!)**: O botão verde **FIRE!** ficará ativo! Clique nele e a mira de alvo aparecerá:
   - Mire em criaturas marinhas hostis (Krakens, Serpentes do Mar, Deep Sea Serpents).
   - Mire em jogadores ou no casco de embarcações inimigas (em zonas marítimas de combate).
   - O canhão rugirá com efeito de fumaça, som de explosão e animação do projétil voando até o alvo, causando estrago devastador!
6. Após o tiro, o canhão fica sujo e o ciclo pode ser reiniciado.

> [!NOTE]
> Para desinstalar o canhão e guardá-lo de volta na mochila, clique em **"Dismantle Cannon"** no menu.

---

## 🔨 Integridade do Casco e Reparos

Embarcações atacadas por monstros marinhos ou canhões inimigos perdem pontos de vida estruturais (HP).

### Tabela de Pontos de Vida:
| Embarcação | Pontos de Vida (HP) |
| :--- | :--- |
| **Rowboat** | 1.000 HP |
| **Small Ship** | 2.500 HP |
| **Medium Ship** | 4.000 HP |
| **Large Ship** | 6.000 HP |
| **Tokuno Galleon** | 12.000 HP |
| **Gargish Galleon** | 14.000 HP |
| **Orcish Galleon** | 15.000 HP |
| **Britannian Ship** | 20.000 HP |

### Estados do Navio:
- **Pristine (100%–75%)**: Navio impecável, navegação em velocidade máxima.
- **Damaged (74%–50%)**: Casco danificado, velas com avarias leves.
- **Heavily Damaged (49%–1%)**: Avarias graves, rangidos de madeira e risco iminente de naufrágio.
- **Scuttled (0% - Incapacitado)**: O navio perdeu a flutuabilidade e não consegue navegar até ser reparado de emergência!

### Conduzindo Reparos de Emergência:
1. Tenha na mochila **10 tábuas de madeira** (`boards` ou `logs`) ou um **Ship Repair Kit**.
2. Abra a janela de avarias clicando na roda do leme, no piloto ou usando o **Ship Repair Kit**.
3. Clique em **"Perform Emergency Repairs"**.
4. Cada reparo restaura até 1.500 HP de integridade estrutural. Assim que o HP sai de zero, o navio volta a navegar normalmente!

---

## 🧑‍✈️ O Piloto do Galeão (Galleon Pilot NPC)

Os galeões contam com um sentinela e navegador experiente a bordo (`galleon_pilot`):
- Fica a postos no convés e defende a embarcação de monstros e invasores.
- Dando duplo-clique no piloto, ele oferece acesso rápido a:
  - Assumir a Roda do Leme.
  - Abrir o Manifesto de Tripulação.
  - Inspecionar Avarias e Reparar o Casco.
  - Lançar ou Levantar a Âncora.

---

## 🔭 Radar Marítimo e Runas Navais

### Rastreamento com Luneta (Sea Radar)
1. Leve uma luneta comum (`spyglass`) para o mar aberto.
2. Dê duplo-clique na luneta.
3. O **Maritime Sea Radar** vasculhará os oceanos em um raio de até **200 tiles**.
4. A lista exibirá o nome da embarcação, distância exata e direção (Norte, Noroeste, Sudeste, etc.).
5. Clicando no navio, o sistema fixa o rastreio e informa as coordenadas atuais em tempo real para perseguição ou socorro.

### Sintonizando e Usando Runas Navais
1. Adquira uma **Ship Rune** (`ship_rune`, runa mágica verde-mar).
2. Fique no convés do seu navio e dê duplo-clique na runa em sua mochila.
3. A runa será sintonizada magicamente com o seu navio e passará a se chamar `"Ship Rune: [Nome do Navio]"`.
4. Ao usar a magia **Recall** ou **Gate Travel** nela (ou dar duplo-clique para ativar sua carga mágica), você será teletransportado diretamente ao convés do navio, mesmo que ele esteja em alto mar!

> [!IMPORTANT]
> Por questões de segurança naval, apenas membros com patente de **Oficial** ou **Capitão** podem gravar runas no navio, e o teleporte só pode ser concluído quando a embarcação estiver ancorada ou fora de alta velocidade de combate.

---

Bons ventos e mares calmos, Capitão! 🏴‍☠️
