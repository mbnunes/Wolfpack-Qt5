
import wolfpack
from skills import mining

# ingot script
def onShowTooltip(player, object, tooltip):
	name = 'Unknown Ingot'

	if object.hastag('resname'):
		resname = str(object.gettag('resname'))
		if mining.ORES.has_key(resname):
			name = mining.ORES[resname][mining.INGOTNAME]

	tooltip.reset()
	tooltip.add(1050039, "%u\t%s" % (object.amount, name))
