/*
 *     Wolfpack Emu (WP)
 * UO Server Emulation Program
 *
 * Copyright 2001-2004 by holders identified in AUTHORS.txt
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
 * Wolfpack Homepage: http://wpdev.sf.net/
 */

// Wolfpack includes
#include "getopts.h"

// Library Includes
#include <qstring.h>

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>

void daemonize()
{
	int pid, fd;

	pid = fork();

	switch ( pid )
	{
	case 0:
		// child
		setsid();

		if ( ( fd = open( "/dev/null", O_RDWR ) ) != -1 )
		{
			dup2( fd, 0 );
			dup2( fd, 1 );
			close( fd );
		}

		break;

	case -1:
		perror( "fork" );
		break;

	default:
		// we forked, so silently exit the parent
		exit( 0 );
	}
}

void pidfile_add( QString pidfile )
{
	FILE* pf;

	if ( ( pf = fopen( pidfile, "w+" ) ) != NULL )
	{
		fprintf( pf, "%i", getpid() );
		fclose( pf );
	}
	else
	{
		perror( "fopen" );
	}
}

void pidfile_del( QString pidfile )
{
	if ( unlink( pidfile ) == -1 )
		perror( "unlink" );
}

cGetopts::cGetopts() : isDaemon_( false ), usePidfile_( false )
{
}

cGetopts::~cGetopts()
{
	// cleanup
	if ( usePidfile_ == true )
		pidfile_del( pidfile_ );
}

// get user's options
void cGetopts::parse_options( int argc, char** argv )
{
	unsigned int i;

	for ( i = 1; i < ( uint ) argc; i++ )
	{
		if ( argv[i][0] == '-' )
		{
			switch ( argv[i][1] )
			{
			case 'h':
				fprintf( stderr, "Usage: %s [-d [-p file]]\n", argv[0] );
				fputs( "  -d\trun as daemon.\n", stderr );
				fputs( "  -p\tuse file as PID file.\n", stderr );
				exit( 1 );

			case 'd':
				isDaemon_ = true;
				daemonize();
				break;

			case 'p':
				if ( isDaemon_ == true && usePidfile_ == false )
				{
					usePidfile_ = true;
					pidfile_ = argv[i + 1];
					pidfile_add( pidfile_ );
				}
				break;
			}
		}
	}
}
