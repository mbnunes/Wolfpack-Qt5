
import wolfpack
import wolfpack.gumps
from wolfpack import tr, console
from wolfpack.consts import *

def edit_response(player, arguments, response):
	pattern = getPattern(player, arguments[0])
	pattern.response(player, response)	

def tileResponse(player, arguments, target):
	if len(arguments) < 2:
		player.socket.sysmessage('Please select the second corner.')
		player.socket.attachtarget("commands.tilepattern.tileResponse", list(arguments) + [target.pos])
		return
		
	pattern = arguments[0]
	x1 = min(arguments[1].x, target.pos.x)
	x2 = max(arguments[1].x, target.pos.x)
	y1 = min(arguments[1].y, target.pos.y)
	y2 = max(arguments[1].y, target.pos.y)
	
	unlimited = player.account.authorized('Misc', 'Unlimited Tile')
	count = ((x2 - x1) + 1) * ((y2 - y1) + 1)	
	
	# Cap at 250 items if not an admin is using it
	if not unlimited and count > 250:
		player.socket.sysmessage('You are not allowed to tile more than 250 items at once.')
		return	
		
	player.log(LOG_MESSAGE, "Tiling %u items from %u,%u to %u,%u at z=%d.\n" % (count, x1, y1, x2, y2, pattern.z))
	player.socket.sysmessage('Creating %u items from %u,%u to %u,%u at z=%d.' % (count, x1, y1, x2, y2, pattern.z))
	pos = player.pos
	pos.z = pattern.z
	
	col = 0
	for x in range(x1, x2 + 1):		
		row = 0
		for y in range(y1, y2 + 1):			
			baseid = pattern.getBaseid(col, row)
			row += 1
			
			if baseid != '':			
				pos.x = x
				pos.y = y
				item = wolfpack.additem(baseid)
				if item:
					item.moveto(pos)
					item.decay = False
					item.update()
		col += 1

#
# Patterns
#
class Pattern:
	def __init__(self, id):
		self.id = id
		self.width = 1
		self.height = 1
		self.z = 0
		self.baseids = ['']
		
	#
	# Response
	#
	def response(self, player, response):
		if response.button == 0:
			tilepattern(player.socket, '', '')
		elif response.button == 1:
			self.tile(player)
		elif response.button == 2:
			self.delete(player)
		elif response.button == 3:
			# change width-height
			try:
				width = int(response.text[1000])
				height = int(response.text[1001])
				z = int(response.text[1002])
				self.z = int(z)
				
				if width < 1 or width > 10 or height < 1 or height > 10:
					player.socket.sysmessage(tr('The maximum width or height of a pattern is 10. The minimum width or height of a pattern is 1.'))
				else:
					self.width = width
					self.height = height
					
					# Truncate the baseid array
					if len(self.baseids) > width * height:
						self.baseids = list(self.baseids[:width*height])
					
					# Fill up the baseid arary
					while len(self.baseids) < width * height:
						self.baseids.append('')
						
					self.save(player)
			except:
				player.socket.sysmessage(tr('You have to specify a valid width and height.'))
				self.edit(player)
				raise
			self.edit(player)			

		elif response.button == 4:
			# Gather the baseids for all cells
			count = self.width * self.height
			for i in range(0, count):
				if 5 + i in response.text:
					baseid = response.text[5 + i]
					self.baseids[i] = baseid
			self.save(player)
			self.edit(player)

	#
	# Get Baseid	
	#
	def getBaseid(self, x, y):
		col = x % self.width
		row = y % self.height
		return self.baseids[ row * self.width + col ]

	#
	# Send an edit gump for this pattern to the player
	#
	def edit(self, player):
		dialog = wolfpack.gumps.cGump()
		dialog.setCallback(edit_response)
		dialog.setArgs([self.id])
		
		height = 200 + 40 * self.height
		width = max(416, 60 + 79 * self.width)
		
		dialog.startPage(0)
		dialog.addResizeGump(0, 0, 9250, width, height)
		dialog.addText(28, 20, "Editing Pattern %u" % self.id, 2100)
		dialog.addText(28, 56, "Width:", 2100)
		dialog.addButton(264, 24, 9903, 9905, 2) # Clear
		dialog.addButton(28, 140 + 40 * self.height, 9903, 9905, 4) # Save Changes
		dialog.addButton(196, 24, 9903, 9905, 1) # Tile
		dialog.addButton(308, 56, 9903, 9905, 3) # Change WidthXHeight
		dialog.addText(224, 24, "Tile", 2100)
		
		dialog.addText(292, 24, "Clear", 2100)
		dialog.addText(140, 56, "Height:", 2100)
		dialog.addText(245, 56, "Z:", 2100)
		dialog.addResizeGump(77, 52, 3000, 38, 30)
		dialog.addResizeGump(196, 52, 3000, 38, 30)
		dialog.addText(336, 56, "Change", 2100)	
		dialog.addText(56, 140 + 40 * self.height, "Save Changes", 2100)
		dialog.addInputField(80, 56, 28, 20, 2100, 1000, str(self.width))
		dialog.addInputField(200, 56, 28, 20, 2100, 1001, str(self.height))
		dialog.addResizeGump(261, 52, 3000, 38, 30)
		dialog.addInputField(265, 56, 28, 20, 2100, 1002, str(self.z))
		
		for col in range(0, self.width):
			for row in range(0, self.height):
				# Add Col Header
				if row == 0:
					dialog.addText(76 + col * 79, 100, str(col + 1), 2100)
					
				# Add Row Header
				if col == 0:
					dialog.addText(24, 128 + row * 40, str(row + 1), 2100)
		
				dialog.addResizeGump(49 + col * 79, 124 + row * 40, 3000, 69, 30)
				dialog.addInputField(56 + col * 79, 128 + row * 40, 55, 20, 2100, 5 + (row * self.width + col), self.getBaseid(col, row))
		
		dialog.send(player)
		
	#
	# Delete this pattern
	#
	def delete(self, player):
		player.deltag('tilepattern_%u' % self.id)
		player.socket.sysmessage(tr('The tile pattern has been cleared.'))
		
	#
	# Tile this pattern
	#
	def tile(self, player):
		player.socket.sysmessage('Please select the first corner.')
		player.socket.attachtarget('commands.tilepattern.tileResponse', [self])

	def save(self, player):
		tag = u'%u|%u|%d|%s' % (self.width, self.height, self.z, u';'.join(self.baseids))
		player.settag('tilepattern_%u' % self.id, tag)

	def load(self, player):
		if not player.hastag('tilepattern_%u' % self.id):
			return
		
		value = player.gettag('tilepattern_%u' % self.id)
		
		try:
			# (width, height, z, baseids)
			(self.width, self.height, self.z, self.baseids) = value.split('|')
			self.width = int(self.width)
			self.height = int(self.height)
			self.z = int(self.z)
			self.baseids = list(self.baseids.split(';'))
			
			if self.width < 1:
				self.width = 1
			if self.height < 1:
				self.height = 1
			if self.width > 10:
				self.width = 10
			if self.height > 10:
				self.height = 10
				
			while len(self.baseids) < self.width * self.height:
				self.baseids.append('')
		except:
			return
		
		pass

#
# ParsePattern
#
def getPattern(player, id):
	pattern = Pattern(id)
	pattern.load(player)	
	return pattern

def tilepattern_response(player, arguments, response):
	if response.button == 0:
		return
		
	id = (response.button - 2) / 3
	button = (response.button - 2) % 3
	
	if id < 5:
		pattern = getPattern(player, id)
		if button == 0:
			pattern.tile(player)
		elif button == 1:
			pattern.edit(player)
		elif button == 2:
			pattern.delete(player)
	

#
# Show Selection Gump
#
def tilepattern(socket, command, arguments):
	if arguments == '':	
		dialog = wolfpack.gumps.cGump()
		dialog.setCallback(tilepattern_response)
		
		dialog.startPage(0)
		dialog.addResizeGump(0, 0, 9250, 453, 375)
		dialog.addText(173, 19, "Tile Pattern", 2100)

		#dialog.addButton(28, 44, 9903, 9905, 1)
		#dialog.addText(56, 44, "New Pattern", 2100)

		# Loop for all the patterns on the char
		yoffset = 44
		for i in range(0, 5):
			pattern = getPattern(socket.player, i)

			dialog.addText(28, yoffset + 0, "Pattern: %u (Z: %d)" % (i, pattern.z), 2100)
			dialog.addText(284, yoffset + 0, "Size: %ux%u" % (pattern.width, pattern.height), 2100)
			dialog.addButton(28, yoffset + 28, 9903, 9905, i * 3 + 2)
			dialog.addText(56, yoffset + 28, "Tile", 2100)
			dialog.addButton(116, yoffset + 28, 9903, 9905, i * 3 + 3)
			dialog.addText(144, yoffset + 28, "Edit", 2100)
			dialog.addButton(204, yoffset + 28, 9903, 9905, i * 3 + 4)
			dialog.addText(232, yoffset + 28, "Clear", 2100)
			
			if i < 4:
				dialog.addTiledGump(14, yoffset + 56, 425, 3, 2700, 0)
			
			yoffset += 65

		dialog.send(socket.player)

def onLoad():
	wolfpack.registercommand('tilepattern', tilepattern)

"""
	\command tilepattern
	\description Brings up a gump which allows you to create patterns of items for building.
"""