#ifndef __MENUACTIONS_H__
#define __MENUACTIONS_H__

#include "wolfpack.h"
#include <qdom.h>

UI32 countResources( P_ITEM Container, const vector< UI16 > &MatchingIDs, const vector< UI16 >&MatchingColors, const QString &MatchingName, const UI08 MatchingType, bool CheckID, bool CheckColor, bool CheckName, bool CheckType );

// Returns the remaining resource amount
UI32 consumeResources( P_ITEM Container, UI32 Amount, const vector< UI16 > &MatchingIDs, const vector< UI16 >&MatchingColors, const QString &MatchingName, const UI08 MatchingType, bool CheckID, bool CheckColor, bool CheckName, bool CheckType );

bool useItem( UOXSOCKET Socket, QDomElement Action, bool Failed = false );
bool hasItem( UOXSOCKET Socket, QDomElement Action );
bool makeItem( UOXSOCKET Socket, QDomElement Action );
bool checkSkill( UOXSOCKET Socket, QDomElement Action );

// Utility Function
void playSkillSound( UOXSOCKET Socket, UI08 Skill, bool Success );
UI08 calcRank( P_CHAR Character, UI08 Skill, UI08 MinSkill, UI08 MaxSkill );

#endif

