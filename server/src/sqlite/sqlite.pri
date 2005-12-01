
# Wolfpack sqlite module

# Message: 'This function or variable may be unsafe. Consider using strcpy_s instead. 
# To disable deprecation, use _CRT_SECURE_NO_DEPRECATE. See online help for details.'
win32-msvc2005:DEFINES *= _CRT_SECURE_NO_DEPRECATE

INCLUDEPATH += $$PWD


HEADERS += \
	$$PWD/btree.h \
	$$PWD/config.h \
	$$PWD/hash.h \
	$$PWD/opcodes.h \
	$$PWD/os.h \
	$$PWD/pager.h \
	$$PWD/parse.h \
	$$PWD/sqlite.h \
	$$PWD/sqliteInt.h \
	$$PWD/vdbe.h \
	$$PWD/vdbeInt.h

SOURCES += \
	$$PWD/sqlite_attach.c \
	$$PWD/sqlite_auth.c \
	$$PWD/sqlite_btree.c \
	$$PWD/sqlite_btree_rb.c \
	$$PWD/sqlite_build.c \
	$$PWD/sqlite_copy.c \
	$$PWD/sqlite_date.c \
	$$PWD/sqlite_delete.c \
	$$PWD/sqlite_expr.c \
	$$PWD/sqlite_func.c \
	$$PWD/sqlite_hash.c \
	$$PWD/sqlite_insert.c \
	$$PWD/sqlite_main.c \
	$$PWD/sqlite_opcodes.c \
	$$PWD/sqlite_os.c \
	$$PWD/sqlite_pager.c \
	$$PWD/sqlite_parse.c \
	$$PWD/sqlite_pragma.c \
	$$PWD/sqlite_printf.c \
	$$PWD/sqlite_random.c \
	$$PWD/sqlite_select.c \
	$$PWD/sqlite_table.c \
	$$PWD/sqlite_tokenize.c \
	$$PWD/sqlite_trigger.c \
	$$PWD/sqlite_update.c \
	$$PWD/sqlite_util.c \
	$$PWD/sqlite_vacuum.c \
	$$PWD/sqlite_vdbe.c \
	$$PWD/sqlite_vdbeaux.c \
	$$PWD/sqlite_where.c
