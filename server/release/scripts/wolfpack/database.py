
"""
	\library wolfpack.database
	\description Contains several server functions for databases.
"""

import _wolfpack.database

ACCOUNTS = 1
WORLD = 2

# Let's make sure these tables are in sync with the current database
SQLITE_ACCOUNTS = "accounts"
MYSQL_ACCOUNTS = "`accounts`"
SQLITE_WORLD = "characters, corpses, corpses_equipment, effects, effects_properties, guilds, guilds_canidates, guilds_members, items, npcs, players, settings, skills, spawnregions, tags, uobject, uobjectmap"
MYSQL_WORLD = "`characters`, `corpses`, `corpses_equipment`, `effects`, `effects_properties`, `guilds`, `guilds_canidates`, `guilds_members`, `items`, `npcs`, `players`, `settings`, `skills`, `spawnregions`, `tags`, `uobject`, `uobjectmap`"

query = _wolfpack.database.query
execute = _wolfpack.database.execute
driver = _wolfpack.database.driver
open = _wolfpack.database.open
close = _wolfpack.database.close
