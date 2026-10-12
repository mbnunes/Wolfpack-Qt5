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
        # Quando há menu ativo, renderiza a coluna lateral inteira
        nav_html = """
        <aside class="col-12 col-md-4 col-lg-3">
          <div class="card wp-sidebar-card shadow-sm">
            <div class="card-header wp-sidebar-header py-2">
              Remote Administration
            </div>
            <div class="list-group list-group-flush small">
              <a href="main.py?session=%(session)s" class="list-group-item list-group-item-action py-2">Main</a>
              <a href="console.py?session=%(session)s" class="list-group-item list-group-item-action py-2">Console</a>
              <a href="accounts.py?session=%(session)s" class="list-group-item list-group-item-action py-2">Accounts</a>
              <a href="pythonlog.py?session=%(session)s" class="list-group-item list-group-item-action py-2">Python Log</a>
              <a href="maps.py?session=%(session)s" class="list-group-item list-group-item-action py-2">Maps with Players</a>
              <hr class="my-2 border-top border-secondary-subtle opacity-25">
              <a href="logout.py?session=%(session)s" class="list-group-item list-group-item-action text-danger py-2">Logout</a>
            </div>
          </div>
        </aside>
        """ % {'session': session_id}
        content_col = "col-12 col-md-8 col-lg-9"
    else:
        # Quando não há menu (ex: tela de login), o miolo fica centralizado e mais largo
        nav_html = ""
        content_col = "col-12 col-md-10 col-lg-8"

    template = template.replace('%USERNAME%', username)
    template = template.replace('%NAVBAR%', nav_html)
    template = template.replace('%CONTENT_COL%', content_col)
    template = template.replace('%CONTENT%', content)
    
    print("Content-type: text/html; charset=utf-8\n\n")
    print(template)