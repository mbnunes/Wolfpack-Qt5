/*
*     Wolfpack Emu (WP)
* UO Server Emulation Program
*
* Copyright 2001-2005 by holders identified in AUTHORS.txt
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

#if !defined ( __MAINWINDOW_H__ )
#define __MAINWINDOW_H__

#include <QMainWindow>
#include "ui_mainwindow.h"
#include "../server.h"

class QAction;
class QMenu;
class QTextEdit;
class TrayIcon;

class MainWindow : public QMainWindow
{
	Q_OBJECT
	Ui::MainWindow ui;
	TrayIcon* trayicon;
public:
	MainWindow();
	~MainWindow();

protected:
	bool event ( QEvent * e );
	void closeEvent ( QCloseEvent * e );
	void handleConsoleMessage( const QString& );
	void handleConsoleRollbackChars( unsigned int );
	void handleConsoleNotifyState( enServerState );

private slots:
    void exportDefs();
	void closeWP();
	void reloadAcc();
	void reloadCfg();
	void reloadPyt();
	void reloadScp();
	void saveworld();
	void listusers();
	void homepage();
	void about();

	void profilerStopped();

private:
	void createActions();
    void createMenus();

    QMenu *fileMenu;
	QMenu *reloadMenu;
	QMenu *serverMenu;
	QMenu *scriptingMenu;
	QMenu *helpMenu;
	QAction *expdefAct;
	QAction *exitAct;
	QAction *reloadAccountsAct;
	QAction *reloadConfigAct;
	QAction *reloadPythonAct;
	QAction *reloadScriptsAct;
	QAction *serverSaveAct;
	QAction *serverUsersAct;
	QAction *scriptingProfilerStart;
	QAction *scriptingProfilerStop;
	QAction *helpHPAct;
	QAction *helpAboutAct;

protected slots:
	void onServerStoped();
};


#endif // __MAINWINDOW_H__
