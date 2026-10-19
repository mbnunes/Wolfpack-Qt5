
import os
import struct
import zlib
import json
import wolfpack
from wolfpack.consts import EVENT_CHCOMMAND, EVENT_CHQUERY
from wolfpack.gumps import cGump
import wolfpack.utilities

#
# Custom Housing System
#

# Global cache of active design sessions:
# sessions[player.serial] = {
#     'multi_serial': multi.serial,
#     'level': 1,
#     'max_levels': 3 or 4,
# }
SESSIONS = {}

# In-memory design state cache:
# DESIGNS[multi_serial] = {
#     'current': list of (item_id, x, y, z),
#     'design': list of (item_id, x, y, z),
#     'backup': list of (item_id, x, y, z),
#     'revision': int,
#     'bounds': (xMin, yMin, xMax, yMax, width, height)
# }
DESIGNS = {}

# Item height cache
TILE_HEIGHTS = {}
TILE_FLOORS = set()

MUL_PATH = '/opt/hd_jogos/UOC/'

def get_tile_height(item_id):
	global TILE_HEIGHTS
	item_id = item_id & 0xFFFF
	if item_id in TILE_HEIGHTS:
		return TILE_HEIGHTS[item_id]
	
	tiledata_file = os.path.join(MUL_PATH, 'tiledata.mul')
	if not os.path.exists(tiledata_file):
		return 0
		
	try:
		with open(tiledata_file, 'rb') as f:
			land_bytes = 493568 # High Seas 7.0.9+ with 0x4000 land tiles
			block = item_id // 32
			idx = item_id % 32
			offset = land_bytes + block * 1316 + 4 + idx * 41
			f.seek(offset)
			data = f.read(41)
			if len(data) >= 21:
				h = data[20]
				TILE_HEIGHTS[item_id] = h
				return h
	except:
		pass
	return 0

def is_floor_tile(item_id):
	global TILE_FLOORS
	item_id = item_id & 0xFFFF
	if not TILE_FLOORS:
		load_component_definitions()
	if item_id in TILE_FLOORS:
		return True
	return get_tile_height(item_id) <= 0

def load_component_definitions():
	global TILE_FLOORS
	floor_path = '/opt/hd_jogos/Wolfpack-Qt5/server/release/data/components/floors.txt'
	if os.path.exists(floor_path):
		try:
			with open(floor_path, 'r') as f:
				for line in f:
					line = line.strip()
					if not line or line.startswith('int') or line.startswith('Category'):
						continue
					parts = line.split('\t')
					for p in parts[1:-2]:
						try:
							v = int(p)
							if v > 0:
								TILE_FLOORS.add(v)
						except:
							pass
		except:
			pass

def load_multi_components(multi_id):
	idx_path = os.path.join(MUL_PATH, 'multi.idx')
	mul_path = os.path.join(MUL_PATH, 'multi.mul')
	if not os.path.exists(idx_path) or not os.path.exists(mul_path):
		return []

	try:
		multi_id = multi_id & 0x3FFF
		with open(idx_path, 'rb') as fidx, open(mul_path, 'rb') as fmul:
			fidx.seek(multi_id * 12)
			data = fidx.read(12)
			if len(data) < 12:
				return []
			start, length, extra = struct.unpack('<iii', data)
			if start == -1 or length <= 0:
				return []
			fmul.seek(start)
			mdata = fmul.read(length)
			rec_size = 16 if length % 16 == 0 else 12
			count = length // rec_size
			items = []
			for i in range(count):
				entry = mdata[i*rec_size : (i+1)*rec_size]
				item_id, x, y, z = struct.unpack('<Hhhh', entry[:8])
				items.append((item_id, x, y, z))
			return items
	except Exception as e:
		print("Error loading multi components:", e)
		return []

def get_foundation_dimensions(foundation_id):
	comps = load_multi_components(foundation_id)
	if not comps:
		return (-3, -3, 3, 3, 7, 7)
	xs = [it[1] for it in comps]
	ys = [it[2] for it in comps]
	x_min = min(xs)
	x_max = max(xs)
	y_min = min(ys)
	y_max = max(ys)
	# Resizes height + 1 for foundation walkway
	width = (x_max - x_min) + 1
	height = (y_max - y_min) + 2
	y_max += 1
	return (x_min, y_min, x_max, y_max, width, height)

FOUNDATION_GRAPHICS = {
	0: (0x0015, 0x0016, 0x0017, 0x0014), # DarkWood
	1: (0x00BE, 0x00BF, 0x00C0, 0x00BD), # LightWood
	2: (0x02FF, 0x02FE, 0x0300, 0x02FD), # Dungeon
	3: (0x0043, 0x0042, 0x0044, 0x0041), # Brick
	4: (0x0064, 0x0063, 0x0066, 0x0065), # Stone
	5: (0x2DF9, 0x2DFA, 0x2DF8, 0x2DF7), # ElvenGrey
	6: (0x2DFD, 0x2DFE, 0x2DFC, 0x2DFB), # ElvenNatural
	7: (0x2CEF, 0x2CF0, 0x2BC8, 0x2BC7), # SimpleMarble
	8: (0x2DCF, 0x2DD0, 0x2DC6, 0x2DC3), # PlainMarble
	9: (0x2BB9, 0x2BBA, 0x2BB0, 0x2BAD), # OrnateMarble
	10: (0x3671, 0x3670, 0x3673, 0x3672), # Crystal
	11: (0x3675, 0x3674, 0x3677, 0x3676), # Shadow
	12: (0x41A8, 0x41A7, 0x419E, 0x41A6), # GargishGreenMarble
	13: (0x4166, 0x4167, 0x415F, 0x415C), # GargishTwoToneStone
	14: (0x4B00, 0x4AFA, 0x4B06, 0x4B08), # Gothic
	15: (0x9AC0, 0x9ABF, 0x9AC1, 0x9ABE), # Brick1
	16: (0x9BD2, 0x9BD1, 0x9BD3, 0x9BD0), # Brick2
}

def get_empty_foundation_tiles(foundation_id, f_type=0):
	east, south, post, corner = FOUNDATION_GRAPHICS.get(f_type, FOUNDATION_GRAPHICS[0])
	
	comps = load_multi_components(foundation_id)
	xs = [it[1] for it in comps] if comps else [-3, 3]
	ys = [it[2] for it in comps] if comps else [-3, 3]
	x_min = min(xs)
	x_max = max(xs)
	y_min = min(ys)
	y_max = max(ys)
	
	width = (x_max - x_min) + 1
	height = (y_max - y_min) + 2 # +1 for walkway
	
	x_center = -x_min
	y_center = -y_min
	
	tiles = []
	# 1. Post and corner
	tiles.append((post, 0 - x_center, 0 - y_center, 0))
	tiles.append((corner, width - 1 - x_center, height - 2 - y_center, 0))
	
	# 2. South edges
	for x in range(1, width):
		tiles.append((south, x - x_center, 0 - y_center, 0))
		if x < width - 1:
			tiles.append((south, x - x_center, height - 2 - y_center, 0))
			
	# 3. East edges
	for y in range(1, height - 1):
		tiles.append((east, 0 - x_center, y - y_center, 0))
		if y < height - 2:
			tiles.append((east, width - 1 - x_center, y - y_center, 0))
			
	# 4. Front walkway steps (0x751)
	walkway_y = height - 1
	for x in range(1, width):
		tiles.append((0x751, x - x_center, walkway_y - y_center, 0))
		
	# 5. Base dirt/floor tiles (0x31F4) at Z=7
	for x in range(1, width):
		for y in range(1, height - 1):
			tiles.append((0x31F4, x - x_center, y - y_center, 7))
			
	return tiles

def is_custom_house(multi):
	if not multi:
		return False
	if multi.hastag('ch_revision') or multi.hastag('custom'):
		return True
	model = multi.id & 0x3FFF
	if 0x13EC <= model <= 0x147F:
		return True
	return False

def get_or_create_design(multi):
	if not is_custom_house(multi):
		return None
	serial = multi.serial
	if serial in DESIGNS:
		return DESIGNS[serial]
		
	# Check if multi has saved design state in tags
	f_id = multi.id
	f_type = int(multi.gettag('foundation_type') or 0)
	x_min, y_min, x_max, y_max, width, height = get_foundation_dimensions(f_id)
	
	if multi.hastag('ch_current'):
		try:
			raw = multi.gettag('ch_current')
			current_tiles = json.loads(raw)
		except:
			current_tiles = get_empty_foundation_tiles(f_id, f_type)
	else:
		current_tiles = get_empty_foundation_tiles(f_id, f_type)
		
	revision = int(multi.gettag('ch_revision') or 1)
	
	DESIGNS[serial] = {
		'current': list(current_tiles),
		'design': list(current_tiles),
		'backup': list(current_tiles),
		'revision': revision,
		'bounds': (x_min, y_min, x_max, y_max, width, height)
	}
	return DESIGNS[serial]

#
# Packet construction
#

def build_d8_packet(serial, revision, response, bounds, tiles):
	x_min, y_min, x_max, y_max, width, height = bounds
	
	plane_buffers = [bytearray(width * height * 2)]
	for i in range(4):
		plane_buffers.append(bytearray((width - 1) * (height - 2) * 2))
	for i in range(4):
		plane_buffers.append(bytearray(width * (height - 1) * 2))
		
	plane_used = [False] * 9
	stair_buffer = []
	
	for item_id, ox, oy, oz in tiles:
		x = ox - x_min
		y = oy - y_min
		z = oz
		floor = is_floor_tile(item_id)
		
		if z == 0: plane = 0
		elif z == 7: plane = 1
		elif z == 27: plane = 2
		elif z == 47: plane = 3
		elif z == 67: plane = 4
		else:
			stair_buffer.append((item_id, ox, oy, oz))
			continue
			
		if plane == 0:
			size = height
		elif floor:
			size = height - 2
			x -= 1
			y -= 1
		else:
			size = height - 1
			plane += 4
			
		index = ((x * size) + y) * 2
		if x < 0 or y < 0 or y >= size or (index + 1) >= len(plane_buffers[plane]):
			stair_buffer.append((item_id, ox, oy, oz))
		else:
			plane_used[plane] = True
			plane_buffers[plane][index] = (item_id >> 8) & 0xFF
			plane_buffers[plane][index + 1] = item_id & 0xFF
			
	compressed_planes = []
	for i in range(9):
		if not plane_used[i]:
			continue
		inflated = plane_buffers[i]
		size = len(inflated)
		deflated = zlib.compress(bytes(inflated))
		deflated_len = len(deflated)
		header = bytearray(4)
		header[0] = 0x20 | i
		header[1] = size & 0xFF
		header[2] = deflated_len & 0xFF
		header[3] = (((size >> 4) & 0xF0) | ((deflated_len >> 8) & 0xF))
		compressed_planes.append(header + deflated)
		
	max_per_stair = 750
	stair_chunks = [stair_buffer[i:i+max_per_stair] for i in range(0, len(stair_buffer), max_per_stair)]
	for idx, chunk in enumerate(stair_chunks):
		inflated = bytearray()
		for item_id, ox, oy, oz in chunk:
			inflated.extend(struct.pack('>Hbbb', item_id, ox, oy, oz))
		size = len(inflated)
		deflated = zlib.compress(bytes(inflated))
		deflated_len = len(deflated)
		header = bytearray(4)
		header[0] = 9 + idx
		header[1] = size & 0xFF
		header[2] = deflated_len & 0xFF
		header[3] = (((size >> 4) & 0xF0) | ((deflated_len >> 8) & 0xF))
		compressed_planes.append(header + deflated)
		
	plane_count = len(compressed_planes)
	total_length = 1 + sum(len(cp) for cp in compressed_planes)
	
	packet_body = bytearray()
	packet_body.append(0x03)
	packet_body.append(0x01 if response else 0x00)
	packet_body.extend(struct.pack('>IIHHB', serial, revision, len(tiles), total_length, plane_count))
	for cp in compressed_planes:
		packet_body.extend(cp)
		
	packet_len = 3 + len(packet_body)
	packet = [0xD8, (packet_len >> 8) & 0xFF, packet_len & 0xFF] + list(packet_body)
	return packet

def build_revision_packet(serial, revision):
	return [0xBF, 0, 13, 0x00, 0x1D] + list(struct.pack('>II', serial, revision))

def build_start_customize_packet(serial):
	return [0xBF, 0, 17, 0x00, 0x20] + list(struct.pack('>IBHHHB', serial, 0x04, 0x0000, 0xFFFF, 0xFFFF, 0xFF))

def build_end_customize_packet(serial):
	return [0xBF, 0, 17, 0x00, 0x20] + list(struct.pack('>IBHHHB', serial, 0x05, 0x0000, 0xFFFF, 0xFFFF, 0xFF))

#
# Level Z calculations
#

def get_level_z(level):
	if level < 1: level = 1
	return (level - 1) * 20 + 7

#
# Custom house session entry point
#

def start_customizing(player, multi):
	design = get_or_create_design(multi)
	design['design'] = list(design['current'])
	design['revision'] += 1
	
	width, height = design['bounds'][4], design['bounds'][5]
	max_levels = 4 if (width >= 14 or height >= 14) else 3
	
	SESSIONS[player.serial] = {
		'multi_serial': multi.serial,
		'level': 1,
		'max_levels': max_levels
	}
	
	player.settag('customizing', multi.serial)
	
	# Teleport player into house at level 1
	player.moveto(wolfpack.coord(multi.pos.x, multi.pos.y, multi.pos.z + 7, multi.pos.map))
	player.update()
	
	# Send start customization packet
	start_packet = build_start_customize_packet(multi.serial)
	player.socket.sendpacket(start_packet)
	
	# Send revision packet
	rev_packet = build_revision_packet(multi.serial, design['revision'])
	player.socket.sendpacket(rev_packet)
	
	# Send detailed 0xD8 design packet
	d8_packet = build_d8_packet(multi.serial, design['revision'], True, design['bounds'], design['design'])
	player.socket.sendpacket(d8_packet)

def end_customizing(player, multi, commit=False):
	if player.serial in SESSIONS:
		del SESSIONS[player.serial]
	if player.hastag('customizing'):
		player.deltag('customizing')
		
	end_packet = build_end_customize_packet(multi.serial)
	player.socket.sendpacket(end_packet)
	
	design = get_or_create_design(multi)
	if not commit:
		# Revert uncommitted changes
		design['design'] = list(design['current'])
		design['revision'] += 1
		
	d8_packet = build_d8_packet(multi.serial, design['revision'], False, design['bounds'], design['current'])
	player.socket.sendpacket(d8_packet)

#
# Network State Senders
#

def send_detailed_state(player, multi, response=True):
	if not multi or not player or not player.socket:
		return
	if not is_custom_house(multi):
		return
	design = get_or_create_design(multi)
	if not design:
		return
	state_tiles = design['design'] if (player.serial in SESSIONS and SESSIONS[player.serial]['multi_serial'] == multi.serial) else design['current']
	d8_packet = build_d8_packet(multi.serial, design['revision'], response, design['bounds'], state_tiles)
	player.socket.sendpacket(d8_packet)

def send_general_state(player, multi):
	if not multi or not player or not player.socket:
		return
	if not is_custom_house(multi):
		return
	design = get_or_create_design(multi)
	if not design:
		return
	rev_pkt = build_revision_packet(multi.serial, design['revision'])
	player.socket.sendpacket(rev_pkt)

def build_empty_d8_packet(serial, revision=1):
	return [0xD8, 0x00, 0x11, 0x00, 0x01] + list(struct.pack('>IIHH', serial, revision, 0, 0))

#
# Event Handlers
#

def onCHQuery(player, multi):
	if not multi or not player or not player.socket:
		return
	if not is_custom_house(multi):
		player.socket.sendpacket(build_empty_d8_packet(multi.serial))
		return
	send_detailed_state(player, multi, response=True)

def onCHCommand(player, command, params):
	if player.serial not in SESSIONS:
		return
		
	session = SESSIONS[player.serial]
	multi = wolfpack.findobject(session['multi_serial'])
	if not multi:
		return
		
	design = get_or_create_design(multi)
	
	# Commands:
	# 0x02: Backup
	# 0x03: Restore
	# 0x04: Commit
	# 0x05: Delete
	# 0x06: AddElement
	# 0x0C: Close
	# 0x0D: Stairs
	# 0x0E: Sync
	# 0x10: Clear
	# 0x1A: Revert
	
	if command == 0x02: # Backup
		design['backup'] = list(design['design'])
		player.socket.sysmessage("House design backed up.")
		
	elif command == 0x03: # Restore
		design['design'] = list(design['backup'])
		design['revision'] += 1
		rev_pkt = build_revision_packet(multi.serial, design['revision'])
		player.socket.sendpacket(rev_pkt)
		d8_pkt = build_d8_packet(multi.serial, design['revision'], True, design['bounds'], design['design'])
		player.socket.sendpacket(d8_pkt)
		
	elif command == 0x04: # Commit
		show_commit_gump(player, multi, design)
		
	elif command == 0x05: # Delete
		elem_id = params.get('elementId', 0)
		x = params.get('x', 0)
		y = params.get('y', 0)
		z = params.get('z', 0)
		
		# Do not allow deleting ground level foundation boundary
		x_min, y_min, x_max, y_max, width, height = design['bounds']
		ax = x - x_min
		ay = y - y_min
		if z == 0 and ax >= 0 and ax < width and ay >= 0 and ay < (height - 1):
			d8_pkt = build_d8_packet(multi.serial, design['revision'], True, design['bounds'], design['design'])
			player.socket.sendpacket(d8_pkt)
			return
			
		# Remove tile
		new_list = [t for t in design['design'] if not (t[0] == elem_id and t[1] == x and t[2] == y and t[3] == z)]
		# If removed floor at Z=7 and no floor left at (x,y), replace with dirt 0x31F4
		if ax >= 1 and ax < width and ay >= 1 and ay < height - 1:
			has_base = any(t[1] == x and t[2] == y and t[3] == 7 and t[0] != 1 for t in new_list)
			if not has_base:
				new_list.append((0x31F4, x, y, 7))
				
		design['design'] = new_list
		design['revision'] += 1
		
		rev_pkt = build_revision_packet(multi.serial, design['revision'])
		player.socket.sendpacket(rev_pkt)
		d8_pkt = build_d8_packet(multi.serial, design['revision'], True, design['bounds'], design['design'])
		player.socket.sendpacket(d8_pkt)
		
	elif command == 0x06: # AddElement
		elem_id = params.get('elementId', 0)
		x = params.get('x', 0)
		y = params.get('y', 0)
		
		level = session.get('level', 1)
		z = get_level_z(level)
		
		x_min, y_min, x_max, y_max, width, height = design['bounds']
		ay = y - y_min
		if ay == (height - 1):
			z = 0 # South stairs / walkway edge
			
		# Filter existing same-position/layer tiles if necessary
		new_list = [t for t in design['design'] if not (t[1] == x and t[2] == y and t[3] == z and (is_floor_tile(t[0]) == is_floor_tile(elem_id)))]
		new_list.append((elem_id, x, y, z))
		design['design'] = new_list
		design['revision'] += 1
		
		rev_pkt = build_revision_packet(multi.serial, design['revision'])
		player.socket.sendpacket(rev_pkt)
		d8_pkt = build_d8_packet(multi.serial, design['revision'], True, design['bounds'], design['design'])
		player.socket.sendpacket(d8_pkt)
		
	elif command == 0x0C: # Close
		end_customizing(player, multi, commit=False)
		
	elif command == 0x0D: # Stairs
		multi_id = params.get('multiId', 0)
		x = params.get('x', 0)
		y = params.get('y', 0)
		level = session.get('level', 1)
		z = get_level_z(level)
		
		stair_tiles = load_multi_components(multi_id)
		for s_id, sx, sy, sz in stair_tiles:
			if s_id != 1:
				design['design'].append((s_id, x + sx, y + sy, z + sz))
				
		design['revision'] += 1
		rev_pkt = build_revision_packet(multi.serial, design['revision'])
		player.socket.sendpacket(rev_pkt)
		d8_pkt = build_d8_packet(multi.serial, design['revision'], True, design['bounds'], design['design'])
		player.socket.sendpacket(d8_pkt)
		
	elif command == 0x0E: # Sync
		rev_pkt = build_revision_packet(multi.serial, design['revision'])
		player.socket.sendpacket(rev_pkt)
		d8_pkt = build_d8_packet(multi.serial, design['revision'], True, design['bounds'], design['design'])
		player.socket.sendpacket(d8_pkt)
		
	elif command == 0x10: # Clear
		f_id = multi.id
		f_type = int(multi.gettag('foundation_type') or 0)
		design['design'] = get_empty_foundation_tiles(f_id, f_type)
		design['revision'] += 1
		rev_pkt = build_revision_packet(multi.serial, design['revision'])
		player.socket.sendpacket(rev_pkt)
		d8_pkt = build_d8_packet(multi.serial, design['revision'], True, design['bounds'], design['design'])
		player.socket.sendpacket(d8_pkt)
		
	elif command == 0x1A: # Revert
		design['design'] = list(design['current'])
		design['revision'] += 1
		rev_pkt = build_revision_packet(multi.serial, design['revision'])
		player.socket.sendpacket(rev_pkt)
		d8_pkt = build_d8_packet(multi.serial, design['revision'], True, design['bounds'], design['design'])
		player.socket.sendpacket(d8_pkt)

#
# Level change
#

def onLevelChange(player, level):
	if player.serial not in SESSIONS:
		return
	session = SESSIONS[player.serial]
	if level < 1: level = 1
	if level > session['max_levels']: level = session['max_levels']
	session['level'] = level
	
	multi = wolfpack.findobject(session['multi_serial'])
	if multi:
		z = multi.pos.z + get_level_z(level)
		player.moveto(wolfpack.coord(player.pos.x, player.pos.y, z, player.pos.map))
		player.update()

#
# Commit Gump & Logic
#

def show_commit_gump(player, multi, design):
	old_price = int(multi.gettag('value') or 0)
	old_count = len(design['current'])
	new_count = len(design['design'])
	delta_tiles = new_count - old_count
	tile_cost = delta_tiles * 500
	new_price = old_price + tile_cost
	cost = new_price - old_price
	
	bank_balance = player.getbankbox().countresource(0xeed)
	
	gump = cGump()
	gump.addRawLayout("{page 0}")
	gump.addRawLayout("{resizepic 0 0 5054 320 320}")
	gump.addRawLayout("{gumppictiled 10 10 300 20 2624}")
	gump.addRawLayout("{gumppictiled 10 40 300 240 2624}")
	gump.addRawLayout("{gumppictiled 10 290 300 20 2624}")
	gump.addRawLayout("{checkertrans 10 10 300 300}")
	
	# <CENTER>COMMIT DESIGN</CENTER>
	gump.addRawLayout("{xmfhtmlgumpcolor 10 10 300 20 1062060 0 0 32736}")
	
	msg_cliloc = 1061898 if cost <= bank_balance else 1061903
	gump.addRawLayout("{xmfhtmlgumpcolor 10 40 300 140 %d 0 1 1023}" % msg_cliloc)
	
	# Bank Balance:
	gump.addRawLayout("{xmfhtmlgumpcolor 10 190 150 20 1061902 0 0 32736}")
	gump.addRawLayout("{text 170 190 55 0}")
	
	# Old Value:
	gump.addRawLayout("{xmfhtmlgumpcolor 10 215 150 20 1061899 0 0 1023}")
	gump.addRawLayout("{text 170 215 90 1}")
	
	# Cost To Commit:
	gump.addRawLayout("{xmfhtmlgumpcolor 10 235 150 20 1061900 0 0 1023}")
	gump.addRawLayout("{text 170 235 90 2}")
	
	if cost < 0:
		# Your Refund:
		gump.addRawLayout("{xmfhtmlgumpcolor 10 260 150 20 1062059 0 0 992}")
		gump.addRawLayout("{text 170 260 70 3}")
	else:
		# Your Cost:
		gump.addRawLayout("{xmfhtmlgumpcolor 10 260 150 20 1061901 0 0 31744}")
		gump.addRawLayout("{text 170 260 40 3}")
		
	# Buttons: OK (1), CANCEL (0)
	gump.addRawLayout("{button 10 290 4005 4007 1 0 1}")
	gump.addRawLayout("{xmfhtmlgumpcolor 45 290 55 20 1011036 0 0 32767}")
	
	gump.addRawLayout("{button 170 290 4005 4007 1 0 0}")
	gump.addRawLayout("{xmfhtmlgumpcolor 195 290 55 20 1011012 0 0 32767}")
	
	gump.addRawText(str(bank_balance))
	gump.addRawText(str(old_price))
	gump.addRawText(str(new_price))
	gump.addRawText(str(abs(cost)))
	
	gump.setCallback(commit_gump_callback)
	gump.setArgs([multi.serial, cost, new_price])
	gump.send(player)

def commit_gump_callback(player, args, target):
	if target.button != 1:
		return
		
	multi_serial, cost, new_price = args
	multi = wolfpack.findobject(multi_serial)
	if not multi:
		return
		
	bank = player.getbankbox()
	bank_gold = bank.countresource(0xeed)
	
	if not player.gm:
		if cost > 0:
			if bank_gold < cost:
				player.socket.clilocmessage(1061903)
				return
			bank.useresource(cost, 0xeed)
			player.socket.clilocmessage(1060398, str(cost))
		elif cost < 0:
			refund = -cost
			gold = wolfpack.additem("eed")
			gold.amount = refund
			if not wolfpack.utilities.tocontainer(gold, bank):
				gold.moveto(player.pos)
				gold.update()
			player.socket.clilocmessage(1060397, str(refund))
	else:
		if cost != 0:
			player.socket.sysmessage("%d gold would have been %s your bank if you were not a GM." % (cost, "withdrawn from" if cost > 0 else "deposited into"))
			
	# Apply design state to current
	design = get_or_create_design(multi)
	design['current'] = list(design['design'])
	design['revision'] += 1
	
	multi.settag('value', new_price)
	multi.settag('ch_revision', design['revision'])
	multi.settag('ch_current', json.dumps(design['current']))
	
	player.socket.sysmessage("House design committed successfully!")
	end_customizing(player, multi, commit=True)

#
# Foundation style change
#
def apply_foundation_change(multi, new_type):
	multi.settag('foundation_type', new_type)
	design = get_or_create_design(multi)
	f_id = multi.id
	
	new_base_tiles = get_empty_foundation_tiles(f_id, new_type)
	new_border_map = {(t[1], t[2], t[3]): t[0] for t in new_base_tiles if t[3] == 0 and t[0] != 0x751}
	
	for state_name in ('current', 'design', 'backup'):
		old_list = design.get(state_name, [])
		updated_list = []
		for it in old_list:
			pos_key = (it[1], it[2], it[3])
			if pos_key in new_border_map:
				updated_list.append((new_border_map[pos_key], it[1], it[2], it[3]))
			else:
				updated_list.append(it)
		existing_border_keys = {(it[1], it[2], it[3]) for it in updated_list}
		for pos_key, item_id in new_border_map.items():
			if pos_key not in existing_border_keys:
				updated_list.append((item_id, pos_key[0], pos_key[1], pos_key[2]))
		design[state_name] = updated_list
		
	design['revision'] += 1
	multi.settag('ch_revision', design['revision'])
	multi.settag('ch_current', json.dumps(design['current']))
	
	rev_pkt = build_revision_packet(multi.serial, design['revision'])
	d8_pkt = build_d8_packet(multi.serial, design['revision'], False, design['bounds'], design['current'])
	for char in wolfpack.chars(multi.pos, 30):
		if char.player and char.socket:
			char.socket.sendpacket(rev_pkt)
			char.socket.sendpacket(d8_pkt)

# Register global events
def onLoad():
	wolfpack.registerglobal(EVENT_CHCOMMAND, "housing.customhouse")
	wolfpack.registerglobal(EVENT_CHQUERY, "housing.customhouse")
