
# Input
HEADERS += \ 
	$$PWD/maps.h \
	$$PWD/multiscache.h \
	$$PWD/tilecache.h

SOURCES += \ 
	$$PWD/maps.cpp \
	$$PWD/multiscache.cpp \
	$$PWD/tilecache.cpp

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD
#The following line was inserted by qt3to4
QT +=  qt3support 
