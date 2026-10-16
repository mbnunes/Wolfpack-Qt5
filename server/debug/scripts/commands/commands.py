#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    |
#  ((    _/{  "-;  | Commands Help & PLEVEL Directory           |
#   )).-' {{ ;'`   |                                            |
#  ( (  ;._ \\ ctr |                                            |
#===============================================================#
r"""
	\command commands
	\description Shows an interactive gump listing all commands available for your PLEVEL / Rank, with detailed descriptions, usage syntax and direct execution.
	\usage - <code>commands</code>
	- <code>commands [search_term]</code>
"""

import wolfpack
try:
	from wolfpack import tr
except ImportError:
	tr = getattr(wolfpack, 'tr', lambda s: s)

import re
from wolfpack.gumps import cGump
from wolfpack.consts import *

# Rank Mapping
RANK_TITLES = {
	100: "Administrator",
	99: "Co-Administrator",
	75: "Game Master",
	50: "Seer",
	25: "Counselor",
	1: "Player"
}

def getRankTitle(rank):
	if rank >= 100:
		return "Administrator"
	elif rank >= 99:
		return "Co-Administrator"
	elif rank >= 75:
		return "Game Master"
	elif rank >= 50:
		return "Seer"
	elif rank >= 25:
		return "Counselor"
	else:
		return "Player"

# Categories
CATEGORIES = [
	"All",
	"General",
	"Travel",
	"Staff",
	"Building",
	"NPC/Spawns",
	"Properties",
	"Admin"
]

# Master Commands Database
# Structure: cmd: (category, min_rank, desc, usage, runnable)
COMMANDS_DB = {
	# General / Player
	"commands": ("General", 1, "Abre este painel com a lista de comandos disponíveis para o seu PLEVEL, com descrições e sintaxe.", "commands [busca]", True),
	"cmd": ("General", 1, "Atalho para o comando 'commands'.", "cmd [busca]", True),
	"time": ("General", 1, "Exibe a hora atual do mundo do Ultima Online, tempo decorrido e nível de luz.", "time", True),
	"where": ("General", 1, "Mostra suas coordenadas atuais (X, Y, Z), região/cidade e o mapa onde você está.", "where", True),
	"fix": ("General", 1, "Reenvia os itens e objetos ao redor para sincronizar e desbugar sua visão caso algo trave.", "fix", True),
	"resend": ("General", 1, "Força o reenvio de todo o mundo visível ao cliente de jogo.", "resend", True),
	"updateplayer": ("General", 1, "Atualiza o estado visual, status e animação do seu personagem.", "updateplayer", True),
	"serverinfo": ("General", 1, "Exibe informações gerais sobre o shard, tempo online e versão do servidor.", "serverinfo", True),
	"action": ("General", 1, "Executa uma ação ou animação visual no seu personagem.", "action <id>", False),
	"password": ("General", 1, "Permite alterar a senha da sua conta de jogo com segurança.", "password <nova_senha>", False),
	"email": ("General", 1, "Altera o endereço de e-mail associado à sua conta.", "email <novo_email>", False),
	"showskilltitles": ("General", 1, "Exibe os títulos de habilidades acima do nome do personagem.", "showskilltitles", True),
	"hideskilltitle": ("General", 1, "Oculta o título de habilidade sobre o nome do personagem.", "hideskilltitle", True),

	# Movement & Travel
	"go": ("Travel", 25, "Abre o menu interativo de viagens categorizado com mais de 550 locais em todos os 6 mapas, ou teleporta direto por nome/coordenadas.", "go [local|x,y,z,map|maps|reload]", True),
	"tele": ("Travel", 25, "Teleporta seu personagem instantaneamente para a posição ou chão que você alvejar.", "tele", True),
	"telem": ("Travel", 50, "Teleporte contínuo: permite clicar sucessivas vezes para navegar pelo mapa até cancelar.", "telem", True),
	"send": ("Travel", 50, "Envia o objeto ou criatura alvo para uma coordenada ou localização especificada.", "send [x,y,z,map|local]", False),
	"move": ("Travel", 50, "Move um item ou criatura alvo para sua posição ou para um deslocamento relativo.", "move [dx,dy,dz]", False),
	"follow": ("Travel", 50, "Faz seu personagem seguir automaticamente o alvo selecionado.", "follow", True),
	"nudgeup": ("Travel", 50, "Eleva o objeto ou piso alvo em N unidades de altura Z (padrão: 1).", "nudgeup [altura]", False),
	"nudgedown": ("Travel", 50, "Abaixa o objeto ou piso alvo em N unidades de altura Z (padrão: 1).", "nudgedown [altura]", False),
	"smove": ("Travel", 50, "Move o objeto alvo suavemente na direção indicada.", "smove <direcao>", False),
	"goname": ("Travel", 50, "Teleporta você diretamente até um personagem ou NPC pelo nome.", "goname <nome>", False),
	"gouid": ("Travel", 50, "Teleporta você até o objeto ou criatura pelo serial numérico hexadecimal.", "gouid <serial>", False),
	"goitem": ("Travel", 50, "Teleporta você até um item específico pelo ID ou tipo.", "goitem <item_id>", False),
	"gobaseid": ("Travel", 50, "Teleporta você até o item que possua a baseid fornecida.", "gobaseid <baseid>", False),

	# Staff & Moderation
	"invis": ("Staff", 50, "Alterna a invisibilidade de Game Master (não é revelado por feitiços normais).", "invis", True),
	"invul": ("Staff", 50, "Ativa ou desativa a invulnerabilidade (God Mode) no seu personagem.", "invul", True),
	"freeze": ("Staff", 50, "Congela o alvo selecionado, impedindo que se mova ou fuja.", "freeze", True),
	"jail": ("Staff", 50, "Envia o jogador alvo imediatamente para as celas da prisão (Jail).", "jail", True),
	"forgive": ("Staff", 50, "Libera um jogador da prisão, retornando-o a uma cidade segura.", "forgive", True),
	"mute": ("Staff", 50, "Silencia o jogador alvo por determinado tempo, impedindo que fale.", "mute [minutos]", True),
	"unmute": ("Staff", 50, "Remove o silêncio de um jogador, restaurando a fala normal.", "unmute", True),
	"kick": ("Staff", 75, "Desconecta imediatamente o jogador selecionado do servidor.", "kick", True),
	"ban": ("Staff", 75, "Bane a conta de um jogador do servidor.", "ban <conta>", False),
	"unban": ("Staff", 75, "Remove o banimento da conta especificada.", "unban <conta>", False),
	"disconnect": ("Staff", 50, "Desconecta forçadamente a conexão de rede do jogador alvo.", "disconnect", True),
	"kill": ("Staff", 50, "Mata instantaneamente a criatura ou personagem alvo.", "kill", True),
	"resurrect": ("Staff", 50, "Ressuscita o jogador fantasma ou criatura alvo imediatamente.", "resurrect", True),
	"heal": ("Staff", 50, "Cura completamente os pontos de vida do alvo.", "heal", True),
	"restore": ("Staff", 50, "Restaura Vida, Mana e Stamina ao máximo do alvo ou de si mesmo.", "restore", True),
	"who": ("Staff", 25, "Abre a lista completa de jogadores online com opções de teleporte e moderação.", "who", True),
	"staff": ("Staff", 25, "Exibe a lista de membros do Staff online e seus cargos.", "staff", True),
	"gmtalk": ("Staff", 50, "Ativa ou desativa o canal de comunicação privado entre a equipe.", "gmtalk", True),
	"broadcast": ("Staff", 50, "Envia uma mensagem de transmissão na tela de todos os jogadores conectados.", "broadcast <mensagem>", False),
	"sysmessage": ("Staff", 50, "Envia uma mensagem do sistema particular para o jogador alvo.", "sysmessage <mensagem>", False),
	"hearall": ("Staff", 50, "Permite ao staff escutar todas as conversas do shard independente da distância.", "hearall", True),
	"pagenotify": ("Staff", 25, "Ativa ou desativa o alerta sonoro e visual quando um jogador envia um Page de ajuda.", "pagenotify", True),

	# Building & Items
	"add": ("Building", 50, "Abre o menu de criação de itens e NPCs ou cria um item diretamente pelo ID.", "add [id_do_item]", True),
	"adddecor": ("Building", 50, "Abre a ferramenta especializada de adicionar decorações e móveis no mundo.", "adddecor", True),
	"remove": ("Building", 50, "Remove permanentemente o item ou criatura alvo do mundo.", "remove", True),
	"dupe": ("Building", 50, "Duplica o item alvo com todas as suas propriedades, cor e conteúdo.", "dupe [quantidade]", True),
	"dye": ("Building", 50, "Abre a paleta de cores para pintar o item ou vestimenta alvo.", "dye", True),
	"turnitem": ("Building", 1, "Gira móveis, portas ou decorações de acordo com suas variações direcionais.", "turnitem", True),
	"tile": ("Building", 50, "Preenche uma área retangular com o item ou piso especificado.", "tile <item_id> [z]", False),
	"tilecolor": ("Building", 50, "Altera a cor/matiz de todos os itens dentro de uma área demarcada.", "tilecolor <hue>", False),
	"tilecolorz": ("Building", 50, "Altera a cor dos itens em uma área que estejam em uma altura Z específica.", "tilecolorz <hue> <z>", False),
	"tilemove": ("Building", 50, "Move todos os itens contidos em uma área retangular por um deslocamento X, Y, Z.", "tilemove <dx,dy,dz>", False),
	"tilepattern": ("Building", 50, "Aplica padrões complexos de azulejos ou ladrilhos em uma área demarcada.", "tilepattern", True),
	"nuke": ("Building", 75, "Remove em massa itens em uma área demarcada via mira ou em todo o mapa.", "nuke", True),
	"nukez": ("Building", 75, "Remove itens em uma área demarcada filtrando por uma altura Z específica.", "nukez <z>", False),
	"wipe": ("Building", 75, "Apaga todos os itens dinâmicos do mapa selecionado ou região.", "wipe", False),
	"removedupes": ("Building", 75, "Varre e remove automaticamente itens sobrepostos duplicados na mesma coordenada.", "removedupes", True),
	"removemulti": ("Building", 75, "Remove construções multi (casas, barcos) alvejadas.", "removemulti", True),
	"makestatic": ("Building", 50, "Converte um item dinâmico para estático permanente.", "makestatic", True),
	"static": ("Building", 50, "Cria um item estático permanente no local alvo.", "static <item_id>", False),
	"decoration": ("Building", 75, "Executa scripts de decoração em massa de cidades e masmorras a partir de XMLs.", "decoration <mapa>", False),
	"doorgen": ("Building", 75, "Gera os componentes de portas automaticamente na posição alvo.", "doorgen", True),
	"build": ("Building", 50, "Ferramenta de auxílio na construção rápida de pisos e paredes.", "build", True),
	"allmove": ("Building", 50, "Permite arrastar e mover livremente itens estáticos e bloqueados pelo cenário.", "allmove", True),
	"allshow": ("Building", 50, "Permite enxergar itens ocultos e invisíveis no mundo.", "allshow", True),

	# NPC & Spawns
	"spawnregion": ("NPC/Spawns", 50, "Ativa, desativa ou preenche regiões de spawn ('spawnregion fill [all|towns|vendors|<nome>]).", "spawnregion <respawn|fill|clear|info> <nome>", True),
	"respawntowns": ("NPC/Spawns", 50, "Dá respawn completo em todas as cidades ou numa cidade específica ('respawntowns [cidade|list|clear]).", "respawntowns [cidade|list|clear]", True),
	"champ": ("NPC/Spawns", 50, "Abre o painel de gerenciamento de Champion Spawns para ativar e monitorar altares.", "champ", True),
	"peerless": ("NPC/Spawns", 50, "Controla os altares e chefes Peerless (Mondain's Legacy).", "peerless", True),
	"paragon": ("NPC/Spawns", 50, "Transforma a criatura alvo em uma criatura Paragon de Ilshenar com buffs de status.", "paragon", True),
	"restock": ("NPC/Spawns", 75, "Força os vendedores e lojas a reabastecerem imediatamente seu estoque de mercadorias.", "restock", True),
	"followers": ("NPC/Spawns", 50, "Mostra a contagem e slots de seguidores/pets do jogador alvo.", "followers", True),
	"showserials": ("NPC/Spawns", 75, "Exibe os seriais hexadecimais flutuando acima da cabeça de criaturas e itens.", "showserials", True),
	"multigems": ("NPC/Spawns", 50, "Exibe gemas de visualização em torno de estruturas multi.", "multigems", True),
	"bod": ("NPC/Spawns", 50, "Gerencia ou cria contratos de encomendas (Bulk Order Deeds).", "bod", True),
	"getbod": ("NPC/Spawns", 50, "Gera e entrega um Bulk Order Deed para o seu personagem.", "getbod", True),

	# Properties & Scripting
	"info": ("Properties", 50, "Abre a janela de inspeção detalhada de propriedades, tags, posições e status do alvo.", "info", True),
	"edit": ("Properties", 50, "Abre o editor interativo de variáveis e flags do item ou criatura alvo.", "edit", True),
	"stat": ("Properties", 50, "Exibe ou altera atributos de força, destreza e inteligência do alvo.", "stat [str|dex|int] [val]", False),
	"skill": ("Properties", 50, "Ajusta ou consulta o valor exato de uma perícia específica do personagem alvo.", "skill <skill_name> <valor>", False),
	"skillinfo": ("Properties", 50, "Abre o gump completo de gerenciamento e ajuste de todas as perícias e caps.", "skillinfo", True),
	"allskills": ("Properties", 50, "Define todas as perícias do personagem alvo para um valor determinado.", "allskills <valor>", False),
	"tags": ("Properties", 50, "Abre a lista de todas as tags associadas ao objeto alvo.", "tags", True),
	"taginfo": ("Properties", 50, "Exibe no console ou chat as tags salvas no objeto alvo.", "taginfo", True),
	"gettag": ("Properties", 50, "Lê e imprime o valor de uma tag específica no objeto alvo.", "gettag <nome_da_tag>", False),
	"settag": ("Properties", 50, "Grava uma nova tag de string ou número no objeto alvo.", "settag <nome_da_tag> <valor>", False),
	"deltag": ("Properties", 50, "Exclui uma tag existente no objeto alvo.", "deltag <nome_da_tag>", False),
	"set": ("Properties", 50, "Altera diretamente uma propriedade C++ no objeto alvo.", "set <propriedade> <valor>", False),
	"addscript": ("Properties", 50, "Vincula um script Python de comportamento ao objeto alvo.", "addscript <nome_script>", False),
	"removescript": ("Properties", 50, "Remove um script Python do objeto alvo.", "removescript <nome_script>", False),
	"hasscript": ("Properties", 50, "Verifica se o objeto alvo possui determinado script vinculado.", "hasscript <nome_script>", False),
	"events": ("Properties", 50, "Exibe a lista de manipuladores de eventos ativos no objeto alvo.", "events", True),
	"find": ("Properties", 50, "Localiza itens, NPCs ou estruturas no mundo por ID ou nome.", "find <nome|id>", False),
	"findlock": ("Properties", 50, "Encontra contêineres e portas trancadas no mapa.", "findlock", True),

	# Administration
	"account": ("Admin", 75, "Gerenciador de contas: permite criar, bloquear, alterar ACL e auditar contas de jogadores.", "account", True),
	"reload": ("Admin", 99, "Recarrega scripts Python, arquivos de definições XML ou banco de contas em tempo real.", "reload [scripts|definitions|accounts]", False),
	"save": ("Admin", 75, "Força o salvamento imediato do estado do mundo (Worldsave) no disco.", "save", True),
	"shutdown": ("Admin", 99, "Inicia a contagem regressiva para fechar o servidor com segurança.", "shutdown [segundos]", False),
	"export": ("Admin", 99, "Exporta itens e construções do mundo para arquivos XML ou WSC.", "export", True),
	"import": ("Admin", 99, "Importa arquivos WSC ou XML de cenários e itens para o mapa atual.", "import <arquivo>", False),
	"exportstatics": ("Admin", 99, "Exporta itens estáticos personalizados para arquivo.", "exportstatics", True),
	"exportdefinitions": ("Admin", 100, "Exporta definições de itens e monstros do servidor.", "exportdefinitions", True),
	"exportmissingids": ("Admin", 99, "Gera relatório de IDs e gráficos ausentes no mundo.", "exportmissingids", True),
	"exportplayer": ("Admin", 100, "Exporta os dados de um personagem jogador para arquivo.", "exportplayer", True),
	"importplayer": ("Admin", 100, "Importa os dados de um personagem jogador de arquivo.", "importplayer <arquivo>", False),
	"eval": ("Admin", 100, "Executa uma expressão ou comando de código Python diretamente no interpretador do servidor.", "eval <expressao>", False),
	"season": ("Admin", 50, "Muda a estação climática visual no mapa atual (Primavera, Verão, Outono, Inverno, Desolação).", "season <0-4>", False),
	"sound": ("Admin", 50, "Reproduz um efeito sonoro do Ultima Online pelo número de ID.", "sound <id_som>", False),
	"effect": ("Admin", 50, "Reproduz um efeito gráfico ou magia na coordenada alvo.", "effect <id_efeito>", False),
	"targeteffect": ("Admin", 50, "Aplica um efeito visual direcionado ao alvo selecionado.", "targeteffect <id>", False),
	"bolt": ("Admin", 50, "Dispara um raio mágico sobre o alvo ou coordenada selecionada.", "bolt", True),
	"boom": ("Admin", 50, "Cria uma explosão sonora e visual sobre o alvo.", "boom", True),
	"cast": ("Admin", 50, "Conjura qualquer magia do jogo pelo ID ou nome sem custo de reagentes ou mana.", "cast <id_magia>", False),
	"newlos": ("Admin", 50, "Testa os novos algoritmos de linha de visão (Line of Sight).", "newlos", True),
	"testlos": ("Admin", 50, "Verifica se há linha de visão desobstruída entre você e o tile alvo.", "testlos", True),
	"testapi": ("Admin", 100, "Executa a suíte de testes de integridade da API Python do Wolfpack.", "testapi", True),
}

PAGE_SIZE = 8

def cmdCommands(socket, command, arguments):
	player = socket.player
	search = arguments.strip() if arguments else ""
	category = "All"
	page = 0
	selected_cmd = None

	# If search provided, look if it's a category
	if search:
		for cat in CATEGORIES:
			if cat.lower() == search.lower():
				category = cat
				search = ""
				break

	showCommandsGump(player, category, page, search, selected_cmd)
	return

def getAuthorizedCommands(player, category="All", search=""):
	account = player.account
	results = []

	search_lower = search.lower().strip()

	for cmd_name, info in sorted(COMMANDS_DB.items()):
		# Check permission
		if not account.authorized("command", cmd_name.upper()):
			continue

		cat, min_rank, desc, usage, runnable = info

		# Category filter
		if category != "All" and cat.lower() != category.lower():
			continue

		# Search filter
		if search_lower:
			text = (cmd_name + " " + desc + " " + usage + " " + cat).lower()
			if search_lower not in text:
				continue

			results.append((cmd_name, info))
		else:
			results.append((cmd_name, info))

	return results

def showCommandsGump(player, category, page, search, selected_cmd):
	auth_cmds = getAuthorizedCommands(player, category, search)
	total_cmds = len(auth_cmds)
	max_pages = max(1, (total_cmds + PAGE_SIZE - 1) // PAGE_SIZE)

	if page >= max_pages:
		page = max_pages - 1
	if page < 0:
		page = 0

	# Default selected command
	if not selected_cmd and total_cmds > 0:
		selected_cmd = auth_cmds[0][0]

	# Build Gump
	gump = cGump(0, 0, 0, 40, 40)
	gump.addBackground(0x13be, 600, 450)
	gump.addTiledGump(10, 10, 580, 24, 0xA40)
	gump.addTiledGump(10, 38, 260, 362, 0xA40)
	gump.addTiledGump(275, 38, 315, 362, 0xA40)
	gump.addCheckerTrans(10, 10, 580, 430)

	# Main Title
	rank = player.account.rank
	rank_title = getRankTitle(rank)
	header_title = "<center><basefont color='#FFD700'><b>Wolfpack Commands Directory</b></basefont></center>"
	gump.addHtmlGump(10, 12, 580, 20, header_title)

	# User Info Line
	info_line = "<basefont color='#FFFFFF'>Conta: <b>%s</b> | Nível: <b>%s (PLEVEL %d)</b> | Comandos: <b>%d</b></basefont>" % (
		player.account.name, rank_title, rank, total_cmds
	)
	gump.addHtmlGump(15, 42, 570, 20, info_line)

	# Category Navigation Buttons (Rows at top of left panel)
	# Button IDs: 100 + category_index
	cat_x = 15
	cat_y = 66
	for idx, cat_name in enumerate(CATEGORIES):
		btn_normal = 4005
		btn_pressed = 4007
		color = 0x834 if cat_name == category else 0x384
		gump.addButton(cat_x, cat_y, btn_normal, btn_pressed, 100 + idx)
		gump.addText(cat_x + 32, cat_y + 2, cat_name[:10], color)
		cat_x += 70
		if (idx + 1) % 4 == 0:
			cat_x = 15
			cat_y += 24

	# Left Panel: Command List
	list_y = 120
	start_idx = page * PAGE_SIZE
	end_idx = min(start_idx + PAGE_SIZE, total_cmds)

	gump.addHtmlGump(15, 118, 250, 20, "<center><basefont color='#FFA500'><b>COMANDOS DISPONÍVEIS</b></basefont></center>")
	list_y += 24

	for i in range(start_idx, end_idx):
		cmd_name, info = auth_cmds[i]
		is_selected = (cmd_name == selected_cmd)
		btn_normal = 4006 if is_selected else 4005
		btn_pressed = 4007
		text_color = 0x53 if is_selected else 0x834

		# Button ID: 1000 + i (selects command)
		gump.addButton(20, list_y, btn_normal, btn_pressed, 1000 + i)
		display_name = "'" + cmd_name
		gump.addText(55, list_y + 2, display_name, text_color)
		list_y += 26

	# Left Panel: Pagination Controls
	page_y = 368
	if page > 0:
		gump.addButton(20, page_y, 0xFAE, 0xFB0, 2) # Previous Page
		gump.addText(55, page_y + 3, tr("Anterior"), 0x834)

	gump.addText(125, page_y + 3, "%d / %d" % (page + 1, max_pages), 0x384)

	if page + 1 < max_pages:
		gump.addButton(180, page_y, 4005, 4007, 1) # Next Page
		gump.addText(215, page_y + 3, tr("Próximo"), 0x834)

	# Right Panel: Selected Command Details
	cmd_info = COMMANDS_DB.get(selected_cmd)
	if cmd_info:
		cat, min_rank, desc, usage, runnable = cmd_info

		detail_html = [
			"<basefont color='#FFD700' size='4'><b>Comando: '%s</b></basefont><br>" % selected_cmd,
			"<basefont color='#ADD8E6'><b>Categoria:</b></basefont> <basefont color='#FFFFFF'>%s</basefont><br>" % cat,
			"<basefont color='#ADD8E6'><b>Nível Requerido:</b></basefont> <basefont color='#FFFFFF'>%s (Rank %d)</basefont><br><br>" % (getRankTitle(min_rank), min_rank),
			"<basefont color='#ADD8E6'><b>Descrição:</b></basefont><br>",
			"<basefont color='#FFFFFF'>%s</basefont><br><br>" % desc,
			"<basefont color='#ADD8E6'><b>Sintaxe / Uso:</b></basefont><br>",
			"<basefont color='#90EE90'><code>'%s</code></basefont>" % usage.replace("\n", "<br>")
		]
		gump.addHtmlGump(285, 75, 295, 260, "".join(detail_html), True, True)

		# Action Buttons (Run Command if executable)
		if runnable:
			# Button 500: Execute Command
			gump.addButton(285, 345, 0xFA5, 0xFA7, 500)
			gump.addText(320, 347, "Executar '%s' Agora" % selected_cmd, 0x53)
		else:
			gump.addText(285, 347, "Requer argumentos no chat (veja sintaxe)", 0x384)
	else:
		gump.addHtmlGump(285, 120, 295, 200, "<center><basefont color='#FFFFFF'>Selecione um comando à esquerda para ver os detalhes e sintaxe.</basefont></center>")

	# Bottom Bar: Close Button
	gump.addButton(20, 410, 0xFB1, 0xFB3, 0)
	gump.addText(55, 412, tr("Fechar"), 0x834)

	# Direct Search Notice
	gump.addText(285, 412, "Dica: Digite 'commands [termo] para buscar", 0x384)

	# Store state arguments
	gump.setArgs([category, page, search, selected_cmd or ""])
	gump.setCallback(callbackCommands)
	gump.send(player.socket)
	return

def executeCommandDirect(player, cmd_name):
	socket = player.socket
	if not socket:
		return

	# Handle runnable commands
	if cmd_name == "go":
		import commands.go
		commands.go.go(socket, "go", "")
	elif cmd_name == "who":
		import commands.who
		commands.who.cmdWho(socket, "who", "")
	elif cmd_name == "where":
		import commands.where
		commands.where.where(socket, "where", "")
	elif cmd_name == "invis":
		import commands.invis
		commands.invis.invis(socket, "invis", "")
	elif cmd_name == "invul":
		import commands.invul
		commands.invul.invul(socket, "invul", "")
	elif cmd_name == "tele":
		import commands.tele
		commands.tele.tele(socket, "tele", "")
	elif cmd_name == "fix" or cmd_name == "resend":
		import commands.fix
		commands.fix.commandFix(socket, cmd_name, "")
	elif cmd_name == "time":
		import commands
		commands.uotime(socket, "time", "")
	elif cmd_name == "serverinfo":
		import commands.serverinfo
		commands.serverinfo.serverinfo(socket, "serverinfo", "")
	elif cmd_name == "updateplayer":
		socket.updateplayer()
		socket.sysmessage("Player atualizado.")
	elif cmd_name == "resurrect":
		import commands.resurrect
		commands.resurrect.resurrect(socket, "resurrect", "")
	elif cmd_name == "heal":
		import commands.restore
		commands.restore.heal(socket, "heal", "")
	elif cmd_name == "restore":
		import commands.restore
		commands.restore.restore(socket, "restore", "")
	elif cmd_name == "kill":
		import commands.kill
		commands.kill.kill(socket, "kill", "")
	elif cmd_name == "info":
		import commands.info
		commands.info.info(socket, "info", "")
	elif cmd_name == "add":
		import commands.add
		commands.add.add(socket, "add", "")
	elif cmd_name == "turnitem":
		import commands.turnitem
		commands.turnitem.turnitem(socket, "turnitem", "")
	elif cmd_name == "dupe":
		import commands.dupe
		commands.dupe.dupe(socket, "dupe", "")
	elif cmd_name == "dye":
		import commands.dye
		commands.dye.dye(socket, "dye", "")
	elif cmd_name == "allshow":
		import commands.allshow
		commands.allshow.allshow(socket, "allshow", "")
	elif cmd_name == "freeze":
		import commands.freeze
		commands.freeze.freeze(socket, "freeze", "")
	elif cmd_name == "jail":
		import commands.jail
		commands.jail.jail(socket, "jail", "")
	elif cmd_name == "mute":
		import commands.mute
		commands.mute.mute(socket, "mute", "")
	elif cmd_name == "unmute":
		import commands.mute
		commands.mute.unmute(socket, "unmute", "")
	elif cmd_name == "tags":
		import commands.tags
		commands.tags.tags(socket, "tags", "")
	elif cmd_name == "champ":
		import commands.champ
		commands.champ.commandChamp(socket, "champ", "")
	elif cmd_name == "skillinfo":
		import commands.skillinfo
		commands.skillinfo.skillinfo(socket, "skillinfo", "")
	elif cmd_name == "bank" or cmd_name == "bankself":
		import commands.bank
		commands.bank.bankself(socket, "bankself", "")
	elif cmd_name == "account":
		import commands.account
		commands.account.commandAccount(socket, "account", "")
	elif cmd_name == "save":
		wolfpack.save()
		socket.sysmessage("Worldsave executado com sucesso.")
	elif cmd_name == "commands" or cmd_name == "cmd":
		cmdCommands(socket, "commands", "")
	else:
		socket.sysmessage("Comando '%s' ativado. Verifique o cursor ou sintaxe no chat." % cmd_name)
	return

def callbackCommands(char, args, choice):
	button = choice.button
	category, page, search, selected_cmd = args

	# Close
	if button == 0:
		return

	# Next Page
	if button == 1:
		showCommandsGump(char, category, page + 1, search, selected_cmd)
		return

	# Previous Page
	if button == 2:
		showCommandsGump(char, category, max(0, page - 1), search, selected_cmd)
		return

	# Execute selected command
	if button == 500:
		executeCommandDirect(char, selected_cmd)
		# Reopen gump to allow more actions
		showCommandsGump(char, category, page, search, selected_cmd)
		return

	# Category Selection (100 .. 100 + len(CATEGORIES))
	if 100 <= button < 100 + len(CATEGORIES):
		cat_idx = button - 100
		new_category = CATEGORIES[cat_idx]
		# Reset page to 0 and search to empty
		showCommandsGump(char, new_category, 0, "", None)
		return

	# Command Selection (1000 + index)
	if button >= 1000:
		auth_cmds = getAuthorizedCommands(char, category, search)
		cmd_idx = button - 1000
		if cmd_idx < len(auth_cmds):
			new_selected = auth_cmds[cmd_idx][0]
			showCommandsGump(char, category, page, search, new_selected)
		return

	return

def onLoad():
	wolfpack.registercommand("commands", cmdCommands)
	wolfpack.registercommand("cmd", cmdCommands)
	return

def onUnload():
	return
