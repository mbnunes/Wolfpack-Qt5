
import wolfpack
from skills import mining
from wolfpack import tr

# ingot script
def onShowTooltip(player, object, tooltip):
	name = tr('Unknown Ingot')

	if object.hastag('resname'):
		resname = str(object.gettag('resname'))
		if resname in mining.ORES:
			name = mining.ORES[resname][mining.INGOTNAME]

	tooltip.reset()
	tooltip.add(1050039, "%u\t%s" % (object.amount, name))
