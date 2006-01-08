/*
*     Wolfpack Emu (WP)
* UO Server Emulation Program
*
* Copyright 2001-2006 by holders identified in AUTHORS.txt
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

#if !defined ( __PROFILERWINDOW_H__ )
#define __PROFILERWINDOW_H__

#include <QMainWindow>

#include "../python/pyprofiler.h"
#include "ui_profilerwindow.h"

class ProfilerWindow : public QMainWindow
{
	Q_OBJECT
	Ui::ProfilerWindow ui;
public:
	explicit ProfilerWindow(QWidget *parent = 0, Qt::WFlags flags = 0);
	void setProfileSession( const ProfileSession& d );
};

#endif // __PROFILERWINDOW_H__
