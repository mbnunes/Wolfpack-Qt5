#include "packlen.h"

//==================================================================
unsigned short packlen_cl::size(const unsigned char id)
{
 #include "extract.h"

	
	unsigned short uiValue = 0 ;
	if (id <256)
		uiValue = m_packetLen[id] ;
	return uiValue ;
}

