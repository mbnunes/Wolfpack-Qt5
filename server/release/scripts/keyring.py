
def onDropOnItem(keyring, key):
  if 'keyring' not in keyring.events or 'key' not in key.events:
    return 0

  char = key.container

  # Only non-blank keys may be put on a keyring...
  if not key.hastag('lock'):
    char.socket.clilocmessage(501689)
    return 0

  # Keyring is full
  keycount = len(keyring.content)

  if keycount >= 20:
    char.socket.clilocmessage(1008138)
    return 0

  # Add the key to the keyring
  key.removefromview(0)
  key.container = keyring
  key.layer = 0

  # Id?
  if keycount >= 5:
    newid = 0x176b
  elif keycount >= 3:
    newid = 0x176a
  else:
    newid = 0x1769

  if keyring.id != newid:
    keyring.id = newid
    keyring.update()

  char.socket.clilocmessage(501691)
  return 1

def onUse(char, item):
  return 1