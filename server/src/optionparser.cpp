/*
*     Wolfpack Emu (WP)
* UO Server Emulation Program
*
* Copyright 2001-2017 by holders identified in AUTHORS.txt
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Palace - Suite 330, Boston, MA 02111-1307, USA.
*
* In addition to that license, if you are running this program or modified
* versions of it on a public system you HAVE TO make the complete source of
* the version used by you available or provide people with a location to
* download it.
*
* Wolfpack Homepage: https://github.com/Mutilador/Wolfpack
*/

#include "optionparser.h"

#include <stdio.h>

void OptionParser::addOption( const QString& opt, const QString& dest, const QString& help, bool hasArguments )
{
	Option o;
	o.dest = dest;
	o.help = help;
	o.requireArgument = hasArguments;
	optionSet.insert( opt, o );
}

void OptionParser::addOption( const QString& opt, const QString& dest, PlatformSpecific plat, const QString& help, bool hasArguments )
{
#if defined( Q_OS_WIN )
	if ( plat == OptionParser::Windows )
#elif defined ( Q_OS_UNIX )
	if ( plat == OptionParser::Posix )
#endif
		addOption(opt, dest, help, hasArguments);
}

/*!
	Returns true if all options were recognized
*/
void OptionParser::parse()
{
#if QT_VERSION < 0x0401000
	QStringList args;
	for( int j = 0; j < QCoreApplication::argc(); ++j )
		args.append( QCoreApplication::argv()[ j ] );
#else
	QStringList args = QCoreApplication::arguments();
#endif

	int i = 1;
	while ( i < args.size() )
	{
		// first, check if it's one of the "help" variants
		if ( args[i] == "-h" || args[i] == "--help" )
		{
			printUsage();
		}

		QMap<QString, Option>::iterator it = optionSet.find( args[i] );
		if ( it != optionSet.end() )
		{
			Option o = it.value();
			if ( !o.requireArgument )
				options.insert( args[i], QVariant::fromValue(true) );
			else
			{
				if ( i >= args.size() - 1 ) // we are one arg short
					printUsage();

				QString optionArgument = args[++i];
				// check for "" enclosed arguments
				if ( optionArgument.startsWith("\"") )
				{
					bool found = false;
					while ( i < args.size() && !found )
					{
						QString part = args[++i];
						found = part.contains("\"");
						optionArgument.append( part );
					}
					if ( !found )
						printUsage();
				}
				options.insert( args[i], QVariant::fromValue( optionArgument ) );
			}
		}
		else
			printUsage();
		++i;
	}
}

void OptionParser::printUsage()
{
	QString usage;
	usage.append( "Usage: ");

	fprintf( stderr, qPrintable( usage ) );
	QCoreApplication::exit( 1 );
}
