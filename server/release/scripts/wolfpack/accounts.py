
"""
	\library wolfpack.accounts
	\description Contains several server functions for accounts.
"""

import _wolfpack.accounts

d = globals()
for (name, obj) in _wolfpack.accounts.__dict__.items():
	d[name] = obj
