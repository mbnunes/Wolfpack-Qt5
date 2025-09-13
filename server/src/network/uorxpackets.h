/*
 *     Wolfpack Emu (WP)
 * UO Server Emulation Program
 *
 * Copyright 2001-2017 by holders identified in AUTHORS.txt
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
 * Wolfpack Homepage: https://github.com/Mutilador/Wolfpack
 */

#if !defined(__UO_RXPACKETS__)
#define __UO_RXPACKETS__

// Wolfpack Includes
#include "../typedefs.h"
#include "../gumps.h"
#include "uopacket.h"
#include "../defines.h"

// Qt Includes
#include <QByteArray>
#include <QString>
#include <QStringList>

cUOPacket* getUORxPacket( const QByteArray& data );

// 0x12: Action
class cUORxAction : public cUOPacket
{
public:
	cUORxAction( const QByteArray& data ) : cUOPacket( data )
	{
	}
	uchar type() const
	{
		return ( *this )[3];
	}
	QString action() const
	{
		return this->getAsciiString( 4, getShort( 1 ) - 4 );
	}
};

// 0x34: Query
class cUORxQuery : public cUOPacket
{
public:
	enum eType
	{
		Stats		= 0x04,
		Skills
	};

	cUORxQuery( const QByteArray& data ) : cUOPacket( data )
	{
	}

	uint pattern( void ) const
	{
		return getInt( 1 );
	}
	eType type( void ) const
	{
		return ( ( *this )[5] == 0x05 ) ? Skills : Stats;
	}
	uint serial( void ) const
	{
		return getInt( 6 );
	}
};

// 0x00: Create Char
class cUORxCreateChar : public cUOPacket
{
public:
	cUORxCreateChar() : cUOPacket( 0x00, 104 )
	{
	}
	cUORxCreateChar( const QByteArray& data ) : cUOPacket( data )
	{
	}
	uint pattern1( void ) const
	{
		return getInt( 1 );
	}
	uint pattern2( void ) const
	{
		return getInt( 5 );
	}
	uchar pattern3( void ) const
	{
		return ( *this )[9];
	}
	QByteArray name( void ) const
	{
		return this->getAsciiString( 10, 30 );
	}
	// Here are 4 unkown bytes
	ushort flags( void ) const
	{
		return getShort( 44 );
	}
	// Here are 24 unkown bytes
	uchar gender( void ) const
	{
		return ( *this )[70];
	} // 0 = male, 1 = female, 2 = Elf Male, 3 = Elf Female
	uchar strength( void ) const
	{
		return ( *this )[71];
	}
	uchar dexterity( void )	const
	{
		return ( *this )[72];
	}
	uchar intelligence( void ) const
	{
		return ( *this )[73];
	}
	uchar skillId1( void ) const
	{
		return ( *this )[74];
	}
	uchar skillValue1( void ) const
	{
		return ( *this )[75];
	}
	uchar skillId2( void ) const
	{
		return ( *this )[76];
	}
	uchar skillValue2( void ) const
	{
		return ( *this )[77];
	}
	uchar skillId3( void ) const
	{
		return ( *this )[78];
	}
	uchar skillValue3( void ) const
	{
		return ( *this )[79];
	}
	short skinColor( void ) const
	{
		return getShort( 80 );
	}
	short hairStyle( void ) const
	{
		return getShort( 82 );
	}
	short hairColor( void ) const
	{
		return getShort( 84 );
	}
	short beardStyle( void ) const
	{
		return getShort( 86 );
	}
	short beardColor( void ) const
	{
		return getShort( 88 );
	}
	// Here is an unkown byte (!)
	uchar startTown( void ) const
	{
		return ( *this )[91];
	}
	ushort unknown1( void ) const
	{
		return getShort( 92 );
	}
	ushort slot( void ) const
	{
		return getShort( 94 );
	}
	uint ip( void ) const
	{
		return getInt( 96 );
	}
	short shirtColor( void ) const
	{
		return getShort( 100 );
	}
	short pantsColor( void ) const
	{
		return getShort( 102 );
	}

	void setPattern1( uint d )
	{
		setInt( 1, d );
	}
	void setPattern2( uint d )
	{
		setInt( 5, d );
	}
	void setPattern3( uchar d )
	{
		( *this )[9] = d;
	}
	void setName( const QByteArray& d )
	{
		this->setAsciiString( 10, d.data(), 30 );
	}
	// Here are 4 unkown bytes
	void setFlags( ushort d )
	{
		setShort( 44, d );
	}
	// Here are 24 unkown bytes
	void setGender( uchar d )
	{
		( *this )[70] = d;
	}
	void setStrength( uchar d )
	{
		( *this )[71] = d;
	}
	void setDexterity( uchar d )
	{
		( *this )[72] = d;
	}
	void setIntelligence( uchar d )
	{
		( *this )[73] = d;
	}
	void setSkillId1( uchar d )
	{
		( *this )[74] = d;
	}
	void setSkillValue1( uchar d )
	{
		( *this )[75] = d;
	}
	void setSkillId2( uchar d )
	{
		( *this )[76] = d;
	}
	void setSkillValue2( uchar d )
	{
		( *this )[77] = d;
	}
	void setSkillId3( uchar d )
	{
		( *this )[78] = d;
	}
	void setSkillValue3( uchar d )
	{
		( *this )[79] = d;
	}
	void setSkinColor( ushort d )
	{
		setShort( 80, d );
	}
	void sethairStyle( ushort d )
	{
		setShort( 82, d );
	}
	void setHairColor( ushort d )
	{
		setShort( 84, d );
	}
	void setBeardStyle( ushort d )
	{
		setShort( 86, d );
	}
	void setBeardColor( ushort d )
	{
		setShort( 88, d );
	}
	// Here is an unkown byte (!)
	void setStartTown( uchar d )
	{
		( *this )[91] = d;
	}
	void setUnknown1( ushort d )
	{
		setShort( 92, d );
	}
	void slot( ushort d )
	{
		setShort( 94, d );
	}
	void setIP( uint d )
	{
		setInt( 96, d );
	}
	void setShirtColor( ushort d )
	{
		setShort( 100, d );
	}
	void setPantsColor( ushort d )
	{
		setShort( 102, d );
	}
};

// 0xF8: Create Char New
class cUORxCreateCharNew : public cUOPacket
{
public:
	cUORxCreateCharNew() : cUOPacket(0xF8, 106)
	{
	}
	cUORxCreateCharNew(const QByteArray& data) : cUOPacket(data)
	{
	}

	uint pattern1(void) const
	{
		return getInt(1);
	}
	uint pattern2(void) const
	{
		return getInt(5);
	}
	uchar pattern3(void) const
	{
		return (*this)[9];
	}
	QByteArray name(void) const
	{
		return this->getAsciiString(10, 30);
	}
	// Here are 2 unkown bytes
	ushort Unknown2(void) const
	{
		return getShort(40);
	}
	uint flags(void) const
	{
		return getInt(42);
	}
	// Here are 24 unkown bytes
	uint Unknown3(void) const
	{
		return getInt(46);
	}

	uint ClientLoginCount(void) const
	{
		return getInt(50);
	}

	uchar Profession(void) const
	{
		return (*this)[54];
	}

	uchar genderRace(void) const
	{
		return (*this)[70];
	} // 0 = Human male, 1 = Human female, 2 = Human Male, 3 = Human Female, 4 = Elf Male, 5 = Elf Female, 6 = Gargoyle Male, 7 = Gargoyle Female

	uchar strength(void) const
	{
		return (*this)[71];
	}
	uchar dexterity(void)	const
	{
		return (*this)[72];
	}
	uchar intelligence(void) const
	{
		return (*this)[73];
	}
	uchar skillId1(void) const
	{
		return (*this)[74];
	}
	uchar skillValue1(void) const
	{
		return (*this)[75];
	}
	uchar skillId2(void) const
	{
		return (*this)[76];
	}
	uchar skillValue2(void) const
	{
		return (*this)[77];
	}
	uchar skillId3(void) const
	{
		return (*this)[78];
	}
	uchar skillValue3(void) const
	{
		return (*this)[79];
	}
	uchar skillId4(void) const
	{
		return (*this)[80];
	}
	uchar skillValue4(void) const
	{
		return (*this)[81];
	}
	ushort skinColor(void) const
	{
		return getShort(82);
	}
	ushort hairStyle(void) const
	{
		return getShort(84);
	}
	ushort hairColor(void) const
	{
		return getShort(86);
	}

	ushort beardStyle(void) const
	{
		return getShort(88);
	}
	ushort beardColor(void) const
	{
		return getShort(90);
	}
	uchar shardIndex(void) const
	{
		return (*this)[92];
	}
	// Here is an unkown byte (!)
	uchar startTown(void) const
	{
		return (*this)[93];
	}
	uint slot(void) const
	{
		return getInt(94);
	}
	uint ip(void) const
	{
		return getInt(98);
	}
	ushort shirtColor(void) const
	{
		return getShort(102);
	}
	ushort pantsColor(void) const
	{
		return getShort(104);
	}
	void setPattern1(uint d)
	{
		setInt(1, d);
	}
	void setPattern2(uint d)
	{
		setInt(5, d);
	}
	void setPattern3(uchar d)
	{
		(*this)[9] = d;
	}
	void setName(const QByteArray& d)
	{
		this->setAsciiString(10, d.data(), 30);
	}
	void setUnknown2(ushort d)
	{
		setShort(40, d);
	}
	// Here are 4 unkown bytes
	void setFlags(uint d)
	{
		setInt(42, d);
	}
	void setUnknown3(uint d)
	{
		setInt(46, d);
	}
	void setClientLoginCount(uint d)
	{
		setInt(50, d);
	}

	void setProfession(uchar d)
	{
		(*this)[54] = d;
	}

	// Here are 24 unkown bytes
	void setGenderRace(uchar d)
	{
		(*this)[70] = d;
	}
	void setStrength(uchar d)
	{
		(*this)[71] = d;
	}
	void setDexterity(uchar d)
	{
		(*this)[72] = d;
	}
	void setIntelligence(uchar d)
	{
		(*this)[73] = d;
	}
	void setSkillId1(uchar d)
	{
		(*this)[74] = d;
	}
	void setSkillValue1(uchar d)
	{
		(*this)[75] = d;
	}
	void setSkillId2(uchar d)
	{
		(*this)[76] = d;
	}
	void setSkillValue2(uchar d)
	{
		(*this)[77] = d;
	}
	void setSkillId3(uchar d)
	{
		(*this)[78] = d;
	}
	void setSkillValue3(uchar d)
	{
		(*this)[79] = d;
	}
	void setSkillId4(uchar d)
	{
		(*this)[80] = d;
	}
	void setSkillValue4(uchar d)
	{
		(*this)[81] = d;
	}
	void setSkinColor(ushort d)
	{
		setShort(82, d);

	}
	void sethairStyle(ushort d)
	{
		setShort(84, d);
	}
	void setHairColor(ushort d)
	{
		setShort(86, d);
	}
	void setBeardStyle(ushort d)
	{
		setShort(88, d);
	}
	void setBeardColor(ushort d)
	{
		setShort(90, d);
	}
	// Here is an unkown byte (!)
	void setUnknown1(uchar d)
	{
		(*this)[92] = d;
	}

	void setStartTown(uchar d)
	{
		(*this)[93] = d;
	}

	void slot(uint d)
	{
		setInt(94, d);
	}
	void setIP(uint d)
	{
		setInt(98, d);
	}
	void setShirtColor(ushort d)
	{
		setShort(102, d);
	}
	void setPantsColor(ushort d)
	{
		setShort(104, d);
	}
};

// 0x8D Create Char New
class cUORxCreateCharacter3D : public cUOPacket
{
public:
    cUORxCreateCharacter3D() : cUOPacket(0x8D, 146)
    {
    }
    cUORxCreateCharacter3D(const QByteArray& data) : cUOPacket(data)
    {
    }

    ushort packetSize(void) const
    {
        return getShort(1);
    }
    uint pattern1(void) const
    {
        return getInt(3);
    }
    uchar charSlot(void) const
    {
        return getInt(7);
    }
    QByteArray name(void) const
    {
        return this->getAsciiString(11, 30);
    }
    // Here are 2 unkown bytes
    QByteArray Unknown1(void) const
    {
        return this->getAsciiString(41, 30);
    }
    uchar profession(void) const
    {
        return (*this)[71];
    }
    uchar startTown(void) const
    {
        return (*this)[72];
    }
    uchar genderId(void) const
    {
        return (*this)[73];
    } // 0 = Human male, 1 = Human female, 2 = Human Male, 3 = Human Female, 4 = Elf Male, 5 = Elf Female, 6 = Gargoyle Male, 7 = Gargoyle Female

    uchar raceId(void) const
    {
        return (*this)[74];
    }

    uchar strength(void) const
    {
        return (*this)[75];
    }
    uchar dexterity(void)	const
    {
        return (*this)[76];
    }
    uchar intelligence(void) const
    {
        return (*this)[77];
    }

    ushort skinColor(void) const
    {
        return getShort(78);
    }

    uint unknown8(void) const
    {
        return getInt(80);
    }

    uint unknown9(void) const
    {
        return getInt(84);
    }

    uchar skillId1(void) const
    {
        return (*this)[88];
    }
    uchar skillValue1(void) const
    {
        return (*this)[89];
    }
    uchar skillId2(void) const
    {
        return (*this)[90];
    }
    uchar skillValue2(void) const
    {
        return (*this)[91];
    }
    uchar skillId3(void) const
    {
        return (*this)[92];
    }
    uchar skillValue3(void) const
    {
        return (*this)[93];
    }
    uchar skillId4(void) const
    {
        return (*this)[94];
    }
    uchar skillValue4(void) const
    {
        return (*this)[95];
    }

    // Here are 2 unkown bytes
    QByteArray Unknown2(void) const
    {
        return this->getAsciiString(96, 25);
    }

    uchar Unknown3(void) const
    {
        return (*this)[121];
    }

    ushort hairColor(void) const
    {
        return getShort(122);
    }

    ushort hairStyle(void) const
    {
        return getShort(124);
    }

    uchar Unknown4(void) const
    {
        return (*this)[126];
    }

    uint Unknown5(void) const
    {
        return getInt(127);
    }

    uchar Unknown6(void) const
    {
        return (*this)[131];
    }

    ushort shirtColor(void) const
    {
        return getShort(132);
    }
    ushort pantsColor(void) const
    {
        return getShort(134);
    }

    uchar Unknown7(void) const
    {
        return (*this)[136];
    }

    ushort faceColor(void) const
    {
        return getShort(137);
    }

    ushort faceStyle(void) const
    {
        return getShort(139);
    }

    uchar Unknown8(void) const
    {
        return (*this)[141];
    }

    ushort beardColor(void) const
    {
        return getShort(142);
    }

    ushort beardStyle(void) const
    {
        return getShort(144);
    }

    void setPacketSize(uint d)
    {
        setShort(1, d);
    }

    void setPattern1(uint d)
    {
        setInt(3, d);
    }

    void setCharSlot(uchar d)
    {
        setShort(7, d);
    }
    void setName(const QByteArray& d)
    {
        this->setAsciiString(11, d.data(), 30);
    }

    void setUnknown1(const QByteArray& d)
    {
        this->setAsciiString(41, d.data(), 30);
    }

    // Here are 4 unkown bytes
    void setProfession(uint d)
    {
        (*this)[71] = d;
    }

    void setStartTown(uint d)
    {
        (*this)[72] = d;
    }

    void setGenderId(uint d)
    {
        (*this)[73] = d;
    }

    void setRaceId(uint d)
    {
        (*this)[74] = d;
    }

    void setStrength(uchar d)
    {
        (*this)[75] = d;
    }
    void setDexterity(uchar d)
    {
        (*this)[76] = d;
    }
    void setIntelligence(uchar d)
    {
        (*this)[77] = d;
    }
    void setSkinColor(ushort d)
    {
        setShort(78, d);
    }

    void setUnknown8(uint d)
    {
        setInt(80, d);
    }
    void setUnknown9(uint d)
    {
        setInt(84, d);
    }


    void setSkillId1(uchar d)
    {
        (*this)[88] = d;
    }
    void setSkillValue1(uchar d)
    {
        (*this)[89] = d;
    }
    void setSkillId2(uchar d)
    {
        (*this)[90] = d;
    }
    void setSkillValue2(uchar d)
    {
        (*this)[91] = d;
    }
    void setSkillId3(uchar d)
    {
        (*this)[92] = d;
    }
    void setSkillValue3(uchar d)
    {
        (*this)[93] = d;
    }
    void setSkillId4(uchar d)
    {
        (*this)[94] = d;
    }
    void setSkillValue4(uchar d)
    {
        (*this)[95] = d;
    }
    void setUnknown2(const QByteArray& d)
    {
        this->setAsciiString(96, d.data(), 25);
    }
    void setUnknown3(uchar d)
    {
        (*this)[121] = d;
    }
    void setHairColor(ushort d)
    {
        setShort(122, d);
    }
    void sethairStyle(ushort d)
    {
        setShort(124, d);
    }
    void setUnknown4(uchar d)
    {
        (*this)[126] = d;
    }
    void setUnknown5(uchar d)
    {
        setShort(127, d);
    }
    void setUnknown6(uchar d)
    {
        (*this)[131] = d;
    }
    void setShirtColor(ushort d)
    {
        setShort(132, d);
    }
    void setPantsColor(ushort d)
    {
        setShort(134, d);
    }
    void setUnknown7(uchar d)
    {
        (*this)[136] = d;
    }
    void setFaceColor(ushort d)
    {
        setShort(137, d);
    }
    void setFaceStyle(ushort d)
    {
        setShort(139, d);
    }
    void setUnknown8(uchar d)
    {
        (*this)[141] = d;
    }
    void setBeardStyle(ushort d)
    {
        setShort(142, d);
    }
    void setBeardColor(ushort d)
    {
        setShort(144, d);
    }

};

// 0xC8: UpdateRange
class cUORxUpdateRange : public cUOPacket
{
public:
	cUORxUpdateRange( const QByteArray& data ) : cUOPacket( data )
	{
	}
	uchar range( void ) const
	{
		return ( *this )[1];
	}
};

// 0x01: NotifyDisconnect
class cUORxNotifyDisconnect : public cUOPacket
{
public:
	cUORxNotifyDisconnect( const QByteArray& data ) : cUOPacket( data )
	{
	}
	uint pattern( void ) const
	{
		return getInt( 1 );
	}
};

// 0x80: Login Request
class cUORxLoginRequest : public cUOPacket
{
public:
	cUORxLoginRequest() : cUOPacket( 0x80, 62 )
	{
	}

	cUORxLoginRequest( const QByteArray& data ) : cUOPacket( data )
	{
	}

	QByteArray username( void ) const
	{
		return this->getAsciiString( 1, 30 );
	}

	QByteArray password( void ) const
	{
		return this->getAsciiString( 31, 30 );
	}

	void setUsername( const QString& n )
	{
		this->setAsciiString( 1, qPrintable(n), 30 );
	}

	void setPassword( const QString& n )
	{
		this->setAsciiString( 31, qPrintable(n), 30 );
	}
};

// 0xA4: System Info
class cUORxSystemInfo : public cUOPacket
{
public:
	cUORxSystemInfo( const QByteArray& data ) : cUOPacket( data )
	{
	}
	uchar processorType( void ) const
	{
		return ( *this )[0x01];
	}
	ushort processorSpeed( void ) const
	{
		return getShort( 0x02 );
	}
	uchar processorCount( void ) const
	{
		return ( *this )[0x04];
	}
	// QString directory -- unicode ? -- > 20 bytes useless trash
	// Video Card descritor: -- unicode ? -- > 20 bytes useless trash
	// ??? > another 20 byte trash field
	// ??? > ANOTHER 20 byte trash field
	ushort memoryInMb( void ) const
	{
		return getShort( 0x85 );
	}
	ushort largestPartitionInMb( void ) const
	{
		return getShort( 0x87 );
	}
	int timezoneBias( void ) const
	{
		return getInt( 0x8C );
	}
};

// 0xA0: Select Shard
class cUORxSelectShard : public cUOPacket
{
public:
	cUORxSelectShard( const QByteArray& data ) : cUOPacket( data )
	{
	}
	ushort shardId( void ) const
	{
		return getShort( 1 );
	}
};

// 0x91: Charlist Request
class cUORxServerAttach : public cUOPacket
{
public:
	cUORxServerAttach() : cUOPacket( 0x91, 65 )
	{
	}
	cUORxServerAttach( const QByteArray& data ) : cUOPacket( data )
	{
	}
	uint authId( void )		  const
	{
		return getInt( 1 );
	}
	QByteArray username( void ) const
	{
		return this->getAsciiString( 5, 30 );
	}
	QByteArray password( void ) const
	{
		return this->getAsciiString( 35, 30 );
	}

	void setAuthId( uint d )
	{
		setInt( 1, d );
	}
	void setUsername( const QByteArray& d )
	{
		this->setAsciiString( 5, d.data(), 30 );
	}
	void setPassword( const QByteArray& d )
	{
		this->setAsciiString( 35, d.data(), 30 );
	}
};

// 0x73 Ping
class cUORxPing : public cUOPacket
{
public:
	cUORxPing( const QByteArray& data ) : cUOPacket( data )
	{
	}
};

// 0x83 Delete Character
class cUORxDeleteCharacter : public cUOPacket
{
public:
	cUORxDeleteCharacter( const QByteArray& data ) : cUOPacket( data )
	{
	}

	QString password( void ) const
	{
		return this->getAsciiString( 1, 30 );
	}
	int index( void ) const
	{
		return getInt( 31 );
	}
	int ip( void ) const
	{
		return getInt( 35 );
	}
};

// 0x5D Play Character
class cUORxPlayCharacter : public cUOPacket
{
public:
	cUORxPlayCharacter() : cUOPacket( 0x5D, 73 )
	{
	}
	cUORxPlayCharacter( const QByteArray& data ) : cUOPacket( data )
	{
	}

	QString character( void ) const
	{
		return this->getAsciiString( 5, 30 );
	}

	// the client now uses a certain part of this packet to transmit flags
	uint flags( void ) const
	{
		return getInt( 37 );
	}

	int slot( void ) const
	{
		return getInt( 65 );
	}
	uint ip( void ) const
	{
		return getInt( 69 );
	}
};

// 0x05 Request Use
class cUORxRequestAttack : public cUOPacket
{
public:
	cUORxRequestAttack( const QByteArray& data ) : cUOPacket( data )
	{
	}

	uint serial( void ) const
	{
		return getInt( 1 );
	}
};

// 0x09 Request Look
class cUORxRequestLook : public cUOPacket
{
public:
	cUORxRequestLook( const QByteArray& data ) : cUOPacket( data )
	{
	}

	uint serial( void ) const
	{
		return getInt( 1 );
	}
};

// 0x2C Resurrection Menu
class cUORxResurrectionMenu : public cUOPacket
{
public:
	cUORxResurrectionMenu( const QByteArray& data ) : cUOPacket( data )
	{
	}

	enum eChoice
	{
		Manifest		= 0,
		Resurrect		= 1,
		Ghost			= 2,
		Count			= 3,
	};

	eChoice choice( void ) const
	{
		unsigned char choice = ( *this )[1];
		switch ( choice )
		{
		case 0:
			return Manifest;
		case 1:
			return Resurrect;
		default:
		case 2:
			return Ghost;
		};
	}
};

// 0xBF Multi Purpose Packet -> Split up into other packets later
class cUORxMultiPurpose : public cUOPacket
{
public:
	enum eSubCommands
	{
		unknown											= 0x00,
		initFastWalk									= 0x01,
		addFastWalk										= 0x02,
		closeGump										= 0x04,
		screenSize										= 0x05,
		partySystem										= 0x06,
		closeQuestArrow									= 0x07,
		changeMap										= 0x08,
		wrestlingStun									= 0x0a,
		setLanguage										= 0x0b,
		closedStatusGump								= 0x0c,
				// Unknown: 0x0d
		// Unknown: 0x0e
		unknownLoginInfo								= 0x0f,
		toolTip											= 0x10,
				// Unknown: 0x11
		// Unknown: 0x12
		contextMenuRequest								= 0x13,
		contextMenuSelection							= 0x15,
		codexOfWisdom									= 0x17,
		enableMapDiff									= 0x18,
				// extendedStats = 0x19, = Server Message
		extendedStats									= 0x1a,
		castSpell										= 0x1c,
		customHouseRequest								= 0x1e
	};

	cUORxMultiPurpose( const QByteArray& data ) : cUOPacket( data )
	{
	}
	eSubCommands subCommand( void ) const
	{
		return ( eSubCommands ) getShort( 3 );
	}
	static cUOPacket* packet( const QByteArray& data );
};

class cUORxExtendedStats : public cUORxMultiPurpose
{
public:
	cUORxExtendedStats( const QByteArray& data ) : cUORxMultiPurpose( data )
	{
	}
	unsigned char stat() const
	{
		return ( *this )[5];
	}

	unsigned char lock() const
	{
		return ( *this )[6];
	}
};

// 0xBF 0x1C
class cUORxCastSpell : public cUORxMultiPurpose
{
public:
	cUORxCastSpell( const QByteArray& data ) : cUORxMultiPurpose( data )
	{
	}
	ushort unknown1() const
	{
		return getShort( 5 );
	}
	ushort spell() const
	{
		return getShort( 7 );
	}
};

// 0xBF 0x07
class cUORxCloseQuestArrow : public cUORxMultiPurpose
{
public:
	cUORxCloseQuestArrow( const QByteArray& data ) : cUORxMultiPurpose( data )
	{
	}
};

// 0xBF 0x13 PopUp Menu Request
class cUORxContextMenuRequest : public cUORxMultiPurpose
{
public:
	cUORxContextMenuRequest( const QByteArray& data ) : cUORxMultiPurpose( data )
	{
	}
	uint serial( void ) const
	{
		return getInt( 5 );
	}
};

// 0xBF 0x15 PopUp Menu Selection
class cUORxContextMenuSelection : public cUORxMultiPurpose
{
public:
	cUORxContextMenuSelection( const QByteArray& data ) : cUORxMultiPurpose( data )
	{
	}

	uint serial( void ) const
	{
		return getInt( 5 );
	}

	ushort entryTag( void ) const
	{
		return getShort( 9 );
	}
};

// 0xBF 0x0B Set Client Language
class cUORxSetLanguage : public cUORxMultiPurpose
{
public:
	cUORxSetLanguage( const QByteArray& data ) : cUORxMultiPurpose( data )
	{
	}

	QByteArray language( void ) const
	{
		return this->getAsciiString( 5, 4 );
	}
};

//0xBF 0x10 Server tooltip question

class cUORxRequestToolTip : public cUORxMultiPurpose
{
public:
	cUORxRequestToolTip( const QByteArray& data ) : cUORxMultiPurpose( data )
	{
	}
	uint serial( void ) const
	{
		return getInt( 5 );
	}
};

// 0xBF 0x1E Client request for custom house design details
class cUORxCustomHouseRequest : public cUORxMultiPurpose
{
public:
	cUORxCustomHouseRequest( const QByteArray& data ) : cUORxMultiPurpose( data )
	{
	}
	uint serial( void ) const
	{
		return getInt( 5 );
	}
};


// 0xBD Set Version
class cUORxSetVersion : public cUOPacket
{
public:
	cUORxSetVersion( const QByteArray& data ) : cUOPacket( data )
	{
	}
	QString version( void ) const
	{
		return this->getAsciiString( 3, getShort( 1 ) - 3 );
	}
};

// 0xEF New Client Version
class cUORxNewSetVersion : public cUOPacket
{
public:
	cUORxNewSetVersion() : cUOPacket( 0xEF, 21 )
	{
	}
	cUORxNewSetVersion( const QByteArray& data ) : cUOPacket( data )
	{
	}
	uint getIp( void ) const
	{
		return getInt( 1 );
	}
	uint getMajor( void ) const
	{
		return getInt( 5 );
	}
	uint getMinor( void ) const
	{
		return getInt( 9 );
	}
	uint getRevision( void ) const
	{
		return getInt( 13 );
	}
	uint getBuild( void ) const
	{
		return getInt( 17 );
	}
};

// 0x02 Walk Request.
class cUORxWalkRequest : public cUOPacket
{
public:
	cUORxWalkRequest( const QByteArray& data ) : cUOPacket( data )
	{
	}
	uchar direction() const
	{
		return (*this)[1];
	}
	uchar key() const
	{
		return ( *this )[2];
	}
	uint fastWalkKey() const
	{
		return getInt( 3 );
	}
};

// 0xAD Speech Request
class cUORxSpeechRequest : public cUOPacket
{
public:
	cUORxSpeechRequest( const QByteArray& data ) : cUOPacket( data )
	{
	}
	uchar type() const
	{
		return ( *this )[3];
	}
	ushort color() const
	{
		return getShort( 4 );
	}
	ushort font() const
	{
		return getShort( 6 );
	}
	ushort keywordCount() const
	{
		return getShort( 12 ) >> 4;
	}
	QList< ushort > keywords();
	QString language() const
	{
		return this->getAsciiString( 12, 4 );
	}
	QString message();
};

//0x06 Double Click
class cUORxDoubleClick : public cUOPacket
{
public:
	cUORxDoubleClick( const QByteArray& data ) : cUOPacket( data )
	{
	}
	SERIAL serial() const
	{
        return getInt( 1 ) & 0x7FFFFFFF;
	}
	bool keyboard() const
	{
        return ( *this )[1] & 0x80000000;
	}
};

//0xA7 Get Tip
class cUORxGetTip : public cUOPacket
{
public:
	cUORxGetTip( const QByteArray& data ) : cUOPacket( data )
	{
	}
	ushort lastTip() const
	{
		return getShort( 1 );
	}
	bool isTip() const
	{
		return ( *this )[3] == 0;
	}
	bool isNotice() const
	{
		return ( *this )[3] == 1;
	}
};

// 0x75 Rename Character
class cUORxRename : public cUOPacket
{
public:
	cUORxRename( const QByteArray& data ) : cUOPacket( data )
	{
	}
	uint serial() const
	{
		return getInt( 1 );
	}
	QString name() const
	{
		return this->getAsciiString( 5, 30 );
	}
};

// 0x72 Change Warmode
class cUORxChangeWarmode : public cUOPacket
{
public:
	cUORxChangeWarmode( const QByteArray& data ) : cUOPacket( data )
	{
	}
	bool warmode() const
	{
		return ( ( *this )[1] == 0 ) ? false : true;
	}
};

// 0x6C Target
class cUORxTarget : public cUOPacket
{
public:
	cUORxTarget( const QByteArray& data ) : cUOPacket( data )
	{
	}
	uchar type() const
	{
		return ( *this )[1];
	}
	uint targetSerial() const
	{
		return getInt( 2 );
	}
	uchar cursorType() const
	{
		return ( *this )[6];
	}
	uint serial()	const
	{
		return getInt( 7 );
	}
	ushort x() const
	{
		return getShort( 11 );
	}
	ushort y() const
	{
		return getShort( 13 );
	}
	uchar unknown() const
	{
		return ( *this )[15];
	}
	signed char z() const
	{
		return ( *this )[16];
	}
	ushort model() const
	{
		return getShort( 17 );
	}
};

// 0x6F Secure Trading
class cUORxSecureTrading : public cUOPacket
{
public:
	cUORxSecureTrading( const QByteArray& data ) : cUOPacket( data )
	{
	}
	uchar type() const
	{
		return ( *this )[3];
	}
	uint itemserial() const
	{
		return getInt( 4 );
	}
	uchar buttonstate() const
	{
		return ( *this )[11];
	}
};

// 0x22 ResyncWalk
class cUORxResyncWalk : public cUOPacket
{
public:
	cUORxResyncWalk( const QByteArray& data ) : cUOPacket( data )
	{
	}
	uchar sequence() const
	{
		return ( *this )[1];
	}
};

// 0x07 Drag Items
class cUORxDragItem : public cUOPacket
{
public:
	cUORxDragItem( const QByteArray& data ) : cUOPacket( data )
	{
	}
	ushort amount() const
	{
		return getShort( 5 );
	}
	uint serial() const
	{
		return getInt( 1 );
	}
};

// 0x08 Drop Items
// Old Clients
class cUORxDropItem : public cUOPacket
{
public:
	cUORxDropItem( const QByteArray& data ) : cUOPacket( data )
	{
	}
	uint serial() const
	{
		return getInt( 1 );
	}
	ushort x() const
	{
		return getShort( 5 );
	}
	ushort y() const
	{
		return getShort( 7 );
	}
	signed char z() const
	{
		return ( *this )[9];
	}
	uint cont() const
	{
		return getInt( 10 );
	}
};

class cUORxNewDropItem : public cUOPacket
{
public:
	cUORxNewDropItem(const QByteArray& data) : cUOPacket(data)
	{
	}
	uint serial() const
	{
		return getInt(1);
	}
	ushort x() const
	{
		return getShort(5);
	}
	ushort y() const
	{
		return getShort(7);
	}
	signed char z() const
	{
		return (*this)[9];
	}
	unsigned char gridLocation() const
	{
		return (*this)[10];
	}
	uint cont() const
	{
		return getInt(11);
	}
};

// 0x13 Wear Item
class cUORxWearItem : public cUOPacket
{
public:
	cUORxWearItem( const QByteArray& data ) : cUOPacket( data )
	{
	}
	uint serial()	const
	{
		return getInt( 1 );
	}
	uchar layer() const
	{
		return ( *this )[5];
	}
	uint wearer() const
	{
		return getInt( 6 );
	}
};

// 0xB1 Gump Response
class cUORxGumpResponse : public cUOPacket
{
public:
	cUORxGumpResponse( const QByteArray& data ) : cUOPacket( data )
	{
	}
	short size() const
	{
		return getShort( 1 );
	}
	uint serial() const
	{
		return getInt( 3 );
	}
	uint type() const
	{
		return getInt( 7 );
	}
	gumpChoice_st choice();
};

// 0x3B Buy
class cUORxBuy : public cUOPacket
{
public:
	cUORxBuy( const QByteArray& data ) : cUOPacket( data )
	{
	}
	uint serial() const
	{
		return getInt( 3 );
	}
	ushort itemCount() const
	{
		return ( ( size() - 8 ) / 7 );
	}
	uchar iLayer( ushort item ) const
	{
		return ( *this )[8 + ( item * 7 )];
	}
	uint iSerial( ushort item ) const
	{
		return getInt( 9 + ( item * 7 ) );
	}
	ushort iAmount( ushort item ) const
	{
		return getShort( 13 + ( item * 7 ) );
	}
};

// 0x9B Help Request - nice one :D
class cUORxHelpRequest : public cUOPacket
{
public:
	cUORxHelpRequest( const QByteArray& data ) : cUOPacket( data )
	{
	}
};

// 0x3A Set Skill Lock
class cUORxSkillLock : public cUOPacket
{
public:
	cUORxSkillLock( const QByteArray& data ) : cUOPacket( data )
	{
	}
	ushort skill() const
	{
		return getShort( 3 );
	}
	uchar lock() const
	{
		return ( *this )[5];
	}
};

// 0xB8 Profile
class cUORxProfile : public cUOPacket
{
public:
	cUORxProfile( const QByteArray& data ) : cUOPacket( data )
	{
	}
	uchar mode() const
	{
		return ( *this )[3];
	}
	uint serial() const
	{
		return getInt( 4 );
	}
	ushort command() const
	{
		return getShort( 8 );
	}
	QString text() const
	{
		return getUnicodeString( 12, getShort( 10 ) * 2 );
	}
};

// 0xD7 Aos fighting book, Custom houses build commands
class  cUORxAosMultiPurpose : public cUOPacket
{
public:
	enum eSubCommands
	{
		CHBackup			= 0x02, //Custom house backup, no additional data in packet (nadip)
		CHRestore			= 0x03, //Custom house restore (nadip)
		CHCommit			= 0x04, //Custom house commitment (nadip)
		CHDelete			= 0x05, //Custom house delete element
		CHAddElement		= 0x06, //Build wall or other element when customizing house
		CHClose				= 0x0C, //Close CH designer (nadip)
		CHStairs			= 0x0D, //Build stairs
		CHSync				= 0x0E, //Sync (nadip)
		CHClear				= 0x10, //Clear (nadip)
		CHLevel				= 0x12, //Select house level
		AbilitySelect		= 0x19, //Ability select
		CHRevert			= 0x1A, //Revert (nadip)
		GuildButton			= 0x28, //Guild Button
		QuestButton			= 0x32, //Quest Button
	};

	cUORxAosMultiPurpose( const QByteArray& data ) : cUOPacket( data )
	{
	}
	uint serial() const
	{
		return getInt( 3 );
	}
	ushort subCommand() const
	{
		return getShort( 7 );
	}
	static cUOPacket* packet( const QByteArray& data );
};

// 0xD7 0x05 Custom house delete element
class cUORxCHDelete : public cUORxAosMultiPurpose
{
public:
	cUORxCHDelete( const QByteArray& data ) : cUORxAosMultiPurpose( data )
	{
	}
	uint elementId() const
	{
		return getInt( 10 );
	}
	uint x() const
	{
		return getInt( 15 );
	}
	uint y() const
	{
		return getInt( 20 );
	}
	uint z() const
	{
		return getInt( 25 );
	}
};

// 0xD7 0x06 Build wall or other element when customizing house
class cUORxCHAddElement : public cUORxAosMultiPurpose
{
public:
	cUORxCHAddElement( const QByteArray& data ) : cUORxAosMultiPurpose( data )
	{
	}
	uint elementId() const
	{
		return getInt( 10 );
	}
	uint x() const
	{
		return getInt( 15 );
	}
	uint y() const
	{
		return getInt( 20 );
	}
	// z is == level z
};
// 0xD7 0x0D Build stairs
class cUORxCHStairs : public cUORxAosMultiPurpose
{
public:
	cUORxCHStairs( const QByteArray& data ) : cUORxAosMultiPurpose( data )
	{
	}
	uint multiId() const
	{
		return getInt( 10 );
	}
	uint x() const
	{
		return getInt( 15 );
	}
	uint y() const
	{
		return getInt( 20 );
	}
	// z is == level z
};
// 0xD7 0x12 Select house level
class cUORxCHLevel : public cUORxAosMultiPurpose
{
public:
	cUORxCHLevel( const QByteArray& data ) : cUORxAosMultiPurpose( data )
	{
	}
	uint level() const
	{
		return getInt( 10 );
	}
};

class cUORxSell : public cUOPacket
{
public:
	cUORxSell( const QByteArray& data ) : cUOPacket( data )
	{
	}

	uint serial() const
	{
		return getInt( 3 );
	}
	ushort itemCount() const
	{
		return getShort( 7 );
	}
	uint iSerial( ushort item ) const
	{
		return getInt( 9 + ( item * 6 ) );
	}
	ushort iAmount( ushort item ) const
	{
		return getShort( 13 + ( item * 6 ) );
	}
};

// 0x98 AllNames
class cUORxAllNames : public cUOPacket
{
public:
	cUORxAllNames( const QByteArray& data ) : cUOPacket( data )
	{
	}

	unsigned int serial()
	{
		return getInt( 3 );
	}

	const QString name()
	{
		return getAsciiString( 7 );
	}
};

// 0xD6 Request Tooltips
class cUORxRequestTooltips : public cUOPacket
{
public:
	cUORxRequestTooltips( const QByteArray& data ) : cUOPacket( data ) {
	}

	const unsigned short getCount() {
		return (getShort(1) - 3) / 4;
	}

	const unsigned int getSerial(unsigned short index) {
		return getInt(3 + index * 4);
	}
};


// 0xFB Update View Public House Contents
class cUORxUpdateViewPublicHouseContents : public cUOPacket
{
public:
	cUORxUpdateViewPublicHouseContents( const QByteArray& data ) : cUOPacket( data )
	{
	}

	uchar show( void ) const
	{
		return ( *this )[1];
	}
};

// 0xD9: Hardware Info
class cUORxHardwareInfo : public cUOPacket
{
public:
	cUORxHardwareInfo() : cUOPacket( 0xD9, 268 )
	{
	}
	cUORxHardwareInfo( const QByteArray& data ) : cUOPacket( data )
	{
	}
	uchar clientType( void ) const
	{
		return ( *this )[1];
	}
	uint instanceID( void ) const
	{
		return getInt( 2 );
	}
	uint osMajor( void ) const
	{
		return getInt( 6 );
	}
	uint osMinor( void ) const
	{
		return getInt( 10 );
	}
	uint osRevision( void ) const
	{
		return getInt( 14 );
	}
	uchar cpuManifacture( void ) const
	{
		return ( *this )[18];
	}
	uint cpuFamily( void ) const
	{
		return getInt( 19 );
	}
	uint cpuModel( void ) const
	{
		return getInt( 23 );
	}
	uint cpuClockSpeed( void ) const
	{
		return getInt( 27 );
	}
	uchar cpuQuantity( void ) const
	{
		return ( *this )[31];
	}
	uint memoryPhysical( void ) const
	{
		return getInt( 32 );
	}
	uint screenWidth( void ) const
	{
		return getInt( 36 );
	}
	uint screenHeight( void ) const
	{
		return getInt( 40 );
	}
	uint screenDeth( void ) const
	{
		return getInt( 44 );
	}
	ushort directxMajor( void ) const
	{
		return getShort( 48 );
	}
	ushort directxMinor( void ) const
	{
		return getShort( 51 );
	}
	QByteArray gpuDescription( void ) const
	{
		return this->getAsciiString( 52, 64 );
	}
	uint gpuVendorID( void ) const
	{
		return getInt( 179 );
	}
	uint gpuDeviceID( void ) const
	{
		return getInt( 182 );
	}
	uint gpuMemory( void ) const
	{
		return getInt( 186 );
	}
	uchar distribution( void ) const
	{
		return ( *this )[190];
	}
	uchar clientsRnning( void ) const
	{
		return ( *this )[191];
	}
	uchar clientsInstalled( void ) const
	{
		return ( *this )[192];
	}
	uchar clientsPartialInstalled( void ) const
	{
		return ( *this )[193];
	}
	QByteArray language( void ) const
	{
		return this->getAsciiString( 194, 4 );
	}
	QByteArray unk( void ) const
	{
		return this->getAsciiString( 202, 32 );
	}
};

#endif // __UO_RXPACKETS__
