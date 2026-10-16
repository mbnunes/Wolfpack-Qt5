# Detailed Housing / Properties Management View

import web.sessions
import web.template
import cgi
import sys
import wolfpack
import wolfpack.accounts
import urllib.parse

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
show_all = form.getvalue('all', '0')

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

# Coleta de Casas
all_houses = []
user_char_serials = set()

if user_acc:
	for c in getattr(user_acc, 'characters', []):
		user_char_serials.add(getattr(c, 'serial', 0))

if housing and hasattr(housing, 'getHouses'):
	try:
		h_dict = housing.getHouses()
		for owner_serial, house_serial in h_dict.items():
			house_item = wolfpack.finditem(house_serial)
			if not house_item:
				continue

			# Se for admin e pediu para ver tudo, ou se o owner for um char do usuário
			if (is_admin and show_all == '1') or (owner_serial in user_char_serials):
				owner_char = wolfpack.findchar(owner_serial)
				all_houses.append((owner_char, house_item))
	except Exception:
		pass

# Monta Cards de Propriedades
cards_html = []
for owner_char, house in all_houses:
	h_serial = getattr(house, 'serial', 0)
	h_name = getattr(house, 'name', 'Residência')
	if hasattr(house, 'hastag') and house.hastag('sign'):
		sign_item = wolfpack.finditem(int(house.gettag('sign')))
		if sign_item and getattr(sign_item, 'name', ''):
			h_name = sign_item.name

	h_pos = getattr(house, 'pos', None)
	map_id = h_pos.map if h_pos else 0
	map_name = MAP_NAMES.get(map_id, f"Map {map_id}")
	coords_str = f"{h_pos.x}, {h_pos.y}, {h_pos.z}" if h_pos else "N/A"

	owner_name = getattr(owner_char, 'name', 'Desconhecido') if owner_char else f"Serial 0x{getattr(house, 'owner', 0):x}"

	# Regras de Housing
	is_pub = False
	coowners = []
	friends = []
	guests = []
	bans = []

	if housing and hasattr(housing, 'house'):
		try:
			is_pub = housing.house.isPublic(house)
			coowners = housing.house.getCoOwners(house)
			friends = housing.house.getFriends(house)
			guests = housing.house.getGuests(house)
			bans = housing.house.getBans(house)
		except Exception:
			pass

	sec_badge = '<span class="badge bg-success-subtle text-success border border-success">Pública</span>' if is_pub else '<span class="badge bg-secondary-subtle text-secondary border">Privada</span>'

	# Resolve nomes dos coowners
	coowner_names = []
	for s in coowners:
		try:
			ch = wolfpack.findchar(int(s))
			coowner_names.append(ch.name if ch else f"0x{int(s):x}")
		except Exception:
			coowner_names.append(s)

	# Resolve nomes dos amigos
	friend_names = []
	for s in friends:
		try:
			ch = wolfpack.findchar(int(s))
			friend_names.append(ch.name if ch else f"0x{int(s):x}")
		except Exception:
			friend_names.append(s)

	cards_html.append(f"""
	<div class="col-12 col-lg-6">
		<div class="card h-100 border border-light-subtle shadow-sm">
			<div class="card-header bg-white py-3 d-flex justify-content-between align-items-center">
				<div>
					<h5 class="fw-bold text-dark mb-0">{cgi.escape(str(h_name))}</h5>
					<small class="text-secondary">Proprietário: <strong>{cgi.escape(str(owner_name))}</strong></small>
				</div>
				<div class="text-end">
					{sec_badge}
					<div class="font-monospace small text-muted mt-1">Multi: 0x{h_serial:x}</div>
				</div>
			</div>
			<div class="card-body">
				<div class="row g-2 mb-3">
					<div class="col-6">
						<span class="text-secondary small d-block">Mundo / Mapa:</span>
						<strong class="text-dark">{map_name}</strong>
					</div>
					<div class="col-6">
						<span class="text-secondary small d-block">Coordenadas:</span>
						<span class="font-monospace text-dark small">{coords_str}</span>
					</div>
				</div>

				<div class="p-3 bg-light rounded mb-3">
					<div class="d-flex justify-content-between align-items-center mb-2 pb-1 border-bottom">
						<span class="small fw-semibold text-secondary">Co-proprietários ({len(coowners)}):</span>
						<span class="small text-dark">{cgi.escape(', '.join(coowner_names)) if coowner_names else 'Nenhum'}</span>
					</div>
					<div class="d-flex justify-content-between align-items-center mb-2 pb-1 border-bottom">
						<span class="small fw-semibold text-secondary">Amigos da Residência ({len(friends)}):</span>
						<span class="small text-dark">{cgi.escape(', '.join(friend_names)) if friend_names else 'Nenhum'}</span>
					</div>
					<div class="d-flex justify-content-between align-items-center">
						<span class="small fw-semibold text-secondary">Personagens Banidos ({len(bans)}):</span>
						<span class="badge bg-danger-subtle text-danger border border-danger-subtle">{len(bans)}</span>
					</div>
				</div>

				<div class="d-flex justify-content-between align-items-center small text-muted">
					<span>Acesso no jogo: Utilize a placa da casa</span>
					{f'<a href="character.py?session={session_id}&serial=0x{getattr(owner_char, "serial", 0):x}" class="btn btn-outline-success btn-sm py-0">Ver Ficha do Dono &rarr;</a>' if owner_char else ''}
				</div>
			</div>
		</div>
	</div>
	""")

admin_toggle_btn = ""
if is_admin:
	if show_all == '1':
		admin_toggle_btn = f'<a href="properties.py?session={session_id}&all=0" class="btn btn-outline-secondary btn-sm">Ver Apenas Minhas Propriedades</a>'
	else:
		admin_toggle_btn = f'<a href="properties.py?session={session_id}&all=1" class="btn btn-outline-warning btn-sm">Ver Todas as Casas do Servidor (Admin)</a>'

content = f"""
<div class="d-flex flex-wrap justify-content-between align-items-center pb-2 mb-4 border-bottom gap-2">
	<div>
		<h4 class="fw-bold text-dark mb-0">&#127960; Propriedades e Residências</h4>
		<p class="text-secondary small mb-0">Listagem de casas, terrenos e estruturas registradas.</p>
	</div>
	<div class="d-flex gap-2">
		{admin_toggle_btn}
		<a href="main.py?session={session_id}&view=player" class="btn btn-outline-secondary btn-sm">
			&larr; Voltar ao Painel
		</a>
	</div>
</div>

<div class="row g-4 mb-4">
	{''.join(cards_html) if cards_html else '<div class="col-12"><div class="alert alert-secondary text-center py-5"><h5 class="fw-bold">Nenhuma Propriedade Encontrada</h5><p class="text-muted small mb-0">Não há casas ou terrenos associados aos seus personagens no momento.</p></div></div>'}
</div>
"""

web.template.output(
	f'&gt; <a href="properties.py?session={session_id}" class="text-decoration-none text-success fw-semibold">Propriedades</a>',
	username,
	1,
	content,
	session_id
)
