
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

SOURCES += $$SQLITE_CPP/sqlite_attach.c \
	$$SQLITE_CPP/sqlite_auth.c \
	$$SQLITE_CPP/sqlite_btree.c \
	$$SQLITE_CPP/sqlite_btree_rb.c \
	$$SQLITE_CPP/sqlite_build.c \
	$$SQLITE_CPP/sqlite_copy.c \
	$$SQLITE_CPP/sqlite_date.c \
	$$SQLITE_CPP/sqlite_delete.c \
	$$SQLITE_CPP/sqlite_expr.c \
	$$SQLITE_CPP/sqlite_func.c \
	$$SQLITE_CPP/sqlite_hash.c \
	$$SQLITE_CPP/sqlite_insert.c \
	$$SQLITE_CPP/sqlite_main.c \
	$$SQLITE_CPP/sqlite_opcodes.c \
	$$SQLITE_CPP/sqlite_os.c \
	$$SQLITE_CPP/sqlite_pager.c \
	$$SQLITE_CPP/sqlite_parse.c \
	$$SQLITE_CPP/sqlite_pragma.c \
	$$SQLITE_CPP/sqlite_printf.c \
	$$SQLITE_CPP/sqlite_random.c \
	$$SQLITE_CPP/sqlite_select.c \
	$$SQLITE_CPP/sqlite_table.c \
	$$SQLITE_CPP/sqlite_tokenize.c \
	$$SQLITE_CPP/sqlite_trigger.c \
	$$SQLITE_CPP/sqlite_update.c \
	$$SQLITE_CPP/sqlite_util.c \
	$$SQLITE_CPP/sqlite_vacuum.c \
	$$SQLITE_CPP/sqlite_vdbe.c \
	$$SQLITE_CPP/sqlite_vdbeaux.c \
	$$SQLITE_CPP/sqlite_where.c
