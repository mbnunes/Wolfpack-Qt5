
"""
	\command lock
	\description Manage or attach a lock on an object.
	\note Lock ids are strings. They don't need to be numeric.
"""

import wolfpack
from wolfpack.gumps import cGump
import random

def createkey(char, id):
	keys = ['100e', '100f', '1010', '1013']
	key = wolfpack.additem(random.choice(keys))
	key.settag('lock', id)
	key.container = char.getbackpack()
	key.update()
	char.socket.sysmessage('A key has been added to your backpack.')

def gump_response(char, args, response):
	if len(args) < 1:
		return

	item = wolfpack.finditem(args[0])

	if not item:
		return

	# Add a lock to this item
	if response.button == 1:
		# Prepend 'lock' to the event chain
		if not item.hasscript( 'lock' ):
			item.addscript( 'lock' )

		# Set the lock id
		item.settag('lock', response.text[1])
		item.resendtooltip()

		char.socket.sysmessage('Added the lock to the item.')

		# Should we create a key in the backpack of the user?
		if 1 in response.switches:
			createkey(char, response.text[1])

	# Modify lock
	elif response.button == 2:
		# Set the lock id
		item.settag('lock', response.text[1])

		# Remove or Add the locked tag
		if 1 in response.switches:
			item.settag('locked', 1)
		elif item.hastag('locked'):
			item.deltag('locked')

		item.resendtooltip()

		char.socket.sysmessage('Modified the lock of this item.')

	# Remove lock
	elif response.button == 3:
		item.deltag('lock')
		item.deltag('locked')

		# Remove the event from the eventlist
		item.removescript( 'lock' )
		item.resendtooltip()

		char.socket.sysmessage('The lock has been removed from the item.')

	# Create a key
	elif response.button == 4:
		if not item.hastag('lock'):
			char.socket.sysmessage('This item has no lock.')
		else:
			lock = str(item.gettag('lock'))
			createkey(char, lock)

def lock_response(char, args, target):
	# Check if an item was targetted
	if not target.item:
		char.socket.sysmessage('You have to target an item.')
		return

	# Build the lock gump
	gump = cGump(x=100, y=100, callback="lock.gump_response")
	gump.setArgs([target.item.serial])

	# Check if the item already has a lock
	if target.item.hasscript('lock') and target.item.hastag('lock'):
		gump.addBackground(id=0x2436, width=425, height=285)

		lock = str(target.item.gettag('lock'))
		locked = 0

		if target.item.hastag('locked') and int(target.item.gettag('locked')) != 0:
			locked = 1

		text = '<basefont color="#FECECE"><h3>Manage Lock</h3><br><basefont color="#FEFEFE">This dialog will help you to manage the lock and status of this item.'
		gump.addHtmlGump(x=20, y=20, width=410, height=90, html=text)

		# InputField for the key id
		gump.addText(x=20, y=90, text='The id for this lock:', hue=0x835)
		gump.addResizeGump(x=20, y=113, id=0xBB8, width=160, height=25)
		gump.addInputField(x=25, y=115, width=150, height=20, hue=0x834, id=1, starttext=lock)

		# "Locked? Unlocked?"
		gump.addCheckbox(x=20, y=164, off=0x25f8, on=0x25fb, id=1, checked=locked)
		gump.addText(x=55, y=168, text='Locked', hue=0x835)

		# Add Button
		gump.addText(x=50, y=212, text='Modify lock', hue=0x835)
		gump.addButton(x=20, y=212, up=0x26af, down=0x26b1, returncode=2)

		gump.addText(x=50, y=242, text='Cancel', hue=0x835)
		gump.addButton(x=20, y=242, up=0x26af, down=0x26b1, returncode=0)

		gump.addText(x=250, y=212, text='Remove lock', hue=0x835)
		gump.addButton(x=220, y=212, up=0x26af, down=0x26b1, returncode=3)

		gump.addText(x=250, y=242, text='Create key', hue=0x835)
		gump.addButton(x=220, y=242, up=0x26af, down=0x26b1, returncode=4)

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
		gump.addText(x=55, y=171, text='Create a key in my backpack', hue=0x835)

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

def searchkey(item, lock):
	# It's a key.
	if item.hasscript( 'key' ) and item.hastag( 'lock' ):
		if lock == str(item.gettag('lock')):
			return 1

	for subitem in item.content:
		if searchkey(subitem, lock):
			return 1

	return 0

def onUse(char, item):
	# The item is not locked
	if not item.hastag('lock'):
		return 0

	# The magic lock spell has the lock id 'magic'
	lock = str(item.gettag('lock'))
	locked = 0

	if item.hastag('locked'):
		locked = int(item.gettag('locked'))

	# For magic locks, the locked flag is irrelevant
	if lock != 'magic' and locked == 0:
		return 0

	# GMs ignore locks but their access should be logged
	if char.gm:
		char.socket.clilocmessage(501281)
		return 0

	# Only doors can be opened without unlocking them
	# if the user has the key in his posession.
	if item.hasscript( 'door' ):
		if searchkey(char.getbackpack(), lock):
			char.socket.clilocmessage(501282)
			return 0

		char.socket.clilocmessage(500788)
	else:
		char.socket.clilocmessage(501746)

	return 1

def onTelekinesis(char, item):
	return onUse(char, item)

#
# Show a special tooltip
#
def onShowTooltip(viewer, object, tooltip):
  # If the character is a gm and the targetted item has a lock, display the lock id
  if viewer.gm:
    lock = 'None'

    if object.hastag('lock'):
      lock = str(object.gettag('lock'))

    if object.hastag('locked') and int(object.gettag('locked')) != 0:
      suffix = ' (Locked)'
    else:
      suffix = ' (Unlocked)'

    tooltip.add(1050045, " \tLock: " + lock + suffix + "\t ")
