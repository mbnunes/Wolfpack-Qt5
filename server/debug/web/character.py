# Detailed Character Sheet - Paperdoll, Status, Skills, Bank Box e Propriedades

import web.sessions
import web.template
import cgi
import sys
import wolfpack
import wolfpack.accounts
import urllib.parse

try:
	from wolfpack.consts import ALLSKILLS
	from skills import SKILLS, SKILL_NAME
except Exception:
	ALLSKILLS = 55
	SKILLS = {}
	SKILL_NAME = 1

try:
	import housing
	import housing.house
except Exception:
	housing = None

MAP_NAMES = {
	0: "Felucca",
	1: "Trammel",
	2: "Ilshenar",
	3: "Malas",
	4: "Tokuno",
	5: "Ter Mur"
}

form = cgi.FieldStorage()
session_id = form.getvalue('session', '')
serial_str = form.getvalue('serial', '')
account_user = form.getvalue('account', '')

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

# Conversão do serial
serial_int = 0
try:
	if serial_str.startswith(('0x', '0X')):
		serial_int = int(serial_str, 16)
	else:
		serial_int = int(serial_str)
except Exception:
	serial_int = 0

char = None
is_online = False

# 1. Busca no mundo ativo (se online)
try:
	if serial_int > 0:
		active_char = wolfpack.findchar(serial_int)
		if active_char:
			char = active_char
			is_online = bool(getattr(active_char, 'socket', None))
except Exception:
	pass

# 2. Se offline, busca no registro persistente da conta
if not char:
	try:
		acc_list = [wolfpack.accounts.find(account_user)] if account_user else []
		if not acc_list or acc_list[0] is None:
			acc = wolfpack.accounts.first()
			while acc:
				for c in getattr(acc, 'characters', []):
					if getattr(c, 'serial', 0) == serial_int:
						char = c
						acc_list = [acc]
						break
				if char:
					break
				acc = wolfpack.accounts.next()
		else:
			for c in getattr(acc_list[0], 'characters', []):
				if getattr(c, 'serial', 0) == serial_int:
					char = c
					break
	except Exception:
		pass

if not char:
	content = f"""
	<div class="alert alert-warning border shadow-sm my-4">
		<h5 class="alert-heading fw-bold">Personagem Não Encontrado</h5>
		<p class="mb-0">Não foi possível localizar o serial <code>0x{serial_int:x}</code>.</p>
		<div class="mt-3">
			<a href="main.py?session={session_id}" class="btn btn-outline-dark btn-sm">&larr; Voltar ao Painel</a>
		</div>
	</div>
	"""
	web.template.output("Personagem", username, 1, content, session_id)
	sys.exit()

# Verificação de Permissão: Jogadores comuns só podem ver personagens da sua própria conta!
char_account_obj = getattr(char, 'account', None)
char_account_name = getattr(char_account_obj, 'name', '') if char_account_obj else account_user

if not is_admin and char_account_name.lower() != username.lower():
	content = f"""
	<div class="alert alert-danger border shadow-sm my-4">
		<h5 class="alert-heading fw-bold">&#128683; Acesso Negado</h5>
		<p class="mb-0">Você não tem permissão para visualizar a ficha de personagens pertencentes a outras contas.</p>
		<div class="mt-3">
			<a href="main.py?session={session_id}&view=player" class="btn btn-outline-danger btn-sm">&larr; Voltar ao Meu Painel</a>
		</div>
	</div>
	"""
	web.template.output("Acesso Negado", username, 1, content, session_id)
	sys.exit()

# Dados Básicos do Personagem
char_name = getattr(char, 'name', 'Sem Nome')
char_title = getattr(char, 'title', '')
char_gender = 'Mulher' if getattr(char, 'id', 0) in [0x191, 0x193] else 'Homem'
char_body_hex = f"0x{getattr(char, 'id', 0):04x}"

char_pos = getattr(char, 'pos', None)
map_id = getattr(char_pos, 'map', 0) if char_pos else 0
map_name = MAP_NAMES.get(map_id, f"Map {map_id}")
coords_str = f"{char_pos.x}, {char_pos.y}, {char_pos.z}" if char_pos else "N/A"

karma = getattr(char, 'karma', 0)
fame = getattr(char, 'fame', 0)
kills = getattr(char, 'kills', 0)
deaths = getattr(char, 'deaths', 0)

# Atributos (Stats)
str_val = getattr(char, 'strength', 0)
dex_val = getattr(char, 'dexterity', 0)
int_val = getattr(char, 'intelligence', 0)
statcap = getattr(char, 'statcap', 225)
total_stats = str_val + dex_val + int_val

hp_cur = getattr(char, 'hitpoints', str_val)
hp_max = max(1, getattr(char, 'maxhitpoints', str_val))
hp_pct = max(0, min(100, int((hp_cur / hp_max) * 100)))

stam_cur = getattr(char, 'stamina', dex_val)
stam_max = max(1, getattr(char, 'maxstamina', dex_val))
stam_pct = max(0, min(100, int((stam_cur / stam_max) * 100)))

mana_cur = getattr(char, 'mana', int_val)
mana_max = max(1, getattr(char, 'maxmana', int_val))
mana_pct = max(0, min(100, int((mana_cur / mana_max) * 100)))

weight_cur = getattr(char, 'weight', 0)
weight_max = getattr(char, 'maxweight', int(40 + (str_val * 3.5)))
weight_pct = max(0, min(100, int((weight_cur / max(1, weight_max)) * 100)))

# Ouro na Mochila
char_gold = 0
try:
	char_gold = char.countresource(0xEED)
except Exception:
	char_gold = 0

# ==============================================================================
# COFRE BANCÁRIO (BANK BOX) - ITENS E OURO
# ==============================================================================
bank = None
try:
	if hasattr(char, 'getbankbox'):
		bank = char.getbankbox()
except Exception:
	bank = None

bank_gold = 0
bank_items_count = 0
bank_rows = []

if bank:
	try:
		bank_gold = bank.countresource(0xEED)
	except Exception:
		bank_gold = 0

	raw_content = getattr(bank, 'content', [])
	bank_items_count = len(raw_content)

	for b_item in raw_content:
		i_name = getattr(b_item, 'name', '')
		i_id = getattr(b_item, 'id', 0)
		if not i_name:
			i_name = f"Item 0x{i_id:04x}"
		i_amount = getattr(b_item, 'amount', 1)
		i_serial = getattr(b_item, 'serial', 0)
		i_color = getattr(b_item, 'color', 0)
		i_color_str = f"0x{i_color:04x}" if i_color else "Padrão"

		is_subcontainer = bool(getattr(b_item, 'iscontainer', lambda: False)())
		sub_badge = '<span class="badge bg-info-subtle text-info-emphasis border border-info-subtle ms-1">Recipiente</span>' if is_subcontainer else ''

		bank_rows.append(f"""
		<tr>
			<td class="ps-3">
				<span class="fw-semibold text-dark">{cgi.escape(str(i_name))}</span>
				{sub_badge}
			</td>
			<td class="font-monospace fw-bold text-success">{i_amount:,}</td>
			<td class="font-monospace small text-muted">0x{i_id:04x}</td>
			<td class="small text-secondary">{i_color_str}</td>
			<td class="font-monospace small text-muted pe-3 text-end">0x{i_serial:x}</td>
		</tr>
		""")

total_wealth = char_gold + bank_gold

# ==============================================================================
# PAPERDOLL (EQUIPAMENTOS)
# ==============================================================================
PAPERDOLL_LAYERS = [
	(1,  "Mão Principal (Arma)"),
	(2,  "Mão Secundária (Escudo)"),
	(6,  "Cabeça (Elmo/Chapéu)"),
	(10, "Pescoço (Gargantilha)"),
	(13, "Torso (Armadura/Túnica)"),
	(17, "Costas (Manto/Capa)"),
	(7,  "Braços"),
	(8,  "Mãos (Luvas)"),
	(4,  "Pernas"),
	(3,  "Pés (Botas/Sapatos)"),
	(14, "Cintura (Cinto)"),
	(9,  "Dedo (Anel)"),
	(18, "Pulso (Bracelete)"),
	(11, "Cabelo"),
	(16, "Barba")
]

equipped_rows = []
for layer_id, layer_name in PAPERDOLL_LAYERS:
	item = None
	try:
		if hasattr(char, 'getlayer'):
			item = char.getlayer(layer_id)
		elif hasattr(char, 'itemonlayer'):
			item = char.itemonlayer(layer_id)
	except Exception:
		pass

	if item:
		item_name = getattr(item, 'name', '')
		item_id = getattr(item, 'id', 0)
		if not item_name:
			item_name = f"Item 0x{item_id:04x}"
		item_color = getattr(item, 'color', 0)
		color_text = f" &bull; Hue: 0x{item_color:04x}" if item_color else ""

		equipped_rows.append(f"""
		<li class="list-group-item d-flex justify-content-between align-items-center py-2 px-3 bg-light border-light-subtle">
			<span class="small fw-semibold text-secondary">{layer_name}:</span>
			<div class="text-end">
				<span class="small fw-bold text-dark d-block">{cgi.escape(str(item_name))}</span>
				<span class="font-monospace text-muted" style="font-size: 0.72rem;">0x{item_id:04x}{color_text}</span>
			</div>
		</li>
		""")

if not equipped_rows:
	equipped_rows.append('<li class="list-group-item text-muted small py-3 text-center border-0">Nenhum equipamento equipado.</li>')

# ==============================================================================
# HABILIDADES (SKILLS)
# ==============================================================================
skill_cards_learned = []
skill_cards_all = []
total_skills = 0.0

LOCK_ICONS = {0: '&#9650; Sobe', 1: '&#9660; Desce', 2: '&#128274; Trancada'}

for i in range(0, ALLSKILLS):
	try:
		val = char.skill[i] / 10.0
	except Exception:
		val = 0.0

	try:
		cap = char.skillcap[i] / 10.0
	except Exception:
		cap = 100.0

	try:
		lock = char.skilllock[i]
	except Exception:
		lock = 0

	total_skills += val
	s_name = SKILLS.get(i, [None, f"Skill #{i}"])[SKILL_NAME] if i in SKILLS else f"Skill #{i}"
	pct = min(100, int((val / max(1.0, cap)) * 100))
	lock_label = LOCK_ICONS.get(lock, '&#9650;')

	card_html = f"""
	<div class="col-12 col-md-6 col-xl-4">
		<div class="p-2 border rounded bg-light-subtle">
			<div class="d-flex justify-content-between align-items-center mb-1">
				<span class="small fw-semibold text-dark">{s_name}</span>
				<span class="small text-secondary font-monospace">
					<strong>{val:.1f}</strong> / {cap:.1f}% 
					<span class="ms-1 badge bg-light text-secondary border" title="Lock">{lock_label}</span>
				</span>
			</div>
			<div class="progress" style="height: 5px;">
				<div class="progress-bar bg-success" style="width: {pct}%;"></div>
			</div>
		</div>
	</div>
	"""
	skill_cards_all.append(card_html)
	if val > 0.0:
		skill_cards_learned.append(card_html)

# ==============================================================================
# PROPRIEDADES (CASAS) DESTE PERSONAGEM
# ==============================================================================
char_house = None
if housing:
	try:
		if hasattr(housing, 'findHouse'):
			char_house = housing.findHouse(char)
		if not char_house and hasattr(housing, 'getHouses'):
			h_dict = housing.getHouses()
			if serial_int in h_dict:
				char_house = wolfpack.finditem(h_dict[serial_int])
	except Exception:
		char_house = None

house_section = ""
if char_house:
	h_title = getattr(char_house, 'name', 'Propriedade')
	if hasattr(char_house, 'hastag') and char_house.hastag('sign'):
		sign_obj = wolfpack.finditem(int(char_house.gettag('sign')))
		if sign_obj and getattr(sign_obj, 'name', ''):
			h_title = sign_obj.name

	h_pos = getattr(char_house, 'pos', None)
	h_map = MAP_NAMES.get(h_pos.map if h_pos else 0, "Desconhecido")
	h_coords = f"{h_pos.x}, {h_pos.y}, {h_pos.z}" if h_pos else "N/A"

	is_pub = False
	coowner_list = []
	friends_list = []
	bans_list = []
	if housing and hasattr(housing, 'house'):
		try:
			is_pub = housing.house.isPublic(char_house)
			coowner_list = housing.house.getCoOwners(char_house)
			friends_list = housing.house.getFriends(char_house)
			bans_list = housing.house.getBans(char_house)
		except Exception:
			pass

	pub_badge = '<span class="badge bg-success-subtle text-success border border-success">Pública</span>' if is_pub else '<span class="badge bg-secondary-subtle text-secondary border">Privada</span>'

	house_section = f"""
	<div class="card border border-light-subtle shadow-sm mb-4">
		<div class="card-header bg-white py-3 d-flex justify-content-between align-items-center">
			<h6 class="fw-bold text-dark mb-0">&#127960; Casa Própria deste Personagem</h6>
			<a href="properties.py?session={session_id}" class="btn btn-sm btn-outline-success">Gerenciar Propriedades &rarr;</a>
		</div>
		<div class="card-body">
			<div class="row g-3">
				<div class="col-12 col-md-4">
					<div class="text-secondary small">Nome da Placa:</div>
					<h5 class="fw-bold text-dark mb-0">{cgi.escape(str(h_title))}</h5>
				</div>
				<div class="col-12 col-md-4">
					<div class="text-secondary small">Localização:</div>
					<div class="fw-semibold text-dark">{h_map} <span class="font-monospace text-muted small">({h_coords})</span></div>
				</div>
				<div class="col-12 col-md-4">
					<div class="text-secondary small">Acesso & Segurança:</div>
					<div>{pub_badge}</div>
				</div>
				<div class="col-12 pt-2 border-top">
					<span class="small text-secondary me-3">Co-proprietários: <strong>{len(coowner_list)}</strong></span>
					<span class="small text-secondary me-3">Amigos: <strong>{len(friends_list)}</strong></span>
					<span class="small text-secondary">Banidos: <strong>{len(bans_list)}</strong></span>
				</div>
			</div>
		</div>
	</div>
	"""
else:
	house_section = f"""
	<div class="card border border-light-subtle shadow-sm mb-4">
		<div class="card-body py-3 d-flex justify-content-between align-items-center">
			<span class="text-secondary small">&#127960; Este personagem não é proprietário principal de nenhuma casa no momento.</span>
			<a href="properties.py?session={session_id}" class="btn btn-sm btn-outline-secondary py-0">Ver Propriedades da Conta</a>
		</div>
	</div>
	"""

status_badge = '<span class="badge bg-success-subtle text-success border border-success">Online no Jogo</span>' if is_online else '<span class="badge bg-secondary-subtle text-secondary border">Offline</span>'

content = f"""
<!-- Cabeçalho -->
<div class="d-flex flex-wrap justify-content-between align-items-center pb-2 mb-3 border-bottom gap-2">
	<div>
		<div class="d-flex align-items-center gap-2">
			<h4 class="fw-bold text-dark mb-0">{cgi.escape(str(char_name))}</h4>
			{status_badge}
			<span class="badge bg-secondary-subtle text-dark border font-monospace">0x{serial_int:x}</span>
		</div>
		<p class="text-secondary small mb-0">
			{cgi.escape(str(char_title)) if char_title else 'Aventureiro de Britannia'} &bull; 
			Gênero: <strong>{char_gender}</strong> &bull; 
			Localização: <strong>{map_name} ({coords_str})</strong> &bull; 
			Conta: <strong>{cgi.escape(str(char_account_name))}</strong>
		</p>
	</div>
	<div class="d-flex gap-2">
		<a href="player_chars.py?session={session_id}" class="btn btn-outline-secondary btn-sm">
			&larr; Meus Personagens
		</a>
		<a href="main.py?session={session_id}&view=player" class="btn btn-outline-success btn-sm">
			Painel Principal
		</a>
	</div>
</div>

<!-- Linha 1: Resumo Financeiro & Vitals -->
<div class="row g-3 mb-4">
	<!-- Cofre Bancário Resumo -->
	<div class="col-12 col-md-4">
		<div class="card border border-light-subtle shadow-sm h-100">
			<div class="card-body">
				<span class="text-secondary small fw-semibold text-uppercase">&#129689; Ouro no Banco</span>
				<h3 class="fw-bold text-success mt-2 mb-1 font-monospace">{bank_gold:,} gps</h3>
				<div class="text-secondary small mt-2">
					Mochila: <span class="font-monospace fw-semibold text-dark">{char_gold:,} gps</span><br>
					Patrimônio Líquido: <span class="font-monospace fw-bold text-dark">{total_wealth:,} gps</span>
				</div>
			</div>
		</div>
	</div>

	<!-- Notoriedade & Reputação -->
	<div class="col-12 col-md-4">
		<div class="card border border-light-subtle shadow-sm h-100">
			<div class="card-body">
				<span class="text-secondary small fw-semibold text-uppercase">&#9876; Notoriedade & Honra</span>
				<div class="d-flex justify-content-between align-items-center mt-2 py-1 border-bottom">
					<span class="small text-secondary">Karma:</span>
					<span class="font-monospace fw-bold text-dark">{karma}</span>
				</div>
				<div class="d-flex justify-content-between align-items-center py-1 border-bottom">
					<span class="small text-secondary">Fame:</span>
					<span class="font-monospace fw-bold text-dark">{fame}</span>
				</div>
				<div class="d-flex justify-content-between align-items-center py-1">
					<span class="small text-secondary">Mortes / Kills:</span>
					<span class="font-monospace text-muted">{deaths} mortes / {kills} kills</span>
				</div>
			</div>
		</div>
	</div>

	<!-- Carga & Atributos Cap -->
	<div class="col-12 col-md-4">
		<div class="card border border-light-subtle shadow-sm h-100">
			<div class="card-body">
				<span class="text-secondary small fw-semibold text-uppercase">&#9878; Capacidade de Carga</span>
				<div class="d-flex justify-content-between small text-secondary mt-2 mb-1">
					<span>Peso Carregado:</span>
					<span class="font-monospace fw-bold text-dark">{weight_cur} / {weight_max} stones</span>
				</div>
				<div class="progress mb-3" style="height: 8px;">
					<div class="progress-bar {'bg-danger' if weight_pct > 90 else 'bg-success'}" style="width: {weight_pct}%;"></div>
				</div>
				<div class="d-flex justify-content-between align-items-center small pt-1 border-top">
					<span class="text-secondary">Soma de Stats:</span>
					<span class="badge bg-light text-dark border font-monospace">{total_stats} / {statcap}</span>
				</div>
			</div>
		</div>
	</div>
</div>

<!-- Linha 2: Status Vital e Paperdoll -->
<div class="row g-4 mb-4">
	<!-- Coluna Esquerda: Status Vital & Atributos -->
	<div class="col-12 col-lg-6">
		<div class="card border border-light-subtle shadow-sm h-100">
			<div class="card-header bg-white py-2">
				<span class="fw-semibold text-secondary small text-uppercase">Status Vital & Atributos</span>
			</div>
			<div class="card-body">
				<div class="mb-3">
					<div class="d-flex justify-content-between small text-secondary mb-1">
						<span class="fw-semibold">Pontos de Vida (HP):</span>
						<span class="font-monospace fw-bold text-dark">{hp_cur} / {hp_max}</span>
					</div>
					<div class="progress" style="height: 10px;">
						<div class="progress-bar bg-danger" role="progressbar" style="width: {hp_pct}%;"></div>
					</div>
				</div>

				<div class="mb-3">
					<div class="d-flex justify-content-between small text-secondary mb-1">
						<span class="fw-semibold">Stamina:</span>
						<span class="font-monospace fw-bold text-dark">{stam_cur} / {stam_max}</span>
					</div>
					<div class="progress" style="height: 10px;">
						<div class="progress-bar bg-warning" role="progressbar" style="width: {stam_pct}%;"></div>
					</div>
				</div>

				<div class="mb-3">
					<div class="d-flex justify-content-between small text-secondary mb-1">
						<span class="fw-semibold">Mana:</span>
						<span class="font-monospace fw-bold text-dark">{mana_cur} / {mana_max}</span>
					</div>
					<div class="progress" style="height: 10px;">
						<div class="progress-bar bg-primary" role="progressbar" style="width: {mana_pct}%;"></div>
					</div>
				</div>

				<div class="row text-center pt-3 border-top g-2 mt-2">
					<div class="col-4">
						<span class="text-secondary small d-block">FORÇA (STR)</span>
						<span class="fs-4 fw-bold text-dark">{str_val}</span>
					</div>
					<div class="col-4">
						<span class="text-secondary small d-block">DESTREZA (DEX)</span>
						<span class="fs-4 fw-bold text-dark">{dex_val}</span>
					</div>
					<div class="col-4">
						<span class="text-secondary small d-block">INTELIGÊNCIA (INT)</span>
						<span class="fs-4 fw-bold text-dark">{int_val}</span>
					</div>
				</div>
			</div>
		</div>
	</div>

	<!-- Coluna Direita: Paperdoll (Equipamentos) -->
	<div class="col-12 col-lg-6">
		<div class="card border border-light-subtle shadow-sm h-100">
			<div class="card-header bg-white py-2 d-flex justify-content-between align-items-center">
				<span class="fw-semibold text-secondary small text-uppercase">Equipamentos em Uso (Paperdoll)</span>
				<span class="badge bg-dark-subtle text-dark font-monospace">{len(equipped_rows)} camadas</span>
			</div>
			<ul class="list-group list-group-flush overflow-auto" style="max-height: 380px;">
				{''.join(equipped_rows)}
			</ul>
		</div>
	</div>
</div>

<!-- Seção 3: Propriedades / Casas -->
{house_section}

<!-- Seção 4: Cofre Bancário (Itens Guardados no Banco) -->
<div class="card border border-light-subtle shadow-sm mb-4">
	<div class="card-header bg-white py-3 d-flex justify-content-between align-items-center">
		<div>
			<h6 class="fw-bold text-dark mb-0">&#128176; Cofre Bancário (Bank Box)</h6>
			<small class="text-muted">Total de Itens: <strong>{bank_items_count}</strong> &bull; Saldo de Ouro: <strong class="text-success">{bank_gold:,} gps</strong></small>
		</div>
		<span class="badge bg-success-subtle text-success border border-success">Seguro na Cidade</span>
	</div>
	<div class="table-responsive" style="max-height: 400px; overflow-y: auto;">
		<table class="table table-hover table-striped align-middle mb-0">
			<thead class="table-light small text-uppercase text-secondary sticky-top">
				<tr>
					<th scope="col" class="ps-3">Item</th>
					<th scope="col">Quantidade</th>
					<th scope="col">Graphic ID</th>
					<th scope="col">Cor (Hue)</th>
					<th scope="col" class="pe-3 text-end">Serial</th>
				</tr>
			</thead>
			<tbody>
				{''.join(bank_rows) if bank_rows else '<tr><td colspan="5" class="text-center py-4 text-muted">O cofre bancário deste personagem está vazio no momento.</td></tr>'}
			</tbody>
		</table>
	</div>
</div>

<!-- Seção 5: Perícias (Skills) -->
<div class="card border border-light-subtle shadow-sm mb-4">
	<div class="card-header bg-white py-3 d-flex flex-wrap justify-content-between align-items-center gap-2">
		<div>
			<h6 class="fw-bold text-dark mb-0">&#128218; Grade de Habilidades (Skills)</h6>
			<small class="text-muted">Total de Pontos: <strong class="text-success font-monospace">{total_skills:.1f}%</strong></small>
		</div>
		<div>
			<ul class="nav nav-pills nav-fill small" id="skillTabs" role="tablist">
				<li class="nav-item" role="presentation">
					<button class="nav-link active py-1 px-3" id="learned-tab" data-bs-toggle="pill" data-bs-target="#learned-skills" type="button" role="tab">
						Aprendidas ({len(skill_cards_learned)})
					</button>
				</li>
				<li class="nav-item" role="presentation">
					<button class="nav-link py-1 px-3" id="all-tab" data-bs-toggle="pill" data-bs-target="#all-skills" type="button" role="tab">
						Todas as Perícias ({len(skill_cards_all)})
					</button>
				</li>
			</ul>
		</div>
	</div>
	<div class="card-body">
		<div class="tab-content" id="skillTabsContent">
			<!-- Tab Aprendidas -->
			<div class="tab-pane fade show active" id="learned-skills" role="tabpanel">
				<div class="row g-2">
					{''.join(skill_cards_learned) if skill_cards_learned else '<div class="col-12 text-muted small py-3 text-center">Nenhuma habilidade com pontos aprendida neste personagem.</div>'}
				</div>
			</div>
			<!-- Tab Todas -->
			<div class="tab-pane fade" id="all-skills" role="tabpanel">
				<div class="row g-2">
					{''.join(skill_cards_all)}
				</div>
			</div>
		</div>
	</div>
</div>
"""

web.template.output(
	f'&gt; <a href="character.py?session={session_id}&serial={serial_str}" class="text-decoration-none text-success fw-semibold">{char_name}</a>',
	username,
	1,
	content,
	session_id
)