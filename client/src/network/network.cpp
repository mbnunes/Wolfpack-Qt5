
#include <Python.h>

#include "scripts.h"
#include "network/network.h"
#include "network/outgoingpackets.h"
#include "network/uosocket.h"
#include "python/utilities.h"

PyObject *cNetwork::createCharacter(PyObject *args) {
	cCharacterCreationInfo info;

	if (!Scripts->parseArguments(args, "QTbbbbbbbbbHHHHHbIHH", &info.name, &info.female, &info.strength, &info.dexterity, &info.intelligence,
		&info.skill1, &info.skill1Value, &info.skill2, &info.skill2Value, &info.skill3, &info.skill3Value,
		&info.skinColor, &info.hairColor, &info.beardColor,
		&info.hairStyle, &info.beardStyle,
		&info.startLocation, &info.characterSlot,
		&info.shirtHue, &info.pantsHue)) {
		return 0;
	}

	UoSocket->send(cCharacterCreationPacket(info));
	Py_RETURN_TRUE;
}

cNetwork *Network = 0;
