
from wolfpack.consts import *

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
		LOWERREQS: 50
	},
	'shadowiron': {
		DAMAGE_FIRE: 0,
		DAMAGE_COLD: 20,
		DAMAGE_POISON: 0,
		DAMAGE_ENERGY: 0
	},
	'copper': {
		DAMAGE_FIRE: 0,
		DAMAGE_COLD: 0,
		DAMAGE_POISON: 10,
		DAMAGE_ENERGY: 20
	},
	'bronze': {
		DAMAGE_FIRE: 40,
		DAMAGE_COLD: 0,
		DAMAGE_POISON: 0,
		DAMAGE_ENERGY: 0
	},
	'gold': {
		DAMAGE_FIRE: 0,
		DAMAGE_COLD: 0,
		DAMAGE_POISON: 0,
		DAMAGE_ENERGY: 0,
		LUCK: 40,
		LOWERREQS: 50
	},
	'agapite': {
		DAMAGE_FIRE: 0,
		DAMAGE_COLD: 30,
		DAMAGE_POISON: 0,
		DAMAGE_ENERGY: 20
	},
	'verite': {
		DAMAGE_FIRE: 0,
		DAMAGE_COLD: 0,
		DAMAGE_POISON: 40,
		DAMAGE_ENERGY: 20
	},
	'valorite': {
		DAMAGE_FIRE: 10,
		DAMAGE_COLD: 20,
		DAMAGE_POISON: 10,
		DAMAGE_ENERGY: 20
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
    PROJECTILE: 0xf42,
    AMMUNITION: 'f3f',
    HITSOUND: [ 0x234 ],
    MISSSOUND: [ 0x238 ]
  },
  # Bow 2
  '13b2': {
    MINDAMAGE: 16,
    MAXDAMAGE: 18,
    SPEED: 25,
    PROJECTILE: 0xf42,
    AMMUNITION: 'f3f',
    HITSOUND: [ 0x234 ],
    MISSSOUND: [ 0x238 ]
  },
  # Crossbow 1
  'f4f': {
    MINDAMAGE: 18,
    MAXDAMAGE: 20,
    SPEED: 24,
    PROJECTILE: 0x1bfe,
    AMMUNITION: '1bfb',
    HITSOUND: [ 0x234 ],
    MISSSOUND: [ 0x238 ]
  },
  # Crossbow 2
  'f50': {
    MINDAMAGE: 18,
    MAXDAMAGE: 20,
    SPEED: 24,
    PROJECTILE: 0x1bfe,
    AMMUNITION: '1bfb',
    HITSOUND: [ 0x234 ],
    MISSSOUND: [ 0x238 ]
  },
  # Heavy Crossbow 1
  '13fd': {
    MINDAMAGE: 19,
    MAXDAMAGE: 20,
    SPEED: 22,
    PROJECTILE: 0x1bfe,
    AMMUNITION: '1bfb',
    HITSOUND: [ 0x234 ],
    MISSSOUND: [ 0x238 ]
  },
  # Heavy Crossbow 2
  '13fc': {
    MINDAMAGE: 19,
    MAXDAMAGE: 20,
    SPEED: 22,
    PROJECTILE: 0x1bfe,
    AMMUNITION: '1bfb',
    HITSOUND: [ 0x234 ],
    MISSSOUND: [ 0x238 ]
  },
  # Composite Bow 1
  '26c2': {
    MINDAMAGE: 15,
    MAXDAMAGE: 17,
    SPEED: 25,
    PROJECTILE: 0xf42,
    AMMUNITION: 'f3f',
    HITSOUND: [ 0x234 ],
    MISSSOUND: [ 0x238 ]
  },
  # Composite Bow 2
  '26cc': {
    MINDAMAGE: 15,
    MAXDAMAGE: 17,
    SPEED: 25,
    PROJECTILE: 0xf42,
    AMMUNITION: 'f3f',
    HITSOUND: [ 0x234 ],
    MISSSOUND: [ 0x238 ]
  },
  # Repeating Crossbow 1
  '26cd': {
    MINDAMAGE: 30,
    MAXDAMAGE: 49,
    SPEED: 41,
    PROJECTILE: 0x1bfe,
    AMMUNITION: '1bfb',
    HITSOUND: [ 0x234 ],
    MISSSOUND: [ 0x238 ]
  },
  # Repeating Crossbow 2
  '26c3': {
    MINDAMAGE: 30,
    MAXDAMAGE: 49,
    SPEED: 41,
    PROJECTILE: 0x1bfe,
    AMMUNITION: '1bfb',
    HITSOUND: [ 0x234 ],
    MISSSOUND: [ 0x238 ]
  },
  # Katana 1
  '13ff': {
    MINDAMAGE: 11,
    MAXDAMAGE: 13,
    SPEED: 46,
    HITSOUND: [0x23b, 0x23c],
    MISSSOUND: [ 0x23a, 0x238, 0x239 ]
  },
  # Katana 2
  '13fe': {
    MINDAMAGE: 11,
    MAXDAMAGE: 13,
    SPEED: 46,
    HITSOUND: [0x23b, 0x23c],
    MISSSOUND: [ 0x23a, 0x238, 0x239 ]
  },
  # Axe 1
  'f49': {
    MINDAMAGE: 14,
    MAXDAMAGE: 16,
    SPEED: 37,
    HITSOUND: [0x233, 0x232],
    MISSSOUND: [ 0x23a, 0x239 ]
  },
  # Axe 2
  'f4a': {
    MINDAMAGE: 14,
    MAXDAMAGE: 16,
    SPEED: 37,
    HITSOUND: [0x233, 0x232],
    MISSSOUND: [ 0x23a, 0x239 ]
  },
  # Bardiche 1
  'f4d': {
    MINDAMAGE: 17,
    MAXDAMAGE: 18,
    SPEED: 28,
    HITSOUND: [ 0x237 ],
    MISSSOUND: [ 0x23a, 0x238, 0x239 ]
  },
  # Bardiche 2
  'f4e': {
    MINDAMAGE: 17,
    MAXDAMAGE: 18,
    SPEED: 28,
    HITSOUND: [ 0x237 ],
    MISSSOUND: [ 0x23a, 0x238, 0x239 ]
  },
  # Battle Axe 1
  'f47': {
    MINDAMAGE: 15,
    MAXDAMAGE: 17,
    SPEED: 31,
    HITSOUND: [0x233, 0x232],
    MISSSOUND: [ 0x23a, 0x239 ]
  },
  # Battle Axe 2
  'f48': {
    MINDAMAGE: 15,
    MAXDAMAGE: 17,
    SPEED: 31,
    HITSOUND: [0x233, 0x232],
    MISSSOUND: [ 0x23a, 0x239 ]
  },
  # Bladded Staff 1
  '26c7': {
    MINDAMAGE: 14,
    MAXDAMAGE: 16,
    SPEED: 37,
    HITSOUND: [0x23b, 0x23c],
    MISSSOUND: [ 0x239 ]
  },
  # Bladed Staff 2
  '26bd': {
    MINDAMAGE: 14,
    MAXDAMAGE: 16,
    SPEED: 37,
    HITSOUND: [0x23b, 0x23c],
    MISSSOUND: [ 0x239 ]
  },
  # Bone Harvester 1
  '26c5': {
    MINDAMAGE: 13,
    MAXDAMAGE: 15,
    SPEED: 36,
    HITSOUND: [0x23b, 0x23c],
    MISSSOUND: [ 0x239 ]
  },
  # Bone Harvester 2
  '26bb': {
    MINDAMAGE: 13,
    MAXDAMAGE: 15,
    SPEED: 36,
    HITSOUND: [0x23b, 0x23c],
    MISSSOUND: [ 0x239 ]
  },
  # Broadsword 1
  'f5e': {
    MINDAMAGE: 14,
    MAXDAMAGE: 15,
    SPEED: 33,
    HITSOUND: [0x23b, 0x23c, 0x237],
    MISSSOUND: [ 0x23a, 0x238, 0x239 ]
  },
  # Broadsword 2
  'f5f': {
    MINDAMAGE: 14,
    MAXDAMAGE: 15,
    SPEED: 33,
    HITSOUND: [0x23b, 0x23c, 0x237],
    MISSSOUND: [ 0x23a, 0x238, 0x239 ]
  },
  # Butcher Knife 1
  '13f6': {
    MINDAMAGE: 9,
    MAXDAMAGE: 11,
    SPEED: 49,
    HITSOUND: [0x23b, 0x23c],
    MISSSOUND: [ 0x23a, 0x238, 0x239 ]
  },
  # Butcher Knife 2
  '13f7': {
    MINDAMAGE: 9,
    MAXDAMAGE: 11,
    SPEED: 49,
    HITSOUND: [0x23b, 0x23c],
    MISSSOUND: [ 0x23a, 0x238, 0x239 ]
  },
  # Cleaver 1
  'ec3': {
    MINDAMAGE: 11,
    MAXDAMAGE: 13,
    SPEED: 46,
    HITSOUND: [0x23b, 0x23c],
    MISSSOUND: [ 0x23a, 0x238, 0x239 ]
  },
  # Cleaver 2
  'ec2': {
    MINDAMAGE: 11,
    MAXDAMAGE: 13,
    SPEED: 46,
    HITSOUND: [0x23b, 0x23c],
    MISSSOUND: [ 0x23a, 0x238, 0x239 ]
  },
  # Crescent Blade 1
  '26cb': {
    MINDAMAGE: 11,
    MAXDAMAGE: 14,
    SPEED: 47,
    HITSOUND: [0x23b, 0x23c],
    MISSSOUND: [ 0x239 ]
  },
  # Crescent Blade 2
  '26c1': {
    MINDAMAGE: 11,
    MAXDAMAGE: 14,
    SPEED: 47,
    HITSOUND: [0x23b, 0x23c],
    MISSSOUND: [ 0x239 ]
  },
  # Cutlass 1
  '1441': {
    MINDAMAGE: 11,
    MAXDAMAGE: 13,
    SPEED: 44,
    HITSOUND: [0x23b, 0x23c],
    MISSSOUND: [ 0x23a, 0x238, 0x239 ]
  },
  # Cutlass 2
  '1440': {
    MINDAMAGE: 11,
    MAXDAMAGE: 13,
    SPEED: 44,
    HITSOUND: [0x23b, 0x23c],
    MISSSOUND: [ 0x23a, 0x238, 0x239 ]
  },
  # Double Axe 1
  'f4b': {
    MINDAMAGE: 15,
    MAXDAMAGE: 17,
    SPEED: 33,
    HITSOUND: [0x233, 0x232],
    MISSSOUND: [ 0x23a, 0x239 ]
  },
  # Double Axe 2
  'f4c': {
    MINDAMAGE: 15,
    MAXDAMAGE: 17,
    SPEED: 33,
    HITSOUND: [0x233, 0x232],
    MISSSOUND: [ 0x23a, 0x239 ]
  },
  # Executioners Axe 1
  'f45': {
    MINDAMAGE: 15,
    MAXDAMAGE: 17,
    SPEED: 33,
    HITSOUND: [0x233, 0x232],
    MISSSOUND: [ 0x23a, 0x239 ]
  },
  # Executioners Axe 2
  'f46': {
    MINDAMAGE: 15,
    MAXDAMAGE: 17,
    SPEED: 33,
    HITSOUND: [0x233, 0x232],
    MISSSOUND: [ 0x23a, 0x239 ]
  },
  # Halberd 1
  '143f': {
    MINDAMAGE: 18,
    MAXDAMAGE: 19,
    SPEED: 25,
    HITSOUND: [ 0x237 ],
    MISSSOUND: [ 0x23a, 0x238, 0x239 ]
  },
  # Halberd 2
  '143e': {
    MINDAMAGE: 18,
    MAXDAMAGE: 19,
    SPEED: 25,
    HITSOUND: [ 0x237 ],
    MISSSOUND: [ 0x23a, 0x238, 0x239 ]
  },
  # Hatchet 1
  'f43': {
    MINDAMAGE: 13,
    MAXDAMAGE: 15,
    SPEED: 41,
    HITSOUND: [0x233, 0x232],
    MISSSOUND: [ 0x23a, 0x239 ]
  },
  # Hatchet 2
  'f44': {
    MINDAMAGE: 13,
    MAXDAMAGE: 15,
    SPEED: 41,
    HITSOUND: [0x233, 0x232],
    MISSSOUND: [ 0x23a, 0x239 ]
  },
  # Large Battle Axe 1
  '13fb': {
    MINDAMAGE: 16,
    MAXDAMAGE: 17,
    SPEED: 29,
    HITSOUND: [0x233, 0x232],
    MISSSOUND: [ 0x23a, 0x239 ]
  },
  # Large Battle Axe 2
  '13fa': {
    MINDAMAGE: 16,
    MAXDAMAGE: 17,
    SPEED: 29,
    HITSOUND: [0x233, 0x232],
    MISSSOUND: [ 0x23a, 0x239 ]
  },
  # Long Sword 1
  '13b8': {
    MINDAMAGE: 15,
    MAXDAMAGE: 16,
    SPEED: 30,
    HITSOUND: [0x23b, 0x23c],
    MISSSOUND: [ 0x23a, 0x238, 0x239 ]
  },
  # Long Sword 2
  '13b7': {
    MINDAMAGE: 15,
    MAXDAMAGE: 16,
    SPEED: 30,
    HITSOUND: [0x23b, 0x23c],
    MISSSOUND: [ 0x23a, 0x238, 0x239 ]
  },
  # Longsword 1
  'f61': {
    MINDAMAGE: 15,
    MAXDAMAGE: 16,
    SPEED: 30,
    HITSOUND: [0x23b, 0x23c],
    MISSSOUND: [ 0x23a, 0x238, 0x239 ]
  },
  # Longsword 2
  'f60': {
    MINDAMAGE: 15,
    MAXDAMAGE: 16,
    SPEED: 30,
    HITSOUND: [0x23b, 0x23c],
    MISSSOUND: [ 0x23a, 0x238, 0x239 ]
  },
  # Paladin Sword 1
  '26cf': {
    MINDAMAGE: 14,
    MAXDAMAGE: 15,
    SPEED: 33,
    HITSOUND: [0x23b, 0x23c, 0x237],
    MISSSOUND: [ 0x23a, 0x238, 0x239 ]
  },
  # Paladin Sword 2
  '26ce': {
    MINDAMAGE: 14,
    MAXDAMAGE: 15,
    SPEED: 33,
    HITSOUND: [0x23b, 0x23c, 0x237],
    MISSSOUND: [ 0x23a, 0x238, 0x239 ]
  },
  # Pickaxe 1
  'e86': {
    MINDAMAGE: 13,
    MAXDAMAGE: 15,
    SPEED: 35,
    HITSOUND: [0x233, 0x232],
    MISSSOUND: [ 0x23a, 0x239 ]
  },
  # Pickaxe 2
  'e85': {
    MINDAMAGE: 13,
    MAXDAMAGE: 15,
    SPEED: 35,
    HITSOUND: [0x233, 0x232],
    MISSSOUND: [ 0x23a, 0x239 ]
  },
  # Scimitar 1
  '13b6': {
    MINDAMAGE: 13,
    MAXDAMAGE: 15,
    SPEED: 37,
    HITSOUND: [0x23b, 0x23c],
    MISSSOUND: [ 0x23a, 0x238, 0x239 ]
  },
  # Scimitar 2
  'f13b5': {
    MINDAMAGE: 13,
    MAXDAMAGE: 15,
    SPEED: 37,
    HITSOUND: [0x23b, 0x23c],
    MISSSOUND: [ 0x23a, 0x238, 0x239 ]
  },
  # Scythe 1
  '26c4': {
    MINDAMAGE: 15,
    MAXDAMAGE: 18,
    SPEED: 32,
    HITSOUND: [0x23b, 0x23c],
    MISSSOUND: [ 0x23a, 0x238, 0x239 ]
  },
  # Scythe 2
  '26ba': {
    MINDAMAGE: 15,
    MAXDAMAGE: 18,
    SPEED: 32,
    HITSOUND: [0x23b, 0x23c],
    MISSSOUND: [ 0x23a, 0x238, 0x239 ]
  },
  # Skinning Knife 1
  'ec4': {
    MINDAMAGE: 9,
    MAXDAMAGE: 11,
    SPEED: 49,
    HITSOUND: [0x23b, 0x23c],
    MISSSOUND: [ 0x23a, 0x238, 0x239 ]
  },
  # Skinning Knife 2
  'ec5': {
    MINDAMAGE: 9,
    MAXDAMAGE: 11,
    SPEED: 49,
    HITSOUND: [0x23b, 0x23c],
    MISSSOUND: [ 0x23a, 0x238, 0x239 ]
  },
  # Two Handed Axe 1
  '1443': {
    MINDAMAGE: 16,
    MAXDAMAGE: 17,
    SPEED: 31,
    HITSOUND: [0x233, 0x232],
    MISSSOUND: [ 0x23a, 0x239 ]
  },
  # Two Handed Axe 2
  '1442': {
    MINDAMAGE: 16,
    MAXDAMAGE: 17,
    SPEED: 31,
    HITSOUND: [0x233, 0x232],
    MISSSOUND: [ 0x23a, 0x239 ]
  },
  # Viking Sword 1
  '13b9': {
    MINDAMAGE: 15,
    MAXDAMAGE: 17,
    SPEED: 28,
    HITSOUND: [0x23b, 0x23c, 0x237],
    MISSSOUND: [ 0x23a, 0x238, 0x239 ]
  },
  # Viking Sword 2
  '13ba': {
    MINDAMAGE: 15,
    MAXDAMAGE: 17,
    SPEED: 28,
    HITSOUND: [0x23b, 0x23c, 0x237],
    MISSSOUND: [ 0x23a, 0x238, 0x239 ]
  },
  # Dagger 1
  'f51': {
    MINDAMAGE: 10,
    MAXDAMAGE: 11,
    SPEED: 49,
    HITSOUND: [0x23b, 0x23c],
    MISSSOUND: [ 0x23a, 0x239 ]
  },
  # Dagger 2
  'f52': {
    MINDAMAGE: 10,
    MAXDAMAGE: 11,
    SPEED: 49,
    HITSOUND: [0x23b, 0x23c],
    MISSSOUND: [ 0x23a, 0x239 ]
  },
  # Double Bladed Staff 1
  '26c9': {
    MINDAMAGE: 12,
    MAXDAMAGE: 13,
    SPEED: 49,
    HITSOUND: [0x23b, 0x23c],
    MISSSOUND: [ 0x23a, 0x239 ]
  },
  # Double Bladed Staff 2
  '26bf': {
    MINDAMAGE: 12,
    MAXDAMAGE: 13,
    SPEED: 49,
    HITSOUND: [0x23b, 0x23c],
    MISSSOUND: [ 0x23a, 0x239 ]
  },
  # Kryss 1
  '1401': {
    MINDAMAGE: 10,
    MAXDAMAGE: 12,
    SPEED: 53,
    HITSOUND: [0x23b, 0x23c],
    MISSSOUND: [ 0x23a, 0x239 ]
  },
  # Kryss 2
  '1400': {
    MINDAMAGE: 10,
    MAXDAMAGE: 12,
    SPEED: 53,
    HITSOUND: [0x23b, 0x23c],
    MISSSOUND: [ 0x23a, 0x239 ]
  },
  # Lance 1
  '26ca': {
    MINDAMAGE: 17,
    MAXDAMAGE: 18,
    SPEED: 24,
    HITSOUND: [0x23b, 0x23c],
    MISSSOUND: [ 0x23a, 0x239 ]
  },
  # Lance 2
  '26c0': {
    MINDAMAGE: 17,
    MAXDAMAGE: 18,
    SPEED: 24,
    HITSOUND: [0x23b, 0x23c],
    MISSSOUND: [ 0x23a, 0x239 ]
  },
  # Pike 1
  '26be': {
    MINDAMAGE: 14,
    MAXDAMAGE: 16,
    SPEED: 37,
    HITSOUND: [0x23b, 0x23c],
    MISSSOUND: [ 0x23a, 0x239 ]
  },
  # Pike 2
  '26c8': {
    MINDAMAGE: 14,
    MAXDAMAGE: 16,
    SPEED: 37,
    HITSOUND: [0x23b, 0x23c],
    MISSSOUND: [ 0x23a, 0x239 ]
  },
  # Pitchfork 1
  'e87': {
    MINDAMAGE: 13,
    MAXDAMAGE: 14,
    SPEED: 43,
    HITSOUND: [0x23b, 0x23c],
    MISSSOUND: [ 0x23a, 0x239 ]
  },
  # Pitchfork 2
  'e88': {
    MINDAMAGE: 13,
    MAXDAMAGE: 14,
    SPEED: 43,
    HITSOUND: [0x23b, 0x23c],
    MISSSOUND: [ 0x23a, 0x239 ]
  },
  # Short Spear 1
  '1403': {
    MINDAMAGE: 10,
    MAXDAMAGE: 13,
    SPEED: 55,
    HITSOUND: [0x23b, 0x23c],
    MISSSOUND: [ 0x23a, 0x239 ]
  },
  # Short Spear 2
  '1402': {
    MINDAMAGE: 10,
    MAXDAMAGE: 13,
    SPEED: 55,
    HITSOUND: [0x23b, 0x23c],
    MISSSOUND: [ 0x23a, 0x239 ]
  },
  # Spear 1
  'f62': {
    MINDAMAGE: 13,
    MAXDAMAGE: 15,
    SPEED: 42,
    HITSOUND: [0x23b, 0x23c],
    MISSSOUND: [ 0x23a, 0x239 ]
  },
  # Spear 2
  'f63': {
    MINDAMAGE: 13,
    MAXDAMAGE: 15,
    SPEED: 42,
    HITSOUND: [0x23b, 0x23c],
    MISSSOUND: [ 0x23a, 0x239 ]
  },
  # War Fork 1
  '1405': {
    MINDAMAGE: 12,
    MAXDAMAGE: 13,
    SPEED: 43,
    HITSOUND: [0x23b, 0x23c, 0x237],
    MISSSOUND: [ 0x23a, 0x239, 0x238 ]
  },
  # War Fork 2
  '1404': {
    MINDAMAGE: 12,
    MAXDAMAGE: 13,
    SPEED: 43,
    HITSOUND: [0x23b, 0x23c, 0x237],
    MISSSOUND: [ 0x23a, 0x239, 0x238 ]
  },
  # Black Staff 1
  'df1': {
    MINDAMAGE: 13,
    MAXDAMAGE: 16,
    SPEED: 39,
    HITSOUND: [0x233, 0x232],
    MISSSOUND: [ 0x23a, 0x239 ]
  },
  # Black Staff 2
  'df0': {
    MINDAMAGE: 13,
    MAXDAMAGE: 16,
    SPEED: 39,
    HITSOUND: [0x233, 0x232],
    MISSSOUND: [ 0x23a, 0x239 ]
  },
  # Club 1
  '13b4': {
    MINDAMAGE: 11,
    MAXDAMAGE: 13,
    SPEED: 44,
    HITSOUND: [0x233, 0x232],
    MISSSOUND: [ 0x23a, 0x239 ]
  },
  # Club 2
  '13b3': {
    MINDAMAGE: 11,
    MAXDAMAGE: 13,
    SPEED: 44,
    HITSOUND: [0x233, 0x232],
    MISSSOUND: [ 0x23a, 0x239 ]
  },
  # Gnarled Staff 1
  '13f8': {
    MINDAMAGE: 15,
    MAXDAMAGE: 17,
    SPEED: 33,
    HITSOUND: [0x233, 0x232],
    MISSSOUND: [ 0x23a, 0x239 ]
  },
  # Gnarled Staff 2
  '13f9': {
    MINDAMAGE: 15,
    MAXDAMAGE: 17,
    SPEED: 33,
    HITSOUND: [0x233, 0x232],
    MISSSOUND: [ 0x23a, 0x239 ]
  },
  # Hammer Pick 1
  '143d': {
    MINDAMAGE: 15,
    MAXDAMAGE: 17,
    SPEED: 28,
    HITSOUND: [0x233, 0x232],
    MISSSOUND: [ 0x23a, 0x239 ]
  },
  # Hammer Pick 2
  '143c': {
    MINDAMAGE: 15,
    MAXDAMAGE: 17,
    SPEED: 28,
    HITSOUND: [0x233, 0x232],
    MISSSOUND: [ 0x23a, 0x239 ]
  },
  # Mace 1
  'f5c': {
    MINDAMAGE: 12,
    MAXDAMAGE: 14,
    SPEED: 40,
    HITSOUND: [0x233, 0x232],
    MISSSOUND: [ 0x23a, 0x239 ]
  },
  # Mace 2
  'f5d': {
    MINDAMAGE: 12,
    MAXDAMAGE: 14,
    SPEED: 40,
    HITSOUND: [0x233, 0x232],
    MISSSOUND: [ 0x23a, 0x239 ]
  },
  # Maul 1
  '143b': {
    MINDAMAGE: 14,
    MAXDAMAGE: 16,
    SPEED: 32,
    HITSOUND: [0x233, 0x232],
    MISSSOUND: [ 0x23a, 0x239 ]
  },
  # Maul 2
  '143a': {
    MINDAMAGE: 14,
    MAXDAMAGE: 16,
    SPEED: 32,
    HITSOUND: [0x233, 0x232],
    MISSSOUND: [ 0x23a, 0x239 ]
  },
  # Quarter Staff 1
  'e89': {
    MINDAMAGE: 11,
    MAXDAMAGE: 14,
    SPEED: 48,
    HITSOUND: [0x233, 0x232],
    MISSSOUND: [ 0x23a, 0x239 ]
  },
  # Quarter Staff 2
  'e8a': {
    MINDAMAGE: 11,
    MAXDAMAGE: 14,
    SPEED: 48,
    HITSOUND: [0x233, 0x232],
    MISSSOUND: [ 0x23a, 0x239 ]
  },
  # Scepter 1
  '26c6': {
    MINDAMAGE: 14,
    MAXDAMAGE: 17,
    SPEED: 30,
    HITSOUND: [0x233, 0x232],
    MISSSOUND: [ 0x23a, 0x239 ]
  },
  # Scepter 2
  '26bc': {
    MINDAMAGE: 14,
    MAXDAMAGE: 17,
    SPEED: 30,
    HITSOUND: [0x233, 0x232],
    MISSSOUND: [ 0x23a, 0x239 ]
  },
  # Shepherds Crook 1
  '13f5': {
    MINDAMAGE: 13,
    MAXDAMAGE: 15,
    SPEED: 40,
    HITSOUND: [0x233, 0x232],
    MISSSOUND: [ 0x23a, 0x239 ]
  },
  # Shepherds Crook 2
  '13f4': {
    MINDAMAGE: 13,
    MAXDAMAGE: 15,
    SPEED: 40,
    HITSOUND: [0x233, 0x232],
    MISSSOUND: [ 0x23a, 0x239 ]
  },
  # War Axe 1
  '13b0': {
    MINDAMAGE: 14,
    MAXDAMAGE: 15,
    SPEED: 33,
    HITSOUND: [0x233, 0x232],
    MISSSOUND: [ 0x23a, 0x239 ]
  },
  # War Axe 2
  '13af': {
    MINDAMAGE: 14,
    MAXDAMAGE: 15,
    SPEED: 33,
    HITSOUND: [0x233, 0x232],
    MISSSOUND: [ 0x23a, 0x239 ]
  },
  # War Hammer 1
  '1439': {
    MINDAMAGE: 17,
    MAXDAMAGE: 18,
    SPEED: 28,
    HITSOUND: [0x233, 0x232],
    MISSSOUND: [ 0x23a, 0x239 ]
  },
  # War Hammer 2
  '1438': {
    MINDAMAGE: 17,
    MAXDAMAGE: 18,
    SPEED: 28,
    HITSOUND: [0x233, 0x232],
    MISSSOUND: [ 0x23a, 0x239 ]
  },
  # War Mace 1
  '1407': {
    MINDAMAGE: 16,
    MAXDAMAGE: 17,
    SPEED: 26,
    HITSOUND: [0x233, 0x232],
    MISSSOUND: [ 0x23a, 0x239 ]
  },
  # War Mace 2
  '1406': {
    MINDAMAGE: 16,
    MAXDAMAGE: 17,
    SPEED: 26,
    HITSOUND: [0x233, 0x232],
    MISSSOUND: [ 0x23a, 0x239 ]
  },
  # Smithing Hammer 1
  '13e3': {
    MINDAMAGE: 13,
    MAXDAMAGE: 15,
    SPEED: 33,
    HITSOUND: [0x233, 0x232],
    MISSSOUND: [ 0x23a, 0x239 ]
  },
  # Smithing Hammer 2
  '13e4': {
    MINDAMAGE: 13,
    MAXDAMAGE: 15,
    SPEED: 33,
    HITSOUND: [0x233, 0x232],
    MISSSOUND: [ 0x23a, 0x239 ]
  },
  # Sledge Hammer 1
  'fb4': {
    MINDAMAGE: 13,
    MAXDAMAGE: 15,
    SPEED: 33,
    HITSOUND: [0x233, 0x232],
    MISSSOUND: [ 0x23a, 0x239 ]
  },
  # Sledge Hammer 2
  'fb5': {
    MINDAMAGE: 13,
    MAXDAMAGE: 15,
    SPEED: 33,
    HITSOUND: [0x233, 0x232],
    MISSSOUND: [ 0x23a, 0x239 ]
  }
}
