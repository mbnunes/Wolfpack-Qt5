
// Character templates
var CharacterTemplates = new Array(
	// Elements in order: Name, Name Cliloc Id, Description Cliloc Id, Gump Id, Skill1, Skill1Value, Skill2, Skill2Value, Skill3, Skill3Value, Strength, Dexterity, Intelligence
	new Array("Samurai", 1062948, 1062950, 5591, Skills.Bushido, 50, Skills.Swordsmanship, 50, Skills.Wrestling, 0, 40, 30, 10 ),
	new Array("Ninja", 1062949, 1062951, 5589, Skills.Ninjitsu, 50, Skills.Hiding, 50, Skills.Wrestling, 0, 40, 30, 10 ),
	new Array("Paladin", 1061177, 1061227, 5587, Skills.Chivalry, 50, Skills.Tactics, 50, Skills.Wrestling, 0, 45, 20, 15 ),
	new Array("Necromancer", 1061178, 1061228, 5557, Skills.Necromancy, 50, Skills.Swordsmanship, 30, Skills.Tactics, 20, 25, 20, 35 ),
	new Array("Warrior", 1061180, 1061230, 5577, Skills.Tactics, 50, Skills.Healing, 45, Skills.Swordsmanship, 5, 35, 35, 10 ),
	new Array("Mage", 1061179, 1061229, 5569, Skills.Magery, 50, Skills.Meditation, 50, Skills.Wrestling, 0, 25, 10, 45 ),
	new Array("Blacksmith", 1061181, 1061231, 5555, Skills.Blacksmithy, 50, Skills.Tinkering, 45, Skills.Mining, 5, 60, 10, 10 ),
	new Array("Advanced", 3000448, 3000448, 5505, Skills.Alchemy, 0, Skills.Alchemy, 0, Skills.Alchemy, 0, 30, 25, 25)
);

const CTINDEX_NAME = 0; // Index Constant for the template name
const CTINDEX_LOCALIZEDNAME = 1;
const CTINDEX_LOCALIZEDDESC = 2;
const CTINDEX_GUMP = 3;
const CTINDEX_STRENGTH = 10;
const CTINDEX_DEXTERITY = 11;
const CTINDEX_INTELLIGENCE = 12;

// Note: This won't work with sphere
const MAXIMUM_STATS = 80;
const MAX_STAT = 60;
const MIN_STAT = 10;
const SKILLCOUNT = Skills.count();

/**
 * Select a profession
 */
function characterCreation1Next(button) {
	// We only process profession buttons here
	if (!button.hasTag("profession")) {
		return;
	}

	characterCreation.selectProfession(parseInt(button.getTag("profession")));
}

/**
 * Strength scroller has been used.
 */
function ccStrengthScrolled(oldpos) {
	var loginDialog = Gui.findByName("LoginDialog");
	var strengthScroller = loginDialog.findByName("StrengthScroller");
	characterCreation.setStrength(strengthScroller.pos);
}

/**
 * Dexterity scroller has been used.
 */
function ccDexterityScrolled(oldpos) {
	var loginDialog = Gui.findByName("LoginDialog");
	var dexterityScroller = loginDialog.findByName("DexterityScroller");
	characterCreation.setDexterity(dexterityScroller.pos);
}

/**
 * Intelligence scroller has been used.
 */
function ccIntelligenceScrolled(oldpos) {
	var loginDialog = Gui.findByName("LoginDialog");
	var intelligenceScroller = loginDialog.findByName("IntelligenceScroller");
	characterCreation.setIntelligence(intelligenceScroller.pos);
}

/**
 * Show the first Character Creation screen again
 */
function characterCreation2Back() {
	var loginDialog = Gui.findByName("LoginDialog");
	loginDialog.findByName("CharacterCreation2").visible = false;
	loginDialog.findByName("CharacterCreation1").visible = true;
}

class CharacterCreation {
	// Currently selected profession
	var profession = -1;
	var strength = 10;
	var dexterity = 10;
	var intelligence = 10;
	
	/**
	 * Select the character profession.
	 */
	function selectProfession(id) {	
		this.profession = id;
		
		setStrength(CharacterTemplates[id][CTINDEX_STRENGTH]);
		setDexterity(CharacterTemplates[id][CTINDEX_DEXTERITY]);
		setIntelligence(CharacterTemplates[id][CTINDEX_INTELLIGENCE]);
		
		// Hide current dialog and show next
		var loginDialog = Gui.findByName("LoginDialog");
		loginDialog.findByName("CharacterCreation1").visible = false;
		loginDialog.findByName("CharacterCreation2").visible = true;
	}
	
	/**
	 * Initialize the Character creation dialog
	 */
	static function initialize(dialog) {
		if (dialog.objectName == "CharacterCreation1") {
			characterCreation.setupDialog1(dialog);
		} else if (dialog.objectName == "CharacterCreation2") {
			characterCreation.setupDialog2(dialog);
		}
	}
	
	/**
	 * Setup the profession selection dialog
	 */
	function setupDialog1(dialog) {
	    // Center the title label
	    var label = dialog.findByName("TitleLabel");
	    if (label != undefined) {
	    	label.update(); // Make sure the size is correct
	    	label.x = label.x + (400 - label.width) / 2;
	    }

	    var xpos = 135;
	    var ypos = 147;

	    // Create the advanced character option and wire it to this script
	    for (var i = 0; i < CharacterTemplates.length; ++i) {
	    	var template = CharacterTemplates[i];
	    	var optn = Gui.createDialog("CharacterTemplate");
	    	if (optn != undefined) {
	    		// Set the label text and the gump id
	    		var label = optn.findByName("TemplateLabel");
	    		label.text = Localization.get(template[CTINDEX_LOCALIZEDNAME]);
	    		var btn = optn.findByName("TemplateButton");
	    		btn.setStateGump(Button.Unpressed, template[CTINDEX_GUMP]);
	    		btn.setStateGump(Button.Pressed, template[CTINDEX_GUMP] + 1);
	    		btn.setStateGump(Button.Hover, template[CTINDEX_GUMP] + 1);
	    		btn.setTag("profession", i);
	    		connect(btn, "onButtonPress(cControl*)", "characterCreation1Next");
	    			    		
	    		optn.setPosition(xpos, ypos);
	    		dialog.addControl(optn);
	    		
	    		if (i % 2 == 0) {
	    			xpos += 205;
	    		} else {
	    			xpos -= 205;
	    			ypos += 70;
	    		}
	    	}
	    }
	    
	    //dialog.visible = true;
	}

	/**
	 * Setup the advanced profession options dialog
	 */
	function setupDialog2(dialog) {
		// Center the title label
		var label = dialog.findByName("TitleLabel");
		if (label != undefined) {
			label.update(); // Make sure the size is correct
			label.x = label.x + (400 - label.width) / 2;
		}
	
		var backButton = dialog.findByName("BackButton");
		connect(backButton, "onButtonPress(cControl*)", "characterCreation2Back");
		
		connect(dialog.findByName("StrengthScroller"), "scrolled(int)", "ccStrengthScrolled");
		connect(dialog.findByName("DexterityScroller"), "scrolled(int)", "ccDexterityScrolled");
		connect(dialog.findByName("IntelligenceScroller"), "scrolled(int)", "ccIntelligenceScrolled");

		var items = new Array();
		
		var i;
		for (i = 0; i < SKILLCOUNT; ++i) {			
			items.push(Localization.get(1044060 + i));
		}
		
		// Set up the combo boxes
		var cb1 = dialog.findByName("SkillBox1");
		if (cb1 != undefined) {
			cb1.setItems(items);
		}
	}
	
	/**
	 * If the statsum is greater than the maximum allowance, 
	 * reduce several stats.
	 */
	function normalizeStats(excludeStat) {
		// Temporary variables for str, dex, int
		var strength = this.strength;
		var dexterity = this.dexterity;
		var intelligence = this.intelligence;
		
		var statsum = strength + dexterity + intelligence;
		var overflow = statsum - MAXIMUM_STATS;
						
		if (overflow > 0) {
			switch (excludeStat) {
				// Exclude Strength
				case 0:
					if (dexterity > intelligence) {
						dexterity -= overflow;
						if (dexterity < MIN_STAT) {
							intelligence -= MIN_STAT - dexterity;
							dexterity = MIN_STAT;
						}
					} else {
						intelligence -= overflow;
						if (intelligence < MIN_STAT) {
							dexterity -= MIN_STAT - intelligence;
							intelligence = MIN_STAT;
						}
					}
					break;
					
				// Exclude Dexterity
				case 1:
					if (strength > intelligence) {
						strength -= overflow;
						if (strength < MIN_STAT) {
							intelligence -= MIN_STAT - strength;
							strength = MIN_STAT;
						}
					} else {
						intelligence -= overflow;
						if (intelligence < MIN_STAT) {
							strength -= MIN_STAT - intelligence;
							intelligence = MIN_STAT;
						}
					}
					break;
					
				// Exclude Intelligence
				case 2:
					if (strength > dexterity) {
						strength -= overflow;
						if (strength < MIN_STAT) {
							dexterity -= MIN_STAT - strength;
							strength = MIN_STAT;
						}							
					} else {
						dexterity -= overflow;
						if (dexterity < MIN_STAT) {
							strength -= MIN_STAT - dexterity;
							dexterity = MIN_STAT;
						}
					}
					break;
			}
		} else if (overflow < 0) {
			switch (excludeStat) {
				// Exclude Strength
				case 0:
					if (dexterity < intelligence) {
						dexterity -= overflow;
					} else {
						intelligence -= overflow;
					}
					break;
					
				// Exclude Dexterity
				case 1:
					if (strength < intelligence) {
						strength -= overflow;
					} else {
						intelligence -= overflow;
					}
					break;
					
				// Exclude Intelligence
				case 2:
					if (strength < dexterity) {
						strength -= overflow;
					} else {
						dexterity -= overflow;
					}
					break;
			}
		}			
		
		if (strength != this.strength) {
			setStrength(strength, true); // Set strength but dont normalize again
		}
		if (dexterity != this.dexterity) {
			setDexterity(dexterity, true); // Set dexterity but dont normalize again
		}
		if (intelligence != this.intelligence) {
			setIntelligence(intelligence, true); // Set intelligence but dont normalize again
		}				
	}
	
	/**
	 * Set the character strength.
	 */
	function setStrength(strength, dontnormalize) {		
		var loginDialog = Gui.findByName("LoginDialog");
		
		// Update the scrollbar
		var strengthScroller = loginDialog.findByName("StrengthScroller");
		if (strengthScroller.pos != strength) {
			strengthScroller.pos = strength;
		}
		
		// Update the label
		var strengthLabel = loginDialog.findByName("StrengthLabel");
		strengthLabel.text = strength;
		
		// Save strength
		this.strength = strength;
		
		if (!dontnormalize) {
			normalizeStats(0);
		}
	}
	
	/**
	 * Set the character dexterity.
	 */
	function setDexterity(dexterity, dontnormalize) {
		var loginDialog = Gui.findByName("LoginDialog");
		
		// Update the scrollbar
		var dexterityScroller = loginDialog.findByName("DexterityScroller");
		if (dexterityScroller.pos != dexterity) {
			dexterityScroller.pos = dexterity;
		}
		
		// Update the label
		var dexterityLabel = loginDialog.findByName("DexterityLabel");
		dexterityLabel.text = dexterity;
		
		// Save dexterity
		this.dexterity = dexterity;
		
		if (!dontnormalize) {
			normalizeStats(1);
		}
	}	
	
	/**
	 * Set the character intelligence.
	 */
	function setIntelligence(intelligence, dontnormalize) {
		var loginDialog = Gui.findByName("LoginDialog");
		
		// Update the scrollbar
		var intelligenceScroller = loginDialog.findByName("IntelligenceScroller");
		if (intelligenceScroller.pos != intelligence) {
			intelligenceScroller.pos = intelligence;
		}
		
		// Update the label
		var intelligenceLabel = loginDialog.findByName("IntelligenceLabel");
		intelligenceLabel.text = intelligence;
		
		// Save intelligence
		this.intelligence = intelligence;
		
		if (!dontnormalize) {
			normalizeStats(2);
		}
	}	
}

var characterCreation = new CharacterCreation();
