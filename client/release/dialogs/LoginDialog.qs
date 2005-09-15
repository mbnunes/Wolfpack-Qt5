
class LoginDialog {    
    static function initialize(dialog) {
	var charcreation1 = Gui.createDialog("CharacterCreation1");
	if (charcreation1 != undefined) {
	    dialog.addControl(charcreation1);
	}
	
	var charcreation2 = Gui.createDialog("CharacterCreation2");
	if (charcreation2 != undefined) {
	    dialog.addControl(charcreation2);
	}
    }
}
