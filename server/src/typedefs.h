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

#ifndef __TYPEDEFS_H__
#define __TYPEDEFS_H__

// Forward Base Classes declaration

class cItem;
class cBaseChar;
class cNPC;
class cPlayer;
class cUObject;
class cUOSocket;
class Coord_cl;
class cParty;
class cUOPacket;

// Typedefs

typedef int GUILDID;
typedef unsigned int TIMERVAL;
typedef int PLAYER;
typedef int CHARACTER;
typedef unsigned char LIGHTLEVEL;
typedef unsigned char SECONDS;
typedef int ITEM;
typedef int SERIAL;
typedef cItem * P_ITEM;
typedef const cItem * PC_ITEM;
typedef cBaseChar * P_CHAR;
typedef const cBaseChar * PC_CHAR;
typedef cNPC * P_NPC;
typedef const cNPC * PC_NPC;
typedef cPlayer * P_PLAYER;
typedef const cPlayer * PC_PLAYER;
typedef cUObject * P_OBJECT;

enum WPPAGE_TYPE
{
	PT_GM = 0,
	PT_COUNSELOR
};

enum enBodyParts
{
	ALLBODYPARTS = 0,
	LEGS,
	BODY,
	ARMS,
	HANDS,
	NECK,
	HEAD,
	DEADLY
};

enum enCharTypes
{
	enNPC = 0, // Non Player Characters (cNPC)
	enPlayer, // Player Characters (cPlayer)
	enNumberOfCharTypes
};

enum enWanderTypes
{
	enHalt = 0,
	enFreely,
	enRectangle,
	enCircle,
	enFollowTarget,
	enDestination,
	enNumberOfWanderTypes
};

enum eDamageType
{
	DAMAGE_PHYSICAL = 0,
	DAMAGE_MAGICAL = 1,
	DAMAGE_GODLY = 2,
	DAMAGE_HUNGER = 3
};

enum enAnimations {
	ANIM_WALK_UNARM = 0x00,
	ANIM_WALK_ARM = 0x01,
	ANIM_RUN_UNARM = 0x02,
	ANIM_RUN_ARMED = 0x03,
	ANIM_STAND = 0x04,
	ANIM_FIDGET1 = 0x05,
	ANIM_FIDGET_YAWN = 0x06,
	ANIM_STAND_WAR_1H = 0x07,
	ANIM_STAND_WAR_2H = 0x08,
	ANIM_ATTACK_1H_WIDE = 0x09,
	ANIM_ATTACK_1H_JAB = 0x0a,
	ANIM_ATTACK_1H_DOWN = 0x0b,
	ANIM_ATTACK_2H_DOWN = 0x0c,
	ANIM_ATTACK_2H_WIDE = 0x0d,
	ANIM_ATTACK_2H_JAB = 0x0e,
	ANIM_WALK_WAR = 0x0f,
	ANIM_CAST_DIR = 0x10,
	ANIM_CAST_AREA = 0x11,
	ANIM_ATTACK_BOW = 0x12,
	ANIM_ATTACK_XBOW = 0x13,
	ANIM_GET_HIT = 0x14,
	ANIM_DIE_BACK = 0x15,
	ANIM_DIE_FORWARD = 0x16,
	ANIM_BLOCK = 0x1e,
	ANIM_ATTACK_UNARM = 0x1f,
	ANIM_BOW = 0x20,
	ANIM_SALUTE = 0x21,
	ANIM_EAT = 0x22,
	ANIM_HORSE_RIDE_SLOW = 0x17,
	ANIM_HORSE_RIDE_FAST = 0x18,
	ANIM_HORSE_STAND = 0x19,
	ANIM_HORSE_ATTACK = 0x1a,
	ANIM_HORSE_ATTACK_BOW = 0x1b,
	ANIM_HORSE_ATTACK_XBOW = 0x1c,
	ANIM_HORSE_SLAP = 0x1d,

	// These animations are used by animals
	// animals. (Most All animals have all anims)
	ANIM_ANI_WALK = 0x00,
	ANIM_ANI_RUN = 0x01,
	ANIM_ANI_STAND = 0x02,
	ANIM_ANI_EAT = 0x03,
	ANIM_ANI_ALERT = 0x04,
	ANIM_ANI_ATTACK1 = 0x05,
	ANIM_ANI_ATTACK2 = 0x06,
	ANIM_ANI_GETHIT = 0x07,
	ANIM_ANI_DIE1 = 0x08,
	ANIM_ANI_FIDGET1 = 0x09,
	ANIM_ANI_FIDGET2 = 0x0a,
	ANIM_ANI_SLEEP = 0x0b,
	ANIM_ANI_DIE2 = 0x0c,
};

#endif
