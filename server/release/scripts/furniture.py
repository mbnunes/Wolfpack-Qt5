
import wolfpack

def onShowTooltip(viewer, object, tooltip):
        furniture_tooltip( viewer, object, tooltip )

def furniture_tooltip(viewer, object, tooltip):
    	# Exceptional item?
	if object.hastag('exceptional'):
		tooltip.add(1060636, '')

		# 1050043: Crafted by ~param~
		serial = int(object.gettag('exceptional'))
		crafter = wolfpack.findchar(serial)
		if crafter:
			tooltip.add(1050043, crafter.name)
