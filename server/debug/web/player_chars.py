# Player Characters List View

import web.sessions
import web.template
import cgi
import sys
import wolfpack
import wolfpack.accounts
import urllib.parse

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

web.sessions.check_timeouts()
session = web.sessions.get_session(session_id)

if session is None:
	web.sessions.display_timeout()
	sys.exit()

username = session['username']
user_acc = wolfpack.accounts.find(username)

chars = getattr(user_acc, 'characters', []) if user_acc else []

char_rows = []
for char in chars:
	c_name = getattr(char, 'name', 'Sem Nome')
	c_title = getattr(char, 'title', '')
	c_serial = getattr(char, 'serial', 0)
	is_online = bool(getattr(char, 'socket', None))

	s_str = getattr(char, 'strength', 0)
	s_dex = getattr(char, 'dexterity', 0)
	s_int = getattr(char, 'intelligence', 0)

	b_gold = 0
	try:
		bank = char.getbankbox()
		if bank:
			b_gold = bank.countresource(0xEED)
	except Exception:
		b_gold = 0

	c_gold = 0
	try:
		c_gold = char.countresource(0xEED)
	except Exception:
		c_gold = 0

	total_skill = 0.0
	for i in range(55):
		try:
			total_skill += (char.skill[i] / 10.0)
		except Exception:
			pass

	c_pos = getattr(char, 'pos', None)
	map_id = c_pos.map if c_pos else 0
	map_name = MAP_NAMES.get(map_id, f"Map {map_id}")

	status_badge = '<span class="badge bg-success-subtle text-success border border-success">Online</span>' if is_online else '<span class="badge bg-secondary-subtle text-secondary border">Offline</span>'

	char_rows.append(f"""
	<tr>
		<td class="ps-3">
			<div class="fw-bold text-dark">{cgi.escape(str(c_name))}</div>
			<div class="small text-muted">{cgi.escape(str(c_title)) if c_title else 'Aventureiro'}</div>
		</td>
		<td>{status_badge}</td>
		<td class="font-monospace small text-secondary">0x{c_serial:x}</td>
		<td>
			<span class="badge bg-light text-dark border">STR {s_str}</span>
			<span class="badge bg-light text-dark border">DEX {s_dex}</span>
			<span class="badge bg-light text-dark border">INT {s_int}</span>
		</td>
		<td class="font-monospace text-success fw-semibold">{b_gold:,} gps</td>
		<td class="font-monospace text-dark">{total_skill:.1f}%</td>
		<td class="small text-secondary">{map_name}</td>
		<td class="text-end pe-3">
			<a href="character.py?session={session_id}&serial=0x{c_serial:x}&account={urllib.parse.quote(username)}" class="btn btn-sm btn-outline-success">
				Ver Ficha &rarr;
			</a>
		</td>
	</tr>
	""")

content = f"""
<div class="d-flex flex-wrap justify-content-between align-items-center pb-2 mb-4 border-bottom gap-2">
	<div>
		<h4 class="fw-bold text-dark mb-0">&#128101; Meus Personagens</h4>
		<p class="text-secondary small mb-0">Listagem de heróis vinculados à sua conta <strong>{username}</strong>.</p>
	</div>
	<div>
		<a href="main.py?session={session_id}&view=player" class="btn btn-outline-secondary btn-sm">
			&larr; Voltar ao Painel
		</a>
	</div>
</div>

<div class="card border border-light-subtle shadow-sm mb-4">
	<div class="card-header bg-white py-3">
		<h6 class="fw-bold text-dark mb-0">Personagens Registrados ({len(chars)} / 5)</h6>
	</div>
	<div class="table-responsive">
		<table class="table table-hover table-striped align-middle mb-0">
			<thead class="table-light small text-uppercase text-secondary">
				<tr>
					<th scope="col" class="ps-3">Nome / Título</th>
					<th scope="col">Status</th>
					<th scope="col">Serial</th>
					<th scope="col">Atributos</th>
					<th scope="col">Ouro no Banco</th>
					<th scope="col">Perícias</th>
					<th scope="col">Localização</th>
					<th scope="col" class="text-end pe-3">Ação</th>
				</tr>
			</thead>
			<tbody>
				{''.join(char_rows) if char_rows else '<tr><td colspan="8" class="text-center py-4 text-muted">Nenhum personagem registrado nesta conta.</td></tr>'}
			</tbody>
		</table>
	</div>
</div>
"""

web.template.output(
	f'&gt; <a href="player_chars.py?session={session_id}" class="text-decoration-none text-success fw-semibold">Meus Personagens</a>',
	username,
	1,
	content,
	session_id
)
