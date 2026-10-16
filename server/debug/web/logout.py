# Handles User Logout

import web.sessions
import cgi
import sys

form = cgi.FieldStorage()
session_id = form.getvalue('session', '')

# Remove a sessão ativa se ela existir
if session_id:
    web.sessions.remove_session(session_id)

content = """<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="utf-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1" />
  <meta http-equiv="refresh" content="3; URL=login.py" />
  <title>Logout Realizado - Wolfpack Administration</title>
  
  <!-- Bootstrap 5 CSS -->
  <link rel="stylesheet" href="css/bootstrap.min.css" />
  <link rel="stylesheet" href="css/custom.css" />

  <style>
    body {
      background-color: #f8fafc;
      min-height: 100vh;
      display: flex;
      flex-direction: column;
    }
    .wp-header {
      background-color: #004026;
      border-bottom: 2px solid #002b1a;
      height: 60px;
    }
  </style>
</head>
<body class="d-flex flex-column min-vh-100">

  <!-- Topbar com Logo Oficial -->
  <header class="wp-header shadow-sm">
    <div class="container-fluid px-3 px-md-4 d-flex align-items-center h-100">
      <a href="login.py" class="d-flex align-items-center text-decoration-none">
        <img src="images/top_logo.jpg" alt="Wolfpack Logo" class="img-fluid" style="max-height: 48px;" />
      </a>
    </div>
  </header>

  <!-- Card Centralizado na Tela -->
  <main class="container d-flex flex-grow-1 align-items-center justify-content-center py-4">
    <div class="col-12 col-sm-10 col-md-7 col-lg-5 col-xl-4">
      <div class="card border border-light-subtle shadow-sm p-4 text-center bg-white rounded-3">
        
        <!-- Ícone de Sucesso / Desconectado -->
        <div class="mb-3">
          <span class="d-inline-flex align-items-center justify-content-center rounded-circle bg-success-subtle text-success" style="width: 56px; height: 56px; font-size: 1.5rem;">
            &#10003;
          </span>
        </div>

        <h4 class="fw-bold text-dark mb-2">Sessão Encerrada</h4>
        <p class="text-secondary small mb-4">
          Você foi desconectado com sucesso da administração remota. Você será redirecionado para o login em instantes...
        </p>

        <!-- Botão Imediato de Retorno -->
        <a href="login.py" class="btn text-white w-100 py-2 fw-semibold shadow-sm" style="background-color: #004026;">
          Retornar ao Login
        </a>
      </div>
    </div>
  </main>

  <!-- Rodapé -->
  <footer class="py-3 bg-white border-top text-center text-secondary small">
    Wolfpack-Qt5 &bull; Remote Administration Console
  </footer>

  <!-- Bootstrap 5 JS -->
  <script src="js/bootstrap.bundle.min.js"></script>
</body>
</html>
"""

print("Content-type: text/html; charset=utf-8\n\n")
print(content)