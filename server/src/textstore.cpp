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
//	Wolfpack Homepage: http://wpdev.sourceforge.net/
//========================================================================================


// Wolfpack Includes

#include "textstore.h"

// Method implementations


//=========================================================================================
/// Constructor
Textstore_cl::Textstore_cl()
{
	// Initilize our iterators
	bSectionPending = false;
	bInSection = false;
	uiSectionOffset = 0 ;
	uiSectionCount = 0 ;
	
	sComment = "//" ;
	sSectionKey="SECTION" ;
	sSectionStart = "{";
	sSectionClose = "}";

}
//=========================================================================================
/// Destructor
Textstore_cl::~Textstore_cl()
{
}
//=========================================================================================
/// 

/*!
 * Process a file containing a list of file names to process, one per line. The opened files
 * will be processed as text files.
 *
 * @param sFile : string with the filename
 * @param siType : not used by Basestore - for inheranted usage
 * @param bVerbose : specify if it should spawn messages to WPConsole
 *
 * @return bool  : return true if succefully loaded all files
 */
bool Textstore_cl::processList(string sFile, SI32 siType,bool bVerbose)
{
    bool bReturn = false;
	Textstore_cl::bVerbose = bVerbose ;
	
	fstream fList;
	
	fList.open(sFile.c_str(), ios::in);
	
	if (fList.is_open())
	{
		string sLine;
		while (getline(fList, sLine))
		{
			sLine = comment(sLine);
			if (!sLine.empty())
			{
				// Processing something!
				if (bVerbose)
				{
					string sOutput;
					sOutput = "Processing file: " + sLine + "\n";
					::clConsole.send(sOutput);
                }
				bReturn = processFile(sLine,siType,bVerbose);
			}
		}
		
		fList.close();
    }
	
	return bReturn;
}

//=========================================================================================

/*!
 * Process a file, as text script
 *
 * @param sFile : string with the filename
 * @param siType : not used by Basestore - for inheranted usage
 * @param bVerbose : specify if it should spawn messages to WPConsole
 *
 * @return bool  : return true if succefull.
 */
bool Textstore_cl::processFile(string sFile,SI32 siType,bool bVerbose)
{
	bool bReturn = false;
	Textstore_cl::bVerbose = bVerbose ;
	fstream fFile;

	fFile.open(sFile.c_str(), ios::in);
	
	if (fFile.is_open())
	{
		bReturn = true;
		if (bVerbose)
		{
			string sOutput;
			sOutput = "Processing file: " + sFile + "\n";
			::clConsole.send(sOutput);
        }

		// Flag for any special entering file processing
		startFile(sFile,siType);

		string sLine;
		
		while (getline(fFile, sLine))
		{
			sLine = comment(sLine);

			if (!sLine.empty())
			{
				// There is something on the line

				if (!bInSection)
				{

					if (openSection(sLine))
					{
						startSection(sLine);
					}
				}
				else
				{
					if (closeSection(sLine))
					{
						endSection(sLine);
					}
					else
						addEntry(sLine);
				}
			}
		}
		// We do an implied end section if we didnt get one, but at the end of a file
		if (bInSection)
		{
			string sTemp ;
			bInSection = false;
			endSection(sTemp);
		}

		// Any special file closing handling
		endFile();

		fFile.close();
	}
	else
	{
		::clConsole.error("Unable to open file %s\n",sFile.c_str());
	}

	return bReturn;
}
//=========================================================================================
/// 
bool Textstore_cl::saveFile(string sFile,SI32 siType,bool bVerbose)
{
	bool bReturn =false ;
	Textstore_cl::bVerbose = bVerbose ;
	string sMessage ;
	fSavefile.open(sFile.c_str(),ios::out) ;
	if (fSavefile.is_open())
	{
		bReturn = true ;
		if (bVerbose)
		{
			sMessage = "Saving to: " + sFile + "\n" ;
			clConsole.send(sMessage) ;
		}
		saveData(sFile,siType) ;
		fSavefile.close();

	}
	else 
	{
		sMessage = "Unable to save to : " + sFile  + "\n";
		clConsole.error( sMessage) ;
	}
	return bReturn ;
}




//=========================================================================================
/// 
void Textstore_cl::writeStartSection(string& sLine)
{
	if (fSavefile.is_open())
	{
		fSavefile << sSectionKey << " "<<sLine << endl;
		fSavefile <<sSectionStart <<endl;
	}
}

//=========================================================================================
/// 
void Textstore_cl::writeEndSection(string& sLine)
{
	// We current dont use sLine, but if we go to XML for our text format...
	if (fSavefile.is_open())
	{
		fSavefile << sSectionClose <<endl;
	}
}
//=========================================================================================
/// 
void Textstore_cl::writeComment(string& sLine)
{
	if (fSavefile.is_open())
	{
		fSavefile <<sComment <<sLine<<endl;
	}
}
//=========================================================================================
/// 
void Textstore_cl::writeEntry(string& sKey,string& sLine)
{
	if (fSavefile.is_open())
	{
		
		fSavefile <<sKey << " "  <<sLine<<endl;
	}
}
//=========================================================================================
/// Process a file
bool Textstore_cl::closeFile()
{
	bool bReturn = true ;
	fSavefile.close() ;
	return bReturn ;
}
//=========================================================================================
/// Process a file
bool Textstore_cl::openFile(string sFile)
{
	bool bReturn = false ;
	fSavefile.open(sFile.c_str(),ios::out) ;
	if (fSavefile.is_open())
	{
		bReturn = true ;
	}
	return bReturn ;
}






//=========================================================================================
// Find a comment, and strip off everything after that

string Textstore_cl::comment(string& sLine)
{
//	SI32 siLocation;
	
	mstring sData(sLine);

	// We also trim up everthing
	sData.trim();
	vector<mstring> vecData = sData.split(sComment,2) ;
	return vecData[0];
}
// End of comment
//=========================================================================================
/// Should we start a section?
bool Textstore_cl::openSection(string& sLine)
{
	bool bReturn = false;
	static string sSectionName;

	
	// first are we pending start of a section?
	if (!bSectionPending)
	{
		// We haven't found a SECTION keyword yet
		// See if we have one now
		
		// We assume keyword section is the first thing after we trim up the line
		SI32 siStart;
		if ((siStart = sLine.find(sSectionKey)) != string::npos)
		{
			// Keyword found
			bSectionPending = true;
			// We dont set bReturn true, as we are not "in" a section yet
			
			// We want to remove the keyword off the sLine (it was passed by reference, so we are modifying it)
			if ((siStart = sLine.find_first_of(MYWHITESPACE,siStart)) != string::npos)
			{
				siStart = sLine.find_first_not_of(MYWHITESPACE,siStart) ;
				if (siStart != string::npos)
					sLine = sLine.substr(siStart) ;
				else
					sLine.erase() ;
			}

			sSectionName = sLine;
		}
	}
	else
	{

		string sStart("{");

		if (sLine.find(sStart) ==0)
		{
			bInSection = true;
			bSectionPending = false;
			bReturn = true;
			sLine = sSectionName;
		}
		else
		{
			// start did not follow keyword, reset everything?
			bSectionPending = false;
		}
	}
	
	return bReturn;
}

//=========================================================================================
/// Should we close a section?
bool Textstore_cl::closeSection(string& sLine)
{
	bool bReturn = false;
	// Assume it is in the first position
	
	// We only care if we are in a section
	if (bInSection)
	{
		if (sLine.find(sSectionClose) == 0)
		{
			bReturn = true;
			bInSection = false;
			bSectionPending = false;
		}
	}
	return bReturn;
}







