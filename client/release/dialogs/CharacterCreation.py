
from client import *

# Character Templates
CharacterTemplates = (
	("Samurai", 1062948, 1062950, 5591, Skills.Bushido, 50, Skills.Swordsmanship, 50, Skills.Wrestling, 0, 40, 30, 10 ),
	("Ninja", 1062949, 1062951, 5589, Skills.Ninjitsu, 50, Skills.Hiding, 50, Skills.Wrestling, 0, 40, 30, 10 ),
	("Paladin", 1061177, 1061227, 5587, Skills.Chivalry, 50, Skills.Tactics, 50, Skills.Wrestling, 0, 45, 20, 15 ),
	("Necromancer", 1061178, 1061228, 5557, Skills.Necromancy, 50, Skills.Swordsmanship, 30, Skills.Tactics, 20, 25, 20, 35 ),
	("Warrior", 1061180, 1061230, 5577, Skills.Tactics, 50, Skills.Healing, 45, Skills.Swordsmanship, 5, 35, 35, 10 ),
	("Mage", 1061179, 1061229, 5569, Skills.Magery, 50, Skills.Meditation, 50, Skills.Wrestling, 0, 25, 10, 45 ),
	("Blacksmith", 1061181, 1061231, 5555, Skills.Blacksmithy, 50, Skills.Tinkering, 45, Skills.Mining, 5, 60, 10, 10 ),
	("Advanced", 3000448, 3000448, 5505, Skills.Alchemy, 0, Skills.Alchemy, 0, Skills.Alchemy, 0, 30, 25, 25),
)

CTINDEX_NAME = 0 # Index Constant for the template name
CTINDEX_LOCALIZEDNAME = 1
CTINDEX_LOCALIZEDDESC = 2
CTINDEX_GUMP = 3
CTINDEX_STRENGTH = 10
CTINDEX_DEXTERITY = 11
CTINDEX_INTELLIGENCE = 12

# Note: This won't work with sphere
MAXIMUM_STATS = 80
MAX_STAT = 60
MIN_STAT = 10
SKILLCOUNT = Skills.count()

# This class manages the context of the current character creation
class Context:
	"""
	 Setup the contexts default
	"""
	def __init__(self):
		self.profession = -1
		self.strength = 10
		self.dexterity = 10
		self.intelligence = 10
								
	"""
	 Initialize the Character creation dialog (page 1)
	"""
	def setupDialog1(self, dialog):
		# Center the title label
		label = dialog.findByName("TitleLabel")	
		if label:
			label.update()
			label.x = label.x + (400 - label.width) / 2    
		
		xpos = 135
		ypos = 147
		
		# Add profession selection buttons
		for i in range(0, len(CharacterTemplates)):
			template = CharacterTemplates[i]
			optn = Gui.createDialog("CharacterTemplate")
			if optn:
				# Set the label text and the gump id
				label = optn.findByName("TemplateLabel")
				label.text = Localization.get(template[CTINDEX_LOCALIZEDNAME])
				btn = optn.findByName("TemplateButton")
				btn.setStateGump(BS_UNPRESSED, template[CTINDEX_GUMP])
				btn.setStateGump(BS_PRESSED, template[CTINDEX_GUMP] + 1)
				btn.setStateGump(BS_HOVER, template[CTINDEX_GUMP] + 1)
				btn.setTag("profession", str(i))
				#connect(btn, "onButtonPress(cControl*)", "characterCreation1Next")
				    		
				optn.setPosition(xpos, ypos)
				dialog.addControl(optn)
				
				if i % 2 == 0:
					xpos += 205
				else:
					xpos -= 205
					ypos += 70
		
		dialog.visible = True
		
	"""
	 Initialize the Character creation dialog (page 2)
	"""
	def setupDialog2(self, dialog):		
		pass		

# Create the Character Creation context		
context = Context()

def initialize(dialog):
	global context
	if dialog.objectName == "CharacterCreation1":
		context.setupDialog1(dialog)
	elif dialog.objectName == "CharacterCreation2":
		context.setupDialog2(dialog)
