
#if !defined(__HUES_H__)
#define __HUES_H__

struct stColor {
	unsigned int r;
	unsigned int g;
	unsigned int b;
};

// This class defines a single hue
struct stHue {
	stColor colors[32];
	stColor start;
	stColor end;
};

class cHues {
private:
	stHue hues[3000];
public:
	void load();
	void reload();
	void unload();

	inline stHue *get(unsigned short hue) {
		if (hue == 0) {
			return 0;
		} else {
			hue = (hue - 1) % 3000; // Wrap after 3000 entries
			return &(hues[hue]);
		}
	}
};

extern cHues *Hues;

#endif