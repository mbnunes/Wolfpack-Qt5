
import wolfpack

def target(player, arguments, target):
	leaves = wolfpack.finditem(arguments[0])

	# The leaves have to be in the backpack
	if not leaves or leaves.getoutmostitem() != player.getbackpack():
		player.socket.clilocmessage(1042664) # You must have the object in your backpack to use it.
		return

	# The book has to be in the backpack	
	if not target.item or target.item.getoutmostitem() != player.getbackpack():
		player.socket.clilocmessage(1042664) # You must have the object in your backpack to use it.
		return

	# Only works on books	
	if not target.item.hasscript('book'):
		target.item.say(1061911, '', '', False, 0x3b2, player.socket) # You can only use red leaves to seal the ink into book pages!
		return

	# Only on unprotected books
	if target.item.hastag('protected'):
		target.item.say(1061909, '', '', False, 0x3b2, player.socket) # The ink in this book has already been sealed.
		return

	# Seal the book
	target.item.settag('protected', 1)
	target.item.say(1061910, '', '', False, 0x3b2, player.socket) # You seal the ink to the page using wax from the red leaf.

	if leaves.amount <= 1:
		leaves.delete()
	else:
		leaves.amount -= 1
		leaves.update()

def onUse(player, item):
	if player.getbackpack() != item.getoutmostitem():
		player.socket.clilocmessage(1042664) # You must have the object in your backpack to use it.
		return True

	player.socket.attachtarget('plants.redleaves.target', [item.serial])
	player.socket.clilocmessage(1061907) # Choose a book you wish to seal with the wax from the red leaf.
	return True
