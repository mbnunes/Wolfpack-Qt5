/*
	This code has been readapted from NoxWizard
 */

#if !defined( __LINE_H__ )
#define __LINE_H__

class Coord_cl;

class cLine
{
public:
	cLine( const Coord_cl &A, const Coord_cl &B );
	Coord_cl getPosAtX( unsigned short x );
	Coord_cl getPosAtY( unsigned short y );
	char calcZAtX( unsigned short x );
private:
	unsigned short x1, y1;
	char z1;
	unsigned char map;
	unsigned short m_xDist, m_yDist, m_zDist;
};

#endif
