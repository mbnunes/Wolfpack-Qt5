
import wolfpack
import random
from wolfpack.gumps import cGump
from wolfpack.consts import TINKERING

def gump_response(char, args, response):
  if len(args) < 1 or response.button != 1:
    return

  key = wolfpack.finditem(args[0])

  if not char.canreach(key, 5):
    char.socket.clilocmessage(501661)
    return

  # Rename
  new_name = response.text[1][:30] # 30 Chars max.
  key.name = new_name
  char.socket.sysmessage("You renamed the key to '%s'" % new_name)

  # Rekey
  if char.gm:
    new_lock = response.text[2]

  if len(new_lock) != 0:
    key.settag('lock', new_lock)
    char.socket.sysmessage('This key now unlocks: ' + new_lock)
  else:
    key.deltag('lock')
    char.socket.sysmessage('You erase the lock information from the key.')

  key.resendtooltip()

def rename_key(char, key):

# Build the lock gump
  gump = cGump(x=100, y=100, callback="key.gump_response")
  gump.setArgs([key.serial])

  # Renaming, blanking and modification of keys allowed for gms
  gump.addBackground(id=0x2436, width=425, height=285)

  if char.gm:
    text = '<basefont color="#FECECE"><h3>Manage Key</h3><br><basefont color="#FEFEFE">This dialog will help you to manage or rename this key.'
  else:
    text = '<basefont color="#FECECE"><h3>Manage Key</h3><br><basefont color="#FEFEFE">This dialog will help you to rename this key.'

  gump.addHtmlGump(x=20, y=20, width=410, height=90, html=text)

  gump.addText(x=20, y=65, text='The name of this key:', hue=0x835)
  gump.addResizeGump(x=20, y=88, id=0xBB8, width=200, height=25)
  gump.addInputField(x=25, y=90, width=190, height=20, hue=0x834, id=1, starttext=key.name)

  # InputField for the key id
  if char.gm:
    lock = ''
    if key.hastag('lock'):
      lock = str(key.gettag('lock'))

    gump.addText(x=235, y=65, text='The lock id of this key:', hue=0x835)
    gump.addResizeGump(x=235, y=88, id=0xBB8, width=160, height=25)
    gump.addInputField(x=240, y=90, width=150, height=20, hue=0x834, id=2, starttext=lock)

  gump.addText(x=50, y=130, text='Modify key', hue=0x835)
  gump.addButton(x=20, y=130, up=0x26af, down=0x26b1, returncode=1)

  gump.addText(x=50, y=170, text='Cancel', hue=0x835)
  gump.addButton(x=20, y=170, up=0x26af, down=0x26b1, returncode=0)

  gump.send(char)

def lock_response(char, args, target):
  if len(args) != 1:
    return

  key = wolfpack.finditem(args[0])
  if not key or not char.canreach(key,5):
    char.socket.clilocmessage(501661)
    return

  # Check for an item target.
  if not target.item or not char.canreach(target.item,5):
    char.socket.clilocmessage(501666)
    return

  if target.item == key:
    rename_key(char,key)

  elif target.item.hasscript( 'lock' ):
    if target.item.hastag('locked') and int(target.item.gettag('locked')) == 1:
      target.item.deltag('locked')
    else:
      target.item.settag('locked',1)
    char.soundeffect(0x241)

  else:
    char.socket.clilocmessage(501666)

  return

def copy_response(char, args, target):
  if len(args) != 1:
    return

  key = wolfpack.finditem(args[0])
  if not key or not char.canreach(key,5):
    char.socket.clilocmessage(501661)
    return

  # Check if the targetted item is a key
  if not target.item or not target.item.hasscript( 'key' ):
    char.socket.clilocmessage(501679)
    return

  # Also a blank key?
  if not target.item.hastag('lock'):
    char.socket.clilocmessage(501675)
    return

  # Check if the player can reach the item
  if not char.canreach(target.item,5):
    char.socket.clilocmessage(501661)
    return

  # Tinkering check (15%-30%, 25% chance of loosing the key on failure)
  if char.checkskill(TINKERING, 150, 300):
    key.settag('lock',target.item.gettag('lock'))
    key.resendtooltip()
    char.socket.clilocmessage(501676)
  else:
    char.socket.clilocmessage(501677)

    # 25% chance of destroying the blank key
    if random.randint(1,4) == 1:
      char.socket.clilocmessage(501678)
      key.remove()

def onUse(char, key):
  # Does this key open a lock?
  if not key.hastag('lock'):
    # The key is blank so we wan't to make a copy of it
    char.socket.clilocmessage(501663)
    char.socket.attachtarget('key.copy_response',[key.serial])
  else:
    char.socket.clilocmessage(501662)
    char.socket.attachtarget('key.lock_response',[key.serial])

  return 1

def onShowTooltip(char, item, tooltip):
  tooltip.reset()
  tooltip.add(0xF9060 + item.id, '')

  # The user defined name
  if len(item.name) > 0:
    tooltip.add(1050045, " \t" + "Unlocks: " + item.name + "\t ")

  # Add the lock id for gms
  if char.gm:
    if item.hastag('lock'):
      lock = str(item.gettag('lock'))
      tooltip.add(1050045, " \t" + "Lock: " + lock + "\t ")
    else:
      tooltip.add(1050045, " \tThis key is blank\t ")
