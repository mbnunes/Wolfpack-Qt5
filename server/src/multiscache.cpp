

#include "multiscache.h"
#include "defines.h"
#include "classes.h" // only because of illegal_z

// Library Includes
#include <qvaluevector.h>
#include <qfile.h>
#include <qdatastream.h>
#include <qdir.h>

#include <algorithm>

MultiDefinition::MultiDefinition() : width(0), height(0)
{
}

void MultiDefinition::setItems( QValueVector<multiItem_st> items )
{
	// try to sort
	if ( items.empty() )
		return;
	unsigned int i = 0;
	int max_X = 0, max_Y = 0, min_X = 0, min_Y = 0;
	for (; i < items.size(); ++i)
	{
		if ( items[max_X].x < items[i].x )
			max_X = i;
		if ( items[max_Y].y < items[i].y )
			max_Y = i;
		if ( items[min_X].x > items[i].x )
			min_X = i;
		if ( items[min_Y].y > items[i].y )
			min_Y = i;
	}

	// by now we have the dimensions.
	this->width  = items[max_X].x + abs(items[min_X].x);
	this->height = items[max_Y].y + abs(items[min_Y].y);
	entries = items;
}

/*!
	Checks if the given pair of \a x and \a y coordinates
	are inside this Multi. The coordinates should be relative to
	the multi's center (0,0)
*/
bool MultiDefinition::inMulti( short x, short y )
{
	if ( abs(x) > this->width || abs(y) > this->height )
	for ( unsigned int i = 0; i < entries.size(); ++i )
		if ( entries[i].x == x && entries[i].y == y )
			return true;
	return false;
}

signed char MultiDefinition::multiHeight( short x, short y, short z ) const
{
	for ( int j = 0; j < entries.size(); ++j )
	{
		if ( entries[j].visible && ( x + entries[j].x == x ) && ( y + entries[j].y == y ) )
		{
			const int tmpTop = z + entries[j].z;
			if ( tmpTop <= z + MaxZstep && tmpTop >= z-1 )
			{
				return entries[j].z;
			}
			else if ( tmpTop >= z - MaxZstep && tmpTop < z - 1 )
			{
				return entries[j].z;
			}
		}                                                                                                                 
	}
	return illegal_z;
}

/*!
	Returns this Multi's entries as found in the multis.mul file
*/
QValueVector<multiItem_st> MultiDefinition::getEntries() const
{
	return entries;
}

/*****************************************************************************
  MultisCache member functions
 *****************************************************************************/

/*!
	\class cMultisCache multiscache.h

	\brief cMultisCache is responsable for loading and parsing multi definitions
	stored into "multi.idx" and "multi.mul" files.

	This class is prepared for BigEndian/LittleEndian portability.

	\ingroup UO File Handlers
	\ingroup mainclass
*/


/*!
	Constructs a cMultisCache object
*/
cMultisCache::cMultisCache( const QString& path ) : basePath(path)
{
}

/*!
	Parses and loads multi definitions
*/
void cMultisCache::load()
{
	QFile indexFile( basePath + QDir::separator() + "multi.idx" );
	indexFile.open( IO_ReadOnly );
	QDataStream indexStream( &indexFile );
	indexStream.setByteOrder( QDataStream::LittleEndian );

	QFile multiFile( basePath + QDir::separator() + "multi.mul" );
	multiFile.open( IO_ReadOnly );

	struct
	{
		Q_INT32 start;
		Q_INT32 length;
		Q_INT32 unknown;
	} indexData;
	ushort currentID = 0;
	while ( !indexStream.atEnd() )
	{
		indexFile.at( currentID * 12 );
		indexStream >> indexData.start;
		indexStream >> indexData.length;
		indexStream >> indexData.unknown;

		if ( indexData.start == -1 ) // empty record?
		{
			++currentID;
			continue;
		}

		QValueVector<multiItem_st> items;
		items.reserve( indexData.length / 12 );
		uint i = 0;

		multiFile.at( indexData.start );
		QDataStream multiStream( &multiFile );
		multiStream.setByteOrder( QDataStream::LittleEndian );

		for (; i < indexData.length / 12; ++i )
		{
			multiItem_st item;
			multiStream >> item.tile;
			multiStream >> item.x;
			multiStream >> item.y;
			multiStream >> item.z;
			Q_UINT8  empty;
			multiStream >> empty; // ????
			Q_UINT32 isVisible = 0;
			multiStream >> isVisible;
			item.visible = isVisible > 0 ? true : false;

			if ( item.visible ) // we ignore invisible items (?)
				items.push_back(item);
		}
		MultiDefinition* multi = new MultiDefinition;
		multi->setItems( items );
		
		multis.insert( currentID++, multi );
	}
}

/*!
	Retrieves the multi's definition of a given ID.
*/
MultiDefinition* cMultisCache::getMulti( ushort id )
{
	QMap<ushort, MultiDefinition*>::const_iterator it = multis.find(id);
	if ( it != multis.end() )
		return it.data();
	else
		return 0;
}

