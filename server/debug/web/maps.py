# Displays Players on a Map - Hub

import web.sessions
import web.template
import cgi
import sys

form = cgi.FieldStorage()
session_id = form.getvalue('session', '')

web.sessions.check_timeouts()
session = web.sessions.get_session(session_id)

if session is None:
	web.sessions.display_timeout()
	sys.exit()

username = session['username']
user_acc = None
try:
	import wolfpack.accounts
	user_acc = wolfpack.accounts.find(username)
except Exception:
	pass

if not user_acc or not user_acc.authorized('RemoteAccess', 'login'):
	content = f"""
	<div class="alert alert-danger border shadow-sm my-4">
		<h5 class="alert-heading fw-bold">&#128683; Acesso Negado</h5>
		<p class="mb-0">O rastreamento de jogadores nos mapas é restrito a administradores.</p>
		<div class="mt-3">
			<a href="main.py?session={session_id}&view=player" class="btn btn-outline-danger btn-sm">&larr; Voltar ao Meu Painel</a>
		</div>
	</div>
	"""
	web.template.output("Acesso Negado", username, 1, content, session_id)
	sys.exit()

maps_list = [
	("Trammel", "map_trammel.py", "images/map_trammel.jpg", "Faseta pacífica de Britannia"),
	("Felucca", "map_felucca.py", "images/map_trammel.jpg", "Faseta de PvP e regras clássicas"),
	("Malas", "map_malas.py", "images/map_malas.jpg", "Continente sombrio de Luna e Umbra"),
	("Ilshenar", "map_ilshenar.py", "images/map_ilshenar.jpg", "Mundo antigo e masmorras sagradas"),
	("Ter Mur", "map_ter_mur.py", "images/map_trammel.jpg", "Reino dos Gargoyles"),
	("Tokuno", "map_tokuno.py", "images/map_tokuno.jpg", "Ilhas e cultura oriental"),
]

map_cards = []
for name, script, thumb, desc in maps_list:
	map_cards.append(f"""
	<div class="col-12 col-md-6 col-lg-4">
		<div class="card h-100 border border-light-subtle shadow-sm overflow-hidden">
			<div style="height: 120px; background-image: url('{thumb}'); background-size: cover; background-position: center;"></div>
			<div class="card-body d-flex flex-column">
				<h5 class="card-title fw-bold text-dark">{name}</h5>
				<p class="card-text text-secondary small flex-grow-1">{desc}</p>
				<a href="{script}?session={session_id}" class="btn btn-outline-success btn-sm w-100 mt-2">
					Abrir Mapa &rarr;
				</a>
			</div>
		</div>
	</div>
	""")

content = f"""
<div class="d-flex justify-content-between align-items-center pb-2 mb-4 border-bottom">
	<div>
		<h4 class="fw-bold text-dark mb-0">Mapas com Jogadores</h4>
		<p class="text-secondary small mb-0">Selecione uma região para visualizar as posições em tempo real.</p>
	</div>
</div>

<div class="row g-3">
	{''.join(map_cards)}
</div>
"""

web.template.output( 	
	f'&gt; <a href="maps.py?session={session_id}" class="text-decoration-none text-success fw-semibold">Maps with Players</a>', 
	username, 
	1, 
	content,
	session_id
)