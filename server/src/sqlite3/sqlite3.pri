
# Wolfpack sqlite module

HEADERS += $$SQLITE3_H/btree.h \
	$$SQLITE3_H/config.h \
	$$SQLITE3_H/hash.h \
	$$SQLITE3_H/keywordhash.h \
	$$SQLITE3_H/opcodes.h \
	$$SQLITE3_H/os.h \
	$$SQLITE3_H/os_common.h \
	$$SQLITE3_H/os_win.h \
	$$SQLITE3_H/os_unix.h \
	$$SQLITE3_H/pager.h \
	$$SQLITE3_H/parse.h \
	$$SQLITE3_H/sqlite3.h \
	$$SQLITE3_H/sqliteInt.h \
	$$SQLITE3_H/vdbe.h \
	$$SQLITE3_H/vdbeInt.h

SOURCES += $$SQLITE3_CPP/alter.c \
	$$SQLITE3_CPP/attach.c \
	$$SQLITE3_CPP/auth.c \
	$$SQLITE3_CPP/btree.c \
	$$SQLITE3_CPP/build.c \
	$$SQLITE3_CPP/date.c \
	$$SQLITE3_CPP/delete.c \
	$$SQLITE3_CPP/expr.c \
	$$SQLITE3_CPP/func.c \
	$$SQLITE3_CPP/hash.c \
	$$SQLITE3_CPP/insert.c \
	$$SQLITE3_CPP/legacy.c \
	$$SQLITE3_CPP/main.c \
	$$SQLITE3_CPP/opcodes.c \
	$$SQLITE3_CPP/os_win.c \
	$$SQLITE3_CPP/os_unix.c \
	$$SQLITE3_CPP/pager.c \
	$$SQLITE3_CPP/parse.c \
	$$SQLITE3_CPP/pragma.c \
	$$SQLITE3_CPP/printf.c \
	$$SQLITE3_CPP/random.c \
	$$SQLITE3_CPP/select.c \
	$$SQLITE3_CPP/table.c \
	$$SQLITE3_CPP/tokenize.c \
	$$SQLITE3_CPP/trigger.c \
	$$SQLITE3_CPP/update.c \
	$$SQLITE3_CPP/utf.c \
	$$SQLITE3_CPP/util.c \
	$$SQLITE3_CPP/vacuum.c \
	$$SQLITE3_CPP/vdbe.c \
	$$SQLITE3_CPP/vdbeapi.c \
	$$SQLITE3_CPP/vdbeaux.c \
	$$SQLITE3_CPP/vdbemem.c \
	$$SQLITE3_CPP/where.c
