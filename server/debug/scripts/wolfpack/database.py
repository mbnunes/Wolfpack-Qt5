
"""
	\library wolfpack.database
	\description Contains several server functions for databases.
"""

import _wolfpack.database

"""
	\constants wolfpack.database Database Constants
	These constants identify one of the databases.
"""
ACCOUNTS = 1
WORLD = 2
"""
	\end
"""

# Let's make sure these tables are in sync with the current database
SQLITE_ACCOUNTS = "accounts"
MYSQL_ACCOUNTS = "`accounts`"
SQLITE_WORLD = "characters, corpses, corpses_equipment, effects, effects_properties, guilds, guilds_canidates, guilds_members, items, npcs, players, settings, skills, spawnregions, tags, uobject, uobjectmap"
MYSQL_WORLD = "`characters`, `corpses`, `corpses_equipment`, `effects`, `effects_properties`, `guilds`, `guilds_canidates`, `guilds_members`, `items`, `npcs`, `players`, `settings`, `skills`, `spawnregions`, `tags`, `uobject`, `uobjectmap`"

d = globals()
for (name, obj) in _wolfpack.database.__dict__.items():
	d[name] = obj
