import web.sessions
import web.template
import cgi
import sys

form = cgi.FieldStorage()
session_id = form.getvalue( 'session', '' )

web.sessions.check_timeouts()
session = web.sessions.get_session( session_id )

if session == None:
	web.sessions.display_timeout()
	sys.exit()

username = session[ 'username' ]

# Dados Mockados (Prontos para ligar nas APIs do Wolfpack/C++ ou os.sysinfo depois)
server_status = "Online"
uptime = "3d 14h 28m"
players_online = 12
players_max = 128
total_accounts = 345
cpu_usage = 18          # Em %
mem_used_mb = 420       # Em MB
mem_total_mb = 2048     # Em MB
mem_percent = int((mem_used_mb / mem_total_mb) * 100)
net_in = "1.8 Mbps"
net_out = "3.2 Mbps"
server_version = "Wolfpack Beta 12.9.17"
qt_version = "6.10.2"
python_version = "3.14.4"

content = f"""
<!-- Cabeçalho do Dashboard -->
<div class="d-flex flex-wrap justify-content-between align-items-center pb-3 mb-4 border-bottom gap-2">
	<div>
		<h3 class="fw-bold text-dark mb-0">Painel Operacional</h3>
		<p class="text-secondary small mb-0">Status em tempo real da instância Wolfpack Server.</p>
	</div>
	<div class="d-flex align-items-center gap-2">
		<span class="badge bg-success-subtle text-success border border-success px-3 py-2 rounded-pill">
			<span class="spinner-grow spinner-grow-sm me-1" role="status" aria-hidden="true" style="width: 0.5rem; height: 0.5rem;"></span>
			Servidor {server_status}
		</span>
		<button class="btn btn-outline-secondary btn-sm" onclick="window.location.reload();">
			Atualizar
		</button>
	</div>
</div>

<!-- Linha 1: Métricas Principais (Cards) -->
<div class="row g-3 mb-4">
	<!-- Players Online -->
	<div class="col-12 col-sm-6 col-xl-3">
		<div class="card h-100 border border-light-subtle shadow-sm">
			<div class="card-body">
				<span class="text-secondary small fw-semibold text-uppercase">Jogadores Online</span>
				<div class="d-flex align-items-baseline gap-2 mt-2">
					<h2 class="fw-bold text-dark mb-0">{players_online}</h2>
					<span class="text-secondary small">/ {players_max} máx</span>
				</div>
				<div class="progress mt-3" style="height: 5px;">
					<div class="progress-bar bg-success" role="progressbar" style="width: {int((players_online/players_max)*100)}%;"></div>
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
				<small class="text-muted d-block mt-3"><a href="accounts.py?session={session_id}" class="text-decoration-none text-success">Gerenciar contas &rarr;</a></small>
			</div>
		</div>
	</div>

	<!-- CPU -->
	<div class="col-12 col-sm-6 col-xl-3">
		<div class="card h-100 border border-light-subtle shadow-sm">
			<div class="card-body">
				<div class="d-flex justify-content-between align-items-center">
					<span class="text-secondary small fw-semibold text-uppercase">Uso de CPU</span>
					<span class="badge bg-light text-dark border">{cpu_usage}%</span>
				</div>
				<h2 class="fw-bold text-dark mb-0 mt-2">{cpu_usage}%</h2>
				<div class="progress mt-3" style="height: 5px;">
					<div class="progress-bar bg-primary" role="progressbar" style="width: {cpu_usage}%;"></div>
				</div>
			</div>
		</div>
	</div>

	<!-- Memória RAM -->
	<div class="col-12 col-sm-6 col-xl-3">
		<div class="card h-100 border border-light-subtle shadow-sm">
			<div class="card-body">
				<div class="d-flex justify-content-between align-items-center">
					<span class="text-secondary small fw-semibold text-uppercase">Memória RAM</span>
					<span class="badge bg-light text-dark border">{mem_percent}%</span>
				</div>
				<div class="d-flex align-items-baseline gap-1 mt-2">
					<h2 class="fw-bold text-dark mb-0">{mem_used_mb}</h2>
					<span class="text-secondary small">/ {mem_total_mb} MB</span>
				</div>
				<div class="progress mt-3" style="height: 5px;">
					<div class="progress-bar bg-info" role="progressbar" style="width: {mem_percent}%;"></div>
				</div>
			</div>
		</div>
	</div>
</div>

<!-- Linha 2: Rede e Detalhes da Instância -->
<div class="row g-3 mb-4">
	<!-- Rede / Conexão -->
	<div class="col-12 col-lg-6">
		<div class="card h-100 border border-light-subtle shadow-sm">
			<div class="card-header bg-white py-3 border-bottom border-light-subtle">
				<h6 class="fw-bold text-dark mb-0">Tráfego de Conexão</h6>
			</div>
			<div class="card-body d-flex flex-column justify-content-around">
				<div class="d-flex justify-content-between align-items-center py-2 border-bottom">
					<span class="text-secondary">Download / Inbound:</span>
					<span class="fw-bold text-dark">{net_in}</span>
				</div>
				<div class="d-flex justify-content-between align-items-center py-2 border-bottom">
					<span class="text-secondary">Upload / Outbound:</span>
					<span class="fw-bold text-dark">{net_out}</span>
				</div>
				<div class="d-flex justify-content-between align-items-center py-2">
					<span class="text-secondary">Uptime do Processo:</span>
					<span class="badge bg-dark-subtle text-dark border">{uptime}</span>
				</div>
			</div>
		</div>
	</div>

	<!-- Info do Sistema / Build -->
	<div class="col-12 col-lg-6">
		<div class="card h-100 border border-light-subtle shadow-sm">
			<div class="card-header bg-white py-3 border-bottom border-light-subtle">
				<h6 class="fw-bold text-dark mb-0">Ambiente do Servidor</h6>
			</div>
			<div class="card-body d-flex flex-column justify-content-around">
				<div class="d-flex justify-content-between align-items-center py-2 border-bottom">
					<span class="text-secondary">Core:</span>
					<span class="fw-semibold text-dark">{server_version}</span>
				</div>
				<div class="d-flex justify-content-between align-items-center py-2 border-bottom">
					<span class="text-secondary">Qt Framework:</span>
					<span class="fw-semibold text-dark">{qt_version}</span>
				</div>
				<div class="d-flex justify-content-between align-items-center py-2">
					<span class="text-secondary">Python Runtime:</span>
					<span class="fw-semibold text-dark">{python_version}</span>
				</div>
			</div>
		</div>
	</div>
</div>

<!-- Ações Rápidas -->
<div class="card border border-light-subtle shadow-sm">
	<div class="card-header bg-white py-3 border-bottom border-light-subtle">
		<h6 class="fw-bold text-dark mb-0">Acessos Rápidos</h6>
	</div>
	<div class="card-body">
		<div class="d-flex flex-wrap gap-2">
			<a href="console.py?session={session_id}" class="btn btn-outline-success btn-sm">Abrir Console</a>
			<a href="accounts.py?session={session_id}" class="btn btn-outline-success btn-sm">Contas</a>
			<a href="maps.py?session={session_id}" class="btn btn-outline-success btn-sm">Mapas em Tempo Real</a>
			<a href="pythonlog.py?session={session_id}" class="btn btn-outline-secondary btn-sm">Logs do Python</a>
		</div>
	</div>
</div>
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
	f'&gt; <a href="main.py?session={session_id}" class="text-decoration-none text-success fw-semibold">Dashboard</a>', 
	user_badge, 
	1, 
	content,
	session_id
)