# Detailed information about Accounts - Support for Admin and Players

import web.sessions
import web.template
import cgi
import sys
import wolfpack.accounts
from urllib.parse import quote

letters = "#abcdefghijklmnopqrstuvwxyz"

form = cgi.FieldStorage()
session_id = form.getvalue('session', '')
letter = form.getvalue('letter', '#')
account = form.getvalue('username', '')

save = 0
try:
	save = int(form.getvalue('save', '0'))
except Exception:
	save = 0
	
message = None
is_error = False

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

# Se nenhum nome de conta foi passado, assume a própria conta logada
if not account:
	account = username

# Segurança: Usuários sem permissão administrativa só podem visualizar e editar sua própria conta!
if not is_admin and account.lower() != username.lower():
	content = f"""
	<div class="alert alert-danger border shadow-sm my-4">
		<h5 class="alert-heading fw-bold">&#128683; Acesso Negado</h5>
		<p class="mb-0">Você não possui permissão para acessar os dados cadastrais de outro usuário.</p>
		<div class="mt-3">
			<a href="main.py?session={session_id}&view=player" class="btn btn-outline-danger btn-sm">&larr; Voltar ao Meu Painel</a>
		</div>
	</div>
	"""
	web.template.output("Acesso Negado", username, 1, content, session_id)
	sys.exit()

record = wolfpack.accounts.find(account)

if not record:
	print("Content-type: text/html; charset=utf-8\n\n")
	print("<div class='alert alert-danger m-4'>Conta não encontrada.</div>")
	sys.exit()

# Alteração de dados
if save == 1:
	password1 = form.getvalue('password1', '')
	password2 = form.getvalue('password2', '')

	# Alteração de Senha
	if password1 != '':
		if password1 != password2:
			message = "As senhas digitadas não coincidem."
			is_error = True
		else:
			record.password = password1

	if not is_error:
		# Somente Administradores podem alterar ACL e Bloqueios
		if is_admin:
			record.acl = form.getvalue('acl', 'player')
			if form.getvalue('block', 'false') == 'true': 
				record.block()
			else:
				record.unblock()

			if form.getvalue('blockuntil', '') != '':
				record.blockuntil = form.getvalue('blockuntil', '')

		if form.getvalue('email', '') != '':
			record.email = form.getvalue('email', '')

		wolfpack.accounts.save()
		message = "Os dados da conta foram atualizados com sucesso."

alert_html = ""
if message:
	alert_type = "danger" if is_error else "success"
	alert_html = f"""
	<div class="alert alert-{alert_type} alert-dismissible fade show" role="alert">
		{message}
	</div>
	"""

# Personagens vinculados à conta
chars_html = ""
chars_list = getattr(record, 'characters', [])
if len(chars_list) > 0:
	char_items = []
	for char in chars_list:
		c_name = getattr(char, 'name', 'Sem Nome')
		c_serial = getattr(char, 'serial', 0)
		char_items.append(f"""
		<li class="list-group-item d-flex justify-content-between align-items-center">
			<div>
				<a href="character.py?session={session_id}&serial=0x{c_serial:x}&account={quote(record.name)}" class="fw-bold text-success text-decoration-none">
					{cgi.escape(str(c_name))}
				</a>
			</div>
			<div>
				<span class="badge bg-secondary-subtle text-dark border font-monospace me-2">Serial: 0x{c_serial:x}</span>
				<a href="character.py?session={session_id}&serial=0x{c_serial:x}&account={quote(record.name)}" class="btn btn-sm btn-outline-success py-0">Ver Ficha &rarr;</a>
			</div>
		</li>
		""")

	chars_html = f"""
	<div class="card border border-light-subtle shadow-sm mb-4">
		<div class="card-header bg-white py-3">
			<h6 class="fw-bold text-dark mb-0">Personagens Vinculados ({len(chars_list)})</h6>
		</div>
		<ul class="list-group list-group-flush">
			{''.join(char_items)}
		</ul>
	</div>
	"""

# Campos exclusivos para Administrador
admin_fields = ""
delete_btn = ""
back_btn = f'<a href="main.py?session={session_id}&view=player" class="btn btn-outline-secondary btn-sm">&larr; Voltar ao Painel</a>'

if is_admin:
	back_btn = f'<a href="accounts.py?session={session_id}&letter={quote(letter)}" class="btn btn-outline-secondary btn-sm">&larr; Voltar para Contas</a>'
	
	acl_options = []
	for acl_item in wolfpack.accounts.acls():
		selected = 'selected' if acl_item == record.acl else ''
		acl_options.append(f'<option value="{acl_item}" {selected}>{acl_item}</option>')

	is_blocked = bool(record.flags & 0x00000001)
	sel_block_false = '' if is_blocked else 'selected'
	sel_block_true = 'selected' if is_blocked else ''

	admin_fields = f"""
	<div class="col-12 col-md-4">
		<label class="form-label small fw-semibold text-secondary">Nível de Acesso (ACL)</label>
		<select name="acl" class="form-select form-select-sm">
			{''.join(acl_options)}
		</select>
	</div>

	<div class="col-12 col-md-4">
		<label class="form-label small fw-semibold text-secondary">Bloqueio de Conta</label>
		<select name="block" class="form-select form-select-sm">
			<option value="false" {sel_block_false}>Ativa (False)</option>
			<option value="true" {sel_block_true}>Bloqueada (True)</option>
		</select>
	</div>

	<div class="col-12 col-md-4">
		<label class="form-label small fw-semibold text-secondary">Bloqueado Até (Data/Timestamp)</label>
		<input type="text" class="form-control form-control-sm" name="blockuntil" value="{record.blockuntil}" />
	</div>
	"""

	if account != username:
		delete_btn = f"""
		<a href="accounts.py?session={session_id}&letter={quote(letter)}&delete={quote(account)}" 
		   class="btn btn-outline-danger btn-sm"
		   onclick="return confirm('Tem certeza absoluta de que deseja excluir a conta {account}? Esta ação não pode ser desfeita.');">
			Excluir Conta
		</a>
		"""

content = f"""
<div class="d-flex justify-content-between align-items-center pb-2 mb-3 border-bottom">
	<div>
		<h4 class="fw-bold text-dark mb-0">Minha Conta: <span class="text-success">{account}</span></h4>
		<p class="text-secondary small mb-0">Último Login: {record.lastlogin} &bull; ACL: <strong>{record.acl}</strong></p>
	</div>
	<div>
		{back_btn}
	</div>
</div>

{alert_html}

<!-- Formulário de Informações Gerais -->
<div class="card border border-light-subtle shadow-sm mb-4">
	<div class="card-header bg-white py-3">
		<h6 class="fw-bold text-dark mb-0">Configurações Cadastrais</h6>
	</div>
	<div class="card-body">
		<form action="account.py" method="POST" class="row g-3">
			<input type="hidden" name="save" value="1" />
			<input type="hidden" name="session" value="{session_id}" />
			<input type="hidden" name="username" value="{quote(account)}" />
			<input type="hidden" name="letter" value="{quote(letter)}" />

			<div class="col-12 col-md-6">
				<label class="form-label small fw-semibold text-secondary">Nome de Usuário</label>
				<input type="text" class="form-control form-control-sm bg-light" value="{account}" readonly />
			</div>

			<div class="col-12 col-md-6">
				<label class="form-label small fw-semibold text-secondary">E-Mail de Contato</label>
				<input type="email" class="form-control form-control-sm" name="email" value="{record.email}" />
			</div>

			<div class="col-12 col-md-6">
				<label class="form-label small fw-semibold text-secondary">Nova Senha (opcional)</label>
				<input type="password" class="form-control form-control-sm" name="password1" placeholder="Deixe em branco para manter a atual" />
			</div>

			<div class="col-12 col-md-6">
				<label class="form-label small fw-semibold text-secondary">Confirmar Nova Senha</label>
				<input type="password" class="form-control form-control-sm" name="password2" placeholder="Repita a nova senha" />
			</div>

			{admin_fields}

			<div class="col-12 d-flex justify-content-between align-items-center mt-4 pt-3 border-top">
				<button type="submit" class="btn text-white btn-sm px-4 fw-semibold" style="background-color: #004026;">
					Salvar Alterações
				</button>
				{delete_btn}
			</div>
		</form>
	</div>
</div>

{chars_html}
"""

web.template.output( 	
	f'&gt; <a href="account.py?username={quote(account)}&letter={quote(letter)}&session={session_id}" class="text-decoration-none text-success fw-semibold">Minha Conta ({account})</a>', 
	username, 
	1, 
	content,
	session_id
)