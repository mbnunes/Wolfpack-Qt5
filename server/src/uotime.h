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

#if !defined(__UOTIME_H__)
#define __UOTIME_H__

class cUoTime {
private:
	// How many minutes have elapsed
	unsigned int minutes;

public:
	cUoTime() {
		minutes = 0;
	}

	inline void setMinutes(unsigned int data) {
		minutes = data;
	}

	inline unsigned int getMinutes() {
		return minutes;
	}

	inline unsigned char minute() {
		return minutes % 60;
	}

	inline unsigned char hour() {
		return (minutes / 60) % 24;
	}

	inline unsigned char days() {
		return minutes / 1440;
	}
};

typedef SingletonHolder<cUoTime> UoTime;

#endif
