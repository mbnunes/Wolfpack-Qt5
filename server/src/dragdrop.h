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

#if !defined(__DRAGDROP_H__)
#define __DRAGDROP_H__

#include "typedefs.h"

class cUOSocket;
class cUORxDropItem;
class cUORxDragItem;
class cUORxWearItem;

namespace DragAndDrop
{
	void grabItem( cUOSocket*, cUORxDragItem* );
	void equipItem( cUOSocket*, cUORxWearItem* );
	void dropItem( cUOSocket*, cUORxDropItem* );

	// Sub-drop Functions
	void dropOnItem( cUOSocket *socket, P_ITEM pItem, P_ITEM pCont, const Coord_cl &dropPos );
	void dropOnChar( cUOSocket *socket, P_ITEM pItem, P_CHAR pOtherChar );
	void dropOnGround( cUOSocket *socket, P_ITEM pItem, const Coord_cl &pos );

	// Drop-on-char subfunctions
	void dropOnBeggar( cUOSocket* socket, P_ITEM pItem, P_CHAR pBeggar );
	void dropOnBanker( cUOSocket* socket, P_ITEM pItem, P_CHAR pBanker );
	void dropOnBroker( cUOSocket* socket, P_ITEM pItem, P_CHAR pBroker );
	void dropOnTrainer( cUOSocket* socket, P_ITEM pItem, P_CHAR pTrainer );

	void bounceItem( cUOSocket* socket, P_ITEM pi, bool denyMove = false );
};

void equipItem( P_CHAR wearer, P_ITEM item );

#endif // __DRAGDROP_H__
