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

#include "wolfpack.h"
#include "wip.h"

#ifndef __unix__ // win32 only

#include "assert.h"

// server message thread, replying to clients

// complete dokumentation (and wip-client demo source) following soon.
// don't touch and ask questions about it till everything's finsihed ;)
// 
// LB 14-July-2001

// remark: this thread is completely unsynched !
// that's by design ! it desiged in a way that this is not necassairy
// it runs happily parallel to wp-core taking very little cpu time because 99.999...% suspended.
// remark2: the design allows multiple clients at the time w/o problems :)
DWORD WINAPI WIP_Thread(void *p)
{
  T_struct *par = (T_struct*)(p);
  unsigned long int data=0;
  MSG *msg = new MSG;
  
  clConsole.send("WIP Message Thread started with msg_type: %i \n",par->message_type);

  ////////////////////////////////////////////////////////////////////////
  /// getmessage is synchron !
  /// if no message is pending, this thread gets suspended by OS until one arrives
  /// it returns 0 only if the message queue got destroyed, 
  /// this is only the case when the entire app gets closed, 
  /// so a while(1) { getmessage ... } would be ok as well
  ////////////////////////////////////////////////////////////////////////

  while(GetMessage(msg, NULL, par->message_type, par->message_type)) // fetch ALL messages send to this app
  {	 		 
	// debug msg, to be removed
	clConsole.send("\nMessage #%i erhalten, wparam: %i lparam: %i xw: %i\n",msg->message, msg->wParam, msg->lParam, par->message_type);

	// importnat note: all values you can query need to be global veriables.
	// reading from them is unsyched. doesn't need to be !
	// never ever write to the global vars form wp-core !!!
	// never ever call a method/function from wp-core here. will have real catastrophic results
    // if we wanted to do so (I doubt that its usefull), we'd need to create a request query for wp-core 
	// when core answered them they get send back to clients.
	// but this needs loads of synchronisation and more efficent message passing and what not.
	// I wanted to avoid all this.

    BOOL bad_msg=FALSE;

	switch(msg->lParam)
	{
	case 0:   // get serverstrattime - usefull to get for server up time together with 1

		data = serverstarttime;
		break;

	case 1:   // get current time - important: get it from wolfpack core, not here.
		      // because the wp-main thread might be frozen or crashed while this thread continues happily.
		      // usefull for server-heartbeat detection and the like
		data = uiCurrentTime;

		break;

	case 2: // shutdown server - safely

		data = 1;
		keeprun=0;
		break;

	case 3: // get version
		data = wp_version.vernumber;
		break;

	case 4: // get number of players online
		data = now;
		break;

	case 5: // get wip version
		data = 4; // currently version 4
		break;

	default:
        data=0;
		bad_msg=TRUE;
		break;

	}

    if (bad_msg) msg->lParam=666; // unknown message number  -> return a special return *message* number
	if (msg->message==par->message_type) PostThreadMessage(msg->wParam, par->message_type, msg->lParam, data);		
	// don't reply if other messages arriving. shouldn't happen because getmassage already filters, but to be on the safe side I'll do the check anyway ...

	Sleep(333); // prevents misbehaving clients from dropping server performance by  message bombing
  }
 
  return 0;
}

BOOL WIP_Init(void)
{
	// register our own message type	
 	int WOLF_MESSAGE = RegisterWindowMessage("WM_XWOLFMESSAGE"); 	
	assert(WOLF_MESSAGE>=0XC000);
	if (WOLF_MESSAGE<0xC000) return FALSE;

	T_struct* par = new T_struct;	
	par->message_type=WOLF_MESSAGE;

	// create thread, needs to be done here, becasue shared memory needs the threads id, not the wolfpack main thread one's
	unsigned long int id;
	HANDLE h = CreateThread(NULL, 0, WIP_Thread, par, 0, &id); 
	assert(h!=NULL);
	if (h==NULL) return FALSE;

	/// write pid to shared memory
	HANDLE hmem = CreateFileMapping((HANDLE)0xFFFFFFFF,NULL,PAGE_READWRITE,0, sizeof(struct sm_struct),"X-Wolf");
    struct sm_struct *Msg = (struct sm_struct*)MapViewOfFile(hmem,FILE_MAP_WRITE,0,0,sizeof(struct sm_struct));

	bool bad = false;
    if ( hmem==NULL || Msg == NULL) bad=true; else bad=false;

	if ( IsBadWritePtr ( Msg, sizeof(struct sm_struct) ) )  bad = true;

	if (bad)
	{ 
		clConsole.send("shared memory write failed\n"); 
		return FALSE;

	} 
	else 
	{ 
		Msg->messagethread_pid=id; 
		Msg->wolfcore_pid=GetCurrentThreadId();		
        Msg->wolfcore_prid=GetCurrentProcessId();						
		//clConsole.send("core_pid: %i, main_pid: %i\n", Msg->wolfcore_pid, Msg->messagethread_pid);
	}

	return TRUE;
}


#endif

