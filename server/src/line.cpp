/*!
	This code has been taken from NoxWizard's map.cpp/map.h
 */

#include "line.h"
#include "coord.h"

cLine::cLine( const Coord_cl &A, const Coord_cl &B )
{
	m_xDist = (short)( A.x - B.x );
	m_yDist = (short)( A.y - B.y );
	m_zDist = (short)( A.z - B.z );
	x1 = A.x;
	y1 = A.y;
	z1 = A.z;
	map = A.map;
}

char cLine::calcZAtX( unsigned short x )
{
	char z;
	z = (char)( ( (float)( ( (short)(x - x1) * m_zDist ) + (z1 * m_xDist) ) / (float)( m_xDist ) ) + 0.5);
	return z;
}

Coord_cl cLine::getPosAtX( unsigned short x )
{
	Coord_cl pos( x, 0, 0, map );
	pos.y = (unsigned short)( ( (float)( ( (short)(x - x1) * m_yDist ) + ( (short)(y1) * m_xDist) ) / (float)( m_xDist ) ) + 0.5);
	pos.z = calcZAtX( pos.x );
	return pos;
}

Coord_cl cLine::getPosAtY( unsigned short y )
{
	Coord_cl pos( 0, y, 0, map );

	pos.x = (unsigned short)( ( (float)( ( (short)(y - y1) * m_xDist ) + ( (short)(x1) * m_yDist) ) / (float)( m_yDist ) ) + 0.5);
	pos.z = calcZAtX( pos.x );

	return pos;
}
