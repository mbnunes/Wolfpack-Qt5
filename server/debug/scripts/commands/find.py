import wolfpack
from wolfpack.consts import *
from system.makemenus import MakeItemAction, MakeNPCAction, MakeMenu
from wolfpack.utilities import hex2dec
from wolfpack import tr
from commands.add import AddItemAction, AddNpcAction, AddMultiAction
import re

def find(socket, command, arguments):
	if (len(arguments) == 0):
		socket.sysmessage( tr('Usage: find <searchpattern>') )
		return

	findsmenu = MakeMenu('FINDMENU', None, 'Find Menu')
	submenus = {}

	items = wolfpack.definitionsiterator(WPDT_ITEM)
	item = items.first
	while item:
		if not item.hasattribute('id'):
			item = items.next
			continue

		child = item.findchild('category')
		if not child:
			item = items.next
			continue

		categories = ['Items'] + child.text.split('\\')
		description = categories[len(categories)-1] # Name of the action
		categories = ['Items']

		if ( re.search(arguments.lower(), description.lower()) ):
			# Iterate through the categories and see if they're all there
			category = ''
			if len(categories) > 0 and not submenus.has_key('\\'.join(categories) + '\\'):
				for subcategory in categories:
					if not submenus.has_key(category + subcategory + '\\'):
						# Category is our parent category
						parent = None
						if len(category) == 0:
							parent = findsmenu
						elif category in submenus:
							parent = submenus[category]

						category += subcategory + '\\'
						menu = MakeMenu('FINDMENU_' + category, parent, subcategory)
						submenus[category] = menu
					else:
						category += subcategory + '\\'

			child = item.findchild('id')
			if child:
				try:
					id = int(child.value)
				except:
					id = 0
			else:
				id = 0

			definition = item.getattribute('id')
			additem = AddItemAction(menu, description, id, definition)
			additem.otherhtml = 'Definition: ' + definition

		item = items.next

	npcs = wolfpack.definitionsiterator(WPDT_NPC)
	submenus = {}

	npc = npcs.first
	while npc:
		if not npc.hasattribute('id'):
			npc = npcs.next
			continue

		child = npc.findchild('category')
		if not child:
			npc = npcs.next
			continue

		id = npc.findchild('id')
		if id:
			try:
				if id.value.startswith('0x'):
					id = wolfpack.bodyinfo(hex2dec(id.value))['figurine']
				else:
					id = wolfpack.bodyinfo(int(id.value))['figurine']
			except:
				id = 0
		else:
			id = 0

		description = npc.findchild('desc')
		if description:
			description = description.value
		else:
			description = tr('No description available.')

		categories = ['NPCs'] + child.text.split('\\')
		title = categories[len(categories)-1] # Name of the action
		categories = ['NPCs']
			
		if ( re.search(arguments.lower(), title.lower()) ):

			# Iterate trough the categories and see if they're all there
			category = ''
			if len(categories) > 0 and not submenus.has_key('\\'.join(categories) + '\\'):
				for subcategory in categories:
					if not submenus.has_key(category + subcategory + '\\'):
						# Category is our parent category
						parent = None
						if len(category) == 0:
							parent = findsmenu
						elif category in submenus:
							parent = submenus[category]

						category += subcategory + '\\'
						menu = MakeMenu('FINDMENU_' + category, parent, subcategory)
						submenus[category] = menu
					else:
						category += subcategory + '\\'

			definition = npc.getattribute('id')

			# Parse the position of this makemenu entry
			if len(categories) == 0:
				addnpc = AddNpcAction(menu, title , definition, definition)
			else:
				addnpc = AddNpcAction(submenus['\\'.join(categories) + '\\'], title, id, definition)
			addnpc.otherhtml = str(description)
		npc = npcs.next
		
		
	multis = wolfpack.definitionsiterator(WPDT_MULTI)
	submenus = {}
		
	multi = multis.first
	while multi:
		if not multi.hasattribute('id'):
			multi = multi.next
			continue

		child = multi.findchild('category')
		if not child:
			multi = multis.next
			continue

		categories = ['Multis'] + child.text.split('\\')
		description = categories[len(categories)-1] # Name of the action
		categories = ['Multis']

		if ( re.search(arguments.lower(), description.lower()) ):
			# Iterate trough the categories and see if they're all there
			category = ''
			if len(categories) > 0 and not submenus.has_key('\\'.join(categories) + '\\'):
				for subcategory in categories:
					if not submenus.has_key(category + subcategory + '\\'):
						# Category is our parent category
						parent = None
						if len(category) == 0:
							parent = findsmenu
						elif category in submenus:
							parent = submenus[category]

						category += subcategory + '\\'
						menu = MakeMenu('FINDMENU_' + category, parent, subcategory)
						submenus[category] = menu
					else:
						category += subcategory + '\\'

			child = multi.findchild('id')
			if child:
				try:
					id = int(child.value)
				except:
					id = 0
			else:
				id = 0
			multi = multi.getattribute('id')
			# Parse the position of this makemenu entry
			if len(categories) == 0:
				addmulti = AddMultiAction(addmenu, description, id, multi)
			else:
				addmulti = AddMultiAction(submenus['\\'.join(categories) + '\\'], description, id, multi)
			addmulti.otherhtml = 'Definition: ' + multi
		multi = multis.next

	if ( len(findsmenu.submenus) > 0 ):
		findsmenu.send( socket )
	else:
		socket.sysmessage( tr('Could not find any item, npc or multi matching the arguments.') )

def onLoad():
	wolfpack.registercommand('find', find)

"""
	\command find
	\description Use this command to find a item, npc or multi.
	\usage - <code>find <searchpattern></code>
"""