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
import json
import re
from PIL import Image, ImageDraw, ImageFont

_MUL_PATH = None
_UOP_INDEX = None
_MULTICOLLECTION_UOP_INDEX = None
_TILEDATA_CACHE = {}
_STATIC_FLAGS_CACHE = {}
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
			raw_str = name.split(b'\x00')[0].decode('latin1', errors='ignore')
			name_str = ''.join(c for c in raw_str if c.isprintable()).strip()
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

def get_item_art(item_id, hue=0, crop=False):
	"""Extracts static item tile art from art.mul/artidx.mul and caches as PNG.
	Returns (Image, rel_path).
	"""
	web_dir = os.path.dirname(os.path.abspath(__file__))
	suffix = "_crop" if crop else ""
	item_filename = f"item_{item_id}_{hue}{suffix}.png"
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
			if crop:
				bbox = img.getbbox()
				if bbox:
					img = img.crop(bbox)
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

# House multi ID mappings: (item_tile_art_id, fallback_gump_id, display_name, category)
HOUSE_ART_MAP = {
	# Classic Houses
	0x64: (0x22C4, 2275, "Small Stone and Plaster House", "Casa Clássica"),
	0x65: (0x22C4, 2275, "Small Stone and Plaster House", "Casa Clássica"),
	0x66: (0x22C9, 2276, "Small Fieldstone House", "Casa Clássica"),
	0x67: (0x22C9, 2276, "Small Fieldstone House", "Casa Clássica"),
	0x68: (0x22CA, 2277, "Small Brick House", "Casa Clássica"),
	0x69: (0x22CA, 2277, "Small Brick House", "Casa Clássica"),
	0x6A: (0x22CB, 2278, "Small Wood House", "Casa Clássica"),
	0x6B: (0x22CB, 2278, "Small Wood House", "Casa Clássica"),
	0x6C: (0x22CC, 2279, "Small Wood and Plaster House", "Casa Clássica"),
	0x6D: (0x22CC, 2279, "Small Wood and Plaster House", "Casa Clássica"),
	0x6E: (0x22CD, 2280, "Small Thatched Roof Cottage", "Casa Clássica"),
	0x6F: (0x22CD, 2280, "Small Thatched Roof Cottage", "Casa Clássica"),
	0x70: (0x22CE, 2281, "Blue Tent", "Tenda Clássica"),
	0x72: (0x22CF, 2282, "Green Tent", "Tenda Clássica"),
	0x74: (0x22D0, 2283, "Three-Room Wood and Plaster House", "Casa Clássica"),
	0x75: (0x22D0, 2283, "Three-Room Wood and Plaster House", "Casa Clássica"),
	0x76: (0x22D1, 2284, "Two-Story Wood and Plaster House", "Casa Clássica"),
	0x77: (0x22D1, 2284, "Two-Story Wood and Plaster House", "Casa Clássica"),
	0x78: (0x22D2, 2285, "Two-Story Stone and Plaster House", "Casa Clássica"),
	0x79: (0x22D2, 2285, "Two-Story Stone and Plaster House", "Casa Clássica"),
	0x7A: (0x22D3, 2286, "Large Tower", "Torre e Castelo"),
	0x7B: (0x22D3, 2286, "Large Tower", "Torre e Castelo"),
	0x7C: (0x22D4, 2287, "Stone Keep", "Torre e Castelo"),
	0x7D: (0x22D4, 2287, "Stone Keep", "Torre e Castelo"),
	0x7E: (0x22D5, 2288, "Castle", "Torre e Castelo"),
	0x7F: (0x22D5, 2288, "Castle", "Torre e Castelo"),
	0x8D: (0x22D6, 2289, "Large Patio House", "Casa Clássica"),
	0x8E: (0x22D6, 2289, "Large Patio House", "Casa Clássica"),
	0x96: (0x22D8, 2290, "Large Marble Patio House", "Casa Clássica"),
	0x97: (0x22D8, 2290, "Large Marble Patio House", "Casa Clássica"),
	0x98: (0x22D9, 2295, "Small Tower", "Torre e Castelo"),
	0x99: (0x22D9, 2295, "Small Tower", "Torre e Castelo"),
	0x9A: (0x22DA, 2293, "Log Cabin", "Casa Clássica"),
	0x9B: (0x22DA, 2293, "Log Cabin", "Casa Clássica"),
	0x9C: (0x22DB, 2294, "Sandstone Patio House", "Casa Clássica"),
	0x9D: (0x22DB, 2294, "Sandstone Patio House", "Casa Clássica"),
	0x9E: (0x22D7, 2291, "Two-Story Villa", "Casa Clássica"),
	0x9F: (0x22D7, 2291, "Two-Story Villa", "Casa Clássica"),
	0xA0: (0x22DD, 2296, "Small Stone Workshop", "Oficina Clássica"),
	0xA1: (0x22DD, 2296, "Small Stone Workshop", "Oficina Clássica"),
	0xA2: (0x22DE, 2297, "Small Marble Workshop", "Oficina Clássica"),
	0xA3: (0x22DE, 2297, "Small Marble Workshop", "Oficina Clássica"),

	# Contest Castles & Keeps
	0x147C: (0x22D5, 2288, "Stone and Timber Castle", "Castelo Especial"),
	0x147D: (0x22D5, 2288, "Gothic Rose Castle", "Castelo Especial"),
	0x147E: (0x22D5, 2288, "Castle of Fire and Ice", "Castelo Especial"),
	0x147F: (0x22D5, 2288, "Castle of the Black Thorn", "Castelo Especial"),
	0x1480: (0x22D5, 2288, "The Fortress Castle", "Castelo Especial"),
	0x1481: (0x22D5, 2288, "Spires Castle", "Castelo Especial"),
	0x1482: (0x22D5, 2288, "Castle of Oceania", "Castelo Especial"),
	0x1483: (0x22D5, 2288, "Feudal Castle", "Castelo Especial"),
	0x1484: (0x22D4, 2287, "Robin's Nest Keep", "Fortaleza Especial"),
	0x1485: (0x22D4, 2287, "Traditional Keep", "Fortaleza Especial"),
	0x1486: (0x22D4, 2287, "Villa Crowley Keep", "Fortaleza Especial"),
	0x1487: (0x22D4, 2287, "Darkthorn Keep", "Fortaleza Especial"),
	0x1488: (0x22D4, 2287, "Sandalwood Keep", "Fortaleza Especial"),
	0x1489: (0x22D4, 2287, "Casa Moga Keep", "Fortaleza Especial"),
	0x148A: (0x22D5, 2288, "Robin's Roost Castle", "Castelo Especial"),
	0x148B: (0x22D5, 2288, "Camelot Castle", "Castelo Especial"),
	0x148C: (0x22D5, 2288, "Lacrimae In Caelo Castle", "Castelo Especial"),
	0x148D: (0x22D5, 2288, "Okinawa Sweet Dream Castle", "Castelo Especial"),
	0x148E: (0x22D5, 2288, "The Sandstone Castle", "Castelo Especial"),
	0x148F: (0x22D5, 2288, "Grimswind Sisters Castle", "Castelo Especial"),
}

HOUSE_BASEID_MAP = {
	"small_stone_and_plaster_house": 0x64,
	"small_fieldstone_house": 0x66,
	"small_brick_house": 0x68,
	"small_wood_house": 0x6A,
	"small_wood_and_plaster_house": 0x6C,
	"small_thatched_roof_cottage": 0x6E,
	"three_room_wood_and_plaster_house": 0x74,
	"two_story_wood_and_plaster_house": 0x76,
	"two_story_stone_and_plaster_house": 0x78,
	"large_tower": 0x7A,
	"stone_keep": 0x7C,
	"castle": 0x7E,
	"large_patio_house": 0x8D,
	"large_marble_patio_house": 0x96,
	"large_marbel_patio_house": 0x96,
	"small_tower": 0x98,
	"log_cabin": 0x9A,
	"sandstone_patio_house": 0x9C,
	"two_story_villa": 0x9E,
	"small_stone_workshop": 0xA0,
	"small_marble_workshop": 0xA2,
	"stone_and_timber_castle": 0x147C,
	"gothic_rose_castle": 0x147D,
	"castle_of_fire_and_ice": 0x147E,
	"castle_of_the_black_thorn": 0x147F,
	"fortress_castle": 0x1480,
	"spires_castle": 0x1481,
	"castle_of_oceania": 0x1482,
	"feudal_castle": 0x1483,
	"robins_nest_keep": 0x1484,
	"traditional_keep": 0x1485,
	"villa_crowley_keep": 0x1486,
	"darkthorn_keep": 0x1487,
	"sandalwood_keep": 0x1488,
	"casa_moga_keep": 0x1489,
	"robins_roost_castle": 0x148A,
	"camelot_castle": 0x148B,
	"lacrimae_in_caelo_castle": 0x148C,
	"okinawa_sweet_dream_castle": 0x148D,
	"sandstone_castle": 0x148E,
	"grimswind_sisters_castle": 0x148F,
}

def get_static_tile_info(tile_id):
	"""Returns (flags, height) from tiledata.mul for static tile."""
	global _STATIC_FLAGS_CACHE
	tile_id = int(tile_id) & 0xFFFF
	if tile_id in _STATIC_FLAGS_CACHE:
		return _STATIC_FLAGS_CACHE[tile_id]
	tiledata_path = os.path.join(get_mul_path(), 'tiledata.mul')
	if not os.path.exists(tiledata_path):
		return (0, 0)
	try:
		block = tile_id // 32
		entry = tile_id % 32
		# 7.0.9.0+ tiledata format: land block size 964, static block size 1316 (4 + 32*41)
		offset = 493568 + block * 1316 + 4 + entry * 41
		with open(tiledata_path, 'rb') as f:
			f.seek(offset)
			record = f.read(41)
			flags = struct.unpack('<Q', record[:8])[0]
			height = record[20]
			res = (flags, height)
			_STATIC_FLAGS_CACHE[tile_id] = res
			return res
	except Exception:
		return (0, 0)

def _load_multicollection_uop_index():
	global _MULTICOLLECTION_UOP_INDEX
	if _MULTICOLLECTION_UOP_INDEX is not None:
		return _MULTICOLLECTION_UOP_INDEX
	_MULTICOLLECTION_UOP_INDEX = {}
	base_path = get_mul_path()
	uop_file = os.path.join(base_path, 'MultiCollection.uop')
	if not os.path.exists(uop_file):
		return _MULTICOLLECTION_UOP_INDEX
	try:
		with open(uop_file, 'rb') as f:
			f.seek(40)
			file_count, next_block = struct.unpack('<IQ', f.read(12))
			while True:
				for _ in range(file_count):
					offset, hlen, c_len, d_len, hash_val, adler, c_type = struct.unpack('<QIIIQIH', f.read(34))
					if offset > 0 and c_len > 0:
						_MULTICOLLECTION_UOP_INDEX[hash_val] = (offset + hlen, c_len, d_len, c_type)
				if next_block == 0 or next_block >= os.path.getsize(uop_file):
					break
				f.seek(next_block)
				file_count, next_block = struct.unpack('<IQ', f.read(12))
	except Exception:
		pass
	return _MULTICOLLECTION_UOP_INDEX

def load_multi_components(multi_id):
	"""Loads multi components from multi.idx/multi.mul or MultiCollection.uop.
	Returns list of (tile, x, y, z).
	"""
	multi_id = int(multi_id) & 0x3FFF
	mul_path = get_mul_path()
	idx_path = os.path.join(mul_path, 'multi.idx')
	data_path = os.path.join(mul_path, 'multi.mul')

	candidates = [multi_id]
	if multi_id > 0 and (multi_id - 1) not in candidates:
		candidates.append(multi_id - 1)
	if (multi_id + 1) not in candidates:
		candidates.append(multi_id + 1)

	# 1. Try multi.idx / multi.mul
	if os.path.exists(idx_path) and os.path.exists(data_path):
		try:
			idx_size = os.path.getsize(idx_path)
			with open(idx_path, 'rb') as f_idx, open(data_path, 'rb') as f_mul:
				for mid in candidates:
					if mid * 12 + 12 > idx_size:
						continue
					f_idx.seek(mid * 12)
					rec = f_idx.read(12)
					if len(rec) < 12:
						continue
					start, length, extra = struct.unpack('<iii', rec)
					if start < 0 or length <= 0:
						continue
					f_mul.seek(start)
					data = f_mul.read(length)
					rec_size = 16 if (length % 16 == 0) else 12
					count = length // rec_size
					items = []
					for i in range(count):
						item_data = data[i * rec_size : (i + 1) * rec_size]
						tile, x, y, z = struct.unpack('<hhhh', item_data[:8])
						if tile > 1:
							items.append((tile, x, y, z))
					if items:
						return items
		except Exception:
			pass

	# 2. Try MultiCollection.uop
	uop_file = os.path.join(mul_path, 'MultiCollection.uop')
	if os.path.exists(uop_file):
		uop_idx = _load_multicollection_uop_index()
		for mid in candidates:
			pattern = f"build/multicollection/{mid:06d}.bin"
			h = hash_little2(pattern)
			if h in uop_idx:
				pos, clen, dlen, ctype = uop_idx[h]
				try:
					with open(uop_file, 'rb') as f:
						f.seek(pos)
						cdata = f.read(clen)
						ddata = zlib.decompress(cdata)
						if len(ddata) >= 8:
							unknown, count = struct.unpack('<II', ddata[:8])
							offset = 8
							items = []
							for _ in range(count):
								if offset + 14 > len(ddata):
									break
								tile, x, y, z, flagg, clilocsCount = struct.unpack('<hhhhhI', ddata[offset:offset+14])
								offset += 14 + clilocsCount * 4
								if tile > 1:
									items.append((tile, x, y, z))
							if items:
								return items
				except Exception:
					pass

	return []

def render_multi_to_image(items):
	"""Renders a list of (tile, x, y, z) into a high quality PIL RGBA Image."""
	if not items:
		return None

	decorated_items = []
	for it in items:
		tile, x, y, z = it[0], it[1], it[2], it[3]
		if tile <= 1:
			continue
		flags, height = get_static_tile_info(tile)
		is_surface = bool(flags & 0x200 or height == 0)
		is_roof = bool(flags & 0x10000000)
		layer_priority = 0 if is_surface else (2 if is_roof else 1)
		decorated_items.append((tile, x, y, z, layer_priority))

	if not decorated_items:
		return None

	# Isometric painter algorithm:
	# 1. x + y (north to south depth)
	# 2. z (elevation from floor to roof)
	# 3. layer_priority (surfaces before walls before roofs)
	# 4. x - y (west to east)
	decorated_items.sort(key=lambda it: (it[1] + it[2], it[3], it[4], it[1] - it[2]))

	coords = []
	loaded_tiles = []
	for tile, x, y, z, _ in decorated_items:
		img, _ = get_item_art(tile)
		if not img:
			continue
		w, h = img.size
		sx = (x - y) * 22
		sy = (x + y) * 22 - (z * 4)
		px = sx - (w // 2)
		py = sy - h + 44
		coords.append((px, py, px + w, py + h))
		loaded_tiles.append((img, px, py))

	if not loaded_tiles:
		return None

	min_x = min(c[0] for c in coords)
	min_y = min(c[1] for c in coords)
	max_x = max(c[2] for c in coords)
	max_y = max(c[3] for c in coords)

	pad = 8
	width = max_x - min_x + pad * 2
	height = max_y - min_y + pad * 2

	canvas = Image.new('RGBA', (width, height), (0, 0, 0, 0))
	for img, px, py in loaded_tiles:
		canvas.paste(img, (px - min_x + pad, py - min_y + pad), img)

	return canvas

def get_house_multi_image(multi_id, house_obj=None):
	"""Renders or retrieves cached isometric PNG for a house multi or custom house.
	Returns (Image, rel_path) or (None, None).
	"""
	web_dir = os.path.dirname(os.path.abspath(__file__))
	houses_dir = os.path.join(web_dir, "images", "houses")
	os.makedirs(houses_dir, exist_ok=True)

	# Check for customized house
	is_custom = False
	custom_tiles = None
	rev = 1
	if house_obj is not None and hasattr(house_obj, 'hastag'):
		try:
			if house_obj.hastag('ch_current'):
				tag_val = house_obj.gettag('ch_current')
				if tag_val:
					custom_tiles = json.loads(tag_val)
					is_custom = True
					if house_obj.hastag('ch_revision'):
						rev = int(house_obj.gettag('ch_revision'))
		except Exception:
			pass

	if is_custom and custom_tiles and hasattr(house_obj, 'serial'):
		cache_name = f"house_custom_{house_obj.serial}_{rev}.png"
		abs_p = os.path.join(houses_dir, cache_name)
		rel_p = f"images/houses/{cache_name}"
		if os.path.exists(abs_p):
			try:
				return Image.open(abs_p), rel_p
			except Exception:
				pass
		img = render_multi_to_image(custom_tiles)
		if img:
			try:
				img.save(abs_p)
				return img, rel_p
			except Exception:
				return img, rel_p

	# Standard multi
	if multi_id > 0:
		cache_name = f"house_0x{multi_id:04x}.png"
		abs_p = os.path.join(houses_dir, cache_name)
		rel_p = f"images/houses/{cache_name}"
		if os.path.exists(abs_p):
			try:
				return Image.open(abs_p), rel_p
			except Exception:
				pass

		comps = load_multi_components(multi_id)
		if comps:
			img = render_multi_to_image(comps)
			if img:
				try:
					img.save(abs_p)
					return img, rel_p
				except Exception:
					return img, rel_p

	return None, None

def get_house_image(house_or_id, prefer_gump=False):
	"""Retrieves artwork (isometric house render, item miniature or gump) and metadata for a house or foundation.
	Returns (Image, rel_path, title, category).
	"""
	multi_id = 0
	baseid_str = ""
	house_obj_name = ""
	house_obj = None

	if isinstance(house_or_id, (int, float)):
		val = int(house_or_id)
		multi_id = (val - 0x4000) if val >= 0x4000 else val
	elif isinstance(house_or_id, str):
		baseid_str = house_or_id.lower().strip()
		if baseid_str.startswith('0x'):
			try:
				val = int(baseid_str, 16)
				multi_id = (val - 0x4000) if val >= 0x4000 else val
			except Exception:
				pass
		elif baseid_str.isdigit():
			val = int(baseid_str)
			multi_id = (val - 0x4000) if val >= 0x4000 else val
	elif house_or_id is not None:
		house_obj = house_or_id
		# Wolfpack Item / Multi object
		if hasattr(house_or_id, 'baseid') and house_or_id.baseid:
			baseid_str = str(house_or_id.baseid).lower().strip()
		if hasattr(house_or_id, 'id'):
			try:
				val = int(house_or_id.id)
				multi_id = (val - 0x4000) if val >= 0x4000 else val
			except Exception:
				pass
		if hasattr(house_or_id, 'name') and house_or_id.name:
			house_obj_name = str(house_or_id.name).strip()

	if baseid_str in HOUSE_BASEID_MAP:
		multi_id = HOUSE_BASEID_MAP[baseid_str]

	# Check for Custom Foundations
	is_foundation_2 = False
	is_foundation_3 = False
	foundation_dimensions = ""

	m = re.search(r'foundation_([23])_(\d+)x(\d+)', baseid_str)
	if m:
		stories = int(m.group(1))
		w, h = m.group(2), m.group(3)
		foundation_dimensions = f"{w}x{h}"
		if stories == 3:
			is_foundation_3 = True
		else:
			is_foundation_2 = True
	elif (0x13EC <= multi_id <= 0x143A) or baseid_str.startswith('foundation_2'):
		is_foundation_2 = True
	elif (0x140B <= multi_id <= 0x147B) or baseid_str.startswith('foundation_3'):
		is_foundation_3 = True

	if is_foundation_3:
		dim_label = f" ({foundation_dimensions})" if foundation_dimensions else ""
		title = f"Fundação Customizável 3 Andares{dim_label}"
		category = "Fundação Custom"
		item_art_id = 0x22E3
		gump_id = 5528
	elif is_foundation_2:
		dim_label = f" ({foundation_dimensions})" if foundation_dimensions else ""
		title = f"Fundação Customizável 2 Andares{dim_label}"
		category = "Fundação Custom"
		item_art_id = 0x22E2
		gump_id = 5528
	elif multi_id in HOUSE_ART_MAP:
		item_art_id, gump_id, title, category = HOUSE_ART_MAP[multi_id]
	else:
		title = house_obj_name or (f"Multi 0x{multi_id:x}" if multi_id > 0 else "Propriedade")
		category = "Estrutura Residencial"
		item_art_id = 0x22C4
		gump_id = 2275

	# 1. Primary: Try full isometric multi render
	img, rel_path = get_house_multi_image(multi_id, house_obj)
	if img and rel_path:
		return (img, rel_path, title, category)

	# 2. Fallback: item tile art or gump
	if prefer_gump:
		img, rel_path = get_gump(gump_id)
		if not img:
			img, rel_path = get_item_art(item_art_id)
	else:
		img, rel_path = get_item_art(item_art_id)
		if not img:
			img, rel_path = get_gump(gump_id)

	if not rel_path:
		rel_path = "images/items/item_8900_0.png"

	return (img, rel_path, title, category)

