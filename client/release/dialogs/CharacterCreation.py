
from client import *

# Character Templates
CharacterTemplates = (
	("Samurai", 1062948, 1062950, 5591, Skills.Bushido, 50, Skills.Swordsmanship, 50, Skills.Wrestling, 0, 40, 30, 10),
	("Ninja", 1062949, 1062951, 5589, Skills.Ninjitsu, 50, Skills.Hiding, 50, Skills.Wrestling, 0, 40, 30, 10),
	("Paladin", 1061177, 1061227, 5587, Skills.Chivalry, 50, Skills.Tactics, 50, Skills.Wrestling, 0, 45, 20, 15),
	("Necromancer", 1061178, 1061228, 5557, Skills.Necromancy, 50, Skills.Swordsmanship, 30, Skills.Tactics, 20, 25, 20, 35),
	("Warrior", 1061180, 1061230, 5577, Skills.Tactics, 50, Skills.Healing, 45, Skills.Swordsmanship, 5, 35, 35, 10),
	("Mage", 1061179, 1061229, 5569, Skills.Magery, 50, Skills.Meditation, 50, Skills.Wrestling, 0, 25, 10, 45),
	("Blacksmith", 1061181, 1061231, 5555, Skills.Blacksmithy, 50, Skills.Tinkering, 45, Skills.Mining, 5, 60, 10, 10),
	("Advanced", 3000448, 3000448, 5505, Skills.Alchemy, 50, Skills.Alchemy, 50, Skills.Alchemy, 0, 30, 25, 25),
)

# In this order: None, Short, Long, Ponytail, Mohawk, Pageboy, Topknot, Curly, Receding, Pigtails
HairstyleIds = (0, 0x203b, 0x203c, 0x203d, 0x2044, 0x2045, 0x204a, 0x2047, 0x2048, 0x2049)

# In this order: None, Goatee, Long Beard, Short Beard, Moustache, Medium Short Beard, Medium Long beard, Vandyke
FacialHairstyleIds = (0, 0x2040, 0x203e, 0x203f, 0x2041, 0x204b, 0x204c, 0x204d)

CTINDEX_NAME = 0 # Index Constant for the template name
CTINDEX_LOCALIZEDNAME = 1
CTINDEX_LOCALIZEDDESC = 2
CTINDEX_GUMP = 3
CTINDEX_SKILL1 = 4
CTINDEX_SKILL1VALUE = 5
CTINDEX_SKILL2 = 6
CTINDEX_SKILL2VALUE = 7
CTINDEX_SKILL3 = 8
CTINDEX_SKILL3VALUE = 9
CTINDEX_STRENGTH = 10
CTINDEX_DEXTERITY = 11
CTINDEX_INTELLIGENCE = 12

GENDER_MALE = 0
GENDER_FEMALE = 1

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
		self.skill1 = -1
		self.skill2 = -1
		self.skill3 = -1
		self.skill1Value = 50
		self.skill2Value = 50
		self.skill3Value = 0
		self.normalizingSkills = False
		self.hairstyle = 0
		self.pantscolor = random(2, 0x3e9)
		self.shirtcolor = random(2, 0x3e9)
		self.gender = GENDER_MALE
		self.haircolor = random(0x44e, 0x47d)
		self.facialhaircolor = random(0x44e, 0x47d)
		self.skincolor = random(0x3ea, 0x422)

	"""
	  Show the first page of the character creation
	"""
	def showFirst(self):
		loginDialog = Gui.findByName("LoginDialog")
		loginDialog.findByName("CharacterCreation1").visible = True
		
	"""
		Hide all character creation pages
	"""
	def hideAll(self):
		loginDialog = Gui.findByName("LoginDialog")
		loginDialog.findByName("CharacterCreation1").visible = False
		loginDialog.findByName("CharacterCreation2").visible = False
		loginDialog.findByName("CharacterCreation3").visible = False		

	"""
	 Select the profession with the given id
	"""
	def selectProfession(self, id):
		self.profession = id
		
		self.setStrength(CharacterTemplates[id][CTINDEX_STRENGTH], True)
		self.setDexterity(CharacterTemplates[id][CTINDEX_DEXTERITY], True)
		self.setIntelligence(CharacterTemplates[id][CTINDEX_INTELLIGENCE], True)
		
		self.setSkillValue(1, CharacterTemplates[id][CTINDEX_SKILL1VALUE], True)
		self.setSkillValue(2, CharacterTemplates[id][CTINDEX_SKILL2VALUE], True)
		self.setSkillValue(3, CharacterTemplates[id][CTINDEX_SKILL3VALUE], True)
		dialog = Gui.findByName("CharacterCreation2")
		dialog.findByName("SkillBox1").selectItem(CharacterTemplates[id][CTINDEX_SKILL1])
		dialog.findByName("SkillBox2").selectItem(CharacterTemplates[id][CTINDEX_SKILL2])
		dialog.findByName("SkillBox3").selectItem(CharacterTemplates[id][CTINDEX_SKILL3])

		# Hide current dialog and show next
		loginDialog = Gui.findByName("LoginDialog")
		loginDialog.findByName("CharacterCreation1").visible = False
		
		if CharacterTemplates[id][CTINDEX_NAME] == "Advanced":
			loginDialog.findByName("CharacterCreation2").visible = True
		else:
			self.characterCreation2Next(None)
						
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
				connect(btn, "onButtonPress(cControl*)", self.characterCreation1Next)
				    		
				optn.setPosition(xpos, ypos)
				dialog.addControl(optn)
				
				if i % 2 == 0:
					xpos += 205
				else:
					xpos -= 205
					ypos += 70
					
		connect(dialog.findByName("BackButton"), "onButtonPress(cControl*)", self.dialog1Back)
	
	def dialog1Back(self, button):
		LoginDialog.show(PAGE_SELECTCHAR)		
	
	"""
	  Select a profession and advance to the next page.
	"""
	def characterCreation1Next(self, button):
		# We only process profession buttons here
		if not button.hasTag("profession"):
			return	

		self.selectProfession(int(button.getTag("profession")))

	"""
		Go back
	"""
	def characterCreation2Back(self, button):
		# Hide current dialog and show next
		loginDialog = Gui.findByName("LoginDialog")
		loginDialog.findByName("CharacterCreation2").visible = False
		loginDialog.findByName("CharacterCreation1").visible = True
		
	"""
		Close an error dialog and show
		self.showAfterError instead
	"""
	def closeErrorDialog(self, button):
		loginDialog = Gui.findByName("LoginDialog")
		
		errorDialog = loginDialog.findByName("CharacterCreationError")		
		errorDialog.visible = False
		errorDialog.deleteLater()
		
		dialog = loginDialog.findByName(self.showAfterError)
		dialog.visible = True
		
	"""
		Check if the skills differ, then next screen
	"""
	def characterCreation2Next(self, button):
		loginDialog = Gui.findByName("LoginDialog")
		dialog = loginDialog.findByName("CharacterCreation2")
		self.skill1 = dialog.findByName("SkillBox1").selectionIndex()
		self.skill2 = dialog.findByName("SkillBox2").selectionIndex()
		self.skill3 = dialog.findByName("SkillBox3").selectionIndex()
		
		if self.skill1 == self.skill2 or self.skill1 == self.skill3 or self.skill2 == self.skill3 or self.skill1 == -1 or self.skill2 == -1 or self.skill3 == -1:			
			errorDialog = loginDialog.findByName("CharacterCreationError")
			if not errorDialog:
				errorDialog = Gui.createDialog("CharacterCreationError")
				loginDialog.addControl(errorDialog)
				
			label = errorDialog.findByName("MessageLabel")
			label.text = "Please Select Three Different Skills."
			label.x = 25 + (180 - label.width) / 2
			
			self.showAfterError = "CharacterCreation2"
			connect(errorDialog.findByName("OkButton"), "onButtonPress(cControl*)", self.closeErrorDialog)
			
			dialog.visible = False
			errorDialog.visible = True
			return
		
		# Hide current dialog and show next
		dialog.visible = False		
		self.showDialog3()
		
	def strengthScrolled(self, value):
		loginDialog = Gui.findByName("LoginDialog")
		strengthScroller = loginDialog.findByName("StrengthScroller")
		self.setStrength(strengthScroller.pos)
		
	def dexterityScrolled(self, value):
		loginDialog = Gui.findByName("LoginDialog")
		dexterityScroller = loginDialog.findByName("DexterityScroller")
		self.setDexterity(dexterityScroller.pos)
			
	def intelligenceScrolled(self, value):
		loginDialog = Gui.findByName("LoginDialog")
		intelligenceScroller = loginDialog.findByName("IntelligenceScroller")
		self.setIntelligence(intelligenceScroller.pos)
		
	def skill1Scrolled(self, value):
		if self.normalizingSkills:
			return
		loginDialog = Gui.findByName("LoginDialog")
		scroller = loginDialog.findByName("SkillScroller1")
		self.setSkillValue(1, scroller.pos)
		
	def skill2Scrolled(self, value):
		if self.normalizingSkills:
			return		
		loginDialog = Gui.findByName("LoginDialog")
		scroller = loginDialog.findByName("SkillScroller2")
		self.setSkillValue(2, scroller.pos)
		
	def skill3Scrolled(self, value):
		if self.normalizingSkills:
			return		
		loginDialog = Gui.findByName("LoginDialog")
		scroller = loginDialog.findByName("SkillScroller3")
		self.setSkillValue(3, scroller.pos)				

	"""
	 Initialize the Character creation dialog (page 2)
	"""
	def setupDialog2(self, dialog):		
		# Center the title label
		label = dialog.findByName("TitleLabel")
		if label:
			label.update() # Make sure the size is correct
			label.x = label.x + (400 - label.width) / 2
	
		backButton = dialog.findByName("BackButton")
		nextButton = dialog.findByName("NextButton")
		connect(backButton, "onButtonPress(cControl*)", self.characterCreation2Back)
		connect(nextButton, "onButtonPress(cControl*)", self.characterCreation2Next)
		
		connect(dialog.findByName("StrengthScroller"), "scrolled(int)", self.strengthScrolled)
		connect(dialog.findByName("DexterityScroller"), "scrolled(int)", self.dexterityScrolled)
		connect(dialog.findByName("IntelligenceScroller"), "scrolled(int)", self.intelligenceScrolled)
		connect(dialog.findByName("SkillScroller1"), "scrolled(int)", self.skill1Scrolled)
		connect(dialog.findByName("SkillScroller2"), "scrolled(int)", self.skill2Scrolled)
		connect(dialog.findByName("SkillScroller3"), "scrolled(int)", self.skill3Scrolled)		

		items = []
		for i in range(0, SKILLCOUNT - 1):
			items.append(Localization.get(1044060 + i))

		# Set up the combo boxes
		cb1 = dialog.findByName("SkillBox1")
		cb1.setItems(items)
		cb2 = dialog.findByName("SkillBox2")
		cb2.setItems(items)
		cb3 = dialog.findByName("SkillBox3")
		cb3.setItems(items)
		
	"""
		Change the shirt color
	"""
	def setShirtColor(self, color):
		dialog = Gui.findByName("LoginDialog")
		gump = dialog.findByName("PaperdollTorso")
		gump.hue = color
		self.shirtcolor = color
		
	"""
		Change the pants color
	"""
	def setPantsColor(self, color):
		dialog = Gui.findByName("LoginDialog")
		gump = dialog.findByName("PaperdollLegs")
		gump.hue = color
		self.pantscolor = color
		
	"""
		The hair style changed
	"""
	def hairStyleChanged(self):
		dialog = Gui.findByName("CharacterCreation3")
		
		cb = dialog.findByName("HairStyle")
		style = cb.selectionIndex()
		
		self.hairstyle = style
		
		gump = dialog.findByName("PaperdollHair")
		if style == 0:
			gump.visible = False
		else:
			# Get the gump id
			tiledata = Tiledata.getItemInfo(HairstyleIds[style])		
			
			gump.visible = True
			if self.gender == GENDER_FEMALE and Gumpart.exists(60000 + tiledata.animation):
				gump.setId(60000 + tiledata.animation)
			else:
				gump.setId(50000 + tiledata.animation)				
			
	"""
		The facial hair style changed
	"""
	def facialHairStyleChanged(self):
		dialog = Gui.findByName("CharacterCreation3")
		
		cb = dialog.findByName("FacialHairStyle")
		style = cb.selectionIndex()
		self.facialhairstyle = style
		
		gump = dialog.findByName("PaperdollFacialHair")
		
		# No beards for females
		if self.gender == GENDER_FEMALE:		
			self.facialhairstyle = 0
			gump.visible = False
			return
		
		if style == 0:
			gump.visible = False
		else:
			# Get the gump id
			tiledata = Tiledata.getItemInfo(FacialHairstyleIds[style])
			
			gump.visible = True
			gump.setId(50000 + tiledata.animation)
		
	"""
		Show dialog 3
	"""	
	def showDialog3(self):
		self.setShirtColor(self.shirtcolor)
		self.setPantsColor(self.pantscolor)
		loginDialog = Gui.findByName("LoginDialog")
		charCreation = loginDialog.findByName("CharacterCreation3")
		
		# Select random hairstyle
		cb = charCreation.findByName("HairStyle")
		cb.selectItem(random(0, len(HairstyleIds) - 1))
		
		charCreation.visible = True		
	
	"""
		Change the color of the skin.
	"""
	def setSkinColor(self, color, dialog = None):
		if not dialog:
			loginDialog = Gui.findByName("LoginDialog")
			dialog = loginDialog.findByName("CharacterCreation3")
		gump = dialog.findByName("PaperdollBody")
		gump.hue = color
		self.skincolor = color
	
	"""
		Change the color of the hair.
	"""
	def setHairColor(self, color, dialog = None):
		if not dialog:
			loginDialog = Gui.findByName("LoginDialog")
			dialog = loginDialog.findByName("CharacterCreation3")
		gump = dialog.findByName("PaperdollHair")
		gump.hue = color
		self.haircolor = color
		
	"""
		Change the color of the facial hair.
	"""
	def setFacialHairColor(self, color, dialog = None):
		if not dialog:
			loginDialog = Gui.findByName("LoginDialog")
			dialog = loginDialog.findByName("CharacterCreation3")
		gump = dialog.findByName("PaperdollFacialHair")
		gump.hue = color		
		self.facialhaircolor = color
		
	"""
		The hair color button has been clicked
	"""
	def changeHairColorClicked(self, button):
		self.setDialog3Visibility(False)
		
		dialog = Gui.findByName("CharacterCreation3")
		dialog.findByName("HairColorPicker").visible = True
		self.selectingColor = "hair"
		
	"""
		The facial hair color button has been clicked
	"""
	def changeFacialHairColorClicked(self, button):
		self.setDialog3Visibility(False)
		
		dialog = Gui.findByName("CharacterCreation3")
		dialog.findByName("HairColorPicker").visible = True
		self.selectingColor = "facial"

	"""
		Gender button slots
	"""
	def setFemaleGender(self, button):
		self.setGender(GENDER_FEMALE)
		
	def setMaleGender(self, button):
		self.setGender(GENDER_MALE)
		
	"""
		Change gender
	"""
	def setGender(self, gender):
		if self.gender == gender:
			return
			
		self.gender = gender # Change local gender setting
		
		# Change Paperdoll Body
		loginDialog = Gui.findByName("LoginDialog")
		dialog = loginDialog.findByName("CharacterCreation3")
		gump = dialog.findByName("PaperdollBody")
		if gender == GENDER_FEMALE:
			gump.id = 0xd
		else:
			gump.id = 0xc

		# Refresh the gumps
		self.hairStyleChanged()
		self.facialHairStyleChanged()
		
		# Shoes
		gump = dialog.findByName("PaperdollFeet")
		if gender == GENDER_FEMALE:
			gump.id = 60480
		else:
			gump.id = 50480

		# Legs
		gump = dialog.findByName("PaperdollLegs")
		if gender == GENDER_FEMALE:
			gump.id = 60449
		else:
			gump.id = 50430

		# Torso
		gump = dialog.findByName("PaperdollTorso")
		if gender == GENDER_FEMALE:
			gump.id = 60434
		else:
			gump.id = 50434

		# Hide or show the facial hairstyle button
		dialog.findByName("FacialHairStyle").visible = (gender == GENDER_MALE)
		dialog.findByName("FacialHairColorButton").visible = (gender == GENDER_MALE)
		
		# Rename the Skirt/Pants color button accordingly
		if gender == GENDER_MALE:
			dialog.findByName("PantsColorButton").text = Localization.get(3000441)
		else:
			dialog.findByName("PantsColorButton").text = Localization.get(3000439 )
		
	"""
		Set up the third dialog page
	"""
	def setupDialog3(self, dialog):
		styles = []
		for i in range(0, len(HairstyleIds)):
			styles.append(Localization.get(3000340 + i))
		
		cb = dialog.findByName("HairStyle")
		cb.setItems(styles)
		cb.selectItem(0)
		connect(cb, "selectionChanged()", self.hairStyleChanged)
		connect(dialog.findByName("HairColorButton"), "onButtonPress(cControl*)", self.changeHairColorClicked)
		
		styles = [Localization.get(3000340)]
		for i in range(0, len(FacialHairstyleIds)-1):
			styles.append(Localization.get(3000351 + i))
		
		cb = dialog.findByName("FacialHairStyle")
		cb.setItems(styles)
		cb.selectItem(0)
		connect(cb, "selectionChanged()", self.facialHairStyleChanged)
		connect(dialog.findByName("FacialHairColorButton"), "onButtonPress(cControl*)", self.changeFacialHairColorClicked)

		connect(dialog.findByName("FemaleButton"), "onButtonPress(cControl*)", self.setFemaleGender)		
		connect(dialog.findByName("MaleButton"), "onButtonPress(cControl*)", self.setMaleGender)
		connect(dialog.findByName("ShirtColorButton"), "onButtonPress(cControl*)", self.selectShirtColor)
		connect(dialog.findByName("PantsColorButton"), "onButtonPress(cControl*)", self.selectPantsColor)
		connect(dialog.findByName("ClothColorPicker"), "colorSelected(ushort)", self.clothColorSelected)
		connect(dialog.findByName("SkinColorPicker"), "colorSelected(ushort)", self.skinToneSelected)
		connect(dialog.findByName("HairColorPicker"), "colorSelected(ushort)", self.hairColorSelected)
		connect(dialog.findByName("SkinToneButton"), "onButtonPress(cControl*)", self.selectSkinTone)
		
		connect(dialog.findByName("BackButton"), "onButtonPress(cControl*)", self.dialog3Back)
		connect(dialog.findByName("NextButton"), "onButtonPress(cControl*)", self.dialog3Next)
		
		self.setHairColor(self.haircolor, dialog)
		self.setFacialHairColor(self.facialhaircolor, dialog)
		self.setSkinColor(self.skincolor, dialog)


	"""
		Back + Next Button
	"""
	def dialog3Next(self, dialog):
		startLocations = UoSocket.startLocations()
		print repr(startLocations)
		
	def dialog3Back(self, dialog):
		loginDialog = Gui.findByName("LoginDialog")
		loginDialog.findByName("CharacterCreation3").visible = False
		
		# Advanced Profession
		if self.profession == len(CharacterTemplates) - 1:
			loginDialog.findByName("CharacterCreation2").visible = True
		else:
			loginDialog.findByName("CharacterCreation1").visible = True

	"""
		The button for selecting the skin color has been pressed
	"""
	def selectSkinTone(self, button):
		self.setDialog3Visibility(False)
		
		dialog = Gui.findByName("CharacterCreation3")
		dialog.findByName("SkinColorPicker").visible = True

	"""
		The button for selecting the shirt color has been pressed
	"""
	def selectShirtColor(self, button):
		self.setDialog3Visibility(False)
		
		dialog = Gui.findByName("CharacterCreation3")
		dialog.findByName("ClothColorPicker").visible = True
		self.selectingColor = 'shirt' # Remember what we're picking the color for

	"""
		The button for selecting the pants color has been pressed
	"""
	def selectPantsColor(self, button):
		self.setDialog3Visibility(False)
		
		dialog = Gui.findByName("CharacterCreation3")
		dialog.findByName("ClothColorPicker").visible = True
		self.selectingColor = 'pants' # Remember what we're picking the color for

	"""
		This gets triggered when a skin color is selected.
	"""
	def skinToneSelected(self, color):
		self.setSkinColor(color)
		
		# Hide the skin color picker
		dialog = Gui.findByName("CharacterCreation3")
		dialog.findByName("SkinColorPicker").visible = False
		
		# Show all other controls
		self.setDialog3Visibility(True)
		
	"""
		This gets triggered when a hair color is selected.
	"""
	def hairColorSelected(self, color):
		if self.selectingColor == 'hair':
			self.setHairColor(color)
		elif self.selectingColor == 'facial':
			self.setFacialHairColor(color)
		
		# Hide the skin color picker
		dialog = Gui.findByName("CharacterCreation3")
		dialog.findByName("HairColorPicker").visible = False
		
		# Show all other controls
		self.setDialog3Visibility(True)		

	"""
		This gets triggered when a cloth color is selected.
	"""
	def clothColorSelected(self, color):
		if self.selectingColor == 'shirt':
			self.setShirtColor(color)
		elif self.selectingColor == 'pants':
			self.setPantsColor(color)
		
		# Hide the cloth color picker
		dialog = Gui.findByName("CharacterCreation3")
		dialog.findByName("ClothColorPicker").visible = False
		
		# Show all other controls
		self.setDialog3Visibility(True)
	
	"""
		Hide or show the contents of dialog 3
	"""
	def setDialog3Visibility(self, visibility):
		dialog = Gui.findByName("CharacterCreation3")
		ids = ("MaleButton", "FemaleButton", "ShirtColorButton", "PantsColorButton", "SkinToneButton", "HairStyle", "HairColorButton", "FacialHairStyle", "FacialHairColorButton")
		for id in ids:
			ctrl = dialog.findByName(id)
			ctrl.visible = visibility

	"""
		Change one of the characters skills (1, 2 or 3)
	"""
	def setSkillValue(self, skill, value, dontnormalize = False):
		if self.normalizingSkills and not dontnormalize:
			return
		
		loginDialog = Gui.findByName("LoginDialog")
		
		# Update the scrollbar
		scroller = loginDialog.findByName("SkillScroller%u" % skill)
		if scroller.pos != value:
			scroller.pos = value
		
		# Update the label
		label = loginDialog.findByName("SkillLabel%u" % skill)
		label.text = str(value)
		
		# Save strength
		if skill == 1:
			self.skill1 = value
		elif skill == 2:
			self.skill2 = value
		elif skill == 3:
			self.skill3 = value			
		
		if not dontnormalize:
			self.normalizeSkills(skill)

	"""
		Change the character strength
	"""
	def setStrength(self, strength, dontnormalize = False):
		loginDialog = Gui.findByName("LoginDialog")
		
		# Update the scrollbar
		strengthScroller = loginDialog.findByName("StrengthScroller")
		if strengthScroller.pos != strength:
			strengthScroller.pos = strength
		
		# Update the label
		strengthLabel = loginDialog.findByName("StrengthLabel")
		strengthLabel.text = strength
		
		# Save strength
		self.strength = strength
		
		if not dontnormalize:
			self.normalizeStats(0)
	
	"""
		Change the character dexterity
	"""
	def setDexterity(self, dexterity, dontnormalize = False):
		loginDialog = Gui.findByName("LoginDialog")
		
		# Update the scrollbar
		dexterityScroller = loginDialog.findByName("DexterityScroller")
		if dexterityScroller.pos != dexterity:
			dexterityScroller.pos = dexterity
		
		# Update the label
		dexterityLabel = loginDialog.findByName("DexterityLabel")
		dexterityLabel.text = dexterity
		
		# Save dexterity
		self.dexterity = dexterity
		
		if not dontnormalize:
			self.normalizeStats(1)
			
	"""
		Change the character intelligence
	"""
	def setIntelligence(self, intelligence, dontnormalize = False):
		loginDialog = Gui.findByName("LoginDialog")
		
		# Update the scrollbar
		intelligenceScroller = loginDialog.findByName("IntelligenceScroller")
		if intelligenceScroller.pos != intelligence:
			intelligenceScroller.pos = intelligence
		
		# Update the label
		intelligenceLabel = loginDialog.findByName("IntelligenceLabel")
		intelligenceLabel.text = intelligence
		
		# Save intelligence
		self.intelligence = intelligence
		
		if not dontnormalize:
			self.normalizeStats(2)
			
	"""
	 If the statsum is greater than the maximum allowance, 
	 reduce several stats.
	"""
	def normalizeStats(self, excludeStat):
		# Temporary variables for str, dex, int
		strength = self.strength
		dexterity = self.dexterity
		intelligence = self.intelligence
		
		statsum = strength + dexterity + intelligence
		overflow = statsum - MAXIMUM_STATS
						
		if overflow > 0:
			# Exclude Strength
			if excludeStat == 0:
				if dexterity > intelligence:
					dexterity -= overflow
					if dexterity < MIN_STAT:
						intelligence -= MIN_STAT - dexterity
						dexterity = MIN_STAT					
				else:
					intelligence -= overflow;
					if intelligence < MIN_STAT:
						dexterity -= MIN_STAT - intelligence
						intelligence = MIN_STAT
				
			# Exclude Dexterity		
			elif excludeStat == 1:
				if strength > intelligence:
					strength -= overflow
					if strength < MIN_STAT:
						intelligence -= MIN_STAT - strength
						strength = MIN_STAT
				else:
					intelligence -= overflow
					if intelligence < MIN_STAT:
						strength -= MIN_STAT - intelligence
						intelligence = MIN_STAT
					
			# Exclude Intelligence
			elif excludeStat == 2:
				if strength > dexterity:
					strength -= overflow
					if strength < MIN_STAT:
						dexterity -= MIN_STAT - strength
						strength = MIN_STAT
				else:
					dexterity -= overflow
					if dexterity < MIN_STAT:
						strength -= MIN_STAT - dexterity
						dexterity = MIN_STAT
		elif overflow < 0:
			# Exclude Strength
			if excludeStat == 0:
				if dexterity < intelligence:
					dexterity -= overflow
					if dexterity < MIN_STAT:
						intelligence -= MIN_STAT - dexterity
						dexterity = MIN_STAT
				else:
					intelligence -= overflow;
				
			# Exclude Dexterity		
			elif excludeStat == 1:
				if strength < intelligence:
					strength -= overflow
					if strength < MIN_STAT:
						intelligence -= MIN_STAT - strength
						strength = MIN_STAT
				else:
					intelligence -= overflow
					
			# Exclude Intelligence
			elif excludeStat == 2:
				if strength < dexterity:
					strength -= overflow
					if strength < MIN_STAT:
						dexterity -= MIN_STAT - strength
						strength = MIN_STAT
				else:
					dexterity -= overflow

		if strength != self.strength:
			self.setStrength(strength, True) # Set strength but dont normalize again
		if dexterity != self.dexterity:
			self.setDexterity(dexterity, True) # Set dexterity but dont normalize again
		if intelligence != self.intelligence:
			self.setIntelligence(intelligence, True) # Set intelligence but dont normalize again
			
	"""
	 If the skillsum is greater than the maximum allowance, 
	 reduce several skills.
	"""
	def normalizeSkills(self, excludeSkill):
		if self.normalizingSkills:
			return
			
		self.normalizingSkills = True
		
		# Temporary variables for str, dex, int
		skill1 = self.skill1
		skill2 = self.skill2
		skill3 = self.skill3
		
		statsum = skill1 + skill2 + skill3
		overflow = statsum - 100

		if overflow > 0:
			# Exclude Skill1
			if excludeSkill == 1:
				if skill2 < skill3:
					skill2 -= overflow
					if skill2 < 0:
						skill3 -= 0 - skill2
						skill2 = 0
				else:
					skill3 -= overflow;
					if skill3 < 0:
						skill2 -= 0 - skill3
						skill3 = 0
				
			# Exclude Skill2		
			elif excludeSkill == 2:
				if skill1 < skill3:
					skill1 -= overflow
					if skill1 < 0:
						skill3 -= 0 - skill1
						skill1 = 0
				else:
					skill3 -= overflow
					if skill3 < 0:
						skill1 -= 0 - skill3
						skill3 = 0
					
			# Exclude Skill3
			elif excludeSkill == 3:
				if skill1 < skill2:
					skill1 -= overflow
					if skill1 < 0:
						skill2 -= 0 - skill1
						skill1 = 0
				else:
					skill2 -= overflow
					if skill2 < 0:
						skill1 -= 0 - skill2
						skill2 = 0
		elif overflow < 0:
			# Exclude Skill1
			if excludeSkill == 1:
				if skill2 > skill3:
					skill2 -= overflow
					if skill2 > 50:
						skill3 += skill2 - 50
						skill2 = 50
				else:
					skill3 -= overflow;
					if skill3 > 50:
						skill2 += skill3 - 50
						skill3 = 50
				
			# Exclude Skill2		
			elif excludeSkill == 2:
				if skill1 > skill3:
					skill1 -= overflow
					if skill1 > 50:
						skill3 += skill1 - 50
						skill1 = 50
				else:
					skill3 -= overflow
					if skill3 > 50:
						skill1 += skill3 - 50
						skill3 = 50
					
			# Exclude Skill3
			elif excludeSkill == 3:
				if skill1 > skill2:
					skill1 -= overflow
					if skill1 > 50:
						skill2 += skill1 - 50
						skill1 = 50
				else:
					skill2 -= overflow
					if skill2 > 50:
						skill1 += skill2 - 50
						skill2 = 50
		
		if skill1 != self.skill1:
			self.setSkillValue(1, skill1, True) # Set skill1 but dont normalize again
		if skill2 != self.skill1:
			self.setSkillValue(2, skill2, True) # Set skill2 but dont normalize again
		if skill3 != self.skill3:
			self.setSkillValue(3, skill3, True) # Set skill3 but dont normalize again			

		self.normalizingSkills = False

# Create the Character Creation context		
context = Context()

def initialize(dialog):
	global context
	if dialog.objectName == "CharacterCreation1":
		context.setupDialog1(dialog)
	elif dialog.objectName == "CharacterCreation2":
		context.setupDialog2(dialog)
	elif dialog.objectName == "CharacterCreation3":
		context.setupDialog3(dialog)

connect(LoginDialog, "showCharacterCreation()", context.showFirst)
connect(LoginDialog, "hideCharacterCreation()", context.hideAll)
