//========================================================================
//FILE: netsys.cpp
//========================================================================
//	Copyright (c) 2001 by Sheppard Norfleet and Charles Kerr
//  All Rights Reserved
// 
//	Redistribution and use in source and binary forms, with or without
//	modification, are permitted provided the following conditions are met:
//
//	Redistributions of source code must retain the above copyright notice,
//	this list of conditions and the following disclaimer. Redistributions
//	in binary form must reproduce the above copyright notice, this list of
//	conditions and the following disclaimer in the documentation and/or
//	other materials provided with the distribution.
//
//	Neither the name of the SWORDS  nor the names of its contributors may
//	be used to endorse or promote products derived from this software
//	without specific prior written permission. 
//
//	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//  `AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//	LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
//	A PARTICULAR PURPOSE ARE DISCLAIMED. .
//========================================================================
#include "netsys.h"
//========================================================================
//Begin sock_ntop (sockaddr_in)
string	sock_ntop(sockaddr_in sockaddr) 
{
	return sock_ntop(sockaddr.sin_addr.s_addr) ;
}
//End of sock_ntop
//========================================================================
//Begin sock_ntop
string	sock_ntop(in_addr_t uiAddress) 
{
	string sValue ;

//	char*  str = new char[128];  // Unix domain is the longest
	char  str[128] ;
	size_t cnt = 128 ;

#if defined(IPV6)

	inet_ntop(AF_INET,&uiAddress,str,cnt) ;
	sValue = str ;
#else
	str[0]=0;
	in_addr stAddr ;
	stAddr.s_addr = uiAddress ;
	sValue  = inet_ntoa(stAddr) ;

#endif
	
	return sValue ;	
}
//End of sock_ntop (in_addr_t)
//========================================================================
//Begin sock_errro
SI32 sock_error(SI32 siStatus, SI32 siErrno)
{
	if (siStatus == SOCKET_ERROR)
	{

#if defined(_POSIX)
		siErrno = errno ;
		cout << strerror(errno) << endl ;
#elif defined (WIN32)
		siErrno = WSAGetLastError() ;
		WSASetLastError(0) ;
#endif
	}
	else
		siErrno = 0 ;
	return siStatus ;
}
// end sock_errro
//========================================================================
//Begin sock_pton
UI32 sock_pton(string sIP)
{
	in_addr stAddr ;
	
#if defined(IPV6)
	inet_pton(AF_INET, sIP.c_str(),&stAddr) ;
#else
	stAddr.s_addr = inet_addr(sIP.c_str()) ;
#endif
	return stAddr.s_addr ;
}
//End of sock_pton
//========================================================================
//Begin sock_size
SI32 sock_size(SI32 siSocket)
{
	SI32 siSize = 0 ;
	bool bAction = false ;
	// We dont want to wait at all
	timeval	stTime ;
	stTime.tv_sec = 0 ;
	stTime.tv_usec = 0 ;
	// We want to get the data in the socket.  If the socket has been closed (client side)
	// we want that as well
	if (siSocket != -1)
	{
		//  first, see if anything has happend on the input side of it
		fd_set	fdSockSet ;
		// Zero the descriptor set
		FD_ZERO(&fdSockSet) ;
		FD_SET(siSocket,&fdSockSet) ;
		if( select(siSocket+1,&fdSockSet,0,0,&stTime) > 0)
		{
			// We had activity on our socket (either was written to or closed)
			bAction = true ;
		} 

		ioctlsocket(siSocket,FIONREAD,(unsigned long *)&siSize) ;
		// if size is 0, and we had activity, then it was closed
		if (bAction & (siSize == 0))
			siSize = -1 ;
	}
	return siSize ;
}
//========================================================================
//Begin sock_linger
SI32	sock_linger(SI32 siSocket,bool bLinger)
{
	UI32 uiTemp ;
	if (siSocket != -1)
	{
		if (bLinger)
		{

			ioctlsocket(siSocket,FIONBIO,(unsigned long*)&uiTemp) ;
		}
		else
		{

			ioctlsocket(siSocket,FIONBIO,0) ;
		}
	}
	return 1 ;
}
// end sock_linger
//========================================================================
// sock_flush
SI32	sock_flush(SI32 siSocket)
{
	SI32 siStatus = 0 ;
	/// REMAINS TO BE DONE
	
	return siStatus ;
}
// end sock_flush
//=======================================================================
// getDNS
UI32	getDNS(string sIP)
{
//	we do a dns lookup on this
	UI32 uiValue = 0 ;

    hostent* ptrHost = NULL;
    char **ptrPtr;
#if defined(WIN32)
	uiValue = inet_addr(sIP.c_str()) ;
#else
	uiValue = INADDR_NONE ;
#endif
	if (uiValue == INADDR_NONE)
	{
		ptrHost = gethostbyname2(sIP.c_str(),AF_INET);
 		if(ptrHost != NULL)
		{
	        ptrPtr = ptrHost->h_addr_list;
                //We only use the first one
            if(*ptrPtr != NULL)
            {
            //      I can think of no other way
	            memcpy(&uiValue,*ptrPtr,sizeof(in_addr)) ;
		    }
		}
	}
	return uiValue ;
}
//========================================================================
bool	networkinit()
{
	bool bValue = true;
#if defined(WIN32)
	// Under windows we have to initialize our winsock
	
	WORD wVersionRequested;
	WSADATA wsaData;
	
 
	wVersionRequested = MAKEWORD( 2, 0 );
 


	switch(WSAStartup(wVersionRequested,&wsaData))
	{
	case 0:
		break;
	case WSASYSNOTREADY:
		cerr << "The underlying network subsystem is not ready for network communication." <<endl;
		bValue = false ;
		break;
	case WSAVERNOTSUPPORTED:
		cerr << "The version of Windows Sockets support requested (2.0) is not provided by this particular Windows Sockets implementation." <<endl;
		bValue = false ;
		break;
	}
#endif
printf("Checking unix init\n") ;
#if defined(_POSIX) && !defined(__linux__)
// On BSD machines, a broken pipe generates a SIGPIPE signal.  On 
// linux we can trap that with the MSG_NOSIGNAL. But BSD doesn't have that,
// so we catch the signal.
	signal(SIGPIPE ,&signal_handler) ;
#endif
	return bValue;
}
//========================================================================
//========================================================================
//======================  End of netsys.cpp  =============================
//========================================================================
//========================================================================
