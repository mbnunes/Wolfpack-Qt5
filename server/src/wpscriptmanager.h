// WPScriptManager.h: Schnittstelle für die Klasse WPScriptManager.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_WPSCRIPTMANAGER_H__267F80C6_32E0_43C9_992F_9FAD5DECDC2C__INCLUDED_)
#define AFX_WPSCRIPTMANAGER_H__267F80C6_32E0_43C9_992F_9FAD5DECDC2C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "wpdefaultscript.h"
#include "wppythonscript.h"

#include <map>
#include <string>

//#include "wolfpack.h"

typedef std::map< std::string, WPDefaultScript* >::iterator ScriptIterator;

using namespace std;

enum WPScriptTypes
{
	WPST_DEFAULT	= 0,
	WPST_PYTHON		= 1,
	WPST_TRIGGER	= 2,
};

class WPScriptManager  
{
private:
	std::map< QString, WPDefaultScript* > Scripts;

public:
	WPScriptManager();
	virtual ~WPScriptManager();

	WPDefaultScript* FindScript( QString Name );
	void AddScript( QString Name, WPDefaultScript *Script );
	void RemoveScript( QString Name );

	void Load( void );
};

#endif // !defined(AFX_WPSCRIPTMANAGER_H__267F80C6_32E0_43C9_992F_9FAD5DECDC2C__INCLUDED_)
