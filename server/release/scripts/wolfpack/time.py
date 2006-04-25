
"""
	\library wolfpack.time
	\description Contains several functions for ingame time.
"""

import _wolfpack.time

d = globals()
for (name, obj) in _wolfpack.time.__dict__.items():
	d[name] = obj

servertime = _wolfpack.time.currenttime
