
#include "dialogs/movecenter.h"
#include "gui/bordergump.h"
#include "gui/label.h"
#include "gui/imagebutton.h"
#include "game/world.h"

void upClicked(cControl *ctrl) {
	World->moveCenter(World->x() - 5, World->y() - 5, World->z());
}

void downClicked(cControl *ctrl) {
	World->moveCenter(World->x() + 5, World->y() + 5, World->z());
}

void leftClicked(cControl *ctrl) {
	World->moveCenter(World->x() - 5, World->y() + 5, World->z());
}

void rightClicked(cControl *ctrl) {
	World->moveCenter(World->x() + 5, World->y() - 5, World->z());
}

cMoveCenterDialog::cMoveCenterDialog() {
	setSize(115, 95);

	// Add a nice background image
	cBorderGump *border = new cBorderGump(0x2454);
	border->setAlign(CA_CLIENT);
	addControl(border);

	// Add buttons for moving the center of the world
	cImageButton *up = new cImageButton(50, 10, 0x15e0, 0x15e4);
	connect(up, SIGNAL(onButtonPress(cControl*)), this, SLOT(moveUp()));
    addControl(up);	

	cImageButton *left = new cImageButton(10, 40, 0x15e3, 0x15e7);
	connect(left, SIGNAL(onButtonPress(cControl*)), this, SLOT(moveLeft()));
    addControl(left);

	cImageButton *down = new cImageButton(50, 70, 0x15e2, 0x15e6);
	connect(down, SIGNAL(onButtonPress(cControl*)), this, SLOT(moveDown()));
    addControl(down);

	cImageButton *right = new cImageButton(90, 40, 0x15e1, 0x15e5);
	connect(right, SIGNAL(onButtonPress(cControl*)), this, SLOT(moveRight()));
    addControl(right);
}

void cMoveCenterDialog::moveUp(cControl *sender) {
}

void cMoveCenterDialog::moveDown(cControl *sender) {
}

void cMoveCenterDialog::moveRight(cControl *sender) {
}

void cMoveCenterDialog::moveLeft(cControl *sender) {
}

cMoveCenterDialog::~cMoveCenterDialog() {
}
