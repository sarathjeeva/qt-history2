EXTRAS = handlers tests docs
for(dir, EXTRAS) {
    exists(dir) {
        SUBDIRS += dir
    }
}

SOURCES = paintwidget_mac.cpp paintwidget_unix.cpp paintwidget_win.cpp
macx {
    SOURCES = $$find(SOURCES, "_mac")
}

CONFIG = release
CONFIG += $$OTHER_CONFIG_VALUES
CONFIG = $$unique(CONFIG)

CONFIG += debug
options = $$find(CONFIG, "debug") $$find(CONFIG, "release")
count(options, 2) {
    message(Both release and debug specified.)
}
