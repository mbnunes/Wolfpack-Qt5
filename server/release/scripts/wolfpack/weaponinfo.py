
from wolfpack.consts import RESISTANCE_PHYSICAL, RESISTANCE_ENERGY, \
	RESISTANCE_COLD, RESISTANCE_POISON, RESISTANCE_FIRE, DAMAGE_ENERGY, \
	DAMAGE_COLD, DAMAGE_POISON, DAMAGE_FIRE, DAMAGEBONUS, SPEEDBONUS, \
	HITBONUS, DEFENSEBONUS, MINDAMAGE, MAXDAMAGE, SPEED, MISSSOUND, \
	HITSOUND, PROJECTILE, PROJECTILEHUE, AMMUNITION, REQSTR, REQDEX, REQINT, \
	LUCK, GOLDINCREASE, LOWERREQS, HITPOINTRATE, STAMINARATE, MANARATE, \
	SPELLDAMAGEBONUS, MATERIALPREFIX, SWING, REFLECTPHYSICAL, \
	DURABILITYBONUS, WEIGHTBONUS, DAMAGE_PHYSICAL

WEAPON_RESNAME_BONI = {
	# Metals
	'iron': {
		DAMAGE_FIRE: 0,
		DAMAGE_COLD: 0,
		DAMAGE_POISON: 0,
		DAMAGE_ENERGY: 0
	},
	'dullcopper': {
		DAMAGE_FIRE: 0,
		DAMAGE_COLD: 0,
		DAMAGE_POISON: 0,
		DAMAGE_ENERGY: 0,
		LOWERREQS: 50,
		MATERIALPREFIX: 'Dull Copper',
		DURABILITYBONUS: 100,
	},
	'shadowiron': {
		DAMAGE_FIRE: 0,
		DAMAGE_COLD: 20,
		DAMAGE_POISON: 0,
		DAMAGE_ENERGY: 0,
		MATERIALPREFIX: 'Shadow Iron',
		DURABILITYBONUS: 50,
	},
	'copper': {
		DAMAGE_FIRE: 0,
		DAMAGE_COLD: 0,
		DAMAGE_POISON: 10,
		DAMAGE_ENERGY: 20,
		MATERIALPREFIX: 'Copper',
	},
	'bronze': {
		DAMAGE_FIRE: 40,
		DAMAGE_COLD: 0,
		DAMAGE_POISON: 0,
		DAMAGE_ENERGY: 0,
		MATERIALPREFIX: 'Bronze',
	},
	'gold': {
		DAMAGE_FIRE: 0,
		DAMAGE_COLD: 0,
		DAMAGE_POISON: 0,
		DAMAGE_ENERGY: 0,
		LUCK: 40,
		LOWERREQS: 50,
		MATERIALPREFIX: 'Gold',
	},
	'agapite': {
		DAMAGE_FIRE: 0,
		DAMAGE_COLD: 30,
		DAMAGE_POISON: 0,
		DAMAGE_ENERGY: 20,
		MATERIALPREFIX: 'Agapite',
	},
	'verite': {
		DAMAGE_FIRE: 0,
		DAMAGE_COLD: 0,
		DAMAGE_POISON: 40,
		DAMAGE_ENERGY: 20,
		MATERIALPREFIX: 'Verite',
	},
	'valorite': {
		DAMAGE_FIRE: 10,
		DAMAGE_COLD: 20,
		DAMAGE_POISON: 10,
		DAMAGE_ENERGY: 20,
		MATERIALPREFIX: 'Valorite',
	}

}

#
# Weapon Information
# This file includes all known standard weapon information.
# The baseid of the item is used as the key for WEAPONINFO.
# For unspecified entries the default value will be used.
#
WEAPONINFO = {
	# Demonstration entry
	#'baseid': {
	# MINDAMAGE: The minimum damage this weapon deals
	# MAXDAMAGE: The maximum damage this weapon deals
	# HITSOUND: A list of soundeffects played for this weapon if it hits
	# MISSSOUND: A list of soundeffects played for this weapon if it misses
	# SPEED: The weapon speed
	# AMMUNITION: The BASE ID of the ammunition to use for this weapon.
	# PROJECTILE: The item id of the projectile to throw at the defender if shot.
	# PROJECTILEHUE: The hue of the projectile this weapon shoots
	#}

	# Bow 1
	'13b1': {
		MINDAMAGE: 16,
		MAXDAMAGE: 18,
		SPEED: 25,
		REQSTR: 30,
		PROJECTILE: 0xf42,
		AMMUNITION: 'f3f',
		SWING: [ 0x12 ],
		HITSOUND: [ 0x234 ],
		MISSSOUND: [ 0x238 ]
	},
	# Crossbow 1
	'f4f': {
		MINDAMAGE: 18,
		MAXDAMAGE: 20,
		SPEED: 24,
		REQSTR: 35,
		PROJECTILE: 0x1bfe,
		AMMUNITION: '1bfb',
		SWING: [ 0x13 ],
		HITSOUND: [ 0x234 ],
		MISSSOUND: [ 0x238 ]
	},
	# Heavy Crossbow 1
	'13fd': {
		MINDAMAGE: 19,
		MAXDAMAGE: 20,
		SPEED: 22,
		REQSTR: 80,
		PROJECTILE: 0x1bfe,
		AMMUNITION: '1bfb',
		SWING: [ 0x13 ],
		HITSOUND: [ 0x234 ],
		MISSSOUND: [ 0x238 ]
	},
	# Composite Bow 1
	'26c2': {
		MINDAMAGE: 15,
		MAXDAMAGE: 17,
		SPEED: 25,
		REQSTR: 45,
		PROJECTILE: 0xf42,
		AMMUNITION: 'f3f',
		SWING: [ 0x12 ],
		HITSOUND: [ 0x234 ],
		MISSSOUND: [ 0x238 ]
	},
	# Repeating Crossbow 1
	'26cd': {
		MINDAMAGE: 30,
		MAXDAMAGE: 49,
		SPEED: 41,
		REQSTR: 30,
		PROJECTILE: 0x1bfe,
		AMMUNITION: '1bfb',
		SWING: [ 0x13 ],
		HITSOUND: [ 0x234 ],
		MISSSOUND: [ 0x238 ]
	},
	# Katana 1
	'13ff': {
		MINDAMAGE: 11,
		MAXDAMAGE: 13,
		SPEED: 46,
		REQSTR: 25,
		SWING: [ 0x9 ],
		HITSOUND: [0x23b, 0x23c],
		MISSSOUND: [ 0x23a, 0x238, 0x239 ]
	},
	# Axe 1
	'f49': {
		MINDAMAGE: 14,
		MAXDAMAGE: 16,
		SPEED: 37,
		REQSTR: 35,
		SWING: [ 0xd ],
		HITSOUND: [0x233, 0x232],
		MISSSOUND: [ 0x23a, 0x239 ]
	},
	# Bardiche 1
	'f4d': {
		MINDAMAGE: 17,
		MAXDAMAGE: 18,
		SPEED: 28,
		REQSTR: 45,
		SWING: [0xc, 0xd],
		HITSOUND: [ 0x237 ],
		MISSSOUND: [ 0x23a, 0x238, 0x239 ]
	},
	# Battle Axe 1
	'f47': {
		MINDAMAGE: 15,
		MAXDAMAGE: 17,
		SPEED: 31,
		REQSTR: 35,
		SWING: [ 0xd ],
		HITSOUND: [0x233, 0x232],
		MISSSOUND: [ 0x23a, 0x239 ]
	},
	# Bladded Staff 1
	'26c7': {
		MINDAMAGE: 14,
		MAXDAMAGE: 16,
		SPEED: 37,
		REQSTR: 40,
		SWING: [0xc, 0xd],
		HITSOUND: [0x23b, 0x23c],
		MISSSOUND: [ 0x239 ]
	},
	# Bone Harvester 1
	'26c5': {
		MINDAMAGE: 13,
		MAXDAMAGE: 15,
		SPEED: 36,
		REQSTR: 25,
		SWING: [0x9],
		HITSOUND: [0x23b, 0x23c],
		MISSSOUND: [ 0x239 ]
	},
	# Broadsword 1
	'f5e': {
		MINDAMAGE: 14,
		MAXDAMAGE: 15,
		SPEED: 33,
		REQSTR: 30,
		SWING: [ 0x9 ],
		HITSOUND: [0x23b, 0x23c, 0x237],
		MISSSOUND: [ 0x23a, 0x238, 0x239 ]
	},
	# Butcher Knife 1
	'13f6': {
		MINDAMAGE: 9,
		MAXDAMAGE: 11,
		SPEED: 49,
		REQSTR: 5,
		SWING: [ 0x9 ],
		HITSOUND: [0x23b, 0x23c],
		MISSSOUND: [ 0x23a, 0x238, 0x239 ]
	},
	# Cleaver 1
	'ec3': {
		MINDAMAGE: 11,
		MAXDAMAGE: 13,
		SPEED: 46,
		REQSTR: 10,
		SWING: [ 0x9 ],
		HITSOUND: [0x23b, 0x23c],
		MISSSOUND: [ 0x23a, 0x238, 0x239 ]
	},
	# Crescent Blade 1
	'26cb': {
		MINDAMAGE: 11,
		MAXDAMAGE: 14,
		SPEED: 47,
		REQSTR: 55,
		SWING: [0xc, 0xd],
		HITSOUND: [0x23b, 0x23c],
		MISSSOUND: [ 0x239 ]
	},
	# Cutlass 1
	'1441': {
		MINDAMAGE: 11,
		MAXDAMAGE: 13,
		SPEED: 44,
		REQSTR: 25,
		SWING: [0x9],
		HITSOUND: [0x23b, 0x23c],
		MISSSOUND: [ 0x23a, 0x238, 0x239 ]
	},
	# Double Axe 1
	'f4b': {
		MINDAMAGE: 15,
		MAXDAMAGE: 17,
		SPEED: 33,
		REQSTR: 45,
		SWING: [ 0xd ],
		HITSOUND: [0x233, 0x232],
		MISSSOUND: [ 0x23a, 0x239 ]
	},
	# Executioners Axe 1
	'f45': {
		MINDAMAGE: 15,
		MAXDAMAGE: 17,
		SPEED: 33,
		REQSTR: 40,
		SWING: [ 0xd ],
		HITSOUND: [0x233, 0x232],
		MISSSOUND: [ 0x23a, 0x239 ]
	},
	# Halberd 1
	'143f': {
		MINDAMAGE: 18,
		MAXDAMAGE: 19,
		SPEED: 25,
		REQSTR: 95,
		SWING: [0xc, 0xd],
		HITSOUND: [ 0x237 ],
		MISSSOUND: [ 0x23a, 0x238, 0x239 ]
	},
	# Hatchet 1
	'f43': {
		MINDAMAGE: 13,
		MAXDAMAGE: 15,
		SPEED: 41,
		REQSTR: 20,
		SWING: [ 0xd ],
		HITSOUND: [0x233, 0x232],
		MISSSOUND: [ 0x23a, 0x239 ]
	},
	# Large Battle Axe 1
	'13fb': {
		MINDAMAGE: 16,
		MAXDAMAGE: 17,
		SPEED: 29,
		REQSTR: 80,
		SWING: [ 0xd ],
		HITSOUND: [0x233, 0x232],
		MISSSOUND: [ 0x23a, 0x239 ]
	},
	# Long Sword 1
	'13b8': {
		MINDAMAGE: 15,
		MAXDAMAGE: 16,
		SPEED: 30,
		REQSTR: 35,
		SWING: [0x9],
		HITSOUND: [0x23b, 0x23c],
		MISSSOUND: [ 0x23a, 0x238, 0x239 ]
	},
	# Longsword 1
	'f61': {
		MINDAMAGE: 15,
		MAXDAMAGE: 16,
		SPEED: 30,
		REQSTR: 35,
		SWING: [0x9],
		HITSOUND: [0x23b, 0x23c],
		MISSSOUND: [ 0x23a, 0x238, 0x239 ]
	},
	# Paladin Sword 1
	'26cf': {
		MINDAMAGE: 14,
		MAXDAMAGE: 15,
		SPEED: 33,
		REQSTR: 30,
		SWING: [0x9],
		HITSOUND: [0x23b, 0x23c, 0x237],
		MISSSOUND: [ 0x23a, 0x238, 0x239 ]
	},
	# Pickaxe 1
	'e86': {
		MINDAMAGE: 13,
		MAXDAMAGE: 15,
		SPEED: 35,
		REQSTR: 50,
		SWING: [0x9],
		HITSOUND: [0x233, 0x232],
		MISSSOUND: [ 0x23a, 0x239 ]
	},
	# Scimitar 1
	'13b6': {
		MINDAMAGE: 13,
		MAXDAMAGE: 15,
		SPEED: 37,
		REQSTR: 25,
		SWING: [0x9],
		HITSOUND: [0x23b, 0x23c],
		MISSSOUND: [ 0x23a, 0x238, 0x239 ]
	},
	# Scythe 1
	'26c4': {
		MINDAMAGE: 15,
		MAXDAMAGE: 18,
		SPEED: 32,
		REQSTR: 45,
		SWING: [0xc, 0xd],
		HITSOUND: [0x23b, 0x23c],
		MISSSOUND: [ 0x23a, 0x238, 0x239 ]
	},
	# Skinning Knife 1
	'ec4': {
		MINDAMAGE: 9,
		MAXDAMAGE: 11,
		SPEED: 49,
		REQSTR: 5,
		SWING: [0x9],
		HITSOUND: [0x23b, 0x23c],
		MISSSOUND: [ 0x23a, 0x238, 0x239 ]
	},
	# Two Handed Axe 1
	'1443': {
		MINDAMAGE: 16,
		MAXDAMAGE: 17,
		SPEED: 31,
		REQSTR: 40,
		SWING: [ 0xd ],
		HITSOUND: [0x233, 0x232],
		MISSSOUND: [ 0x23a, 0x239 ]
	},
	# Viking Sword 1
	'13b9': {
		MINDAMAGE: 15,
		MAXDAMAGE: 17,
		SPEED: 28,
		REQSTR: 40,
		SWING: [0x9],
		HITSOUND: [0x23b, 0x23c, 0x237],
		MISSSOUND: [ 0x23a, 0x238, 0x239 ]
	},
	# Dagger 1
	'f51': {
		MINDAMAGE: 10,
		MAXDAMAGE: 11,
		SPEED: 49,
		REQSTR: 10,
		SWING: [0xa],
		HITSOUND: [0x23b, 0x23c],
		MISSSOUND: [ 0x23a, 0x239 ]
	},
	# Double Bladed Staff 1
	'26c9': {
		MINDAMAGE: 12,
		MAXDAMAGE: 13,
		SPEED: 49,
		REQSTR: 50,
		SWING: [0xe],
		HITSOUND: [0x23b, 0x23c],
		MISSSOUND: [ 0x23a, 0x239 ]
	},
	# Kryss 1
	'1401': {
		MINDAMAGE: 10,
		MAXDAMAGE: 12,
		SPEED: 53,
		REQSTR: 10,
		SWING: [0xa],
		HITSOUND: [0x23b, 0x23c],
		MISSSOUND: [ 0x23a, 0x239 ]
	},
	# Lance 1
	'26ca': {
		MINDAMAGE: 17,
		MAXDAMAGE: 18,
		SPEED: 24,
		REQSTR: 95,
		SWING: [0xe],
		HITSOUND: [0x23b, 0x23c],
		MISSSOUND: [ 0x23a, 0x239 ]
	},
	# Pike 1
	'26be': {
		MINDAMAGE: 14,
		MAXDAMAGE: 16,
		SPEED: 37,
		REQSTR: 50,
		SWING: [0xe],
		HITSOUND: [0x23b, 0x23c],
		MISSSOUND: [ 0x23a, 0x239 ]
	},
	# Pitchfork 1
	'e87': {
		MINDAMAGE: 13,
		MAXDAMAGE: 14,
		SPEED: 43,
		REQSTR: 50,
		SWING: [0xe],
		HITSOUND: [0x23b, 0x23c],
		MISSSOUND: [ 0x23a, 0x239 ]
	},
	# Short Spear 1
	'1403': {
		MINDAMAGE: 10,
		MAXDAMAGE: 13,
		SPEED: 55,
		REQSTR: 40,
		SWING: [0xe],
		HITSOUND: [0x23b, 0x23c],
		MISSSOUND: [ 0x23a, 0x239 ]
	},
	# Spear 1
	'f62': {
		MINDAMAGE: 13,
		MAXDAMAGE: 15,
		SPEED: 42,
		REQSTR: 50,
		SWING: [0xe],
		HITSOUND: [0x23b, 0x23c],
		MISSSOUND: [ 0x23a, 0x239 ]
	},
	# War Fork 1
	'1405': {
		MINDAMAGE: 12,
		MAXDAMAGE: 13,
		SPEED: 43,
		REQSTR: 35,
		SWING: [0xa],
		HITSOUND: [0x23b, 0x23c, 0x237],
		MISSSOUND: [ 0x23a, 0x239, 0x238 ]
	},
	# Black Staff 1
	'df1': {
		MINDAMAGE: 13,
		MAXDAMAGE: 16,
		SPEED: 39,
		REQSTR: 35,
		SWING: [0xc],
		HITSOUND: [0x233, 0x232],
		MISSSOUND: [ 0x23a, 0x239 ]
	},
	# Club 1
	'13b4': {
		MINDAMAGE: 11,
		MAXDAMAGE: 13,
		SPEED: 44,
		REQSTR: 40,
		SWING: [0x9],
		HITSOUND: [0x233, 0x232],
		MISSSOUND: [ 0x23a, 0x239 ]
	},
	# Gnarled Staff 1
	'13f8': {
		MINDAMAGE: 15,
		MAXDAMAGE: 17,
		SPEED: 33,
		REQSTR: 20,
		SWING: [0xc],
		HITSOUND: [0x233, 0x232],
		MISSSOUND: [ 0x23a, 0x239 ]
	},
	# Hammer Pick 1
	'143d': {
		MINDAMAGE: 15,
		MAXDAMAGE: 17,
		SPEED: 28,
		REQSTR: 45,
		SWING: [0x9],
		HITSOUND: [0x233, 0x232],
		MISSSOUND: [ 0x23a, 0x239 ]
	},
	# Mace 1
	'f5c': {
		MINDAMAGE: 12,
		MAXDAMAGE: 14,
		SPEED: 40,
		REQSTR: 45,
		SWING: [0x9],
		HITSOUND: [0x233, 0x232],
		MISSSOUND: [ 0x23a, 0x239 ]
	},
	# Maul 1
	'143b': {
		MINDAMAGE: 14,
		MAXDAMAGE: 16,
		SPEED: 32,
		REQSTR: 45,
		SWING: [0x9],
		HITSOUND: [0x233, 0x232],
		MISSSOUND: [ 0x23a, 0x239 ]
	},
	# Quarter Staff 1
	'e89': {
		MINDAMAGE: 11,
		MAXDAMAGE: 14,
		SPEED: 48,
		REQSTR: 35,
		SWING: [0xc],
		HITSOUND: [0x233, 0x232],
		MISSSOUND: [ 0x23a, 0x239 ]
	},
	# Scepter 1
	'26c6': {
		MINDAMAGE: 14,
		MAXDAMAGE: 17,
		SPEED: 30,
		REQSTR: 40,
		SWING: [0x9],
		HITSOUND: [0x233, 0x232],
		MISSSOUND: [ 0x23a, 0x239 ]
	},
	# Shepherds Crook 1
	'13f5': {
		MINDAMAGE: 13,
		MAXDAMAGE: 15,
		SPEED: 40,
		REQSTR: 20,
		SWING: [ 0xc ],
		HITSOUND: [0x233, 0x232],
		MISSSOUND: [ 0x23a, 0x239 ]
	},
	# War Axe 1
	'13b0': {
		MINDAMAGE: 14,
		MAXDAMAGE: 15,
		SPEED: 33,
		REQSTR: 35,
		SWING: [ 0xd ],
		HITSOUND: [0x233, 0x232],
		MISSSOUND: [ 0x23a, 0x239 ]
	},
	# War Hammer 1
	'1439': {
		MINDAMAGE: 17,
		MAXDAMAGE: 18,
		SPEED: 28,
		REQSTR: 95,
		SWING: [ 0xc ],
		HITSOUND: [0x233, 0x232],
		MISSSOUND: [ 0x23a, 0x239 ]
	},
	# War Mace 1
	'1407': {
		MINDAMAGE: 16,
		MAXDAMAGE: 17,
		SPEED: 26,
		REQSTR: 80,
		SWING: [0x9],
		HITSOUND: [0x233, 0x232],
		MISSSOUND: [ 0x23a, 0x239 ]
	},
	# Smithing Hammer 1
	'13e3': {
		MINDAMAGE: 13,
		MAXDAMAGE: 15,
		SPEED: 33,
		REQSTR: 30,
		SWING: [0x9],
		HITSOUND: [0x233, 0x232],
		MISSSOUND: [ 0x23a, 0x239 ]
	},
	# Sledge Hammer 1
	'fb4': {
		MINDAMAGE: 13,
		MAXDAMAGE: 15,
		SPEED: 33,
		REQSTR: 40,
		SWING: [0x9],
		HITSOUND: [0x233, 0x232],
		MISSSOUND: [ 0x23a, 0x239 ]
	}
}
