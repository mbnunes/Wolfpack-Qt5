#######################################################
#
# Wrapper for wolfpack.map
#
#######################################################

import _wolfpack

def gettile(x,y):
	return _wolfpack.map_gettile(x,y)
	
def getheight(x,y):
	return _wolfpack.map_getheight(x,y)
