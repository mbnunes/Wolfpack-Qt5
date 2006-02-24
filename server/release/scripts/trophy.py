from taxidermykit import corpses, createTrophyDeed
from wolfpack.utilities import hex2dec
import deeds.carpentry_deed

def onShowTooltip(player, object, tooltip):
	if object.hastag('animalweight') and object.gettag('animalweight') >= 20:
		if object.hastag('hunter'):
			serial = int(object.gettag('hunter'))
			hunter = wolfpack.findchar(serial)
			if hunter:
				tooltip.add( 1070857, hunter.orgname ) # Caught by ~1_fisherman~
		tooltip.add( 1070858, str(object.gettag('animalweight')) ) # ~1_weight~ stones

def onUse(char, item):
	if not deeds.carpentry_deed.checkmulti(char):
		return False
	if not char.canreach(item, 1):
		char.socket.clilocmessage( 500295 ) # You are too far away to do that.
	corpse = str(getbase(char, item.id))
	if not corpse:
		return False
	createTrophyDeed( char, corpse, item )
	item.delete()
	return True

def getbase(char, id):
	for corpse in corpses.keys():
		trophyid = str(corpses[corpse][1])
		trophyid2 = str(corpses[corpse][1] + 0x7)
		if str(id) == trophyid or str(id) == trophyid2:
			return corpse
	return False
