TEMPLATE = app
DEFINES += QDOC2_COMPAT QT_KEYWORDS
QCONFIG += compat
CONFIG  += debug
HEADERS += archiveextractor.h \
	   atom.h \
	   bookgenerator.h \
	   ccodeparser.h \
           codechunk.h \
           codemarker.h \
	   codeparser.h \
	   command.h \
           config.h \
	   cppcodemarker.h \
	   cppcodeparser.h \
	   cpptoqsconverter.h \
           doc.h \
	   editdistance.h \
	   generator.h \
	   htmlgenerator.h \
           location.h \
	   loutgenerator.h \
	   mangenerator.h \
           node.h \
           openedlist.h \
	   pagegenerator.h \
	   plaincodemarker.h \
	   polyarchiveextractor.h \
	   polyuncompressor.h \
	   qsakernelparser.h \
	   qscodemarker.h \
	   qscodeparser.h \
           quoter.h \
	   separator.h \
           set.h \
	   sgmlgenerator.h \
	   text.h \
	   tokenizer.h \
	   tr.h \
	   tree.h \
	   uncompressor.h
SOURCES += archiveextractor.cpp \
	   atom.cpp \
	   bookgenerator.cpp \
	   ccodeparser.cpp \
           codechunk.cpp \
           codemarker.cpp \
	   codeparser.cpp \
	   command.cpp \
           config.cpp \
	   cppcodemarker.cpp \
	   cppcodeparser.cpp \
	   cpptoqsconverter.cpp \
           doc.cpp \
	   editdistance.cpp \
	   generator.cpp \
	   htmlgenerator.cpp \
           location.cpp \
	   loutgenerator.cpp \
	   mangenerator.cpp \
           main.cpp \
           node.cpp \
           openedlist.cpp \
	   pagegenerator.cpp \
	   plaincodemarker.cpp \
	   polyarchiveextractor.cpp \
	   polyuncompressor.cpp \
	   qsakernelparser.cpp \
	   qscodemarker.cpp \
	   qscodeparser.cpp \
           quoter.cpp \
	   separator.cpp \
	   sgmlgenerator.cpp \
	   text.cpp \
	   tokenizer.cpp \
	   tree.cpp \
	   uncompressor.cpp \
	   $(QTDIR)/tools/designer/editor/yyindent.cpp
