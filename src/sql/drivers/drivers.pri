contains(sql-drivers, all ) {
    sql-driver +=   psql mysql odbc oci tds db2 sqlite ibase
}

contains(sql-drivers, psql) {
    HEADERS +=      drivers/psql/qsql_psql.h
    SOURCES +=      drivers/psql/qsql_psql.cpp

    unix:!contains( LIBS, .*pq.* ):LIBS *= -lpq

    win32 {
        !contains( LIBS, .*pq.* ):LIBS *= -lpq
        LIBS *= -lws2_32 -ladvapi32
    }
}

contains(sql-drivers, mysql) {
    HEADERS +=      drivers/mysql/qsql_mysql.h
    SOURCES +=      drivers/mysql/qsql_mysql.cpp

    unix:use_mysqlclient_r {
        !contains( LIBS, .*mysqlclient_r.* ):LIBS    *= -lmysqlclient_r
    } else:unix {
        !contains( LIBS, .*mysqlclient.* ):LIBS    *= -lmysqlclient
    }

    win32 {
        use_mysqlclient_r {
	    !contains( LIBS, .*mysqlclient_r.* ):LIBS    *= -lmysqlclient_r
        }
        else {
            !contains( LIBS, .*mysqlclient.* ):LIBS    *= -lmysqlclient
        }
    }
}

contains(sql-drivers, odbc) {
     HEADERS += drivers/odbc/qsql_odbc.h
     SOURCES += drivers/odbc/qsql_odbc.cpp

     mac:!contains( LIBS, .*odbc.* ):LIBS        *= -liodbc
     unix:!contains( LIBS, .*odbc.* ):LIBS        *= -lodbc

     win32 {
         !win32-borland:LIBS     *= -lodbc32
         win32-borland:LIBS      *= $(BCB)/lib/PSDK/odbc32.lib
     }
}

contains(sql-drivers, oci) {
    HEADERS += drivers/oci/qsql_oci.h
    SOURCES += drivers/oci/qsql_oci.cpp

    unix:!contains( LIBS, .*clnts.* ):LIBS += -lclntsh

    win32:LIBS += -loci
}

contains(sql-drivers, tds) {
    HEADERS += drivers/tds/qsql_tds.h
    SOURCES += drivers/tds/qsql_tds.cpp

    unix:LIBS += -L$SYBASE/lib -lsybdb

    win32 {
        !win32-borland:LIBS += -lNTWDBLIB
        win32-borland:LIBS += $(BCB)/lib/PSDK/NTWDBLIB.LIB
    }
}

contains(sql-drivers, db2) {
    HEADERS += drivers/db2/qsql_db2.h
    SOURCES += drivers/db2/qsql_db2.cpp
    
    unix:LIBS += -ldb2
    
    win32 {
        !win32-borland:LIBS += -ldb2cli
#        win32-borland:LIBS  += $(BCB)/lib/PSDK/db2cli.lib
    }
}

contains(sql-drivers, ibase) {
    HEADERS += drivers/ibase/qsql_ibase.h
    SOURCES += drivers/ibase/qsql_ibase.cpp
    
    unix:LIBS *= -lgds  
    
    win32 {
        !win32-borland:LIBS *= -lgds32_ms
        win32-borland:LIBS  += gds32.lib
    }
}

contains(sql-drivers, sqlite2) {
    HEADERS += drivers/sqlite2/qsql_sqlite2.h
    SOURCES += drivers/sqlite2/qsql_sqlite2.cpp
    !contains(LIBS, .*sqlite.*):LIBS *= -lsqlite
}

contains(sql-drivers, sqlite) {
    !contains( LIBS, .*sqlite3.* ) {
        INCLUDEPATH +=  ../3rdparty/sqlite

        SOURCES +=      ../3rdparty/sqlite/alter.c \
                        ../3rdparty/sqlite/analyze.c \
                        ../3rdparty/sqlite/attach.c \
                        ../3rdparty/sqlite/auth.c \
                        ../3rdparty/sqlite/btree.c \
                        ../3rdparty/sqlite/build.c \
                        ../3rdparty/sqlite/callback.c \
                        ../3rdparty/sqlite/date.c \
                        ../3rdparty/sqlite/delete.c \
                        ../3rdparty/sqlite/expr.c \
                        ../3rdparty/sqlite/func.c \
                        ../3rdparty/sqlite/hash.c \
                        ../3rdparty/sqlite/insert.c \
                        ../3rdparty/sqlite/legacy.c \
                        ../3rdparty/sqlite/main.c \
                        ../3rdparty/sqlite/opcodes.c \
                        ../3rdparty/sqlite/pager.c \
                        ../3rdparty/sqlite/parse.c \
                        ../3rdparty/sqlite/pragma.c \
                        ../3rdparty/sqlite/prepare.c \
                        ../3rdparty/sqlite/printf.c \
                        ../3rdparty/sqlite/random.c \
                        ../3rdparty/sqlite/select.c \
                        ../3rdparty/sqlite/table.c \
                        ../3rdparty/sqlite/tokenize.c \
                        ../3rdparty/sqlite/trigger.c \
                        ../3rdparty/sqlite/update.c \
                        ../3rdparty/sqlite/utf.c \
                        ../3rdparty/sqlite/util.c \
                        ../3rdparty/sqlite/vacuum.c \
                        ../3rdparty/sqlite/vdbeapi.c \
                        ../3rdparty/sqlite/vdbeaux.c \
                        ../3rdparty/sqlite/vdbe.c \
                        ../3rdparty/sqlite/vdbefifo.c \
                        ../3rdparty/sqlite/vdbemem.c \
                        ../3rdparty/sqlite/where.c
        unix:SOURCES += ../3rdparty/sqlite/os_unix.c
        win32:SOURCES +=  ../3rdparty/sqlite/os_win.c
    }

    HEADERS +=      drivers/sqlite/qsql_sqlite.h
    SOURCES +=      drivers/sqlite/qsql_sqlite.cpp
}
