import web.sessions
import web.template
import cgi
import sys
import wolfpack
import wolfpack.sockets
import wolfpack.accounts
import wolfpack.time
import urllib.parse
try:
	import housing
	import housing.house
except Exception:
	housing = None

form = cgi.FieldStorage()
session_id = form.getvalue('session', '')
view = form.getvalue('view', '')

web.sessions.check_timeouts()
session = web.sessions.get_session(session_id)

if session is None:
	web.sessions.display_timeout()
	sys.exit()

username = session['username']
user_acc = wolfpack.accounts.find(username)

is_admin = False
if user_acc:
	try:
		is_admin = bool(user_acc.authorized('RemoteAccess', 'login'))
	except Exception:
		is_admin = False

MAP_NAMES = {
	0: "Felucca",
	1: "Trammel",
	2: "Ilshenar",
	3: "Malas",
	4: "Tokuno",
	5: "Ter Mur"
}

# ==============================================================================
# PLAYER VIEW (Default for regular players, or toggled by staff with view=player)
# ==============================================================================
if not is_admin or view == 'player':
	chars = getattr(user_acc, 'characters', []) if user_acc else []
	chars_count = len(chars)

	total_bank_gold = 0
	total_char_gold = 0
	char_cards = []
	all_player_houses = []

	# Mapa global de casas
	houses_map = {}
	if housing and hasattr(housing, 'getHouses'):
		try:
			houses_map = housing.getHouses()
		except Exception:
			houses_map = {}

	for char in chars:
		c_name = getattr(char, 'name', 'Sem Nome')
		c_title = getattr(char, 'title', '')
		c_serial = getattr(char, 'serial', 0)
		is_online = bool(getattr(char, 'socket', None))

		# Stats
		s_str = getattr(char, 'strength', 0)
		s_dex = getattr(char, 'dexterity', 0)
		s_int = getattr(char, 'intelligence', 0)

		hp_cur = getattr(char, 'hitpoints', s_str)
		hp_max = max(1, getattr(char, 'maxhitpoints', s_str))
		hp_pct = max(0, min(100, int((hp_cur / hp_max) * 100)))

		stam_cur = getattr(char, 'stamina', s_dex)
		stam_max = max(1, getattr(char, 'maxstamina', s_dex))
		stam_pct = max(0, min(100, int((stam_cur / stam_max) * 100)))

		mana_cur = getattr(char, 'mana', s_int)
		mana_max = max(1, getattr(char, 'maxmana', s_int))
		mana_pct = max(0, min(100, int((mana_cur / mana_max) * 100)))

		# Ouro
		b_gold = 0
		try:
			bank = char.getbankbox()
			if bank:
				b_gold = bank.countresource(0xEED)
		except Exception:
			b_gold = 0
		total_bank_gold += b_gold

		c_gold = 0
		try:
			c_gold = char.countresource(0xEED)
		except Exception:
			c_gold = 0
		total_char_gold += c_gold

		# Skills
		skill_total = 0.0
		for i in range(55):
			try:
				skill_total += (char.skill[i] / 10.0)
			except Exception:
				pass

		# Casas do Personagem
		c_house = None
		if housing:
			try:
				if hasattr(housing, 'findHouse'):
					c_house = housing.findHouse(char)
				if not c_house and c_serial in houses_map:
					c_house = wolfpack.finditem(houses_map[c_serial])
			except Exception:
				c_house = None

		if c_house:
			all_player_houses.append((char, c_house))

		# Gênero
		gender_icon = '&#9792; Mulher' if getattr(char, 'id', 0) in [0x191, 0x193] else '&#9794; Homem'
		status_badge = '<span class="badge bg-success-subtle text-success border border-success">Online</span>' if is_online else '<span class="badge bg-secondary-subtle text-secondary border">Offline</span>'

		char_cards.append(f"""
		<div class="col-12 col-md-6 col-xl-4">
			<div class="card h-100 border border-light-subtle shadow-sm">
				<div class="card-header bg-white py-3 d-flex justify-content-between align-items-center">
					<div>
						<h6 class="fw-bold text-dark mb-0">{cgi.escape(str(c_name))}</h6>
						<small class="text-secondary">{cgi.escape(str(c_title)) if c_title else 'Aventureiro'}</small>
					</div>
					<div class="text-end">
						{status_badge}
						<div class="font-monospace small text-muted mt-1">0x{c_serial:x}</div>
					</div>
				</div>
				<div class="card-body">
					<!-- Vitals Bars -->
					<div class="mb-2">
						<div class="d-flex justify-content-between small text-secondary mb-1">
							<span>HP:</span>
							<span class="font-monospace fw-semibold text-dark">{hp_cur} / {hp_max}</span>
						</div>
						<div class="progress" style="height: 6px;">
							<div class="progress-bar bg-danger" style="width: {hp_pct}%;"></div>
						</div>
					</div>

					<div class="mb-2">
						<div class="d-flex justify-content-between small text-secondary mb-1">
							<span>Stamina:</span>
							<span class="font-monospace fw-semibold text-dark">{stam_cur} / {stam_max}</span>
						</div>
						<div class="progress" style="height: 6px;">
							<div class="progress-bar bg-warning" style="width: {stam_pct}%;"></div>
						</div>
					</div>

					<div class="mb-3">
						<div class="d-flex justify-content-between small text-secondary mb-1">
							<span>Mana:</span>
							<span class="font-monospace fw-semibold text-dark">{mana_cur} / {mana_max}</span>
						</div>
						<div class="progress" style="height: 6px;">
							<div class="progress-bar bg-primary" style="width: {mana_pct}%;"></div>
						</div>
					</div>

					<!-- Atributos -->
					<div class="row text-center py-2 bg-light rounded g-1 mb-3">
						<div class="col-4">
							<span class="text-muted d-block" style="font-size: 0.7rem;">STR</span>
							<strong class="text-dark small">{s_str}</strong>
						</div>
						<div class="col-4">
							<span class="text-muted d-block" style="font-size: 0.7rem;">DEX</span>
							<strong class="text-dark small">{s_dex}</strong>
						</div>
						<div class="col-4">
							<span class="text-muted d-block" style="font-size: 0.7rem;">INT</span>
							<strong class="text-dark small">{s_int}</strong>
						</div>
					</div>

					<!-- Informações Bancárias & Skills -->
					<div class="d-flex justify-content-between align-items-center small py-1 border-bottom">
						<span class="text-secondary">&#129689; Ouro no Banco:</span>
						<span class="fw-bold text-success font-monospace">{b_gold:,} gps</span>
					</div>
					<div class="d-flex justify-content-between align-items-center small py-1 border-bottom">
						<span class="text-secondary">&#128188; Ouro na Mochila:</span>
						<span class="fw-bold text-dark font-monospace">{c_gold:,} gps</span>
					</div>
					<div class="d-flex justify-content-between align-items-center small py-1 mb-3">
						<span class="text-secondary">&#128218; Total de Perícias:</span>
						<span class="badge bg-secondary-subtle text-dark font-monospace">{skill_total:.1f}%</span>
					</div>

					<a href="character.py?session={session_id}&serial=0x{c_serial:x}&account={urllib.parse.quote(username)}" class="btn btn-outline-success btn-sm w-100 fw-semibold">
						Ver Ficha Completa, Skills & Banco &rarr;
					</a>
				</div>
			</div>
		</div>
		""")

	# Tabela de Propriedades do Jogador
	houses_html = ""
	if all_player_houses:
		house_rows = []
		for pchar, house in all_player_houses:
			h_name = getattr(house, 'name', 'Casa Sem Nome')
			if hasattr(house, 'hastag') and house.hastag('sign'):
				sign_item = wolfpack.finditem(int(house.gettag('sign')))
				if sign_item and getattr(sign_item, 'name', ''):
					h_name = sign_item.name

			h_pos = getattr(house, 'pos', None)
			map_id = h_pos.map if h_pos else 0
			map_name = MAP_NAMES.get(map_id, f"Map {map_id}")
			coords_str = f"{h_pos.x}, {h_pos.y}, {h_pos.z}" if h_pos else "N/A"

			is_pub = False
			coowner_cnt = 0
			friend_cnt = 0
			ban_cnt = 0
			if housing and hasattr(housing, 'house'):
				try:
					is_pub = housing.house.isPublic(house)
					coowner_cnt = len(housing.house.getCoOwners(house))
					friend_cnt = len(housing.house.getFriends(house))
					ban_cnt = len(housing.house.getBans(house))
				except Exception:
					pass

			sec_badge = '<span class="badge bg-success-subtle text-success border border-success">Pública</span>' if is_pub else '<span class="badge bg-secondary-subtle text-secondary border">Privada</span>'

			house_rows.append(f"""
			<tr>
				<td class="ps-3 fw-bold text-dark">{cgi.escape(str(h_name))}</td>
				<td><span class="text-secondary">{cgi.escape(str(pchar.name))}</span></td>
				<td><span class="badge bg-light text-dark border">{map_name}</span> <span class="font-monospace small text-muted">({coords_str})</span></td>
				<td>{sec_badge}</td>
				<td class="text-secondary small font-monospace">{coowner_cnt} co-prop. &bull; {friend_cnt} amigos</td>
				<td class="text-end pe-3">
					<a href="properties.py?session={session_id}" class="btn btn-sm btn-outline-secondary py-0">Detalhes &rarr;</a>
				</td>
			</tr>
			""")

		houses_html = f"""
		<div class="card border border-light-subtle shadow-sm mb-4">
			<div class="card-header bg-white py-3 d-flex justify-content-between align-items-center">
				<h6 class="fw-bold text-dark mb-0">&#127960; Minhas Casas & Propriedades ({len(all_player_houses)})</h6>
				<a href="properties.py?session={session_id}" class="btn btn-outline-success btn-sm py-1">Gerenciar Propriedades &rarr;</a>
			</div>
			<div class="table-responsive">
				<table class="table table-hover align-middle mb-0">
					<thead class="table-light small text-uppercase text-secondary">
						<tr>
							<th class="ps-3">Nome da Residência</th>
							<th>Proprietário</th>
							<th>Localização</th>
							<th>Acesso</th>
							<th>Associados</th>
							<th class="text-end pe-3">Ações</th>
						</tr>
					</thead>
					<tbody>
						{''.join(house_rows)}
					</tbody>
				</table>
			</div>
		</div>
		"""
	else:
		houses_html = f"""
		<div class="card border border-light-subtle shadow-sm mb-4">
			<div class="card-body text-center py-4">
				<div class="mb-2 text-secondary" style="font-size: 2rem;">&#127960;</div>
				<h6 class="fw-bold text-dark">Nenhuma Propriedade Registrada</h6>
				<p class="text-muted small mb-3">Você ainda não possui casas ou terrenos vinculados aos personagens desta conta.</p>
				<a href="properties.py?session={session_id}" class="btn btn-sm btn-outline-success">Saber mais sobre Housing</a>
			</div>
		</div>
		"""

	# Botão de retorno ao modo admin se o usuário for staff
	admin_switch_btn = ""
	if is_admin:
		admin_switch_btn = f"""
		<a href="main.py?session={session_id}" class="btn btn-warning btn-sm fw-semibold shadow-sm">
			&larr; Voltar ao Painel Admin
		</a>
		"""

	content = f"""
	<!-- Cabeçalho do Dashboard do Jogador -->
	<div class="d-flex flex-wrap justify-content-between align-items-center pb-2 mb-4 border-bottom gap-2">
		<div>
			<h4 class="fw-bold text-dark mb-0">Painel do Jogador</h4>
			<p class="text-secondary small mb-0">Visão geral da sua conta, personagens, cofre e propriedades.</p>
		</div>
		<div class="d-flex align-items-center gap-2">
			{admin_switch_btn}
			<a href="account.py?session={session_id}&username={urllib.parse.quote(username)}" class="btn btn-outline-secondary btn-sm">
				Configurações da Conta
			</a>
		</div>
	</div>

	<!-- Cards de Métricas do Jogador -->
	<div class="row g-3 mb-4">
		<!-- Personagens -->
		<div class="col-12 col-sm-6 col-xl-3">
			<div class="card h-100 border border-light-subtle shadow-sm">
				<div class="card-body">
					<span class="text-secondary small fw-semibold text-uppercase">Personagens</span>
					<div class="d-flex align-items-baseline gap-2 mt-2">
						<h2 class="fw-bold text-dark mb-0">{chars_count}</h2>
						<span class="text-secondary small">/ 5 slots</span>
					</div>
					<small class="d-block mt-3"><a href="player_chars.py?session={session_id}" class="text-decoration-none text-success">Ver todos os personagens &rarr;</a></small>
				</div>
			</div>
		</div>

		<!-- Ouro Total no Cofre -->
		<div class="col-12 col-sm-6 col-xl-3">
			<div class="card h-100 border border-light-subtle shadow-sm">
				<div class="card-body">
					<span class="text-secondary small fw-semibold text-uppercase">Ouro nos Bancos</span>
					<div class="d-flex align-items-baseline gap-2 mt-2">
						<h2 class="fw-bold text-success mb-0">{total_bank_gold:,}</h2>
						<span class="text-secondary small">gps</span>
					</div>
					<small class="text-muted d-block mt-3">Soma de todos os cofres bancários</small>
				</div>
			</div>
		</div>

		<!-- Propriedades -->
		<div class="col-12 col-sm-6 col-xl-3">
			<div class="card h-100 border border-light-subtle shadow-sm">
				<div class="card-body">
					<span class="text-secondary small fw-semibold text-uppercase">Casas / Terrenos</span>
					<div class="d-flex align-items-baseline gap-2 mt-2">
						<h2 class="fw-bold text-dark mb-0">{len(all_player_houses)}</h2>
						<span class="text-secondary small">propriedades</span>
					</div>
					<small class="d-block mt-3"><a href="properties.py?session={session_id}" class="text-decoration-none text-success">Ver imóveis &rarr;</a></small>
				</div>
			</div>
		</div>

		<!-- Status da Conta -->
		<div class="col-12 col-sm-6 col-xl-3">
			<div class="card h-100 border border-light-subtle shadow-sm">
				<div class="card-body">
					<span class="text-secondary small fw-semibold text-uppercase">Status da Conta</span>
					<div class="d-flex align-items-center gap-2 mt-2">
						<span class="badge bg-success-subtle text-success border border-success px-2 py-1">Ativa</span>
						<span class="badge bg-light text-secondary border font-monospace">{getattr(user_acc, 'acl', 'player')}</span>
					</div>
					<small class="text-muted d-block mt-3">Último Login: {getattr(user_acc, 'lastlogin', 'Desconhecido')}</small>
				</div>
			</div>
		</div>
	</div>

	<!-- Seção: Meus Personagens -->
	<div class="d-flex justify-content-between align-items-center mb-3">
		<h5 class="fw-bold text-dark mb-0">&#128101; Meus Personagens ({chars_count})</h5>
		<a href="player_chars.py?session={session_id}" class="btn btn-sm btn-outline-secondary">Ver em Lista &rarr;</a>
	</div>

	<div class="row g-3 mb-4">
		{''.join(char_cards) if char_cards else '<div class="col-12"><div class="alert alert-secondary text-center py-4">Nenhum personagem criado nesta conta até o momento. Conecte-se ao jogo para criar seu herói.</div></div>'}
	</div>

	<!-- Seção: Propriedades -->
	{houses_html}
	"""

	web.template.output(
		f'&gt; <a href="main.py?session={session_id}&view=player" class="text-decoration-none text-success fw-semibold">Painel do Jogador</a>',
		username,
		1,
		content,
		session_id
	)
	sys.exit()

# ==============================================================================
# ADMINISTRATOR VIEW (For staff members)
# ==============================================================================
total_accounts = wolfpack.accounts.count()
npc_count = wolfpack.npccount()
item_count = wolfpack.itemcount()

# Tempo in-game
game_day = wolfpack.time.days()
game_hour = wolfpack.time.hour()
game_min = "%02u" % wolfpack.time.minute()

# Jogadores Conectados
connected_players = []
socket = wolfpack.sockets.first()
nextsocket = wolfpack.sockets.next()

NOTO_MAP = {
	1: ('#0d6efd', 'Innocent'),     # Azul
	2: ('#198754', 'Guild/Ally'),    # Verde
	3: ('#6c757d', 'Neutral'),       # Cinza
	4: ('#6c757d', 'Criminal'),      # Cinza
	5: ('#fd7e14', 'Enemy'),         # Laranja
	6: ('#dc3545', 'Murderer'),      # Vermelho
}

while socket:
	if socket.player and not socket.player.invisible:
		p_name = str(socket.player.name)
		region_name = str(socket.player.region.name) if (socket.player.region and socket.player.region.name) else 'Unknown'
		noto_id = socket.player.notoriety()
		noto_color, noto_label = NOTO_MAP.get(noto_id, ('#495057', 'Unknown'))
		gender_icon = '&#9792;' if socket.player.id in [0x191, 0x193] else '&#9794;'

		connected_players.append({
			'name': p_name,
			'region': region_name,
			'color': noto_color,
			'label': noto_label,
			'gender': gender_icon,
			'x': socket.player.pos.x,
			'y': socket.player.pos.y,
			'map': socket.player.pos.map
		})
	socket = nextsocket
	nextsocket = wolfpack.sockets.next()

players_online = len(connected_players)

if connected_players:
	player_rows = []
	for p in connected_players:
		player_rows.append(f"""
		<tr>
			<td class="ps-3">
				<span class="me-1 text-secondary">{p['gender']}</span>
				<strong style="color: {p['color']};">{p['name']}</strong>
			</td>
			<td>
				<span class="badge border" style="color: {p['color']}; border-color: {p['color']} !important;">
					{p['label']}
				</span>
			</td>
			<td class="text-secondary">{p['region']}</td>
			<td class="font-monospace small text-secondary">Map {p['map']} ({p['x']}, {p['y']})</td>
		</tr>
		""")
	player_table_html = "".join(player_rows)
else:
	player_table_html = '<tr><td colspan="4" class="text-center py-4 text-muted">Nenhum jogador conectado no momento.</td></tr>'

content = f"""
<!-- Cabeçalho do Dashboard Admin -->
<div class="d-flex flex-wrap justify-content-between align-items-center pb-2 mb-4 border-bottom gap-2">
	<div>
		<h4 class="fw-bold text-dark mb-0">Painel Operacional (Admin)</h4>
		<p class="text-secondary small mb-0">Status e telemetria da instância em tempo real.</p>
	</div>
	<div class="d-flex align-items-center gap-2">
		<a href="main.py?session={session_id}&view=player" class="btn btn-outline-success btn-sm">
			&#128100; Ver como Jogador
		</a>
		<span class="badge bg-success-subtle text-success border border-success px-3 py-2 rounded-pill">
			<span class="spinner-grow spinner-grow-sm me-1" role="status" aria-hidden="true" style="width: 0.5rem; height: 0.5rem;"></span>
			Servidor Ativo
		</span>
		<button class="btn btn-outline-secondary btn-sm" onclick="window.location.reload();">
			Atualizar
		</button>
	</div>
</div>

<!-- Métricas Principais -->
<div class="row g-3 mb-4">
	<!-- Jogadores Online -->
	<div class="col-12 col-sm-6 col-xl-3">
		<div class="card h-100 border border-light-subtle shadow-sm">
			<div class="card-body">
				<span class="text-secondary small fw-semibold text-uppercase">Jogadores Online</span>
				<div class="d-flex align-items-baseline gap-2 mt-2">
					<h2 class="fw-bold text-dark mb-0">{players_online}</h2>
					<span class="text-secondary small">ativos</span>
				</div>
				<div class="progress mt-3" style="height: 5px;">
					<div class="progress-bar bg-success" role="progressbar" style="width: {min(100, int((players_online/max(1, total_accounts))*100))}%;"></div>
				</div>
			</div>
		</div>
	</div>

	<!-- Total de Contas -->
	<div class="col-12 col-sm-6 col-xl-3">
		<div class="card h-100 border border-light-subtle shadow-sm">
			<div class="card-body">
				<span class="text-secondary small fw-semibold text-uppercase">Contas Cadastradas</span>
				<div class="d-flex align-items-baseline gap-2 mt-2">
					<h2 class="fw-bold text-dark mb-0">{total_accounts}</h2>
					<span class="text-secondary small">contas</span>
				</div>
				<small class="d-block mt-3"><a href="accounts.py?session={session_id}" class="text-decoration-none text-success">Gerenciar contas &rarr;</a></small>
			</div>
		</div>
	</div>

	<!-- Itens Instanciados no Mundo -->
	<div class="col-12 col-sm-6 col-xl-3">
		<div class="card h-100 border border-light-subtle shadow-sm">
			<div class="card-body">
				<span class="text-secondary small fw-semibold text-uppercase">Itens no Mundo</span>
				<div class="d-flex align-items-baseline gap-2 mt-2">
					<h2 class="fw-bold text-dark mb-0">{item_count:,}</h2>
					<span class="text-secondary small">objetos</span>
				</div>
				<small class="text-muted d-block mt-3">Total em memória / banco</small>
			</div>
		</div>
	</div>

	<!-- NPCs Vivos -->
	<div class="col-12 col-sm-6 col-xl-3">
		<div class="card h-100 border border-light-subtle shadow-sm">
			<div class="card-body">
				<span class="text-secondary small fw-semibold text-uppercase">NPCs Vivos</span>
				<div class="d-flex align-items-baseline gap-2 mt-2">
					<h2 class="fw-bold text-dark mb-0">{npc_count:,}</h2>
					<span class="text-secondary small">criaturas/vendedores</span>
				</div>
				<small class="text-muted d-block mt-3">Simulação ativa</small>
			</div>
		</div>
	</div>
</div>

<!-- Linha 2: Tempo In-game e Atalhos -->
<div class="row g-3 mb-4">
	<div class="col-12 col-lg-6">
		<div class="card h-100 border border-light-subtle shadow-sm">
			<div class="card-header bg-white py-3 border-bottom border-light-subtle">
				<h6 class="fw-bold text-dark mb-0">Tempo do Mundo (In-game)</h6>
			</div>
			<div class="card-body d-flex flex-column justify-content-around">
				<div class="d-flex justify-content-between align-items-center py-2 border-bottom">
					<span class="text-secondary">Dia do Calendário:</span>
					<span class="fw-bold text-dark">Dia {game_day}</span>
				</div>
				<div class="d-flex justify-content-between align-items-center py-2">
					<span class="text-secondary">Horário de Britannia:</span>
					<span class="badge bg-dark-subtle text-dark border font-monospace fs-6 px-3">{game_hour}:{game_min}</span>
				</div>
			</div>
		</div>
	</div>

	<div class="col-12 col-lg-6">
		<div class="card h-100 border border-light-subtle shadow-sm">
			<div class="card-header bg-white py-3 border-bottom border-light-subtle">
				<h6 class="fw-bold text-dark mb-0">Navegação Rápida Staff</h6>
			</div>
			<div class="card-body d-flex align-items-center">
				<div class="d-flex flex-wrap gap-2 w-100">
					<a href="console.py?session={session_id}" class="btn btn-outline-success btn-sm flex-fill">Console</a>
					<a href="accounts.py?session={session_id}" class="btn btn-outline-success btn-sm flex-fill">Contas</a>
					<a href="maps.py?session={session_id}" class="btn btn-outline-success btn-sm flex-fill">Mapas</a>
					<a href="pythonlog.py?session={session_id}" class="btn btn-outline-secondary btn-sm flex-fill">Python Logs</a>
				</div>
			</div>
		</div>
	</div>
</div>

<!-- Tabela de Jogadores Ativos -->
<div class="card border border-light-subtle shadow-sm mb-2">
	<div class="card-header bg-white py-3 d-flex justify-content-between align-items-center">
		<h6 class="fw-bold text-dark mb-0">Jogadores Conectados ({players_online})</h6>
		<a href="maps.py?session={session_id}" class="btn btn-outline-success btn-sm py-1">Rastrear nos Mapas &rarr;</a>
	</div>
	<div class="table-responsive">
		<table class="table table-hover table-striped align-middle mb-0">
			<thead class="table-light small text-uppercase text-secondary">
				<tr>
					<th scope="col" class="ps-3">Personagem</th>
					<th scope="col">Notoriedade</th>
					<th scope="col">Região / Cidade</th>
					<th scope="col">Coordenadas</th>
				</tr>
			</thead>
			<tbody>
				{player_table_html}
			</tbody>
		</table>
	</div>
</div>
"""

web.template.output( 	
	f'&gt; <a href="main.py?session={session_id}" class="text-decoration-none text-success fw-semibold">Dashboard Admin</a>', 
	username, 
	1, 
	content, 
	session_id
)