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


#ifndef __IM_H__
#define __IM_H__

#include <string>

using namespace std;

/*
 * im.h
 *   Pedro "Kathrrak" Rabinovitch <miller@tecgraf.puc-rio.br>
 * Item menu header file.
 */

#define IM_MAX_SIZE 40 /* Max number of menu options */

//##ModelId=3C5D92F003AE
typedef struct _item_menu item_menu;

/* void im_callback( item_menu *menu, int player, long item_param )
 *
 * Callback called by im when an item is selected from the
 * menu. States menu, player, and parameter.
 */
//##ModelId=3C5D92F10020
typedef void (*im_callback)( item_menu *menu, int player, long item_param );

/* item_menu structure */
//##ModelId=3C5D92F10098
struct _item_menu {
	//##ModelId=3C5D92F100DF
	std::string title;           /* Menu title */
	//##ModelId=3C5D92F100F3
	im_callback callback;  /* Callback function */

	//##ModelId=3C5D92F101EC
  struct im_item {
		//##ModelId=3C5D92F10200
    int id;              /* id (used for picture) */
		//##ModelId=3C5D92F10214
    long param;          /* parameter (for callback) */
		//##ModelId=3C5D92F10234
	std::string title;         /* option name */
  } im_items[IM_MAX_SIZE];  /* List of menu options */
  
	//##ModelId=3C5D92F10110
  int nitems;

	//##ModelId=3C5D92F10124
  unsigned char *gump_message; /* byte block for message */
	//##ModelId=3C5D92F1014C
  int total_bytes; /* total bytes = prefix + len + text + # items + ... */
                           /* ... + middle + len + text (for each item) */

};

/*
 * item_menu *im_create( char * title )
 *
 *   Initializes and returns a pointer to a new item_menu
 *   with the string title as title. callback will be called
 *   when an item is selected.
 *   Returns NULL if there's any problem, menu otherwise.
 */
item_menu *im_create( char * title, im_callback callback);

/*
 * int im_additem( item_menu *menu, int id, int param, char *title )
 *
 *   Adds a new item to the menu with the specified id, param
 *   and title. param will be passed via callback when an item 
 *   is selected.
 *   Returns 0 if succesful, error code otherwise.
 */
int im_additem( item_menu *menu, int id, long param, char *title );

/*
 * int im_close( item_menu *menu )
 *
 *   Closes menu and generates gump message for later sending.
 *   Returns 0 if succesful, error code otherwise.
 */
int im_close( item_menu *menu );

/*
 * int im_send( item_menu *menu, int player )
 *
 *   Sends menu to the client. Response will come via callback.
 *   Returns 0 if succesful, error code otherwise.
 */
int im_send( item_menu *menu, int player );

/*
 * int im_choice( int player, int menu_code, int option )
 *
 *   Function called by choice to tell which menu option was selected.
 *   Returns 0 if succesful, error code otherwise.
 */
int im_choice( int player, int menu_code, int option );

/*
 * int im_kill( item_menu *menu )
 *
 *   Deletes the memory used by menu.
 *   Returns 0 if succesful, error code otherwise.
 */
int im_kill( item_menu *menu );

/* void im_menucallback( char *menu_name, int player, long item_param )
 *
 * Callback called by im when an item is selected from the
 * menu. States menu, player, and parameter.
 */
//##ModelId=3C5D92F10251
typedef void (*im_menucallback)( char *menu_name, int player, long item_param );

/*
 * int im_loadmenus( char *file_name )
 *
 *   Loads and prepares all the menus found in the gump file file_name.
 *   Returns 0 if succesful, error code otherwise.
 */
int im_loadmenus( const char *file_name, im_menucallback callback );

/*
 * void im_clearmenus()
 *
 *   Frees all internal memory used in menus.
 */
void im_clearmenus();

/*
 * int im_sendmenu( char *menu_name, int player )
 *
 *   Sends the menu referenced by menu_name, previously loaded via loadmenus.
 *   Returns 0 if succesful, error code otherwise.
 */
int im_sendmenu( char *menu_name, int player );


#endif

