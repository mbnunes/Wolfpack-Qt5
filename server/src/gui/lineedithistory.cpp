/*
*     Wolfpack Emu (WP)
* UO Server Emulation Program
*
* Copyright 2001-2016 by holders identified in AUTHORS.txt
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

#include "lineedithistory.h"

#include <QKeyEvent>
#include <QListWidget>
#include <QApplication>
#include <QDesktopWidget>

LineEditHistory::LineEditHistory( QWidget* parent ) : QLineEdit( parent ), currentHistoryIndex( 0 ), listBox( 0 )
{
}

void LineEditHistory::accepted( const QString& t )
{
	history.prepend( t );
	currentHistoryIndex = 0;
}

void LineEditHistory::updateVisibleSymbols( const QStringList& data )
{
	searchSymbols = data;
}

int LineEditHistory::beginOfWord( bool mark )
{
	int pos = cursorPosition();
	if( pos == 0 )
		return 0;
	int move = 0;
	QString text = this->text();
	pos--;
	while( !text[ pos ].isSpace() &&
		( text[ pos ].isLetter() || text[ pos ].isNumber() ||
		( text[ pos ] == '_' ) ) )
	{
		move++;
		if( pos == 0 )
			break;
		pos--;
	}
	cursorBackward( mark, move );
	return move;
}


QPoint LineEditHistory::textCursorPoint() const
{
	int cursorPara;
	int cursorPos;
	//getCursorPosition(&cursorPara, &cursorPos);
	QRect rect = QWidget::rect();
	QPoint point(rect.left(), rect.bottom());
	//while (charAt(point, 0) < cursorPos)
	point.rx() += QFontMetrics(this->font()).boundingRect( text().mid(0,cursorPosition())).width();
	return mapToGlobal(point);
}

void LineEditHistory::complete()
{
	QString para = this->text();
	int wordStart = beginOfWord( false );
	while (wordStart > 0 && para[wordStart - 1].isLetterOrNumber())
		--wordStart;
	wordPrefix = para.mid(wordStart, cursorPosition() - wordStart);
//	if (wordPrefix.isEmpty())
//		return;

	QStringList list = searchSymbols;
	QMap<QString, QString> map;
	QStringList::Iterator it = list.begin();
	while (it != list.end()) {
		if ((*it).startsWith(wordPrefix) && (*it).length() > wordPrefix.length())
			map[(*it).toLower()] = *it;
		++it;
	}

	if (map.count() == 1) {
		insert((*map.begin()).mid(wordPrefix.length()));
	} else if (map.count() > 1) {
		if (!listBox)
			createListBox();
		listBox->clear();
		listBox->addItems( map.values() );

		QPoint point = textCursorPoint();
		adjustListBoxSize(QApplication::desktop()->height() - point.y(), width() / 2);
		listBox->move(point);
		listBox->show();
		listBox->raise();
		listBox->activateWindow();
	}
}

void LineEditHistory::createListBox()
{
	listBox = new QListWidget(this);
	listBox->setWindowFlags( Qt::Popup );

	connect(listBox, SIGNAL(itemClicked(QListWidgetItem *)), this, SLOT(itemChosen(QListWidgetItem *)));
	//connect(listBox, SIGNAL(returnPressed(QListWidgetItem *)),	this, SLOT(itemChosen(QListWidgetItem *)));
}

void LineEditHistory::itemChosen(QListWidgetItem *item)
{
	if (item)
		insert(item->text().mid( wordPrefix.length() ) );
	listBox->close();
}

void LineEditHistory::adjustListBoxSize( int maxHeight /* = 32767 */, int maxWidth /* = 32767  */)
{
/*	if (!listBox->count())
		return;
	int totalHeight = listBox->item(0)->size().height() * listBox->count();
	if ( listBox->variableHeight() ) {
		totalHeight = 0;
		for (int i = 0; i < (int)listBox->count(); ++i)
			totalHeight += listBox->itemHeight(i);
	}
	listBox->setFixedHeight(qMin(totalHeight, maxHeight));
	listBox->setFixedWidth(qMin(listBox->maxItemWidth(), maxWidth));
*/
}

void LineEditHistory::keyPressEvent( QKeyEvent *e )
{
	switch( e->key() )
	{
	case Qt::Key_Up:
		emit invalidate();
		if ( !history.isEmpty() )
		{
			if ( currentHistoryIndex < history.size() )
				currentHistoryIndex++;
			if ( currentHistoryIndex == history.size() )
				clear();
			else
				setText( history[currentHistoryIndex] );
		}
		break;
	case Qt::Key_Down:
		emit invalidate();
		if ( !history.isEmpty() )
		{
			if ( currentHistoryIndex > 0 )
				currentHistoryIndex--;
			setText( history[currentHistoryIndex] );
		}
		break;
	case Qt::Key_Escape:
		emit invalidate();
		clear();
		break;
	case Qt::Key_Control:
		break;
	case Qt::Key_N:
		if( e->modifiers() & Qt::ControlModifier )
		{
			emit searchNext();
			break;
		}
	case Qt::Key_P:
		if( e->modifiers() & Qt::ControlModifier )
		{
			emit searchPrev();
			break;
		}
	case Qt::Key_T:
		if( e->modifiers() & Qt::ControlModifier )
		{
			emit requestVisibleSymbols();
			complete();
			break;
		}
	default:
		emit invalidate();
		QLineEdit::keyPressEvent( e );
	}
}

