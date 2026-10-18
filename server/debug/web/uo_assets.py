#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Ultima Online Asset & Gump Extractor       #
#   )).-' {{ ;'`   | Direct UOP / MUL reader for Web Portal     #
#  ( (  ;._ \\ ctr | Extracts Paperdoll, Status & Skills Gumps  #
#===============================================================#

import os
import sys

for _p in [
	'/usr/lib/python3/dist-packages',
	'/usr/local/lib/python3.14/dist-packages',
	'/usr/lib/python3.14/dist-packages',
	'/usr/local/lib/python3/dist-packages',
	'/usr/lib/python3.13/dist-packages',
	'/usr/lib/python3.12/dist-packages',
]:
	if os.path.isdir(_p) and _p not in sys.path:
		sys.path.append(_p)

import struct
import zlib
from PIL import Image, ImageDraw, ImageFont

_MUL_PATH = None
_UOP_INDEX = None
_TILEDATA_CACHE = {}
_HUE_CACHE = {}

# Paperdoll Drawing Layer Order (bottom to top)
PAPERDOLL_LAYER_ORDER = [
	3,   # Shoes / Boots
	4,   # Pants
	5,   # Shirt
	6,   # Helm / Hat
	7,   # Gloves
	9,   # Ring
	10,  # Neck / Gorget
	11,  # Hair
	13,  # Chest / Armor / Tunic
	14,  # Waist / Belt
	16,  # Facial Hair / Beard
	17,  # Torso Inner / Surcoat
	18,  # Bracelet / Wrist
	19,  # Arms
	20,  # Cloak / Cape
	22,  # Robe / Dress
	23,  # Skirt / Kilt
	24,  # Torso Outer
	1,   # One-Handed Weapon
	2    # Two-Handed Weapon / Shield
]

# Body Gump mappings (Race & Gender)
BODY_GUMP_MAP = {
	# Human Male
	0x190: 12,
	400:   12,
	# Human Female
	0x191: 13,
	401:   13,
	# Ghost Male / Female
	0x192: 12,
	402:   12,
	0x193: 13,
	403:   13,
	# Elf Male
	0x25D: 14,
	605:   14,
	# Elf Female
	0x25E: 15,
	606:   15,
	# Elf Ghosts
	0x25F: 14,
	607:   14,
	0x260: 15,
	608:   15,
	# Gargoyle Male
	0x29A: 666,
	666:   666,
	# Gargoyle Female
	0x29B: 665,
	667:   665,
	# Gargoyle Ghosts
	0x2B4: 666,
	692:   666,
	0x2B5: 665,
	693:   665,
}

def get_mul_path():
	global _MUL_PATH
	if _MUL_PATH and os.path.isdir(_MUL_PATH):
		return _MUL_PATH

	candidates = [
		'/opt/hd_jogos/UOC',
		'/opt/hd_jogos/UOC/',
		os.path.join(os.path.dirname(__file__), '..', '..', '..', 'UOC'),
	]

	try:
		import wolfpack
		import wolfpack.serverconfig
		cfg_path = wolfpack.serverconfig.Config.instance().mulPath()
		if cfg_path:
			candidates.insert(0, cfg_path)
	except Exception:
		pass

	for c in candidates:
		if os.path.isdir(c) and (os.path.exists(os.path.join(c, 'gumpartLegacyMUL.uop')) or os.path.exists(os.path.join(c, 'gumpart.mul'))):
			_MUL_PATH = os.path.abspath(c)
			return _MUL_PATH

	_MUL_PATH = '/opt/hd_jogos/UOC'
	return _MUL_PATH

def hash_little2(s):
	"""Bob Jenkins lookup3 hashlittle2 used by Mythic UOP archives."""
	length = len(s)
	a = b = c = (0xDEADBEEF + length) & 0xFFFFFFFF
	k = 0
	raw = s.encode('latin1')

	while length > 12:
		a = (a + raw[k] + (raw[k+1] << 8) + (raw[k+2] << 16) + (raw[k+3] << 24)) & 0xFFFFFFFF
		b = (b + raw[k+4] + (raw[k+5] << 8) + (raw[k+6] << 16) + (raw[k+7] << 24)) & 0xFFFFFFFF
		c = (c + raw[k+8] + (raw[k+9] << 8) + (raw[k+10] << 16) + (raw[k+11] << 24)) & 0xFFFFFFFF

		a = (a - c) & 0xFFFFFFFF; a = (a ^ (((c << 4) & 0xFFFFFFFF) | (c >> 28))) & 0xFFFFFFFF; c = (c + b) & 0xFFFFFFFF
		b = (b - a) & 0xFFFFFFFF; b = (b ^ (((a << 6) & 0xFFFFFFFF) | (a >> 26))) & 0xFFFFFFFF; a = (a + c) & 0xFFFFFFFF
		c = (c - b) & 0xFFFFFFFF; c = (c ^ (((b << 8) & 0xFFFFFFFF) | (b >> 24))) & 0xFFFFFFFF; b = (b + a) & 0xFFFFFFFF
		a = (a - c) & 0xFFFFFFFF; a = (a ^ (((c << 16) & 0xFFFFFFFF) | (c >> 16))) & 0xFFFFFFFF; c = (c + b) & 0xFFFFFFFF
		b = (b - a) & 0xFFFFFFFF; b = (b ^ (((a << 19) & 0xFFFFFFFF) | (a >> 13))) & 0xFFFFFFFF; a = (a + c) & 0xFFFFFFFF
		c = (c - b) & 0xFFFFFFFF; c = (c ^ (((b << 4) & 0xFFFFFFFF) | (b >> 28))) & 0xFFFFFFFF; b = (b + a) & 0xFFFFFFFF

		length -= 12
		k += 12

	if length > 0:
		if length >= 12: c = (c + (raw[k+11] << 24)) & 0xFFFFFFFF
		if length >= 11: c = (c + (raw[k+10] << 16)) & 0xFFFFFFFF
		if length >= 10: c = (c + (raw[k+9] << 8)) & 0xFFFFFFFF
		if length >= 9:  c = (c + raw[k+8]) & 0xFFFFFFFF
		if length >= 8:  b = (b + (raw[k+7] << 24)) & 0xFFFFFFFF
		if length >= 7:  b = (b + (raw[k+6] << 16)) & 0xFFFFFFFF
		if length >= 6:  b = (b + (raw[k+5] << 8)) & 0xFFFFFFFF
		if length >= 5:  b = (b + raw[k+4]) & 0xFFFFFFFF
		if length >= 4:  a = (a + (raw[k+3] << 24)) & 0xFFFFFFFF
		if length >= 3:  a = (a + (raw[k+2] << 16)) & 0xFFFFFFFF
		if length >= 2:  a = (a + (raw[k+1] << 8)) & 0xFFFFFFFF
		if length >= 1:  a = (a + raw[k]) & 0xFFFFFFFF

		c = (c ^ b) & 0xFFFFFFFF; c = (c - (((b << 14) & 0xFFFFFFFF) | (b >> 18))) & 0xFFFFFFFF
		a = (a ^ c) & 0xFFFFFFFF; a = (a - (((c << 11) & 0xFFFFFFFF) | (c >> 21))) & 0xFFFFFFFF
		b = (b ^ a) & 0xFFFFFFFF; b = (b - (((a << 25) & 0xFFFFFFFF) | (a >> 7))) & 0xFFFFFFFF
		c = (c ^ b) & 0xFFFFFFFF; c = (c - (((b << 16) & 0xFFFFFFFF) | (b >> 16))) & 0xFFFFFFFF
		a = (a ^ c) & 0xFFFFFFFF; a = (a - (((c << 4) & 0xFFFFFFFF) | (c >> 28))) & 0xFFFFFFFF
		b = (b ^ a) & 0xFFFFFFFF; b = (b - (((a << 14) & 0xFFFFFFFF) | (a >> 18))) & 0xFFFFFFFF
		c = (c ^ b) & 0xFFFFFFFF; c = (c - (((b << 24) & 0xFFFFFFFF) | (b >> 8))) & 0xFFFFFFFF

	return (b << 32) | c

def bwt_decompress(buf):
	"""Burrows-Wheeler Transform decompression for UOP compression type 3."""
	first_char = buf[4]
	table = list(range(65536))
	list_bytes = bytearray(len(buf) - 4)
	i = 0
	pos = 5
	cur_fc = first_char
	buf_len = len(buf)

	while pos < buf_len:
		cv = cur_fc
		val = table[cv]
		if cv > 0:
			table[1:cv+1] = table[0:cv]
		table[0] = val
		list_bytes[i] = val & 0xFF
		i += 1
		cur_fc = buf[pos]
		pos += 1

	val = table[cur_fc]
	list_bytes[i] = val & 0xFF

	partial_input = [0] * (256 * 3)
	p_ints = struct.unpack('<256i', bytes(list_bytes[:1024]))
	for k in range(256):
		partial_input[k] = p_ints[k]

	length = sum(partial_input[:256])
	if length <= 0:
		return bytearray()

	non_zero = sum(1 for k in range(256) if partial_input[k] != 0)

	tmp = list(partial_input[:256])
	frequency = [0] * 256
	for k in range(256):
		max_val = 0
		max_idx = 0
		for j in range(256):
			if tmp[j] > max_val:
				max_idx = j
				max_val = tmp[j]
		if max_val == 0:
			break
		frequency[k] = max_idx
		tmp[max_idx] = 0

	symbol_table = list(range(256))
	m = 0
	for k in range(non_zero):
		freq = frequency[k]
		symbol_table[list_bytes[m + 1024]] = freq
		partial_input[freq + 256] = m + 1
		m += partial_input[freq]
		partial_input[freq + 512] = m

	val = symbol_table[0]
	count = 0
	output = bytearray(length)

	while count < length:
		first_ref = partial_input[val + 256]
		output[count] = val
		if first_ref >= partial_input[val + 512]:
			if non_zero > 0:
				non_zero -= 1
				symbol_table[:non_zero] = symbol_table[1:non_zero+1]
				val = symbol_table[0]
		else:
			idx = list_bytes[first_ref + 1024]
			partial_input[val + 256] += 1
			if idx != 0:
				symbol_table[:idx] = symbol_table[1:idx+1]
				symbol_table[idx] = val
				val = symbol_table[0]
		count += 1

	return output

def _load_uop_index():
	global _UOP_INDEX
	if _UOP_INDEX is not None:
		return _UOP_INDEX

	_UOP_INDEX = {}
	base_path = get_mul_path()
	uop_file = os.path.join(base_path, 'gumpartLegacyMUL.uop')

	if not os.path.exists(uop_file):
		return _UOP_INDEX

	try:
		with open(uop_file, 'rb') as f:
			f.seek(40)
			file_count, next_block = struct.unpack('<IQ', f.read(12))
			while True:
				for _ in range(file_count):
					offset, hlen, c_len, d_len, hash_val, adler, c_type = struct.unpack('<QIIIQIH', f.read(34))
					if offset > 0 and c_len > 0:
						_UOP_INDEX[hash_val] = (offset + hlen, c_len, d_len, c_type)
				if next_block == 0 or next_block >= os.path.getsize(uop_file):
					break
				f.seek(next_block)
				file_count, next_block = struct.unpack('<IQ', f.read(12))
	except Exception as e:
		pass

	return _UOP_INDEX

def load_hue_table(hue_id):
	"""Returns 32-color 16-bit array from hues.mul for the requested hue ID."""
	global _HUE_CACHE
	if hue_id <= 0:
		return None
	# Normalize hue masks (e.g. 0x8000 / 0x4000 flags in UO)
	hid = hue_id & 0x3FFF
	if hid in _HUE_CACHE:
		return _HUE_CACHE[hid]

	hues_path = os.path.join(get_mul_path(), 'hues.mul')
	if not os.path.exists(hues_path):
		return None

	try:
		block = (hid - 1) // 8
		idx = (hid - 1) % 8
		offset = block * 708 + 4 + idx * 88
		with open(hues_path, 'rb') as f:
			f.seek(offset)
			raw = f.read(64)
			table = struct.unpack('<32H', raw)
			_HUE_CACHE[hid] = table
			return table
	except Exception:
		return None

def get_item_anim_id(item_id):
	"""Reads tiledata.mul static tile table to retrieve layer, count/animID."""
	global _TILEDATA_CACHE
	if item_id in _TILEDATA_CACHE:
		return _TILEDATA_CACHE[item_id]

	tiledata_path = os.path.join(get_mul_path(), 'tiledata.mul')
	if not os.path.exists(tiledata_path):
		return (0, 0, '')

	try:
		block = item_id // 32
		entry = item_id % 32
		# 7.0.9.0+ tiledata format: land block size 964, static block size 1316 (4 + 32*41)
		offset = 493568 + block * 1316 + 4 + entry * 41
		with open(tiledata_path, 'rb') as f:
			f.seek(offset)
			record = f.read(41)
			flags, weight, layer, count, anim, hue, light, height, name = struct.unpack('<QBBiHHBB20s', record[:40])
			name_str = name.split(b'\x00')[0].decode('latin1', errors='ignore').strip()
			# In 7.0.9.0+, anim is stored in count for equipment animations, or anim for hair
			anim_val = anim if anim > 0 else count
			res = (layer, anim_val, name_str)
			_TILEDATA_CACHE[item_id] = res
			return res
	except Exception:
		return (0, 0, '')

def decode_gump_raw(raw, hue_id=0):
	"""Decodes raw uncompressed UO gump data to a PIL RGBA Image."""
	w, h = struct.unpack('<II', raw[:8])
	if w == 0 or h == 0 or w > 2000 or h > 2000:
		return None

	row_lookup = struct.unpack(f'<{h}i', raw[8:8+h*4])
	data_start = 8
	half_len = (len(raw) - data_start) // 4
	img = Image.new('RGBA', (w, h), (0, 0, 0, 0))
	pixels = img.load()
	hue_table = load_hue_table(hue_id) if hue_id > 0 else None

	for y in range(h):
		pos = data_start + (row_lookup[y] * 4)
		if y < h - 1:
			gsize = row_lookup[y + 1] - row_lookup[y]
		else:
			gsize = half_len - row_lookup[y]

		x = 0
		for _ in range(gsize):
			if pos + 4 > len(raw):
				break
			color16, run = struct.unpack('<HH', raw[pos:pos+4])
			pos += 4
			if color16 != 0:
				if hue_table is not None:
					c_idx = (color16 >> 10) & 0x1F
					color16 = hue_table[c_idx]

				r = ((color16 >> 10) & 0x1F) * 255 // 31
				g = ((color16 >> 5) & 0x1F) * 255 // 31
				b = (color16 & 0x1F) * 255 // 31
				rgba = (r, g, b, 255)
				for i in range(run):
					if x + i < w:
						pixels[x + i, y] = rgba
			x += run
			if x >= w:
				break

	return img

def get_gump(gump_id, hue_id=0):
	"""Retrieves or renders a gump image from UOP / MUL, caching it as PNG."""
	web_dir = os.path.dirname(os.path.abspath(__file__))
	gump_filename = f"gump_{gump_id}_{hue_id}.png"
	gump_rel_path = f"images/gumps/{gump_filename}"
	gump_abs_path = os.path.join(web_dir, "images", "gumps", gump_filename)

	if os.path.exists(gump_abs_path):
		return Image.open(gump_abs_path), gump_rel_path

	uop_index = _load_uop_index()
	uop_file = os.path.join(get_mul_path(), 'gumpartLegacyMUL.uop')

	raw_data = None
	# Try primary 8-digit format, then 7-digit
	for fmt in [f"build/gumpartlegacymul/{gump_id:08d}.tga", f"build/gumpartlegacymul/{gump_id:07d}.tga"]:
		h = hash_little2(fmt)
		if h in uop_index:
			pos, clen, dlen, ctype = uop_index[h]
			try:
				with open(uop_file, 'rb') as f:
					f.seek(pos)
					cdata = f.read(clen)
					ddata = zlib.decompress(cdata)
					raw_data = bwt_decompress(ddata) if ctype == 3 else ddata
					break
			except Exception:
				pass

	if not raw_data:
		return None, None

	try:
		img = decode_gump_raw(raw_data, hue_id)
		if img:
			os.makedirs(os.path.dirname(gump_abs_path), exist_ok=True)
			img.save(gump_abs_path)
			# Mirror to debug directory if present
			debug_path = gump_abs_path.replace('/release/web/', '/debug/web/')
			if '/release/web/' in gump_abs_path and os.path.exists('/opt/hd_jogos/Wolfpack-Qt5/server/debug/web'):
				os.makedirs(os.path.dirname(debug_path), exist_ok=True)
				img.save(debug_path)
			return img, gump_rel_path
	except Exception:
		pass

	return None, None

def decode_static_art(data, hue_table=None):
	"""Decodes RLE-encoded static item tile art from art.mul into a PIL RGBA Image."""
	if not data or len(data) < 8:
		return None
	dummy, width, height = struct.unpack_from('<IHH', data, 0)
	if width <= 0 or height <= 0 or width > 1024 or height > 1024:
		return None
	word_count = len(data) // 2
	words = struct.unpack(f'<{word_count}H', data[:word_count * 2])
	if len(words) < 4 + height:
		return None
	lookups = [height + 4 + words[4 + i] for i in range(height)]
	img = Image.new('RGBA', (width, height), (0, 0, 0, 0))
	pixels = img.load()
	for y in range(height):
		count = lookups[y]
		x = 0
		while count + 1 < word_count:
			x_offset = words[count]
			x_run = words[count + 1]
			count += 2
			if x_offset + x_run == 0:
				break
			x += x_offset
			if x + x_run > width:
				break
			for r in range(x_run):
				if count >= word_count:
					break
				c = words[count] ^ 0x8000
				count += 1
				if c != 0:
					if hue_table is not None:
						c_idx = (c >> 10) & 0x1F
						c = hue_table[c_idx]
					red = ((c >> 10) & 0x1F) * 255 // 31
					green = ((c >> 5) & 0x1F) * 255 // 31
					blue = (c & 0x1F) * 255 // 31
					pixels[x + r, y] = (red, green, blue, 255)
			x += x_run
	return img

def get_item_art(item_id, hue=0):
	"""Extracts static item tile art from art.mul/artidx.mul and caches as PNG.
	Returns (Image, rel_path).
	"""
	web_dir = os.path.dirname(os.path.abspath(__file__))
	item_filename = f"item_{item_id}_{hue}.png"
	item_rel_path = f"images/items/{item_filename}"
	item_abs_path = os.path.join(web_dir, "images", "items", item_filename)

	if os.path.exists(item_abs_path):
		try:
			return Image.open(item_abs_path), item_rel_path
		except Exception:
			pass

	base_path = get_mul_path()
	artidx_path = os.path.join(base_path, 'artidx.mul')
	art_path = os.path.join(base_path, 'art.mul')

	if not (os.path.exists(artidx_path) and os.path.exists(art_path)):
		return None, None

	try:
		raw_idx = 0x4000 + item_id
		with open(artidx_path, 'rb') as f_idx:
			f_idx.seek(raw_idx * 12)
			record = f_idx.read(12)
			if len(record) < 12:
				return None, None
			lookup, length, extra = struct.unpack('<III', record)

		if lookup == 0xFFFFFFFF or lookup <= 0 or length <= 8:
			return None, None

		with open(art_path, 'rb') as f_art:
			f_art.seek(lookup)
			data = f_art.read(length)

		hue_table = load_hue_table(hue) if hue > 0 else None
		img = decode_static_art(data, hue_table)
		if img:
			os.makedirs(os.path.dirname(item_abs_path), exist_ok=True)
			img.save(item_abs_path)
			# Mirror to debug directory
			debug_path = item_abs_path.replace('/release/web/', '/debug/web/')
			if '/release/web/' in item_abs_path and os.path.exists('/opt/hd_jogos/Wolfpack-Qt5/server/debug/web'):
				os.makedirs(os.path.dirname(debug_path), exist_ok=True)
				img.save(debug_path)
			return img, item_rel_path
	except Exception:
		pass

	return None, None

def get_character_paperdoll(char):
	"""Composites the complete in-game paperdoll for a character:
	Base Frame (Gump 2001) + Body (Sex & Race) with Skin Hue + All Equipped Items in Layer Order.
	Returns the relative web URL to the cached PNG image.
	"""
	import hashlib

	web_dir = os.path.dirname(os.path.abspath(__file__))
	try:
		c_serial = getattr(char, 'serial', 0) if not isinstance(char, int) else char
		c_body = getattr(char, 'id', 0x190) if not isinstance(char, int) else 0x190
		c_skin = getattr(char, 'skin', 0) or getattr(char, 'color', 0) or getattr(char, 'skincolor', 0) if not isinstance(char, int) else 0
		c_gender = getattr(char, 'gender', 0) if not isinstance(char, int) else 0

		# Determine body gump ID
		body_gump_id = BODY_GUMP_MAP.get(c_body)
		if not body_gump_id:
			body_gump_id = 13 if (c_gender == 1 or c_body in (0x191, 401, 0x25E, 606, 0x29B, 667)) else 12
		is_female = body_gump_id in (13, 15, 665, 1888, 1893, 1898)

		# Collect all equipped items
		equipped_items = []
		if not isinstance(char, int):
			for layer in PAPERDOLL_LAYER_ORDER:
				item = None
				try:
					if hasattr(char, 'getlayer'):
						item = char.getlayer(layer)
					elif hasattr(char, 'itemonlayer'):
						item = char.itemonlayer(layer)
				except Exception:
					item = None

				if item:
					i_id = getattr(item, 'id', 0)
					i_color = getattr(item, 'color', 0)
					_, i_anim, _ = get_item_anim_id(i_id)
					if i_anim > 0:
						equipped_items.append((layer, i_id, i_anim, i_color))

		# Compute deterministic signature hash
		eq_sig = f"{body_gump_id}_{c_skin}_" + "_".join([f"{l}:{a}:{c}" for l, _, a, c in equipped_items])
		sig_hash = hashlib.md5(eq_sig.encode('utf-8')).hexdigest()[:16]

		doll_filename = f"paperdoll_0x{c_serial:x}_{sig_hash}.png"
		doll_rel_path = f"images/paperdolls/{doll_filename}"
		doll_abs_path = os.path.join(web_dir, "images", "paperdolls", doll_filename)

		if os.path.exists(doll_abs_path):
			return doll_rel_path

		# Composite base frame (Gump 2001 - simple frame without buttons)
		base_frame, _ = get_gump(2001, 0)
		if not base_frame:
			base_frame, _ = get_gump(2000, 0)
		if not base_frame:
			return "images/gumps/gump_2001_0.png"

		canvas = base_frame.copy()

		# Composite body with skin hue
		body_img, _ = get_gump(body_gump_id, c_skin)
		if not body_img and c_skin != 0:
			body_img, _ = get_gump(body_gump_id, 0)
		if not body_img:
			body_img, _ = get_gump(12, 0)

		if body_img:
			body_pos = (-22, 7) if body_img.size == (322, 294) else (0, 19)
			canvas.paste(body_img, body_pos, body_img)

		# Composite equipped items in layer order
		for layer, i_id, anim_id, item_hue in equipped_items:
			item_gump_id = (60000 + anim_id) if is_female else (50000 + anim_id)
			item_img, _ = get_gump(item_gump_id, item_hue)
			if not item_img and is_female:
				# Fallback to male item gump if female variation is omitted
				item_img, _ = get_gump(50000 + anim_id, item_hue)

			if item_img:
				canvas.paste(item_img, (0, 19), item_img)

		os.makedirs(os.path.dirname(doll_abs_path), exist_ok=True)
		canvas.save(doll_abs_path)

		# Mirror to debug directory
		debug_path = doll_abs_path.replace('/release/web/', '/debug/web/')
		if '/release/web/' in doll_abs_path and os.path.exists('/opt/hd_jogos/Wolfpack-Qt5/server/debug/web'):
			os.makedirs(os.path.dirname(debug_path), exist_ok=True)
			canvas.save(debug_path)

		return doll_rel_path
	except Exception as err:
		try:
			import traceback
			os.makedirs('logs', exist_ok=True)
			with open('logs/paperdoll_debug.log', 'a') as f:
				f.write(f"Error in get_character_paperdoll: {err}\n")
				traceback.print_exc(file=f)
		except Exception:
			pass
		return "images/gumps/gump_2001_0.png"

def ensure_core_gumps():
	"""Ensures all essential core UI gumps are extracted and cached."""
	core_gumps = [
		12,   # Classic Human Male Body (260x237)
		13,   # Classic Human Female Body (260x237)
		14,   # Classic Elf Male Body (260x237)
		15,   # Classic Elf Female Body (260x237)
		665,  # Classic Gargoyle Female Body (260x237)
		666,  # Classic Gargoyle Male Body (260x237)
		1888, # Human Female Body (322x294)
		1889, # Human Male Body (322x294)
		1893, # Elf Female Body (322x294)
		1894, # Elf Male Body (322x294)
		1898, # Gargoyle Female Body (322x294)
		1899, # Gargoyle Male Body (322x294)
		2000, # Paperdoll frame with buttons (262x324)
		2001, # Simple paperdoll frame without buttons (262x324)
		2050, # Classic Status Bar (282x151)
		10860, # Modern Full Status Bar (577x216)
		2080, # Skills Scroll Top (300x37)
		2081, # Skills Scroll Parchment Body (263x70)
		2082, # Skills Scroll Parchment Body Alternate (263x70)
		2083, # Skills Scroll Bottom (273x34)
		2084, # Skill Scroll Button Icon (20x15)
		2085, # Skill Scroll Button Icon Pressed (22x16)
		2092, # Skill Lock Icon (12x15)
		2435, # Skill Up Arrow (9x11)
		2436, # Skill Down Arrow (9x11)
		2437, # Skill Lock Padlock (9x11)
	]
	results = {}
	for gid in core_gumps:
		_, rel = get_gump(gid, 0)
		results[gid] = rel
	return results
