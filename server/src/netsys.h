//========================================================================
//FILE: netsys.h
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
//Directives
#if !defined(IN_NETSYS_H)
#define IN_NETSYS_H
//========================================================================
//Platform Specs
#include "platform.h"
//========================================================================
//System Files

#include <iostream>
#include <string>

#if defined(_POSIX)
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <sys/select.h>
// NETWORK DEFINES
#define closesocket(s)  close(s) 
//doesnt have the new gethostbyname2
#define gethostbyname2(host,family) gethostbyname(host)
// But does have the inet_pton and inet_ntop
#define IPV6
// Correspond to Windows error symbol
#define SOCKET_ERROR -1
// ioctl is just ioctl under unix
#define ioctlsocket ioctl
//  in case inet_addr returns an error (compat with windows)
#define INET_ADDR 0

#define WSAECONNRESET EPIPE

#elif defined(WIN32)

#include <winsock2.h>
#include <ws2tcpip.h>
// NETWORK defines
#define SHUT_RDWR SD_BOTH
#define socklen_t  int FAR 
#define in_addr_t  UI32
#define bzero(ptr,n)				memset(ptr,0,n)
#define gethostbyname2(host,family) gethostbyname(host)
#endif
#ifndef MSG_NOSIGNAL
// windows and BSD's dont have this, so set to zero 
#define MSG_NOSIGNAL 0
#endif

#ifndef in_addr_t
#define in_addr_t UI32
#endif


using namespace std;
//========================================================================
// backlog setting
#define NETBACKLOG 512
// Protocol setting
#define IP_PROTOCOL 0 
//=========================================================================
// prototype some usefule network functions we will use
string	sock_ntop(sockaddr_in ) ;
string  sock_ntop(in_addr_t) ;
UI32	sock_pton(string ) ;
SI32	sock_error(SI32 siStatus, SI32 siErrno) ;
SI32	sock_size(SI32 siSocket) ;
SI32	sock_linger(SI32 siSocket,bool bLinger=false) ;
SI32	sock_flush(SI32 siSocket);
UI32	getDNS(string sIP) ;
bool	networkinit() ;
//=========================================================================
//=========================================================================
//=====================  End of netsys.h  =================================
//=========================================================================
//=========================================================================
#endif
