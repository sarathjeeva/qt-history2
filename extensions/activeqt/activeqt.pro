TEMPLATE	= subdirs
CONFIG		+= ordered

!contains( QT_PRODUCT, qt-enterprise ) {
    message( "ActiveQt requires a Qt/Enterprise license." )
}
contains( QT_PRODUCT, qt-enterprise ) {
    SUBDIRS	= idc \
		  container \
		  control \
		  examples
}
