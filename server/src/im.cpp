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

/*
 * im.cpp
 *   Pedro "Kathrrak" Rabinovitch <miller@tecgraf.puc-rio.br>
 * Item menu implementation.
 */

#include <ctype.h>

#include "wolfpack.h"
#include "im.h"
#include "debug.h"

#undef  DBGFILE
#define DBGFILE "im.cpp"

#define Free(_) { if( _ != NULL) delete _ ; _ = NULL; }

/* Static internal structure. List of menus waiting for an answer.
   waiting_status is 0 if free, 1 if occupied. firstfree points to
   the first free slot. */
#define WAIT_MAX 256 /* must be a power of two */
static item_menu *waiting_menus[WAIT_MAX];
static char waiting_status[WAIT_MAX];
static int waiting_firstfree = -1;

/*
 * item_menu *im_create( char * title )
 *
 *   Initializes and returns a pointer to a new item_menu
 *   with the string title as title. callback will be called
 *   when an item is selected.
 */
item_menu *im_create( char * title, im_callback callback)
{

	item_menu *new_menu = new item_menu;

	if( waiting_firstfree == -1 )
	{
		/* First time a menu is created. Initialize internals. */
		waiting_firstfree = 0;
		memset( waiting_status, 0, WAIT_MAX );
	}

	if( new_menu == NULL ) /* shouldn't happen. :) */
		return NULL;

	if( title == NULL )    
		new_menu->title.erase();
	else
		new_menu->title = title;

	new_menu->callback = callback;
	new_menu->nitems = 0;
	/* Reserve space for prefix + len + text + # items */
	new_menu->total_bytes = 9 + 1 + new_menu->title.length() + 1;  
	new_menu->gump_message = NULL;

	return new_menu;
}

/*
 * int im_additem( item_menu *menu, int id, int param, char *title )
 *
 *   Adds a new item to the menu with the specified id, param
 *   and title. param will be passed via callback when an item 
 *   is selected.
 *   Returns 0 if succesful, error code otherwise.
 */
int im_additem( item_menu *menu, int id, long param, char *title ) 
{
	if( menu == NULL )
		return -1;

	if( menu->nitems >= IM_MAX_SIZE )
		return -2;

	menu->im_items[menu->nitems].id = id;
	menu->im_items[menu->nitems].param = param;
	if( title )
		menu->im_items[menu->nitems].title = title;
	else
		menu->im_items[menu->nitems].title.erase();

	/* Reserve space for gmmiddle + lenght + title */
	menu->total_bytes += 5 + menu->im_items[menu->nitems].title.length();

	menu->nitems++;

	return 0;
}

/*
 * int im_close( item_menu *menu )
 *
 *   Closes menu and generates gump message for later sending.
 *   Returns 0 if succesful, error code otherwise.
 */
int im_close( item_menu *menu )
{
	int i, curr;

	if( menu == NULL )
		return -1;

	if( menu->nitems == 0 )
		return -3;

	menu->gump_message = new unsigned char[menu->total_bytes];

	/* Gump prefix */
	menu->gump_message[0] = 0x7C;
	/* Total bytes */
	menu->gump_message[1] = (unsigned char)(menu->total_bytes >> 8);
	menu->gump_message[2] = (unsigned char)(menu->total_bytes & 0xFF);
	/* message [3 .. 6] -> character serial (set by im_send) */  
	/* message [7 .. 8] -> special code (set by im_send) */  
	/* Title length */
	menu->gump_message[9] = menu->title.length();
	/* Title */
	strncpy( (char*)menu->gump_message + 10, menu->title.c_str(), menu->gump_message[9] );

	curr = 10 + menu->gump_message[9];

	/* Number of items */
	menu->gump_message[curr++] = menu->nitems;

	for( i=0; i < menu->nitems; i++ )
	{
		int ctl;
		/* item id */
		menu->gump_message[curr++] = (unsigned char)(menu->im_items[i].id >> 8);
		menu->gump_message[curr++] = (unsigned char)(menu->im_items[i].id & 0xFF);
		menu->gump_message[curr++] = 0;
		menu->gump_message[curr++] = 0;
		/* item title length */
		ctl = menu->im_items[i].title.length();
		menu->gump_message[curr++] = ctl;
		/* item title */
		strncpy( (char*)menu->gump_message + curr, menu->im_items[i].title.c_str(), ctl );
		curr += ctl;
	}

	return 0;
}

/*
 * int im_send( item_menu *menu, int player )
 *
 *   Sends menu to the client. Response will come via callback.
 *   Returns 0 if succesful, error code otherwise.
 */
int im_send( item_menu *menu, int player )
{
	if( menu == NULL )
		return -1;

	if( menu->gump_message == NULL )
		return -4;

	/* Player's character's serial number */
	LongToCharPtr(currchar[player]->serial, &menu->gump_message[3]);
	/* Special code for IM_ menu, read by choice() */
	// menu->gump_message[7] = (unsigned char) ~ ( (WAIT_MAX-1) >> 8 ); 
	menu->gump_message[ 7 ] = 0xFF; // by DarkStorm

	/* Index for waiting table (internal) */
	menu->gump_message[8] = (unsigned char)( waiting_firstfree & WAIT_MAX );
	if( waiting_firstfree == WAIT_MAX )
	{
		clConsole.send( "[IM]im_send: Uh-oh. All waiting slots are busy.\n" );
		return -5;
	}	
	/* Add this menu to the wait list. */

	waiting_status[waiting_firstfree] = 1;
	waiting_menus[waiting_firstfree] = menu;
	/* Find next free menu. */
	while( waiting_status[waiting_firstfree] == 1 && waiting_firstfree < WAIT_MAX )
		waiting_firstfree++;

	/* Fire away! */
	Xsend( player, menu->gump_message, menu->total_bytes);

	/* ... and we're done. Now sit and wait for im_choice() to be called. */
	return 0;
}  

/*
 * int im_choice( int player, int menu_code, int option )
 *
 *   Function called by choice to tell which menu option was selected.
 *   Returns 0 if succesful, error code otherwise.
 */
int im_choice( int player, int menu_code, int option )
{
	/* discard leftmost part of code (which are 1's) */
	menu_code &= menu_code & (WAIT_MAX-1);

	if( option == 0 )
	{
		/* User right-clicked, cancelling the menu. Right? */
	
		 currchar[player]->lastTarget = INVALID_SERIAL;
	     return 0; /** Lord binary **/
	}

	if( waiting_status[menu_code] == 0 || waiting_menus[menu_code] == NULL )
		/* No menus here with that code, no sir! */
		return -1;

	/* Update status and firstfree */
	waiting_status[menu_code] = 0;
	if( waiting_firstfree > menu_code )
		waiting_firstfree = menu_code;

	/* Call callback */
	waiting_menus[menu_code]->callback( 
    waiting_menus[menu_code],                       /* Menu */
    player,                                         /* Player */
    waiting_menus[menu_code]->im_items[option-1].param /* parameter */
	);

	/* Ok, now it's his problem. */
	return 0;
}

/*
 * int im_kill( item_menu *menu )
 *
 *   Deletes the memory used by menu.
 *   Returns 0 if succesful, error code otherwise.
 */
int im_kill( item_menu *menu )
{
	if( menu == NULL )
		return -1;
	Free( menu->gump_message );
	Free( menu );

	return 0;
}

/*   ************************   */
/*  Start of .GMP menu section  */
/*   ************************   */

/* menulist_node: node in linked list of loaded menus. */
typedef struct _menulist_node {
  item_menu *menu;
  std::string name;
  im_menucallback callback;
  struct _menulist_node *next;
} menulist_node;

static menulist_node *menu_list;

static char imbuffer[256];
static long buf_number;
static enum {
  TK_NUMBER,
  TK_IDENT,
  TK_STRING,
  TK_SYMB,
  TK_EOF,
  TK_ERROR
} curr_token;

typedef struct _menu_option_info {
  menulist_node *parent;
  char is_final; /* 1 if param is valid, 0 if name. */
  union { 
    char *option_name;
    long param;
  } data;
} menu_option_info;

/** static int gettoken( FILE *fmenu )
 *  Places the next command string in buffer.
 *  Returns EOF if file is at the end, 1 if lexical error or 0 if ok.
 */
static int gettoken( FILE *fmenu )
{
  int c;
  unsigned long loopexit=0,loopexit2=0,loopexit3=0,loopexit4=0,loopexit5=0;

  buf_number = 0;
  c = getc( fmenu );
  
  while( (++loopexit < MAXLOOPS) ) {
    switch( c ) {
      case '/':
        c = getc( fmenu );

        if( c != '/' ) {
          curr_token = TK_ERROR;
          return 1;  /* Lexical error */
        }
                          
		loopexit2=0;
        while( c != '\n' && (++loopexit2 < MAXLOOPS) ) /* skip line */
          c = getc( fmenu );

        c = getc( fmenu );
        break;

      case '\n': case '\t':
      case '\r': case ' ':
        c = getc( fmenu );
        break;

      case EOF:
        curr_token = TK_EOF;
        return EOF;

      case ',': case ';': case '=':
      case '{': case '}':
        imbuffer[0] = c;
        imbuffer[1] = '\0';
        curr_token = TK_SYMB;
        return 0;

      case '"':
        {
          int i = 0;
          
          c = getc( fmenu );
		  loopexit4=0;
          while( c != '"' && (++loopexit4 < MAXLOOPS)) {
            if( c == EOF ) {
              curr_token = TK_ERROR;
              return 1;
            }

            imbuffer[i++] = c;
            c = getc( fmenu );
          }
          imbuffer[i] = '\0';
          curr_token = TK_STRING;
          return 0;
        }
        
      default:
        if( isdigit( c )) { /* Number */
          int inhex = 0;
          int i = 0;

          if( c == '0' ) {
            c = getc( fmenu );
            if( c == 'x' || c == 'X' ) {
              inhex = 1;
              c = getc( fmenu );
            }
          }

          /* if it is a digit or hex is allowed and it is valid... */
		  loopexit3=0;
          while( (isdigit( c ) || 
                 ( inhex && toupper(c) >= 'A' && toupper(c) <= 'F' )) && (++loopexit3 < MAXLOOPS) ) {
            imbuffer[i++] = c;
            c = getc( fmenu );
          }
          imbuffer[i] = '\0';
          
          ungetc( c, fmenu );
          if( inhex )
            sscanf( imbuffer, "%lx", &buf_number );
          else
            sscanf( imbuffer, "%ld", &buf_number );

          imbuffer[0] = '\0';
          curr_token = TK_NUMBER;

        } else if( isalpha( c ) || c == '_' ) { /* Ident */
          int i = 0;

		  loopexit5=0;
          while( (isalnum( c ) || c == '_') && (++loopexit5 < MAXLOOPS) ) {
            imbuffer[i++] = c;
            c = getc( fmenu );
          }
          ungetc( c, fmenu );
          imbuffer[i] = '\0';
          curr_token = TK_IDENT;

        } else
          return 1;

        return 0;
    }
  } 
  return 0;
}

/** static void menus_callback( item_menu *menu, int player, long item_param )
 *  Callback used by the menu system.
 */
static void menus_callback( item_menu *menu, int player, long item_param )
{
  menu_option_info *moi = (menu_option_info *)item_param;

  if( moi->is_final )
    moi->parent->callback( (char*)moi->parent->name.c_str(), player, moi->data.param );
  else
    im_sendmenu( moi->data.option_name, player );
}

/** static int read_menu( FILE *fmenu, im_menucallback callback )
 *  Reads the next menu, setting it with callback.
 *  Returns 0 if succesful, error code otherwise.
 */
static int read_menu( FILE *fmenu, im_menucallback callback )
{
  menulist_node *node;
  int icon,loopexit=0;

  if( curr_token != TK_IDENT )
    return -1;

  node = new menulist_node;
  if( node == NULL ) /* God, I'm boring. :)) */
    return -2;

  node->name = imbuffer;
  node->callback = callback;

  gettoken( fmenu ); /* read '=' */
  if( curr_token != TK_SYMB && imbuffer[0] != '=' )
  {
	  Free(node);
	  return -3;
  }

  gettoken( fmenu ); /* read '{' */
  if( curr_token != TK_SYMB && imbuffer[0] != '{' )
  {
	  Free(node);
      return -4;
  }

  gettoken( fmenu ); /* read title */
  if( curr_token != TK_STRING )
  {
	  Free(node);
      return -5;
  }
  
  node->menu = im_create( imbuffer, menus_callback );
  if( node->menu == NULL )
  {
	  Free(node);
      return -6;
  }

  gettoken( fmenu ); /* read ';' */
  if( curr_token != TK_SYMB && imbuffer[0] != ';' )
  {
	  Free(node->menu);
	  Free(node);
      return -7;
  }

  while( (++loopexit < MAXLOOPS) ) {
    menu_option_info *moi;

    gettoken( fmenu ); /* get icon or '}' */
    if( curr_token == TK_SYMB && imbuffer[0] == '}' )
      break;

    if( curr_token != TK_NUMBER )
	{
	    Free(node->menu);
		Free(node);
        return -8;
	}
    icon = buf_number;

    gettoken( fmenu ); /* get ',' */
    if( curr_token != TK_SYMB && imbuffer[0] != ',' )
	{
        Free(node->menu);
		Free(node);
        return -9;
	}

    gettoken( fmenu ); /* read title */
    if( curr_token != TK_STRING )
	{
	    Free(node->menu);
		Free(node);
        return -10;
	}
    
    moi = new menu_option_info;
    if( moi == NULL ) /* This REALLY shouldn't happen. */
	{
	    Free(node->menu);
		Free(node);
        return -11; 
	}
    
    moi->parent = node;

    if( im_additem( node->menu, icon, (long)moi, imbuffer ))
	{
		Free(node);
		Free(moi);
        return -12;
	}
    
    gettoken( fmenu ); /* get ',' */
    if( curr_token != TK_SYMB && imbuffer[0] != ',' )
	{
		Free(node);
		Free(moi);
        return -13;
	}

    gettoken( fmenu ); /* read ident or number */
    if( curr_token == TK_IDENT ) {
      moi->data.option_name = strdup( imbuffer );
      moi->is_final = 0;
    } else if( curr_token == TK_NUMBER ) {
      moi->data.param = buf_number;
      moi->is_final = 1;
    } else {
		Free(node);
		Free(moi);
		return -14;
	}

    gettoken( fmenu ); /* read ';' */
    if( curr_token != TK_SYMB && imbuffer[0] != ';' )
      return -15;
  }

  if( im_close( node->menu ))
  {
	  Free(node);
	  return -16;
  }

  node->next = menu_list;
  menu_list = node;

  return 0;
}
        

/*
 * int im_loadmenus( char *file_name, im_menucallback callback )
 *
 *   Loads and prepares all the menus found in the gump file file_name.
 *   Returns 0 if succesful, error code otherwise.
 */
int im_loadmenus( const char *file_name, im_menucallback callback )
{
  FILE *fmenu;
  int result,loopexit=0;

  if( file_name == NULL )
    return -1;

  fmenu = fopen( file_name, "rt" );
  if( fmenu == NULL )
    return -2;

  imbuffer[0] = '\0';
  buf_number = 0;

  result = gettoken( fmenu );

  while( result == 0 && (++loopexit < MAXLOOPS) ) {
    result = read_menu( fmenu, callback );
    if( result != 0 )
      return result;
    result = gettoken( fmenu );
  }

  if( result == EOF )
    return 0;
  else
    return result;
}

/*
 * void im_clearmenus()
 *
 *   Frees all internal memory used in menus.
 */
void im_clearmenus()
{
  unsigned long loopexit=0;
  menulist_node *p = menu_list, *next;

  while( p && (++loopexit < MAXLOOPS)) {
    next = p->next;
    im_kill( p->menu );
    Free( p );
    p = next;
  }
}

/*
 * int im_sendmenu( char *menu_name, int player )
 *
 *   Sends the menu referenced by menu_name, previously loaded via loadmenus.
 *   Returns 0 if succesful, error code otherwise.
 */
int im_sendmenu( char *menu_name, int player )
{
  menulist_node *p = menu_list;

  while( p ) {
    if( !strcmp( p->name.c_str(), menu_name )) /* Found it. */
      return im_send( p->menu, player );

    p = p->next;
  }

  return -66;
}


