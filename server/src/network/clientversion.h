/*
 *     Wolfpack Emu (WP)
 * UO Server Emulation Program
 *
 * Copyright 2001-2026 by holders identified in AUTHORS.txt
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef __CLIENTVERSION_H__
#define __CLIENTVERSION_H__

#include <QString>
#include <QtGlobal>

enum ClientType
{
	ClientRegular = 0,
	ClientUOTD,
	ClientGod,
	ClientSA
};

class ClientVersion
{
public:
	quint32 major;
	quint32 minor;
	quint32 revision;
	quint32 patch;
	ClientType type;

	ClientVersion( quint32 maj = 0, quint32 min = 0, quint32 rev = 0, quint32 pat = 0, ClientType t = ClientRegular );
	ClientVersion( const QString& str );

	void fromString( const QString& str );
	QString toString() const;

	bool operator==( const ClientVersion& o ) const;
	bool operator!=( const ClientVersion& o ) const;
	bool operator<( const ClientVersion& o ) const;
	bool operator<=( const ClientVersion& o ) const;
	bool operator>( const ClientVersion& o ) const;
	bool operator>=( const ClientVersion& o ) const;
};

// Protocol feature flags dynamically determined by ClientVersion (derived from ServUO ProtocolChanges)
enum ProtocolChange
{
	ProtocolNone                      = 0,
	ProtocolNewSpellbook              = 0x00000001,
	ProtocolDamagePacket              = 0x00000002,
	ProtocolUnpack                    = 0x00000004,
	ProtocolBuffIcon                  = 0x00000008,
	ProtocolNewHaven                  = 0x00000010,
	ProtocolContainerGridLines        = 0x00000020,
	ProtocolExtendedSupportedFeatures = 0x00000040,
	ProtocolStygianAbyss              = 0x00000080,
	ProtocolHighSeas                  = 0x00000100,
	ProtocolNewCharacterList          = 0x00000200,
	ProtocolNewCharacterCreation      = 0x00000400,
	ProtocolExtendedStatus            = 0x00000800,
	ProtocolNewMobileIncoming         = 0x00001000,
	ProtocolNewSecureTrading          = 0x00002000,
	ProtocolUltimaStore               = 0x00004000,
	ProtocolEndlessJourney            = 0x00008000,

	ProtocolVersion400a   = ProtocolNewSpellbook,
	ProtocolVersion407a   = ProtocolVersion400a | ProtocolDamagePacket,
	ProtocolVersion500a   = ProtocolVersion407a | ProtocolUnpack,
	ProtocolVersion502b   = ProtocolVersion500a | ProtocolBuffIcon,
	ProtocolVersion6000   = ProtocolVersion502b | ProtocolNewHaven,
	ProtocolVersion6017   = ProtocolVersion6000 | ProtocolContainerGridLines,
	ProtocolVersion60142  = ProtocolVersion6017 | ProtocolExtendedSupportedFeatures,
	ProtocolVersion7000   = ProtocolVersion60142 | ProtocolStygianAbyss,
	ProtocolVersion7090   = ProtocolVersion7000 | ProtocolHighSeas,
	ProtocolVersion70130  = ProtocolVersion7090 | ProtocolNewCharacterList,
	ProtocolVersion70160  = ProtocolVersion70130 | ProtocolNewCharacterCreation,
	ProtocolVersion70300  = ProtocolVersion70160 | ProtocolExtendedStatus,
	ProtocolVersion70331  = ProtocolVersion70300 | ProtocolNewMobileIncoming,
	ProtocolVersion704565 = ProtocolVersion70331 | ProtocolNewSecureTrading,
	ProtocolVersion70500  = ProtocolVersion704565 | ProtocolUltimaStore,
	ProtocolVersion70610  = ProtocolVersion70500 | ProtocolEndlessJourney
};

// Common version milestones
extern const ClientVersion Version400a;
extern const ClientVersion Version407a;
extern const ClientVersion Version500a;
extern const ClientVersion Version502b;
extern const ClientVersion Version6000;
extern const ClientVersion Version6017;
extern const ClientVersion Version60142;
extern const ClientVersion Version7000;
extern const ClientVersion Version7090;
extern const ClientVersion Version70130;
extern const ClientVersion Version70160;
extern const ClientVersion Version70300;
extern const ClientVersion Version70331;
extern const ClientVersion Version704565;
extern const ClientVersion Version70500;
extern const ClientVersion Version70610;

quint32 protocolChangesForVersion( const ClientVersion& v );

#endif // __CLIENTVERSION_H__
