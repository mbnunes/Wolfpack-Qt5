

#INCLUDEPATH += $$PWD

SOURCES +=	\
			$$PWD/wpmain.cpp \
			$$PWD/mainwindow.cpp \
			$$PWD/profilersessionmodel.cpp \
			$$PWD/profilerwindow.cpp
			
HEADERS +=	\
			$$PWD/mainwindow.h \
			$$PWD/profilersessionmodel.h \
			$$PWD/profilerwindow.h
			
			
FORMS +=	\
			$$PWD/mainwindow.ui \
			$$PWD/profilerwindow.ui

RESOURCES += $$PWD/gui.qrc

include ( $$PWD/trayicon/trayicon.pri )