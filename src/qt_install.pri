#always install the library
target.path=$$libs.path
INSTALLS += target

#mac framework
macx { 
    QtFramework = /Library/Frameworks/$${TARGET}.framework
    framework.path = $$QtFramework/Headers
    framework.commands += $$quote("cp -rf $$target.path/$(TARGET) $(INSTALL_ROOT)/$$QtFramework/$${TARGET}")
    framework.commands += $$quote("\n\t install_name_tool -id $$QtFramework/$$TARGET $(INSTALL_ROOT)/$$QtFramework/$${TARGET}")       
    for(lib, $$list(QtCore QtGui QtNetwork QtXml QtOpenGL QtSql Qt3Compat)) {
        framework.commands += $$quote("\n\t install_name_tool -change lib$${lib}.4.dylib /Library/Frameworks/$${lib}.framework/$$lib $(INSTALL_ROOT)/$$QtFramework/$${TARGET}")       
    }
    INSTALLS += framework
}

#headers
INSTALL_HEADERS =
HEADERS_PRI = $(QTDIR)/include/$$TARGET/install.pri
exists($$HEADERS_PRI) {
    INSTALL_HEADERS = $$fromfile($$HEADERS_PRI, HEADER_FILES)

    flat_headers.files = $$INSTALL_HEADERS
    flat_headers.path = $$headers.path/Qt
    INSTALLS += flat_headers

    targ_headers.files = $$INSTALL_HEADERS
    targ_headers.path = $$headers.path/$$TARGET
    targ_headers.commands = $$fromfile($$HEADERS_PRI, CLASSES_COPY)
    targ_headers.commands ~= s,PREFIXPATH,$$targ_headers.path,g
    INSTALLS += targ_headers
    contains(INSTALLS, framework) {
          framework.files += $$INSTALL_HEADERS
	  framework.commands += $$quote("\n\t") $$fromfile($$HEADERS_PRI, CLASSES_COPY)
	  framework.commands ~= s,PREFIXPATH,$$framework.path,g
    }
}

