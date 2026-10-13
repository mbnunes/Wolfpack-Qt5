import os

def output(navigation, username, navbar, content, session_id):

    template_path = 'web_new/template.html' if os.path.exists('web_new/template.html') else 'web/template.html'
    try:
        with open(template_path, 'r', encoding='utf-8') as f:
            template = f.read()
    except:
        template = "<b>ERROR:</b> Couldn't read template.html."

    template = template.replace('%NAVIGATION%', navigation)

    if navbar:
        # Sidebar fina e moderna
        nav_html = """
        <aside class="wp-sidebar-slim shadow-sm">
          <div class="px-3 pt-3 pb-2 text-uppercase text-muted fw-bold" style="font-size: 0.68rem; letter-spacing: 0.5px;">
            Administração
          </div>
          <nav class="nav flex-column mb-auto">
            <a href="main.py?session=%(session)s" class="nav-link">
              <span>&#128202;</span> Dashboard
            </a>
            <a href="console.py?session=%(session)s" class="nav-link">
              <span>&#128187;</span> Console
            </a>
            <a href="accounts.py?session=%(session)s" class="nav-link">
              <span>&#128101;</span> Contas
            </a>
            <a href="pythonlog.py?session=%(session)s" class="nav-link">
              <span>&#128220;</span> Python Log
            </a>
            <a href="maps.py?session=%(session)s" class="nav-link">
              <span>&#128506;</span> Mapas
            </a>
          </nav>
        </aside>
        """ % {'session': session_id}

        # Gera inicial do usuário para a bolinha do avatar (ex: 'admin' -> 'A')
        initial = username[0].upper() if username else 'U'

        # Submenu Dropdown de Perfil à Direita no Topbar
        user_html = f"""
        <div class="dropdown">
          <button class="btn btn-link text-decoration-none text-light p-0 d-flex align-items-center gap-2 dropdown-toggle" 
                  type="button" data-bs-toggle="dropdown" aria-expanded="false" style="box-shadow: none;">
            <div class="wp-avatar-circle shadow-sm">{initial}</div>
            <span class="fw-semibold small d-none d-sm-inline">{username}</span>
          </button>
          <ul class="dropdown-menu dropdown-menu-end shadow-sm border border-light-subtle py-1 mt-2">
            <li>
              <div class="dropdown-header small pb-1">
                Conectado como <strong class="text-dark d-block">{username}</strong>
              </div>
            </li>
            <li><hr class="dropdown-divider my-1"></li>
            <li>
              <a class="dropdown-item small d-flex align-items-center gap-2 text-secondary py-2" href="main.py?session={session_id}">
                <span>&#128202;</span> Visão Geral
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