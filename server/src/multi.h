
#if !defined(__MULTI_H__)
#define __MULTI_H__

#include "uobject.h"
#include "items.h"
#include <qstring.h>
#include <qptrlist.h>

class cMulti : public cItem {
protected:
	// Objects in the Multi.
	// Items that are in the Multi
	// are removed along with it.
	QPtrList<cUObject> objects;

public:
	const char *objectID() const {
        return "cMulti";
	}

	cMulti();
	virtual ~cMulti();

	/*static void buildSqlString(QStringList &fields, QStringList &tables, QStringList &conditions);
	void load( char **, UINT16& );
	void save();
	bool del();*/

	// Find at certain position
	static cMulti *find(const Coord_cl &pos);

	// Register in load factory
	static void registerInFactory();

	// Property Interface Methods
	stError *setProperty( const QString &name, const cVariant &value );
	stError *getProperty( const QString &name, cVariant &value ) const;

	// Python Interface Methods
	PyObject *getPyObject();
	const char *className() const;

	// Object List
	void addObject(cUObject *object);
	void removeObject(cUObject *object);
	inline const QPtrList<cUObject> &getObjects() {
		return objects;
	}

	// Remove Handler
	void remove();

	// Move all contained items along.
	// Also recheck our position in the multi grid.
	void moveTo(const Coord_cl&, bool noRemove = false);

	// Create a multi from a script definition
	static cMulti *createFromScript(const QString &section);

	// See if a certain coordinate is within the multi
	bool inMulti(const Coord_cl &pos);
};

#endif
