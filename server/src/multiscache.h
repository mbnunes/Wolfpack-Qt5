
#if !defined(__MULTISCACHE_H__)
#define __MULTISCACHE_H__

#include <qvaluevector.h>
#include <qmap.h>

struct multiItem_st
{
	Q_INT16 tile;
	Q_INT16 x;
	Q_INT16 y;
	Q_INT8  z;
	bool visible;
};

class MultiDefinition
{
protected:
	QValueVector<multiItem_st> entries; // sorted list of items
	uint width;
	uint height;
public:
	MultiDefinition();

	void setItems( QValueVector<multiItem_st> items );
	bool inMulti( short x, short y );
	signed char multiHeight( short x, short y, short z ) const;
	QValueVector<multiItem_st> getEntries() const;
};

class cMultisCache
{
	QString basePath;
	QMap< ushort, MultiDefinition* > multis;
public:

	cMultisCache( const QString& basePath );

	void load();
	MultiDefinition* getMulti( ushort id );
};


#endif // __MULTISCACHE_H__