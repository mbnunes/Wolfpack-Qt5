
import wolfpack
from wolfpack import console
from wolfpack.consts import *

#
# Add multiple instances of items based on tags
# defined for the item on creation.
#
def onAttach(object):
	if not object.isitem():
		return

	if not object.hastag('itemdef'):
		console.log(LOG_ERROR, "Missing itemdef tag for multibag object.\n")
		return

	if not object.hastag('itemamount'):
		console.log(LOG_ERROR, "Missing itemamount tag for multibag object.\n")
		return

	id = str(object.gettag('itemdef'))
	amount = int(object.gettag('itemamount'))

	for i in range(0, amount):
		item = wolfpack.additem(id)
		item.container = object

	object.deltag('itemdef')
	object.deltag('itemamount')
	object.removeevent('testing.multibag')
