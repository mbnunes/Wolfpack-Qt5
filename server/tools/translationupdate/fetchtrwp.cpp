#include <metatranslator.h>

#include <qfile.h>
#include <qregexp.h>
#include <qstring.h>
#include <qtextstream.h>
#include <qvaluestack.h>
#include <qregexp.h>
#include <qxml.h>


#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>


void fetchtr_py( const char* fileName, MetaTranslator* tor, const char* /*defaultContext*/, bool mustExist )
{
	static QRegExp matchtr("\\W(tr)\\s{0,1}\\(");
	static QRegExp stringDelimiter("(\"?|\'?)");
	static QString context("@pythonscript");

	QFile f( fileName );
	if ( !f.open( IO_ReadOnly ) )
	{
		if ( mustExist )
			fprintf( stderr, "translationUpdate error: Cannot open Python source file '%s': %s\n", fileName, f.errorString() );
		return;
	}
	QString content = QString( f.readAll() );
	content.replace( QRegExp("#[^\n]*"), "" );; // remove single line comments
	int pos = 0;
	while ( ( pos = matchtr.search(content, pos) ) != -1 )
	{
		QString message = content.mid(pos + matchtr.matchedLength());
		int start = 0, end = 0;
		if ( ( start = stringDelimiter.search(message) ) == -1 )
		{
			pos += matchtr.matchedLength();
			continue;
		}
		QString startDelimiter = message.mid( start, stringDelimiter.matchedLength() );
		end = message.find( startDelimiter, start + startDelimiter.length() );
		if ( end == -1 )
		{
			fprintf( stderr, "translationUpdate error: Open string in tr() '%s': %i\n", fileName, pos );
			return;
		}
		
		QString source = message.mid( start + startDelimiter.length(), end - startDelimiter.length() );
		if ( !source.isEmpty() )
			tor->insert( MetaTranslatorMessage( context.utf8(), source.utf8(), QString(fileName).utf8(), QString::null, TRUE ) );
		pos += QMAX( end, matchtr.matchedLength() );
	}
	
}

class DefinitionHandler : public QXmlDefaultHandler
{
public:
	DefinitionHandler( MetaTranslator* translator, const char* fileName ) : tor( translator ), 
		fname( fileName ), comment( "" )
	{
		context = "@definitions";
	}

	virtual bool startElement( const QString& namespaceURI, const QString& localName, const QString& qName, const QXmlAttributes& atts );
	virtual bool endElement( const QString& namespaceURI, const QString& localName, const QString& qName );
	virtual bool characters( const QString& ch );
	virtual bool fatalError( const QXmlParseException& exception );

private:
	void flush();

	MetaTranslator *tor;
	QCString fname;
	QString context;
	QString source;
	QString comment;

	QString accum;
};

bool DefinitionHandler::startElement( const QString& /* namespaceURI */, const QString& /* localName */, const QString& qName, const QXmlAttributes& /* atts */ )
{
	if ( qName == QString( "title" ) )
	{
		flush();
	}
	accum.truncate( 0 );
	return TRUE;
}

bool DefinitionHandler::endElement( const QString& /* namespaceURI */, const QString& /* localName */, const QString& qName )
{
	accum.replace( QRegExp( QString( "\r\n" ) ), "\n" );

	if ( qName == QString( "title" ) )
	{
		source = accum;
	}
	else
	{
		flush();
	}
	return TRUE;
}

bool DefinitionHandler::characters( const QString& ch )
{
	accum += ch;
	return TRUE;
}

bool DefinitionHandler::fatalError( const QXmlParseException& exception )
{
	QString msg;
	msg.sprintf( "Parse error at line %d, column %d (%s).", exception.lineNumber(), exception.columnNumber(), exception.message().latin1() );
	fprintf( stderr, "XML error: %s\n", msg.latin1() );
	return FALSE;
}

void DefinitionHandler::flush()
{
	if ( !context.isEmpty() && !source.isEmpty() )
		tor->insert( MetaTranslatorMessage( context.utf8(), source.utf8(), comment.utf8(), QString::null, TRUE ) );
	source.truncate( 0 );
	comment.truncate( 0 );
}

void fetchtr_xml( const char* fileName, MetaTranslator* tor, const char* /* defaultContext */, bool mustExist )
{
	QFile f( fileName );
	if ( !f.open( IO_ReadOnly ) )
	{
		if ( mustExist )
			fprintf( stderr, "translationupdate error: cannot open XML file '%s': %s\n", fileName, strerror( errno ) );
		return;
	}

	QTextStream t( &f );
	QXmlInputSource in( t );
	QXmlSimpleReader reader;
	reader.setFeature( "http://xml.org/sax/features/namespaces", FALSE );
	reader.setFeature( "http://xml.org/sax/features/namespace-prefixes", TRUE );
	reader.setFeature( "http://trolltech.com/xml/features/report-whitespace-only-CharData", FALSE );
	QXmlDefaultHandler *hand = new DefinitionHandler( tor, fileName );
	reader.setContentHandler( hand );
	reader.setErrorHandler( hand );

	if ( !reader.parse( in ) )
		fprintf( stderr, "%s: Parse error in XML file\n", fileName );
	reader.setContentHandler( 0 );
	reader.setErrorHandler( 0 );
	delete hand;
	f.close();
}
