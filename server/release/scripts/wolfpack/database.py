
"""
	\library wolfpack.database
	\description Contains several server functions for databases.
"""

import _wolfpack.database

ACCOUNTS = 1
WORLD = 2

query = _wolfpack.database.query
execute = _wolfpack.database.execute
driver = _wolfpack.database.driver
open = _wolfpack.database.open
close = _wolfpack.database.close
