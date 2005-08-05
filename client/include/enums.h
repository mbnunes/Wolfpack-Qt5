
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
	FACETCOUNT = 5, // important
	INTERNAL = 0xff
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

// Layers
enum enLayer {
		LAYER_TRADE = 0,
		LAYER_RIGHTHAND,
		LAYER_LEFTHAND,
		LAYER_SHOES,
		LAYER_PANTS,
		LAYER_SHIRT,
		LAYER_HAT,
		LAYER_GLOVES,
		LAYER_RING,
		LAYER_NECK = 0xA,
		LAYER_HAIR,
		LAYER_WAIST,
		LAYER_INNERTORSO,
		LAYER_BRACELET,
		LAYER_BEARD = 0x10,
		LAYER_MIDDLETORSO,
		LAYER_EARRINGS,
		LAYER_ARMS,
		LAYER_BACK,
		LAYER_BACKPACK,
		LAYER_OUTERTORSO,
		LAYER_OUTERLEGS,
		LAYER_INNERLEGS = 0x18,
		LAYER_VISIBLECOUNT = 0x18,
		LAYER_MOUNT,
		LAYER_BUYRESTOCKCONTAINER,
		LAYER_BUYNORESTOCKCONTAINER,
		LAYER_SELLCONTAINER,
		LAYER_BANKBOX,
		LAYER_DRAGGING,
		LAYER_COUNT
};

#endif
