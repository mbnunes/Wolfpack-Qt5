
# Wolfpack sqlite module

HEADERS += $$SQLITE_H/btree.h \
	$$SQLITE_H/config.h \
	$$SQLITE_H/hash.h \
	$$SQLITE_H/opcodes.h \
	$$SQLITE_H/os.h \
	$$SQLITE_H/pager.h \
	$$SQLITE_H/parse.h \
	$$SQLITE_H/sqlite.h \
	$$SQLITE_H/sqliteInt.h \
	$$SQLITE_H/vdbe.h \
	$$SQLITE_H/vdbeInt.h

SOURCES += $$SQLITE_CPP/attach.c \
	$$SQLITE_CPP/auth.c \
	$$SQLITE_CPP/btree.c \
	$$SQLITE_CPP/btree_rb.c \
	$$SQLITE_CPP/build.c \
	$$SQLITE_CPP/copy.c \
	$$SQLITE_CPP/date.c \
	$$SQLITE_CPP/delete.c \
	$$SQLITE_CPP/expr.c \
	$$SQLITE_CPP/func.c \
	$$SQLITE_CPP/hash.c \
	$$SQLITE_CPP/insert.c \
	$$SQLITE_CPP/main.c \
	$$SQLITE_CPP/opcodes.c \
	$$SQLITE_CPP/os.c \
	$$SQLITE_CPP/pager.c \
	$$SQLITE_CPP/parse.c \
	$$SQLITE_CPP/pragma.c \
	$$SQLITE_CPP/printf.c \
	$$SQLITE_CPP/random.c \
	$$SQLITE_CPP/select.c \
	$$SQLITE_CPP/table.c \
	$$SQLITE_CPP/tokenize.c \
	$$SQLITE_CPP/trigger.c \
	$$SQLITE_CPP/update.c \
	$$SQLITE_CPP/util.c \
	$$SQLITE_CPP/vacuum.c \
	$$SQLITE_CPP/vdbe.c \
	$$SQLITE_CPP/vdbeaux.c \
	$$SQLITE_CPP/where.c
