#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts
#  ((    _/{  "-;  | Created by: MagnusBr
#   )).-' {{ ;'`   | Revised by:
#  ( (  ;._ \\ ctr | Last Modification: Created
#===============================================================#
# Quest System
#===============================================================#

from wolfpack import tr
import wolfpack
from wolfpack.gumps import cGump

def openquestplayer(player):
	
	socket = player.socket

	dialog = cGump( nomove=1, x=100, y=30 )

	dialog.addResizeGump(37, 14, 9260, 445, 422)
	dialog.addTiledGump(81, 35, 358, 386, 9204, 0x991)
	dialog.addTiledGump(55, 32, 30, 391, 10460, 0x0)
	dialog.addTiledGump(439, 32, 30, 391, 10460, 0x0)
	dialog.addGump(-12, 290, 10402, 0)
	dialog.addGump(53, 137, 10411, 0)
	dialog.addGump(95, 39, 9005, 0)
	dialog.addTiledGump(40, 421, 440, 17, 10100, 0x0)
	dialog.addTiledGump(39, 15, 440, 17, 10100, 0x0)
	dialog.addGump(4, 16, 10421, 0)
	dialog.addGump(21, 4, 10420, 0)
	dialog.addGump(449, 162, 10411, 0)
	dialog.addGump(449, 323, 10412, 0)
	dialog.addGump(394, 52, 1417, 0)
	dialog.addGump(449, 3, 10410, 0)
	dialog.addGump(402, 61, 9012, 0)
	dialog.addText(135, 49, "Quest Log", 1149)
	dialog.addTiledGump(139, 69, 161, 2, 2700, 0x995)
	dialog.addButton(338, 394, 12012, 12013, 0)

	dialog.addButton(391, 142, 9904, 9905, 1)
	dialog.addText(106, 142, "No Quest", 1149)
	dialog.addButton(391, 163, 9904, 9905, 2)
	dialog.addText(106, 163, "No Quest", 1149)
	dialog.addButton(391, 184, 9904, 9905, 3)
	dialog.addText(106, 184, "No Quest", 1149)
	dialog.addButton(391, 205, 9904, 9905, 4)
	dialog.addText(106, 205, "No Quest", 1149)
	dialog.addButton(391, 226, 9904, 9905, 5)
	dialog.addText(106, 226, "No Quest", 1149)
	dialog.addButton(391, 247, 9904, 9905, 6)
	dialog.addText(106, 247, "No Quest", 1149)
	dialog.addButton(391, 268, 9904, 9905, 7)
	dialog.addText(106, 268, "No Quest", 1149)
	dialog.addButton(391, 289, 9904, 9905, 8)
	dialog.addText(106, 289, "No Quest", 1149)
	dialog.addButton(391, 310, 9904, 9905, 9)
	dialog.addText(106, 310, "No Quest", 1149)
	dialog.addButton(391, 331, 9904, 9905, 10)
	dialog.addText(106, 331, "No Quest", 1149)

	dialog.setArgs( [] )
	dialog.setCallback( questplayerresponse )

	dialog.send( player.socket )

def questplayerresponse( char, args, target ):

	button = target.button

	if button == 1:

		char.socket.sysmessage("Quest System is not yet implemented.")
		return True

	if button == 2:

		char.socket.sysmessage("Quest System is not yet implemented.")
		return True

	if button == 3:

		char.socket.sysmessage("Quest System is not yet implemented.")
		return True

	if button == 4:

		char.socket.sysmessage("Quest System is not yet implemented.")
		return True

	if button == 5:

		char.socket.sysmessage("Quest System is not yet implemented.")
		return True

	if button == 6:

		char.socket.sysmessage("Quest System is not yet implemented.")
		return True

	if button == 7:

		char.socket.sysmessage("Quest System is not yet implemented.")
		return True

	if button == 8:

		char.socket.sysmessage("Quest System is not yet implemented.")
		return True

	if button == 9:

		char.socket.sysmessage("Quest System is not yet implemented.")
		return True

	if button == 10:

		char.socket.sysmessage("Quest System is not yet implemented.")
		return True