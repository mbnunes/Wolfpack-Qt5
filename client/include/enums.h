
#if !defined(__ENUMS_H__)
#define __ENUMS_H__

enum enTextAlign {
	ALIGN_LEFT = 0,
	ALIGN_CENTER,
	ALIGN_RIGHT
};

enum enControlAlign {
	CA_NONE = 0,
	CA_LEFT,
	CA_TOP,
	CA_RIGHT,
	CA_BOTTOM,
	CA_CLIENT
};

// Used to identify the image that should be used to draw the button
enum enButtonStates {
	BS_UNPRESSED = 0,
	BS_HOVER,
	BS_PRESSED,
};

// Definitions of the different map facets
enum enFacet {
	FELUCCA = 0,
	TRAMMEL,
	ILSHENAR,
	MALAS,
	TOKUNO,
	FACETCOUNT = 5 // important
};

// types of entities known to the game
enum enEntityType {
	UNKNOWN = 0,
	GROUND,
	STATIC,
	MOBILE,
	ITEM,
};

// Body types
enum enBodyType {
	ANIMAL = 0,
	MONSTER,
	HUMAN,
	EQUIPMENT
};

#endif
