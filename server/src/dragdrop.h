//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
//	Copyright 1997, 98 by Marcus Rating (Cironian)
//  Copyright 2001 by holders identified in authors.txt
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
//	* In addition to that license, if you are running this program or modified
//	* versions of it on a public system you HAVE TO make the complete source of
//	* the version used by you available or provide people with a location to
//	* download it.
//
//
//
//	Wolfpack Homepage: http://wpdev.sf.net/
//========================================================================================

#if !defined(__DRAGDROP_H__)
#define __DRAGDROP_H__

class cUOSocket;
class cUORxDropItem;
class cUORxDragItem;
class cUORxWearItem;

class cDragItems
{
public:
	void	grabItem( cUOSocket*, cUORxDragItem* );
	void	equipItem( cUOSocket*, cUORxWearItem* );
	void	dropItem( cUOSocket*, cUORxDropItem* );

	// Sub-drop Functions
	void	dropOnItem( cUOSocket *socket, P_ITEM pItem, P_ITEM pCont, const Coord_cl &dropPos );
	void	dropOnChar( cUOSocket *socket, P_ITEM pItem, P_CHAR pOtherChar );
	void	dropOnGround( cUOSocket *socket, P_ITEM pItem, const Coord_cl &pos );

	// Drop-on-char subfunctions
	void	dropFoodOnChar( cUOSocket* socket, P_ITEM pItem, P_CHAR pChar );
	void	dropOnGuard( cUOSocket* socket, P_ITEM pItem, P_CHAR pGuard );
	void	dropOnBeggar( cUOSocket* socket, P_ITEM pItem, P_CHAR pBeggar );
	void	dropOnBanker( cUOSocket* socket, P_ITEM pItem, P_CHAR pBanker );
	void	dropOnBroker( cUOSocket* socket, P_ITEM pItem, P_CHAR pBroker );
	void	dropOnTrainer( cUOSocket* socket, P_ITEM pItem, P_CHAR pTrainer );

	void	bounceItem( cUOSocket* socket, P_ITEM pi, bool denyMove = false );

	static cDragItems *getInstance( void )
	{
		static cDragItems instance;
		return &instance;
	}
};

void equipItem( P_CHAR wearer, P_ITEM item );

#endif // __DRAGDROP_H__
