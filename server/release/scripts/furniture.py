import wolfpack
from wolfpack.consts import *
from wolfpack.weaponinfo import WEAPON_RESNAME_BONI as RESNAME_BONI

extended_carpentry = int( wolfpack.settings.getbool("General", "Extended Carpentry", False, True) )

def onShowTooltip( viewer, object, tooltip ):
	if extended_carpentry:
		# Reinsert the name if we need a wood prefix
		prefix1 = None
		if object.hastag('resname'):
			resname = str(object.gettag('resname'))
			if RESNAME_BONI.has_key(resname):
				resinfo = RESNAME_BONI[resname]
				if resinfo.has_key(MATERIALPREFIX):
					prefix1 = resinfo[MATERIALPREFIX]
		elif object.hasstrproperty( 'resname' ):
			resname = str( object.getstrproperty( 'resname' ) )
			if RESNAME_BONI.has_key(resname):
				resinfo = RESNAME_BONI[resname]
				if resinfo.has_key(MATERIALPREFIX):
					prefix1 = resinfo[MATERIALPREFIX]

		prefix2 = None
		if object.hastag('resname2'):
			resname = str(object.gettag('resname2'))
			if RESNAME_BONI.has_key(resname):
				resinfo = RESNAME_BONI[resname]
				if resinfo.has_key(MATERIALPREFIX):
					prefix2 = resinfo[MATERIALPREFIX]
		elif object.hasstrproperty( 'resname2' ):
			resname = str( object.getstrproperty( 'resname2' ) )
			if RESNAME_BONI.has_key(resname):
				resinfo = RESNAME_BONI[resname]
				if resinfo.has_key(MATERIALPREFIX):
					prefix2 = resinfo[MATERIALPREFIX]

		if len(object.name) == 0:
			itemname = '#' + str(1020000 + object.id)
		else:
			itemname = object.name

		if prefix1 and prefix2:
			tooltip.reset()
			tooltip.add(1053099, "%s %s\t%s" % (prefix1, prefix2, itemname))
		elif prefix1 and not prefix2:
			tooltip.reset()
			tooltip.add(1053099, "%s\t%s" % (prefix1, itemname))
		elif not prefix1 and prefix2:
			tooltip.reset()
			tooltip.add(1053099, "%s\t%s" % (prefix2, itemname))
		# For containers (hardcoded type), add count of items and total weight
		if object.type == 1:
			tooltip.add(1050044, "%i\t%i" % (object.countitem(), object.totalweight))
		if object.lockeddown:
			tooltip.add(501643, "")

	# Exceptional item?
	if object.hastag('exceptional'):
		tooltip.add(1060636, '')

		# 1050043: Crafted by ~param~
		serial = int(object.gettag('exceptional'))
		crafter = wolfpack.findchar(serial)
		if crafter:
			tooltip.add(1050043, crafter.orgname)
