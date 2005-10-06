
#INCLUDE = 'C:/program files/Microsoft Visual Studio .NET 2003/Vc7/atlmfc/include'
#INCLUDE2 = 'C:/program files/Microsoft Visual Studio .NET 2003/Vc7/PlatformSDK/include'

import sys
import os

file = open('version.h', 'ru')
text = file.read().split("\n")
file.close()

text = text[0].strip()

# Increase version number
if not text.startswith('#define UOFILEVERSION'):
	sys.exit()

parts = text.split(' ')
(m, n, o, p) = parts[2].split(',')
	
p = int(p) + 1

file = open('version.h', 'w')
file.write("#define UOFILEVERSION 0,0,0,%u\n" % p)
#file.write("#define LZMA_FILEVERSION_STR \"0,0,0,%u\"\n" % p)
file.write("#define UOFILEVERSION_STR \"Wolfpack Client Build %u\"\n" % p)
file.close()

# Touch the resource file
os.utime("uoclient.rc", None)

#os.system('rc /dAFX_TARG_DEU /d_WIN32 /i"%s" /i"%s" /foDebug/lzma.res lzma.rc' % (INCLUDE, INCLUDE2))
#os.system('rc /dAFX_TARG_DEU /d_WIN32 /i"%s" /i"%s" /foRelease/lzma.res lzma.rc' % (INCLUDE, INCLUDE2))
