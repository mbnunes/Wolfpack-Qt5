

#INCLUDEPATH += $$PWD

SOURCES +=	\
			$$PWD/wpmain.cpp \
			$$PWD/mainwindow.cpp \
			$$PWD/lineedithistory.cpp \
			$$PWD/profilersessionmodel.cpp \
			$$PWD/profilerwindow.cpp \
			$$PWD/pythoninteractivewindow.cpp
			
HEADERS +=	\
			$$PWD/mainwindow.h \
			$$PWD/lineedithistory.h \
			$$PWD/profilersessionmodel.h \
			$$PWD/profilerwindow.h \
			$$PWD/pythoninteractivewindow.h
			
			
FORMS +=	\
			$$PWD/mainwindow.ui \
			$$PWD/profilerwindow.ui

RESOURCES += $$PWD/gui.qrc

include ( $$PWD/trayicon/trayicon.pri )