#ifndef _H_WEATHER
#define _H_WEATHER
class cWeather
{
public:
	cWeather()
	{
		for(int x=0;x<256;x++);
		{
			Type[x]=0;
			RainChance[x]=0;
			SnowChance[x]=0;
			Duration[x]=0;
			Check[x]=0;
			StartTime[x]=0;
			Active[x]=false;
		}
		CurrentRegions=0; // lb bugfix !? 
	}
	void DoWeather(int s); // start the weather
	void SetActive(int reg) 
	{
		if(Active[reg]=true)
			Active[reg]=false;
		else
			Active[reg]=true;
	}
	void InitWeathThread();
	void TermWeathThread();
	void WTimer();
	void CTimer();
	int Type[256]; // 0 dry, 1 rain, 2 snow
	int RainChance[256]; // chance of rain in this region
	int SnowChance[256]; // chance of snow in this region
	unsigned int Duration[256]; // how long weather lasts in this region
	unsigned int Check[256]; // how many seconds should it attempt to restart weather?
	unsigned int StartTime[256]; // when it started
	unsigned int BoltTimer[256];
	int CurrentRegions;
	bool Active[256]; // weather it is active or not
protected:
	HANDLE WeathThread;
	unsigned long WeathThreadID;
};
extern cWeather* Weather;
#endif
