
#include "charsmgr.h"

#include <algorithm>
/*
template<class _T, class _P>
class maxKeyPred : public binary_function<pair<_T, _P>, pair<_T, _P>, bool>
{
public:
	bool operator()(pair<_T, _P> a, pair<_T, _P> b)
	{
		return a.first < b.first;
	}
};
*/

struct max_serialPred : binary_function<pair<SERIAL, cChar*>, pair<SERIAL, cChar*>, bool>
{
	bool operator()(pair<SERIAL,cChar*> a, pair<SERIAL,cChar*> b)
	{
		return a.first < b.first;
	}
};

void cCharsManager::registerChar(cChar* pc) throw(wp_exceptions::bad_ptr)
{
	if ( pc != NULL)
		insert(make_pair(pc->serial, pc));
	else
	{
		throw wp_exceptions::bad_ptr("Invalid argument pc at cCharsManager::registerChar");
	}
}

void cCharsManager::unregisterChar(cChar* pc) throw(wp_exceptions::bad_ptr)
{
	if ( pc != NULL)
		erase(pc->serial);
	else
		throw wp_exceptions::bad_ptr("Invalid argument pc at cCharsManager::unregisterChar");
}

SERIAL cCharsManager::getUnusedSerial() const
{
//	typedef maxKeyPred<SERIAL, cChar*> max_serialPred;
	map<SERIAL, cChar*>::const_iterator temp = std::max_element(this->begin(), this->end(), max_serialPred());
	return max(1, temp->first+1); // no serial 0
}

