//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
//  Copyright 2001-2003 by holders identified in authors.txt
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Palace - Suite 330, Boston, MA 02111-1307, USA.
//
//	* In addition to that license, if you are running this program or modified
//	* versions of it on a public system you HAVE TO make the complete source of
//	* the version used by you available or provide people with a location to
//	* download it.
//
//
//
//	Wolfpack Homepage: http://wpdev.sf.net/
//==================================================================================

#include "guilds.h"
#include "globals.h"
#include "persistentbroker.h"
#include "dbdriver.h"
#include "world.h"

cGuilds::~cGuilds() {
	for (iterator it = begin(); it != end(); ++it) {
		delete it.data();
	}
}

unsigned int cGuilds::findFreeSerial() {
	unsigned int serial = 1;

	for (iterator it = begin(); it != end(); ++it) {
		if (it.data()->serial() >= serial) {
			serial = it.data()->serial() + 1;
		}
	}
	
	return serial;
}

void cGuilds::save() {
	// Clear the tables first: guilds are not saved incremental.
	persistentBroker->executeQuery("DELETE FROM guilds;");
	persistentBroker->executeQuery("DELETE FROM guilds_members;");
	persistentBroker->executeQuery("DELETE FROM guilds_canidates;");

	for (iterator it = begin(); it != end(); ++it) {
		it.data()->save();
	}
}

void cGuilds::load() {
	// Get all guilds from the database
	cDBResult result = persistentBroker->query("SELECT serial,name,abbreviation,charta,website,alignment,leader,founded,guildstone FROM guilds");

	while (result.fetchrow()) {
		cGuild *guild = new cGuild(false);
		guild->load(result);
		registerGuild(guild);
	}

	result.free();
}

void cGuild::load(const cDBResult &result) {
	serial_ = result.getInt(0);
	name_ = result.getString(1);
	abbreviation_ = result.getString(2);
	charta_ = result.getString(3);
	website_ = result.getString(4);
	alignment_ = (eAlignment)result.getInt(5);
	leader_ = dynamic_cast<P_PLAYER>(World::instance()->findChar(result.getInt(6)));
	founded_.setTime_t(result.getInt(7));
	guildstone_ = World::instance()->findItem(result.getInt(8));

	// Load members and canidates
	cDBResult members = persistentBroker->query(QString("SELECT player,showsign,guildtitle,joined FROM guilds_members WHERE guild = %1").arg(serial_));

	while (members.fetchrow()) {
		P_PLAYER player = dynamic_cast<P_PLAYER>(World::instance()->findChar(members.getInt(0)));

		if (player) {
			player->setGuild(this);
			members_.append(player);

			MemberInfo *info = new MemberInfo;
			info->setShowSign(members.getInt(1) != 0);
			info->setGuildTitle(members.getString(2));
			info->setJoined(members.getInt(3));
			memberinfo_.insert(player, info);
		}
	}

	members.free();

	cDBResult canidates = persistentBroker->query(QString("SELECT player FROM guilds_canidates WHERE guild = %1").arg(serial_));

	while (canidates.fetchrow()) {
		P_PLAYER player = dynamic_cast<P_PLAYER>(World::instance()->findChar(canidates.getInt(0)));

		if (player) {
			player->setGuild(this);
			canidates_.append(player);
		}
	}

	canidates.free();

	// Clear the leader if he's not a member of the guild
	if (!members_.contains(leader_)) {
		leader_ = 0;
	}
}

void cGuild::save() {
	QStringList fields;
	fields.append(QString::number(serial_));
	fields.append(QString("'%1'").arg(persistentBroker->quoteString(name_)));
	fields.append(QString("'%1'").arg(persistentBroker->quoteString(abbreviation_)));
	fields.append(QString("'%1'").arg(persistentBroker->quoteString(charta_)));
	fields.append(QString("'%1'").arg(persistentBroker->quoteString(website_)));
	fields.append(QString::number(alignment_));
	if (leader_) {
		fields.append(QString::number(leader_->serial()));
	} else {
		fields.append("-1");
	}
	fields.append(QString::number(founded_.toTime_t()));
	if (guildstone_) {
		fields.append(QString::number(guildstone_->serial()));
	} else {
		fields.append("-1");
	}

	persistentBroker->executeQuery(QString("INSERT INTO guilds VALUES(%1);").arg(fields.join(",")));

	// Save Members/Canidates
	P_PLAYER player;

	for (player = members_.first(); player; player = members_.next()) {
		MemberInfo *info = getMemberInfo(player);
		persistentBroker->executeQuery(QString("INSERT INTO guilds_members VALUES(%1,%2,%3,'%4',%5);")
			.arg(serial_).arg(player->serial()).arg(info->showSign() ? 1 : 0)
			.arg(persistentBroker->quoteString(info->guildTitle()))
			.arg(info->joined()));
	}

	for (player = canidates_.first(); player; player = canidates_.next()) {
		persistentBroker->executeQuery(QString("INSERT INTO guilds_canidates VALUES(%1,%2);").arg(serial_).arg(player->serial()));
	}
}

cGuild::cGuild(bool createSerial) {
	if (createSerial) {
		serial_ = Guilds::instance()->findFreeSerial();
		Guilds::instance()->registerGuild(this);
	} else {
		serial_ = 0;
	}

	alignment_ = Neutral;
	name_ = QString::null;
	abbreviation_ = QString::null;
	website_ = QString::null;
	charta_ = QString::null;
	founded_ = QDateTime::currentDateTime();
	guildstone_ = 0;
	leader_ = 0;
}

cGuild::~cGuild() {
	P_PLAYER player;

	for (player = members_.first(); player; player = members_.next()) {
		player->setGuild(0);

		// Remove the MemberInfo structure
		MemberInfo *info = getMemberInfo(player);
		delete info;
		memberinfo_.remove(player);
	}

	for (player = canidates_.first(); player; player = canidates_.next()) {
		player->setGuild(0);

		// Remove the MemberInfo structure
		MemberInfo *info = getMemberInfo(player);
		delete info;
		memberinfo_.remove(player);	
	}
}

void cGuild::setSerial(unsigned int data)
 {
	if (serial_ != data && data) {
		if (serial_) {
			Guilds::instance()->unregisterGuild(this);
		}

		serial_ = data;		
		Guilds::instance()->registerGuild(this);
	}
}

void cGuilds::registerGuild(cGuild *guild) {
	guilds.insert(guild->serial(), guild, true);
}

void cGuilds::unregisterGuild(cGuild *guild) {
	guilds.remove(guild->serial());
}

cGuild *cGuilds::findGuild(unsigned int serial) {
	cGuild *result = 0;

	if (serial) {
		iterator it = guilds.find(serial);

		if (it != end()) {
			result = it.data();
		}
	}

	return result;
}

void cGuild::addMember(P_PLAYER member, MemberInfo *info) {
	if (!members_.contains(member)) {
		canidates_.remove(member);
		members_.append(member);

		if (memberinfo_.contains(member)) {
			delete memberinfo_[member];
			memberinfo_.remove(member);
		}

		if (!info) {
			info = new MemberInfo;
		}

		memberinfo_.insert(member, info);
		member->setGuild(this);
	}
}

void cGuild::removeMember(P_PLAYER member) {
	canidates_.remove(member);
	members_.remove(member);
	member->setGuild(0);

  if (leader_ == member) {
    leader_ = 0;
  }
}

void cGuild::addCanidate(P_PLAYER canidate) {
	// Only add him to the canidates if he is not a member already
	if (!members_.contains(canidate)) {	
		canidates_.append(canidate);
		canidate->setGuild(this);
	}
}

void cGuild::removeCanidate(P_PLAYER canidate) {
	members_.remove(canidate);
	canidates_.remove(canidate);	
	canidate->setGuild(0);
}

const char *cGuild::className() const {
	return "guild";
}

bool cGuild::implements(const QString &name) const {
	if (name == cGuild::className()) {
		return true;
	} else {
		return cPythonScriptable::implements(name);
	}
}

/*!
	The python object structure for a guild.
*/
struct wpGuild {
    PyObject_HEAD;
	cGuild *guild;
};

static PyObject *wpGuild_getAttr(wpGuild *self, char *name);
static int wpGuild_setAttr(wpGuild *self, char *name, PyObject *value);
static int wpGuild_compare(PyObject *a, PyObject *b);

/*!
	The python type object for the guild type.
*/
PyTypeObject wpGuildType = {
    PyObject_HEAD_INIT(NULL)
    0,
    "guild",
    sizeof(wpGuildType),
    0,
    wpDealloc,
    0,
    (getattrfunc)wpGuild_getAttr,
    (setattrfunc)wpGuild_setAttr,
	wpGuild_compare,
};

static int wpGuild_compare(PyObject *a, PyObject *b) {
	if (a->ob_type == &wpGuildType && b->ob_type == &wpGuildType) {
		if (((wpGuild*)a)->guild == ((wpGuild*)b)->guild) {
			return 0;
		}
	}

	return PyObject_Compare(a, b);
}

PyObject *wpGuild_addmember(wpGuild *self, PyObject *args) {
	P_CHAR character;
	if (PyArg_ParseTuple(args, "O&:guild.addmember(char)", PyConvertChar, &character)) {
		P_PLAYER player = dynamic_cast<P_PLAYER>(character);
		if (player) {
			self->guild->addMember(player);
			return PyTrue;
		} else {
			return PyFalse;
		}
	}
	return 0;
}

PyObject *wpGuild_removemember(wpGuild *self, PyObject *args) {
	P_CHAR character;
	if (PyArg_ParseTuple(args, "O&:guild.removemember(char)", PyConvertChar, &character)) {
		P_PLAYER player = dynamic_cast<P_PLAYER>(character);
		if (player) {
			self->guild->removeMember(player);
			return PyTrue;
		} else {
			return PyFalse;
		}
	}
	return 0;
}

PyObject *wpGuild_addcanidate(wpGuild *self, PyObject *args) {
	P_CHAR character;
	if (PyArg_ParseTuple(args, "O&:guild.addcanidate(char)", PyConvertChar, &character)) {
		P_PLAYER player = dynamic_cast<P_PLAYER>(character);
		if (player) {
			self->guild->addCanidate(player);
			return PyTrue;
		} else {
			return PyFalse;
		}
	}
	return 0;
}

PyObject *wpGuild_removecanidate(wpGuild *self, PyObject *args) {
	P_CHAR character;
	if (PyArg_ParseTuple(args, "O&:guild.removecanidate(char)", PyConvertChar, &character)) {
		P_PLAYER player = dynamic_cast<P_PLAYER>(character);
		if (player) {
			self->guild->removeCanidate(player);
			return PyTrue;
		} else {
			return PyFalse;
		}
	}
	return 0;
}

PyObject *wpGuild_setmemberinfo(wpGuild *self, PyObject *args) {
	P_PLAYER player;
	PyObject *dict;

	if (!PyArg_ParseTuple(args, "O&O!:guild.setmemberinfo(player, info)", PyConvertPlayer, &player, &PyDict_Type, &dict)) {
		return 0;
	}

	cGuild::MemberInfo *info = self->guild->getMemberInfo(player);

	if (!info) {
		PyErr_SetString(PyExc_RuntimeError, "The supplied character is not a member of this guild.");
		return 0;
	}

	PyObject *showsign = PyDict_GetItemString(dict, "showsign");
	if (showsign) {
		info->setShowSign(PyObject_IsTrue(showsign));
	}

	PyObject *guildtitle = PyDict_GetItemString(dict, "guildtitle");
	if (guildtitle) {
		if (PyUnicode_Check(guildtitle)) {
			info->setGuildTitle(QString::fromUcs2((ushort*)PyUnicode_AS_UNICODE(guildtitle)));
		} else if (PyString_Check(guildtitle)) {
			info->setGuildTitle(QString::fromUtf8(PyString_AsString(guildtitle)));
		} else {
			info->setGuildTitle(QString::null);
		}
	}

	PyObject *joined = PyDict_GetItemString(dict, "joined");
	if (joined) {
		if (PyInt_Check(joined)) {
			info->setJoined((unsigned int)PyInt_AsLong(joined));
		} else if (PyLong_Check(joined)) {
			info->setJoined((unsigned int)floor(PyFloat_AsDouble(joined)));
		}
	}

	return PyTrue;
}

PyObject *wpGuild_getmemberinfo(wpGuild *self, PyObject *args) {
	P_PLAYER player;

	if (!PyArg_ParseTuple(args, "O&:guild.getmemberinfo(player)", PyConvertPlayer, &player)) {
		return 0;
	}

	cGuild::MemberInfo *info = self->guild->getMemberInfo(player);

	if (!info) {
		PyErr_SetString(PyExc_RuntimeError, "The supplied character is not a member of this guild.");
		return 0;
	}

    // Return a dictionary with two elements
	PyObject *result = PyDict_New();
	PyDict_SetItemString(result, "showsign", PyInt_FromLong(info->showSign() ? 1 : 0));
	PyDict_SetItemString(result, "joined", PyInt_FromLong(info->joined()));
	
	if (info->guildTitle().isNull()) {
		PyDict_SetItemString(result, "guildtitle", PyUnicode_FromWideChar(L"", 0));		
	} else {
		PyDict_SetItemString(result, "guildtitle", PyUnicode_FromUnicode((Py_UNICODE*)info->guildTitle().ucs2(), info->guildTitle().length()));		
	}

	return result;
}

PyObject *wpGuild_delete(wpGuild *self, PyObject *args) {
	Guilds::instance()->unregisterGuild(self->guild);
	delete self->guild;
	self->guild = 0;
	return PyTrue;
}

static PyMethodDef wpGuildMethods[] = {
	{"delete", (getattrofunc)wpGuild_delete, METH_VARARGS, 0},
	{"addmember", (getattrofunc)wpGuild_addmember, METH_VARARGS, 0},
	{"removemember", (getattrofunc)wpGuild_removemember, METH_VARARGS, 0},
	{"addcanidate", (getattrofunc)wpGuild_addcanidate, METH_VARARGS, 0},
	{"removecanidate", (getattrofunc)wpGuild_removecanidate, METH_VARARGS, 0},
	{"getmemberinfo", (getattrofunc)wpGuild_getmemberinfo, METH_VARARGS, 0},
	{"setmemberinfo", (getattrofunc)wpGuild_setmemberinfo, METH_VARARGS, 0},
	{0, 0, 0, 0}
};

static PyObject *wpGuild_getAttr(wpGuild *self, char *name) {
	if (!self->guild) {
		PyErr_SetString(PyExc_RuntimeError, "Calling member functions after deletion.");
		return 0;
	}

	if (!strcmp(name, "members")) {
		QPtrList<cPlayer> &members = self->guild->members();
		PyObject *list = PyList_New(members.count());
		unsigned int i = 0;

		for (P_PLAYER player = members.first(); player; player = members.next()) {
			PyList_SetItem(list, i++, player->getPyObject());
		}

		return list;
	} else if (!strcmp(name, "canidates")) {
		QPtrList<cPlayer> &canidates = self->guild->canidates();
		PyObject *list = PyList_New(canidates.count());
		unsigned int i = 0;

		for (P_PLAYER player = canidates.first(); player; player = canidates.next()) {
			PyList_SetItem(list, i++, player->getPyObject());
		}

		return list;
	} else if (!strcmp(name, "leader")) {
		if (!self->guild->leader()) {
			Py_INCREF(Py_None);
			return Py_None;
		} else {
			return self->guild->leader()->getPyObject();
		}
	} else if (!strcmp(name, "name")) {
		const QString &value = self->guild->name();
		if (value == QString::null) {
			return PyUnicode_FromWideChar(L"", 0);
		} else {
			return PyUnicode_FromUnicode((Py_UNICODE*)value.ucs2(), value.length());
		}
	} else if (!strcmp(name, "abbreviation")) {
		const QString &value = self->guild->abbreviation();
		if (value == QString::null) {
			return PyUnicode_FromWideChar(L"", 0);
		} else {
			return PyUnicode_FromUnicode((Py_UNICODE*)value.ucs2(), value.length());
		}
	} else if (!strcmp(name, "charta")) {
		const QString &value = self->guild->charta();
		if (value == QString::null) {
			return PyUnicode_FromWideChar(L"", 0);
		} else {
			return PyUnicode_FromUnicode((Py_UNICODE*)value.ucs2(), value.length());
		}
	} else if (!strcmp(name, "website")) {
		const QString &value = self->guild->website();
		if (value == QString::null) {
			return PyUnicode_FromWideChar(L"", 0);
		} else {
			return PyUnicode_FromUnicode((Py_UNICODE*)value.ucs2(), value.length());
		}
	} else if (!strcmp(name, "alignment")) {
		return PyInt_FromLong(self->guild->alignment());
	} else if (!strcmp(name, "serial")) {
		return PyInt_FromLong(self->guild->serial());
	} else if (!strcmp(name, "founded")) {
		return PyInt_FromLong(self->guild->founded().toTime_t());
	} else if (!strcmp(name, "guildstone")) {
		if (self->guild->guildstone()) {
			return self->guild->guildstone()->getPyObject();
		} else {
			Py_INCREF(Py_None);
			return Py_None;
		}
	}

	return Py_FindMethod(wpGuildMethods, (PyObject*)self, name);
}

static int wpGuild_setAttr(wpGuild *self, char *name, PyObject *value) {
	if (!self->guild) {
		PyErr_SetString(PyExc_RuntimeError, "Calling member functions after deletion.");
		return 0;
	}

	if (!strcmp(name, "leader")) {
		P_PLAYER leader = dynamic_cast<P_PLAYER>(getWpChar(value));
		self->guild->setLeader(leader);
	} else if (!strcmp(name, "guildstone")) {
		P_ITEM guildstone = getWpItem(value);
		self->guild->setGuildstone(guildstone);
	} else if (!strcmp(name, "name")) {
		if (PyUnicode_Check(value)) {
			self->guild->setName(QString::fromUcs2((ushort*)PyUnicode_AS_UNICODE(value)));
		} else if (PyString_Check(value)) {
			self->guild->setName(QString::fromUtf8(PyString_AsString(value)));
		} else {
			self->guild->setName(QString::null);
		}
	} else if (!strcmp(name, "abbreviation")) {
		if (PyUnicode_Check(value)) {
			self->guild->setAbbreviation(QString::fromUcs2((ushort*)PyUnicode_AS_UNICODE(value)));
		} else if (PyString_Check(value)) {
			self->guild->setAbbreviation(QString::fromUtf8(PyString_AsString(value)));
		} else {
			self->guild->setAbbreviation(QString::null);
		}
	} else if (!strcmp(name, "charta")) {
		if (PyUnicode_Check(value)) {
			self->guild->setCharta(QString::fromUcs2((ushort*)PyUnicode_AS_UNICODE(value)));
		} else if (PyString_Check(value)) {
			self->guild->setCharta(QString::fromUtf8(PyString_AsString(value)));
		} else {
			self->guild->setCharta(QString::null);
		}
	} else if (!strcmp(name, "website")) {
		if (PyUnicode_Check(value)) {
			self->guild->setWebsite(QString::fromUcs2((ushort*)PyUnicode_AS_UNICODE(value)));
		} else if (PyString_Check(value)) {
			self->guild->setWebsite(QString::fromUtf8(PyString_AsString(value)));
		} else {
			self->guild->setWebsite(QString::null);
		}
	} else if (!strcmp(name, "alignment")) {
		if (PyInt_Check(value)) {
			self->guild->setAlignment((cGuild::eAlignment)PyInt_AsLong(value));
		}
	} else if (!strcmp(name, "founded")) {
		if (PyInt_Check(value)) {
			QDateTime founded;
			founded.setTime_t(PyInt_AsLong(value));
			self->guild->setFounded(founded);
		} else if (PyFloat_Check(value)) {
			QDateTime founded;
			founded.setTime_t((int)ceil(PyFloat_AsDouble(value)));
			self->guild->setFounded(founded);
		}
	} else {
		return -1;
	}

	return 0;
}

// Python Implementation
PyObject *cGuild::getPyObject() {
	wpGuild *returnVal = PyObject_New(wpGuild, &wpGuildType);
	returnVal->guild = this;
	return (PyObject*)returnVal;
}
