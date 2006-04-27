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

#include <boost/python.hpp>

#include "pythoninteractivewindow.h"
#include "lineedithistory.h"
#include "../python/utilities.h"
#include "../serverconfig.h"

#include <QLabel>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStatusBar>

#include <QPair>

using namespace boost::python;

class PythonInteractiveWindow::PythonInterpreterData
{
public:
	PyThreadState* interpreterState;

	dict main_namespace;
	object main_module;
	dict internals;

	PythonInterpreterData() : interpreterState( 0 )
	{

	}

	~PythonInterpreterData()
	{
		if ( interpreterState )
		{
			PythonGILLocker lock();
			PyThreadState* global_state = PyThreadState_Get();
			PyThreadState_Swap( interpreterState );
			Py_EndInterpreter( interpreterState );
			PyThreadState_Swap( global_state );
		}
	}

	QPair<bool,object> getObjectBeforePoint( LineEditHistory* edit )
	{
		int pos = edit->cursorPosition();
		QString line = edit->text();
		QChar prev;
		if( pos > 0 )
		{
			prev = line[ pos-1 ];
		}
		else
		{
			prev = ' ';
		}
		if( prev == '.' )
		{
			edit->cursorBackward( false );
			//edit->cursorWordBackward( true );
			edit->beginOfWord( true );
			QString name = edit->selectedText();
			QPair<bool,boost::python::object> module = getObjectBeforePoint( edit );
			if( module.first )
			{
				boost::python::dict d = ( boost::python::dict )module.second;
				if( d.has_key( name.toLatin1() ) )
				{
					boost::python::object result = d[ name.toLatin1() ];
					if( PyModule_Check( result.ptr() ) )
					{
						boost::python::handle<> hd( borrowed( PyModule_GetDict( result.ptr() ) ) );
						boost::python::dict md( hd );
						return QPair<bool,object>( true, md );
					}
					return QPair<bool,object>( false, result );
				}
			}
			return QPair<bool,object>( false, boost::python::object() );
		}
		boost::python::dict result( internals );
		result.update( main_namespace );
		return QPair<bool,object>( true, result );
	}

	list getList( LineEditHistory* edit )
	{
		QPair<bool,object> answer = getObjectBeforePoint( edit );
		object o = answer.second;
		if( answer.first )
		{
			dict d = ( dict )o;
			return d.keys();
		}
		handle<> hl( PyObject_Dir( o.ptr() ) );
		list result( hl );
		return result;
	}
};

PythonInteractiveWindow::PythonInteractiveWindow( QWidget * parent ) : QMainWindow( parent ), d( new PythonInterpreterData )
{
	QWidget* box = new QWidget( this );
	QVBoxLayout *vboxlayout = new QVBoxLayout( box );
	output = new QTextEdit(box);
	output->setReadOnly(true);
	vboxlayout->addWidget( output );
	QHBoxLayout *hboxlayout1 = new QHBoxLayout( 0 );
	hboxlayout1->addWidget( new QLabel( "Command :", box ) );
	input = new LineEditHistory(box);
	hboxlayout1->addWidget( input );
	vboxlayout->addLayout( hboxlayout1 );
	setCentralWidget(box);

	resize(QSize(555, 403).expandedTo(minimumSizeHint()));
	setWindowTitle("Wolfpack Embedded Interactive Python Interpreter");
	statusBar();

	connect( input, SIGNAL( returnPressed() ), this, SLOT( evalCommand()) );
	connect( input, SIGNAL( invalidate() ), this, SLOT( resetSearch() ) );
	connect( input, SIGNAL( searchNext() ), this, SLOT( searchNext() ) );
	connect( input, SIGNAL( searchPrev() ), this, SLOT( searchPrev() ) );
	connect( input, SIGNAL( requestVisibleSymbols() ), this, SLOT( updateAutoComplete() ) );

	startPythonInterpreter();
}

PythonInteractiveWindow::~PythonInteractiveWindow()
{
	delete d;
}

void PythonInteractiveWindow::startPythonInterpreter()
{
	QString outputCatcher = 
		"class WolfpackStdoutCapture:\n"
		"\tdef __init__(self):\n"
		"\t\tself.data = ''\n"
		"\tdef write(self, d):\n"
		"\t\tself.data = self.data + d\n"
		"\n"
		"import sys\n"
		"TheWolfpackStdoutCatcher = WolfpackStdoutCapture()\n"
		"TheWolfpackStderrCatcher = WolfpackStdoutCapture()\n"
		"sys.stdout = TheWolfpackStdoutCatcher\n"
		"sys.stderr = TheWolfpackStderrCatcher\n";

	if ( !Py_IsInitialized() )
		close();

	PythonGILLocker lock;
	PyThreadState* global_state = PyThreadState_Get();
	d->interpreterState = Py_NewInterpreter();
	PyThreadState_Swap( d->interpreterState );
	/* Code to execute in the interpreter */
	d->main_module = object( (handle<>( borrowed( PyImport_AddModule( "__main__" ) ) )) );
	d->main_namespace = dict( (handle<>( borrowed( PyModule_GetDict( d->main_module.ptr() ) ) )) );

	// Modify our search-path
	list searchPath = extract<list>( object( handle<>( borrowed( PySys_GetObject( "path" ) ) ) ) );
	QStringList elements = Config::instance()->getString( "General", "Python Searchpath", "./scripts;.", true ).split( ";" );

	// Prepend our items to the searchpath
	for ( int i = elements.count() - 1; i >= 0; --i )
	{
		searchPath.insert( 0, str( elements[i].toLatin1() ) );
	}

	// Import site now
	object siteModule = extract<object>( PyImport_ImportModule( "site" ) );

	PyRun_String( outputCatcher.toLatin1().constData(), Py_file_input, d->main_namespace.ptr(), d->main_namespace.ptr() );

	initInternals();

	PyThreadState_Swap(global_state);
}

void PythonInteractiveWindow::runPythonString( const QString& text )
{
	PythonGILLocker lock;
	PyThreadState* global_state = PyThreadState_Swap( d->interpreterState );
	try 
	{
		handle<> result( PyRun_String( text.toLatin1().constData(), Py_file_input, d->main_namespace.ptr(), d->main_namespace.ptr() ) );
	}
	catch ( error_already_set& )
	{
		PyErr_Print();
	}
	updateOutput();
	// Done with Python calls
	PyThreadState_Swap( global_state );
}

void PythonInteractiveWindow::updateOutput()
{
	// Warning! You must hold the GIL lock AND be in the right thread state before calling this.
	// Check sys.stdout
	object stdoutCatcher (d->main_namespace["TheWolfpackStdoutCatcher"]);
	QString stdoutString = extract<QString>(stdoutCatcher.attr("data"));
	if ( !stdoutString.isEmpty() )
	{
		output->setTextColor( Qt::darkGreen );
		output->append( stdoutString );
		output->setTextColor( Qt::black );
		stdoutCatcher.attr( "data" ) = str("");
	}

	// Check sys.stderr
	object stderrCatcher (d->main_namespace["TheWolfpackStderrCatcher"]);
	QString stderrString = extract<QString>(stderrCatcher.attr("data"));
	if ( !stderrString.isEmpty() )
	{
		output->setTextColor( Qt::red );
		output->append( stderrString );
		output->setTextColor( Qt::black );
		stderrCatcher.attr( "data" ) = str("");
	}
}

void PythonInteractiveWindow::evalCommand()
{
	output->setTextColor( Qt::blue );
	output->append( ">>> " + input->text() );
	output->setTextColor( Qt::black );
	// do something
	QString command = input->text();
	command.replace(QRegExp("\\s*$"), "");

	if ( !buffer.isEmpty() )
	{
		if ( command.isEmpty() ) // Hit enter on a block
		{
			buffer.append("\n");
			parse( buffer ); // Update our IntelliSense :D
			runPythonString( buffer );
			buffer.clear();
		}
		else
		{
			input->accepted( command );
			buffer.append("\n");
			buffer.append( command );
		}
	}
	else
	{
		input->accepted( command );
		if ( command.endsWith(":") ) // New ident block
		{
			buffer.append( command );
		}
		else
		{
			parse( command );
			runPythonString( command );
		}
	}
	input->clear();
}

void PythonInteractiveWindow::parse( QString& command )
{
	/*
	QRegExp rx_print( "\\bprint\\s(.*)\n" );
	command.replace( rx_print, "printObject(\\1)\n" );
	rx_print.setPattern( "\\bprint\\s(.*)" );
	command.replace( rx_print, "printObject(\\1)" );
	*/
	if( command.isEmpty() )
	{
		return;
	}
	QRegExp internals( "^(print|import|exec|del|def|class|assert|pass|return|yield|raise|break|continue|global|if|else|elif|while|for|try|except|finally|from)\\b" );
	if( ( command.indexOf( "\n" ) == -1 ) &&
		( command.indexOf( "=" ) == -1 ) &&
		( internals.indexIn( command ) == -1 ) )
	{
		command.insert( 0, "__internal_var__ = " );
		command.append( "\nprint __internal_var__" );
	}
}

void PythonInteractiveWindow::resetSearch()
{
	search.clear();
}

void PythonInteractiveWindow::updateAutoComplete()
{
	if ( search.isEmpty() )
		searchSymbols();
	input->updateVisibleSymbols( search );
}

void PythonInteractiveWindow::searchNext()
{
	if( search.isEmpty() )
	{
		searchSymbols();
	}

	if ( current_search < search.size() )
	{
		current_search++;
		updateInputText();
	}
	else
		statusBar()->showMessage(tr("You are at the end of the symbols list"), 60);
}

void PythonInteractiveWindow::searchPrev()
{
	if( search.isEmpty() )
	{
		searchSymbols();
	}

	if ( current_search > 0 )
	{
		current_search--;
		updateInputText();
	}
	else
		statusBar()->showMessage(tr("You are at the beginning of the symbols list"), 60);
}

int PythonInteractiveWindow::moveCurorToEndOfWord( bool mark )
{
	int pos = input->cursorPosition();
	QString text = input->text();
	int move = 0, size = text.length();
	while( !text[ pos ].isSpace() &&
		( text[ pos ].isLetter() || text[ pos ].isNumber() ||
		( text[ pos ] == '_' ) ) )
	{
		move++;
		if( pos == size-1 )
			break;
		pos++;
	}
	input->cursorForward( mark, move );
	return move;
}

void PythonInteractiveWindow::updateInputText()
{
	QString text = baseline;
	if( current_search == search.size() )
	{
		text.insert( insert_position, base );
	}
	else
	{
		text.insert( insert_position, search[current_search] );
	}
	input->setText( text );
	input->setCursorPosition( insert_position );
	//edit->cursorWordForward( false );
	moveCurorToEndOfWord( false );
}

void PythonInteractiveWindow::searchSymbols()
{
	insert_position = input->cursorPosition();
	PythonGILLocker lock;
	boost::python::list keys;
	QString line = input->text();
	QChar prev;
	if( insert_position > 0 )
	{
		prev = line[ insert_position - 1 ];
	}
	else
	{
		prev = ' ';
	}
	if( ( prev == '.' ) ||
		( prev == ' ' ) ||
		( prev == '\t' ) ||
		( prev == '\n' ) )
	{
		base = "";
		baseline = input->text();
		keys = d->getList( input );
	}
	else
	{
		int move = input->beginOfWord( false );
		insert_position = input->cursorPosition();
		input->cursorForward( true, move );
		base = input->selectedText();
		input->del();
		baseline = input->text();
		keys = d->getList(input);
	}
	int len = extract<int>(keys.attr("__len__")());
	for( int i = 0 ; i < len ; ++i )
	{
		QString key = extract<QString>( keys[ i ] );

		// Hide our internal stdout/stderr capture code.
		if ( key == "TheWolfpackStdoutCatcher" || key == "TheWolfpackStderrCatcher" || key == "WolfpackStdoutCapture" )
			continue;

		if( key.startsWith( base ) )
		{
			search.push_back( key );
		}
	}
	search.sort();
	current_search = search.size();
}

void PythonInteractiveWindow::initInternals()
{
	d->internals[ "__future__" ] = boost::python::object();
	d->internals[ "abs" ] = boost::python::object();
	d->internals[ "and" ] = boost::python::object();
	d->internals[ "as" ] = boost::python::object();
	d->internals[ "assert" ] = boost::python::object();
	d->internals[ "break" ] = boost::python::object();
	d->internals[ "class" ] = boost::python::object();
	d->internals[ "complex" ] = boost::python::object();
	d->internals[ "continue" ] = boost::python::object();
	d->internals[ "def" ] = boost::python::object();
	d->internals[ "del" ] = boost::python::object();
	d->internals[ "elif" ] = boost::python::object();
	d->internals[ "else" ] = boost::python::object();
	d->internals[ "except" ] = boost::python::object();
	d->internals[ "exec" ] = boost::python::object();
	d->internals[ "execfile" ] = boost::python::object();
	d->internals[ "finally" ] = boost::python::object();
	d->internals[ "float" ] = boost::python::object();
	d->internals[ "for" ] = boost::python::object();
	d->internals[ "from" ] = boost::python::object();
	d->internals[ "global" ] = boost::python::object();
	d->internals[ "hex" ] = boost::python::object();
	d->internals[ "if" ] = boost::python::object();
	d->internals[ "import" ] = boost::python::object();
	d->internals[ "in" ] = boost::python::object();
	d->internals[ "int" ] = boost::python::object();
	d->internals[ "is" ] = boost::python::object();
	d->internals[ "lambda" ] = boost::python::object();
	d->internals[ "len" ] = boost::python::object();
	d->internals[ "long" ] = boost::python::object();
	d->internals[ "None" ] = boost::python::object();
	d->internals[ "not" ] = boost::python::object();
	d->internals[ "NotImplemented" ] = boost::python::object();
	d->internals[ "oct" ] = boost::python::object();
	d->internals[ "or" ] = boost::python::object();
	d->internals[ "pass" ] = boost::python::object();
	d->internals[ "print" ] = boost::python::object();
	d->internals[ "raise" ] = boost::python::object();
	d->internals[ "return" ] = boost::python::object();
	d->internals[ "try" ] = boost::python::object();
	d->internals[ "TypeError" ] = boost::python::object();
	d->internals[ "while" ] = boost::python::object();
	d->internals[ "yield" ] = boost::python::object();
}
