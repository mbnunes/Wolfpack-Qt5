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

/*
 * based on code by (C) 2003 by Pierre Barbier de Reuille, 
 * pierre.barbier@cirad.fr. Posted on gmane.comp.python.c++
 * http://article.gmane.org/gmane.comp.python.c++/3982/match=qt
*/

#if !defined( __LINEEDIT_HISTORY_H__ )
#define __LINEEDIT_HISTORY_H__

#include <QLineEdit>
#include <QStringList>

class QKeyEvent;
class QListWidget;
class QListWidgetItem;

class LineEditHistory : public QLineEdit
{
	Q_OBJECT
public:
	LineEditHistory( QWidget* parent );

	void accepted( const QString& );
	void updateVisibleSymbols( const QStringList& );

	int beginOfWord( bool mark );


signals:
	void searchNext();
	void searchPrev();
	void invalidate();
	void requestVisibleSymbols();

private slots:
	void complete();
	void itemChosen(QListWidgetItem *item);

private:
	void createListBox();
	void adjustListBoxSize( int maxHeight = 32767, int maxWidth = 32767 );
	QPoint textCursorPoint() const;

	QString wordPrefix;
	QListWidget *listBox;
	QStringList history;
	QStringList searchSymbols;
	int currentHistoryIndex;

protected:
	virtual void keyPressEvent ( QKeyEvent * e );

};


#endif // __LINEEDIT_HISTORY_H__
