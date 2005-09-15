
// Character templates
var CharacterTemplates = new Array(
	// Elements in order: Name, Name Cliloc Id, Description Cliloc Id, Gump Id, Skill1, Skill1Value, Skill2, Skill2Value, Skill3, Skill3Value, Strength, Dexterity, Intelligence
	new Array("Samurai", 1062948, 1062950, 5591, 'Bushido', 50, 'Swordsmanship', 50, 'Wrestling', 0, 40, 30, 10 ),
	new Array("Ninja", 1062949, 1062951, 5589, 'Ninjitsu', 50, 'Hiding', 50, 'Wrestling', 0, 40, 30, 10 ),
	new Array("Paladin", 1061177, 1061227, 5587, 'Chivalry', 50, 'Tactics', 50, 'Wrestling', 0, 45, 20, 15 ),
	new Array("Necromancer", 1061178, 1061228, 5557, 'Necromancy', 50, 'Swordsmanship', 30, 'Tactics', 20, 25, 20, 35 ),
	new Array("Warrior", 1061180, 1061230, 5577, 'Tactics', 50, 'Healing', 45, 'Swordsmanship', 5, 35, 35, 10 ),
	new Array("Mage", 1061179, 1061229, 5569, 'Magery', 50, 'Meditation', 50, 'Wrestling', 0, 25, 10, 45 ),
	new Array("Blacksmith", 1061181, 1061231, 5555, 'Blacksmith', 50, 'Tinkering', 45, 'Mining', 5, 60, 10, 10 ),
	new Array("Advanced", 3000448, 3000448, 5505, '', 0, '', 0, '', 0, 30, 25, 25)
);

const CTINDEX_NAME = 0; // Index Constant for the template name
const CTINDEX_LOCALIZEDNAME = 1;
const CTINDEX_LOCALIZEDDESC = 2;
const CTINDEX_GUMP = 3;

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
	
	/**
	 * Select the character profession.
	 */
	function selectProfession(id) {	
		this.profession = id;
		
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
	    
	    dialog.visible = true;
	}
	
	/**
	 * Setup the advanced profession options dialog
	 */
	function setupDialog2(dialog) {
		var backButton = dialog.findByName("BackButton");
		connect(backButton, "onButtonPress(cControl*)", "characterCreation2Back");
	}	
}

var characterCreation = new CharacterCreation();
