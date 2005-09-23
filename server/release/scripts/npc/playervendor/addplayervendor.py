# -*- coding: iso-8859-15 -*-
# Definition des addplayervendor commands.
# $Header: /var/lib/cvs/ancientrealms/playervendor/addplayervendor.py,v 1.5 2004/11/11 23:14:17 valuial Exp $

#
# NOTE: This is a custom script conversion from my shard.
# If anyone wants to use this, customize it first!
#

"""
	\command addplayervendor
	\description Add an NPC and set the playervendor scripts
"""

import wolfpack
from wolfpack.consts import *

def addvendor(player, arguments, target):
	if target.item and target.item.container:
		player.socket.sysmessage("You can't add the npc there.")
		return

	owner = wolfpack.findchar(arguments[1])
	if not owner:
		player.socket.sysmessage(u'Ooops. Da ist was schief gelaufen. Oder der Owner wurde gerade gelöscht. So kann ich nicht Arbeiten!')
		return
	npc = wolfpack.addnpc(str(arguments[0]), target.pos)
	npc.owner = owner
	npc.invulnerable = True
	npc.update()
	
	player.log(LOG_MESSAGE, "Adds %s's Playervendor (0x%x) at %s.\n" % (owner.orgname, npc.serial, str(target.pos)))

def getowner(player, arguments, target):
	socket = player.socket
	
	if target.char:
		char = target.char
		vendors = 0
		for follower in char.followers:
			if follower.hasscript('npc.playervendor'):
				vendors = vendors + 1
		if vendors >= 2:
			player.socket.sysmessage(u'Sorry, der Spieler hat bereits %d Vendoren. Es sind maximal 2 erlaubt.')
			return
		if char.player:
			socket.sysmessage(u'Wo soll der Vendor hin?')
			socket.attachtarget('npc.playervendor.addplayervendor.addvendor', [arguments[0], char.serial])
			return
	socket.sysmessage(u'Das war wohl nichts... Bitte das nächste mal einen Player anclicken')


def addplayervendor(socket, command, arguments):
	if len(arguments) > 0:
		if arguments == 'female':
			socket.sysmessage(u'Wer soll der Besitzer des Vendors sein?')
			socket.attachtarget('npc.playervendor.addplayervendor.getowner', ['playervendor_female'])
		elif arguments == 'male':
			socket.sysmessage(u'Wer soll der Besitzer des Vendors sein?')
			socket.attachtarget('npc.playervendor.addplayervendor.getowner', ['playervendor_male'])
		else:
			socket.sysmessage(u'Diese Art/Rasse/Geschlecht von Vendor kenne ich nicht.')
			return 1
	else:
		socket.sysmessage(u'Usage: .addpv [type], Supportet types: male, female')
		
def onLoad():
	wolfpack.registercommand('addpv',addplayervendor)
