// WPPythonScript.h: Schnittstelle für die Klasse WPPythonScript.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_WPPYTHONSCRIPT_H__F98DC0D9_AE54_46A6_9557_729DBDD07C03__INCLUDED_)
#define AFX_WPPYTHONSCRIPT_H__F98DC0D9_AE54_46A6_9557_729DBDD07C03__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "wpdefaultscript.h"
#include "Python.h"
#include "pyerrors.h"

class WPPythonScript : public WPDefaultScript  
{
private:
	PyObject *codeModule; // This object stores the compiled Python Module

public:
	virtual const QString Type( void ) {
		return "python";
	};
	
	WPPythonScript() {};
	virtual ~WPPythonScript() {};

	virtual void load( const QDomElement &Data );

	// Normal Events
	virtual bool onUse( P_CHAR User, P_ITEM Used );

	virtual bool onShowItemName( P_ITEM Item, P_CHAR Viewer );
	virtual bool onShowCharName( P_CHAR Character, P_CHAR Viewer );

	virtual void onCollideItem( P_CHAR Character, P_ITEM Obstacle );
	virtual void onCollideChar( P_CHAR Character, P_CHAR Obstacle );
	virtual bool onWalk( P_CHAR Character, UI08 Direction, UI08 Sequence );

	// if this events returns true (handeled) then we should not display the text
	virtual bool onTalk( P_CHAR Character, QString Text );

	virtual void onTalkToNPC( P_CHAR Talker, P_CHAR Character, const QString &Text );
	virtual void onTalkToItem( P_CHAR Talker, P_ITEM Item, const QString &Text );

	virtual void onWarModeToggle( P_CHAR Character, bool War );

	// Is the Client version already known to us here ???
	virtual bool onConnect( UOXSOCKET Socket, const QString &IP, const QString &Username, const QString &Password );

	virtual bool onDisconnect( UOXSOCKET Socket, QString IP );

	virtual void onEnterWorld( P_CHAR Character );

	virtual bool onHelp( P_CHAR Character );

	virtual bool onChat( P_CHAR Character );

	virtual bool onSkillUse( P_CHAR Character, UI08 Skill );
};

//////////////
// We will define our Python extensions here (!)
void initPythonExtensions( void );

inline void Py_WPDealloc( PyObject* self )
{
    PyObject_Del( self );
}

//////////////
// wolfpack.console
//	print( String )
//	progress( String )
//	printDone()
//	printFail()
//	printSkip()
PyObject* PyWPConsole_send( PyObject* self, PyObject* args );
PyObject* PyWPConsole_progress( PyObject* self, PyObject* args );
PyObject* PyWPConsole_printDone( PyObject* self, PyObject* args );
PyObject* PyWPConsole_printFail( PyObject* self, PyObject* args );
PyObject* PyWPConsole_printSkip( PyObject* self, PyObject* args );

PyObject* PyWPServer_shutdown( PyObject* self, PyObject* args );
PyObject* PyWPServer_save( PyObject* self, PyObject* args );

PyObject* PyWPItems_findbyserial( PyObject* self, PyObject* args );

PyObject* PyWPChars_findbyserial( PyObject* self, PyObject* args );

PyObject* PyWPMovement_deny( PyObject* self, PyObject* args );
PyObject* PyWPMovement_accept( PyObject* self, PyObject* args );

PyObject* PyWP_clients( PyObject* self, PyObject* args );

static PyMethodDef WPGlobalMethods[] = 
{
    { "console_send",		PyWPConsole_send,		METH_VARARGS, "Prints something to the wolfpack console" },
	{ "console_progress",	PyWPConsole_progress,	METH_VARARGS, "Prints a .....[xxxx] block" },
	{ "console_printDone",	PyWPConsole_printDone,	METH_VARARGS, "Prints a [done] block" },
	{ "console_printFail",	PyWPConsole_printFail,	METH_VARARGS, "Prints a [fail] block" },
	{ "console_printSkip",	PyWPConsole_printSkip,	METH_VARARGS, "Prints a [skip] block" },

	// .server
	{ "server_shutdown",	PyWPServer_shutdown,	METH_VARARGS, "Shuts the server down" },
	{ "server_save",		PyWPServer_save,		METH_VARARGS, "Saves the worldstate" },
	
	// .items
	{ "items_findbyserial",	PyWPItems_findbyserial,	METH_VARARGS, "Finds an item by it's serial" },

	// .chars	
	{ "chars_findbyserial",	PyWPChars_findbyserial,	METH_VARARGS, "Finds an char by it's serial" },

	// .movement
	{ "movement_accept",	PyWPMovement_accept,	METH_VARARGS, "Accepts the movement of the character" },
	{ "movement_deny",		PyWPMovement_deny,		METH_VARARGS, "Denies the movement of a character" },

	// .clients
	{ "clients",			PyWP_clients,			METH_VARARGS, "Retrieves the clientS object" },

    { NULL, NULL, 0, NULL }
};

//========================================= WPItem

typedef struct {
    PyObject_HEAD;
	P_ITEM Item;
} Py_WPItem;

PyObject *Py_WPItemGetAttr( Py_WPItem *self, char *name );
int Py_WPItemSetAttr( Py_WPItem *self, char *name, PyObject *value );
PyObject* Py_WPItembySerial( PyObject* self, PyObject* args );

// WPItem
PyObject* Py_WPItem_update( PyObject* self, PyObject* args );
PyObject* Py_WPItem_delete( PyObject* self, PyObject* args );

static PyTypeObject Py_WPItemType = {
    PyObject_HEAD_INIT(NULL)
    0,
    "WPItem",
    sizeof(Py_WPItemType),
    0,
    Py_WPDealloc,				/*tp_dealloc*/
    0,								/*tp_print*/
    (getattrfunc)Py_WPItemGetAttr,	/*tp_getattr*/
    (setattrfunc)Py_WPItemSetAttr,								/*tp_setattr*/
    0,								/*tp_compare*/
    0,								/*tp_repr*/
    0,								/*tp_as_number*/
    0,								/*tp_as_sequence*/
    0,								/*tp_as_mapping*/
    0,								/*tp_hash */
};

static PyMethodDef Py_WPItemMethods[] = 
{
    { "update", (getattrofunc)Py_WPItem_update, METH_VARARGS, "Sends the item to all clients in range." },
	{ "delete", (getattrofunc)Py_WPItem_delete, METH_VARARGS, "Deletes the item and the underlying reference." },
    { NULL, NULL, 0, NULL }
};

//========================================= WPChar
typedef struct {
    PyObject_HEAD;
	P_CHAR Char;
} Py_WPChar;

PyObject *Py_WPCharGetAttr( Py_WPChar *self, char *name );
int Py_WPCharSetAttr( Py_WPChar *self, char *name, PyObject *value );

// WPChar
PyObject* Py_WPChar_update( Py_WPChar* self, PyObject* args );
PyObject* Py_WPChar_message( Py_WPChar* self, PyObject* args );

static PyTypeObject Py_WPCharType = {
    PyObject_HEAD_INIT(NULL)
    0,
    "WPChar",
    sizeof(Py_WPCharType),
    0,
    Py_WPDealloc,				/*tp_dealloc*/
    0,								/*tp_print*/
    (getattrfunc)Py_WPCharGetAttr,	/*tp_getattr*/
    (setattrfunc)Py_WPCharSetAttr,	/*tp_setattr*/
    0,								/*tp_compare*/
    0,								/*tp_repr*/
    0,								/*tp_as_number*/
    0,								/*tp_as_sequence*/
    0,								/*tp_as_mapping*/
    0,								/*tp_hash */
};

static PyMethodDef Py_WPCharMethods[] = 
{
    { "update",  (getattrofunc)Py_WPChar_update, METH_VARARGS, "Sends the char to all clients in range." },
	{ "message", (getattrofunc)Py_WPChar_message, METH_VARARGS, "Displays a message above the characters head - only visible for the player." },
    { NULL, NULL, 0, NULL }
};

//========================================= WPClients

typedef struct {
    PyObject_HEAD;
} Py_WPClients;

int Py_WPClientsLength( Py_WPClients *self );
PyObject *Py_WPClientsGet( Py_WPClients *self, int Num );

static PySequenceMethods Py_WPClientsSequence = {
	(inquiry)Py_WPClientsLength,	/* sq_length */
	0,		/* sq_concat */
	0,		/* sq_repeat */
	(intargfunc)Py_WPClientsGet,			/* sq_item */
	0,		/* sq_slice */
	0,					/* sq_ass_item */
	0,					/* sq_ass_slice */
	0,		/* sq_contains */
};

static PyTypeObject Py_WPClientsType = {
    PyObject_HEAD_INIT(NULL)
    0,
    "WPClients",
    sizeof(Py_WPClientsType),
    0,
    Py_WPDealloc,						/*tp_dealloc*/
    0,									/*tp_print*/
    0,									/*tp_getattr*/
    0,									/*tp_setattr*/
    0,									/*tp_compare*/
    0,									/*tp_repr*/
    0,									/*tp_as_number*/
    &Py_WPClientsSequence,				/*tp_as_sequence*/
    0,									/*tp_as_mapping*/
    0,	/*tp_hash */
};

//================================= WPClient
typedef struct {
    PyObject_HEAD;
	UOXSOCKET Socket;
} Py_WPClient;

PyObject *Py_WPClientGetAttr( Py_WPClient *self, char *name );

static PyTypeObject Py_WPClientType = {
    PyObject_HEAD_INIT(NULL)
    0,
    "WPClient",
    sizeof(Py_WPClientType),
    0,
    Py_WPDealloc,						/*tp_dealloc*/
    0,									/*tp_print*/
    (getattrfunc)Py_WPClientGetAttr,		/*tp_getattr*/
    0,									/*tp_setattr*/
    0,									/*tp_compare*/
    0,									/*tp_repr*/
    0,									/*tp_as_number*/
    0,				/*tp_as_sequence*/
    0,									/*tp_as_mapping*/
    0,	/*tp_hash */
};

PyObject* Py_WPClient_disconnect( Py_WPClient* self, PyObject* args );
PyObject* Py_WPClient_send( Py_WPClient* self, PyObject* args );

static PyMethodDef Py_WPClientMethods[] = 
{
    { "disconnect",		(getattrofunc)Py_WPClient_disconnect,	METH_VARARGS, "Disconnects this socket." },
	{ "send",			(getattrofunc)Py_WPClient_send,			METH_VARARGS, "Sends a buffer to the socket." },
    { NULL, NULL, 0, NULL }
};

//================================= WPContent

typedef struct {
    PyObject_HEAD;
	cItem *Item;
} Py_WPContent;

int Py_WPContentLength( Py_WPContent *self );
PyObject *Py_WPContentGet( Py_WPContent *self, int Num );

static PySequenceMethods Py_WPContentSequence = {
	(inquiry)Py_WPContentLength,
	0,
	0,
	(intargfunc)Py_WPContentGet,
	0,
	0,
	0,
	0,
};

static PyTypeObject Py_WPContentType = {
    PyObject_HEAD_INIT(NULL)
    0,
    "WPContent",
    sizeof(Py_WPContentType),
    0,
    Py_WPDealloc,
    0,
    0,
    0,
    0,
    0,
    0,
    &Py_WPContentSequence,
    0,
    0,
};

//================================= WPEquipment

typedef struct {
    PyObject_HEAD;
	cChar *Char;
} Py_WPEquipment;

int Py_WPEquipmentLength( Py_WPEquipment *self );
PyObject *Py_WPEquipmentGet( Py_WPEquipment *self, int Num );

static PySequenceMethods Py_WPEquipmentSequence = {
	(inquiry)Py_WPEquipmentLength,
	0,
	0,
	(intargfunc)Py_WPEquipmentGet,
	0,
	0,
	0,
	0,
};

static PyTypeObject Py_WPEquipmentType = {
    PyObject_HEAD_INIT(NULL)
    0,
    "WPEquipment",
    sizeof(Py_WPEquipmentType),
    0,
    Py_WPDealloc,
    0,
    0,
    0,
    0,
    0,
    0,
    &Py_WPEquipmentSequence,
    0,
    0,
};

#endif // !defined(AFX_WPPYTHONSCRIPT_H__F98DC0D9_AE54_46A6_9557_729DBDD07C03__INCLUDED_)
