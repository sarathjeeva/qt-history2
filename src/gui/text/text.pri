# Qt kernel module

HEADERS += \
	text/qfont.h \
	text/qfontdatabase.h \
	text/qfontengine_p.h \
	text/qfontinfo.h \
	text/qfontmetrics.h \
	text/qfontdata_p.h \
	text/qscriptengine_p.h \
	text/qtextengine_p.h \
	text/qfontengine_p.h \
	text/qtextlayout_p.h \
	text/qtextformat.h \
	text/qtextformat_p.h \
	text/qfragmentmap_p.h \
	text/qtextpiecetable_p.h \
	text/qtextdocument.h \
	text/qtexthtmlparser_p.h \
	text/qtextdocumentlayout_p.h \
	text/qtextcursor.h \
	text/qtextcursor_p.h \
	text/qtextlistmanager_p.h \
	text/qtexttablemanager_p.h \
	text/qtextdocumentfragment.h \
	text/qtextdocumentfragment_p.h \
	text/qtextobjectmanager_p.h \
	text/qtextimagehandler_p.h \
	text/qtexttable.h \
	text/qtextlist.h \
	text/qtextlist_p.h

SOURCES += \
	text/qfont.cpp \
	text/qfontdatabase.cpp \
	text/qscriptengine.cpp \
	text/qtextengine.cpp \
	text/qtextlayout.cpp \
	text/qtextformat.cpp \
	text/qfragmentmap.cpp \
	text/qtextpiecetable.cpp \
	text/qtextdocument.cpp \
	text/qtexthtmlparser.cpp \
	text/qtextdocumentlayout.cpp \
	text/qtextcursor.cpp \
	text/qtextlistmanager.cpp \
	text/qtexttablemanager.cpp \
	text/qtextdocumentfragment.cpp \
	text/qtextobjectmanager.cpp \
	text/qtextimagehandler.cpp \
	text/qtexttable.cpp \
	text/qtextlist.cpp


win32 {
	SOURCES += \
		text/qfont_win.cpp \
		text/qfontengine_win.cpp
}

wince-* {
	SOURCES -= text/qfontengine_win.cpp
	SOURCES += text/qfontengine_wce.cpp
}

unix:x11 {
	SOURCES += \
		text/qfont_x11.cpp \
		text/qfontengine_x11.cpp
}

!embedded:!x11:mac {
	SOURCES += \
		text/qfont_mac.cpp \
		text/qfontengine_mac.cpp
}

embedded {
	SOURCES += \
		text/qfont_qws.cpp \
		text/qfontengine_qws.cpp
}

