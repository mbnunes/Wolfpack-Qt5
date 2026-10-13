# Detailed Character Sheet - Paperdoll, Status e Skills

import web.sessions
import web.template
import cgi
import sys
import wolfpack
import wolfpack.accounts

# Importações de Perícias do Wolfpack
try:
	from wolfpack.consts import ALLSKILLS
	from skills import SKILLS, SKILL_NAME
except Exception:
	ALLSKILLS = 55
	SKILLS = {}
	SKILL_NAME = 1

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
			is_online = True
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
			<a href="accounts.py?session={session_id}" class="btn btn-outline-dark btn-sm">&larr; Voltar para Contas</a>
		</div>
	</div>
	"""
	web.template.output("Personagem", username, 1, content, session_id)
	sys.exit()

# Dados Básicos
char_name = getattr(char, 'name', 'Sem Nome')
char_title = getattr(char, 'title', '')
acc_name = getattr(getattr(char, 'account', None), 'name', account_user)

# Atributos (Stats)
str_val = getattr(char, 'strength', 0)
dex_val = getattr(char, 'dexterity', 0)
int_val = getattr(char, 'intelligence', 0)

hp_cur = getattr(char, 'hitpoints', str_val)
hp_max = getattr(char, 'maxhitpoints', str_val)
if hp_max <= 0: hp_max = max(1, str_val)
hp_pct = max(0, min(100, int((hp_cur / hp_max) * 100)))

stam_cur = getattr(char, 'stamina', dex_val)
stam_max = getattr(char, 'maxstamina', dex_val)
if stam_max <= 0: stam_max = max(1, dex_val)
stam_pct = max(0, min(100, int((stam_cur / stam_max) * 100)))

mana_cur = getattr(char, 'mana', int_val)
mana_max = getattr(char, 'maxmana', int_val)
if mana_max <= 0: mana_max = max(1, int_val)
mana_pct = max(0, min(100, int((mana_cur / mana_max) * 100)))

# Equipamentos (Paperdoll)
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
		if not item_name:
			item_id = getattr(item, 'id', 0)
			item_name = f"Item 0x{item_id:04x}"
		equipped_rows.append(f"""
		<li class="list-group-item d-flex justify-content-between align-items-center py-2 px-3 bg-light border-light-subtle">
			<span class="small fw-semibold text-secondary">{layer_name}:</span>
			<span class="small fw-bold text-dark">{cgi.escape(str(item_name))}</span>
		</li>
		""")

if not equipped_rows:
	equipped_rows.append('<li class="list-group-item text-muted small py-3 text-center border-0">Nenhum equipamento equipado.</li>')

# Processamento de Skills
skill_cards = []
total_skills = 0.0

LOCK_ICONS = {0: '&#9650;', 1: '&#9660;', 2: '&#128274;'}

for i in range(0, ALLSKILLS):
	try:
		raw_val = char.skill[i]
		val = raw_val / 10.0
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

	# Mostra perícias que têm algum valor aprendido
	if val > 0.0:
		s_name = SKILLS.get(i, [None, f"Skill #{i}"])[SKILL_NAME] if i in SKILLS else f"Skill #{i}"
		pct = min(100, int((val / max(1.0, cap)) * 100))
		lock_badge = LOCK_ICONS.get(lock, '&#9650;')

		skill_cards.append(f"""
		<div class="col-12 col-md-6 col-xl-4">
			<div class="p-2 border rounded bg-light-subtle">
				<div class="d-flex justify-content-between align-items-center mb-1">
					<span class="small fw-semibold text-dark">{s_name}</span>
					<span class="small text-secondary font-monospace">
						{val:.1f} / {cap:.1f}% 
						<span class="ms-1" title="Lock">{lock_badge}</span>
					</span>
				</div>
				<div class="progress" style="height: 5px;">
					<div class="progress-bar bg-success" role="progressbar" style="width: {pct}%;"></div>
				</div>
			</div>
		</div>
		""")

status_badge = '<span class="badge bg-success-subtle text-success border border-success">Online no Jogo</span>' if is_online else '<span class="badge bg-secondary-subtle text-secondary border border-secondary-subtle">Offline</span>'

content = f"""
<!-- Cabeçalho -->
<div class="d-flex flex-wrap justify-content-between align-items-center pb-2 mb-3 border-bottom gap-2">
	<div>
		<div class="d-flex align-items-center gap-2">
			<h4 class="fw-bold text-dark mb-0">{cgi.escape(str(char_name))}</h4>
			{status_badge}
			<span class="badge bg-secondary-subtle text-dark border font-monospace">0x{serial_int:x}</span>
		</div>
		<p class="text-secondary small mb-0">{cgi.escape(str(char_title)) if char_title else 'Aventureiro de Britannia'} &bull; Conta: <strong>{cgi.escape(str(acc_name))}</strong></p>
	</div>
	<div>
		<a href="account.py?session={session_id}&username={cgi.escape(str(acc_name))}" class="btn btn-outline-secondary btn-sm">
			&larr; Voltar para a Conta
		</a>
	</div>
</div>

<!-- Linha 1: Equipamentos e Status Vital -->
<div class="row g-4 mb-4">
	<!-- Coluna Esquerda: Paperdoll -->
	<div class="col-12 col-lg-6">
		<div class="card border border-light-subtle shadow-sm h-100">
			<div class="card-header bg-white py-2 d-flex justify-content-between align-items-center">
				<span class="fw-semibold text-secondary small text-uppercase">Equipamentos (Paperdoll)</span>
				<span class="badge bg-dark-subtle text-dark font-monospace">{len(equipped_rows)} itens</span>
			</div>
			<ul class="list-group list-group-flush overflow-auto" style="max-height: 480px;">
				{''.join(equipped_rows)}
			</ul>
		</div>
	</div>

	<!-- Coluna Direita: Status -->
	<div class="col-12 col-lg-6">
		<div class="card border border-light-subtle shadow-sm h-100">
			<div class="card-header bg-white py-2">
				<span class="fw-semibold text-secondary small text-uppercase">Status Vital</span>
			</div>
			<div class="card-body">
				<div class="mb-3">
					<div class="d-flex justify-content-between small text-secondary mb-1">
						<span class="fw-semibold">Hit Points (HP):</span>
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
						<span class="text-secondary small d-block">FORÇA</span>
						<span class="fs-5 fw-bold text-dark">{str_val}</span>
					</div>
					<div class="col-4">
						<span class="text-secondary small d-block">DESTREZA</span>
						<span class="fs-5 fw-bold text-dark">{dex_val}</span>
					</div>
					<div class="col-4">
						<span class="text-secondary small d-block">INTELIGÊNCIA</span>
						<span class="fs-5 fw-bold text-dark">{int_val}</span>
					</div>
				</div>
			</div>
		</div>
	</div>
</div>

<!-- Linha 2: Grade de Habilidades (Skills) -->
<div class="card border border-light-subtle shadow-sm">
	<div class="card-header bg-white py-3 d-flex justify-content-between align-items-center">
		<h6 class="fw-bold text-dark mb-0">&#128218; Habilidades (Skills)</h6>
		<span class="badge bg-success-subtle text-success border border-success font-monospace">
			Total: {total_skills:.1f}%
		</span>
	</div>
	<div class="card-body">
		<div class="row g-2">
			{''.join(skill_cards) if skill_cards else '<div class="col-12 text-muted small py-3 text-center">Nenhuma habilidade com pontos aprendida neste personagem.</div>'}
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