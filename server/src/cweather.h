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

#if !defined (__CWEATHER_H__)
#define __CWEATHER_H__

#include <zthread/Thread.h>

//##ModelId=3C5D92F50288
class cWeather : public ZThread::Thread
{
public:
	//##ModelId=3C5D92F502B0
	cWeather()
	{
        int x;
        for(x=0;x<256;x++);
		{
			Type[x]=0;
			RainChance[x]=0;
			SnowChance[x]=0;
			Duration[x]=0;
			Check[x]=0;
			StartTime[x]=0;
			Active[x]=false;
		}
		CurrentRegions=0; 
	}
	//##ModelId=3C5D92F502BA
	virtual ~cWeather() throw() {}
	//##ModelId=3C5D92F502C4
	void DoWeather(int s); 
	//##ModelId=3C5D92F502CF
	void SetActive(int reg) 
	{
		if(Active[reg]=true)
			Active[reg]=false;
		else
			Active[reg]=true;
	}
	//##ModelId=3C5D92F502E3
	void WTimer();
	//##ModelId=3C5D92F502ED
	void CTimer();
	//##ModelId=3C5D92F502F7
	void CalcType(UOXSOCKET s);
	//##ModelId=3C5D92F5030B
	virtual void run() throw();
	//##ModelId=3C5D92F50316
	virtual void kill() throw();
	//##ModelId=3C5D92F50320
	int Type[256]; // 0 dry, 1 rain, 2 snow
	//##ModelId=3C5D92F50333
	int RainChance[256]; // chance of rain in this region
	//##ModelId=3C5D92F50347
	int SnowChance[256]; // chance of snow in this region
	//##ModelId=3C5D92F50351
	unsigned int Duration[256]; // how long weather lasts in this region
	//##ModelId=3C5D92F5036F
	unsigned int Check[256]; // how many seconds should it attempt to restart weather?
	//##ModelId=3C5D92F50383
	unsigned int StartTime[256]; // when it started
	//##ModelId=3C5D92F5038D
	unsigned int BoltTimer[256];
	//##ModelId=3C5D92F503A1
	int CurrentRegions;
	//##ModelId=3C5D92F503B5
	bool Active[256]; // weather it is active or not
};
extern cWeather* Weather;

#endif // __CWEATHER_H__
