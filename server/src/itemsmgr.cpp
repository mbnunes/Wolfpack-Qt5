
#include "itemsmgr.h"

#include <algorithm>

template<class _T, class _P>
struct maxKeyPred : binary_function<pair<_T, _P>, pair<_T, _P>, bool>
{
	bool operator()(pair<_T, _P> a, pair<_T, _P> b)
	{
		return a.first < b.first;
	}
};
/*
struct max_serialPred : binary_function<pair<SERIAL, cItem*>, pair<SERIAL, cItem*>, bool>
{
	bool operator()(pair<SERIAL,cItem*> a, pair<SERIAL,cItem*> b)
	{
		return a.first < b.first;
	}
};*/

void cItemsManager::registerItem(cItem* pi) throw(wp_exceptions::bad_ptr)
{
	if ( pi != NULL)
		insert(make_pair(pi->serial, pi));
	else
	{
		throw wp_exceptions::bad_ptr("Invalid argument PI at cItemsManager::registerItem");
	}
}

void cItemsManager::unregisterItem(cItem* pi) throw(wp_exceptions::bad_ptr)
{
	if ( pi != NULL)
		erase(pi->serial);
	else
		throw wp_exceptions::bad_ptr("Invalid argument PI at cItemsManager::unregisterItem");
}

SERIAL cItemsManager::getUnusedSerial() const
{
	typedef maxKeyPred<SERIAL, cItem*> max_serialPred;
	map<SERIAL, cItem*>::const_iterator temp = std::max_element(this->begin(), this->end(), max_serialPred());
	return max(0x40000000, temp->first+1);
}

