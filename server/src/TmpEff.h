//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
//	Copyright 1997, 98 by Marcus Rating (Cironian)
//  Copyright 2001 by holders identified in authors.txt
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
//	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
//	* In addition to that license, if you are running this program or modified
//	* versions of it on a public system you HAVE TO make the complete source of
//	* the version used by you available or provide people with a location to
//	* download it.
//
//
//
//	Wolfpack Homepage: http://wpdev.sf.net/
//========================================================================================

// TmpEff.h: interface for the TmpEff class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(__TMPEFF_H__)
#define __TMPEFF_H__ 

//Platform specifics
#include "platform.h"

//System includes
#include <vector>
#include <functional>
#include <algorithm>

//Forward class declarations
class cTempEffect;
class cTmpEff;
class cScriptEffect;
class cTimedSpellAction;
class cTempEffects;

// Wolfpack includes
#include "typedefs.h"
#include "iserialization.h"

// Library includes
#include "Python.h"

class cTempEffect : public cSerializable
{
protected:
	SERIAL		sourSer;
	SERIAL		destSer;
	QString		objectid;
	bool		serializable;

public:
	unsigned int expiretime;
	unsigned char dispellable;

	// Fib Heap Node variables 
	cTempEffect*	left;
	cTempEffect*	right;
	cTempEffect*	father;
	cTempEffect*	son;
	unsigned int	rank;
	bool			marker;

public:
//	cTempEffect() { serializable = true; }
	cTempEffect( cTempEffect* left_ = NULL, cTempEffect* right_ = NULL, cTempEffect* father_ = NULL,
				cTempEffect* son_ = NULL, int rank_ = 0, bool marker_ = false )
	{
		serializable = true;
		left = left_;
		right = right_;
		if( !left )
			left = this;
		if( !right )
			right = this;
		father = father_;
		son = son_;
		rank = rank_;
		marker = marker_;
	}

	virtual				~cTempEffect() {}
	void				setExpiretime_s(int seconds);
	void				setExpiretime_ms(float milliseconds);
	void				setDest(int ser);
	int					getDest();
	void				setSour(int ser);
	int					getSour();
	void				On(P_CHAR pc) {;}
	void				Off(P_CHAR pc) {;}
	virtual void		Expire() = 0;
	virtual void		Serialize(ISerialization &archive);
	virtual QString		objectID() const  { return objectid;}
	bool				isSerializable( void ) { return serializable; }
	void				setSerializable( bool data ) { serializable = data; }

	std::vector< cTempEffect* > asVector();
};

class cTmpEff : public cTempEffect
{
public:
	unsigned char num;
	unsigned short more1;
	unsigned short more2;
	unsigned short more3;
public:
	cTmpEff() { objectid = "TmpEff"; }
	virtual ~cTmpEff() {;}
	void Init();
	void Reverse();
	void On(P_CHAR pc);
	void Off(P_CHAR pc);
	virtual void Expire();
	virtual void Serialize(ISerialization &archive);
};

class cDelayedHideChar : public cTempEffect
{
public:
	cDelayedHideChar( SERIAL serial );
	virtual void Expire();
	virtual void Serialize(ISerialization &archive);
	virtual QString		objectID() const  { return "HIDECHAR";}
	SERIAL character;
};

class cTimedSpellAction : public cTempEffect
{
private:
	SERIAL character;
	UI08 action;
public:
	// Do that as long as we're casting
	cTimedSpellAction( SERIAL serial, UI08 nAction );
	virtual void Expire();
};

class cPythonEffect : public cTempEffect
{
protected:
	QString functionName;
	PyObject *args;
public:
	void setFunctionName( const QString &data ) { functionName = data; }
	void setArgs( PyObject *data ) { args = data; Py_INCREF( data ); }

	cPythonEffect() { objectid = "PythonEffect"; }
	virtual ~cPythonEffect() {;}
	virtual void Expire();
	virtual void Serialize( ISerialization &archive );
};

class cTmpEffFibHeap
{
public:
	cTmpEffFibHeap()						{ head = NULL; }
	cTmpEffFibHeap( cTempEffect* head_ )	{ head = head_; }
	~cTmpEffFibHeap()	{}

	// methods
	cTempEffect*	accessMin();
	void			deleteMin();
	void			erase( cTempEffect* pT );
	void			insert( cTempEffect* pT );
	cTempEffect*	meld( cTmpEffFibHeap &nFheap );

	std::vector< cTempEffect* >		asVector();

private:
	void			decrease( cTempEffect* pT, int diffTime );

public:
	// variables
	cTempEffect*	head;
};

class cTempEffects
{
private:
	
	struct ComparePredicate : public std::binary_function<cTempEffect*, cTempEffect*, bool>
	{
		bool operator()(const cTempEffect *a, const cTempEffect *b)
		{
			return a->expiretime < b->expiretime;
		}
	};

	static cTempEffects instance;

protected:
	cTempEffects()	{ teffects = cTmpEffFibHeap(); }  // No temp effects to start with

public:
	cTmpEffFibHeap	teffects;

	void check();
	bool add(P_CHAR pc_source, P_CHAR pc_dest, int num, unsigned char more1, unsigned char more2, unsigned char more3, short dur);
	bool add(P_CHAR pc_source, P_ITEM piDest, int num, unsigned char more1, unsigned char more2, unsigned char more3);
	void serialize(ISerialization &archive);
//	bool Exists( P_CHAR pc_source, P_CHAR pc_dest, int num );
	void dispel( P_CHAR pc_dest );

	void insert( cTempEffect* pT )
	{
		teffects.insert( pT );
	}

	int	 size()
	{
		return teffects.asVector().size();
	}

	static cTempEffects *getInstance( void ) { return &instance; }
};

#endif
