//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
//	Copyright 1997, 98 by Marcus Rating (Cironian)
//  Copyright 2001-2003 by holders identified in authors.txt
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
//	Foundation, Inc., 59 Temple Palace - Suite 330, Boston, MA 02111-1307, USA.
//
//	* In addition to that license, if you are running this program or modified
//	* versions of it on a public system you HAVE TO make the complete source of
//	* the version used by you available or provide people with a location to
//	* download it.
//
//
//
//	Wolfpack Homepage: http://wpdev.sf.net/
//==================================================================================

//Adapted by TANiS - Thanks to Armageddon for the original code.

#include "wolfpack.h"
#include "debug.h"
#include "verinfo.h"
#include "srvparams.h"
#include "utilsys.h"

static long entries_e=0;
static long entries_c=0;
static long entries_w=0;
static long entries_m=0;
bool NewErrorsLogged() {return (entries_e > 0 || entries_c > 0);}
bool NewWarningsLogged() {return (entries_w > 0);}
/********************************************************
 *                                                      *
 *  Function to be called when a string is ready to be  *
 *    written to the log.  Insert access to your log in *
 *    this function.                                    *
 *                                                      *
 *  Rewritten/Improved/touched by LB 30-July 2000       *
 ********************************************************/

void MessageReady(char *OutputMessage, char type)
{
	char file_name[256];
	FILE *er_log;	

    switch (type)
	{
	   case 'E': { strcpy(file_name,"errors_log.txt");          entries_e++; break; }
	   case 'C': { strcpy(file_name,"critical_errors_log.txt"); entries_c++; break; }
	   case 'W': { strcpy(file_name,"warnings_log.txt");        entries_w++; break; }
	   case 'M': { strcpy(file_name,"messages_log.txt");        entries_m++; break; }
	} 
	
	er_log=fopen(file_name,"a");

	if (er_log==NULL)
	{
		clConsole.send("FATAL ERROR: CANT CREATE/OPEN ERROR LOGFILE, writing to stdout\n");
		clConsole.send(OutputMessage);
		return;
	}

	if (entries_c==1 && type=='C') // @serverstart, write out verison# !!!
	{
       fprintf(er_log,"\nRunning Wolfpack Version: %s\n\n",wp_version.verstring.c_str() );
       fprintf(er_log,"********************************************************************************************************************************************\n");
	   fprintf(er_log,"*** to increase the stability and quality of this software please send this file to the wolfpack developers - thanks for your support!!! ***\n");
	   fprintf(er_log,"********************************************************************************************************************************************\n\n");
	   
	}

    if ( (entries_e==1 && type=='E') || (entries_w==1 && type=='W') || (entries_m==1 && type=='M'))
	{
	   fprintf(er_log,"\nRunning Wolfpack Version: %s\n\n",wp_version.verstring.c_str() );
	}
	
	fprintf(er_log,OutputMessage); // ignoring I/O errors for now !

	if (SrvParams->errors_to_console() && type!='M')
		clConsole.send(OutputMessage);

	fclose(er_log);
}

/********************************************************
 *                                                      *
 *  Rountine to process and stamp a message.            *
 *                                                      *
 ********************************************************/
void LogMessageF(unsigned char Type, long Line, char *File, char *Message, ...)
{
	char fullMessage[512];
	char builtMessage[512];
	va_list argptr;

	va_start(argptr, Message);
	vsprintf(builtMessage, Message, argptr);
	va_end(argptr);

	char timestamp[60];

	time_t currentTime;
	tm *ct;
	time(&currentTime);
	ct = gmtime(&currentTime);
	sprintf(timestamp, "%c:[%2d.%02d][%2d:%02d:%02d]", Type, ct->tm_mday, ct->tm_mon, ct->tm_hour, ct->tm_min, ct->tm_sec);
//#endif

	if (Type=='M')
		strcpy(fullMessage,timestamp);	// no debug info for normal messages (Duke)
	else
		sprintf(fullMessage, "%s %s:%ld ", timestamp, File, Line);
	strcat(fullMessage, builtMessage);
	strcat(fullMessage, "\n");
	MessageReady(fullMessage,Type);
}


/*
	Based on article found at www.codeproject.com, original code from
	Yarantsau Andrei, QA
	Minsk, Belarus

	Modifications from Correa
*/

#if defined(Q_OS_WIN32) && 1

#include <qfile.h>
#include <qdatetime.h>
#include <qthread.h>

#define SystemBasicInformation       0
#define SystemPerformanceInformation 2
#define SystemTimeInformation        3
#define Li2Double(x) ((double)((x).HighPart) * 4.294967296E9 + (double)((x).LowPart))

typedef struct
{
    DWORD   dwUnknown1;
    ULONG   uKeMaximumIncrement;
    ULONG   uPageSize;
    ULONG   uMmNumberOfPhysicalPages;
    ULONG   uMmLowestPhysicalPage;
    ULONG   uMmHighestPhysicalPage;
    ULONG   uAllocationGranularity;
    PVOID   pLowestUserAddress;
    PVOID   pMmHighestUserAddress;
    ULONG   uKeActiveProcessors;
    BYTE    bKeNumberProcessors;
    BYTE    bUnknown2;
    WORD    wUnknown3;
} SYSTEM_BASIC_INFORMATION;

typedef struct
{
    LARGE_INTEGER   liIdleTime;
    DWORD           dwSpare[76];
} SYSTEM_PERFORMANCE_INFORMATION;

typedef struct
{
    LARGE_INTEGER liKeBootTime;
    LARGE_INTEGER liKeSystemTime;
    LARGE_INTEGER liExpTimeZoneBias;
    ULONG         uCurrentTimeZoneId;
    DWORD         dwReserved;
} SYSTEM_TIME_INFORMATION;


// ntdll!NtQuerySystemInformation (NT specific!)
//
// The function copies the system information of the
// specified type into a buffer
//
// NTSYSAPI
// NTSTATUS
// NTAPI
// NtQuerySystemInformation(
//    IN UINT SystemInformationClass,    // information type
//    OUT PVOID SystemInformation,       // pointer to buffer
//    IN ULONG SystemInformationLength,  // buffer size in bytes
//    OUT PULONG ReturnLength OPTIONAL   // pointer to a 32-bit
//                                       // variable that receives
//                                       // the number of bytes
//                                       // written to the buffer 
// );
typedef LONG (WINAPI *PROCNTQSI)(UINT,PVOID,ULONG,PULONG);

PROCNTQSI NtQuerySystemInformation;

#define HEAP_DUMP_FILENAME "heap.out"

class CCompInfo
{
public:
	CCompInfo(char* filename);
	CCompInfo();
	~CCompInfo();
	void SetAnalyzeTime(UINT nTime);
	UINT GetCPUInfo();
	DWORD HeapMakeSnapShot();
	DWORD HeapCommitedBytes();
	DWORD HeapCompareSnapShots();
	void HeapStoreDumpToFile();
	void HeapCompareDumpWithFile(BOOL bShowContent);
	void HeapPrintDump(BOOL bShowContent);
	char* GetTimeString();
private:
	void DoInit();
	void GetHeaps();
	void GetHeapWalk();
public:
	uint				m_nHeaps;
	HANDLE				m_aHeaps[ 256 ];
	DWORD				m_dwCurrentSize;
	DWORD				m_dwSnapShotSize;
	UINT				m_Sleep;
	QTextStream			m_log;
	QFile				m_logFile;
};

CCompInfo::CCompInfo(char* filename) 
{ 
	DoInit();
	m_logFile.setName( filename );
	m_logFile.open( IO_WriteOnly );
	m_log.setDevice( &m_logFile );
};

CCompInfo::CCompInfo() 
{ 
	DoInit();
};
	
CCompInfo::~CCompInfo() 
{
	DeleteFile(HEAP_DUMP_FILENAME);
};

void CCompInfo::DoInit()
{
	//Sleep for getting results
	m_Sleep = 1000;
	//Switch Crt_heap to Process_heap
	//Variable:__MSVCRT_HEAP_SELECT
	//Value:__GLOBAL_HEAP_SELECTED,1
	if (!SetEnvironmentVariable( "__MSVCRT_HEAP_SELECT","__GLOBAL_HEAP_SELECTED,1"))
		return;
	m_dwCurrentSize = 0;
	m_dwSnapShotSize = 0;	
	m_log.setDevice(0);
}

void CCompInfo::SetAnalyzeTime(UINT nTime) 
{ 
	if (nTime > 0) 
		m_Sleep = nTime; 
};

UINT CCompInfo::GetCPUInfo()
{
    SYSTEM_PERFORMANCE_INFORMATION SysPerfInfo;
    SYSTEM_TIME_INFORMATION        SysTimeInfo;
    SYSTEM_BASIC_INFORMATION       SysBaseInfo;
    double                         dbIdleTime;
    double                         dbSystemTime;
    LONG                           status;
    LARGE_INTEGER                  liOldIdleTime = {0,0};
    LARGE_INTEGER                  liOldSystemTime = {0,0};

    NtQuerySystemInformation = (PROCNTQSI)GetProcAddress(
                                          GetModuleHandle("ntdll"),
                                         "NtQuerySystemInformation"
                                         );

    if (!NtQuerySystemInformation)
        return 0;

    // get number of processors in the system
    status = NtQuerySystemInformation(SystemBasicInformation,&SysBaseInfo,sizeof(SysBaseInfo),NULL);
    if (status != NO_ERROR)
        return 0;
    
    while(1)
    {
        // get new system time
	    status = NtQuerySystemInformation(SystemTimeInformation,&SysTimeInfo,sizeof(SysTimeInfo),0);
        if (status != NO_ERROR)
            return 0;

        // get new CPU's idle time
        status = NtQuerySystemInformation(SystemPerformanceInformation,&SysPerfInfo,sizeof(SysPerfInfo),NULL);
        if (status != NO_ERROR)
            return 0;

        // if it's a first call - skip it
       if (liOldIdleTime.QuadPart != 0)
       {
            // CurrentValue = NewValue - OldValue
            dbIdleTime = Li2Double(SysPerfInfo.liIdleTime) - Li2Double(liOldIdleTime);
            dbSystemTime = Li2Double(SysTimeInfo.liKeSystemTime) - Li2Double(liOldSystemTime);

            // CurrentCpuIdle = IdleTime / SystemTime
            dbIdleTime = dbIdleTime / dbSystemTime;

            // CurrentCpuUsage% = 100 - (CurrentCpuIdle * 100) / NumberOfProcessors
            dbIdleTime = 100.0 - dbIdleTime * 100.0 / (double)SysBaseInfo.bKeNumberProcessors + 0.5;

			return (UINT)dbIdleTime;
       }

        // store new CPU's idle and system time
        liOldIdleTime = SysPerfInfo.liIdleTime;
        liOldSystemTime = SysTimeInfo.liKeSystemTime;
		
        // wait one second
        Sleep(m_Sleep);
    }
}

void CCompInfo::GetHeaps()
{
		memset(m_aHeaps,0,sizeof(m_aHeaps));
		m_nHeaps = GetProcessHeaps(sizeof(m_aHeaps)/sizeof(m_aHeaps[0]),m_aHeaps);
};

void CCompInfo::GetHeapWalk() 
{
		PROCESS_HEAP_ENTRY pEntry;
		m_dwCurrentSize = 0;

		GetHeaps();

		for ( unsigned i=0; i < m_nHeaps; i++ )
		{
			pEntry.lpData = NULL;
			while (HeapWalk(m_aHeaps[i], &pEntry ))
			{
			if ((PROCESS_HEAP_ENTRY_BUSY & pEntry.wFlags) == PROCESS_HEAP_ENTRY_BUSY)
				m_dwCurrentSize += pEntry.cbData;
			}
		}
};

DWORD CCompInfo::HeapMakeSnapShot()
{
		GetHeapWalk();
		m_dwSnapShotSize = m_dwCurrentSize;
		return m_dwSnapShotSize;
};

DWORD CCompInfo::HeapCommitedBytes()
{
		GetHeapWalk();
		return m_dwCurrentSize;
}

DWORD CCompInfo::HeapCompareSnapShots()
{
		GetHeapWalk();
		return (m_dwCurrentSize - m_dwSnapShotSize);
}

void CCompInfo::HeapStoreDumpToFile()
{
	PROCESS_HEAP_ENTRY pEntry;
	m_dwCurrentSize = 0;
	HANDLE hFile;
	DWORD dwIn[2], dwOut;

	GetHeaps();
	
	hFile = CreateFile(HEAP_DUMP_FILENAME, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS , NULL, NULL );

	if ( INVALID_HANDLE_VALUE == hFile ) return;

	for ( unsigned i=0; i < m_nHeaps; i++ )
	{
		pEntry.lpData = NULL;
		while (HeapWalk(m_aHeaps[i], &pEntry ))
		{
			if ((PROCESS_HEAP_ENTRY_BUSY & pEntry.wFlags) == PROCESS_HEAP_ENTRY_BUSY)
			{
				dwIn[0] = (DWORD)pEntry.lpData;
				dwIn[1] = (DWORD)pEntry.cbData;
				WriteFile(hFile, &dwIn, sizeof(DWORD)*2 , &dwOut , NULL);
			}
		}
	}

	CloseHandle(hFile);
}

void CCompInfo::HeapCompareDumpWithFile(BOOL bShowContent)
{
	PROCESS_HEAP_ENTRY pEntry;
	m_dwCurrentSize = 0;
	BOOL bIsPresent = FALSE, bResult = FALSE;
	HANDLE hFile;
	DWORD dwPlace = 0, dwSize = 0;
	DWORD dwIn[2], dwOut;

	GetHeaps();
	
	hFile = CreateFile(HEAP_DUMP_FILENAME, GENERIC_READ, 0, NULL, OPEN_EXISTING , FILE_ATTRIBUTE_READONLY , NULL );
	if ( INVALID_HANDLE_VALUE == hFile ) return;

	if (m_log.device())
	{
		if (bShowContent)
			m_log << ( "\n\n ----- E X T E N D E D  H E A P  D U M P ----- \n\n");
		else
			m_log << ("\n\n ----- B A S I C  H E A P  D U M P ----- \n\n");
	}

	for ( unsigned i=0; i < m_nHeaps; i++ )
	{
		pEntry.lpData = NULL;
		while (HeapWalk(m_aHeaps[i], &pEntry ))
		{
			if ((PROCESS_HEAP_ENTRY_BUSY & pEntry.wFlags) == PROCESS_HEAP_ENTRY_BUSY)
			{
				SetFilePointer (hFile, NULL , NULL, FILE_BEGIN) ; 
				bIsPresent = FALSE;
				memset( dwIn , 0 , sizeof(DWORD)*2 );
				dwOut = 0;
				while( 1 )
				{      
					bResult = ReadFile(hFile, &dwIn, sizeof(DWORD)*2 , &dwOut, NULL );
					if (bResult && dwOut == 0)  break;
					if ( dwIn[0] == (DWORD)pEntry.lpData && dwIn[1] == (DWORD)pEntry.cbData )
					{
						bIsPresent = TRUE;
						break;
					}
							
				}
				if (!bIsPresent)
				{
					if (m_log.device())
						m_log << "block at [0x" << QString::number((uint)pEntry.lpData, 16) << "] size [" << pEntry.cbData << "]\n";

					if (bShowContent)
					{
						char* pData = (char *)pEntry.lpData;
						for ( unsigned k = 0 ; k < pEntry.cbData; k++ )
						{
							if ( m_log.device() )
								m_log << (char)pData[k];
						}
						if ( m_log.device() )
							m_log << "\n\n";
					}
				}

			}
		}
	}

	CloseHandle(hFile);
}

void CCompInfo::HeapPrintDump(BOOL bShowContent)
{
	PROCESS_HEAP_ENTRY pEntry;
	m_dwCurrentSize = 0;

	GetHeaps();

	if ( m_log.device() )
	{
		if (bShowContent)
			m_log << ( "--------- Extended heap dump ---------\n");
		else
			m_log << ( "--------- Basic heap dump ---------\n");
	}

	for ( uint i = 0; i < m_nHeaps; ++i )
	{
		pEntry.lpData = NULL;
		uint j = 0;
		while ( HeapWalk(m_aHeaps[i], &pEntry ) )
		{
		if ((PROCESS_HEAP_ENTRY_BUSY & pEntry.wFlags) == PROCESS_HEAP_ENTRY_BUSY)
		{
			if ( m_log.device() )
				m_log << QString("heap[%1] block[%2] at [0x%3] size[%4]\n").arg(i).arg(j).arg((uint)pEntry.lpData, 16).arg(pEntry.cbData);
			if (bShowContent)
			{
				char* pData = (char *)pEntry.lpData;
				for ( unsigned k = 0 ; k < pEntry.cbData; k++ )
				{
					if ( m_log.device() )
						m_log << static_cast<char>( pData[k] );
				}
				if ( m_log.device() )
					m_log << endl << endl;
			}
		}
		j++;
		}
	}
	if ( m_log.device() )
		m_log << ( "\n------------------------------------\n");
}


#define PERFORMANCE_FILENAME "DocProcPerf.log"

class debugThread : public QThread 
{
	bool volatile running;

public:
	debugThread() : running( true )
	{
		start();
	}

	~debugThread()
	{
		running = false;
		wait();
	}

protected:
	void run()
	{
		
		DWORD  dwStartTime = GetTickCount();
		
		CCompInfo* hInfo = new CCompInfo(PERFORMANCE_FILENAME);
		
		hInfo->HeapMakeSnapShot();
		hInfo->HeapStoreDumpToFile();
		
		hInfo->m_log << "DocProcTest started at " << QDateTime::currentDateTime().toString() << endl; 
		
		while (running)
		{
			hInfo->m_log << QString("%1 CPU[%2%]\tMemory[%3Kb]\n")
				.arg(QDateTime::currentDateTime().toString())
				.arg(hInfo->GetCPUInfo())
				.arg(hInfo->HeapCommitedBytes()/1024);
			msleep(1000);
		};
		
		hInfo->m_log << QString("%1 CPU[%2%]\tMemory[%3Kb]\n")
			.arg(QDateTime::currentDateTime().toString())
			.arg(hInfo->GetCPUInfo())
			.arg(hInfo->HeapCommitedBytes()/1024);
		
		hInfo->m_log << "DocProcTest finished at " << QDateTime::currentDateTime().toString() << endl;
		hInfo->m_log << QString("Elapsed time %1 sec\n").arg((GetTickCount() - dwStartTime)/1000 );
		hInfo->m_log << QString("Total memory difference: %1Kb\n\n").arg(hInfo->HeapCompareSnapShots()/1024 );
		
//		hInfo->HeapCompareDumpWithFile(FALSE); // basic report
		hInfo->HeapCompareDumpWithFile(TRUE);  // extended report
		
		delete hInfo;
	}
};

//static debugThread myDebugThread;


#endif
