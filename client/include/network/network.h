
#if !defined(__NETWORK_H__)
#define __NETWORK_H__

#include <QObject>
#include <QString>
#include "outgoingpacket.h"

typedef struct _object PyObject;
class cMobile;

/*
	This class simplifies packet creation and allows exporting packet classes to Python
*/
class cNetwork : public QObject {
Q_OBJECT
public:
	void emitWarmodeChanged(bool atwar) {
		emit warmodeChanged(atwar);
	}
public slots:
	/*
		The argument is a tuple with the following elements:
		name, female (Bool), str, dex, int, 
		skill1Id, skill1Value, skill2Id, skill2Value, skill3Id, skill3Value,
		skinColor, hairColor, beardColor,
		hairStyle, beardStyle, (Item ids, 0 for none)
		startLocation, characterSlot, shirtHue, pantsHue
	*/
	PyObject *createCharacter(PyObject *parameters);
	void requestHelp();
	void changeWarmode(bool atwar);
	void requestStatus(cMobile *mobile);
	void showLargeStatus();
signals:
	void warmodeChanged(bool atwar);
};

extern cNetwork *Network;

#endif
