
import wolfpack
from wolfpack.gumps import cGump
import random

def gump_response(char, args, response):
  # Add a lock to this item
  if response.button == 1 and len(args) == 1:
    item = wolfpack.finditem(args[0])

    if item:
      # Prepend 'lock' to the event chain
      if not 'lock' in item.events:
        events = item.events
        events[:0] = ['lock']
        item.events = events

      # Set the lock id
      item.settag('lock', response.text[1])

      char.socket.sysmessage('Added the lock to the item.')

      # Should we create a key in the backpack of the user?
      if 1 in response.switches:
        keys = ['100e', '100f', '1010', '1013']
        key = wolfpack.additem(random.choice(keys))
        key.settag('lock', response.text[1])
        key.container = char.getbackpack()
        key.update()

        char.socket.sysmessage('A key has been added to your backpack.')

      return

def lock_response(char, args, target):
  # Check if an item was targetted
  if not target.item:
    char.socket.sysmessage('You have to target an item.')
    return

  # Build the lock gump
  gump = cGump(x=100, y=100, callback="lock.gump_response")
  gump.setArgs([target.item.serial])
  
  # Check if the item already has a lock
  if 'lock' in target.item.events and target.item.hastag('lock'):
    gump.addBackground(id=0x2436, width=450, height=350)	

    lock = str(target.item.gettag('lock'))
    locked = 0
    if target.item.hastag('locked') and int(target.item.gettag('locked')) != 0:
      locked = 1

    text = '<basefont color="#FECECE"><h3>Manage Lock</h3><br><basefont color="#FEFEFE">This dialog will help you to manage the lock of this item.'
    gump.addHtmlGump(x=20, y=20, width=410, height=200, html=text)

  # The targetted item is unlocked 
  else:
    gump.addBackground(id=0x2436, width=425, height=285)

    text = '<basefont color="#FECECE"><h3>Add Lock</h3><br><basefont color="#FEFEFE">This dialog will help you to add a lock to this item.Please enter a unique identifier for this lock. The lock has to share the same identifier with any key that is supposed to fit.'
    gump.addHtmlGump(x=20, y=20, width=390, height=200, html=text)
 
    # Create a random key id (several characters are excluded that can be easily mistaken for something else)
    parts = ['A','B','C','D','E','F','G','H','J','K','L','M','N','P','Q','R','S','T','U','V','W','X','Y','Z','2','3','4','5','6','7','8','9']
    rkeyid = ''

    for i in range(1,8):
      rkeyid += random.choice(parts)

    # InputField for the key id
    gump.addText(x=20, y=105, text='Please enter an id for this lock:', hue=0x835)
    gump.addResizeGump(x=20, y=128, id=0xBB8, width=160, height=25)
    gump.addInputField(x=25, y=130, width=150, height=20, hue=0x834, id=1, starttext=rkeyid)

    # "Create a key in my backpack"
    gump.addCheckbox(x=20, y=167, off=0x25f8, on=0x25fb, id=1)
    gump.addText(x=55, y=171, text='Create a key in my backpack.', hue=0x835)

    # Add Button
    gump.addText( x=50, y=212, text='Add lock', hue=0x835 )
    gump.addButton( x=20, y=212, up=0x26af, down=0x26b1, returncode=1 )

    gump.addText( x=50, y=242, text='Cancel', hue=0x835 )
    gump.addButton( x=20, y=242, up=0x26af, down=0x26b1, returncode=0 )
 
  gump.send(char)

def commandLock( socket, command, arguments ):
  # Select a target you want to edit.
  socket.sysmessage("Select a target you want to modify.")
  socket.attachtarget("lock.lock_response")

def onLoad():
  wolfpack.registercommand("lock", commandLock)

#
# Event for Locking Items
#
def onUse( char, item ):
	events = item.events

	if not item.hastag( 'lock' ):
		return 0

	lock = item.gettag( 'lock' )
	
	if lock != 'magic':
		if item.hastag('locked'):
			locked = int(item.gettag('locked'))
		else:
			locked = 0
			
		if locked == 0:
			return 0
	
		# Search for a key in the users backpack
		backpack = char.getbackpack()
				
		if searchkey(backpack, lock):
			if 'door' in events:
				char.message(501282) # quickly open / relock
			return 0
	
	char.message(502503) # Thats locked
	return 1
