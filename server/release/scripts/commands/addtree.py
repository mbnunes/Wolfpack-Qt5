#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts
#  ((    _/{  "-;  | Created by: Dreoth
#   )).-' {{ ;'`   | Revised by:
#  ( (  ;._ \\ ctr | Last Modification: Created
#===============================================================#
# .addtree Command
#===============================================================#

import wolfpack
import random
from wolfpack.consts import WPDT_ITEM
from wolfpack.utilities import hex2dec
from wolfpack import console


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

jungletreeindex = [ 'jungle1', 'jungle2', 'jungle3', 'jungle4', 'jungle5', 'jungle6', 'jungle7' ]

treeindex = ['cca','ccb','ccc','ccd','cd0','cd3','cd6','cd8','cda','cdd','ce0','ce3','ce6','cf8','cfe','d01','d94','d98','d9c','da0','da4','da8']
forestlist = ['ccd','cd0','cd3','cd6','cd8','cda','cdd','ce0','ce3','ce6']
swamplist = ['cf8','cfe','d01']
fruitlist = ['d94', 'd98', 'd9c', 'da0', 'da4', 'da8']

trees = \
{
	'cca': [ 'cca', 'cce' ],
	'ccb': [ 'ccb', 'cce' ],
	'ccc': [ 'ccc', 'cce' ],
	'ccd': [ 'ccd', 'cce' ],
	'cd0': [ 'cd0', 'cd1' ],
	'cd3': [ 'cd3', 'cd4' ],
	'cd6': [ 'cd6', 'cd7' ],
	'cd8': [ 'cd8', 'cd9' ],
	'cda': [ 'cda', 'cdb' ],
	'cdd': [ 'cdd', 'cde' ],
	'ce0': [ 'ce0', 'ce1' ],
	'ce3': [ 'ce3', 'ce4' ],
	'ce6': [ 'ce6', 'ce7' ],
	'cf8': [ 'cf8', 'cf9' ], # Cypress
	'cfe': [ 'cfe', 'cff' ], # Cypress
	'd01': [ 'd01', 'd02' ], # Cypress
	'd94': [ 'd94', 'd95' ], # Apple
	'd98': [ 'd98', 'd99' ], # Apple
	'd9c': [ 'd9c', 'd9d' ], # Peach
	'da0': [ 'da0', 'da1' ], # Peach
	'da4': [ 'da4', 'da5' ], # Pear
	'da8': [ 'da8', 'da9' ] # Pear
}

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
				object.moveto( target.item.pos )
		elif target.char:
			object.moveto( target.char.pos )
		else:
			object.moveto( target.pos )
		pos = object.pos
		pos.x = int( pos.x + xmod )
		pos.y = int( pos.y + ymod )
		object.moveto( pos )
		xmod += 1
		ymod -= 1
		object.update()
	return

def createtree(player, arguments, target):
	tree = wolfpack.additem( trees[arguments[0]][TREE] )
	leaves = wolfpack.additem( trees[arguments[0]][LEAVES] )
	if target.item:
		if target.item.type == 1:
			target.item.additem( tree, 1, 1, 0 )
			target.item.additem( leaves, 1, 1, 0 )
		elif target.item.container:
			target.item.container.additem( tree, 1, 1, 0 )
			target.item.container.additem( leaves, 1, 1, 0 )
		else:
			tree.moveto( target.item.pos )
			leaves.moveto( target.item.pos )
	elif target.char:
		tree.moveto( target.char.pos )
		leaves.moveto( target.char.pos )
	else:
		tree.moveto( target.pos )
		leaves.moveto( target.pos )

	tree.update()
	leaves.update()
	return

def createjungletree( player, arguments, target):
	treetype = arguments[0]

	if treetype == 'jungle1' or treetype == 'jungle2':
		base1 = wolfpack.additem( 'd41' )
		base2 = wolfpack.additem( 'd42' )
		base3 = wolfpack.additem( 'd43' )
		base4 = wolfpack.additem( 'd44' )
		base = [ base1, base2, base3, base4 ]
		addcomplextree( target, base, -2, 2)
		if treetype == 'jungle1':
			top1 = wolfpack.additem( 'd45' )
			top2 = wolfpack.additem( 'd46' )
			top3 = wolfpack.additem( 'd47' )
			top4 = wolfpack.additem( 'd48' )
			top5 = wolfpack.additem( 'd49' )
			top6 = wolfpack.additem( 'd4a' )
			top7 = wolfpack.additem( 'd4b' )
			top8 = wolfpack.additem( 'd4c' )
			top = [ top1, top2, top3, top4, top5, top6, top7, top8 ]
			addcomplextree( target, top, -4, 4)
		elif treetype == 'jungle2':
			top1 = wolfpack.additem( 'd4d' )
			top2 = wolfpack.additem( 'd4e' )
			top3 = wolfpack.additem( 'd4f' )
			top4 = wolfpack.additem( 'd50' )
			top5 = wolfpack.additem( 'd51' )
			top6 = wolfpack.additem( 'd52' )
			top7 = wolfpack.additem( 'd53' )
			top = [ top1, top2, top3, top4, top5, top6, top7 ]
			addcomplextree( target, top, -3, 3)

	if treetype == 'jungle3':
		base1 = wolfpack.additem( 'd57' )
		base2 = wolfpack.additem( 'd58' )
		base3 = wolfpack.additem( 'd59' )
		base4 = wolfpack.additem( 'd5a' )
		base5 = wolfpack.additem( 'd5b' )
		base = [ base1, base2, base3, base4, base5 ]
		addcomplextree( target, base, -2, 2)
		top1 = wolfpack.additem( 'd5c' )
		top2 = wolfpack.additem( 'd5d' )
		top3 = wolfpack.additem( 'd5e' )
		top4 = wolfpack.additem( 'd5f' )
		top5 = wolfpack.additem( 'd60' )
		top6 = wolfpack.additem( 'd61' )
		top7 = wolfpack.additem( 'd62' )
		top = [ top1, top2, top3, top4, top5, top6, top7 ]
		addcomplextree( target, top, -3, 3)

	if treetype == 'jungle4' or treetype == 'jungle5':
		base1 = wolfpack.additem( 'd6e' )
		base2 = wolfpack.additem( 'd6f' )
		base3 = wolfpack.additem( 'd70' )
		base4 = wolfpack.additem( 'd71' )
		base5 = wolfpack.additem( 'd72' )
		base = [ base1, base2, base3, base4, base5 ]
		addcomplextree( target, base, -2, 2)
		if treetype == 'jungle4':
			top1 = wolfpack.additem( 'd73' )
			top2 = wolfpack.additem( 'd74' )
			top3 = wolfpack.additem( 'd75' )
			top4 = wolfpack.additem( 'd76' )
			top5 = wolfpack.additem( 'd77' )
			top6 = wolfpack.additem( 'd78' )
			top7 = wolfpack.additem( 'd79' )
			top = [ top1, top2, top3, top4, top5, top6, top7 ]
			addcomplextree( target, top, -3, 3)
		elif treetype == 'jungle5':
			top1 = wolfpack.additem( 'd7a' )
			top2 = wolfpack.additem( 'd7b' )
			top3 = wolfpack.additem( 'd7c' )
			top4 = wolfpack.additem( 'd7d' )
			top5 = wolfpack.additem( 'd7e' )
			top6 = wolfpack.additem( 'd7f' )
			top7 = wolfpack.additem( 'd80' )
			top = [ top1, top2, top3, top4, top5, top6, top7 ]
			addcomplextree( target, top, -3, 3)

	if treetype == 'jungle6' or treetype == 'jungle7':
		base1 = wolfpack.additem( 'd84' )
		base2 = wolfpack.additem( 'd85' )
		base3 = wolfpack.additem( 'd86' )
		base = [ base1, base2, base3 ]
		addcomplextree( target, base, -1, 1)
		if treetype == 'jungle6':
			top1 = wolfpack.additem( 'd87' )
			top2 = wolfpack.additem( 'd88' )
			top3 = wolfpack.additem( 'd89' )
			top4 = wolfpack.additem( 'd8a' )
			top5 = wolfpack.additem( 'd8b' )
			top = [ top1, top2, top3, top4, top5 ]
			addcomplextree( target, top, -2, 2)
		elif treetype == 'jungle7':
			top1 = wolfpack.additem( 'd8c' )
			top2 = wolfpack.additem( 'd8d' )
			top3 = wolfpack.additem( 'd8e' )
			top4 = wolfpack.additem( 'd8f' )
			top5 = wolfpack.additem( 'd90' )
			top = [ top1, top2, top3, top4, top5 ]
			addcomplextree( target, top, -2, 2)

	return

def createyewtree( player, arguments, target):

	# Tree
	yew1 = wolfpack.additem( '12b6' )
	yew2 = wolfpack.additem( '12b7' )
	yew3 = wolfpack.additem( '12b8' )
	yew4 = wolfpack.additem( '12b9' )
	yew5 = wolfpack.additem( '12ba' )
	yew6 = wolfpack.additem( '12bb' )
	yew7 = wolfpack.additem( '12bc' )
	yew8 = wolfpack.additem( '12bd' )
	yewtree = [yew1, yew2, yew3, yew4, yew5, yew6, yew7, yew8]
	xmod = -4
	ymod = 4
	addcomplextree( target, yewtree, xmod, ymod)
	# Leaves
	leaves1 = wolfpack.additem( '12be' )
	leaves2 = wolfpack.additem( '12bf' )
	leaves3 = wolfpack.additem( '12c0' )
	leaves4 = wolfpack.additem( '12c1' )
	leaves5 = wolfpack.additem( '12c2' )
	leaves6 = wolfpack.additem( '12c3' )
	leaves7 = wolfpack.additem( '12c4' )
	leaves8 = wolfpack.additem( '12c5' )
	leaves9 = wolfpack.additem( '12c6' )
	leaves10 = wolfpack.additem( '12c7' )
	yewleaves = [ leaves1, leaves2, leaves3, leaves4, leaves5, leaves6, leaves7, leaves8, leaves9, leaves10 ]
	xmod = -5
	ymod = 5
	addcomplextree( target, yewleaves, xmod, ymod)

	return

def addtree(socket, command, arguments):
	if len(arguments) > 0:
		item = str( arguments.strip() )
		if item == 'forest':
			if socket.hastag( 'last_foresttree' ):
				templist = []
				for choice in forestlist:
					if choice != str( socket.gettag( 'last_foresttree' ) ):
						templist += [ choice ]
				item = random.choice( templist )
				socket.settag( 'last_foresttree', str( item ) )
			else:
				item = random.choice( forestlist )
				socket.settag( 'last_foresttree', str( item ) )

			if item in trees:
				if wolfpack.getdefinition( WPDT_ITEM, trees[item][TREE] ) and wolfpack.getdefinition( WPDT_ITEM, trees[item][LEAVES] ):
					socket.sysmessage( "Where do you want to place the tree '%s', '%s' ?" % ( trees[item][TREE], trees[item][LEAVES ]) )
					socket.attachtarget( 'commands.addtree.createtree', [ item ] )
				return
		elif item == 'fruit':
			item = random.choice( fruitlist )
			if item in trees:
				if wolfpack.getdefinition( WPDT_ITEM, trees[item][TREE] ) and wolfpack.getdefinition( WPDT_ITEM, trees[item][LEAVES] ):
					socket.sysmessage( "Where do you want to place the tree '%s', '%s' ?" % ( trees[item][TREE], trees[item][LEAVES ]) )
					socket.attachtarget( 'commands.addtree.createtree', [ item ] )
				return
		elif item == 'swamp':
			item = random.choice( swamplist )
			if item in trees:
				if wolfpack.getdefinition( WPDT_ITEM, trees[item][TREE] ) and wolfpack.getdefinition( WPDT_ITEM, trees[item][LEAVES] ):
					socket.sysmessage( "Where do you want to place the tree '%s', '%s' ?" % ( trees[item][TREE], trees[item][LEAVES ]) )
					socket.attachtarget( 'commands.addtree.createtree', [ item ] )
				return
		elif item == 'random':
			if socket.hastag( 'last_randomtree' ):
				templist = []
				for choice in treeindex:
					if choice != str( socket.gettag( 'last_randomtree' ) ):
						templist += [ choice ]
				item = random.choice( templist )
				socket.settag( 'last_randomtree', str( item ) )
			else:
				item = random.choice( treeindex )
				socket.settag( 'last_randomtree', str( item ) )

			if item in trees:
				if wolfpack.getdefinition( WPDT_ITEM, trees[item][TREE] ) and wolfpack.getdefinition( WPDT_ITEM, trees[item][LEAVES] ):
					socket.sysmessage( "Where do you want to place the tree '%s', '%s' ?" % ( trees[item][TREE], trees[item][LEAVES ]) )
					socket.attachtarget( 'commands.addtree.createtree', [ item ] )
				return
		elif item == 'yew':
			socket.sysmessage( "Where do you want to place the yew tree?" )
			socket.attachitemtarget( 'commands.addtree.createyewtree', yewindex, 0, 0, 0, [] )
			return
		elif item in trees and not item in jungletreeindex:
			if wolfpack.getdefinition( WPDT_ITEM, trees[item][TREE] ) and wolfpack.getdefinition( WPDT_ITEM, trees[item][LEAVES] ):
				socket.sysmessage( "Where do you want to place the tree '%s', '%s' ?" % ( trees[item][TREE], trees[item][LEAVES]) )
				socket.attachtarget( 'commands.addtree.createtree', [ item ] )
		elif item in jungletreeindex and item in jungleindex:
			socket.sysmessage( "Where do you want to place the jungle tree [%s]?" %( item ) )
			socket.attachtarget(  'commands.addtree.createjungletree', [ item ] )
			return
		else:
			socket.sysmessage( "Usage: addtree [ id, yew, jungle[1-7], random, forest, fruit, swamp ]" )
	else:
		socket.sysmessage( "Usage: addtree [ id, yew, jungle[1-7], random, forest, fruit, swamp ]" )

def onLoad():
	wolfpack.registercommand( 'addtree', addtree )
