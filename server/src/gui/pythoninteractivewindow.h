/*
*     Wolfpack Emu (WP)
* UO Server Emulation Program
*
* Copyright 2001-2017 by holders identified in AUTHORS.txt
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
* Wolfpack Homepage: https://github.com/Mutilador/Wolfpack
*/

/*
* based on code by (C) 2003 by Pierre Barbier de Reuille, 
* pierre.barbier@cirad.fr. Posted on gmane.comp.python.c++
* http://article.gmane.org/gmane.comp.python.c++/3982/match=qt
*/


#if !defined( __PYTHON_INTERACTIVE_WINDOW_H__ )
#define __PYTHON_INTERACTIVE_WINDOW_H__

#include <QMainWindow>
#include <QString>

#include <list>

class QTextEdit;
class LineEditHistory;

class PythonInteractiveWindow : public QMainWindow
{
	Q_OBJECT
	class PythonInterpreterData;
	PythonInterpreterData* d;
public:
	PythonInteractiveWindow( QWidget * parent );
	~PythonInteractiveWindow();

private:
	void startPythonInterpreter();
	void updateOutput();
	void runPythonString( const QString& text );
	void parse( QString& );
	void initInternals();
	void searchSymbols();
	void updateInputText();
	int moveCurorToEndOfWord( bool mark );

private slots:
	void evalCommand();
	void searchNext();
	void searchPrev();
	void resetSearch();
	void updateAutoComplete();

private:
	QTextEdit* output;
	LineEditHistory* input;
	QString command;
	QString buffer;

	QStringList search;
	int current_search;
	QString base, baseline;
	int insert_position;

};

#endif // __PYTHON_INTERACTIVE_WINDOW_H__
