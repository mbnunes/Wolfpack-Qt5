
import housing.house

def escapeHtml(text):
	text = text.replace('<', '&lt')
	text = text.replace('>', '&gt')
	return text

#
# Show a special tooltip for house signs
#
def onShowTooltip(player, sign, tooltip):
	tooltip.reset()
	tooltip.add(1061638, "") # A house sign
	
	if not sign.multi:
		return
	
	name = 'nothing'
	if len(sign.name) != 0:
		name = escapeHtml(sign.name)
	tooltip.add(1061639, name)
		
	owner = sign.multi.owner
	if not owner:
		tooltip.add(1061640, "nobody")
	else:
		tooltip.add(1061640, owner.orgname)
		
	public = housing.house.isPublic(sign.multi)
	if public:
		tooltip.add(1061641, "")
	else:
		tooltip.add(1061642, "")
