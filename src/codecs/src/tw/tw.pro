TEMPLATE        = lib
CONFIG          += qt warn_on plugin
REQUIRES	= dll !bigcodecs

HEADERS         = $(QTDIR)/src/codecs/qbig5codec.h \
		  $(QTDIR)/src/codecs/qfontcodecs_p.h

SOURCES         = $(QTDIR)/src/codecs/qbig5codec.cpp \
		  $(QTDIR)/src/codecs/qfonttwcodec.cpp \
		  main.cpp

TARGET          = qtwcodecs
DESTDIR         = ../../../../plugins/codecs

target.path=$$plugins.path/codecs
isEmpty(target.path):target.path=$$QT_PREFIX/plugins/codecs
INSTALLS += target

