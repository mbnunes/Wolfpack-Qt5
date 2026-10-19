# Displays Players on a Map - Ter Mur (Interactive Leaflet Map)

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

# Dimensões da imagem de Ter Mur (640x2048 ou 1280x4096 caso troque pela nativa)
IMG_WIDTH = 640
IMG_HEIGHT = 2048
IMAGE_PATH = "images/map_ter_mur.jpg"

# Dimensões nativas de Ter Mur em tiles no UO
UO_TILES_X = 1280
UO_TILES_Y = 4096

# Fator de conversão entre coordenadas do jogo e pixels da imagem
SCALE_X = IMG_WIDTH / UO_TILES_X
SCALE_Y = IMG_HEIGHT / UO_TILES_Y

players_data = []

socket = wolfpack.sockets.first()
nextsocket = wolfpack.sockets.next()
while socket:
	# Ter Mur: mapa 5 no Wolfpack
	if socket.player and socket.player.pos.map == 5 and not socket.player.invisible:
		p_player = socket.player
		p_name = str(p_player.name)
		raw_x = p_player.pos.x
		raw_y = p_player.pos.y
		
		px = raw_x * SCALE_X
		py = raw_y * SCALE_Y

		p_serial = f"0x{int(p_player.serial):x}"

		# Vitals
		hp = int(getattr(p_player, 'hitpoints', 0))
		max_hp = max(1, int(getattr(p_player, 'maxhitpoints', 1)))
		hp_pct = max(0, min(100, int((hp / max_hp) * 100)))

		mana = int(getattr(p_player, 'mana', 0))
		max_mana = max(1, int(getattr(p_player, 'maxmana', 1)))
		mana_pct = max(0, min(100, int((mana / max_mana) * 100)))

		stam = int(getattr(p_player, 'stamina', 0))
		max_stam = max(1, int(getattr(p_player, 'maxstamina', 1)))
		stam_pct = max(0, min(100, int((stam / max_stam) * 100)))

		players_data.append({
			"name": p_name,
			"serial": p_serial,
			"raw_x": raw_x,
			"raw_y": raw_y,
			"px": px,
			"py": py,
			"hp": hp,
			"max_hp": max_hp,
			"hp_pct": hp_pct,
			"mana": mana,
			"max_mana": max_mana,
			"mana_pct": mana_pct,
			"stam": stam,
			"max_stam": max_stam,
			"stam_pct": stam_pct,
		})

	socket = nextsocket
	nextsocket = wolfpack.sockets.next()

# Serializa em JSON para o Leaflet
players_json = json.dumps(players_data)

# Lista lateral de jogadores
if players_data:
	player_items = "".join([
		f'''<li class="list-group-item d-flex justify-content-between align-items-center small py-2">
			<div class="d-flex align-items-center text-truncate me-2">
				<a href="#" onclick="focusPlayer({p['px']}, {p['py']}, '{p['serial']}'); return false;" class="text-decoration-none fw-semibold text-success text-truncate" title="Focar e ver detalhes">{p['name']}</a>
				<a href="character.py?session={session_id}&serial={p['serial']}" class="btn btn-sm btn-outline-secondary py-0 px-1 ms-2 border-0" title="Ver ficha e propriedades de {p['name']}" target="_blank" style="font-size: 1.05rem; line-height: 1;">&#128065;</a>
			</div>
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
		<h4 class="fw-bold text-dark mb-0">Mapa Interativo: Ter Mur</h4>
		<p class="text-secondary small mb-0">Role para dar zoom e arraste com o mouse para navegar.</p>
	</div>
	<div>
		<a href="map_ter_mur.py?session={session_id}" class="btn btn-outline-success btn-sm">Atualizar Posições</a>
	</div>
</div>

<!-- Abas de Navegação -->
<ul class="nav nav-pills mb-3 small">
  <li class="nav-item"><a class="nav-link" href="map_trammel.py?session={session_id}">Trammel</a></li>
  <li class="nav-item"><a class="nav-link" href="map_felucca.py?session={session_id}">Felucca</a></li>
  <li class="nav-item"><a class="nav-link" href="map_ilshenar.py?session={session_id}">Ilshenar</a></li>
  <li class="nav-item"><a class="nav-link" href="map_malas.py?session={session_id}">Malas</a></li>
  <li class="nav-item"><a class="nav-link" href="map_tokuno.py?session={session_id}">Tokuno</a></li>
  <li class="nav-item"><a class="nav-link active" href="map_ter_mur.py?session={session_id}">Ter Mur</a></li>
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
		maxZoom: 3,
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

	const playerMarkers = {{}};

	players.forEach(p => {{
		const mapY = imgH - p.py;
		const mapX = p.px;

		const marker = L.marker([mapY, mapX], {{ icon: playerIcon }}).addTo(map);
		const popupHtml = `
		<div style="min-width: 175px;">
			<div class="d-flex justify-content-between align-items-center mb-1">
				<strong style="font-size: 0.95rem;">${{p.name}}</strong>
			</div>
			<div class="text-muted small mb-2" style="font-size: 0.72rem;">
				Posição: <strong>X:${{p.raw_x}} Y:${{p.raw_y}}</strong>
			</div>

			<!-- Health Bar -->
			<div class="mb-1">
				<div class="d-flex justify-content-between" style="font-size: 0.72rem; line-height: 1.1;">
					<span style="font-weight: 600; color: #dc3545;">HP:</span>
					<span style="font-family: monospace;">${{p.hp}} / ${{p.max_hp}}</span>
				</div>
				<div style="background-color: #e9ecef; border-radius: 3px; height: 6px; overflow: hidden; margin-top: 2px;">
					<div style="background-color: #dc3545; height: 100%; width: ${{p.hp_pct}}%;"></div>
				</div>
			</div>

			<!-- Mana Bar -->
			<div class="mb-1">
				<div class="d-flex justify-content-between" style="font-size: 0.72rem; line-height: 1.1;">
					<span style="font-weight: 600; color: #0d6efd;">Mana:</span>
					<span style="font-family: monospace;">${{p.mana}} / ${{p.max_mana}}</span>
				</div>
				<div style="background-color: #e9ecef; border-radius: 3px; height: 6px; overflow: hidden; margin-top: 2px;">
					<div style="background-color: #0d6efd; height: 100%; width: ${{p.mana_pct}}%;"></div>
				</div>
			</div>

			<!-- Stamina Bar -->
			<div class="mb-2">
				<div class="d-flex justify-content-between" style="font-size: 0.72rem; line-height: 1.1;">
					<span style="font-weight: 600; color: #ffc107;">Stam:</span>
					<span style="font-family: monospace;">${{p.stam}} / ${{p.max_stam}}</span>
				</div>
				<div style="background-color: #e9ecef; border-radius: 3px; height: 6px; overflow: hidden; margin-top: 2px;">
					<div style="background-color: #ffc107; height: 100%; width: ${{p.stam_pct}}%;"></div>
				</div>
			</div>

			<div class="pt-2 border-top text-center">
				<a href="character.py?session={session_id}&serial=${{p.serial}}" target="_blank" class="btn btn-sm btn-success py-0 px-2 text-white w-100" style="font-size: 0.75rem;">&#128065; Ver Ficha do Char</a>
			</div>
		</div>
		`;
		marker.bindPopup(popupHtml);
		if (p.serial) {{
			playerMarkers[p.serial] = marker;
		}}
	}});

	window.focusPlayer = function(px, py, serial) {{
		const mapY = imgH - py;
		map.setView([mapY, px], 1);
		if (serial && playerMarkers[serial]) {{
			playerMarkers[serial].openPopup();
		}}
	}};
</script>
"""


web.template.output( 	
	f'&gt; <a href="maps.py?session={session_id}" class="text-decoration-none text-success">Maps</a> &gt; <a href="map_ter_mur.py?session={session_id}" class="text-decoration-none text-success fw-semibold">Ter Mur</a>', 
	username, 
	1, 
	content,
	session_id
)