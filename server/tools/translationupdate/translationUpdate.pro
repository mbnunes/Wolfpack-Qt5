TEMPLATE	= app
CONFIG		+= qt warn_on console
HEADERS		= metatranslator.h \
		  proparser.h
SOURCES		= fetchtr.cpp \
		  main.cpp \
		  merge.cpp \
		  numberh.cpp \
		  sametexth.cpp \
		  metatranslator.cpp \
		  proparser.cpp

TARGET		= translationUpdate

