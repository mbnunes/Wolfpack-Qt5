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
 * Wolfpack Homepage: http://developer.berlios.de/projects/wolfpack/
 */

#if !defined(__PATHFINDING_H__)
#define __PATHFINDING_H__

#include "coord.h"
#include "singleton.h"
#include "basechar.h"

#include <qvaluevector.h>

/*
	Using ints wherever possible for optimization purposes.
*/

// Use these defines to configure the pathfinding algorithm.
// this is not in the configuration because some ppl tend to
// screw it up and ask for help
const int maxDepth = 300;
const int areaSize = 38;

const int planeOffset = 128;
const int planeCount = 13;
const int planeHeight = 20;

// The number of nodes in our array
const int nodeCount = areaSize * areaSize * planeCount;

/*
	Structure used for nodes in the path.
*/
struct stPathNode {
	int cost, total;
	int parent, next, prev;
	int z;
};

/*
	This class encapsulates pathfinding.
*/
class cPathfinding {
protected:
	stPathNode *nodes; // array of nodes
	unsigned char *path; // array of directions
	bool *touched; // array of bools
	bool *onopen; // array of bools
	int xoffset, yoffset;
	int openlist;
	Coord goal; // target coordinate
	bool ignoreDoors; // Should doors be ignored?
	bool ignoreMovableImpassables; // Should movable impassables be ignored?

	int getNodeIndex(int x, int y, int z);
	int heuristic(int x, int y, int z);
	int findBest( int node );
	void addToChain( int node );
	void removeFromChain( int node );
	int getSuccessors(int node, P_CHAR pChar, int *successors); // successors has to be an eight element array

public:
	cPathfinding();
	virtual ~cPathfinding();

	QValueVector<unsigned char> find(P_CHAR pChar, const Coord &from, const Coord &to);
};

/*
	Get an offset within the nodes array for the given coordinates
*/
inline int cPathfinding::getNodeIndex(int x, int y, int z) {
	x -= xoffset;
	y -= yoffset;
	z += planeOffset;
	z /= planeHeight;

	return x + (y * areaSize) + (z * areaSize * areaSize);
}

/*
	Appraises the given coordinate based on the given heuristic. (lower better)
*/
inline int cPathfinding::heuristic(int x, int y, int z) {
	x -= goal.x - xoffset;
	y -= goal.y - yoffset;
	z -= goal.z;

	x *= 11;
	y *= 11;

	return (x * x) + (y * y) + (z * z);
}

/*
	Remove a node from the linked list.
*/
inline void cPathfinding::removeFromChain(int node) {
	if (node < 0 || node >= nodeCount) {
		return;
	}

	if (!touched[node] || !onopen[node]) {
		return;
	}

	int prev = nodes[node].prev;
	int next = nodes[node].next;

	if (openlist == node)
		openlist = next;

	// remove it from the linked list
	if (prev != -1) {
		nodes[prev].next = next;
	}

	if (next != -1) {
		nodes[next].prev = prev;
	}

	nodes[node].prev = -1;
	nodes[node].next = -1;
}

// Add a node to the linked list
inline void cPathfinding::addToChain(int node) {
	if (node < 0 || node >= nodeCount) {
		return;
	}

	removeFromChain(node); // remove from the old position

	if (openlist != -1)
		nodes[openlist].prev = node; // prepend it to the current node

	nodes[node].next = openlist;
	nodes[node].prev = -1;
	openlist = node;

	touched[node] = true;
	onopen[node] = true;
}

typedef Singleton<cPathfinding> Pathfinding;

#endif
