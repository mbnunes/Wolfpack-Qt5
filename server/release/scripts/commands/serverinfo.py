#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: Dreoth                         #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification:                         #
#################################################################

"""
	\command serverinfo
	\description Show a gump with server information.
	\note The amount of memory has to be set in the script.
"""

import wolfpack
import wolfpack.accounts
from wolfpack.gumps import *
from wolfpack.time import *
from wolfpack.sockets import *
import platform

shardname = 'Wolfpack Shard' # Shard name?
operatingsystem = platform.system() + " " +  platform.release() + " [" + platform.machine() + "]" # Windows? Linux?
processor = platform.processor() # example: 1GHz AMD Athlon(tm)
# If someone finds cross platform memory stuff, please let me know!
memory = '512MB PC133' # example: 512MB PC133

upseconds = 0
upminutes = 0
uphours = 0

def serverinfo( socket, command, arguments ):
	char = socket.player
	version = wolfpack.serverversion()
	uptime = wolfpack.currenttime() / 1000
	upseconds = uptime
	upminutes = ( uptime / 60 )
	upseconds -= ( upminutes * 60 )
	if upseconds <= 9:
		upseconds = '0%s' % ( upseconds )
	uphours = ( upminutes / 60 )
	upminutes -= ( uphours * 60 )
	if upminutes <= 9:
		upminutes = '0%s' % ( upminutes )
	if uphours <= 9:
		uphours = '0%s' % ( uphours )
	timestring = '%s:%s:%s' % ( uphours, upminutes, upseconds )

	onlinecount = wolfpack.sockets.count()
	accountcount = 0
	admincount = 0
	gmcount = 0
	seercount = 0
	counselorcount = 0
	playercount = 0
	acclist = wolfpack.accounts.list
	for i in acclist():
		record = wolfpack.accounts.find( i )
		accountcount += 1
		if record.acl == 'admin':
			admincount += 1
		if record.acl == 'gm':
			gmcount += 1
		if record.acl == 'seer':
			seercount += 1
		if record.acl == 'counselor':
			counselorcount += 1
		if record.acl == 'player':
			playercount += 1

	gump = cGump( x=30, y=30)
	gump.addBackground( id=0x2436, width=280, height=400 )
	# Basic Info
	text = '<basefont color="#FFFF00"><center><h3>%s Shard Status</h3></center><br />' % ( shardname )
	text += '<basefont color="#FEFEFE">'
	text += '  Version: %s<br />' % ( version )
	text += '  Uptime: %s<br />' % ( timestring )
	# Account Info
	text += '<basefont color="#FFFF00"><center><h3>Account Information</h3></center><br />'
	text += '<basefont color="#FEFEFE">'
	text += '  Online Accounts:  <basefont color="#00FFFF">%i <basefont color="#FEFEFE">/ <basefont color="#00FFFF">%i<br />' % ( onlinecount, accountcount )
	text += '<basefont color="#FEFEFE">'
	text += '  Total Admins:     <basefont color="#00FF00">%i<br />' % ( admincount )
	text += '<basefont color="#FEFEFE">'
	text += '  Total Seers:      <basefont color="#00FF00">%i<br />' % ( seercount )
	text += '<basefont color="#FEFEFE">'
	text += '  Total Counselors: <basefont color="#00FF00">%i<br />' % ( counselorcount )
	text += '<basefont color="#FEFEFE">'
	text += '  Total Players:    <basefont color="#00FF00">%i<br />' % ( playercount )
	text += '<basefont color="#FEFEFE">'
	text += '  Total Characters: <basefont color="#00FF00">%i<br />' % ( wolfpack.charcount() )
	text += '<basefont color="#FEFEFE">'
	text += '  Total Items:      <basefont color="#00FF00">%i<br />' % ( wolfpack.itemcount() )
	# Hardware Info
	text += '<basefont color="#FFFF00"><center><h3>Server Information</h3></center><br />'
	text += '<basefont color="#FEFEFE">'
	text += ' OS: %s<br />' % ( operatingsystem )
	text += ' Processor: %s<br />' % ( processor )
	text += ' Memory: %s<br />' % ( memory )

	gump.addHtmlGump( x=20, y=20, width=240, height=360, html=text )
	gump.send( char )

def onLoad():
	wolfpack.registercommand( "serverinfo", serverinfo )
