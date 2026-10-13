import web.sessions
import web.template
import cgi
import sys
import wolfpack
import wolfpack.sockets
import wolfpack.accounts
import wolfpack.time

form = cgi.FieldStorage()
session_id = form.getvalue('session', '')

web.sessions.check_timeouts()
session = web.sessions.get_session(session_id)

if session is None:
	web.sessions.display_timeout()
	sys.exit()

username = session['username']

# 1. Coleta de Métricas em Tempo Real do Servidor
total_accounts = wolfpack.accounts.count()
npc_count = wolfpack.npccount()
item_count = wolfpack.itemcount()

# Tempo in-game
game_day = wolfpack.time.days()
game_hour = wolfpack.time.hour()
game_min = "%02u" % wolfpack.time.minute()

# 2. Coleta e mapeamento de Jogadores Conectados
connected_players = []
socket = wolfpack.sockets.first()
nextsocket = wolfpack.sockets.next()

# Mapeamento oficial de Notoriedade (Cores e Rótulos clássicos de UO)
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
		
		# Gênero
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

# Monta tabela de jogadores conectados
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
<!-- Cabeçalho do Dashboard -->
<div class="d-flex flex-wrap justify-content-between align-items-center pb-2 mb-4 border-bottom gap-2">
	<div>
		<h4 class="fw-bold text-dark mb-0">Painel Operacional</h4>
		<p class="text-secondary small mb-0">Status e telemetria da instância em tempo real.</p>
	</div>
	<div class="d-flex align-items-center gap-2">
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
				<h6 class="fw-bold text-dark mb-0">Navegação Rápida</h6>
			</div>
			<div class="card-body d-flex align-items-center">
				<div class="d-flex flex-wrap gap-2 w-100">
					<a href="console.py?session={session_id}" class="btn btn-outline-success btn-sm flex-fill">Console do Servidor</a>
					<a href="accounts.py?session={session_id}" class="btn btn-outline-success btn-sm flex-fill">Gerenciar Contas</a>
					<a href="maps.py?session={session_id}" class="btn btn-outline-success btn-sm flex-fill">Ver Jogadores nos Mapas</a>
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
	f'&gt; <a href="main.py?session={session_id}" class="text-decoration-none text-success fw-semibold">Dashboard</a>', 
	username, 
	1, 
	content,
	session_id
)