
# Possible and known sections
RES_ACCOUNT = 1					# Define an account instance.
RES_ADVANCE = 2					# Define the advance rates for stats.
RES_AREA = 3						# Complex region. (w/extra tags)
RES_BLOCKEMAIL = 4			# (SL) A list of email addresses we will not accept.
RES_BLOCKIP = 5					# (SL) A list of IP's to block.
RES_BOOK = 6						# A book or a page from a book.
RES_CHARDEF = 7					# Define a char type. (overlap with RES_SPAWN)
RES_COMMENT = 8					# A commented out block type.
RES_CRYSTALBALL = 9		 # (SL) Random (short) tip type messages.
RES_DEFNAME = 10				# (SL) Just add a bunch of new defs and equivs str/values.
RES_DIALOG = 11					# A scriptable gump dialog, text or handler block.
RES_ECONOMY = 12				# Define the prices of a bunch of stuff.
RES_EMAILMSG = 13				# define an email msg that could be sent to an account.
RES_EVENTS = 14					# An Event handler block with the trigger type in it. ON=@Death etc.
RES_FUNCTION = 15				# Define a new command verb script that applies to a char.
RES_GMPAGE = 16					# A GM page. (SAVED in World)
RES_HELP = 17						# Command help blocks. (read in as needed)
RES_ITEMDEF = 18				# Define an item type. (overlap with RES_TEMPLATE)
RES_LOCATION = 19				# Ignore this AXIS data.
RES_MAP = 20						# Define info about the mapx.mul files
RES_MENU = 21						# General scriptable menus.
RES_MOONGATES = 22			# (SL) Define where the moongates are.
RES_NAMES = 23					# A block of possible names for a NPC type. (read as needed)
RES_NEWBIE = 24					# Triggers to execute on Player creation (based on skills selected)
RES_NOTOTITLES = 25			# (SI) Define the noto titles used.
RES_OBSCENE = 26				# (SL) A list of obscene words.
RES_PLEVEL = 27					# Define the list of commands that a PLEVEL can access. (or not access)
RES_PLOTITEM = 28				# (Phase this out in favor of better @trigger TEVENT= and EVENT= support.)
RES_QUEST = 29
RES_RACECLASS = 30			# General race params about creature types. regen rates, etc
RES_REGIONRESOURCE = 31 # Define an Ore type.
RES_REGIONTYPE = 32			# Triggers etc. that can be assinged to a RES_AREA
RES_RESOURCES = 33			# (SL) list of all the resource files we should index !
RES_ROOM = 34						# Non-complex region. (no extra tags)
RES_RUNES = 35					# (SI) Define list of the magic runes.
RES_SCHEDULE = 36				# A list of times things can happen.
RES_SCROLL = 37					# SCROLL_GUEST=message scroll sent to player at guest login. SCROLL_MOTD, SCROLL_NEWBIE
RES_SECTOR = 38					# Make changes to a sector. (SAVED in World)
RES_SERVER = 39					# Define a peer sphere server we should link to. (SAVED in World)
RES_SERVERS = 40				# List a number of servers in 3 line format. (Phase this out)
RES_SKILL = 41					# Define attributes for a skill (how fast it raises etc)
RES_SKILLCLASS = 42			# Define specifics for a char with this skill class. (ex. skill caps)
RES_SKILLMENU = 43			# A menu that is attached to a skill. special arguments over other menus.
RES_SPAWN = 44					# Define a list of NPC's and how often they may spawn.
RES_SPEECH = 45					# A speech block with ON=*blah* in it.
RES_SPELL = 46					# Define a magic spell. (0-64 are reserved)
RES_SPHERE = 47					# Main Server INI block
RES_STARTS = 48					# (SI) List of starting locations for newbies.
RES_STAT = 49						# Stats elements like KARMA,STR,DEX,FOOD,FAME,CRIMINAL etc. Used for resource and desire scripts.
RES_TELEPORTERS = 50		# (SL) Where are the teleporters in the world ? dungeon transports etc.
RES_TEMPLATE = 51				# Define lists of items. (for filling loot etc)
RES_TIP = 52						# Tips (similar to RES_SCROLL) that can come up at startup.
RES_TYPEDEF = 53				# Define a trigger block for a RES_WORLDITEM m_type.
RES_TYPEDEFS = 54
RES_WEBPAGE = 55				# Define a web page template.
RES_WORLDCHAR = 56			# Define instance of char in the world. (SAVED in World)
RES_WORLDITEM = 57			# Define instance of item in the world. (SAVED in World)
RES_WORLDSCRIPT = 58		# Something to load into a script.
RES_WORLDVARS = 59
RES_QTY = 60
