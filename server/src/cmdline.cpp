/****************************************************************************
** $Id: cmdline.cpp,v 1.1 2002/10/31 15:49:58 codex_ Exp $
**
** Implementation of CmdLine class
**
** Copyright (C) 2002 Bernhard Rosenkraenzer.  All rights reserved.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
**********************************************************************/


#include "cmdline.h"


CmdLine::CmdLine(int argc, char **argv)
{
	appname = argv[0];

	arg = new QStringList();
	register int i;
	for ( i = 1; i < argc; ++i)
		arg->append( argv[i] );

	QStringList argcpy( *arg ); // create a copy we can destroy

	bool restart;
	do 
	{
		restart = false;
		for( QStringList::Iterator it = argcpy.begin(); it != argcpy.end(); ++it)
		{
			QString tmp(*it);
			if( tmp.left(2) == "--" ) 
			{
				QString s = tmp.mid(2);
				if( !s.contains("=") )
					options[s.lower()] = "yes";
				else 
				{
					QString opt = s.left(s.find("=")-1);
					QString val = s.mid(s.find("=")+1);
					options[opt.lower()] = val;
				}
				argcpy.remove(*it);
				restart = true;
				break;
			}
		}
	} while(restart);
}

CmdLine::~CmdLine()
{
	if ( arg )
		delete arg;
}

QString CmdLine::option(QString const &opt, QString const &dflt)
{
	QString lopt = opt.lower();
	if(options[lopt]!="")
		return options[lopt];
	else
		return dflt;
}
