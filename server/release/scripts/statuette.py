import wolfpack
import wolfpack.gumps

# ids = { id : [name, soundid] }
ids = {
	0x20da : [1041249 , 660], # Crocodile
	0x20d3 : [1041250, 357], # Daemon
	0x20d6 : [1041251, 362], # Dragon
	0x20d7 : [1041252, 268], # EarthElemental
	0x20d8 : [1041253, 367], # Ettin
	0x20d9 : [1041254, 372], # Gargoyle
	0x20f5 : [1041255, 158], # Gorilla
	0x20f8 : [1041256, 1001], # Lich
	0x20de : [1041257, 417], # Lizardman	
	0x20df : [1041258, 427], # Ogre
	0x20e0 : [1041259, 1114], # Orc
	0x20e3 : [1041260, 437], # Ratman
	0x20e7 : [1041261, 1165], # Skeleton
	0x20e9 : [1041262, 461], # Troll
	0x2103 : [1041263, 120], # Cow
	0x20ec : [1041264, 471], # Zombie
	0x20f6 : [1041265, 1011], # Llama
	0x2133 : [1049742, 634], # Ophidian
	0x20fa : [1049743, 442], # Reaper
	0x20f9 : [1049744, 422], # Mongbat
	0x20f4 : [1049768, 377], # Gazer
	0x20f3 : [1049769, 838], # FireElemental
	0x2122 : [1049770, 229], # Wolf
	0x3ffe : [1063488, 205], # Phillip's Steed
	0x25ba : [1070819, 138]  # Seahorse
}

def onUse(char, item):
	if not item.getoutmostchar() == char:
		char.socket.clilocmessage(502691) # You must be the owner to use this.
		return True
	sendgump(char, item)
	return True

def sendgump(char, item):
	dialog = wolfpack.gumps.cGump()
	dialog.setCallback(gump_callback)
	dialog.setArgs([item.serial])

	dialog.addBackground(0xA28, 300, 150)

	cliloc = 1011034
	if item.hastag('active'):
		cliloc = 1011035

	dialog.addXmfHtmlGump(45, 20, 300, 35, cliloc) # [De]Activate this item

	dialog.addButton(40, 53, 0xFA5, 0xFA7, 1)
	dialog.addXmfHtmlGump(80, 55, 65, 35, 1011036) # OKAY

	dialog.addButton(150, 53, 0xFA5, 0xFA7, 0)
	dialog.addXmfHtmlGump(190, 55, 100, 35, 1011012) # CANCEL
	dialog.send(char.socket)

def gump_callback(player, arguments, response):
	item = wolfpack.finditem(arguments[0])
	if not item:
		return False
	if response.button == 1:
		if item.hastag('active'):
			item.deltag('active')
		else:
			item.settag('active', 0)
			if not item.lockeddown:
				player.socket.clilocmessage(502693) # Remember, this only works when locked down.
		item.resendtooltip()
	else:
		player.socket.clilocmessage(502694) # Cancelled action.
	return True

def onShowTooltip(player, object, tooltip):
	if not object.id in ids.keys():
		return False
	tooltip.reset()
	name = ids[object.id][0]
	tooltip.add(name, '')
	if object.hastag('active'):
		tooltip.add(502695, '') # turned on
	else:
		tooltip.add(502696, '') # turned off
	return True
