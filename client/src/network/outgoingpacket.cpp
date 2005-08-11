
#include "network/outgoingpacket.h"
#include "network/uosocket.h"

cOutgoingPacket::cOutgoingPacket(unsigned char packetId) : m_Stream(&m_Data, QIODevice::WriteOnly) {
	m_packetId = packetId;

	if (isDynamicSize()) {
		m_Stream << packetId;
		m_Stream << 0;
	}
}

void cOutgoingPacket::writeBigUnicodeTerminated(const QString &text) {
	for (unsigned int i = 0; i < text.length(); ++i) {
		m_Stream << text.at(i).unicode();
	}
	m_Stream << (unsigned short)0;
}

cOutgoingPacket::cOutgoingPacket(unsigned char packetId, unsigned short size) : m_Stream(&m_Data, QIODevice::WriteOnly) {
	m_packetId = packetId;
	m_Data.fill(0, size);

	m_Stream << packetId;

	// Dynamic packet
	if (isDynamicSize()) {
		m_Stream << size;
	}
}

void cOutgoingPacket::writeDynamicSize() {
    unsigned short size = m_Data.size();

	if (size >= 3) {
		m_Data[1] = (unsigned char)((size >> 8) & 0xFF);
		m_Data[2] = (unsigned char)(size & 0xFF);
	}
}

bool cOutgoingPacket::isDynamicSize() const {
	return packetLengths[m_packetId] == 0;
}

void cOutgoingPacket::writeFixedAscii(const QString &text, unsigned short length) {	
	QByteArray data = text.toLatin1();

	if (data.size() < length) {
		m_Stream.writeRawBytes(data, data.size());
		for (int i = data.size(); i < length; ++i) {
			m_Stream << (unsigned char)0;
		}
	} else {
		// Make sure the null termination is there
		data[length - 1] = 0;
		m_Stream.writeRawBytes(data, length);
	}
}

void cOutgoingPacket::writeAscii(const QString &text) {
	QByteArray data = text.toLatin1();
	m_Stream.writeRawBytes(data, data.size());
}

QString cOutgoingPacket::dump() const {
	Q_INT32 length = m_Data.size();
	QString dumped = QString( "\n[ packet: %1; length: %2 ]\n" ).arg( ( Q_UINT8 ) m_Data[0], 2, 16 ).arg( m_Data.count() );

	int lines = length / 16;
	if ( length % 16 ) // always round up.
		lines++;

	for ( int actLine = 0; actLine < lines; ++actLine )
	{
		QString line; //= QString("%1: ").arg(actLine*16, 4, 16); // Faster, but doesn't look so good
		line.sprintf( "%04x: ", actLine * 16 );
		int actRow = 0;
		for ( ; actRow < 16; ++actRow )
		{
			if ( actLine * 16 + actRow < length )
			{
				QString number = QString::number( static_cast<uint>( static_cast<Q_UINT8>( m_Data[actLine*16 + actRow] ) ), 16 ) + QString( " " );
				//line += QString().sprintf( "%02x ", (unsigned int)((unsigned char)data[actLine * 16 + actRow]) );
				if ( number.length() < 3 )
					number.prepend( "0" );
				line += number;
			}
			else
				line += "-- ";
		}

		line += ": ";

		for ( actRow = 0; actRow < 16; ++actRow )
		{
			if ( actLine * 16 + actRow < length )
				line += ( isprint( static_cast<Q_UINT8>( m_Data[actLine * 16 + actRow] ) ) ) ? m_Data[actLine * 16 + actRow] : '.' ;
		}

		line += "\n";
		dumped += line;
	}
	return dumped;
}

void cOutgoingPacket::fill(unsigned short size, unsigned char value) {
	for (int i = 0; i < size; ++i) {
		m_Stream << value;
	}
}
