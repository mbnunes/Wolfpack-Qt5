# Detailed Character Sheet - Paperdoll, Status, Skills, Bank Box e Propriedades

import sys, os

for _p in [
	'/usr/lib/python3/dist-packages',
	'/usr/local/lib/python3.14/dist-packages',
	'/usr/lib/python3.14/dist-packages',
	'/usr/local/lib/python3/dist-packages',
	'/usr/lib/python3.13/dist-packages',
	'/usr/lib/python3.12/dist-packages',
]:
	if os.path.isdir(_p) and _p not in sys.path:
		sys.path.append(_p)

import web.sessions
import web.template
try:
	import web.uo_assets
except Exception:
	pass
import cgi
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

# Raça e Gênero
char_body_id = getattr(char, 'id', 0x190)
if char_body_id in [0x25D, 0x25E, 605, 606]:
	char_race = 'Elfo'
elif char_body_id in [0x29A, 0x29B, 666, 667]:
	char_race = 'Gárgula'
else:
	char_race = 'Humano'

char_skin_color = getattr(char, 'skin', 0) or getattr(char, 'color', 0) or getattr(char, 'skincolor', 0)

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

# Armadura e Resistências
armor_val = 0
res_fire = 0
res_cold = 0
res_poison = 0
res_energy = 0
try:
	from wolfpack.consts import RESISTANCE_PHYSICAL, RESISTANCE_FIRE, RESISTANCE_COLD, RESISTANCE_POISON, RESISTANCE_ENERGY
	from wolfpack import properties
	armor_val = properties.fromchar(char, RESISTANCE_PHYSICAL)
	res_fire = properties.fromchar(char, RESISTANCE_FIRE)
	res_cold = properties.fromchar(char, RESISTANCE_COLD)
	res_poison = properties.fromchar(char, RESISTANCE_POISON)
	res_energy = properties.fromchar(char, RESISTANCE_ENERGY)
except Exception:
	armor_val = getattr(char, 'defense', 0)

# Propriedades do Gump 10860 (Modern Status Window)
luck_val = 0
dmg_min = 0
dmg_max = 0
hci_val = 0
dci_val = 0
lmc_val = 0
lrc_val = 0
fc_val = 0
fcr_val = 0

try:
	from wolfpack.consts import (
		LUCK, MINDAMAGE, MAXDAMAGE, HITBONUS, DEFENSEBONUS,
		LOWERMANACOST, LOWERREAGENTCOST, CASTSPEEDBONUS, CASTRECOVERYBONUS
	)
	from wolfpack import properties
	luck_val = properties.fromchar(char, LUCK)
	hci_val = properties.fromchar(char, HITBONUS)
	dci_val = properties.fromchar(char, DEFENSEBONUS)
	lmc_val = properties.fromchar(char, LOWERMANACOST)
	lrc_val = properties.fromchar(char, LOWERREAGENTCOST)
	fc_val = properties.fromchar(char, CASTSPEEDBONUS)
	fcr_val = properties.fromchar(char, CASTRECOVERYBONUS)

	weapon = None
	if hasattr(char, 'getweapon'):
		weapon = char.getweapon()
	if weapon:
		dmg_min = properties.fromitem(weapon, MINDAMAGE)
		dmg_max = properties.fromitem(weapon, MAXDAMAGE)
	else:
		dmg_min = max(1, int(str_val / 10))
		dmg_max = max(4, int(str_val / 5) + 2)
except Exception:
	luck_val = getattr(char, 'luck', 0)
	dmg_min = max(1, int(str_val / 10))
	dmg_max = max(4, int(str_val / 5) + 2)

# Montarias e Seguidores (Followers)
followers_cur = 0
followers_max = getattr(char, 'maxfollowers', 5)
try:
	f_list = getattr(char, 'followers', ())
	followers_cur = len(f_list) if isinstance(f_list, (list, tuple)) else int(f_list or 0)
except Exception:
	followers_cur = 0

# Pontos de Dízimo (Tithing Points)
tithing_val = getattr(char, 'tithing', 0) or getattr(char, 'tithingpoints', 0)

# Limites de Resistência (Caps)
res_phys_max = 70
res_fire_max = 70
res_cold_max = 70
res_poison_max = 70
res_energy_max = 70

# Paperdoll Composited (Gump 2001 - Direto do UOP / MUL)
paperdoll_url = None
try:
	import importlib
	import web.uo_assets
	if not hasattr(web.uo_assets, 'get_character_paperdoll'):
		importlib.reload(web.uo_assets)
	paperdoll_url = web.uo_assets.get_character_paperdoll(char)
except Exception as e:
	try:
		import traceback, os
		os.makedirs('logs', exist_ok=True)
		with open('logs/paperdoll_debug.log', 'a') as f:
			f.write(f"Error loading paperdoll for serial 0x{serial_int:x}: {e}\n")
			traceback.print_exc(file=f)
	except Exception:
		pass
	paperdoll_url = None

if not paperdoll_url:
	paperdoll_url = "images/gumps/gump_2001_0.png"

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
		i_id = getattr(b_item, 'id', 0)
		i_name = getattr(b_item, 'name', '')
		if not i_name or str(i_name).lower().startswith('item 0x') or str(i_name).lower() == 'item':
			if i_id in (0x0EED, 0x0EEE, 0x0EEF):
				i_name = "Moedas de Ouro (Gold)"
			else:
				t_name = ""
				if hasattr(web, 'uo_assets') and hasattr(web.uo_assets, 'get_item_anim_id'):
					try:
						_, _, t_name = web.uo_assets.get_item_anim_id(i_id)
						t_name = ''.join(c for c in str(t_name) if c.isprintable()).strip()
						t_name = t_name.strip().replace('%s%', '').replace('%/s', '').replace('%', '').strip()
					except Exception:
						t_name = ""
				if t_name:
					if t_name.lower() == 'book':
						i_name = "Livro (Book)"
					elif t_name.lower() == 'spellbook':
						i_name = "Livro de Magia (Spellbook)"
					else:
						i_name = t_name.title()
				elif getattr(b_item, 'baseid', ''):
					b_id_str = str(b_item.baseid).strip()
					if b_id_str in ('fef', 'ff0', 'ff1', 'ff2', 'ff3', 'ff4', 'fbd', 'fbe'):
						i_name = "Livro (Book)"
					else:
						i_name = b_id_str.replace('_', ' ').strip().title()
				else:
					i_name = f"Item 0x{i_id:04x}"

		i_amount = getattr(b_item, 'amount', 1)
		i_serial = getattr(b_item, 'serial', 0)
		i_color = getattr(b_item, 'color', 0)
		i_color_str = f"0x{i_color:04x}" if i_color else "Padrão"

		is_subcontainer = bool(getattr(b_item, 'iscontainer', lambda: False)())
		sub_badge = '<span class="badge bg-info-subtle text-info-emphasis border border-info-subtle ms-1" style="font-size: 0.7rem;">Recipiente</span>' if is_subcontainer else ''

		# Resolve Item Artwork (com crop para ajuste perfeito ao thumbnail)
		item_art_url = None
		if hasattr(web, 'uo_assets') and hasattr(web.uo_assets, 'get_item_art'):
			try:
				_, item_art_url = web.uo_assets.get_item_art(i_id, i_color, crop=True)
			except Exception:
				item_art_url = None

		escaped_name = cgi.escape(str(i_name))
		if item_art_url:
			img_cell = f'''<div class="d-inline-flex align-items-center justify-content-center bg-white rounded border shadow-sm p-1" style="width: 44px; height: 44px; min-width: 44px;" title="{escaped_name}"><img src="{item_art_url}" alt="{escaped_name}" title="{escaped_name}" style="max-width: 38px; max-height: 38px; object-fit: contain; image-rendering: pixelated; cursor: pointer;" /></div>'''
		else:
			img_cell = f'''<div class="d-inline-flex align-items-center justify-content-center bg-light rounded border p-1" style="width: 44px; height: 44px; min-width: 44px;" title="{escaped_name}"><span class="text-secondary" style="font-size: 1.2rem;">&#128230;</span></div>'''

		bank_rows.append(f"""
		<tr>
			<td class="ps-3 py-2">
				<div class="d-flex align-items-center gap-2">
					{img_cell}
					<div>
						<div class="fw-semibold text-dark small">{escaped_name}</div>
						{sub_badge}
					</div>
				</div>
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
	(2,  "Mão Secundária (Escudo / Arma 2M)"),
	(6,  "Cabeça (Elmo / Chapéu)"),
	(10, "Pescoço (Gargantilha / Colar)"),
	(5,  "Torso Interior (Camisa)"),
	(13, "Torso Superior (Armadura / Peitoral)"),
	(17, "Torso Médio (Sobretúnica / Túnica)"),
	(22, "Torso Exterior (Robe / Vestido)"),
	(19, "Braços (Mangas / Braçadeiras)"),
	(7,  "Mãos (Luvas)"),
	(4,  "Pernas Interiores (Calças)"),
	(23, "Pernas Exteriores (Saia / Kilt)"),
	(24, "Pernas / Perneiras"),
	(3,  "Pés (Botas / Sapatos)"),
	(20, "Costas (Capa / Manto)"),
	(12, "Cintura (Cinto / Avental)"),
	(14, "Pulso (Bracelete)"),
	(8,  "Dedo (Anel)"),
	(18, "Orelhas (Brincos)"),
	(9,  "Talismã"),
	(11, "Cabelo"),
	(16, "Barba / Pêlo Facial"),
	(25, "Montaria")
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

		# Imagem do Item extraída do art.mul
		art_url = None
		try:
			import web.uo_assets
			_, art_url = web.uo_assets.get_item_art(item_id, item_color)
		except Exception:
			art_url = None

		# Estatísticas e Propriedades do Item
		stats_badges = []

		# Durabilidade (HP / Max HP)
		hp = getattr(item, 'health', 0)
		maxhp = getattr(item, 'maxhealth', 0)
		if maxhp > 0:
			hp_pct = (hp / maxhp) * 100 if maxhp else 100
			hp_color = "success" if hp_pct > 60 else ("warning" if hp_pct > 25 else "danger")
			stats_badges.append(f'<span class="badge bg-{hp_color}-subtle text-{hp_color}-emphasis border border-{hp_color}-subtle" title="Durabilidade">&#128737; Durabilidade: {hp}/{maxhp}</span>')

		# Dano (Armas)
		min_dmg = 0
		max_dmg = 0
		try:
			if hasattr(item, 'hastag') and item.hastag('mindamage'):
				min_dmg = int(item.gettag('mindamage'))
			elif hasattr(item, 'getintproperty'):
				min_dmg = item.getintproperty('mindamage', 0)
			if hasattr(item, 'hastag') and item.hastag('maxdamage'):
				max_dmg = int(item.gettag('maxdamage'))
			elif hasattr(item, 'getintproperty'):
				max_dmg = item.getintproperty('maxdamage', 0)
		except Exception:
			pass
		if max_dmg > 0:
			stats_badges.append(f'<span class="badge bg-danger-subtle text-danger border border-danger-subtle" title="Dano">&#9876; Dano: {min_dmg}-{max_dmg}</span>')

		# Velocidade de Ataque (Speed)
		speed = 0
		try:
			if hasattr(item, 'hastag') and item.hastag('speed'):
				speed = int(item.gettag('speed'))
			elif hasattr(item, 'getintproperty'):
				speed = item.getintproperty('speed', 0)
		except Exception:
			pass
		if speed > 0:
			stats_badges.append(f'<span class="badge bg-warning-subtle text-warning-emphasis border border-warning-subtle" title="Velocidade de Ataque">&#9889; Vel: {speed}</span>')

		# Defesa / Armadura
		defense = 0
		try:
			if hasattr(item, 'hastag') and item.hastag('defense'):
				defense = int(item.gettag('defense'))
			elif hasattr(item, 'getintproperty'):
				defense = item.getintproperty('defense', 0)
			if not defense:
				if hasattr(item, 'hastag') and item.hastag('armor'):
					defense = int(item.gettag('armor'))
				elif hasattr(item, 'getintproperty'):
					defense = item.getintproperty('armor', 0)
		except Exception:
			pass
		if defense > 0:
			stats_badges.append(f'<span class="badge bg-primary-subtle text-primary border border-primary-subtle" title="Defesa">&#128737; Defesa: +{defense}</span>')

		# Resistências Elementais
		for r_prop, r_name, r_icon, r_color in [
			('res_physical', 'Físico', '&#128737;', '#495057'),
			('res_fire', 'Fogo', '&#128293;', '#dc3545'),
			('res_cold', 'Frio', '&#10052;', '#0dcaf0'),
			('res_poison', 'Veneno', '&#9760;', '#198754'),
			('res_energy', 'Energia', '&#9889;', '#fd7e14')
		]:
			r_val = 0
			try:
				if hasattr(item, 'hastag') and item.hastag(r_prop):
					r_val = int(item.gettag(r_prop))
				elif hasattr(item, 'getintproperty'):
					r_val = int(item.getintproperty(r_prop, 0))
			except Exception:
				pass
			if r_val > 0:
				stats_badges.append(f'<span class="badge border" style="background-color: #f8f9fa; color: {r_color}; border-color: {r_color}40 !important;" title="Resistência {r_name}">{r_icon} {r_val}%</span>')

		# Requisito de Força (req_strength)
		req_str = 0
		try:
			if hasattr(item, 'hastag') and item.hastag('req_strength'):
				req_str = int(item.gettag('req_strength'))
			elif hasattr(item, 'getintproperty'):
				req_str = item.getintproperty('req_strength', 0)
		except Exception:
			pass
		if req_str > 0:
			stats_badges.append(f'<span class="badge bg-light text-secondary border" title="Força Necessária">&#128170; Req. STR: {req_str}</span>')

		# Peso
		weight = getattr(item, 'weight', 0)
		if weight > 0:
			w_str = f"{weight:.1f}" if isinstance(weight, float) else str(weight)
			stats_badges.append(f'<span class="badge bg-light text-muted border" title="Peso">&#9878; {w_str} st</span>')

		# Duas Mãos
		if getattr(item, 'twohanded', False):
			stats_badges.append('<span class="badge bg-secondary text-white" title="Arma de Duas Mãos">&#9874; 2 Mãos</span>')

		# Blessed / Newbie
		is_blessed = False
		try:
			is_blessed = bool(getattr(item, 'newbie', False) or (hasattr(item, 'isblessed') and item.isblessed()))
		except Exception:
			pass
		if is_blessed:
			stats_badges.append('<span class="badge bg-warning text-dark" title="Item Abençoado">&#10024; Blessed</span>')

		# Meditação
		if getattr(item, 'allowmeditation', False):
			stats_badges.append('<span class="badge bg-info-subtle text-info-emphasis border border-info-subtle" title="Permite Meditação">&#129496; Meditação</span>')

		# Recurso / Material (ex: iron, leather, shadow...)
		resname = ''
		try:
			if hasattr(item, 'hastag') and item.hastag('resname'):
				resname = str(item.gettag('resname'))
		except Exception:
			pass
		if resname:
			stats_badges.append(f'<span class="badge bg-light text-secondary border text-capitalize">&#128296; {resname}</span>')

		# Slayer
		slayer = ''
		try:
			if hasattr(item, 'hastag') and item.hastag('slayer'):
				slayer = str(item.gettag('slayer'))
		except Exception:
			pass
		if slayer:
			stats_badges.append(f'<span class="badge bg-danger text-white" title="Slayer">&#9760; {slayer} Slayer</span>')

		# Artifact Rarity
		rarity = 0
		try:
			if hasattr(item, 'getintproperty'):
				rarity = item.getintproperty('artifact_rarity', 0)
		except Exception:
			pass
		if rarity > 0:
			stats_badges.append(f'<span class="badge bg-primary text-white" title="Raridade">&#11088; Rarity {rarity}</span>')

		img_html = f'''<div class="d-flex align-items-center justify-content-center bg-dark-subtle rounded border me-3 flex-shrink-0" style="width: 44px; height: 44px; overflow: hidden;"><img src="{art_url}" alt="{cgi.escape(str(item_name))}" style="max-width: 40px; max-height: 40px; object-fit: contain; image-rendering: pixelated;" /></div>''' if art_url else f'''<div class="d-flex align-items-center justify-content-center bg-light rounded border me-3 flex-shrink-0" style="width: 44px; height: 44px;"><span class="text-muted" style="font-size: 1.2rem;">&#128085;</span></div>'''

		stats_html = f'''<div class="d-flex flex-wrap gap-1 mt-1">{''.join(stats_badges)}</div>''' if stats_badges else ''

		equipped_rows.append(f"""
		<li class="list-group-item d-flex align-items-center py-2 px-3 bg-light border-light-subtle">
			{img_html}
			<div class="flex-grow-1 min-w-0">
				<div class="d-flex justify-content-between align-items-center mb-1">
					<span class="small fw-bold text-dark text-truncate me-2">{cgi.escape(str(item_name))}</span>
					<span class="badge bg-secondary-subtle text-secondary-emphasis border font-monospace" style="font-size: 0.72rem;">{layer_name}</span>
				</div>
				<div class="d-flex justify-content-between align-items-center">
					<span class="font-monospace text-muted" style="font-size: 0.72rem;">0x{item_id:04x}{color_text}</span>
				</div>
				{stats_html}
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
scroll_rows = []
total_skills = 0.0

LOCK_ICONS = {0: '&#9650; Sobe', 1: '&#9660; Desce', 2: '&#128274; Trancada'}
ACTIVE_SKILLS = {1, 2, 3, 4, 6, 9, 12, 14, 15, 16, 19, 20, 21, 22, 23, 30, 32, 33, 35, 36, 38, 46, 47, 48}

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

	# Card HTML para Grid Moderna
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

	# Linha do Pergaminho Oficial In-Game
	is_active = i in ACTIVE_SKILLS
	btn_html = '<img src="images/gumps/gump_2084_0.png" width="20" height="15" alt="Active" style="vertical-align: middle;" title="Habilidade Ativável" />' if is_active else '<span style="display:inline-block; width:20px; height:15px;"></span>'

	if lock == 1:
		lock_img = '<img src="images/gumps/gump_2436_0.png" width="9" height="11" alt="Down" title="Desce (Down)" style="vertical-align: middle;" />'
	elif lock == 2:
		lock_img = '<img src="images/gumps/gump_2437_0.png" width="9" height="11" alt="Lock" title="Trancada (Locked)" style="vertical-align: middle;" />'
	else:
		lock_img = '<img src="images/gumps/gump_2435_0.png" width="9" height="11" alt="Up" title="Sobe (Up)" style="vertical-align: middle;" />'

	scroll_rows.append(f"""
	<div class="d-flex justify-content-between align-items-center py-1 px-1 border-bottom skill-scroll-row" data-learned="{'1' if val > 0 else '0'}" style="border-color: rgba(110, 80, 50, 0.2) !important;">
		<div class="d-flex align-items-center gap-1 text-truncate" style="max-width: 155px;">
			{btn_html}
			<span class="text-truncate fw-semibold" style="color: #2b1704; font-size: 11.5px; font-family: 'Cinzel', 'Georgia', serif;" title="{cgi.escape(str(s_name))}">{cgi.escape(str(s_name))}</span>
		</div>
		<div class="d-flex align-items-center gap-1 font-monospace text-end">
			<span class="fw-bold" style="color: #1a0e02; font-size: 11.5px;">{val:.1f}%</span>
			{lock_img}
		</div>
	</div>
	""")

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
		if not char_house:
			it = wolfpack.itemiterator()
			item = getattr(it, 'first', None)
			while item:
				is_multi = (getattr(item, 'objects', None) is not None) or (getattr(item, 'id', 0) >= 0x4000)
				if is_multi:
					owner_val = getattr(item, 'owner', None) or getattr(item, 'ownerserial', 0)
					o_serial = int(getattr(owner_val, 'serial', owner_val) or 0)
					if o_serial == serial_int:
						char_house = item
						if hasattr(housing, 'registerHouse'):
							housing.registerHouse(item)
						break
				item = getattr(it, 'next', None)
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


	# Resolve House Art & Model Info
	ch_h_img = "images/items/item_8900_0.png"
	ch_h_title = "Estrutura Residencial"
	ch_h_cat = "Propriedade"
	try:
		import web.uo_assets as _uo_assets
		_, rel_p, m_title, m_cat = _uo_assets.get_house_image(char_house)
		if rel_p:
			ch_h_img = rel_p
		if m_title:
			ch_h_title = m_title
		if m_cat:
			ch_h_cat = m_cat
	except Exception:
		pass

	house_section = f"""
	<div class="card border border-light-subtle shadow-sm mb-4">
		<div class="card-header bg-white py-3 d-flex justify-content-between align-items-center">
			<h6 class="fw-bold text-dark mb-0">&#127960; Casa Própria deste Personagem</h6>
			<a href="properties.py?session={session_id}" class="btn btn-sm btn-outline-success">Gerenciar Propriedades &rarr;</a>
		</div>
		<div class="card-body">
			<div class="d-flex flex-column flex-md-row gap-3 align-items-md-center mb-3 pb-3 border-bottom">
				<a href="{ch_h_img}" target="_blank" title="Clique para ampliar a residência" class="d-flex align-items-center justify-content-center bg-light rounded border shadow-sm p-1 text-decoration-none" style="width: 72px; height: 72px; min-width: 72px; cursor: pointer;">
					<img src="{ch_h_img}" alt="{cgi.escape(str(ch_h_title))}" style="max-width: 100%; max-height: 100%; object-fit: contain;">
				</a>
				<div class="flex-grow-1 overflow-hidden">
					<div class="d-flex align-items-center gap-2 mb-1">
						<span class="badge bg-secondary-subtle text-secondary border border-secondary-subtle">{cgi.escape(str(ch_h_cat))}</span>
						<span class="small text-muted font-monospace">Multi: 0x{getattr(char_house, 'serial', 0):x}</span>
					</div>
					<h5 class="fw-bold text-dark mb-0">{cgi.escape(str(h_title))}</h5>
					<small class="text-secondary">Modelo: <strong>{cgi.escape(str(ch_h_title))}</strong></small>
				</div>
				<div class="text-md-end">
					{pub_badge}
				</div>
			</div>

			<div class="row g-3">
				<div class="col-12 col-md-6">
					<div class="text-secondary small">Localização / Mundo:</div>
					<div class="fw-semibold text-dark">{h_map} <span class="font-monospace text-muted small">({h_coords})</span></div>
				</div>
				<div class="col-12 col-md-6">
					<div class="text-secondary small">Acesso & Moradores:</div>
					<div>
						<span class="small text-secondary me-3">Co-proprietários: <strong>{len(coowner_list)}</strong></span>
						<span class="small text-secondary me-3">Amigos: <strong>{len(friends_list)}</strong></span>
						<span class="small text-secondary">Banidos: <strong>{len(bans_list)}</strong></span>
					</div>
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

<!-- Linha 2: Paperdoll (Gump 2001) e Status Gump Oficial (Gump 10860) -->
<div class="row g-4 mb-4">
	<!-- Coluna 1: Boneco e Paperdoll (Gump 2001) -->
	<div class="col-12 col-xl-4">
		<div class="card border border-light-subtle shadow-sm h-100">
			<div class="card-header bg-white py-2 d-flex justify-content-between align-items-center">
				<span class="fw-semibold text-secondary small text-uppercase">&#129497; Boneco & Paperdoll</span>
				<span class="badge bg-secondary-subtle text-dark border font-monospace">{char_race} &bull; {char_gender}</span>
			</div>
			<div class="card-body text-center p-2">
				<div class="d-inline-block p-1 bg-dark rounded shadow-sm mb-2" style="max-width: 270px; position: relative;">
					<img src="{paperdoll_url}" alt="Paperdoll {cgi.escape(str(char_name))}" style="width: 262px; height: 324px; image-rendering: pixelated; display: block;" />
					<!-- Nome do Personagem no Paperdoll -->
					<div style="position: absolute; left: 15%; right: 50px; bottom: 40px; height: 22px; line-height: 22px; text-align: left; color: #fffffa; font-family: 'Consolas', 'Courier New', monospace; font-size: 13px; font-weight: bold; text-shadow: 1px 1px 2px #000, -1px -1px 2px #000, 1px -1px 2px #000, -1px 1px 2px #000; pointer-events: none; white-space: nowrap; overflow: hidden; text-overflow: ellipsis;">
						{cgi.escape(str(char_name))}
					</div>
					<!-- Título do Personagem no Paperdoll (Logo Abaixo do Nome) -->
					{f'''<div style="position: absolute; left: 15%; right: 50px; bottom: 20px; height: 18px; line-height: 18px; text-align: left; color: #eedcb2; font-family: 'Consolas', 'Courier New', monospace; font-size: 11px; font-weight: normal; text-shadow: 1px 1px 2px #000; pointer-events: none; white-space: nowrap; overflow: hidden; text-overflow: ellipsis;">{cgi.escape(str(char_title))}</div>''' if char_title else ''}
				</div>
				<div class="d-flex justify-content-center gap-2 mb-3">
					<span class="badge bg-light text-dark border">{char_race}</span>
					<span class="badge bg-light text-dark border">{char_gender}</span>
					<span class="badge bg-light text-secondary border font-monospace">Skin: 0x{char_skin_color:04x}</span>
				</div>

				<!-- Equipamentos em Uso (Acordeão) -->
				<div class="accordion accordion-flush text-start border rounded" id="accordionEquipment">
					<div class="accordion-item">
						<h2 class="accordion-header" id="headingEq">
							<button class="accordion-button collapsed py-2 px-3 small fw-semibold text-dark" type="button" data-bs-toggle="collapse" data-bs-target="#collapseEq" aria-expanded="false" aria-controls="collapseEq">
								&#128087; Equipamentos Vestidos ({len(equipped_rows)} itens equipados &bull; {len(PAPERDOLL_LAYERS)} camadas disponíveis)
							</button>
						</h2>
						<div id="collapseEq" class="accordion-collapse collapse" aria-labelledby="headingEq" data-bs-parent="#accordionEquipment">
							<div class="accordion-body p-0">
								<ul class="list-group list-group-flush overflow-auto" style="max-height: 380px;">
									{''.join(equipped_rows)}
								</ul>
							</div>
						</div>
					</div>
				</div>
			</div>
		</div>
	</div>

	<!-- Coluna 2: Gump de Status Oficial (Gump 10860) com Todos os Atributos -->
	<div class="col-12 col-xl-8">
		<div class="card border border-light-subtle shadow-sm h-100">
			<div class="card-header bg-white py-2 d-flex justify-content-between align-items-center">
				<span class="fw-semibold text-secondary small text-uppercase">&#128737; Gump de Status Oficial do Jogo</span>
				<span class="badge bg-light text-secondary border font-monospace">Gump 10860</span>
			</div>
			<div class="card-body p-3 d-flex flex-column align-items-center">
				<!-- Gump 10860 Stone Status Window (577x216) -->
				<div class="table-responsive w-100 d-flex justify-content-center py-1" style="overflow-x: auto;">
					<div style="position: relative; width: 577px; height: 216px; background: url('images/gumps/gump_10860_0.png') no-repeat center center; user-select: none; font-family: 'Consolas', 'Courier New', monospace; font-size: 11px; font-weight: bold; flex-shrink: 0;">
						<!-- Linha 0 (y=71px): STR, DEX, INT, Stat Cap, HP, Stam -->
						<div style="position: absolute; left: 72px; top: 71px; width: 50px; text-align: center; color: #000000; font-weight: bold;" title="Força (STR)">{str_val}</div>
						<div style="position: absolute; left: 154px; top: 71px; width: 50px; text-align: center; color: #000000; font-weight: bold;" title="Destreza (DEX)">{dex_val}</div>
						<div style="position: absolute; left: 236px; top: 71px; width: 50px; text-align: center; color: #000000; font-weight: bold;" title="Inteligência (INT)">{int_val}</div>
						<div style="position: absolute; left: 318px; top: 71px; width: 50px; text-align: center; color: #000000; font-weight: bold;" title="Limite Total de Atributos (Stat Cap)">{statcap}</div>
						<div style="position: absolute; left: 400px; top: 71px; width: 50px; text-align: center; color: #000000; font-weight: bold; font-size: 10px;" title="Pontos de Vida (HP Cur/Max)">{hp_cur}/{hp_max}</div>
						<div style="position: absolute; left: 482px; top: 71px; width: 50px; text-align: center; color: #000000; font-weight: bold; font-size: 10px;" title="Vigor (Stamina Cur/Max)">{stam_cur}/{stam_max}</div>

						<!-- Linha 1 (y=99px): Mana, Followers (Montarias), Peso, Ouro, Res Fís, Res Fogo -->
						<div style="position: absolute; left: 72px; top: 99px; width: 50px; text-align: center; color: #000000; font-weight: bold; font-size: 10px;" title="Mana (Cur/Max)">{mana_cur}/{mana_max}</div>
						<div style="position: absolute; left: 154px; top: 99px; width: 50px; text-align: center; color: #000000; font-weight: bold;" title="Montarias e Seguidores (Cur/Max)">{followers_cur}/{followers_max}</div>
						<div style="position: absolute; left: 236px; top: 99px; width: 50px; text-align: center; color: #000000; font-weight: bold; font-size: 10px;" title="Carga / Peso (Cur/Max)">{weight_cur}/{weight_max}</div>
						<div style="position: absolute; left: 318px; top: 99px; width: 50px; text-align: center; color: #000000; font-weight: bold; font-size: 10px;" title="Ouro na Mochila">{char_gold:,}</div>
						<div style="position: absolute; left: 400px; top: 99px; width: 50px; text-align: center; color: #000000; font-weight: bold;" title="Resistência Física / Máximo">{armor_val}/{res_phys_max}</div>
						<div style="position: absolute; left: 482px; top: 99px; width: 50px; text-align: center; color: #000000; font-weight: bold;" title="Resistência a Fogo / Máximo">{res_fire}/{res_fire_max}</div>

						<!-- Linha 2 (y=127px): Res Frio, Res Veneno, Res Energia, Sorte, Dano, Dízimo -->
						<div style="position: absolute; left: 72px; top: 127px; width: 50px; text-align: center; color: #000000; font-weight: bold;" title="Resistência a Frio / Máximo">{res_cold}/{res_cold_max}</div>
						<div style="position: absolute; left: 154px; top: 127px; width: 50px; text-align: center; color: #000000; font-weight: bold;" title="Resistência a Veneno / Máximo">{res_poison}/{res_poison_max}</div>
						<div style="position: absolute; left: 236px; top: 127px; width: 50px; text-align: center; color: #000000; font-weight: bold;" title="Resistência a Energia / Máximo">{res_energy}/{res_energy_max}</div>
						<div style="position: absolute; left: 318px; top: 127px; width: 50px; text-align: center; color: #000000; font-weight: bold;" title="Sorte (Luck)">{luck_val}</div>
						<div style="position: absolute; left: 400px; top: 127px; width: 50px; text-align: center; color: #000000; font-weight: bold; font-size: 10px;" title="Dano de Arma (Min - Max)">{dmg_min}-{dmg_max}</div>
						<div style="position: absolute; left: 482px; top: 127px; width: 50px; text-align: center; color: #000000; font-weight: bold; font-size: 10px;" title="Pontos de Dízimo (Tithing Points)">{tithing_val:,}</div>

						<!-- Linha 3 (y=155px): Propriedades de Combate e Conjuração -->
						<div style="position: absolute; left: 72px; top: 155px; width: 50px; text-align: center; color: #000000; font-weight: bold;" title="Aumento de Chance de Acerto (Hit Chance Increase)">{hci_val}%</div>
						<div style="position: absolute; left: 154px; top: 155px; width: 50px; text-align: center; color: #000000; font-weight: bold;" title="Aumento de Chance de Defesa (Defense Chance Increase)">{dci_val}%</div>
						<div style="position: absolute; left: 236px; top: 155px; width: 50px; text-align: center; color: #000000; font-weight: bold;" title="Redução de Custo de Mana (Lower Mana Cost)">{lmc_val}%</div>
						<div style="position: absolute; left: 318px; top: 155px; width: 50px; text-align: center; color: #000000; font-weight: bold;" title="Redução de Custo de Reagentes (Lower Reagent Cost)">{lrc_val}%</div>
						<div style="position: absolute; left: 400px; top: 155px; width: 50px; text-align: center; color: #000000; font-weight: bold;" title="Velocidade de Conjuração (Faster Casting)">{fc_val}</div>
						<div style="position: absolute; left: 482px; top: 155px; width: 50px; text-align: center; color: #000000; font-weight: bold;" title="Recuperação de Conjuração (Faster Cast Recovery)">{fcr_val}</div>
					</div>
				</div>

				<!-- Barras de Vigor e Medidores -->
				<div class="w-100 px-3 mt-3 pt-2 border-top">
					<div class="row g-2">
						<div class="col-12 col-md-4">
							<div class="d-flex justify-content-between small text-secondary mb-1">
								<span class="fw-semibold text-danger">&#10084; Vida (HP):</span>
								<span class="font-monospace fw-bold text-dark">{hp_cur} / {hp_max}</span>
							</div>
							<div class="progress" style="height: 7px;">
								<div class="progress-bar bg-danger" role="progressbar" style="width: {hp_pct}%;"></div>
							</div>
						</div>
						<div class="col-12 col-md-4">
							<div class="d-flex justify-content-between small text-secondary mb-1">
								<span class="fw-semibold text-warning">&#9889; Vigor (Stamina):</span>
								<span class="font-monospace fw-bold text-dark">{stam_cur} / {stam_max}</span>
							</div>
							<div class="progress" style="height: 7px;">
								<div class="progress-bar bg-warning" role="progressbar" style="width: {stam_pct}%;"></div>
							</div>
						</div>
						<div class="col-12 col-md-4">
							<div class="d-flex justify-content-between small text-secondary mb-1">
								<span class="fw-semibold text-primary">&#128302; Mana:</span>
								<span class="font-monospace fw-bold text-dark">{mana_cur} / {mana_max}</span>
							</div>
							<div class="progress" style="height: 7px;">
								<div class="progress-bar bg-primary" role="progressbar" style="width: {mana_pct}%;"></div>
							</div>
						</div>
					</div>
				</div>
			</div>
		</div>
	</div>
</div>

<!-- Linha 3: Pergaminho de Perícias (Skills) e Banco / Propriedades -->
<div class="row g-4 mb-4">
	<!-- Coluna Esquerda: Pergaminho Oficial de Perícias -->
	<div class="col-12 col-xl-4">
		<div class="card border border-light-subtle shadow-sm h-100">
			<div class="card-header bg-white py-2 d-flex justify-content-between align-items-center">
				<span class="fw-semibold text-secondary small text-uppercase">&#128220; Pergaminho de Perícias</span>
				<span class="badge bg-dark-subtle text-dark font-monospace">{total_skills:.1f} / 700.0%</span>
			</div>
			<div class="card-body p-2 d-flex flex-column align-items-center">
				<!-- Botões de Filtro -->
				<div class="btn-group btn-group-sm mb-2 w-100" role="group">
					<button type="button" class="btn btn-secondary active py-1" id="btn-filter-all" onclick="toggleSkillFilter('all')">Todas as Perícias ({len(skill_cards_all)})</button>
					<button type="button" class="btn btn-outline-secondary py-1" id="btn-filter-learned" onclick="toggleSkillFilter('learned')">Aprendidas ({len(skill_cards_learned)})</button>
				</div>

				<!-- Estrutura Gráfica do Pergaminho UO -->
				<div style="width: 100%; max-width: 300px; margin: 0 auto; user-select: none;">
					<!-- Rolo Superior (Gump 2080) -->
					<img src="images/gumps/gump_2080_0.png" style="display: block; width: 300px; height: 37px; margin: 0 auto;" alt="Topo do Pergaminho" />

					<!-- Corpo Central de Pergaminho Repetido (Gump 2081) -->
					<div id="skills-scroll-container" style="width: 263px; margin: 0 auto; background: #e3cca6 url('images/gumps/gump_2081_0.png') repeat-y center top; max-height: 380px; overflow-y: auto; padding: 6px 10px; box-shadow: inset 0 0 10px rgba(0,0,0,0.18); border-left: 1px solid #b79c73; border-right: 1px solid #b79c73;">
						{''.join(scroll_rows)}
						<div id="no-learned-skills-msg" class="d-none text-center py-4 px-2" style="color: #5c3a21; font-family: 'Cinzel', 'Georgia', serif; font-size: 12px; font-style: italic;">
							Nenhuma perícia aprendida (&gt; 0.0%) neste personagem.
						</div>
					</div>

					<!-- Rolo Inferior (Gump 2083) -->
					<img src="images/gumps/gump_2083_0.png" style="display: block; width: 273px; height: 34px; margin: 0 auto;" alt="Fundo do Pergaminho" />
				</div>

				<script>
				function toggleSkillFilter(mode) {{
					var rows = document.querySelectorAll('#skills-scroll-container .skill-scroll-row');
					var bAll = document.getElementById('btn-filter-all');
					var bLearned = document.getElementById('btn-filter-learned');
					var noMsg = document.getElementById('no-learned-skills-msg');

					if (mode === 'learned') {{
						bLearned.classList.add('active', 'btn-secondary');
						bLearned.classList.remove('btn-outline-secondary');
						bAll.classList.remove('active', 'btn-secondary');
						bAll.classList.add('btn-outline-secondary');

						var visibleCount = 0;
						rows.forEach(function(r) {{
							if (r.getAttribute('data-learned') === '1') {{
								r.classList.remove('d-none');
								r.style.setProperty('display', 'flex', 'important');
								visibleCount++;
							}} else {{
								r.classList.add('d-none');
								r.style.setProperty('display', 'none', 'important');
							}}
						}});

						if (noMsg) {{
							if (visibleCount === 0) {{
								noMsg.classList.remove('d-none');
								noMsg.style.setProperty('display', 'block', 'important');
							}} else {{
								noMsg.classList.add('d-none');
								noMsg.style.setProperty('display', 'none', 'important');
							}}
						}}
					}} else {{
						bAll.classList.add('active', 'btn-secondary');
						bAll.classList.remove('btn-outline-secondary');
						bLearned.classList.remove('active', 'btn-secondary');
						bLearned.classList.add('btn-outline-secondary');

						rows.forEach(function(r) {{
							r.classList.remove('d-none');
							r.style.setProperty('display', 'flex', 'important');
						}});

						if (noMsg) {{
							noMsg.classList.add('d-none');
							noMsg.style.setProperty('display', 'none', 'important');
						}}
					}}
				}}
				</script>
			</div>
		</div>
	</div>

	<!-- Coluna Direita: Cofre Bancário e Propriedades -->
	<div class="col-12 col-xl-8">
		<!-- Propriedades / Casas -->
		{house_section}

		<!-- Cofre Bancário (Itens Guardados no Banco) -->
		<div class="card border border-light-subtle shadow-sm mb-4">
			<div class="card-header bg-white py-3 d-flex justify-content-between align-items-center">
				<div>
					<h6 class="fw-bold text-dark mb-0">&#128176; Cofre Bancário (Bank Box)</h6>
					<small class="text-muted">Total de Itens: <strong>{bank_items_count}</strong> &bull; Saldo de Ouro: <strong class="text-success">{bank_gold:,} gps</strong></small>
				</div>
				<span class="badge bg-success-subtle text-success border border-success">Seguro na Cidade</span>
			</div>
			<div class="table-responsive" style="max-height: 340px; overflow-y: auto;">
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
	</div>
</div>

<!-- Seção 4: Grade Expandida de Habilidades (Skills) -->
<div class="card border border-light-subtle shadow-sm mb-4">
	<div class="card-header bg-white py-3 d-flex flex-wrap justify-content-between align-items-center gap-2">
		<div>
			<h6 class="fw-bold text-dark mb-0">&#128218; Grade Completa de Habilidades (Skills)</h6>
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