# Displays Players on a Map - Ilshenar (Interactive Leaflet Map - Resolução 2304x1600)

import web.sessions
import web.template
import cgi
import sys
import json
import wolfpack
import wolfpack.sockets

form = cgi.FieldStorage()
session_id = form.getvalue('session', '')

web.sessions.check_timeouts()
session = web.sessions.get_session(session_id)

if session is None:
	web.sessions.display_timeout()
	sys.exit()

username = session['username']

# Dimensões da imagem nativa de Ilshenar
IMG_WIDTH = 2304
IMG_HEIGHT = 1600
IMAGE_PATH = "images/map_ilshenar.jpg"

players_data = []

socket = wolfpack.sockets.first()
nextsocket = wolfpack.sockets.next()
while socket:
	# Ilshenar: mapa 2 no Wolfpack
	if socket.player and socket.player.pos.map == 2 and not socket.player.invisible:
		p_name = str(socket.player.name)
		raw_x = socket.player.pos.x
		raw_y = socket.player.pos.y
		
		# Em 2304x1600 a escala é 1:1 com os tiles do UO
		players_data.append({
			"name": p_name,
			"raw_x": raw_x,
			"raw_y": raw_y,
			"px": raw_x,
			"py": raw_y
		})

	socket = nextsocket
	nextsocket = wolfpack.sockets.next()

# Serializa em JSON para o Leaflet
players_json = json.dumps(players_data)

# Lista lateral de jogadores
if players_data:
	player_items = "".join([
		f'''<li class="list-group-item d-flex justify-content-between align-items-center small py-2">
			<a href="#" onclick="focusPlayer({p['px']}, {p['py']}); return false;" class="text-decoration-none fw-semibold text-success">{p['name']}</a>
			<span class="badge bg-secondary-subtle text-dark border font-monospace">X:{p['raw_x']} Y:{p['raw_y']}</span>
		</li>''' for p in players_data
	])
else:
	player_items = '<li class="list-group-item text-muted small py-3 text-center">Nenhum jogador online neste mapa.</li>'

content = f"""
<!-- Folha de estilo do Leaflet -->
<link rel="stylesheet" href="css/leaflet.css" />

<!-- Cabeçalho -->
<div class="d-flex flex-wrap justify-content-between align-items-center pb-2 mb-3 border-bottom gap-2">
	<div>
		<h4 class="fw-bold text-dark mb-0">Mapa Interativo: Ilshenar</h4>
		<p class="text-secondary small mb-0">Resolução nativa 2304x1600. Role para dar zoom e arraste para navegar.</p>
	</div>
	<div>
		<a href="map_ilshenar.py?session={session_id}" class="btn btn-outline-success btn-sm">Atualizar Posições</a>
	</div>
</div>

<!-- Abas de Navegação -->
<ul class="nav nav-pills mb-3 small">
  <li class="nav-item"><a class="nav-link" href="map_trammel.py?session={session_id}">Trammel</a></li>
  <li class="nav-item"><a class="nav-link" href="map_felucca.py?session={session_id}">Felucca</a></li>
  <li class="nav-item"><a class="nav-link active" href="map_ilshenar.py?session={session_id}">Ilshenar</a></li>
  <li class="nav-item"><a class="nav-link" href="map_malas.py?session={session_id}">Malas</a></li>
  <li class="nav-item"><a class="nav-link" href="map_tokuno.py?session={session_id}">Tokuno</a></li>
  <li class="nav-item"><a class="nav-link" href="map_ter_mur.py?session={session_id}">Ter Mur</a></li>
</ul>

<!-- Grid: Mapa Leaflet + Lista Lateral -->
<div class="row g-3">
	<div class="col-12 col-xl-9">
		<div class="card border border-light-subtle shadow-sm p-1 bg-dark">
			<div id="uomap" style="width: 100%; height: 600px; background-color: #0b151e; border-radius: 4px;"></div>
		</div>
	</div>

	<div class="col-12 col-xl-3">
		<div class="card border border-light-subtle shadow-sm h-100">
			<div class="card-header bg-white py-2">
				<span class="fw-semibold text-secondary small text-uppercase">Jogadores ({len(players_data)})</span>
			</div>
			<ul class="list-group list-group-flush overflow-auto" style="max-height: 560px;">
				{player_items}
			</ul>
		</div>
	</div>
</div>

<!-- Script Leaflet -->
<script src="js/leaflet.js"></script>
<script>
	const imgW = {IMG_WIDTH};
	const imgH = {IMG_HEIGHT};
	const imgUrl = '{IMAGE_PATH}';
	const players = {players_json};

	const map = L.map('uomap', {{
		crs: L.CRS.Simple,
		minZoom: -2,
		maxZoom: 2,
		zoomSnap: 0.5
	}});

	const bounds = [[0, 0], [imgH, imgW]];
	L.imageOverlay(imgUrl, bounds).addTo(map);

	map.fitBounds(bounds);

	const playerIcon = L.icon({{
		iconUrl: 'images/marker.gif',
		iconSize: [12, 12],
		iconAnchor: [6, 6],
		popupAnchor: [0, -6]
	}});

	players.forEach(p => {{
		// Inversão do eixo Y no plano cartesiano do Leaflet
		const mapY = imgH - p.py;
		const mapX = p.px;

		const marker = L.marker([mapY, mapX], {{ icon: playerIcon }}).addTo(map);
		marker.bindPopup(`<strong>${{p.name}}</strong><br>X: ${{p.raw_x}} | Y: ${{p.raw_y}}`);
	}});

	window.focusPlayer = function(px, py) {{
		const mapY = imgH - py;
		map.setView([mapY, px], 1);
	}};
</script>
"""

user_badge = f"""
<div class="d-flex justify-content-between align-items-center w-100">
	<div>
		<span class="text-muted">Conectado como:</span> 
		<span class="badge bg-secondary-subtle text-dark border border-secondary-subtle fw-semibold px-2 py-1">{username}</span>
	</div>
	<div>
		<a href="logout.py?session={session_id}" class="text-danger text-decoration-none fw-semibold small">Sair (Logout)</a>
	</div>
</div>
"""

web.template.output( 	
	f'&gt; <a href="maps.py?session={session_id}" class="text-decoration-none text-success">Maps</a> &gt; <a href="map_ilshenar.py?session={session_id}" class="text-decoration-none text-success fw-semibold">Ilshenar</a>', 
	user_badge, 
	1, 
	content,
	session_id
)