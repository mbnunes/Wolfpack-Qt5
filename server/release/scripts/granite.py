from skills import mining
from wolfpack import tr

def onShowTooltip(player, object, tooltip):
	name = tr('Unknown Granite')

	if object.hastag('resname'):
		resname = str(object.gettag('resname'))
		if mining.ORES.has_key(resname):
			name = str(mining.ORES[resname][mining.GRANITETOOLTIP])

	if name.isdigit():
		tooltip.add(int(name), "")
	else:
		tooltip.add(1044607, '') # High Qualitiy Granite
		tooltip.add(1042971, name)
