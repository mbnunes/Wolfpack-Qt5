import random

def onShowTooltip(player, object, tooltip):
	if object.hastag('animalweight') and object.gettag('animalweight') >= 20:
		if object.hastag('hunter'):
			serial = int(object.gettag('hunter'))
			hunter = wolfpack.findchar(serial)
			if hunter:
				tooltip.add( 1070857, hunter.orgname ) # Caught by ~1_fisherman~
		tooltip.add( 1070858, str(object.gettag('animalweight')) ) # ~1_weight~ stones

def onCreate(object, id):
	weight = random.randint(3, 200)
	object.settag('animalweight', weight)
