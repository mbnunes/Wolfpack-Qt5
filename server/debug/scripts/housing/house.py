
import wolfpack
from wolfpack.consts import *
from wolfpack.utilities import hex2dec
import housing
from housing.consts import *

#
# CoOwner management
# Get a list of character STRINGS(!) 
# Serials that are coowners of the house
#
def getCoOwners(house):
	if not house:
		return []

	coowners = []
	if house.hastag('coowners'):
		coowners = str(house.gettag('coowners')).split(',')		

	return coowners

#
# Add a coowner to this house
#
def addCoOwner(house, coowner):
	if not coowner or not house:
		return

	coowners = getCoOwners(house)
	serial = str(coowner.serial)

	if serial not in coowners:
		coowners.append(serial)
		house.settag('coowners', ','.join(coowners))

#
# Remove a co-owner from the house
#
def removeCoOwner(house, coowner):
	if not house or not coowner:
		return

	coowners = getCoOwners(house)
	serial = str(coowner.serial)
	changed = False

	while serial in coowners:
		coowners.remove(serial)
		changed = True

	if changed:
		if len(coowners)==0:
			return removeAllCoOwner(house)
		house.settag('coowners', ','.join(coowners))

#
# Remove all coowners
#
def removeAllCoOwner(house):
	if not house:
		return
	house.deltag('coowners')
#
# Friends management
# Get a list of character STRINGS(!) 
# Serials that are friends of the house
#
def getFriends(house):
	if not house:
		return []

	friends = []
	if house.hastag('friends'):
		friends = str(house.gettag('friends')).split(',')		

	return friends

#
# Add a friend to this house
#
def addFriend(house, friend):
	if not friend or not house:
		return

	friends = getFriends(house)
	serial = str(friend.serial)

	if serial not in friends:
		friends.append(serial)
		house.settag('friends', ','.join(friends))
		
#
# Remove a friend from the house
#
def removeFriend(house, friend):
	if not house or not friend:
		return

	friends = getFriends(house)
	serial = str(friend.serial)
	changed = False

	while serial in friends:
		friends.remove(serial)
		changed = True

	if changed:
		if len(friends)==0:
			return removeAllFriend(house)
		house.settag('friends', ','.join(friends))


#
# Remove all friends
#
def removeAllFriend(house):
	if not house:
		return
	house.deltag('friends')

#
# Guests management
# Get a list of character STRINGS(!) 
# Serials that are guests of the house
#
def getGuests(house):
	if not house:
		return []

	guests = []
	if house.hastag('guests'):
		guests = str(house.gettag('guests')).split(',')		

	return guests

#
# Add a guest to this house
#
def addGuest(house, guest):
	if not guest or not house:
		return

	guests = getGuests(house)
	serial = str(guest.serial)

	if serial not in guests:
		guests.append(serial)
		house.settag('guests', ','.join(guests))
		
#
# Remove a guest from the house
#
def removeGuest(house, guest):
	if not house or not guest:
		return

	guests = getGuests(house)
	serial = str(guest.serial)
	changed = False

	while serial in guests:
		guests.remove(serial)
		changed = True

	if changed:
		if len(guests)==0:
			return removeAllGuest(house)
		house.settag('guests', ','.join(guests))


#
# Remove all guests
#
def removeAllGuest(house):
	if not house:
		return
	house.deltag('guests')

#
# Is the given house public or private?
#
def isPublic(house):
	if house.hastag('security_level'):
		level = house.gettag('security_level')
		if level == 3: # open for anyone
			return True
	return False

def setPublic(house, public):
	if not house:
		return
	if public:
		house.settag('security_level', 3)
	else:
		house.settag('security_level', 1)

#
# Ban management
#
def getBans(house):
	if not house:
		return []
	bans = []
	if house.hastag('bans'):
		bans = str(house.gettag('bans')).split(',')
	return bans

def addBan(house, banned):
	if not banned or not house:
		return
	bans = getBans(house)
	serial = str(banned.serial)
	if serial not in bans:
		bans.append(serial)
		house.settag('bans', ','.join(bans))

def removeBan(house, banned):
	if not house or not banned:
		return
	bans = getBans(house)
	serial = str(banned.serial)
	changed = False
	while serial in bans:
		bans.remove(serial)
		changed = True
	if changed:
		if len(bans) == 0:
			house.deltag('bans')
		else:
			house.settag('bans', ','.join(bans))

def removeAllBans(house):
	if not house:
		return
	house.deltag('bans')

def isBanned(player, house):
	if not player or not house:
		return False
	return str(player.serial) in getBans(house)

#
# Register house on load
#
def onAttach(house):
	housing.registerHouse(house)

#
# Unregister house on unload/deletion
#
def onDetach(house):
	housing.unregisterHouse(house)

#
# Create the housing items
#
def onCreate(house, definition):
	buildHouse(house, definition)

#def already define
#
# Is the given character the owner of the given house?
#
#~ def isOwner(house, player):
	#~ if not house:
		#~ return False
	
	#~ if player.gm or house.owner == player:
		#~ return True
	#~ else:
		#~ return False

#
# Place items in the house based on a given house definition
#
def buildHouse(house, definition):
	node = wolfpack.getdefinition(WPDT_MULTI, definition)

	if not node:
		return

	if node.hasattribute('inherit'):
		value = str(node.getattribute('inherit'))
		buildHouse(house, value) # Recursion

	for i in range(0, node.childcount):
		child = node.getchild(i)

		# Inherit another definition
		if child.name == 'inherit':
			if child.hasattribute('id'):
				buildHouse(house, child.getattribute('id'))
			else:
				buildHouse(house, child.value)

		# Add a normal item to the house		
		elif child.name == 'item':
			x = int(child.getattribute('x', '0'))
			y = int(child.getattribute('y', '0'))
			z = int(child.getattribute('z', '0'))
			id = str(child.getattribute('id', ''))

			item = wolfpack.additem(id)
			item.moveto(house.pos.x + x, house.pos.y + y, house.pos.z + z, house.pos.map)
			item.update()

		# Add a house door to the house
		elif child.name == 'door':
			x = int(child.getattribute('x', '0'))
			y = int(child.getattribute('y', '0'))
			z = int(child.getattribute('z', '0'))
			id = hex2dec(child.getattribute('id', ''))

			item = wolfpack.additem('housedoor')
			item.id = id
			item.moveto(house.pos.x + x, house.pos.y + y, house.pos.z + z, house.pos.map)
			item.update()

		# Add a sign to the house
		elif child.name == 'sign':
			x = int(child.getattribute('x', '0'))
			y = int(child.getattribute('y', '0'))
			z = int(child.getattribute('z', '0'))

			sign = wolfpack.additem('housesign')
			if child.hasattribute('id'):
				sign.id = hex2dec(child.getattribute('id', ''))
			sign.moveto(house.pos.x + x, house.pos.y + y, house.pos.z + z, house.pos.map)
			sign.update()

#
# Check if access in this house is allowed to the given item
#
def onCheckSecurity(player, house, item):
	if not player or not house or not item:
		return False

	# GMs bypass house security
	if player.gm:
		return False

	# If the player is banned from this house, reject everything immediately
	if isBanned(player, house) and not isOwner(player, house):
		player.socket.clilocmessage(1061637) # It's not accessable
		return True # Access Denied

	# Check if this item is a door (house door or any door inside the house)
	is_door = False
	try:
		if item.hasscript('door') or item.type == 12:
			is_door = True
		else:
			from door import doors as _door_defs
			is_door = any(item.id == d[0] or item.id == d[1] for d in _door_defs)
	except Exception:
		pass

	if is_door:
		# If the house is open to the public:
		if isPublic(house):
			return False # Allow access to anyone not banned

		# In a private house:
		# Owner, Co-Owners, Friends, and Guests (Grant Access) have full door access
		if isOwner(player, house) or isCoOwner(player, house) or isFriend(player, house) or isGuest(player, house):
			return False # Allow access

		# Otherwise, denied!
		player.socket.clilocmessage(1061637) # It's not accessable
		return True # Access Denied

	# For other items inside the house (containers, securables):
	if item.hastag('security_level'):
		level = int(item.gettag('security_level'))
	else:
		# Default security for items in house: Co-Owner access
		level = ACCESS_COOWNER

	if not checkAccess(player, house, level):
		if item.type == 1: # Container
			player.socket.clilocmessage(501647) # It's secure
		else:
			player.socket.clilocmessage(1061637) # It's not accessable
		return True # Access Denied

	return False # Access Allowed

#
# Checks if the given player is an owner of the given house
# NOTE: This does not check higher access levels.
#
def isOwner(player, house):
	if not player or not house:
		return False
	h_owner = getattr(house, 'owner', None)
	if h_owner:
		if hasattr(h_owner, 'serial'):
			return player.serial == h_owner.serial
		elif isinstance(h_owner, (int, str)):
			return str(player.serial) == str(h_owner)
	return False

#
# Checks if the given player is an co-owner of the given house
# NOTE: This does not check higher access levels.
#
def isCoOwner(player, house):
	if not player or not house:
		return False
	coowners = getCoOwners(house)
	serial = str(player.serial)
	return serial in coowners

#
# Checks if the given player is a friend of the house.
# NOTE: This does not check higher access levels.
#
def isFriend(player, house):
	if not player or not house:
		return False
	friends = getFriends(house)
	serial = str(player.serial)
	return serial in friends

#
# Checks if the given player has been granted access (guest) to the house.
#
def isGuest(player, house):
	if not player or not house:
		return False
	guests = getGuests(house)
	serial = str(player.serial)
	return serial in guests

#
# Check the access level of the given char for this house
#
def checkAccess(player, house, level):
	if not player or not house or level < 0 or level > 3:
		return False # Error Checks

	if player.gm:
		return True

	if isBanned(player, house) and not isOwner(player, house):
		return False

	if isOwner(player, house):
		return True

	if level >= ACCESS_COOWNER and isCoOwner(player, house):
		return True

	if level >= ACCESS_FRIEND and isFriend(player, house):
		return True

	if level >= ACCESS_ANYONE:
		return True

	return False

def onLoad():
	wolfpack.registercommand( 'registrehouse', commandregistrehouse ) #debug use
	wolfpack.registercommand( 'lockthis', commandlockthis )
	wolfpack.registercommand( 'unlockthis', commandunlockthis )
	return
# Debug use
def commandregistrehouse(socket, cmd, args ):
	socket.sysmessage("House : %s" % str(housing.getHouses()) )
	return
#a first way to do that
def commandlockthis(socket, cmd, args ):
	socket.sysmessage('What do you want to lock ?')
	socket.attachtarget("housing.house.locktarget", [])
	return
#a first way to do that
def commandunlockthis(socket, cmd, args ):
	socket.sysmessage('What do you want to unlock ?')
	socket.attachtarget("housing.house.locktarget", [])
	return

def locktarget(char, arguments, target):
	if not target.item or not target.item.isitem:
		char.socket.sysmessage('You have to target an item.')
		return
	multi = target.item.multi or wolfpack.findmulti(target.item.pos)
	if not multi:
		char.socket.sysmessage('You are not in a house.')
		return
	if not checkAccess(char, multi, ACCESS_COOWNER):
		char.socket.sysmessage("You can't do that, ask a co-owner or the owner.")
		return		
	# the item is not locked
	if not target.item.lockeddown: 
		target.item.lockeddown = 1
		target.item.movable = 3
		char.socket.sysmessage('You lock %s' % (target.item.name))
	else: # the item is locked
		target.item.lockeddown = 0
		target.item.movable = 1
		char.socket.sysmessage('You unlock %s' % (target.item.name))
	return

def onServerStart():
	housing.refreshHouses()