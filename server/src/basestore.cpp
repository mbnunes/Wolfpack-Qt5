//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
//	Copyright 1997, 98 by Marcus Rating (Cironian)
//      Copyright 2001 by holders identified in authors.txt
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
//	Wolfpack Homepage: http://www.wpdev.com/
//========================================================================================


// Wolfpack Includes

#include "basestore.h"

// Method implementations


//=========================================================================================
/// Constructor
Basestore_cl::Basestore_cl()
{
	// Initilize our iterators

	uiSectionOffset = 0 ;
	uiSectionCount = 0 ;
	


}
//=========================================================================================
/// Destructor
Basestore_cl::~Basestore_cl()
{
}


//=========================================================================================
/// Process a file
bool Basestore_cl::processFile(string sFile, SI32 siType, bool bVerbose)
{
	bool bReturn = false;
	
	UI32	uiMark;
	SI32	siKey;
	//	UI32	uiKey;		unreferenced local variable
	//	SI32	siStatus;	unreferenced local variable
	UI08	uiType;
	
	UI16	uiSize;
	UI16	uiData16;
	SI16	siData16;
	UI08	uiData08;
	SI08	siData08;
	SI32	siData;
	UI32	uiData;
	
	//	RF32	rfData32 ;	unreferenced local variable
	RF64	rfData64;
	
	char*	ptrData;
	
	bool bContinue = false;
	
	Basestore_cl::bVerbose = bVerbose;
	uiSectionCount = 0;
	uiSectionOffset = 0;
	fstream fFile;
	UI32 uiVersion;
	fFile.open(sFile.c_str(), ios::in | ios::binary);
	
	if (fFile.is_open())
	{
		bReturn = true;
		if (bVerbose)
		{
			string sOutput;
			sOutput = "Processing file: " + sFile + "\n";
			::clConsole.send(sOutput);
        }
		// Get the verson, ensure it matches with ours
		if (fFile.read((char*)&uiVersion, 4))
		{
			if (uiVersion == BBINARY_VERSION)
			{
				// Get the Number of sections
				if (fFile.read((char*)&uiSectionCount, 4))
				{
					bContinue = true;
				}
			}
			else
			{
				::clConsole.error("Version of file: %s is %u does not match this program version of %u \n", sFile.c_str(), uiVersion, BBINARY_VERSION);
			}
		}
		if (bContinue)
		{
			// Flag for any special entering file processing
			startFile(sFile, siType);
			
			while (!fFile.eof())
			{
				if (fFile.read((char*)&uiMark, 4))
				{
					if (!bInSection)
					{
						if (uiMark == BSECTION_START)
						{
							// Get the key
							if (fFile.read((char*)&uiMark, 4))
							{
								if (fFile.read((char*)&siKey, 4))
								{
									bInSection = true;
									startSection(uiMark, siKey);
								}
							}
						}
					}
					else if (uiMark == BSECTION_END)
					{
						endSection();
						bInSection = false;
					}
					else if (fFile.read((char*)&uiType, 1))
					{
						switch (uiType)
						{
							case BTYPE_SI32:
								if (fFile.read((char*)&siData, 4))
								{
									addEntry(uiMark, siData);
								}
								break;
							case BTYPE_UI32:
								if (fFile.read((char*)&uiData, 4))
								{
									addEntry(uiMark, uiData);
								}
								break;
							case BTYPE_SI16:
								if (fFile.read((char*)&siData16, 2))
								{
									addEntry(uiMark, siData16);
								}
								break;
							case BTYPE_UI16:
								if (fFile.read((char*)&uiData16, 2))
								{
									addEntry(uiMark, uiData16);
								}
								break;
							case BTYPE_SI08:
								if (fFile.read((char*)&siData08, 1))
								{
									addEntry(uiMark, siData08);
								}
								break;
							case BTYPE_UI08:
								if (fFile.read((char*)&uiData08, 1))
								{
									addEntry(uiMark, uiData08);
								}
								break;
							case BTYPE_RF32:
								RF32 rfData;
								if (fFile.read((char*)&rfData, 4))
								{
									addEntry(uiMark, rfData);
								}
								break;
							case BTYPE_RF64:
								if (fFile.read((char*)&rfData64, 8))
								{
									addEntry(uiMark, rfData64);
								}
								break;
							case BTYPE_BOOL:
								if (fFile.read((char*)&uiData08, 1))
								{
									bool bData = true;
									if (uiData08 == 0)
										bData = false;
									addEntry(uiMark, bData);
								}
								break;
							case BTYPE_STRING:
								// String data!
								if (fFile.read((char*)&uiSize, 2))
								{
									string sData;
									if (uiSize > 0)
									{
										ptrData = new char[uiSize];
										if (ptrData != NULL)
										{
											if (fFile.read((char*)ptrData, uiSize))
											{
												string sTemp(ptrData, uiSize);
												sData = sTemp;
											}
											delete[] ptrData;
											ptrData = NULL;
										}
									}
									addEntry(uiMark, sData);
								}
								break;
							default:
								::clConsole.error("Corrupted binary file: %s \n", sFile.c_str());
								break;
						}
					}
				}
			}
			
			// We hit the EOF , so......
			if (bInSection)
				endSection();
			endFile();
		}
		fFile.close();
	}
	else
	{
		::clConsole.error("Unable to open file: %s \n) ", sFile.c_str());
	}
	
	return bReturn;
}
//=========================================================================================
/// 
bool Basestore_cl::saveFile(string sFile,SI32 siType,bool bVerbose)
{
	bool bReturn =false ;
// ############### NEED TO BE DONE#####################
	return bReturn ;
}

//=========================================================================================
/// 
UI32 Basestore_cl::getSectionCount()
{
	return uiSectionCount ;
}



//=========================================================================================
/// Process a file
bool Basestore_cl::writeEntry(fstream& fFile,UI32 uiKey,bool bData)
{
	bool bReturn = false ;
	UI08 uiData = 0 ;
	UI08 uiTYPE = BTYPE_BOOL ;
	if (bData)
		uiData = 1 ;
	if(fFile.write((char*)&uiKey,4))
	{
		if(fFile.write((char*)&uiTYPE,1))
		{
			if(fFile.write((char*)&uiData,1))
			{
				bReturn = true ;
			}
		}
	}
	return bReturn;
}
//=========================================================================================
/// Process a file
bool Basestore_cl::writeEntry(fstream& fFile,UI32 uiKey,UI08 uiData)
{
	bool bReturn = false ;
	UI08 uiTYPE = BTYPE_UI08;

	if(fFile.write((char*)&uiKey,4))
	{
		if(fFile.write((char*)&uiTYPE,1))
		{
			if(fFile.write((char*)&uiData,1))
			{
				bReturn = true ;
			}
		}
	}
	return bReturn ;
}

//=========================================================================================
/// Process a file
bool Basestore_cl::writeEntry(fstream& fFile,UI32 uiKey,SI08 siData)
{
	bool bReturn = false ;
	UI08 uiTYPE = BTYPE_SI08;

	if(fFile.write((char*)&uiKey,4))
	{
		if(fFile.write((char*)&uiTYPE,1))
		{
			if(fFile.write((char*)&siData,1))
			{
				bReturn = true ;
			}
		}
	}
	return bReturn ;
}

//=========================================================================================
/// Process a file
bool Basestore_cl::writeEntry(fstream& fFile,UI32 uiKey,UI16 uiData)
{
	bool bReturn = false ;
	UI08 uiTYPE = BTYPE_UI16;

	if(fFile.write((char*)&uiKey,4))
	{
		if(fFile.write((char*)&uiTYPE,1))
		{
			if(fFile.write((char*)&uiData,2))
			{
				bReturn = true ;
			}
		}
	}
	return bReturn ;
}
//=========================================================================================
/// Process a file
bool Basestore_cl::writeEntry(fstream& fFile,UI32 uiKey,SI16 siData)
{
	bool bReturn = false ;
	UI08 uiTYPE = BTYPE_SI16;

	if(fFile.write((char*)&uiKey,4))
	{
		if(fFile.write((char*)&uiTYPE,1))
		{
			if(fFile.write((char*)&siData,2))
			{
				bReturn = true ;
			}
		}
	}
	return bReturn ;
}

//=========================================================================================
/// Process a file
bool Basestore_cl::writeEntry(fstream& fFile,UI32 uiKey,string& sData)
{
	bool bReturn = false ;
	UI16 uiData = sData.size();
	
	UI08 uiTYPE = BTYPE_STRING;

	if(fFile.write((char*)&uiKey,4))
	{
		if(fFile.write((char*)&uiTYPE,1))
		{

			if(fFile.write((char*)&uiData,2))
			{
				if (uiData > 0)
				{
					if(fFile.write((char*)sData.c_str(),sData.size()))
					{
						bReturn = true ;
	
					}
				}
			}
		}
	}
	return bReturn ;
}
//=========================================================================================
/// Process a file
bool Basestore_cl::writeEntry(fstream& fFile,UI32 uiKey,RF32 rfData)
{
	bool bReturn = false ;
	UI08 uiTYPE = BTYPE_RF32 ;

	if(fFile.write((char*)&uiKey,4))
	{
		if(fFile.write((char*)&uiTYPE ,1))
		{
			if(fFile.write((char*)&rfData,4))
			{
				bReturn = true ;
			}
		}
	}
	return bReturn ;
}
//=========================================================================================
/// Process a file
bool Basestore_cl::writeEntry(fstream& fFile,UI32 uiKey,RF64 rfData)
{
	bool bReturn = false ;
	UI08 uiTYPE = BTYPE_RF64 ;

	if(fFile.write((char*)&uiKey,4))
	{
		if(fFile.write((char*)&uiTYPE ,1))
		{
			if(fFile.write((char*)&rfData,8))
			{
				bReturn = true ;
			}
		}
	}
	return bReturn ;
}

//=========================================================================================
/// Process a file
bool Basestore_cl::writeEntry(fstream& fFile,UI32 uiKey,UI32 uiData)
{
	bool bReturn = false ;
	UI08 uiTYPE = BTYPE_UI32 ;

	if(fFile.write((char*)&uiKey,4))
	{
		if(fFile.write((char*)&uiTYPE ,1))
		{
			if(fFile.write((char*)&uiData,4))
			{
				bReturn = true ;
			}
		}
	}
	return bReturn ;
}
//=========================================================================================
/// Process a file
bool Basestore_cl::writeEntry(fstream& fFile,UI32 uiKey,SI32 siData)
{
	bool bReturn = false ;
	UI08 uiTYPE = BTYPE_SI32;

	if(fFile.write((char*)&uiKey,4))
	{
		if(fFile.write((char*)&uiTYPE,1))
		{
			if(fFile.write((char*)&siData,4))
			{
				bReturn = true ;
			}
		}
	}
	return bReturn ;
}
//=========================================================================================
/// Process a file
bool Basestore_cl::writeStartSection(fstream& fFile,UI32 uiType,SI32 siKey)
{
	bool bReturn = false ;
	UI32 uiLTYPE  = BSECTION_START ;
	uiSectionOffset = fFile.tellp() ;
	if(fFile.write((char*)&uiLTYPE,4))
	{
		if(fFile.write((char*)&uiType,4))
		{
			if(fFile.write((char*)&siKey,4))
			{

				bReturn = true ;
			}
		}
	}
	return bReturn ;
}
//=========================================================================================
/// Process a file
bool Basestore_cl::abortSection(fstream& fFile)
{
	bool bReturn = true ;
	fFile.seekp(uiSectionOffset) ;

	return bReturn ;
}
//=========================================================================================
/// Process a file
bool Basestore_cl::writeEndSection(fstream& fFile)
{
	bool bReturn = false ;
	UI32 uiLTYPE = BSECTION_END ;
	uiSectionOffset = 0 ;
	if(fFile.write((char*)&uiLTYPE,4))
	{
		bReturn = true ;
		uiSectionCount++ ;
	}
	return bReturn ;
}
//=========================================================================================
/// Process a file
bool Basestore_cl::closeFile(fstream& fFile)
{
	bool bReturn = false ;
	// Seek past the version dword
	fFile.seekp(4) ;
	//cout << "Writing section count of "<< uiSectionCount <<endl;
	if(fFile.write((char*)&uiSectionCount,4))
	{
		bReturn = true ;
		fSavefile.close() ;

	}
	return bReturn ;
}
//=========================================================================================
/// Process a file
bool Basestore_cl::openFile(string sFile)
{
	bool bReturn = false ;
	UI32 uiCOUNT = 0 ;
	UI32 uiVersion = BBINARY_VERSION ;
	uiSectionCount = 0 ;
	uiSectionOffset = 0 ;
	fSavefile.open(sFile.c_str(),ios::out|ios::binary) ;
	if (fSavefile.is_open())
	{
		// Ensure this version of binary file matches ours
		
		if(fSavefile.write((char*)&uiVersion,4))
		{
			if(fSavefile.write((char*)&uiCOUNT,4))
			{
				bReturn = true ;
			}
		}

	}
	return bReturn ;
}

