
#if !defined(__UOTIME_H__)
#define __UOTIME_H__

class cUoTime {
private:
	// How many minutes have elapsed
	unsigned int minutes;

public:
	cUoTime() {
		minutes = 0;
	}

	inline void setMinutes(unsigned int data) {
		minutes = data;
	}

	inline unsigned int getMinutes() {
		return minutes;
	}

	inline unsigned char minute() {
		return minutes % 60;
	}
	
	inline unsigned char hour() {
		return (minutes / 60) % 24;
	}
	
	inline unsigned char days() {
		return minutes / 1440;
	}
};

typedef SingletonHolder<cUoTime> UoTime;

#endif
