#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts
#  ((    _/{  "-;  | Created by: Dreoth
#   )).-' {{ ;'`   | Revised by:
#  ( (  ;._ \\ ctr | Last Modification: Created
#===============================================================#
# .adddecor Command
#===============================================================#

import wolfpack
import random
from wolfpack.consts import WPDT_ITEM
from wolfpack.utilities import hex2dec
from wolfpack import console


"""
	\command adddecor
	\description Adds a decoration object, such as a tree or flowers.
	\usage - <code>adddecor [ tree, ground ]</code>
		Tree allows tree types.
		<code>adddecor tree [ id, yew, jungle, random, forest, fruit, swamp, log[1-2] ]</code>
		Ground allows ground items.
		<code>adddecor ground [ forest, swamp, plains, jungle, desert ]</code>
	\notes Green leaves will be added ontop of the trees.
"""

# Tree Lists
treeindex = [ 'cca', 'ccb', 'ccc', 'ccd', 'cd0', 'cd3', 'cd6', 'cd8', 'cda', 'cdd', 'ce0', 'ce3', 'ce6', 'cf8', 'cfe', 'd01', 'd94', 'd98', 'd9c', 'da0', 'da4', 'da8' ]
jungletreeindex = [ 'jungle1', 'jungle2', 'jungle3', 'jungle4', 'jungle5', 'jungle6', 'jungle7' ]
forestlist = [ 'ccd', 'cd0', 'cd3', 'cd6', 'cd8', 'cda', 'cdd', 'ce0', 'ce3', 'ce6' ]
swamplist = [ 'cf8', 'cfe', 'd01' ]
fruitlist = [ 'd94', 'd98', 'd9c', 'da0', 'da4', 'da8' ]
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
logs = \
{
	'log1': [ 'cf3', 'cf4' ], # Y + 1
	'log2': [ 'cf5', 'cf6', 'cf7' ] # X + 1
}

# Ground Decorations

forest_decor = [
	'c84', # Foxglove Flowers
	'c85', # Orfluer Flowers
	'c87', # Champion Flowers
	'c88', # Snowdrops
	'c89', # Champion Flowers
	'c8a', # Foxglove Flowers
	'c8b', # White Flowers
	'c8c', # White Flowers
	'cdd', # White Poppies
	'c8e', # Snowdrops
	'cac', # Grasses
	'cad', # Grasses
	'cae', # Grasses
	'caf', # Grasses
	'cb0', # Grasses
	'cb1', # Grasses
	'cb2', # Grasses
	'cb3', # Grasses
	'cb4', # Grasses
	'cb5', # Grasses
	'cb6', # Grasses
	'cb7', # Cattails
	'cb8', # Cattails
	'cb9', # Grasses
	'cba', # Grasses
	'cbe', # Poppies
	'cbf', # Poppies
	'cc0', # Orfluer Flowers
	'cc1', # Orfluer Flowers
	'cc3', # Muck
	'cc5', # Grasses
	'cc6', # Grasses
	'cc7', # Weed
	'cc8', # Juniper Bush
	'cc9', # Spider Tree
	'd3f', # Brambles
	'd40', # Brambles
	'd29', # Flowers
	'd2b', # Flowers
	'd2d', # Flowers
	'd36', # Flowers
	'd2f' # Flowers
]
plains_decor = [
	'c84', # Foxglove Flowers
	'c85', # Orfluer Flowers
	'c87', # Champion Flowers
	'c88', # Snowdrops
	'c89', # Champion Flowers
	'c8a', # Foxglove Flowers
	'c8b', # White Flowers
	'c8c', # White Flowers
	'cdd', # White Poppies
	'c8e', # Snowdrops
	'cac', # Grasses
	'cad', # Grasses
	'cae', # Grasses
	'caf', # Grasses
	'cb0', # Grasses
	'cb1', # Grasses
	'cb2', # Grasses
	'cb3', # Grasses
	'cb4', # Grasses
	'cb5', # Grasses
	'cb6', # Grasses
	'cb7', # Cattails
	'cb8', # Cattails
	'cb9', # Grasses
	'cba', # Grasses
	'cbe', # Poppies
	'cbf', # Poppies
	'cc0', # Orfluer Flowers
	'cc1', # Orfluer Flowers
	'cc3', # Muck
	'cc5', # Grasses
	'cc6', # Grasses
	'cc7', # Weed
	'd29', # Flowers
	'd2b', # Flowers
	'd2d', # Flowers
	'd36', # Flowers
	'd2f' # Flowers
]
desert_decor = [
	'd25', # Cactus
	'd26', # Cactus
	'd27', # Cactus
	'd28', # Cactus
	'd29', # Flowers
	'd2a', # Cactus
	'd2b', # Flowers
	'd2c', # Cactus
	'd2d', # Flowers
	'd2e', # Cactus
	'd2f' # Flowers
	'd30', # Century Plant
	'd31', # Century Plant
	'd36', # Flowers
	'd38', # Yucca
	'd37', # Yucca
	'd35', # Pipe Cactus
	'd34', # Morning Glories
	'd33', # Grasses
	'd32' # Grasses
]
swamp_decor = [
]
jungle_decor = [

]

# Constants
TREE = 0
LEAVES = 1

# Adds a complex tree
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

def createjungletree( player, args, target):
	treetype = args[0]
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
			return True
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
			return True
	elif treetype == 'jungle3':
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
		return True
	elif treetype == 'jungle4' or treetype == 'jungle5':
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
			return True
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
			return True
	elif treetype == 'jungle6' or treetype == 'jungle7':
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
			return True
		elif treetype == 'jungle7':
			top1 = wolfpack.additem( 'd8c' )
			top2 = wolfpack.additem( 'd8d' )
			top3 = wolfpack.additem( 'd8e' )
			top4 = wolfpack.additem( 'd8f' )
			top5 = wolfpack.additem( 'd90' )
			top = [ top1, top2, top3, top4, top5 ]
			addcomplextree( target, top, -2, 2)
			return True
	return False

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
	return True

def createlog( player, args, target ):
	#added by Jim 20040814
	logtype = args[0]
	if logtype == 'log1':
		log1 = wolfpack.additem( 'cf3' )
		log1.moveto( target.pos )
		log2 = wolfpack.additem( 'cf4' )
		log2.pos = "%i,%i,%i,%i" % ( log1.pos.x , log1.pos.y + 1, log1.pos.z, log1.pos.map )
		log1.update()
		log2.update()
		return True
	elif logtype == 'log2':
		log1 = wolfpack.additem( 'cf6' )
		log1.moveto( target.pos )
		log2 = wolfpack.additem( 'cf5' )
		log2.pos = "%i,%i,%i,%i" % ( log1.pos.x - 1, log1.pos.y, log1.pos.z, log1.pos.map )
		log3 = wolfpack.additem( 'cf7' )
		log3.pos = "%i,%i,%i,%i" % ( log1.pos.x + 1, log1.pos.y, log1.pos.z, log1.pos.map )
		log1.update()
		log2.update()
		log3.update()
		return True
	return False

def createground( player, args, target ):
	item = wolfpack.additem( args[0] )
	item.pos = target.pos
	item.update()
	return True

def adddecor( socket, command, args ):
	if len(arguments) > 0:
		args= str( args.strip() )
		args = args.split( ' ' )
		if len( args ) == 2:
			( key, value ) = args
			# Tree Lists
			if key == 'tree':
				# Random Forest Trees
				if value == 'forest':
					if socket.hastag( 'last_tree_forest' ):
						templist = []
						for choice in forestlist:
							if choice != str( socket.gettag( 'last_tree_forest' ) ):
								templist += [ choice ]
						item = random.choice( templist )
						socket.settag( 'last_tree_forest', str( item ) )
					else:
						item = random.choice( forestlist )
						socket.settag( 'last_tree_forest', str( item ) )
					if item in trees:
						if wolfpack.getdefinition( WPDT_ITEM, trees[item][TREE] ) and wolfpack.getdefinition( WPDT_ITEM, trees[item][LEAVES] ):
							socket.sysmessage( "Where do you want to place the tree '%s', '%s' ?" % ( trees[item][TREE], trees[item][LEAVES ]) )
							socket.attachtarget( 'commands.adddecor.createtree', [ item ] )
						return True
				# Random Fruit Tree
				elif value == 'fruit':
					item = random.choice( fruitlist )
					if socket.hastag( 'last_tree_fruit' ):
						templist = []
						for choice in fruitlist:
							if choice != str( socket.gettag( 'last_tree_fruit' ) ):
								templist += [ choice ]
						item = random.choice( templist )
						socket.settag( 'last_tree_fruit', str( item ) )
					else:
						item = random.choice( fruitlist )
						socket.settag( 'last_tree_fruit', str( item ) )
					if item in trees:
						if wolfpack.getdefinition( WPDT_ITEM, trees[item][TREE] ) and wolfpack.getdefinition( WPDT_ITEM, trees[item][LEAVES] ):
							socket.sysmessage( "Where do you want to place the tree '%s', '%s' ?" % ( trees[item][TREE], trees[item][LEAVES ]) )
							socket.attachtarget( 'commands.adddecor.createtree', [ item ] )
						return True
				# Random Swamp Tree
				elif value == 'swamp':
					item = random.choice( swamplist )
					if socket.hastag( 'last_tree_swamp' ):
						templist = []
						for choice in swamplist:
							if choice != str( socket.gettag( 'last_tree_swamp' ) ):
								templist += [ choice ]
						item = random.choice( templist )
						socket.settag( 'last_tree_swamp', str( item ) )
					else:
						item = random.choice( swamplist )
						socket.settag( 'last_tree_swamp', str( item ) )
					if item in trees:
						if wolfpack.getdefinition( WPDT_ITEM, trees[item][TREE] ) and wolfpack.getdefinition( WPDT_ITEM, trees[item][LEAVES] ):
							socket.sysmessage( "Where do you want to place the tree '%s', '%s' ?" % ( trees[item][TREE], trees[item][LEAVES ]) )
							socket.attachtarget( 'commands.adddecor.createtree', [ item ] )
						return True
				# Random Trees
				elif value == 'random':
					if socket.hastag( 'last_tree_random' ):
						templist = []
						for choice in treeindex:
							if choice != str( socket.gettag( 'last_tree_random' ) ):
								templist += [ choice ]
						item = random.choice( templist )
						socket.settag( 'last_tree_random', str( item ) )
					else:
						item = random.choice( treeindex )
						socket.settag( 'last_tree_random', str( item ) )
					if item in trees:
						if wolfpack.getdefinition( WPDT_ITEM, trees[item][TREE] ) and wolfpack.getdefinition( WPDT_ITEM, trees[item][LEAVES] ):
							socket.sysmessage( "Where do you want to place the tree '%s', '%s' ?" % ( trees[item][TREE], trees[item][LEAVES ]) )
							socket.attachtarget( 'commands.adddecor.createtree', [ item ] )
						return True
				# Yew Trees
				elif value == 'yew':
					socket.sysmessage( "Where do you want to place the yew tree?" )
					socket.attachtarget( 'commands.adddecor.createyewtree', [] )
					return True
				# Tree Logs log1, log2
				elif value in [ 'log1', 'log2' ]: #added by Jim 20040814
					socket.sysmessage( "Where do you want to place the fallen log [%s]?" % ( item ) )
					socket.attachtarget( 'commands.adddecor.createlog', [ item ] )
					return True
				# Jungle Trees
				elif value == 'jungle':
					if socket.hastag( 'last_tree_jungle' ):
						templist = []
						for choice in jungleindex:
							if choice != str( socket.gettag( 'last_tree_jungle' ) ):
								templist += [ choice ]
						item = random.choice( templist )
						socket.settag( 'last_tree_jungle', str( item ) )
					else:
						item = random.choice( jungleindex )
						socket.settag( 'last_tree_jungle', str( item ) )
					socket.sysmessage( "Where do you want to place the jungle tree [%s]?" %( item ) )
					socket.attachtarget(  'commands.adddecor.createjungletree', [ item ] )
					return True
				# Specific Tree
				elif value in trees:
					if wolfpack.getdefinition( WPDT_ITEM, trees[item][TREE] ) and wolfpack.getdefinition( WPDT_ITEM, trees[item][LEAVES] ):
						socket.sysmessage( "Where do you want to place the tree '%s', '%s' ?" % ( trees[item][TREE], trees[item][LEAVES]) )
						socket.attachtarget( 'commands.adddecor.createtree', [ item ] )
						return True
				else:
					socket.sysmessage( "Usage: adddecor tree [ id, yew, jungle, random, forest, fruit, swamp, log[1-2] ]" )
					return False
			elif key == 'ground':
				# Forest Ground Decoration
				if value == "forest":
					if socket.hastag( 'last_ground_forest' ):
						templist = []
						for choice in forest_decor:
							if choice != str( socket.gettag( 'last_ground_forest' ) ):
								templist += [ choice ]
						item = random.choice( templist )
						socket.settag( 'last_ground_forest', str( item ) )
					else:
						item = random.choice( forest_decor )
						socket.settag( 'last_ground_forest', str( item ) )
					socket.attachtarget( 'commands.adddecor.createground', [ item ] )
					return True
				# Swamp Ground Decoration
				elif value == "swamp":
					if socket.hastag( 'last_ground_swamp' ):
						templist = []
						for choice in swamp_decor:
							if choice != str( socket.gettag( 'last_ground_swamp' ) ):
								templist += [ choice ]
						item = random.choice( templist )
						socket.settag( 'last_ground_swamp', str( item ) )
					else:
						item = random.choice( swamp_decor )
						socket.settag( 'last_ground_swamp', str( item ) )
					socket.attachtarget( 'commands.adddecor.createground', [ item ] )
					return True
				# Plains/Grass Ground Decoration
				elif value == "plains":
					if socket.hastag( 'last_ground_plains' ):
						templist = []
						for choice in plains_decor:
							if choice != str( socket.gettag( 'last_ground_plains' ) ):
								templist += [ choice ]
						item = random.choice( templist )
						socket.settag( 'last_ground_plains', str( item ) )
					else:
						item = random.choice( plains_decor )
						socket.settag( 'last_ground_plains', str( item ) )
					socket.attachtarget( 'commands.adddecor.createground', [ item ] )
					return True
				# Jungle Ground Decoration
				elif value == "jungle":
					if socket.hastag( 'last_ground_jungle' ):
						templist = []
						for choice in jungle_decor:
							if choice != str( socket.gettag( 'last_ground_jungle' ) ):
								templist += [ choice ]
						item = random.choice( templist )
						socket.settag( 'last_ground_jungle', str( item ) )
					else:
						item = random.choice( jungle_decor )
						socket.settag( 'last_ground_jungle', str( item ) )
					socket.attachtarget( 'commands.adddecor.createground', [ item ] )
					return True
				# Desert Ground Decoration
				elif value ==  "desert":
					if socket.hastag( 'last_ground_desert' ):
						templist = []
						for choice in desert_decor:
							if choice != str( socket.gettag( 'last_ground_desert' ) ):
								templist += [ choice ]
						item = random.choice( templist )
						socket.settag( 'last_ground_desert', str( item ) )
					else:
						item = random.choice( desert_decor )
						socket.settag( 'last_ground_desert', str( item ) )
					socket.attachtarget( 'commands.adddecor.createground', [ item ] )
					return True
				# Nothing
				else:
					socket.sysmessage( "Usage: adddecor ground [ forest, swamp, plains, jungle, desert ]" )
					return False
		# No Args
		else:
			socket.sysmessage( "Usage: adddecor [ tree, ground ]" )
			return False
	return False

def onLoad():
	wolfpack.registercommand( 'adddecor', adddecor )
