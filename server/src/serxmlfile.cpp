//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
//	Copyright 1997, 98 by Marcus Rating (Cironian)
//  Copyright 2001 by holders identified in authors.txt
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
//	* In addition to that license, if you are running this program or modified
//	* versions of it on a public system you HAVE TO make the complete source of
//	* the version used by you available or provide people with a location to
//	* download it.
//
//
//
//	Wolfpack Homepage: http://wpdev.sf.net/
//========================================================================================

#include "serxmlfile.h"
#include "platform.h"

//using namespace std;

// Save Path
#include "wolfpack.h"
#include "srvparams.h"

// Libraries
#include "qstring.h"
#include "qfile.h"
#include "qtextstream.h"
#include "qxml.h"

const unsigned int backuplevel = 4;

bool cObjectParser::startElement( const QString& namespaceURI, const QString& localName, const QString& qName,
                       const QXmlAttributes& att )
{
	if( !doc_ )
	{
		doc_ = new QDomDocument( qName );
		QDomElement docelem_ = doc_->createElement( qName );
		doc_->appendChild( docelem_ );
		currtag_ = doc_->documentElement();
		
		Q_UINT32 i = 0;
		QDomAttr attribute;
		QString name;
		QString value;
		while( i < att.count() )
		{
			name = QString( att.qName( i ) );
			value = QString( att.value( i ) );
			attribute = doc_->createAttribute( name );
			attribute.setValue( value );
			docelem_.setAttributeNode( attribute );
			i++;
		}

		return true;
	}
	else if( objectlevel_ == 0 )
	{
		currtag_ = doc_->documentElement();
	}

	QDomElement tag = doc_->createElement( qName );
	Q_UINT32 i = 0;
	while( i < att.count() )
	{
		QString name = att.qName( i );
		QString value = att.value( i );
		tag.setAttribute( name, value );
		i++;
	}

	currtag_.appendChild( tag );

	if( qName == "objectID" )
	{
		currtag_ = tag;
		this->objectlevel_ += 1;
	}

	return true;
}

bool cObjectParser::characters( const QString& ch )
{
	currtag_.setNodeValue( ch );
	return true;
}

bool cObjectParser::endElement( const QString & namespaceURI, const QString & localName, const QString & qName )
{

	if( qName == "objectID" )
	{
		currtag_ = currtag_.parentNode().toElement();
		this->objectlevel_ -= 1;
	}

	return (objectlevel_ != 0 );
}

serXmlFile::~serXmlFile()
{
	if ( file )
	{
		file->close();
		delete file;
	}
	delete document;
	delete reader;
	delete source;
}

void serXmlFile::setVersion(unsigned int __version)
{
	_version = __version;
}

unsigned int serXmlFile::getVersion()
{
	return _version;
}

unsigned int serXmlFile::size()
{
	return _count;
}

void serXmlFile::prepareReading(std::string ident, int bLevel)
{
	// do not recurse forever
	if ( bLevel >= backuplevel )
		return; // nothing more can be done.

	QString fileName( QString( "%1%2" ).arg( SrvParams->getString( "General", "SavePath", "./", true ) ).arg( ident.c_str() ) );

	if ( bLevel != 0 )
		fileName += QString("-%1").arg(bLevel);
	fileName.append(".xml");
	if ( !QFile::exists(fileName) )
	{
		prepareReading( ident.c_str(), ++bLevel );
		return;
	}
	
	file = new QFile(fileName);

	if ( !file->open( IO_ReadOnly ) )
	{
		qWarning("Failed to open file %s", fileName.latin1());
	}
	
	source = new QXmlInputSource( file );
	reader = new QXmlSimpleReader();
	handler = new cObjectParser();
	reader->setContentHandler( handler );

	QString errorMsg;
	int errorLine = 0, errorColumn = 0;

	reader->parse( source, true );
	document = handler->document();

	root = document->documentElement();
	_version = root.attribute("version", "0").toUInt();
	_count   = root.attribute("count", "0").toUInt();
	if ( _count == 0 )
	{
		file->close();
		document = NULL;
		prepareReading( ident.c_str(), ++bLevel );
	}
	node = root;
	ISerialization::prepareReading(ident);
}

void serXmlFile::prepareWritting(std::string ident)
{
	QString fileName( QString( "%1%2" ).arg( SrvParams->getString( "General", "SavePath", "./", true ) ).arg( ident.c_str() ) );

	// perform backups
	unsigned int i;
	QFile::remove( ident.c_str() + QString("-%1.%2").arg(backuplevel).arg("xml"));
	for ( i = backuplevel - 1; i > 0; --i )
	{
		QString from = ident.c_str() + QString("-%1.%2").arg(i).arg("xml");
		QString to   = ident.c_str() + QString("-%1.%2").arg(i + 1).arg("xml");
		rename( from.latin1(), to.latin1() );
	}
	rename ( ident.c_str() + QString(".bin"), ident.c_str() + QString("-1%1").arg(".xml"));
	fileName.append(".xml");
	file = new QFile(fileName);
	if ( !file->open(IO_WriteOnly) )
		return;
	_count = 0;
	
	document = new QDomDocument(ident.c_str());
	root = document->createElement(document->doctype().name());
	root.setAttribute( "version", _version );
	root.setAttribute( "count", _count);
	node = root;

	ISerialization::prepareWritting(ident);
}

void serXmlFile::close()
{
	if ( isWritting() )
	{
		QString originalName = file->name();
		file->close();
		rename( originalName.latin1(), QString(originalName + ".bak").latin1() );
		file->open( IO_WriteOnly );
		QFile* oldFile = new QFile(originalName + ".bak");
		oldFile->open( IO_ReadOnly );
		QTextStream stream( file );
		stream << "<!DOCTYPE " << document->doctype().name() << ">" << endl;
		stream << "<" << document->doctype().name() << " version=\"" << QString::number(_version) << "\" count=\"" 
			<< QString::number(_count) << "\" >" << endl;
		QString line;
		QTextStream t ( oldFile );
		line = t.readLine();
		while ( line != QString::null )
		{
			stream << line << endl;
			line = t.readLine();
		}
		oldFile->close();
		delete oldFile;
		QFile::remove(originalName + ".bak"); // remove temp file
		stream << "</" << document->doctype().name() << ">";
		file->close();
	}
	if ( file )
	{
		file->close();
		delete file;
		file = 0;
	}
	document = NULL;
}

void serXmlFile::writeObjectID( const QString &data )
{
	if( node == root )
		++_count;
	QDomElement newNode = document->createElement("objectID");
	node.appendChild( newNode );
	node = newNode;
	node.setAttribute("value", data);
}

void serXmlFile::write(const char* Key, std::string &data)
{
	QDomElement newNode = document->createElement(Key);
	newNode.setAttribute( "value", data.c_str() );
	node.appendChild( newNode );
}

void serXmlFile::write(const char* Key, unsigned int data)
{
	QDomElement newNode = document->createElement(Key);
	newNode.setAttribute( "value", QString::number(data) );
	node.appendChild( newNode );
}

void serXmlFile::write(const char* Key, signed int data)
{
	QDomElement newNode = document->createElement(Key);
	newNode.setAttribute( "value", QString::number(data) );
	node.appendChild( newNode );
}

void serXmlFile::write(const char* Key, signed short data)
{
	QDomElement newNode = document->createElement(Key);
	newNode.setAttribute( "value", QString::number(data) );
	node.appendChild( newNode );
}

void serXmlFile::write(const char* Key, unsigned short data)
{
	QDomElement newNode = document->createElement(Key);
	newNode.setAttribute( "value", QString::number(data) );
	node.appendChild( newNode );
}

void serXmlFile::write(const char* Key, unsigned char data)
{
	QDomElement newNode = document->createElement(Key);
	newNode.setAttribute( "value", QString::number(data) );
	node.appendChild( newNode );
}

void serXmlFile::write(const char* Key, signed char data)
{
	QDomElement newNode = document->createElement(Key);
	newNode.setAttribute( "value", QString::number(data) );
	node.appendChild( newNode );
}

void serXmlFile::write(const char* Key, bool data)
{
	QDomElement newNode = document->createElement(Key);
	newNode.setAttribute( "value", data ? "true" : "false" );
	node.appendChild( newNode );
}

void serXmlFile::write(const char* Key, double data)
{
	QDomElement newNode = document->createElement(Key);
	newNode.setAttribute( "value", QString::number(data) );
	node.appendChild( newNode );
}

void serXmlFile::done()
{
	if( this->isWritting() )
	{
		if( node.parentNode().isElement() )
		{
			if( node.parentNode().toElement() == root )
			{
				QTextStream stream ( file );
				node.save( stream, 1 );
			}
			node = node.parentNode().toElement();
		}
	}
	else
	{
		QDomNode parent = node.parentNode();
		QDomNode toDelete = parent.removeChild( node );
		toDelete.clear();

		if( parent.toElement() == document->documentElement() )
		{
			reader->parse( source );
			node = document->documentElement();
		}
		else
			node = parent.toElement();
	}
}

void serXmlFile::readObjectID(QString &data)
{
	// Read the object-id if we're done reading the current item (this *will* lead to bugs with non-new item-files)
	QDomNodeList nodeList = node.elementsByTagName( "objectID" );
	if( nodeList.count() > 0 )
	{
		node = nodeList.item( 0 ).toElement();
		data = nodeList.item( 0 ).toElement().attribute("value");
	}
}

void serXmlFile::read(const char* Key, std::string& data)
{
	QDomNodeList nodeList = node.elementsByTagName( Key );
	if ( nodeList.count() > 0 )
	{
		data = nodeList.item( 0 ).toElement().attribute("value").latin1();
		node.removeChild( nodeList.item(0) ); // Free up the memory
	}
}

void serXmlFile::read(const char* Key, unsigned int  &data)
{
	QDomNodeList nodeList = node.elementsByTagName( Key );
	if ( nodeList.count() > 0 )
	{
		data = nodeList.item( 0 ).toElement().attribute("value").toUInt();
		node.removeChild( nodeList.item(0) ); // Free up the memory
	}
}

void serXmlFile::read(const char* Key, signed   int  &data)
{
	QDomNodeList nodeList = node.elementsByTagName( Key );
	if ( nodeList.count() > 0 )
	{
		data = nodeList.item( 0 ).toElement().attribute("value").toInt();
		node.removeChild( nodeList.item(0) ); // Free up the memory
	}
}

void serXmlFile::read(const char* Key, signed short &data)
{
	QDomNodeList nodeList = node.elementsByTagName( Key );
	if ( nodeList.count() > 0 )
	{
		data = nodeList.item( 0 ).toElement().attribute("value").toShort();
		node.removeChild( nodeList.item(0) ); // Free up the memory
	}
}

void serXmlFile::read(const char* Key, unsigned short &data)
{
	QDomNodeList nodeList = node.elementsByTagName( Key );
	if ( nodeList.count() > 0 )
	{
		data = nodeList.item( 0 ).toElement().attribute("value").toUShort();
		node.removeChild( nodeList.item(0) ); // Free up the memory
	}
}

void serXmlFile::read(const char* Key, unsigned char &data)
{
	QDomNodeList nodeList = node.elementsByTagName( Key );
	if ( nodeList.count() > 0 )
	{
		data = nodeList.item( 0 ).toElement().attribute("value").toUShort();
		node.removeChild( nodeList.item(0) ); // Free up the memory
	}
}

void serXmlFile::read(const char* Key, signed   char &data)
{
	QDomNodeList nodeList = node.elementsByTagName( Key );
	if ( nodeList.count() > 0 )
	{
		data = nodeList.item( 0 ).toElement().attribute("value").toShort();
		node.removeChild( nodeList.item(0) ); // Free up the memory
	}
}

void serXmlFile::read(const char* Key, bool &data)
{
	QDomNodeList nodeList = node.elementsByTagName( Key );
	if ( nodeList.count() > 0 )
	{
		data = nodeList.item( 0 ).toElement().attribute("value") == "true" ? true : false;
		node.removeChild( nodeList.item(0) ); // Free up the memory
	}
}

void serXmlFile::read(const char* Key, double &data)
{
	QDomNodeList nodeList = node.elementsByTagName( Key );
	if ( nodeList.count() > 0 )
	{
		data = nodeList.item( 0 ).toElement().attribute("value").toDouble();
		node.removeChild( nodeList.item(0) ); // Free up the memory
	}
}

