#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts
#  ((    _/{  "-;  | Created by: Dreoth
#   )).-' {{ ;'`   | Revised by:
#  ( (  ;._ \\ ctr | Last Modification: Created
#===============================================================#
# .addtree Command
#===============================================================#

import wolfpack
from wolfpack.consts import *
from wolfpack.utilities import hex2dec
from wolfpack import console
from random import randint

"""
	\command addtree
	\description Adds a complete tree with leaves.
	\usage - <code>addtree ID[,yew, random]</code>
		ID is the base defintion id of the tree itself.
		yew places a yew tree.
		jungle[1-7] places a jungle tree.
		random places a random normal tree.
	\notes Green leaves will be added ontop of the tree.
"""
# Please leave the grayed out indexes for future reference incase itemtarget works better with multiple items.

treeindex = ['cca','ccb','ccc','ccd','cd0','cd3','cd6','cda','cdd','ce0','ce3','ce6','cf8','cfe','d01', 'd94', 'd98', 'd9c', 'da0', 'da4', 'da8']

jungleindex = \
{
	'jungle1': ['jungle1', [[hex2dec(0xd43), 0, 0, 0, 0]]],
	'jungle2': ['jungle2', [[hex2dec(0xd43), 0, 0, 0, 0]]],
	'jungle3': ['jungle2', [[hex2dec(0xd59), 0, 0, 0, 0]]],
	'jungle4': ['jungle3', [[hex2dec(0xd70), 0, 0, 0, 0]]],
	'jungle5': ['jungle4', [[hex2dec(0xd70), 0, 0, 0, 0]]],
	'jungle6': ['jungle5', [[hex2dec(0xd85), 0, 0, 0, 0]]],
	'jungle7': ['jungle6', [[hex2dec(0xd85), 0, 0, 0, 0]]]
}
'''
trees = \
{
	'cca': ['cca', 'cce', [[hex2dec(0xcca), 0, 0, 0, 0],[hex2dec(0xcce), 0, 0, 0, 0]] ],
	'ccb': ['ccb', 'cce', [[hex2dec(0xccb), 0, 0, 0, 0],[hex2dec(0xcce), 0, 0, 0, 0]] ],
	'ccc': ['ccc', 'cce', [[hex2dec(0xccc), 0, 0, 0, 0],[hex2dec(0xcce), 0, 0, 0, 0]] ],
	'ccd': ['ccd', 'cce', [[hex2dec(0xccd), 0, 0, 0, 0],[hex2dec(0xcce), 0, 0, 0, 0]] ],
	'cd0': ['cd0', 'cd1', [[hex2dec(0xcd0), 0, 0, 0, 0],[hex2dec(0xcd1), 0, 0, 0, 0]] ],
	'cd3': ['cd3', 'cd4', [[hex2dec(0xcd3), 0, 0, 0, 0],[hex2dec(0xcd4), 0, 0, 0, 0]] ],
	'cd6': ['cd6', 'cd7', [[hex2dec(0xcd6), 0, 0, 0, 0],[hex2dec(0xcd7), 0, 0, 0, 0]] ],
	'cd8': ['cd8', 'cd0', [[hex2dec(0xcd8), 0, 0, 0, 0],[hex2dec(0xcd0), 0, 0, 0, 0]] ],
	'cda': ['cda', 'cdb', [[hex2dec(0xcda), 0, 0, 0, 0],[hex2dec(0xcdb), 0, 0, 0, 0]] ],
	'cdd': ['cdd', 'cde', [[hex2dec(0xcdd), 0, 0, 0, 0],[hex2dec(0xcde), 0, 0, 0, 0]] ],
	'ce0': ['ce0', 'ce1', [[hex2dec(0xce0), 0, 0, 0, 0],[hex2dec(0xce1), 0, 0, 0, 0]] ],
	'ce3': ['ce3', 'ce4', [[hex2dec(0xce3), 0, 0, 0, 0],[hex2dec(0xce4), 0, 0, 0, 0]] ],
	'ce6': ['ce6', 'ce7', [[hex2dec(0xce6), 0, 0, 0, 0],[hex2dec(0xce7), 0, 0, 0, 0]] ],
	'cf8': ['cf8', 'cf9', [[hex2dec(0xcf8), 0, 0, 0, 0],[hex2dec(0xcf9), 0, 0, 0, 0]] ], # Cypress
	'cfe': ['cfe', 'cff', [[hex2dec(0xcfe), 0, 0, 0, 0],[hex2dec(0xcff), 0, 0, 0, 0]] ], # Cypress
	'd01': ['d01', 'd02', [[hex2dec(0xd01), 0, 0, 0, 0],[hex2dec(0xd02), 0, 0, 0, 0]] ], # Cypress
	'd94': ['d94', 'd95', [[hex2dec(0xd94), 0, 0, 0, 0],[hex2dec(0xd95), 0, 0, 0, 0]] ],
	'd98': ['d98', 'd99', [[hex2dec(0xd98), 0, 0, 0, 0],[hex2dec(0xd99), 0, 0, 0, 0]] ],
	'd9c': ['d9c', 'd9d', [[hex2dec(0xd9c), 0, 0, 0, 0],[hex2dec(0xd9d), 0, 0, 0, 0]] ],
	'da0': ['da0', 'da1', [[hex2dec(0xda0), 0, 0, 0, 0],[hex2dec(0xda1), 0, 0, 0, 0]] ],
	'da4': ['da4', 'da5', [[hex2dec(0xda4), 0, 0, 0, 0],[hex2dec(0xda5), 0, 0, 0, 0]] ],
	'da8': ['da8', 'da9', [[hex2dec(0xda8), 0, 0, 0, 0],[hex2dec(0xda9), 0, 0, 0, 0]] ]
}
'''
trees = \
{
	'cca': ['cca', 'cce', [[hex2dec(0xcca), 0, 0, 0, 0]] ],
	'ccb': ['ccb', 'cce', [[hex2dec(0xccb), 0, 0, 0, 0]] ],
	'ccc': ['ccc', 'cce', [[hex2dec(0xccc), 0, 0, 0, 0]] ],
	'ccd': ['ccd', 'cce', [[hex2dec(0xccd), 0, 0, 0, 0]] ],
	'cd0': ['cd0', 'cd1', [[hex2dec(0xcd0), 0, 0, 0, 0]] ],
	'cd3': ['cd3', 'cd4', [[hex2dec(0xcd3), 0, 0, 0, 0]] ],
	'cd6': ['cd6', 'cd7', [[hex2dec(0xcd6), 0, 0, 0, 0]] ],
	'cd8': ['cd8', 'cd0', [[hex2dec(0xcd8), 0, 0, 0, 0]] ],
	'cda': ['cda', 'cdb', [[hex2dec(0xcda), 0, 0, 0, 0]] ],
	'cdd': ['cdd', 'cde', [[hex2dec(0xcdd), 0, 0, 0, 0]] ],
	'ce0': ['ce0', 'ce1', [[hex2dec(0xce0), 0, 0, 0, 0]] ],
	'ce3': ['ce3', 'ce4', [[hex2dec(0xce3), 0, 0, 0, 0]] ],
	'ce6': ['ce6', 'ce7', [[hex2dec(0xce6), 0, 0, 0, 0]] ],
	'cf8': ['cf8', 'cf9', [[hex2dec(0xcf8), 0, 0, 0, 0]] ], # Cypress
	'cfe': ['cfe', 'cff', [[hex2dec(0xcfe), 0, 0, 0, 0]] ], # Cypress
	'd01': ['d01', 'd02', [[hex2dec(0xd01), 0, 0, 0, 0]] ], # Cypress
	'd94': ['d94', 'd95', [[hex2dec(0xd94), 0, 0, 0, 0]] ],
	'd98': ['d98', 'd99', [[hex2dec(0xd98), 0, 0, 0, 0]] ],
	'd9c': ['d9c', 'd9d', [[hex2dec(0xd9c), 0, 0, 0, 0]] ],
	'da0': ['da0', 'da1', [[hex2dec(0xda0), 0, 0, 0, 0]] ],
	'da4': ['da4', 'da5', [[hex2dec(0xda4), 0, 0, 0, 0]] ],
	'da8': ['da8', 'da9', [[hex2dec(0xda8), 0, 0, 0, 0]] ]
}
'''
yewindex = [ [hex2dec(0x12b6), -4, 4, 0, 0], [hex2dec(0x12b7), -3, 3, 0, 0], [hex2dec(0x12b8), -2, 2, 0, 0],
	[hex2dec(0x12b9), -1, 1, 0, 0], [hex2dec(0x12ba), 0, 0, 0, 0], [hex2dec(0x12bb), 1, -1, 0, 0],
	[hex2dec(0x12bc), 2, -2, 0, 0], [hex2dec(0x12bd), 3, -3, 0, 0]
]
'''
yewindex = [ [hex2dec(0x12ba), 0, 0, 0, 0] ]
yewindex2 = [ [hex2dec(0x12b9), -1, 1, 0, 0] ]

TREE = 0
LEAVES = 1

def addcomplextree( target, index, xmod, ymod):
	for object in index:
		if target.item:
			if target.item.type == 1:
				return
			elif target.item.container:
				return
			else:
				object.moveto(target.item.pos)
		elif target.char:
			object.moveto(target.char.pos)
		else:
			object.moveto(target.pos)
		pos = object.pos
		pos.x = int( hex2dec(pos.x) + xmod )
		pos.y = int( hex2dec(pos.y) + ymod )
		object.moveto(pos)
		xmod += 1
		ymod -= 1
		object.update()
	return

def createtree(player, arguments, target):
	tree = wolfpack.additem(str(trees[arguments[0]][TREE]))
	leaves = wolfpack.additem(str(trees[arguments[0]][LEAVES]))
	if target.item:
		if target.item.type == 1:
			target.item.additem(tree, 1, 1, 0)
			target.item.additem(leaves, 1, 1, 0)
		elif target.item.container:
			target.item.container.additem(tree, 1, 1, 0)
			target.item.container.additem(leaves, 1, 1, 0)
		else:
			tree.moveto(target.item.pos)
			leaves.moveto(target.item.pos)
	elif target.char:
		tree.moveto(target.char.pos)
		leaves.moveto(target.char.pos)
	else:
		tree.moveto(target.pos)
		leaves.moveto(target.pos)

	tree.update()
	leaves.update()
	return

def createjungletree( player, arguments, target):
	treetype = arguments[0]

	if treetype == 'jungle1' or treetype == 'jungle2':
		base1 = wolfpack.additem( str('d41') )
		base2 = wolfpack.additem( str('d42') )
		base3 = wolfpack.additem( str('d43') )
		base4 = wolfpack.additem( str('d44') )
		base = [ base1, base2, base3, base4 ]
		addcomplextree( target, base, -2, 2)
		if treetype == 'jungle1':
			top1 = wolfpack.additem( str('d45') )
			top2 = wolfpack.additem( str('d46') )
			top3 = wolfpack.additem( str('d47') )
			top4 = wolfpack.additem( str('d48') )
			top5 = wolfpack.additem( str('d49') )
			top6 = wolfpack.additem( str('d4a') )
			top7 = wolfpack.additem( str('d4b') )
			top8 = wolfpack.additem( str('d4c') )
			top = [ top1, top2, top3, top4, top5, top6, top7, top8 ]
			addcomplextree( target, top, -4, 4)
		elif treetype == 'jungle2':
			top1 = wolfpack.additem( str('d4d') )
			top2 = wolfpack.additem( str('d4e') )
			top3 = wolfpack.additem( str('d4f') )
			top4 = wolfpack.additem( str('d50') )
			top5 = wolfpack.additem( str('d51') )
			top6 = wolfpack.additem( str('d52') )
			top7 = wolfpack.additem( str('d53') )
			top = [ top1, top2, top3, top4, top5, top6, top7 ]
			addcomplextree( target, top, -3, 3)

	if treetype == 'jungle3':
		base1 = wolfpack.additem( str('d57') )
		base2 = wolfpack.additem( str('d58') )
		base3 = wolfpack.additem( str('d59') )
		base4 = wolfpack.additem( str('d5a') )
		base5 = wolfpack.additem( str('d5b') )
		base = [ base1, base2, base3, base4, base5 ]
		addcomplextree( target, base, -2, 2)
		top1 = wolfpack.additem( str('d5c') )
		top2 = wolfpack.additem( str('d5d') )
		top3 = wolfpack.additem( str('d5e') )
		top4 = wolfpack.additem( str('d5f') )
		top5 = wolfpack.additem( str('d60') )
		top6 = wolfpack.additem( str('d61') )
		top7 = wolfpack.additem( str('d62') )
		top = [ top1, top2, top3, top4, top5, top6, top7 ]
		addcomplextree( target, top, -3, 3)

	if treetype == 'jungle4' or treetype == 'jungle5':
		base1 = wolfpack.additem( str('d6e') )
		base2 = wolfpack.additem( str('d6f') )
		base3 = wolfpack.additem( str('d70') )
		base4 = wolfpack.additem( str('d71') )
		base5 = wolfpack.additem( str('d72') )
		base = [ base1, base2, base3, base4, base5 ]
		addcomplextree( target, base, -2, 2)
		if treetype == 'jungle4':
			top1 = wolfpack.additem( str('d73') )
			top2 = wolfpack.additem( str('d74') )
			top3 = wolfpack.additem( str('d75') )
			top4 = wolfpack.additem( str('d76') )
			top5 = wolfpack.additem( str('d77') )
			top6 = wolfpack.additem( str('d78') )
			top7 = wolfpack.additem( str('d79') )
			top = [ top1, top2, top3, top4, top5, top6, top7 ]
			addcomplextree( target, top, -3, 3)
		elif treetype == 'jungle5':
			top1 = wolfpack.additem( str('d7a') )
			top2 = wolfpack.additem( str('d7b') )
			top3 = wolfpack.additem( str('d7c') )
			top4 = wolfpack.additem( str('d7d') )
			top5 = wolfpack.additem( str('d7e') )
			top6 = wolfpack.additem( str('d7f') )
			top7 = wolfpack.additem( str('d80') )
			top = [ top1, top2, top3, top4, top5, top6, top7 ]
			addcomplextree( target, top, -3, 3)

	if treetype == 'jungle6' or treetype == 'jungle7':
		base1 = wolfpack.additem( str('d84') )
		base2 = wolfpack.additem( str('d85') )
		base3 = wolfpack.additem( str('d86') )
		base = [ base1, base2, base3 ]
		addcomplextree( target, base, -1, 1)
		if treetype == 'jungle6':
			top1 = wolfpack.additem( str('d87') )
			top2 = wolfpack.additem( str('d88') )
			top3 = wolfpack.additem( str('d89') )
			top4 = wolfpack.additem( str('d8a') )
			top5 = wolfpack.additem( str('d8b') )
			top = [ top1, top2, top3, top4, top5 ]
			addcomplextree( target, top, -2, 2)
		elif treetype == 'jungle7':
			top1 = wolfpack.additem( str('d8c') )
			top2 = wolfpack.additem( str('d8d') )
			top3 = wolfpack.additem( str('d8e') )
			top4 = wolfpack.additem( str('d8f') )
			top5 = wolfpack.additem( str('d90') )
			top = [ top1, top2, top3, top4, top5 ]
			addcomplextree( target, top, -2, 2)

	return

def createyewtree( player, arguments, target):

	# Tree
	yew1 = wolfpack.additem( str('12b6') )
	yew2 = wolfpack.additem( str('12b7') )
	yew3 = wolfpack.additem( str('12b8') )
	yew4 = wolfpack.additem( str('12b9') )
	yew5 = wolfpack.additem( str('12ba') )
	yew6 = wolfpack.additem( str('12bb') )
	yew7 = wolfpack.additem( str('12bc') )
	yew8 = wolfpack.additem( str('12bd') )
	yewtree = [yew1, yew2, yew3, yew4, yew5, yew6, yew7, yew8]
	xmod = -4
	ymod = 4
	addcomplextree( target, yewtree, xmod, ymod)
	# Leaves
	leaves1 = wolfpack.additem( str('12be') )
	leaves2 = wolfpack.additem( str('12bf') )
	leaves3 = wolfpack.additem( str('12c0') )
	leaves4 = wolfpack.additem( str('12c1') )
	leaves5 = wolfpack.additem( str('12c2') )
	leaves6 = wolfpack.additem( str('12c3') )
	leaves7 = wolfpack.additem( str('12c4') )
	leaves8 = wolfpack.additem( str('12c5') )
	leaves9 = wolfpack.additem( str('12c6') )
	leaves10 = wolfpack.additem( str('12c7') )
	yewleaves = [ leaves1, leaves2, leaves3, leaves4, leaves5, leaves6, leaves7, leaves8, leaves9, leaves10 ]
	xmod = -5
	ymod = 5
	addcomplextree( target, yewleaves, xmod, ymod)

	return

def addtree(socket, command, arguments):
	if len(arguments) > 0:
		item = str(arguments)
		if item == 'random':
			randomtrees = [ randint(0,2), randint(0,12), randint(2,12), randint(0,20), randint(13,15), randint(16,20) ]
			randomtree = randint( 0, 5 )
			item = str( treeindex[ int( randomtrees[ randomtree ] ) ] )
			if trees[item]:
				if wolfpack.getdefinition(WPDT_ITEM, trees[item][TREE]) and wolfpack.getdefinition(WPDT_ITEM, trees[item][LEAVES]):
					socket.sysmessage( "Where do you want to place the tree '%s', '%s' ?" % ( trees[item][TREE], trees[item][LEAVES]) )
					socket.attachitemtarget( 'commands.addtree.createtree', trees[item][2], 0, 0, 0, [item])
				return
		elif item == 'yew':
			socket.sysmessage( "Where do you want to place the yew tree?" )
			socket.attachitemtarget( 'commands.addtree.createyewtree', yewindex, 0, 0, 0, [])
			return
		elif (item !='jungle1' and item !='jungle2' and item !='jungle3' and item !='jungle4' and item !='jungle5' and item !='jungle6' and item !='jungle7') and trees[item]:
			if wolfpack.getdefinition(WPDT_ITEM, trees[item][TREE]) and wolfpack.getdefinition(WPDT_ITEM, trees[item][LEAVES]):
				socket.sysmessage( "Where do you want to place the tree '%s', '%s' ?" % ( trees[item][TREE], trees[item][LEAVES]) )
				socket.attachitemtarget( 'commands.addtree.createtree', trees[item][2], 0, 0, 0, [item])
		elif jungleindex[item]:
			socket.sysmessage( "Where do you want to place the jungle tree [%s]?" %(item) )
			socket.attachitemtarget( 'commands.addtree.createjungletree', [jungleindex[item][1]], 0, 0, 0, [jungleindex[item][0]] )
			return
		else:
			socket.sysmessage( "Usage: addtree [id, yew, jungle[1-7], random")
	else:
		socket.sysmessage( "Usage: addtree [id, yew, jungle[1-7], random")

def onLoad():
	wolfpack.registercommand('addtree', addtree)
