#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: MagnusBr                       #
#   )).-' {{ ;'`   # Revised by: Naddel                         #
#  ( (  ;._ \\ ctr # Last Modification: Modified                #
#################################################################

import wolfpack

def onShowTooltip(viewer, object, tooltip):
	tooltip.add(1060584, str(object.gettag('remaining_uses')))

	poison = -1
	poisonUses = 0
	if object.hastag('poisoning_strength'):
		poison = object.gettag('poisoning_strength')
	if object.hastag('poisoning_uses'):
		poisonUses = object.gettag('poisoning_uses')
		
	if poison != -1 and poisonUses > 0:
		tooltip.add( 1062412 + poison, str(poisonUses) )
