#!/usr/bin/env python
# -*- coding: utf-8 -*-
import re
import sys
if len(sys.argv) < 3:
    raise ValueError('Não tem argumentos suficientes')

path = sys.argv[1]
map = sys.argv[2]

def comment(text):
    return '\t<!--' + text + '-->'

def item(name, id, rest):
    match = re.match(r'.*Hue=(?P<hue>0x[0-9A-F]+).*', rest)
    if match:
        hue = ' hue="%s"' % (match.groupdict()['hue'],)
    else:
        hue = ''
    result = '\t<item id="%s"%s>' % (id, hue)
    return result
def pos(x, y, z, map):
	return '\t\t<pos x="%s" y="%s" z="%s" map="%s" />' % (x,y,z,map)

def roda_arquivo(nome_input, nome_output, map):
    input = open(nome_input)
    lines = input.readlines()

    result = []
    in_item = False
    for line in lines:
        # Comentário
        match = re.match(r'#(?P<comentario>.*)$', line)
        if match:
            result.append(comment(match.groupdict()['comentario'])) 
            continue

        #2051 933 -23
        match = re.match(r'(?P<x>-?[0-9]+) (?P<y>-?[0-9]+) (?P<z>-?[0-9]+)', line)
        if match:
            groups = match.groupdict()
            x = groups['x']
            y = groups['y']
            z = groups['z']
            result.append(pos(x, y, z, map))
            continue

        # Nome 0x1231 (qqrporra=XYZ)
        match = re.match(r'(?P<name>[A-Za-z][A-Za-z0-9]+) (?P<id>0x[0-9A-F]+)(?P<rest>.*)', line)
        if match:
            in_item = True
            groups = match.groupdict()
            name = groups['name']
            id = groups['id']
            rest = groups['rest']
            result.append(item(name, id, rest))
            continue

        match = re.match(r'\s*', line)
        if match:
            if in_item:
                result.append('\t</item>')
            result.append('')
            in_item = False
            continue

    output = open(nome_output, 'w')
    output.write("<decoration>\n")
    for l in result:
        output.write(l + "\n")
    output.write("</decoration>\n")
    output.close()

import os
files = os.listdir(path)
for file in files:
    match = re.match(r'_(?P<name>.*)\.cfg$', file)
    if match:
        name = match.groupdict()['name']
        input_name = '_%s.cfg' % (name,)
        output_name = '%s.xml' % (name,)
        roda_arquivo(input_name, output_name, map)

