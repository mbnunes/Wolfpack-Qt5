import os
import urllib.parse

def output(navigation, username, navbar, content, session_id):

    template_path = 'web_new/template.html' if os.path.exists('web_new/template.html') else 'web/template.html'
    try:
        with open(template_path, 'r', encoding='utf-8') as f:
            template = f.read()
    except:
        template = "<b>ERROR:</b> Couldn't read template.html."

    template = template.replace('%NAVIGATION%', navigation)

    if navbar:
        is_admin = False
        user_acc = None
        if username:
            try:
                import wolfpack.accounts
                user_acc = wolfpack.accounts.find(username)
                if user_acc:
                    is_admin = bool(user_acc.authorized('RemoteAccess', 'login'))
            except Exception:
                is_admin = False

        username_quoted = urllib.parse.quote(username) if username else ''

        admin_nav = ""
        if is_admin:
            admin_nav = f"""
            <div class="px-3 pt-3 pb-1 text-uppercase text-muted fw-bold" style="font-size: 0.68rem; letter-spacing: 0.5px;">
              Administração
            </div>
            <nav class="nav flex-column mb-2">
              <a href="main.py?session={session_id}" class="nav-link">
                <span>&#128202;</span> Dashboard Admin
              </a>
              <a href="console.py?session={session_id}" class="nav-link">
                <span>&#128187;</span> Console
              </a>
              <a href="accounts.py?session={session_id}" class="nav-link">
                <span>&#128101;</span> Contas
              </a>
              <a href="pythonlog.py?session={session_id}" class="nav-link">
                <span>&#128220;</span> Python Log
              </a>
              <a href="maps.py?session={session_id}" class="nav-link">
                <span>&#128506;</span> Mapas
              </a>
            </nav>
            <hr class="mx-3 my-2 text-secondary-subtle">
            """

        player_nav = f"""
        <div class="px-3 pt-2 pb-1 text-uppercase text-muted fw-bold" style="font-size: 0.68rem; letter-spacing: 0.5px;">
          Portal do Jogador
        </div>
        <nav class="nav flex-column mb-auto">
          <a href="main.py?session={session_id}&view=player" class="nav-link">
            <span>&#128100;</span> Meu Painel
          </a>
          <a href="account.py?session={session_id}&username={username_quoted}" class="nav-link">
            <span>&#9881;</span> Minha Conta
          </a>
          <a href="player_chars.py?session={session_id}" class="nav-link">
            <span>&#128101;</span> Meus Personagens
          </a>
          <a href="properties.py?session={session_id}" class="nav-link">
            <span>&#127960;</span> Propriedades
          </a>
        </nav>
        """

        nav_html = f"""
        <aside class="wp-sidebar-slim shadow-sm">
          {admin_nav}
          {player_nav}
        </aside>
        """

        # Gera inicial do usuário para a bolinha do avatar (ex: 'admin' -> 'A')
        initial = username[0].upper() if username else 'U'
        role_label = "Administrador" if is_admin else "Jogador"
        role_badge = f'<span class="badge bg-warning text-dark ms-1" style="font-size: 0.65rem;">Admin</span>' if is_admin else f'<span class="badge bg-light text-dark ms-1" style="font-size: 0.65rem;">Jogador</span>'

        # Submenu Dropdown de Perfil à Direita no Topbar
        user_html = f"""
        <div class="dropdown">
          <button class="btn btn-link text-decoration-none text-light p-0 d-flex align-items-center gap-2 dropdown-toggle" 
                  type="button" data-bs-toggle="dropdown" aria-expanded="false" style="box-shadow: none;">
            <div class="wp-avatar-circle shadow-sm">{initial}</div>
            <div class="text-start d-none d-sm-inline">
              <span class="fw-semibold small text-white d-block lh-1">{username}</span>
              <span class="text-white-50" style="font-size: 0.72rem;">{role_label}</span>
            </div>
          </button>
          <ul class="dropdown-menu dropdown-menu-end shadow-sm border border-light-subtle py-1 mt-2">
            <li>
              <div class="dropdown-header small pb-1">
                Conectado como <strong class="text-dark d-block">{username} {role_badge}</strong>
              </div>
            </li>
            <li><hr class="dropdown-divider my-1"></li>
            <li>
              <a class="dropdown-item small d-flex align-items-center gap-2 text-secondary py-2" href="account.py?session={session_id}&username={username_quoted}">
                <span>&#9881;</span> Minha Conta
              </a>
            </li>
            <li>
              <a class="dropdown-item small d-flex align-items-center gap-2 text-secondary py-2" href="player_chars.py?session={session_id}">
                <span>&#128101;</span> Meus Personagens
              </a>
            </li>
            <li>
              <a class="dropdown-item small d-flex align-items-center gap-2 text-secondary py-2" href="properties.py?session={session_id}">
                <span>&#127960;</span> Minhas Propriedades
              </a>
            </li>
            <li><hr class="dropdown-divider my-1"></li>
            <li>
              <a class="dropdown-item small d-flex align-items-center gap-2 text-danger fw-semibold py-2" href="logout.py?session={session_id}">
                <span>&#128682;</span> Encerrar Sessão
              </a>
            </li>
          </ul>
        </div>
        """
    else:
        nav_html = ""
        user_html = ""

    template = template.replace('%USERNAME%', user_html)
    template = template.replace('%NAVBAR%', nav_html)
    template = template.replace('%CONTENT%', content)
    
    print("Content-type: text/html; charset=utf-8\n\n")
    print(template)