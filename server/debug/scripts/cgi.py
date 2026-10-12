"""
Compatibility shim for standard library 'cgi' module (removed in Python 3.13/3.14).
Provides FieldStorage and MiniFieldStorage for Wolfpack WebAdmin and CGI scripts.
"""

import os
import sys
import html
import urllib.parse

escape = html.escape

class MiniFieldStorage:
    def __init__(self, name, value):
        self.name = name
        self.value = value
        self.filename = None
        self.file = None
        self.type = None
        self.type_options = {}
        self.disposition = None
        self.disposition_options = {}
        self.headers = {}

    def __repr__(self):
        return f"MiniFieldStorage({self.name!r}, {self.value!r})"

    def __str__(self):
        return str(self.value)

    def __eq__(self, other):
        if isinstance(other, MiniFieldStorage):
            return self.value == other.value
        return self.value == other

MiniField = MiniFieldStorage


class FieldStorage:
    def __init__(self, fp=None, headers=None, outerboundary=b'',
                 environ=None, keep_blank_values=0, strict_parsing=0,
                 limit=None, encoding='utf-8', errors='replace',
                 max_num_fields=None, separator='&'):
        if environ is None:
            environ = os.environ
        self.environ = environ
        self.encoding = encoding
        self.errors = errors
        self.list = []
        self._dict = {}

        # 1. Parse QUERY_STRING if present
        qs = environ.get('QUERY_STRING', '')
        if qs:
            parsed_qs = urllib.parse.parse_qs(qs, keep_blank_values=True)
            for k, vals in parsed_qs.items():
                for v in vals:
                    item = MiniFieldStorage(k, v)
                    self.list.append(item)
                    self._dict.setdefault(k, []).append(item)

        # 2. Parse body for POST / PUT
        method = environ.get('REQUEST_METHOD', '').upper()
        if method in ('POST', 'PUT'):
            clen = environ.get('CONTENT_LENGTH')
            try:
                length = int(clen) if clen else -1
            except ValueError:
                length = -1

            ctype = environ.get('CONTENT_TYPE', '')
            # If application/x-www-form-urlencoded (or unspecified post)
            if 'application/x-www-form-urlencoded' in ctype or not ctype:
                body = ''
                if fp is not None:
                    raw = fp.read(length) if length >= 0 else fp.read()
                elif hasattr(sys.stdin, 'read'):
                    raw = sys.stdin.read(length) if length >= 0 else sys.stdin.read()
                else:
                    raw = ''

                if isinstance(raw, bytes):
                    body = raw.decode(encoding, errors)
                else:
                    body = raw

                if body:
                    parsed_body = urllib.parse.parse_qs(body, keep_blank_values=True)
                    for k, vals in parsed_body.items():
                        for v in vals:
                            item = MiniFieldStorage(k, v)
                            self.list.append(item)
                            self._dict.setdefault(k, []).append(item)

    def __contains__(self, key):
        return key in self._dict

    def has_key(self, key):
        return key in self._dict

    def keys(self):
        return list(self._dict.keys())

    def __iter__(self):
        return iter(self._dict)

    def __getitem__(self, key):
        if key not in self._dict:
            raise KeyError(key)
        items = self._dict[key]
        if len(items) == 1:
            return items[0]
        return items

    def getfirst(self, key, default=None):
        if key in self._dict and self._dict[key]:
            return self._dict[key][0].value
        return default

    def getlist(self, key):
        if key in self._dict:
            return [item.value for item in self._dict[key]]
        return []

    def getvalue(self, key, default=None):
        if key not in self._dict:
            return default
        items = self._dict[key]
        if len(items) == 1:
            return items[0].value
        return [item.value for item in items]
