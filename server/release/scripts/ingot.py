
from skills import mining

# ingot script
def onShowTooltip(player, object, tooltip):
	name = 'Unknown Ingot'
	
	if object.hastag('resname'):
		resname = str(object.gettag('resname'))
		if mining.oretable.has_key(resname):
			name = mining.oretable[resname][mining.INGOTNAME]

	tooltip.reset()
	tooltip.add(1050039, "%u\t%s" % (object.amount, name))
