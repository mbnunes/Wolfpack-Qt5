
"""
	\library wolfpack.time
	\description Contains several functions for wolfpack time.
"""

import _wolfpack.time

"""
	\function wolfpack.time.minute
	\return Integer
	\description Return the minute of the hour for the current game time. Range: 0 - 59
"""
minute = _wolfpack.time.minute

"""
	\function wolfpack.time.hour
	\return Integer
	\description Return the current hour of the day. Range: 0 - 23
"""
hour = _wolfpack.time.hour

"""
	\function wolfpack.time.days
	\return Integer
	\description Returns the days elapsed since the initialization of the game time.
"""
days = _wolfpack.time.days

"""
	\function wolfpack.time.minutes
	\return Integer
	\description Return the minutes elapsed since the initialization of the game time.
"""
minutes = _wolfpack.time.minutes

"""
	\function wolfpack.time.currenttime
	\return Integer
	\description Return the current servertime in miliseconds since start of the gameserver.
"""
servertime = _wolfpack.currenttime

"""
	\function wolfpack.time.currentdatetime
	\return String
	\description Return the current real date and time as a string.
"""
currentdatetime =  _wolfpack.currentdatetime


"""
	\function wolfpack.time.currentlightlevel
	\return Integer
	\description Return the current outside lightlevel.
"""
currentlightlevel = _wolfpack.time.currentlightlevel
