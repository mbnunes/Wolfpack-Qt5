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
* This code is an attempt to clean up the messy "if/then/else" routines
* currently in use for GM commands, as well as adding more functionality
* and more potential for functionality.
*
* Current features:
* - Actual table of commands to execute, what perms are required, dialog
*   messages for target commands, etc handled by a central system
*
* - /SETPRIV3 removed... /SETPRIVLVL included you can determine 255 privlevels so this should be enough...
*/

#include "wolfpack.h"
#include "cmdtable.h"
#include "spawnregions.h"
#include "territories.h"
#include "bounty.h"
#include "SndPkg.h"
#include "worldmain.h"
#include "debug.h"
#include "regions.h"
#include "srvparams.h"
#include "network.h"
#include "classes.h"
#include "gumps.h"
#include "mapstuff.h"
#include "wpdefmanager.h"
#include "wpscriptmanager.h"

// Library Includes
#include "qdatetime.h"

#include "targetrequests.h"

#undef  DBGFILE
#define DBGFILE "cmdtable.cpp"


TARGET_S target_use = { 0, 1, 0, 24, "What object will you use?" };
TARGET_S target_jail = { 0, 1, 0, 126, "Select player to jail." };
TARGET_S target_release = { 0, 1, 0, 127, "Select player to release from jail." };
TARGET_S target_istats = { 0, 1, 0, 12, "Select item to inspect." };
TARGET_S target_cstats = { 0, 1, 0, 13, "Select char to inspect." };
TARGET_S target_tele = { 0, 1, 0, 2, "Select teleport target." };
TARGET_S target_xbank = { 0, 1, 0, 107, "Select target to open bank of." };
TARGET_S target_remove = { 0, 1, 0, 3, "Select item to remove." };
TARGET_S target_makegm = { 0, 1, 0, 14, "Select character to make a GM." };
TARGET_S target_makecns = { 0, 1, 0, 15, "Select character to make a Counselor." };
TARGET_S target_killhair = { 0, 1, 0, 16, "Select character for cutting hair." };
TARGET_S target_killbeard = { 0, 1, 0, 17, "Select character for shaving." };
TARGET_S target_kill = { 0, 1, 0, 20, "Select character to kill." };
TARGET_S target_resurrect = { 0, 1, 0, 21, "Select character to resurrect." };
TARGET_S target_bolt = { 0, 1, 0, 22, "Select character to bolt." };
// This fires a harmless bolt at the user.
TARGET_S target_kick = { 0, 1, 0, 25, "Select character to kick." };
// This disconnects the player targeted from the game. They
// can still log back in.
TARGET_S target_movetobag = { 0, 1, 0, 111, "Select an item to move into your bag." };
TARGET_S target_xgo = { 0, 1, 0, 8, "Select char to teleport." };
TARGET_S target_setmorex = { 0, 1, 0, 63, "Select object to set morex on." };
TARGET_S target_setmorey = { 0, 1, 0, 64, "Select object to set morey on." };
TARGET_S target_setmorez = { 0, 1, 0, 65, "Select object to set morez on." };
TARGET_S target_setmorexyz = { 0, 1, 0, 66, "Select object to set morex, morey, and morez on." };
TARGET_S target_sethexmorexyz = { 0, 1, 0, 66, "Select object to set hex morex, morey, and morez on." };
TARGET_S target_setnpcai = { 0, 1, 0, 106, "Select npc to set AI type on." };
TARGET_S target_newz = { 0, 1, 0, 5, "Select item to reposition." };
TARGET_S target_settype = { 0, 1, 0, 6, "Select item to edit type." };
TARGET_S target_itrig = { 0, 1, 0, 200, "Select item to trigger." };
TARGET_S target_ctrig = { 0, 1, 0, 201, "Select NPC to trigger." };
TARGET_S target_ttrig = { 0, 1, 0, 202, "Select item to set trigger type." };
TARGET_S target_setid = { 0, 1, 0, 7, "Select item to polymorph." };
TARGET_S target_setmore = { 0, 1, 0, 10, "Select item to edit 'more' value." };
TARGET_S target_setfont = { 0, 1, 0, 19, "Select character to change font." };
TARGET_S target_npcaction = { 0, 1, 0, 53, "Select npc to make act." };
TARGET_S target_setamount = { 0, 1, 0, 23, "Select item to edit amount." };
TARGET_S target_setamount2 = { 0, 1, 0, 129, "Select item to edit amount." };
TARGET_S target_setmovable = { 0, 1, 0, 28, "Select item to edit mobility." };
TARGET_S target_setvisible = { 0, 1, 0, 61, "Select item to edit visibility." };
TARGET_S target_setdir = { 0, 1, 0, 88, "Select item to edit direction." };
TARGET_S target_setspeech = { 0, 1, 0, 135, "Select NPC to edit speech." };
TARGET_S target_setowner = { 0, 1, 0, 30, "Select NPC or OBJECT to edit owner." };
TARGET_S target_freeze = { 0, 1, 0, 34, "Select player to freeze in place." };
TARGET_S target_unfreeze = { 0, 1, 0, 35, "Select player to unfreeze." };
TARGET_S target_tiledata = { 0, 1, 0, 46, "Select item to inspect." };
TARGET_S target_recall = { 0, 1, 0, 38, "Select rune from which to recall." };
TARGET_S target_mark = { 0, 1, 0, 39, "Select rune to mark." };
TARGET_S target_gate = { 0, 1, 0, 43, "Select rune from which to gate." };
TARGET_S target_heal = { 0, 1, 0, 44, "Select person to heal." };
TARGET_S target_npctarget = { 0, 1, 0, 56, "Select player for the NPC to follow." };
TARGET_S target_tweak = { 0, 1, 0, 62, "Select item or character to tweak." };
TARGET_S target_sbopen = { 0, 1, 0, 87, "Select spellbook to open as a container." };
TARGET_S target_mana = { 0, 1, 0, 113, "Select person to restore mana to." };
TARGET_S target_stamina = { 0, 1, 0, 114, "Select person to refresh." };
TARGET_S target_makeshop = { 0, 1, 0, 116, "Select the character to add shopkeeper buy containers to." };
TARGET_S target_buy = { 0, 1, 0, 121, "Select the shopkeeper you'd like to buy from." };
TARGET_S target_setvalue = { 0, 1, 0, 122, "Select item to edit value." };
TARGET_S target_setrestock = { 0, 1, 0, 123, "Select item to edit amount to restock." };
TARGET_S target_sell = { 0, 1, 0, 112, "Select the NPC to sell to." };
TARGET_S target_setspattack = { 0, 1, 0, 150, "Select creature to set SPATTACK on." };
TARGET_S target_setspadelay = { 0, 1, 0, 177, "Select creature to set SPADELAY on." };
TARGET_S target_setpoison = { 0, 1, 0, 175, "Select creature to set POISON." };
TARGET_S target_setpoisoned = { 0, 1, 0, 176, "Select creature to set POISONED." };
TARGET_S target_setadvobj = { 0, 1, 0, 178, "Select creature to set ADVOBJ." };
TARGET_S target_setwipe = { 0, 1, 0, 133, "Select item to modify." };
TARGET_S target_fullstats = { 0, 1, 0, 151, "Select creature to restore full stats." };
TARGET_S target_hide = { 0, 1, 0, 131, "Select creature to hide." };
TARGET_S target_unhide = { 0, 1, 0, 132, "Select creature to reveal." };
TARGET_S target_house = { 0, 1, 0, 207, "Select location for house." };
TARGET_S target_split = { 0, 1, 0, 209, "Select creature to make able to split." };
TARGET_S target_splitchance = { 0, 1, 0, 210, "Select creature to set it's chance of splitting." };
TARGET_S target_possess = { 0, 1, 0, 212, "Select creature to possess." };
TARGET_S target_telestuff = { 0, 1, 0, 222, "Select player/object to teleport." };
TARGET_S target_killpack = { 0, 1, 0, 18, "Select character to remove pack." };
TARGET_S target_trainer = { 0, 1, 0, 206, "Select character to become a trainer." };
TARGET_S target_showprivlvl = { 0, 1, 0, 226, "Select character to display priviliges." };
TARGET_S target_ban = { 0, 1, 0, 235, "Select character to BAN." };
TARGET_S target_newx = { 0, 1, 0, 251, "Select item to reposition." };
TARGET_S target_newy = { 0, 1, 0, 252, "Select item to reposition." };
TARGET_S target_incx = { 0, 1, 0, 253, "Select item to reposition." };
TARGET_S target_incy = { 0, 1, 0, 254, "Select item to reposition." };
TARGET_S target_glow = { 0, 1, 0, 255, "Select item to make glowing." };
TARGET_S target_unglow = { 0, 1, 0, 249, "Select item to deactivate glowing." };
TARGET_S target_showskills = { 0, 1, 0, 247, "Select char to see skills" };
TARGET_S target_showaccountcomment = { 0, 1, 0, 48, "Select character to show comment of." };
TARGET_S target_sethome = { 0,1,0,256, "Set home location for a npc."};
TARGET_S target_setwork = { 0,1,0,257, "Set work location for a npc."};
TARGET_S target_setfood = { 0,1,0,258, "Set food location for a npc."};

CMDTABLE_S command_table[] = {
	{"SETGMMOVEEFF",	CMD_FUNC,	(CMD_DEFINE)&command_setGmMoveEff},
	{"BOUNTY",			CMD_FUNC,   (CMD_DEFINE)&command_bounty},
	{"POST",			CMD_FUNC,   (CMD_DEFINE)&command_post},
	{"GPOST",			CMD_FUNC,   (CMD_DEFINE)&command_gpost},
	{"RPOST",			CMD_FUNC,   (CMD_DEFINE)&command_rpost},
	{"LPOST",			CMD_FUNC,   (CMD_DEFINE)&command_lpost},
	{"SHOWP",			CMD_FUNC,	(CMD_DEFINE)&command_showp},
	{"SETPRIVLVL",		CMD_FUNC,	(CMD_DEFINE)&command_setprivlvl},
	{"USE",				CMD_TARGET,	(CMD_DEFINE)&target_use},
	{"RESEND",			CMD_FUNC,	(CMD_DEFINE)&command_resend},
	{"POINT",			CMD_FUNC,	(CMD_DEFINE)&command_teleport},
	{"WHERE",			CMD_FUNC,	(CMD_DEFINE)&command_where},
	{"ADDU",			CMD_ITEMMENU,	(CMD_DEFINE)1}, // Opens the GM add menu.
	{"Q",				CMD_FUNC,	(CMD_DEFINE)&command_q},
	{"NEXT",			CMD_FUNC,	(CMD_DEFINE)&command_next},
	{"CLEAR",			CMD_FUNC,	(CMD_DEFINE)&command_clear},
	{"GOTOCUR",			CMD_FUNC,	(CMD_DEFINE)&command_gotocur},
	{"GMTRANSFER",		CMD_FUNC,	(CMD_DEFINE)&command_gmtransfer},
	//{"JAIL",	0,	11,	CMD_TARGET,	(CMD_DEFINE)&target_jail},
	{"JAIL",			CMD_FUNC,   (CMD_DEFINE)&command_jail},
	{"RELEASE",			CMD_TARGET,	(CMD_DEFINE)&target_release},
	{"ISTATS",			CMD_TARGET,	(CMD_DEFINE)&target_istats},
	{"CSTATS",			CMD_TARGET,	(CMD_DEFINE)&target_cstats},
	{"GOPLACE",			CMD_FUNC,	(CMD_DEFINE)&command_goplace},
	{"GOCHAR",			CMD_FUNC,	(CMD_DEFINE)&command_gochar},
	{"FIX",				CMD_FUNC,	(CMD_DEFINE)&command_fix},
	{"XGOPLACE",		CMD_FUNC,	(CMD_DEFINE)&command_xgoplace},
	{"SHOWIDS",			CMD_FUNC,	(CMD_DEFINE)&command_showids},
	{"POLY",			CMD_FUNC,	(CMD_DEFINE)&command_poly},
	{"SKIN",			CMD_FUNC,	(CMD_DEFINE)&command_skin},
	{"ACTION",			CMD_FUNC,	(CMD_DEFINE)&command_action},
	{"TELE",			CMD_TARGET,	(CMD_DEFINE)&target_tele},
	{"XTELE",			CMD_FUNC,	(CMD_DEFINE)&command_xtele},
	{"GO",				CMD_FUNC,	(CMD_DEFINE)&command_go},
	{"XGO",				CMD_TARGETXYZ,	(CMD_DEFINE)&target_xgo},
	{"SETMOREX",		CMD_TARGETX,(CMD_DEFINE)&target_setmorex},
	{"SETMOREY",		CMD_TARGETX,(CMD_DEFINE)&target_setmorey},
	{"SETMOREZ",		CMD_TARGETX,(CMD_DEFINE)&target_setmorez},
	{"ZEROKILLS",		CMD_FUNC,	(CMD_DEFINE)&command_zerokills},
	{"SETMOREXYZ",		CMD_TARGETXYZ,	(CMD_DEFINE)&target_setmorexyz},
	{"SETHEXMOREXYZ",	CMD_TARGETHXYZ,	(CMD_DEFINE)&target_sethexmorexyz},
	{"SETNPCAI",		CMD_TARGETHX,	(CMD_DEFINE)&target_setnpcai},
	{"XBANK",			CMD_TARGET,	(CMD_DEFINE)&target_xbank},
	{"TILE",			CMD_FUNC,	(CMD_DEFINE)&command_tile},
	{"WIPE",			CMD_FUNC,	(CMD_DEFINE)&command_wipe},
	{"IWIPE",			CMD_FUNC,	(CMD_DEFINE)&command_iwipe},
	{"ADD",				CMD_FUNC,	(CMD_DEFINE)&command_add},
	{"ADDX",			CMD_FUNC,	(CMD_DEFINE)&command_addx},
	{"RENAME",			CMD_FUNC,	(CMD_DEFINE)&command_rename},
	{"TITLE",			CMD_FUNC,	(CMD_DEFINE)&command_title},
	{"SAVE",			CMD_FUNC,	(CMD_DEFINE)&command_save},
	{"REMOVE",			CMD_TARGET,	(CMD_DEFINE)&target_remove},
	{"TRAINER",			CMD_TARGET,	(CMD_DEFINE)&target_trainer},
	{"DYE",				CMD_FUNC,	(CMD_DEFINE)&command_dye},
	{"NEWZ",			CMD_TARGETX,(CMD_DEFINE)&target_newz},
	{"SETTYPE",			CMD_TARGETID1,	(CMD_DEFINE)&target_settype},
	{"ITRIG",			CMD_TARGETX,(CMD_DEFINE)&target_itrig},
	{"CTRIG",			CMD_TARGETX,(CMD_DEFINE)&target_ctrig},
	{"TTRIG",			CMD_TARGETX,(CMD_DEFINE)&target_ttrig},
	{"WTRIG",			CMD_FUNC,	(CMD_DEFINE)&command_wtrig},
	{"SETID",			CMD_TARGETHXY,	(CMD_DEFINE)&target_setid},
	{"SETPRIV",			CMD_FUNC,	(CMD_DEFINE)&command_setpriv},
	{"NODECAY",			CMD_FUNC,	(CMD_DEFINE)&command_nodecay},
#ifdef UNRELEASED
	{"SEND",			CMD_FUNC,	(CMD_DEFINE)&command_send},
#endif
	{"SHOWTIME",		CMD_FUNC,	(CMD_DEFINE)&command_showtime},
	{"SETTIME",			CMD_FUNC,	(CMD_DEFINE)&command_settime},
	{"SETMORE",			CMD_TARGETHID4,	(CMD_DEFINE)&target_setmore},
	{"SHUTDOWN",		CMD_FUNC,	(CMD_DEFINE)&command_shutdown},
	{"MAKEGM",			CMD_TARGET,	(CMD_DEFINE)&target_makegm},
	{"MAKECNS",			CMD_TARGET,	(CMD_DEFINE)&target_makecns},
	{"KILLHAIR",		CMD_TARGET,	(CMD_DEFINE)&target_killhair},
	{"KILLBEARD",		CMD_TARGET,	(CMD_DEFINE)&target_killbeard},
	{"KILLPACK",		CMD_TARGET,	(CMD_DEFINE)&target_killpack},
	{"SETFONT",			CMD_TARGETHID1,	(CMD_DEFINE)&target_setfont},
	{"WHOLIST",			CMD_FUNC,	(CMD_DEFINE)&command_wholist},
	{"PLAYERLIST",		CMD_FUNC,	(CMD_DEFINE)&command_playerlist},
	{"KILL",			CMD_TARGET,	(CMD_DEFINE)&target_kill},
	{"RESURRECT",		CMD_TARGET,	(CMD_DEFINE)&target_resurrect},
	{"RES",				CMD_TARGET,	(CMD_DEFINE)&target_resurrect},
	{"BOLT",			CMD_TARGET,	(CMD_DEFINE)&target_bolt},
#ifdef UNRELEASED
	{"BLT2",			CMD_FUNC,	(CMD_DEFINE)&command_blt2},
#endif
	{"SFX",				CMD_FUNC,	(CMD_DEFINE)&command_sfx},
	{"NPCACTION",		CMD_TARGETHID1,	(CMD_DEFINE)&target_npcaction},
	{"LIGHT",			CMD_FUNC,	(CMD_DEFINE)&command_light},
	{"SETAMOUNT",		CMD_TARGETX,(CMD_DEFINE)&target_setamount},
	{"SETAMOUNT2",		CMD_TARGETX,(CMD_DEFINE)&target_setamount2},
#ifdef UNRELEASED
	{"WEB",				CMD_FUNC,	(CMD_DEFINE)&command_web},
#endif
	{"DISCONNECT",		CMD_FUNC,	(CMD_DEFINE)&command_disconnect},
	{"KICK",			CMD_TARGET,	(CMD_DEFINE)&target_kick},
	{"TELL",			CMD_FUNC,	(CMD_DEFINE)&command_tell},
	{"DRY",				CMD_FUNC,	(CMD_DEFINE)&command_dry},
	/* lord binary's debugging comands - commented out by default, but gotta
	be there I guess... */
	// {"NPCS",			CMD_FUNC,	(CMD_DEFINE)&command_npcs},
	// {"NPCSD",		CMD_FUNC,	(CMD_DEFINE)&command_npcsd},
	// {"PCS",			CMD_FUNC,	(CMD_DEFINE)&command_pcs},
	// {"CHARC",		CMD_FUNC,	(CMD_DEFINE)&command_charc},
	// {"ITEMC",		CMD_FUNC,	(CMD_DEFINE)&command_itemc},
	/* end of lord binary's debugging commands */
	{"RAIN",			CMD_FUNC,	(CMD_DEFINE)&command_rain},
	{"SNOW",			CMD_FUNC,	(CMD_DEFINE)&command_snow},
	{"GMMENU",			CMD_FUNC,	(CMD_DEFINE)&command_gmmenu},
	{"ITEMMENU",		CMD_FUNC,	(CMD_DEFINE)&command_itemmenu},
	{"ADDITEM",			CMD_FUNC,	(CMD_DEFINE)&command_additem},
	{"DUPE",			CMD_FUNC,	(CMD_DEFINE)&command_dupe},
	{"MOVETOBAG",		CMD_TARGET,	(CMD_DEFINE)&target_movetobag},
	{"COMMAND",			CMD_FUNC,	(CMD_DEFINE)&command_command},
	{"GCOLLECT",		CMD_FUNC,	(CMD_DEFINE)&command_gcollect},
	{"ALLMOVEON",		CMD_FUNC,	(CMD_DEFINE)&command_allmoveon},
	{"ALLMOVEOFF",		CMD_FUNC,	(CMD_DEFINE)&command_allmoveoff},
	{"SHOWHS",			CMD_FUNC,	(CMD_DEFINE)&command_showhs},
	{"HIDEHS",			CMD_FUNC,	(CMD_DEFINE)&command_hidehs},
	{"SETMOVABLE",		CMD_TARGETX,(CMD_DEFINE)&target_setmovable},
	{"SET",				CMD_FUNC,	(CMD_DEFINE)&command_set},
#ifdef UNRELEASED
	{"TEMP",			CMD_FUNC,	(CMD_DEFINE)&command_temp},
#endif
	{"SETVISIBLE",		CMD_TARGETX,(CMD_DEFINE)&target_setvisible},
	{"SETDIR",			CMD_TARGETX,(CMD_DEFINE)&target_setdir},
	{"SETSPEECH",		CMD_TARGETX,(CMD_DEFINE)&target_setspeech},
	{"SETOWNER",		CMD_TARGETHID4,	(CMD_DEFINE)&target_setowner},
	{"ADDNPC",			CMD_FUNC,	(CMD_DEFINE)&command_addnpc},
	{"FREEZE",			CMD_TARGET,	(CMD_DEFINE)&target_freeze},
	{"UNFREEZE",		CMD_TARGET,	(CMD_DEFINE)&target_unfreeze},
	{"READINI",			CMD_FUNC,	(CMD_DEFINE)&command_readini},
	{"GUMPMENU",		CMD_FUNC,	(CMD_DEFINE)&command_gumpmenu},
	{"TILEDATA",		CMD_TARGET,	(CMD_DEFINE)&target_tiledata},
	{"RECALL",			CMD_TARGET,	(CMD_DEFINE)&target_recall},
	{"MARK",			CMD_TARGET,	(CMD_DEFINE)&target_mark},
	{"GATE",			CMD_TARGET,	(CMD_DEFINE)&target_gate},
	{"HEAL",			CMD_TARGET,	(CMD_DEFINE)&target_heal},
	{"NPCTARGET",		CMD_TARGET,	(CMD_DEFINE)&target_npctarget},
	{"NPCRECT",			CMD_FUNC,	(CMD_DEFINE)&command_npcrect},
	{"NPCCIRCLE",		CMD_FUNC,	(CMD_DEFINE)&command_npccircle},
	{"NPCWANDER",		CMD_FUNC,	(CMD_DEFINE)&command_npcwander},
	{"TWEAK",			CMD_TARGET,	(CMD_DEFINE)&target_tweak},
	{"SBOPEN",			CMD_TARGET,	(CMD_DEFINE)&target_sbopen},
	{"SECONDSPERUOMINUTE",CMD_FUNC,	(CMD_DEFINE)&command_secondsperuominute},
	{"BRIGHTLIGHT",		CMD_FUNC,	(CMD_DEFINE)&command_brightlight},
	{"DARKLIGHT",		CMD_FUNC,	(CMD_DEFINE)&command_darklight},
	{"DUNGEONLIGHT",	CMD_FUNC,	(CMD_DEFINE)&command_dungeonlight},
	{"TIME",			CMD_FUNC,	(CMD_DEFINE)&command_time},
	{"MANA",			CMD_TARGET,	(CMD_DEFINE)&target_mana},
	{"STAMINA",			CMD_TARGET,	(CMD_DEFINE)&target_stamina},
	{"GMOPEN",			CMD_FUNC,	(CMD_DEFINE)&command_gmopen},
	{"MAKESHOP",		CMD_TARGET,	(CMD_DEFINE)&target_makeshop},
	{"BUY",				CMD_TARGET,	(CMD_DEFINE)&target_buy},
	{"SETVALUE",		CMD_TARGETX,(CMD_DEFINE)&target_setvalue},
	{"SETRESTOCK",		CMD_TARGETX,(CMD_DEFINE)&target_setrestock},
	{"RESTOCK",			CMD_FUNC,	(CMD_DEFINE)&command_restock},
	{"RESTOCKALL",		CMD_FUNC,	(CMD_DEFINE)&command_restockall},
	{"SETSHOPRESTOCKRATE",CMD_FUNC,	(CMD_DEFINE)&command_setshoprestockrate},
	{"WHO",				CMD_FUNC,	(CMD_DEFINE)&command_who},
	{"GMS",				CMD_FUNC,	(CMD_DEFINE)&command_gms},
	{"SELL",			CMD_TARGET,	(CMD_DEFINE)&target_sell},
	{"MIDI",			CMD_FUNC,	(CMD_DEFINE)&command_midi},
	{"GUMPOPEN",		CMD_FUNC,	(CMD_DEFINE)&command_gumpopen},
	{"RESPAWN",			CMD_FUNC,	(CMD_DEFINE)&command_respawn},
	{"REGSPAWNALL",		CMD_FUNC,	(CMD_DEFINE)&command_regspawnall},
	{"REGSPAWNMAX",		CMD_FUNC,	(CMD_DEFINE)&command_regspawnmax},
	{"REGSPAWN",		CMD_FUNC,	(CMD_DEFINE)&command_regspawn},
	{"SETSPATTACK",		CMD_TARGETHTMP,	(CMD_DEFINE)&target_setspattack},
	{"SETSPADELAY",		CMD_TARGETHTMP,	(CMD_DEFINE)&target_setspadelay},
	{"SETPOISON",		CMD_TARGETHTMP,	(CMD_DEFINE)&target_setpoison},
	{"SETPOISONED",		CMD_TARGETHTMP,	(CMD_DEFINE)&target_setpoisoned},
	{"SETADVOBJ",		CMD_TARGETHTMP,	(CMD_DEFINE)&target_setadvobj},
	{"SETWIPE",			CMD_TARGETID1,	(CMD_DEFINE)&target_setwipe},
	{"FULLSTATS",		CMD_TARGET,	(CMD_DEFINE)&target_fullstats},
	{"HIDE",			CMD_TARGET,	(CMD_DEFINE)&target_hide},
	{"UNHIDE",			CMD_TARGET,	(CMD_DEFINE)&target_unhide},
	{"RELOADSERVER",	CMD_FUNC,	(CMD_DEFINE)&command_reloadserver},
	{"CQ",				CMD_FUNC,	(CMD_DEFINE)&command_cq},
	{"WIPENPCS",		CMD_FUNC,	(CMD_DEFINE)&command_wipenpcs},
	{"CNEXT",			CMD_FUNC,	(CMD_DEFINE)&command_cnext},
	{"CCLEAR",			CMD_FUNC,	(CMD_DEFINE)&command_cclear},
	{"MINECHECK",		CMD_FUNC,	(CMD_DEFINE)&command_minecheck},
	{"INVUL",			CMD_FUNC,	(CMD_DEFINE)&command_invul},
	{"NOINVUL",			CMD_FUNC,	(CMD_DEFINE)&command_noinvul},
	{"GUARDSON",		CMD_FUNC,	(CMD_DEFINE)&command_guardson},
	{"GUARDSOFF",		CMD_FUNC,	(CMD_DEFINE)&command_guardsoff},
	{"HOUSE",			CMD_TARGETHTMP,	(CMD_DEFINE)&target_house},
	{"ANNOUNCEON",		CMD_FUNC,	(CMD_DEFINE)&command_announceon},
	{"ANNOUNCEOFF",		CMD_FUNC,	(CMD_DEFINE)&command_announceoff},
	{"WF",				CMD_FUNC,	(CMD_DEFINE)&command_wf},
	{"DECAY",			CMD_FUNC,	(CMD_DEFINE)&command_decay},
	{"SPLIT",			CMD_TARGETHTMP,	(CMD_DEFINE)&target_split},
	{"SPLITCHANCE",		CMD_TARGETHTMP,	(CMD_DEFINE)&target_splitchance},
	{"POSSESS",			CMD_TARGET,	(CMD_DEFINE)&target_possess},
	{"KILLALL",			CMD_FUNC,	(CMD_DEFINE)&command_killall},
	{"PDUMP",			CMD_FUNC,	(CMD_DEFINE)&command_pdump},
	{"RENAME2",			CMD_FUNC,	(CMD_DEFINE)&command_rename2},
	{"READSPAWNREGIONS",CMD_FUNC,	(CMD_DEFINE)&command_readspawnregions},
	{"CLEANUP",			CMD_FUNC,	(CMD_DEFINE)&command_cleanup},
	{"GY",				CMD_FUNC,	(CMD_DEFINE)&command_gy},
	{"TILEW",			CMD_FUNC,	(CMD_DEFINE)&command_tilew},
	{"SQUELCH",			CMD_FUNC,	(CMD_DEFINE)&command_squelch},
	{"MUTE",			CMD_FUNC,	(CMD_DEFINE)&command_squelch},
	{"TELESTUFF",		CMD_TARGET,	(CMD_DEFINE)&target_telestuff},
	{"SPAWNKILL",		CMD_FUNC,	(CMD_DEFINE)&command_spawnkill},
	{"SHOWPRIVLVL",		CMD_TARGET,	(CMD_DEFINE)&target_showprivlvl},
	{"NEWX",			CMD_TARGETX,(CMD_DEFINE)&target_newx},
	{"NEWY",			CMD_TARGETX,(CMD_DEFINE)&target_newy},
	{"INCX",			CMD_TARGETX,(CMD_DEFINE)&target_incx},
	{"INCY",			CMD_TARGETX,(CMD_DEFINE)&target_incy},
	{"WANIM",			CMD_FUNC,	(CMD_DEFINE)&command_wanim},
	
	/* {"SETACCOUNT",		CMD_FUNC,	(CMD_DEFINE)&command_setacct}, */
	
	{"READACCOUNTS",	CMD_FUNC,	(CMD_DEFINE)&command_readaccounts},
	{"LETUSIN",			CMD_FUNC,	(CMD_DEFINE)&command_letusin},
	{"SERVERSLEEP",		CMD_FUNC,	(CMD_DEFINE)&command_serversleep},
	{"RELOADCACHEDSCRIPTS",CMD_FUNC,(CMD_DEFINE)&command_reloadcachedscripts},
	
	/*
	{"REMOVEACCOUNT",		CMD_FUNC,	(CMD_DEFINE)&command_removeacct},
	*/
	
	{"SETMURDER",		CMD_FUNC,	(CMD_DEFINE)&command_setmurder},//taken from 6904t2(5/10/99) - AntiChrist
	{"GLOW",			CMD_TARGET, (CMD_DEFINE)&target_glow},
	{"UNGLOW",			CMD_TARGET, (CMD_DEFINE)&target_unglow},
	{"SETMENUPRIV",		CMD_FUNC,	(CMD_DEFINE)&command_setmenupriv},
	{"SHOWSKILLS",		CMD_TARGETX,(CMD_DEFINE)&target_showskills},
	
	/* always end with a section of NULLs to mark end of list */
	//{NULL,		NULL,	NULL,	NULL,		NULL}
	//taken from 6904t2(5/10/99) - AntiChrist
	
	//{"SHOWACCOUNTCOMMENT",	2,	10,	CMD_TARGET,	(CMD_DEFINE)&target_showaccountcomment},//AntiChrist
	
	{"DELID",			CMD_FUNC,	(CMD_DEFINE)&command_delid},
	{"DELTYPE",			CMD_FUNC,	(CMD_DEFINE)&command_deltype},
	{"SETSEASON",		CMD_FUNC,	(CMD_DEFINE)&command_setseason},
	{"SYSM",			CMD_FUNC,   (CMD_DEFINE)&command_sysm},
	{"SETHOME",			CMD_TARGETXYZ,	(CMD_DEFINE)&target_sethome},
	{"SETWORK",			CMD_TARGETXYZ,	(CMD_DEFINE)&target_setwork},
	{"SETFOOD",			CMD_TARGETXYZ,	(CMD_DEFINE)&target_setfood},	
	{"PASSWORD",		CMD_FUNC,	(CMD_DEFINE)&command_password},
	{"DEBUG",			CMD_FUNC,   (CMD_DEFINE)&command_debug},
	{NULL,				0,			NULL}			// Tseramed, cleaner
};

/* Actual commands go below. Define all the dynamic command_ functions as
* the name of the function in lowercase, a void function, accepting one
* integer, which is the player # that sent the command.
* Like this:
* void command_rain(int s)
*
* If you stick comments (double-slash comments) between the function definition
* and the opening block, they will automatically be used as the
* HTML documentation for that command. The first line will be used as the
* short documentation, any subsequent lines will be used as a seperate
* "more info" page for your command. HTML is allowed, but remember that
* everything is fit into a template.
*/

// When player has been murdered and they are 
// a ghost, they can use the bounty command to 
// place a bounty on the murderers head

void command_bounty(UOXSOCKET s)
{
	// Check if boountys are active
	
	P_CHAR pc_cs = currchar[s];
	if (pc_cs == NULL) return;
	
	if( !SrvParams->bountysactive() )
	{
		sysmessage(s, tr("The bounty system is not active."));
		return;
	}
	
	if( !pc_cs->dead )
	{
		sysmessage(s, tr("You can only place a bounty while you are a ghost."));
		pc_cs->setMurdererSer(INVALID_SERIAL);
		return;
	}
	
	if( pc_cs->murdererSer() == INVALID_SERIAL )
	{
		sysmessage(s, tr("You can only place a bounty once after someone has murdered you."));
		return;
	}
	
	if( tnum == 2 )
	{
		int nAmount = makenumber(1);
		if( Bounty->BountyWithdrawGold( pc_cs, nAmount ) )
		{
			if( Bounty->BountyCreate( pc_cs->murdererSer(), nAmount ) )
			{
				sysmessage( s, tr("You have placed a bounty of %1 gold coins on %2.").arg(nAmount).arg(FindCharBySerial(pc_cs->murdererSer())->name.c_str() ));
			}
			else
				sysmessage( s, tr("You were not able to place a bounty (System Error)") );
			
			// Set murdererSer to 0 after a bounty has been 
			// placed so it can only be done once
			pc_cs->setMurdererSer(INVALID_SERIAL);
		}
		else
			sysmessage( s, tr("You do not have enough gold to cover the bounty."));
		
	}
	else
		sysmessage(s, tr("To place a bounty on a murderer, use BOUNTY <amount>"));	
	
	return;
}


void command_serversleep(UOXSOCKET s)
{
	int seconds,a;
	
	if (tnum==2)
	{
		seconds = makenumber(1);
		sprintf((char*)temp,"server is going to sleep for %i seconds!",seconds);
		
		sysbroadcast((char*)temp); // broadcast server sleep
		for (a=0; a<now; a++) // make sure all send buffers are flushed (=the messages are really send beffore server sleeps)
		{
			//if (perm[a]) cNetwork::instance()->FlushBuffer(a);
		}
		seconds=seconds*1000; 
		
		Sleep(seconds);
		
		sprintf((char*)temp,"server is back from a %i second break",seconds/1000);
		
		sysbroadcast((char*)temp); 
		for (a=0; a<now; a++) // not absolutely necassairy ..
		{
			//if (perm[a]) cNetwork::instance()->FlushBuffer(a);
		}
		
	}
	else sysmessage(s, tr("invalid number of arguments"));
	
}


void command_reloadcachedscripts(UOXSOCKET s)
{
	DefManager->reload();
	ScriptManager->reload();
	Magic->reload();

	loadcustomtitle();
	cAllTerritories::getInstance()->reload();
	cAllSpawnRegions::getInstance()->reload();
	Commands->loadPrivLvlCmds();
	loadskills();
	read_in_teleport(); // hope i've cought all  ...
	cNetwork::instance()->load();
	
	sysmessage(s, tr("Cached scripts reloaded"));
}

// Returns the current bulletin board posting mode for the player
void command_post(UOXSOCKET s)
{
	P_CHAR pc_cs = currchar[s];
	if (pc_cs) return;
	
	switch( pc_cs->postType() )
	{
	case LOCALPOST:
		sysmessage( s, tr("You are currently posting a message to a single board [LOCAL].") );
		break;
		
	case REGIONALPOST:
		sysmessage( s, tr("You are currently posting a message to all boards in this area [REGIONAL].") );
		break;
		
	case GLOBALPOST:
		sysmessage( s, tr("You are currently posting a message to all boards in the world [GLOBAL].") );
		break;
		
	default:
		sysmessage( s, tr("You are currently posting an unknown message type. Setting to normal [LOCAL].") );
		pc_cs->setPostType(LOCALPOST);
	}
	
	return;
}

// Sets the current bulletin board posting mode for the player to GLOBAL
// ALL bulletin boards will see the next message posted to any bulletin board
void command_gpost(UOXSOCKET s)
{
	P_CHAR pc_cs = currchar[s];
	if (pc_cs == NULL) return;
	
	pc_cs->setPostType(GLOBALPOST);
	sysmessage( s, tr("Now posting GLOBAL messages.") );
	return;
}

// Sets the current bulletin board posting mode for the player to REGIONAL
// All bulletin boards in the same region as the board posted to will show the
// next message posted
void command_rpost(UOXSOCKET s)
{
	
	P_CHAR pc_cs = currchar[s];
	if (pc_cs == NULL) return;
	
	pc_cs->setPostType(REGIONALPOST);
	sysmessage( s, tr("Now posting REGIONAL messages.") );
	return;
}

// Sets the current bulletin board posting mode for the player to LOCAL
// Only this bulletin board will have this post
void command_lpost(UOXSOCKET s)
{
	P_CHAR pc_cs = currchar[s];
	if (pc_cs == NULL) return;
	
	pc_cs->setPostType(REGIONALPOST);
	sysmessage( s, tr("Now posting LOCAL messages.") );
	return;
}

// taken from 6904t2(5/10/99) - AntiChrist
void command_setmurder(UOXSOCKET s)
{
	if( tnum == 2 )
	{
		addmitem[s]=makenumber(1);
		target(s, 0, 1, 0, 240, "Select the person to set the murder count of: ");
	}
}

/*
void command_setacct(UOXSOCKET s)
{

  }
  
	void command_addacct(UOXSOCKET s)
	{
	
	  }
	  
		void command_banacct(UOXSOCKET s)
		{
		
		  }
		  
			void command_unbanacct(UOXSOCKET s)
			{
			
			  }
			  
				void command_removeacct(UOXSOCKET s)
				{
				
}*/


// bug clearing if players get account already in use for no reason.
// that bug should be gone in 11.9 but I got a bit paranoid bout it. LB
void command_letusin(UOXSOCKET s)
{
	int a,x;
	for (a=0,x=0; a<MAXCLIENT; a++) // maxclient instead of now is essential here !
	{
		if ( acctno[a]>=0 )
		{
			Accounts->SetOffline(acctno[a]);
			x++;
		}
	}
	
	sysmessage(s, tr("command successfull, cleared %1 poor souls").arg(x));
}

void command_readaccounts(UOXSOCKET s)
{
	Accounts->LoadAccounts( false );
	sysmessage(s,tr("Accounts reloaded...attention, if you changed exisiting(!) account numbers you should use the letusin command afterwards "));
}

void command_showp(UOXSOCKET s)
// Displays hex values of your PRIVLVL settings.
{
    PC_CHAR pcc_cs = currchar[s];
    if (pcc_cs == NULL) return;
	
	sysmessage(s, tr("privlvl : ").arg(pcc_cs->privlvl()));
}

void command_setprivlvl(UOXSOCKET s)
{
	attachTargetRequest( s, new cSetPrivLvlTarget( Commands->GetAllParams() ) );
}

void command_resend(UOXSOCKET s)
// Resends server data to client.
{
	all_items(s);
	LogMessage("ALERT: all_items() called in command_resend().\nThis function could cause a lot of lag!\n");
	teleport((currchar[s]));
	return;
}

void command_teleport(UOXSOCKET s)
{
	teleport((currchar[s])); 
	return;
}

void command_where(UOXSOCKET s)
// Prints your current coordinates+region.
// added region-name too, LB
{
	PC_CHAR pcc_cs = currchar[s];

    if (pcc_cs == NULL) 
		return;

	cTerritory* Region = cAllTerritories::getInstance()->region( pcc_cs->pos.x, pcc_cs->pos.y );

	if( Region != NULL )
		sysmessage( s, tr("You are at: %1").arg(Region->name()) ); 
	else 
		sysmessage(s, tr("You are at: unknown area"));
	
	sprintf((char*)temp, "%i %i (%i)",pcc_cs->pos.x,pcc_cs->pos.y,pcc_cs->pos.z); 
	sysmessage(s,(char*)temp);
	return;
}

void command_q(UOXSOCKET s)
{
	P_CHAR pc_cs = currchar[s];
	
	if (!pc_cs->isGM()) //They are not a GM
	{
		Commands->ShowGMQue(s, 0);
	} else {
		Commands->ShowGMQue(s, 1); // They are a GM
	}
	return;
}


void command_next(UOXSOCKET s)
// For Counselors or GM's, goes to next call in queue.
{
	P_CHAR pc_cs = currchar[s];
	
	if (!pc_cs->isGM()) //They are not a GM
	{
		Commands->NextCall(s, 0);
	}
	else
	{
		Commands->NextCall(s, 1); // They are a GM
	}
	return;
}

void command_clear(UOXSOCKET s)
// For Counselor's and GM's, removes current call from queue.
{
	P_CHAR pc_cs = currchar[s];
	
	if (!pc_cs->isGM()) //They are not a GM
	{
		donewithcall(s, 0);
	}
	else
	{
		donewithcall(s, 1); // They are a GM
	}
	return;
	
}

void command_goplace(UOXSOCKET s) 
// (d) Teleports you to a location from the LOCATIONS.SCP file. 
{ 
	P_CHAR pc_cs = currchar[s]; 
	
	if (tnum==2) 
	{ 
		Commands->MakePlace(s, makenumber(1)); 
		if (addx[s]!=0) 
		{ 
			////////////////////////////////// 
			// Adding the gmmove effects..Aldur 
			doGmMoveEff(s); 
			
			pc_cs->MoveTo(addx[s],addy[s],addz[s]); 
			teleport((currchar[s])); 
			
			doGmMoveEff(s); 
		} 
	} 
	return;
}

void command_gochar(UOXSOCKET s) 
// (h h h h) Teleports you to another character. 
{ 
	P_CHAR pc_cs = currchar[s]; 
	PC_CHAR pc_i; 
	
	int i; 
	if (tnum == 5) 
	{ 
		AllCharsIterator iter_char;
		for (iter_char.Begin(); !iter_char.atEnd(); iter_char++) 
		{ 
			pc_i = iter_char.GetData(); 
			
			if (pc_i != NULL) 
			{ 
				if (pc_i->serial == calcserial(hexnumber(1), hexnumber(2), hexnumber(3), hexnumber(4))) 
				{ 
					////////////////////////////////// 
					// Adding the gmmove effects ..Aldur
					// 
					// 
					doGmMoveEff(s); 
					
					pc_cs->moveTo(pc_i->pos); 
					teleport((currchar[s])); 
					
					doGmMoveEff(s); 
					
					break;
				} 
			} 
		} 
	} 
	else if (tnum == 2) 
	{ 
		for (i = 0; i < now; i++) 
		{ 
			if (i == makenumber(1) && perm[i]) 
			{ 
				pc_i = currchar[i]; 
				
					////////////////////////////////// 
					// Adding the gmmove effects ...Aldur
					// 
					// 
					doGmMoveEff(s); 
					
					pc_cs->moveTo(pc_i->pos); 
					teleport((currchar[s])); 
					
					doGmMoveEff(s); 
					
					break; 
			} 
		} 
	} 
	return; 
}

void command_fix(UOXSOCKET s)
// Try to compensate for messed up Z coordinates. Use this if you find yourself half-embedded in the ground.
{
	P_CHAR pc_cs = currchar[s];
	
	if (tnum == 2)
	{
		if (validtelepos(s)==-1)
		{
			pc_cs->dispz = pc_cs->pos.z = makenumber(1);
		}
		else
		{
			pc_cs->dispz = pc_cs->pos.z = validtelepos(s);
		}
		teleport((currchar[s]));
	}
	return;
}

void command_xgoplace(UOXSOCKET s)
// (d) Send another character to a location in your LOCATIONS.SCP file.
{
	if (tnum==2)
	{
		Commands->MakePlace(s, makenumber(1));
		if (addx[s]!=0)
		{
			target(s, 0, 1, 0, 8, "Select char to teleport.");
		}
	}
	return;
	
}

void command_showids(UOXSOCKET s)
// Display the serial number of every item on your screen.
{
	P_CHAR pc_currchar = currchar[s];
	
	//Char mapRegions
	int getcell=mapRegions->GetCell(pc_currchar->pos);
	cRegion::raw vecEntries = mapRegions->GetCellEntries(getcell);
	cRegion::rawIterator it = vecEntries.begin();
	for (; it != vecEntries.end(); ++it )
	{
		P_CHAR mapchar = FindCharBySerial(*it);
		if ( mapchar != NULL)
		{
			// TODO: FIXME
			//if (inrange1p(currchar[s], mapchar)) 
			//	mapchar->showName( s );
		}
	}
	return;
	
}

void command_poly(UOXSOCKET s)
// (h h) Polymorph yourself into any other creature.
{
	if (tnum==3)
	{
		int k,c1,b;
		P_CHAR pc_currchar = currchar[s];
		k=(hexnumber(1)<<8)+hexnumber(2);
		if (k>=0x000 && k<=0x3e1) // lord binary, body-values >0x3e crash the client
			
		{ 
			pc_currchar->setId(k);
			pc_currchar->xid = pc_currchar->id();
			
			c1 = pc_currchar->skin(); // transparency for mosnters allowed, not for players, 
			// if polymorphing from monster to player we have to switch from transparent to semi-transparent
			// or we have that sit-down-client crash
			b=c1&0x4000; 
			if (b==16384 && (k >=0x0190 && k<=0x03e1))
			{
				if (c1!=0x8000)
				{
					pc_currchar->setSkin(0xF000);
					pc_currchar->setXSkin(0xF000);
				}
			}
			
		}
	}
	
	teleport((currchar[s]));
	return;
	
}

void command_skin(UOXSOCKET s)
// (h h) Change the hue of your skin.
{
	if (tnum == 3)
	{
		int k, b, body;
		
		P_CHAR pc_currchar = currchar[s];
		
		body = pc_currchar->id();
		k = (hexnumber(1) << 8) + hexnumber(2);
		b = k&0x4000; 
		if (b == 16384 &&(body >=0x0190 && body <= 0x03e1))
			k = 0xf000; 
		
		if (k != 0x8000)
		{	
			pc_currchar->setSkin(k);
			pc_currchar->setXSkin( k );
			teleport((currchar[s]));
		}
	}
	return;
}

void command_action(UOXSOCKET s)
// (h) Preform an animated action sequence.
{
	if (tnum==2) action(s, hexnumber(1));
	return;
}

void command_setseason(UOXSOCKET s)
// Sets the season globally
// Season change packet structure:
// BYTE 0xBC	(Season change command)
// BYTE season	(0 spring, 1 summer, 2 fall, 3 winter, 4 dead, 5 unknown (rotating?))
// BYTE unknown	If 0, cannot change from undead, so 1 is the default
{
	int i;
	char setseason[4]="\xBC\x00\x01";
	
	if(tnum==2)
	{
		setseason[1]=hexnumber(1);
		SrvParams->setSeason( setseason[1] );
		for (i=0;i<now;i++) if (perm[i]) Xsend(i,setseason,3);
	}
	else
		sysmessage(s, tr("Setseason takes one argument."));	
}

void command_xtele(UOXSOCKET s)
// (d / h h h h / nothing) Teleport a player to your position.
// <UL><LI>If you specify nothing (/XTELE), you click on the player to teleport in.</LI>
// <LI>If you specify a serial number (/XTELE .. .. .. ..), you teleport that player to you.</LI>
// <LI>If you specify a single number (/XTELE ..), you teleport the player logged in
// under that slot to you.</LI>
// </UL>
{
	if (tnum==5 || tnum==2) Targ->XTeleport(s, tnum);
	else
	{
		target(s, 0, 1, 0, 136, "Select char to teleport to your position.");
	}
	return;
}

void command_go(UOXSOCKET s) 
// (d d d) Go to the specified X/Y/Z coordinates 
{ 
	if (tnum==4) 
	{ 
		int x=makenumber(1);int y=makenumber(2);int z=makenumber(3); // LB 
		if (x<6144 && y<4096 && z>-127 && z<128) 
		{ 
			doGmMoveEff(s); 	// Adding the gmmove effects ..Aldur
			P_CHAR pc_currchar = currchar[s];
			
			pc_currchar->MoveTo(x,y,z); 
			teleport((currchar[s])); 
			
			doGmMoveEff(s); 
		} 
	} 
	return; 
}

void command_zerokills(UOXSOCKET s)
// Sets all PK counters to 0.
{
	sysmessage(s, tr("Zeroing all player kills..."));
	AllCharsIterator iter_char;
	for (iter_char.Begin(); !iter_char.atEnd(); iter_char++)
	{
		P_CHAR pc = iter_char.GetData();
		pc->kills=0;
		setcharflag(pc);//AntiChrist
	}		
	sysmessage(s, tr("All player kills are now 0."));
}

void command_tile(UOXSOCKET s)
// (h h) Tiles the item specified over a square area.
// To find the hexidecimal ID code for an item to tile,
// either create the item with /add or find it in the
// world, and get /ISTATS on the object to get it's ID
// code.
{
	if (tnum==3) {
		addid1[s]=hexnumber(1);
		addid2[s]=hexnumber(2);
		clickx[s]=-1;
		clicky[s]=-1;
		target(s,0,1,0,198,"Select first corner of bounding box.");  // 198 didn't seem taken...
	}
	return;
}

void command_wipe(UOXSOCKET s)
// (d d d d / nothing) Deletes ALL NPC's and items inside a specified square.
// <UL><LI>With no arguments, /WIPE will ask you to click in opposing corners of
// the square.</LI>
// <LI>You may also specify coordinates - X1, Y1, X2, Y2.</LI>
// </UL>
{
	addid1[s]=0; // addid1[s]==0 is used to denote a true wipe
	if (tnum==1) {
		clickx[s]=-1;
		clicky[s]=-1;
		target(s,0,1,0,199,"Select first corner of wiping box.");  // 199 didn't seem taken...
	} else if( tnum == 2 ) {
		// Really should warn that this will wipe ALL objects...
		if( Commands->params[ 1 ].lower().compare( "all" ) )
			Commands->Wipe(s);
	}
	else if (tnum==5) { // Wipe according to world coordinates
		clickx[s]=makenumber(1);
		clicky[s]=makenumber(2);
		buffer[s][11]=makenumber(3)>>8;buffer[s][12]=makenumber(3)%256; // Do NOT try this at home, kids!
		buffer[s][13]=makenumber(4)>>8;buffer[s][14]=makenumber(4)%256;
		Targ->Wiping(s);
	}
	
	return;
}

void command_iwipe(UOXSOCKET s)
// (d d d d / nothing) Deletes ALL NPC's and items NOT inside a specified square.
// <UL><LI>With no arguments, /IWIPE will ask you to click in opposing corners of
// the square.</LI>
// <LI>You may also specify coordinates - X1, Y1, X2, Y2.</LI>
// </UL>
{
	addid1[s]=1;  // addid1[s]==1 is used to denote the INVERSE wipe
	if (tnum==1) {
		clickx[s]=-1;
		clicky[s]=-1;
		target(s,0,1,0,199,"Select first corner of inverse wiping box.");  // 199 didn't seem taken...
	}
	else if (tnum==5) { // Wipe according to world coordinates
		clickx[s]=makenumber(1);
		clicky[s]=makenumber(2);
		buffer[s][11]=makenumber(3)>>8;buffer[s][12]=makenumber(3)%256; // Do NOT try this at home, kids!
		buffer[s][13]=makenumber(4)>>8;buffer[s][14]=makenumber(4)%256;
		Targ->Wiping(s);
	}
	
	return;
	
}

void command_add(UOXSOCKET s)
// (h h) Adds a new item, or opens the GM menu if no hex codes are specified.
{
	if (tnum==2 || tnum>3)//AntiChrist
	{
		sysmessage(s, tr("Sintax error. Usage: /add <id1> <id2>"));
		return;
	} else if (tnum==3)
	{
		addid1[s]=hexnumber(1);
		addid2[s]=hexnumber(2);
		if (addid1[s]<0x40)
		{
			target(s, 0, 1, 0, 0, "Select location for item.");
		}
	} else if (tnum==1)
	{
		ShowMenu(s, 1);
	}
	return;
}

void command_addx(UOXSOCKET s)
// (h h) Adds a new item to your current location.
{
	if (tnum==3)
	{
		addid1[s] = (unsigned char) hexnumber(1);
		addid2[s] = (unsigned char) hexnumber(2);
		P_CHAR pc_currchar = currchar[s];
		Commands->AddHere(s, pc_currchar->pos.z);
	}
	if (tnum==4)
	{
		addid1[s] = (unsigned char) hexnumber(1);
		addid2[s] = (unsigned char) hexnumber(2);
		Commands->AddHere(s, makenumber(3));
	}
	return;
}

void command_rename(UOXSOCKET s)
// (text) Renames any dynamic item in the game.
{
	if (tnum>1)
	{
		strcpy(xtext[s], Commands->GetAllParams().latin1());
		target(s, 0, 1, 0, 1, "Select item or character to rename.");
	}
}

void command_title(UOXSOCKET s)
// (text) Changes the title of any player or NPC.
{
	if (tnum>1)
	{
		strcpy(xtext[s], Commands->GetAllParams().latin1());
		target(s, 0, 1, 0, 47, "Select character to change the title of.");
	}
}


// Saves the world (from dr. evil?)
void command_save(UOXSOCKET s)
{
	if ( !Commands->GetAllParams().isEmpty() )
		cwmWorldState->savenewworld( Commands->GetAllParams().latin1() );
	else
		cwmWorldState->savenewworld( SrvParams->worldSaveModule() );
	SrvParams->flush();
	return;
}

void command_dye(UOXSOCKET s)
// (h h/nothing) Dyes an item a specific color, or brings up a dyeing menu if no color is specified.
{
	dyeall[s]=1;
	if (tnum==3)
	{
		addid1[s]=hexnumber(1);
		addid2[s]=hexnumber(2);
	}
	else
	{
		addid1[s]=255;
		addid2[s]=255;
	}
	target(s, 0, 1, 0, 4, "Select item to dye.");
	return;
	
}

void command_wtrig(UOXSOCKET s)
// (word) Sets the trigger word on an NPC.
{
	if (tnum>1)
	{
		P_CHAR pc_currchar = currchar[s];
		if(!pc_currchar->unicode())
			strcpy(xtext[s], &tbuffer[15]);
		else
			strcpy(xtext[s], &tbuffer[20]);
		target(s, 0, 1, 0, 203, "Select the NPC to set trigger word on.");
	}
}

void command_setpriv(UOXSOCKET s)
// (Chars/NPCs: h h, Objects: h) Sets priviliges on a Character or object.
{
	if (tnum==3)
	{
		addid1[s]=hexnumber(1);
		addid2[s]=hexnumber(2);
		target(s, 0, 1, 0, 9, "Select char to edit priv.");
	}
	if (tnum==2)
	{
		addid1[s]=3;
		addid2[s]=hexnumber(1);
		target(s, 0, 1, 0, 89, "Select object to edit priv.");
	}
	return;
}

void command_nodecay(UOXSOCKET s)
// Prevents an object from ever decaying.
{
	addid1[s]=0; 
	target(s, 0, 1, 0, 89, "Select object to make permenant.");
	return;
}


#ifdef UNRELEASED
void command_send(UOXSOCKET s)
// Debugging command
{
	int i;
	for (i=1;i<tnum;i++) temp[i-1]=hexnumber(i);
	//clConsole.send("Sending to client %i.\n",s);
	Xsend(s, temp, tnum-1);
	return;
}
#endif

void command_showtime(UOXSOCKET s)
// Displays the current UO time.
{
	sysmessage(s, (char*) tr("WOLFPACK Time: %1").arg(uoTime.toString()).latin1());
	return;
}

void command_settime(UOXSOCKET s)
// (d d) Sets the current UO time in hours and minutes.
{
	int newhours, newminutes;
	if (tnum==3)
	{
		newhours = makenumber(1);
		newminutes = makenumber(2);
		if ((newhours < 25) && (newhours > 0) && (newminutes > -1) && (newminutes <60))
		{
			uoTime.time().setHMS(newhours, newminutes, 0);
		}
	}
	return;
}

void command_shutdown(UOXSOCKET s)
// (d) Shuts down the server. Argument is how many minutes until shutdown.
{
	if (tnum==2)
	{
		endtime=uiCurrentTime+(MY_CLOCKS_PER_SEC*makenumber(1));
		if (makenumber(1)==0)
		{
			endtime=0;
			sysbroadcast(tr("Shutdown has been interrupted."));
		}
		else endmessage(0);
	}
	else
		sysmessage(s, tr("Syntax: shutdown [seconds]"));
	return;
}

void command_wholist(UOXSOCKET s)
// Brings up an interactive listing of online users.
{
	whomenu(s, 4);
	return;
}

void command_playerlist(UOXSOCKET s)
{ // Same as wholist but has offline players...Ripper
	playermenu(s, 4);
	return;
}

#ifdef UNRELEASED
void command_blt2(UOXSOCKET s)
// Debugging command.
{
	if (tnum==4)
	{
		boltstring[1]=hexnumber(1);
		boltstring[2]=0;
		boltstring[3]=0;
		boltstring[4]=0;
		boltstring[5]=1;
		boltstring[10]=hexnumber(2);
		boltstring[11]=hexnumber(3);
		Xsend(s, boltstring, 28);
	}
	return;
	
}
#endif

void command_sfx(UOXSOCKET s)
// (h h) Plays the specified sound effect.
{
	if (tnum==3)
	{
		soundeffect(s, hexnumber(1), hexnumber(2));
	}
	return;
}

void command_light(UOXSOCKET s)
// (h) Sets the light level. 0=brightest, 15=darkest, -1=enable day/night cycles.
{
	if (tnum==2)
	{
		SrvParams->setWorldFixedLevel(hexnumber(1));
		if (SrvParams->worldFixedLevel() != 255) setabovelight(SrvParams->worldFixedLevel());
		else setabovelight(SrvParams->worldCurrentLevel());
	}
	return;
}

#ifdef UNRELEASED
void command_web(UOXSOCKET s)
// Debugging command.
{
	if (tnum>1)
	{
		strcpy(xtext[s], Commands->params[1].latin1());
		weblaunch(s, xtext[s]);
	}
	return;
}
#endif

void command_disconnect(UOXSOCKET s)
// (d) Disconnects the user logged in under the specified slot.
{
	//if (tnum==2) cNetwork::instance()->disconnect(makenumber(1));
	return;
}

void command_tell(UOXSOCKET s)
// (d text) Sends an anonymous message to the user logged in under the specified slot.
{
	if (tnum>2) 
	{ 
		QString dummy = Commands->GetAllParams();
		int m = makenumber( 1 );
		if( m < 0 ) 
			sysbroadcast( dummy.latin1() ); 
		else
			tellmessage( s, makenumber(1), dummy.latin1() );
	}
	return;
}

void command_dry(UOXSOCKET s)
// Set weather to dry (no rain or snow).
{
	sysmessage(s, tr("Being Worked On!\n"));
	/*	int i;
	wtype=0;
	for (i=0;i<now;i++) if (perm[i]) weather(i,0);*/
	return;
}

void command_rain(UOXSOCKET s)
// Sets the weather condition to rain.
{
	sysmessage(s, tr("Being Worked On!\n"));
	/*		int i;
	if (wtype==2)
	{
	wtype=0;
	for (i=0;i<now;i++) if (perm[i]) weather(i,0);
	}
	wtype=1;
	for (i=0;i<now;i++) if (perm[i]) weather(i,0); */
	
	return;
}

void command_snow(UOXSOCKET s)
// Sets the weather condition to snow.
{
	sysmessage(s, tr("Being Worked On!\n"));
	/*	int i;
	if (wtype==1)
	{
	wtype=0;
	for (i=0;i<now;i++) if (perm[i]) weather(i,0);
	}
	wtype=2;
	for (i=0;i<now;i++) if (perm[i]) weather(i,0);*/
	return;
	
}

void command_gmmenu(UOXSOCKET s)
// (d) Opens the specified GM Menu.
{
	if (tnum==2) ShowMenu(s, makenumber(1));
	return;
}

void command_itemmenu(UOXSOCKET s)
// (d) Opens the specified Item Menu from ITEMS.SCP.
{
	if (tnum==2) ShowMenu(s, makenumber(1));
	return;
	
}

void command_additem(UOXSOCKET s)
// (d) Adds the specified item from ITEMS.SCP.
{
	if( tnum >= 2 )
	{
		strcpy(xtext[s], Commands->GetAllParams().latin1());
		sprintf( (char*)temp, "Select location for item %s", xtext[ s ] );
		target( s, 0, 1, 0, 26, (char*)temp );
	}

	return;	
}

void command_dupe(UOXSOCKET s)
// (d / nothing) Duplicates an item. If a parameter is specified, it's how many copies to make.
{
	if (tnum==2)
	{
		addid1[s]=makenumber(1);
		target(s, 0, 1, 0, 110, "Select an item to dupe.");
	}
	else
	{
		addid1[s]=1;
		target(s, 0, 1, 0, 110, "Select an item to dupe.");
	}
	return;
	
}


void command_command(UOXSOCKET s)
// Executes a trigger scripting command.
{
	int i;
	if (tnum>1)
	{
		i=0;
		script1[0]=0;
		script2[0]=0;
		strcpy((char*)script1, Commands->params[1].latin1());
		if (Commands->params.size() > 2)
			strcpy((char*)script2, Commands->params[2].latin1());
		scriptcommand(s, script1, script2);
	}
	return;
}

void command_gcollect(UOXSOCKET s)
// Runs garbage collection routines.
{
	gcollect();
	sysmessage(s, tr("command succesfull"));
	return;
}

void command_allmoveon(UOXSOCKET s)
// Enables GM ability to pick up all objects.
{
	P_CHAR pc_currchar = currchar[s];
	pc_currchar->priv2 |= 0x01;
	teleport((currchar[s]));
	sysmessage(s, tr("ALLMOVE enabled."));
	return;
	
}

void command_allmoveoff(UOXSOCKET s)
// Disables GM ability to pick up all objects.
{
	P_CHAR pc_currchar = currchar[s];
	pc_currchar->priv2 &= (0xFF-0x01);
	teleport((currchar[s]));
	sysmessage(s, tr("ALLMOVE disabled."));
	return;
	
}

void command_showhs(UOXSOCKET s)
// Makes houses appear as deeds. (The walls disappear and there's a deed on the ground in their place.)
{
	P_CHAR pc_currchar = currchar[s];
	pc_currchar->priv2 |= 0x04;
	teleport((currchar[s]));
	sysmessage(s, tr("House icons visible. (Houses invisible)"));
	return;
	
}

void command_hidehs(UOXSOCKET s)
// Makes houses appear as houses (opposite of /SHOWHS).
{
	P_CHAR pc_currchar = currchar[s];
	pc_currchar->priv2 &= (0xFF-0x04);
	teleport((currchar[s]));
	sysmessage(s, tr("House icons hidden. (Houses visible)"));
	return;
}


void command_set(UOXSOCKET s)
// (text, d) Set STR/DEX/INT/Skills on yourself arguments are skill & amount.
{
	// DISABLED BY DARKSTORM FOR EVENT TESTING STUFF
	// /SET EVENTS +A,-B
	QString commandLine = Commands->GetAllParams().latin1();

	SocketStrings[ s ] = commandLine.ascii();

	char message[512];
	sprintf( message, "Choose target to set '%s'", commandLine.ascii() );
	target(s, 0, 1, 0, 36, message );	
	
	//QStringList parameters = QStringList::split( QChar( " " ) );

/*	int i;
	if (tnum == 3)
	{
		i=0;
		script1[0]=0;
		strcpy((char*)script1, Commands->params[1].c_str());
		strupr((char*)script1);
		addx[s]=-1;
		
		for (i=0;i<SKILLS;i++)
		{
			if (!(strcmp(skillname[i], (char*)script1))) { addx[s]=i; }
		}
		if (addx[s]!=-1)
		{
			addy[s] = makenumber(2);
#if !defined(TESTCENTER)
			target(s, 0, 1, 0, 36, "Select character to modify.");
#else
			P_CHAR pc = currchar[s];
			if(pc != NULL)
			{
				register unsigned int j;

				switch ( addx[s] )
				{
				case FAME:	pc->fame = addy[s];		return;
				case KARMA: pc->karma = addy[s];	return;
				case ALLSKILLS:
					for ( j = 0; j < TRUESKILLS; ++j )
						pc->baseskill[j] = max(0, min( addy[s], 1000 ) );
					break;
				case STR:
					pc->st = max(0, min( addy[s], 100 ) );
					statwindow(s, pc);
					break;
				case INT:
					pc->in = max(0, min( addy[s], 100 ) );
					statwindow(s, pc);
					break;
				case DEX:
					pc->setDex( max(0, min( addy[s], 100 ) ) );
					statwindow(s, pc);
					break;
				default: // one of the skills.
					pc->baseskill[addx[s]] = max(0, min( addy[s], 1000 ) );
					Skills->updateSkillLevel( pc, addx[s] );
					updateskill(s, addx[s]);
					return; // only one skill gets changed.
				}
				for (j = 0; j < TRUESKILLS; ++j)
				{
					Skills->updateSkillLevel(pc, j);
					updateskill(s, j);
				}
			}
#endif
		}
	}
	return;*/
}

#ifdef UNRELEASED
void command_temp(UOXSOCKET s)
// Debugging command.
{
	if(tnum==2) {
		tempflag=hexnumber(1);
		sysmessage(s, "TEMP changed.");
	}
	return;
	
}
#endif

void command_addnpc(UOXSOCKET s)
// (d or h h) Add the specified NPC from NPC.SCP or by hex ID code.
{
	sysmessage( s, QString( "Where do you want to create the '%1'" ).arg( Commands->GetAllParams() ) );
	attachTargetRequest( s, new cAddNpcTarget( Commands->GetAllParams() ) );
}

void command_readini(UOXSOCKET s)
// Re-loads the WOLFPACK.INI file.
{
	SrvParams->reload();
	sysmessage(s, tr("wolfpack.xml file reloaded."));
	return;
}

void command_gumpmenu(UOXSOCKET s)
// (d) Opens the specified GUMP menu.
{
	if (tnum==2)
	{
		cGumps::instance()->Menu( s, makenumber( 1 ), NULL );
	}
	return;
	
}

void command_npcrect(UOXSOCKET s)
// (d d d d) Set bounding box for a NPC with a NPCWANDER of 3.
{
	if (tnum==5)
	{
		addx[s]=makenumber(1); // bugfix, LB, old npcshape worked only if its only excuted by ONE player at the same time
		addy[s]=makenumber(2);
		addx2[s]=makenumber(3);
		addy2[s]=makenumber(4);
		target(s, 0, 1, 0, 67, "Select the NPC to set the bounding rectangle for."); // lb bugfix, was 58 ...
	}
	return;
	
}

void command_npccircle(UOXSOCKET s)
// (d d d) Set bounding circle for a NPC with a NPCWANDER of 2.
{
	if (tnum==4)
	{
		addx[s]=makenumber(1);
		addy[s]=makenumber(2);
		addx2[s]=makenumber(3);
		target(s, 0, 1, 0, 59, "Select the NPC to set the bounding circle for.");
	}
	return;
	
}

void command_npcwander(UOXSOCKET s)
// (d) Sets the type of wandering a NPC does.
// <UL><LI>0 = NPC Does not move.</LI>
// <LI>1 = NPC Follows specified target. (See
// <A HREF="npctarget.html">/NPCTARGET</A>)</LI>
// <LI>2 = NPC Wanders Freely.</LI>
// <LI>3 = NPC stays in box specified by <A HREF="npcrect.html">NPCRECT</A>.</LI>
// <LI>4 = NPC stays in circle specified by <A HREF="npccircle.html">NPCCIRCLE</A>.</LI></UL>
{
	if (tnum==2)
	{
		npcshape[0]=makenumber(1);
		target(s, 0, 1, 0, 60, "Select the NPC to set the wander method for.");
	}
	return;
	
}

void command_secondsperuominute(UOXSOCKET s)
// (d) Sets the number of real-world seconds that pass for each UO minute.
{
	if (tnum==2)
	{
		SrvParams->setSecondsPerUOMinute( makenumber(1) );
		sysmessage(s, tr("Seconds per UO minute set."));
	}
	return;
	
}

void command_brightlight(UOXSOCKET s)
// (h) Sets default daylight level.
{
	if (tnum==2)
	{
		SrvParams->setWorldBrightLevel( hexnumber(1) );
		sysmessage(s, tr("World bright light level set."));
	}
	return;
	
}

void command_darklight(UOXSOCKET s)
// (h) Sets default nighttime light level.
{
	if (tnum==2)
	{
		SrvParams->setWorldDarkLevel(hexnumber(1));
		sysmessage(s, tr("World dark light level set."));
	}
	return;
	
}

void command_dungeonlight(UOXSOCKET s)
// (h) Sets default dungeon light level.
{
	if (tnum==2)
	{
		SrvParams->setDungeonLightLevel( min(hexnumber(1), 27) );
		sysmessage(s, tr("Dungeon light level set."));
	}
	return;
	
}

void command_gmopen(UOXSOCKET s)
// (h / nothing) Opens specified layer on player/NPC, or player's pack if no layer specified.
// <P>Useful hex codes for this command are:</P>
// <TABLE BORDER=1>
// <TR><TD><B>15</B></TD><TD>Backpack</TD></TR>
// <TR><TD><B>1A</B></TD><TD>NPC Buy Restock container</TD></TR>
// <TR><TD><B>1B</B></TD><TD>NPC Buy no restock container</TD></TR>
// <TR><TD><B>1C</B></TD><TD>NPC Sell container</TD></TR>
// <TR><TD><B>1D</B></TD><TD>Bank Box</TD></TR>
// </TABLE>
{
	if (tnum==2) addmitem[s]=hexnumber(1);
	else addmitem[s]=0x15;
	target(s, 0, 1, 0, 115, "Select the character to open the container on.");
	return;
	
}

void command_restock(UOXSOCKET s)
// Forces a manual vendor restock.
{
	Trade->restock(0);
	sysmessage(s, tr("Manual shop restock has occurred."));
	return;
	
}

void command_restockall(UOXSOCKET s)
// Forces a manual vendor restock to maximum values.
{
	Trade->restock(1);
	sysmessage(s, tr("Restocking all shops to their maximums"));
	return;
	
}

void command_setshoprestockrate(UOXSOCKET s)
// (d) Sets the universe's shop restock rate.
{
	if (tnum==2)
	{
		shoprestockrate=makenumber(1);
		sysmessage(s, tr("NPC shop restock rate changed."));
	}
	else sysmessage(s, tr("Invalid number of parameters."));
	return;
	
}

void command_midi(UOXSOCKET s)
// (d d) Plays the specified MIDI file.
{
	if (tnum==3) playmidi(s, makenumber(1), makenumber(2));
	return;
	
}

void command_gumpopen(UOXSOCKET s)
// (h h) Opens the specified GUMP menu.
{
	if( tnum == 3 ) 
		cGumps::instance()->Open( s, currchar[ s ], makenumber( 1 ), makenumber( 2 ) );

	return;
	
}

void command_respawn(UOXSOCKET s)
// Forces a respawn.
{
	P_CHAR pc_currchar = currchar[s];
	sysbroadcast("World is now respawning, expect some lag!");
	clConsole.log( QString( "Respawn command called by %1.\n" ).arg( pc_currchar->name.c_str() ).latin1() );

	cAllSpawnRegions::getInstance()->reSpawn();
	return;
}

void command_regspawnmax(UOXSOCKET s)
// (d) Spawns in all regions up to the specified maximum number of NPCs/Items.
{
	if (tnum==2)
	{
		P_CHAR pc_currchar = currchar[s];
		sprintf((char*)temp,"MAX Region Respawn command called by %s.\n", pc_currchar->name.c_str());//AntiChrist
		LogMessage((char*)temp);
		Commands->RegSpawnMax(s, Commands->GetAllParams() );
		return;
	}
	
}

void command_regspawn(UOXSOCKET s)
// (d d) Preforms a region spawn. First argument is region, second argument is max # of items/NPCs to spawn in that region.
{
	if (tnum==3)
	{
		QStringList Params = QStringList::split( " ", Commands->GetAllParams() );
		P_CHAR pc_currchar = currchar[s];
		sprintf((char*)temp,"Specific Region Respawn command called by %s.\n", pc_currchar->name.c_str());
		LogMessage((char*)temp);
		Commands->RegSpawnNum(s, Params[0], makenumber(2));
		return;
	}
	
}

void command_reloadserver(UOXSOCKET s)
// Reloads the SERVER.SCP file.
{
	SrvParams->reload();
	sysmessage(s, tr("wolfpack.xml reloaded."));
	return;
}

void command_cq(UOXSOCKET s)
// Display the counselor queue.
{
	Commands->ShowGMQue(s, 0); // Show the Counselor queue, not GM queue   
	return;
}

void command_cnext(UOXSOCKET s)
// Attend to the next call in the counselor queue.
{
	Commands->NextCall(s, 0); // Show the Counselor queue, not GM queue   
	return;
}

void command_cclear(UOXSOCKET s)
// Remove the current call from the counselor queue.
{
	donewithcall(s, 0); // Show the Counselor queue, not GM queue   
	return;
	
}

void command_minecheck(UOXSOCKET s)
// (d) Set the server mine check interval in minutes.
{
	int d;
	d = SrvParams->minecheck();
	if (tnum==2)
	d=makenumber(1);
	sprintf((char*)temp, "Minecheck changed to %i", d);
	sysmessage(s, (char*)temp);
	return;
}

void command_invul(UOXSOCKET s)
// Makes the targeted character immortal.
{
	addx[s]=1;
	target(s,0,1,0,179,"Select creature to make invulnerable.");
	
}

void command_noinvul(UOXSOCKET s)
// Makes the targeted character mortal.
{
	addx[s]=0;
	target(s,0,1,0,179,"Select creature to make mortal again.");
	
}

void command_guardson(UOXSOCKET s)
// Activates town guards.
{
	SrvParams->guardsActive(true);
	sysbroadcast("Guards have been reactivated.");
}

void command_guardsoff(UOXSOCKET s)
// Deactivates town guards.
{
	SrvParams->guardsActive(false);
	sysbroadcast("Warning: Guards have been deactivated globally.");
	
}

void command_announceon(UOXSOCKET s)
// Enable announcement of world saves.
{
	SrvParams->announceWorldSaves(true);
	cwmWorldState->announce(true);
	sysbroadcast("WorldStat Saves will be displayed.");
	
}

void command_announceoff(UOXSOCKET s)
// Disable announcement of world saves.
{
	SrvParams->announceWorldSaves( false );
	cwmWorldState->announce( false );
	sysbroadcast("WorldStat Saves will not be displayed.");
	
}

void command_wf(UOXSOCKET s)
// Make the specified item worldforge compatible.
{
	if (tnum==2)
	{
		addid1[s]=255;
		target(s, 0, 1, 0, 6, "Select item to make WorldForge compatible.");
	}
	return;
	
}

void command_decay(UOXSOCKET s)
// Enables decay on an object.
{
	addid1[s]=1;
	target(s, 0, 1, 0, 89, "Select object to decay.");
	return;
	
}

void command_killall(UOXSOCKET s)
// (d text) Kills all of a specified item.
{
	if(tnum>2)
	{
		//QString dummy( Commands->GetAllParams() );
		//dummy = dummy.substr(dummy.find_first_not_of(" "));
		QString text = Commands->GetAllParams();

		if( !text.contains( " " ) )
			text = "";
		else
			text = text.right( text.length() - text.find( " " ) );

		if(makenumber(1)<10)
			Commands->KillAll(s, makenumber(1), text.latin1());
		else if (makenumber(1)<100)
			Commands->KillAll(s, makenumber(1), text.latin1());
		else
			Commands->KillAll(s, makenumber(1), text.latin1());
	}
	return;
	
}

void command_pdump(UOXSOCKET s)
// Display some performance information.
{
	sysmessage(s, tr("Performace Dump:"));
	sysmessage(s, tr("Network code: %1msec [%2]").arg((float)((float)networkTime/(float)networkTimeCount)).arg(networkTimeCount));
	sysmessage(s, tr("Timer code: %1msec [%2]").arg((float)((float)timerTime/(float)timerTimeCount)).arg(timerTimeCount));
	sysmessage(s, tr("Auto code: %1msec [%2]").arg((float)((float)autoTime/(float)autoTimeCount)).arg(autoTimeCount));
	sysmessage(s, tr("Loop Time: %1msec [%2]").arg((float)((float)loopTime/(float)loopTimeCount)).arg(loopTimeCount));
	sysmessage(s, tr("Simulation Cycles/Sec: %1").arg((1000.0*(1.0/(float)((float)loopTime/(float)loopTimeCount)))));
	return;
}

void command_rename2(UOXSOCKET s)
// (text) Rename an item or character.
{
	 if (tnum>1)
	 {
		 addx[s]=1;
		 strcpy(xtext[s], Commands->GetAllParams().latin1());
		 target(s, 0, 1, 0, 1, "Select item or character to rename.");
	 } 
	 return;
}

void command_readspawnregions(UOXSOCKET s)
{
	cAllSpawnRegions::getInstance()->reload();
	sysmessage(s, tr("Spawnregions reloaded."));
	return;
	
}

void command_gy(UOXSOCKET s)
// (text) GM Yell - Announce a message to all online GMs.
{
	if(now==1) {
		sysmessage(s, tr("There are no other users connected."));
		return;
	}
	
	int i;
	short tl;
	P_CHAR pc_currchar = currchar[s];
	
	sprintf(xtext[s], "(GM ONLY): %s", Commands->GetAllParams().latin1());//AntiChrist bugfix - cms_offset+4, not +7
	tl=44+strlen(&xtext[s][0])+1;			
	
	ShortToCharPtr(tl, &talk[1]);
	LongToCharPtr(pc_currchar->serial, &talk[3]);
	ShortToCharPtr(pc_currchar->id(),  &talk[7]);
	talk[9]=1;
	talk[10]=buffer[s][4];
	talk[11]=buffer[s][5];
	talk[12]=buffer[s][6];
	talk[13]=pc_currchar->fonttype;
			 
	for (i=0;i<now;i++)
	{
		if (perm[i] && currchar[i]->isGM())
		{
			Xsend(i, talk, 14);
			Xsend(i, (void*)pc_currchar->name.c_str(), 30);
			Xsend(i, &xtext[s][0], strlen(&xtext[s][0])+1);   
		}
	}
}

void command_tilew(UOXSOCKET s)
// (h h d d d d d) id id x1 x2 y1 y2 z - Tile an object using specified id & coordinates.
// <UL><LI>The first two numbers are hexidecimal, and are the ID codes for
// the item to tile.</LI>
// <LI>The second pair of numbers is the decimal coordinates of the upper
// left hand corner of the box being tiled. To get this value, go to the
// upper left hand corner and type /WHERE.</LI>
// <LI>The third pair of numbers is the lower right hand corner of the
// box being tiled.</LI>
// <LI>The final number is the Z-Axis of the box being tiled.</LI></UL>
{
	if(tnum==8)
	{
		addid1[s] = static_cast<unsigned char>(hexnumber(1));//id1
		addid2[s] = static_cast<unsigned char>(hexnumber(2));//id2
		int pile=0;
		tile_st tile;
		Map->SeekTile((addid1[s]<<8)+addid2[s], &tile);
		if (tile.flag2&0x08) pile=1;
		for (int x=makenumber(3);x<=makenumber(4);x++)
		{
			for (int y=makenumber(5);y<=makenumber(6);y++)
			{
				P_ITEM pi_a = Items->SpawnItem(s, 1, "#", pile, addid1[s], addid2[s], 0, 0,0);
				if(pi_a != NULL) //AntiChrist - to preview crashes
				{
					pi_a->priv=0; //Make them not decay
					pi_a->pos.x=x;
					pi_a->pos.y=y;
					pi_a->pos.z = static_cast<signed char>(makenumber(7));
					mapRegions->Add(pi_a); // lord Binary
					RefreshItem(pi_a);//AntiChrist
				}
			}
			
			
			
		}
		addid1[s]=0; // lb, i was so free and placed it here so that we dont have y-1 rows of 0-id items ... hope that was not intentinal ..
		addid2[s]=0;
	}  
	else { sysmessage(s, tr("Format: /tilew ID1 ID2 X1 X2 Y1 Y2 Z")); }
	return;
	
}

void command_squelch(UOXSOCKET s)
// (d / nothing) Squelchs specified player. (Makes them unnable to speak.)
{
	if (tnum==2)
    {
		if (makenumber(1)!=0 || makenumber(1)!=-1)
        {
			addid1[s]=255;
            addid1[s]=makenumber(1);
        }
    }
    target(s,0,1,0,223,"Select character to squelch.");
    return;
}

void command_spawnkill(UOXSOCKET s)
// (d) Kills spawns from the specified spawn region in SPAWN.SCP.
{
	if (tnum==2)
	{
		Commands->KillSpawn(s, Commands->GetAllParams() );
	}
	return;
}


void command_wanim(UOXSOCKET s)
// (d d) Changes server lighting animation.
{
	if(tnum==3)
	{
		w_anim[0]=(signed char) hexnumber(1);
		w_anim[1]=(signed char) hexnumber(2);
		
		sysmessage(s, tr("new lightening animation set!"));
	}
}

void command_gotocur(UOXSOCKET s)
// Goes to the current call in the GM/Counsellor Queue
{
	int x=0;
	P_CHAR pc_currchar = currchar[s];
	
	if(pc_currchar->callnum()==0)
	{
		sysmessage(s, tr("You are not currently on a call."));
	}
	else
	{  
		int serial = gmpages[pc_currchar->callnum()].serial;
		P_CHAR pc_i = FindCharBySerial( serial );
		if(pc_i != NULL)
		{
			pc_currchar->moveTo(pc_i->pos);
			sysmessage(s, tr("Transporting to your current call."));
			teleport((currchar[s]));
			x++;
		}  
		if(x == 0)
		{
			SERIAL serial = gmpages[pc_currchar->callnum()].serial;
			P_CHAR pc_i = FindCharBySerial( serial );
			if(pc_i != NULL)
				
			{
				pc_currchar->moveTo(pc_i->pos);
				sysmessage(s, tr("Transporting to your current call."));
				teleport((currchar[s]));
			}
		}
	}
}

void command_gmtransfer(UOXSOCKET s)
// Escilate a Counsellor Page into the GM Queue
{
	int i;
	int x2=0;
	P_CHAR pc_currchar = currchar[s];
	
	if(pc_currchar->callnum()!=0)
	{
		if(!pc_currchar->isGM()) //Char is a counselor
		{
			for(i=1;i<MAXPAGES;i++)
			{
				if(gmpages[i].handled==1)
				{
					gmpages[i].handled=0;
					gmpages[i].name = counspages[pc_currchar->callnum()].name;
					gmpages[i].reason = counspages[pc_currchar->callnum()].reason;
					gmpages[i].serial = counspages[pc_currchar->callnum()].serial;
					strcpy(gmpages[i].timeofcall, QTime::currentTime().toString("hh:mm:ss").latin1());
					x2++;
					break;
				}
			}
			if (x2==0)
			{
				sysmessage(s, tr("The GM Queue is currently full. Contact the shard operator"));
				sysmessage(s, tr("and ask them to increase the size of the queue."));
			}
			else
			{
				sysmessage(s, tr("Call successfully transferred to the GM queue."));
				donewithcall(s,1);
			}
		}
		else
		{
			sysmessage(s, tr("Only Counselors may use this command."));
		}
	}
	else
	{
		sysmessage(s, tr("You are not currently on a call"));
	}
}

void command_who(UOXSOCKET s)
// Displays a list of users currently online.
{
	if(now==1) 
	{
		sysmessage(s, tr("There are no other users connected."));
		return;
	}
	
	int i, j=0;
	sysmessage(s, tr("Current Users in the World:"));
	for (i=0;i<now;i++)
	{
		if(perm[i]) //Keeps NPC's from appearing on the list
		{
			j++;
			sprintf((char*)temp, "%i) %s [%8x]", (j-1), currchar[i]->name.c_str(), currchar[i]->serial);
			sysmessage(s, (char*)temp);
		}
	}
	sysmessage(s, tr("Total Users Online: %1\n").arg(j));
	sysmessage(s, tr("End of userlist"));
}

void command_gms(UOXSOCKET s)
{
	int i, j=0;
	sysmessage(s, tr("Current GMs and Counselors in the world:"));
	for (i=0;i<now;i++)
	{
		if(perm[i] && currchar[i]->isCounselor()) //Keeps NPC's from appearing on the list
		{
			j++;
			sysmessage(s, (char*)currchar[i]->name.c_str());
		}
	}
	sysmessage(s, tr("Total Staff Online: %d\n").arg(j));
	sysmessage(s, tr("End of stafflist"));
}

void command_regspawnall(UOXSOCKET s)
{
	sysbroadcast("ALL Regions Spawning to MAX, this will cause some lag.");
	cAllSpawnRegions::getInstance()->reSpawnToMax();
	return;
}

void command_wipenpcs(UOXSOCKET s)
{
	int i,deleted = 0;
	P_CHAR pc_currchar = currchar[s];
	
	
	AllCharsIterator iter_char;
	for (iter_char.Begin(); !iter_char.atEnd(); iter_char++)
	{
		P_CHAR toCheck = iter_char.GetData();
        if(toCheck->isNpc() && toCheck->npcaitype()!=17 && !toCheck->tamed()) // Ripper
		{			
			LongToCharPtr(toCheck->serial, &removeitem[1]);
			for (i=0;i<now;i++)
			{
				if (perm[i] && inrange1p(toCheck, currchar[i])) 
					Xsend(i, removeitem, 5);
			}
			iter_char--;
			Npcs->DeleteChar(toCheck);
			deleted++;
		}
	}
	
	
	if (SrvParams->gmLog()) 
	{ 
		char temp2[1024];
		sprintf((char*)temp,"%s.log",pc_currchar->name.c_str()); 
		sprintf((char*)temp2,"npc wipe done, %i npcs deleted\n",deleted); 
		savelog((char*)temp2,(char*)temp); 
	}
	
	gcollect();
	
	sysmessage(s, tr("All NPC's have been wiped (%1).").arg(deleted));
	
}

void command_cleanup(UOXSOCKET s)
{
	int corpses=0;
	
	sysmessage(s, tr("Cleaning corpses and closing gates..."));
	AllItemsIterator iter_items;
	for( iter_items.Begin(); !iter_items.atEnd(); iter_items++ )
	{
		P_ITEM pi = iter_items.GetData();
		if(( pi->corpse() ) || (pi->type() == 51) || (pi->type() == 52))
		{
			iter_items--; // Iterator will became invalid when we delete it.
			Items->DeleItem(pi);
			corpses++;
		}
	}
	gcollect();
	sysmessage(s, tr("Done."));
	sysmessage(s, tr("%1 corpses or gates have been cleaned.").arg(corpses));
}

/* new commands go just above this line. :-) */


void command_setmenupriv(UOXSOCKET s)
{
	int i=0;
	
	if (tnum==2)
	{
		i=makenumber(1); 
		addid1[s]=i;	   
		target(s,0,1,0,248,"Select character to set menu privs.");
		return;
		
	} else sysmessage(s, tr("this command takes one arument"));
}

void command_delid( UOXSOCKET s )
{
	if (tnum != 2)
	{
		sysmessage( s, tr("Syntax Error. Usage: /delid <id>") );
		return;
	}
	
	UI16 id = ( static_cast<unsigned char>(hexnumber( 1 )) << 8 ) + static_cast<unsigned char>(hexnumber( 2 ));
	
	AllItemsIterator iterItems;
	for( iterItems.Begin(); !iterItems.atEnd(); iterItems++ )
	{
		P_ITEM pi = iterItems.GetData();
		if( pi->id() == id )
		{
			iterItems--;
			Items->DeleItem( pi );
		}
	}
}

void command_deltype( UOXSOCKET s )
{
	if (tnum != 2)
	{
		sysmessage( s, tr("Syntax Error. Usage: /deltype <type>") );
		return;
	}
	
	unsigned int type = makenumber( 1 );
	AllItemsIterator iter_items;
	for( iter_items.Begin(); !iter_items.atEnd(); iter_items++ )
	{
		P_ITEM pi = iter_items.GetData();
		if( pi->type() == type )
		{
			iter_items--; // Iterator will became invalid when we delete it
			Items->DeleItem( pi );
		}
	}
}

// blackwind system message 
void command_sysm(UOXSOCKET s) 
{ 
	if (now == 1)
	{ 
		sysmessage(s, tr("There are no other users connected.")); 
		return; 
	}	
	strcpy(xtext[s], Commands->GetAllParams().latin1()); 
	sysbroadcast(xtext[s]); 
} 

void command_jail(UOXSOCKET s) 
// (d) Jails the target with given secs. 
{ 
	if (tnum == 2) 
	{ 
		addmitem[s] = makenumber(1); // We temporary use addmitem for jail secs. 
		sprintf((char*)temp, "Select Character to jail. [Jailtime: %i secs]", addmitem[s]); 
		target(s, 0, 1, 0, 126, (char*)temp); 
	} 
	else 
	{ 
		addmitem[s] = SrvParams->default_jail_time(); 
		sprintf((char*)temp, "Select Character to jail. [Jailtime:Server default of %i secs]", addmitem[s]); 
		target(s, 0, 1, 0, 126, (char*)temp); 
	} 
	return; 
} 

// handler for the movement effect 
// Aldur
// 
void command_setGmMoveEff(UOXSOCKET s) 
// (h) set your movement effect. 
{ 
	P_CHAR pc_cs = currchar[s]; 
	if (tnum == 2)
		pc_cs->gmMoveEff = makenumber(1);
	return; 
}

void command_password(UOXSOCKET s)
// (text) Changes the account password
{
	if ( tnum > 1 )
	{
		P_CHAR pc_currchar = currchar[s];
		char pwd[200] = {0,};
		strncpy(pwd, &tbuffer[Commands->cmd_offset+9], 198);
		if ((!isalpha(pwd[0]))&&(!isdigit(pwd[0]))) {
			sysmessage(s, tr("Passwords must start with a letter or a number\n"));
			return;
		}
		if (Accounts->ChangePassword( pc_currchar->account(), pwd )) 
			sysmessage(s, tr("Password changed to %1").arg((char*)pwd));
		else 
			sysmessage(s, tr("Some Error occured while changing password!"));
		
	}
	else 
		sysmessage(s, tr("You must type 'PASSWORD <newpassword>"));
	
	return;
}

void command_debug( UOXSOCKET s )
{
	int temppunt = 0;
	cRegion::RegionIterator4Chars ri(currchar[s]->pos);
	for (ri.Begin(); !ri.atEnd(); ri++)
	{
		P_CHAR pc_vis = ri.GetData();
		++temppunt;
	}
	sysmessage(s, "%i Entries found", temppunt);
}

