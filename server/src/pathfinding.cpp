/*
 *     Wolfpack Emu (WP)
 * UO Server Emulation Program
 *
 * Copyright 2001-2007 by holders identified in AUTHORS.txt
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

#include "pathfinding.h"
#include "walking.h"

// This is based on dividing the right field into 9
// quadrants and chosing the direction based on those
static unsigned char quick_directions[9] = {
	7, 0, 1,
	6, 0, 2,
	5, 4, 3 };

// optimized algorithm for getting the direction from two positions
inline unsigned char getDirection(int x1, int y1, int x2, int y2) {
	int x = x2 + 1 - x1; // Width of the horizontal rectangle
	int y = y2 + 1 - y1; // Height of the vertical rectangle
	int v = (y + y + y) + x; // The quadrante index

	if (v < 0 || v > 8) {
		return 0;
	} else {
		return quick_directions[v];
	}
}

cPathfinding::cPathfinding() {
	nodes = new stPathNode [nodeCount];
	onopen = new bool [nodeCount];
	touched = new bool [nodeCount];
	path = new unsigned char[areaSize * areaSize];
}

cPathfinding::~cPathfinding() {
	delete [] touched;
	delete [] onopen;
	delete [] nodes;
	delete [] path;
}

// Find a better node than the given one
int cPathfinding::findBest( int node ) {
	int least = nodes[node].total;
	int leastNode = node;

	while (node != -1) {
		if (nodes[node].total < least) {
			least = nodes[node].total;
			leastNode = node;
		}

		node = nodes[node].next;
	}

	removeFromChain(leastNode);

	touched[leastNode] = true;
	onopen[leastNode] = false;

	return leastNode;
}

int cPathfinding::getSuccessors(int node, P_CHAR pChar, int *successors) {
	// OPTIMIZE THESE OUT FOR DECREASED MEMORY EFFICIENCY ??
	int nodex = node % areaSize;
	int nodey = (node / areaSize) % areaSize;
	int nodez = nodes[node].z;

	int count = 0;
	int index;
	Coord coord;
	coord.map = goal.map;

	// Create our current position as a coordinate.
	coord.x = xoffset + nodex;
	coord.y = yoffset + nodey;

	// characters may not block our path.
	// But don't check our goal, this IS blocked in combat.

	// Now this is a personalized loop unrolling effort
	#define CHECK_OFFSET(xoff,yoff) coord.z = nodez; \
		coord.x += xoff; \
		coord.y += yoff; \
		if (coord.x < xoffset + areaSize && coord.y < yoffset + areaSize) { \
			if (mayWalk(pChar, coord)) { \
				if (goal == coord || !CheckForCharacterAtXYZ(pChar, coord)) { \
					index = getNodeIndex(coord.x, coord.y, coord.z); \
					if (index >= 0 && index < nodeCount) { \
						nodes[index].z = coord.z; \
						successors[count++] = index; \
					} \
				} \
			} \
		}

	// Please note that the arguments are the offset related to the PREVIOUS call
	CHECK_OFFSET(0, -1); // 0, -1
	CHECK_OFFSET(+1, 0); // 1, -1
	CHECK_OFFSET(0, +1); // 1, 0
	CHECK_OFFSET(0, +1); // 1, 1
	CHECK_OFFSET(-1, 0); // 0, 1
	CHECK_OFFSET(-1, 0); // -1, 1
	CHECK_OFFSET(0, -1); // -1, 0
	CHECK_OFFSET(0, -1); // -1, -1

	return count;
}

QList<unsigned char> cPathfinding::find(P_CHAR pChar, const Coord &from, const Coord &to)
{
	QList<unsigned char> result;
	int i;

	// We can only calculate a path on the normal maps and if the destination is not out of range
	if (from.isInternalMap() || from.distance(to) > (unsigned int)areaSize) {
		return result;
	}

	memset(touched, 0, sizeof(bool) * nodeCount); // Clear the touched nodes
	this->goal = to; // Save the goal

	// Actually th�s should be the x/y offset of our area
	xoffset = (from.x + to.x - areaSize) / 2;
	yoffset = (from.y + to.y - areaSize) / 2;

	int fromNode = getNodeIndex(from.x, from.y, from.z);
	int toNode = getNodeIndex(to.x, to.y, to.z);
	openlist = fromNode; // Where are we

	// Initialize the node
	nodes[fromNode].cost = 0;
	nodes[fromNode].total = heuristic(from.x - xoffset, from.y - yoffset, from.z);
	nodes[fromNode].parent = -1;
	nodes[fromNode].next = -1;
	nodes[fromNode].prev = -1;
	nodes[fromNode].z = from.z;

	// We touched this node
	onopen[fromNode] = true;
	touched[fromNode] = true;

	int depth = 0;
	int newTotal, newCost;

	// This is controlled by the npc moving. Some npcs can fly (move over impassables)
	// others can open doors
	ignoreDoors = false;
	ignoreMovableImpassables = false;
	int successors[8]; // List of successor nodes used in subsequent iterations. Never more than 8
	int successorCount; // Number of successors found

	while (openlist != -1) {
		if (++depth > maxDepth)
			break; // Break if we would exceed the maximum iteration count

		int bestnode = findBest(openlist);

		// Get adjacent nodes that we can walk to
		successorCount = getSuccessors(bestnode, pChar, successors);

		if (successorCount == 0) {
			break; // We've run into a situation where we'll never find a suitable successor
		}

		// Follow every possible successor
		for (i = 0; i < successorCount; ++i) {
			int successor = successors[i];

			if (touched[successor]) {
				continue; // If we worked on the node already, skip this part
			}

			// calculate the cost of the successor based on the currents node cost
			newCost = nodes[bestnode].cost + 1;
			newTotal = newCost + heuristic(successor % areaSize, (successor / areaSize) % areaSize, nodes[successor].z);

			// Always execute, !wasTouched was always true here
			// if ( !wasTouched || m_Nodes[newNode].total > newTotal )
			nodes[successor].parent = bestnode;
			nodes[successor].cost = newCost;
			nodes[successor].total = newTotal;
			addToChain(successor);

			// We found our target
			if (successor == toNode) {
				int pathCount = 0; // Stack allocation speed isn't a concern here anymore
				int parent = nodes[successor].parent;

				// Record the path in reverse order
				while (parent != -1) {
					path[pathCount++] = getDirection(parent % areaSize, (parent / areaSize) % areaSize, successor % areaSize, (successor / areaSize) % areaSize);
					successor = parent; // Track back
					parent = nodes[successor].parent;

					if (successor == fromNode) {
						break;
					}
				}

				int backtrack = 0;
				while (pathCount != 0) {
					result.append( path[--pathCount] );
				}
				return result; // Immediately return
			}
		}
	}

	return result; // Nothing found
}
