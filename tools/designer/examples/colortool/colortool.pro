SOURCES	+= main.cpp
unix {
  UI_DIR = .ui
  MOC_DIR = .moc
  OBJECTS_DIR = .obj
}
FORMS	= mainform.ui \
	colornameform.ui \
	findform.ui \
	optionsform.ui
IMAGES	= images/filenew \
	images/fileopen \
	images/filesave \
	images/print \
	images/undo \
	images/redo \
	images/editcut \
	images/editcopy \
	images/editpaste \
	images/searchfind \
	images/tabwidget.png \
	images/table.png \
	images/iconview.png \
	images/richtextedit.png \
	images/widgetstack.png \
	images/editraise.png
TEMPLATE	=app
CONFIG	+= qt warn_on release thread
DBFILE	= colortool.db
LANGUAGE	= C++
