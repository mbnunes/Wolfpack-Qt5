# Wolfpack network module

# Input
HEADERS += $$NETWORK_H/asyncnetio.h \
	$$NETWORK_H/encryption.h \
	$$NETWORK_H/listener.h \
	$$NETWORK_H/network.h \
	$$NETWORK_H/uopacket.h \
	$$NETWORK_H/uorxpackets.h \
	$$NETWORK_H/uosocket.h \
	$$NETWORK_H/uotxpackets.h

SOURCES += $$NETWORK_CPP/asyncnetio.cpp \
	$$NETWORK_CPP/encryption.cpp \
	$$NETWORK_CPP/listener.cpp \
	$$NETWORK_CPP/network.cpp \
	$$NETWORK_CPP/uopacket.cpp \
	$$NETWORK_CPP/uorxpackets.cpp \
	$$NETWORK_CPP/uosocket.cpp \
	$$NETWORK_CPP/uotxpackets.cpp 
