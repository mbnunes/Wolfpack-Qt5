#include "wolfpack.h"

unsigned long __stdcall WeatherThread( void *Arg );

void cWeather::DoWeather(int s)
{  
	char rain[5]="\x65\x01\x46\x00";
	char snow[5]="\x65\x02\x46\xEC";
	int rchance=0;
	int schance=0;
	int wchance=0;
	int i=calcCharFromSer(chars[currchar[s]].serial);
	rchance=RainChance[chars[currchar[s]].region]; // chance % for it to rain in this region
	schance=SnowChance[chars[currchar[s]].region]; // chance % for it to snow in this region
	wchance=rand()%100; // the chance for there to be rain or snow (the lower the bigger chance of weather)

	if(rchance>schance) // if the rain chance is bigger then the snow chance
	{
		if(schance>wchance) 
		{
			Network->xSend(s,snow,4,0);
			Type[chars[currchar[s]].region]=2;
			return;
		} // if schance>wchance
		else if(rchance>wchance) // if the rain chance is bigger then the weather chance
		{
			Network->xSend(s,rain,4,0);
			Type[chars[currchar[s]].region]=1;
			return;
		} // if rchance>wchance
		else
		{
			Type[chars[currchar[s]].region]=0;
			return;
		} // if it doesnt rain or snow, it is dry :)
	}// if rchance>schance
	else
	{
		if(rchance>wchance)
		{
			Network->xSend(s,rain,4,0);
			Type[chars[currchar[s]].region]=1;
			return;
		}// if rchance>wchance
		else if(schance>wchance)
		{
			Network->xSend(s,snow,4,0);
			return;
		} //if schance>wchance
		else // if it doesnt rain or snow
		{
			Type[chars[currchar[s]].region]=0;
			return;
		}// if it doesnt rain or snow
	} // if schance>rchance
}

void cWeather::WTimer()
{
	unsigned int CurTime=uiCurrentTime;
	if (CurrentRegions>255) CurrentRegions=0;  
	                                           
	                                                                                 
	for(int x=0;x<CurrentRegions;x++)
	{
		if(CurTime>(StartTime[x]+Duration[x]) && Active[x])
		{
			Active[x]=false;
			StartTime[x]=uiCurrentTime+Check[x]*CLOCKS_PER_SEC;
		}
		else if(CurTime>StartTime[x] && StartTime[x]>0 && Active[x]==false)
		{
			Active[x]=true;
		}
	}
}

void cWeather::CTimer()
{	
	unsigned char r;
	int player_index;
	unsigned char s=0,now_old=now;
	
	for(s=0;s<now_old;s++)
	{	
		player_index=currchar[s];
		if ( online( player_index ) )  // LB, fix for funny crash when server started and clients already waiting for login.
			                           // In an essence the currchar array cant always be trusted. especially on login.									   
		{		   
		   r=chars[player_index].region;			
		   if(Active[r] && perm[s])
		   Weather->DoWeather(s);				
		}
	}
}

void cWeather::InitWeathThread()
{
	WeathThread = CreateThread( NULL, 0, &WeatherThread, NULL, 0, &WeathThreadID);
}

void cWeather::TermWeathThread()
{
	WaitForSingleObject(WeathThread, INFINITE); 
	CloseHandle( WeathThread );
}

unsigned long __stdcall WeatherThread( void *Arg )
{
	while ( keeprun )
	{
		Sleep( 200 );

		Weather->WTimer();
		Weather->CTimer();
	}

	return TRUE;
}
