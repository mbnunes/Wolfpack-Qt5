#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: DarkStorm                      #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#

"""
	\library wolfpack
	\description Contains several server functions.
"""
import _wolfpack
from wolfpack.consts import *
from types import *

charbase = _wolfpack.charbase
additem = _wolfpack.additem
addnpc = _wolfpack.addnpc
guilds = _wolfpack.guilds
newguild = _wolfpack.newguild
findguild = _wolfpack.findguild
queueaction = _wolfpack.queueaction
registerpackethook = _wolfpack.registerpackethook
charcount = _wolfpack.charcount
itemcount = _wolfpack.itemcount
npccount = _wolfpack.npccount
playercount = _wolfpack.playercount
getdefinition = _wolfpack.getdefinition
getdefinitions = _wolfpack.getdefinitions
hasevent = _wolfpack.hasevent
hasnamedevent = _wolfpack.hasnamedevent
callevent = _wolfpack.callevent
callnamedevent = _wolfpack.callnamedevent
statics = _wolfpack.statics
finditem = _wolfpack.finditem
findchar = _wolfpack.findchar
findmulti = _wolfpack.findmulti
region = _wolfpack.region
itemregion = _wolfpack.itemregion
charregion = _wolfpack.charregion
map = _wolfpack.map
hasmap = _wolfpack.hasmap
statics =  _wolfpack.statics
items = _wolfpack.items
chars = _wolfpack.chars
itemiterator = _wolfpack.itemiterator
chariterator = _wolfpack.chariterator
addtimer = _wolfpack.addtimer
list = _wolfpack.list
registerglobal = _wolfpack.registerglobal
registercommand = _wolfpack.registercommand
tiledata = _wolfpack.tiledata
landdata = _wolfpack.landdata
coord = _wolfpack.coord
effect = _wolfpack.effect
serverversion = _wolfpack.serverversion
isstarting = _wolfpack.isstarting
isrunning = _wolfpack.isrunning
isreloading = _wolfpack.isreloading
isclosing = _wolfpack.isclosing
addmulti = _wolfpack.addmulti
newnpc = _wolfpack.newnpc
newitem = _wolfpack.newitem
newplayer = _wolfpack.newplayer
tickcount = _wolfpack.tickcount
currenttime = _wolfpack.currenttime
packet = _wolfpack.packet
getoption = _wolfpack.getoption
setoption = _wolfpack.setoption
playercount = _wolfpack.playercount
npccount = _wolfpack.npccount

"""
	\function wolfpack.findobject
	\param serial The serial number of the object.
	\return An <object id="item">item</object>, <object id="char">char</object>, or None object.
	\description This function tries to find an item or character with the given serial and
	returns an object if either is found. Otherwise it returns None.
"""
def findobject(serial):
	if serial == -1 or serial == 0:
		return None
	elif serial >= 0x40000000:
		return finditem(serial)
	else:
		return findchar(serial)
