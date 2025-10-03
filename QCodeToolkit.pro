QT       += core gui svg sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17 qscintilla2

INCLUDEPATH += 3rdparty/include

INCLUDEPATH += gui types core util widgets popover
INCLUDEPATH += gui/mainwindow gui/tabs gui/noteeditor gui/custom
INCLUDEPATH += gui/setting gui/codeeditor gui/widgets gui/popover
include (3rdparty/qmarkdowntextedit/qmarkdowntextedit.pri)

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    core/databasemanager.cpp \
    core/filemanager.cpp \
    core/projectmanager.cpp \
    core/settingmanager.cpp \
    gui/codeeditor/codeeditor.cpp \
    gui/codeeditor/cpplanguagespec.cpp \
    gui/codeeditor/javalanguagespec.cpp \
    gui/codeeditor/javascriptlanguagespec.cpp \
    gui/codeeditor/languagespecfactory.cpp \
    gui/codeeditor/pythonlanguagespec.cpp \
    gui/codeeditor/rustlanguagespec.cpp \
    gui/custom/combobox.cpp \
    gui/custom/dialog.cpp \
    gui/custom/flowlayout.cpp \
    gui/custom/imagelabel.cpp \
    gui/custom/rotationbutton.cpp \
    gui/custom/spinbox.cpp \
    gui/custom/textbrowser.cpp \
    gui/custom/textedit.cpp \
    gui/mainwindow/framelesswindowbase.cpp \
    gui/mainwindow/mainwidget.cpp \
    gui/mainwindow/titlebar.cpp \
    gui/noteeditor/codeitemcell.cpp \
    gui/noteeditor/imageitemcell.cpp \
    gui/noteeditor/itemcell.cpp \
    gui/noteeditor/markdownitemcell.cpp \
    gui/noteeditor/noteeditor.cpp \
    gui/noteeditor/textitemcell.cpp \
    gui/popover/popoverwidget.cpp \
    gui/popover/tagspopover.cpp \
    gui/setting/appearancesetpage.cpp \
    gui/setting/codesetpage.cpp \
    gui/setting/editorsetpage.cpp \
    gui/setting/exportsetpage.cpp \
    gui/setting/generalsetpage.cpp \
    gui/setting/menusetpage.cpp \
    gui/setting/settingdialog.cpp \
    gui/setting/settingspage.cpp \
    gui/setting/shortcutsdialog.cpp \
    gui/tabs/dynamicnotetab.cpp \
    gui/tabs/fixednotetab.cpp \
    gui/tabs/hometab.cpp \
    gui/tabs/notetab.cpp \
    gui/tabs/tagonlynotetab.cpp \
    gui/widgets/fontcombobox.cpp \
    gui/widgets/logviewer.cpp \
    gui/widgets/menubar.cpp \
    gui/widgets/menutoolbutton.cpp \
    gui/widgets/searchbox.cpp \
    gui/widgets/tabwidget.cpp \
    gui/widgets/tagswidget.cpp \
    gui/widgets/treepanel.cpp \
    gui/widgets/treewidget.cpp \
    main.cpp \
    util/fontmanager.cpp \
    util/logmanager.cpp \
    util/metactk.cpp \
    util/sqldatabase.cpp \
    util/stylemanager.cpp

HEADERS += \
    core/databasemanager.h \
    core/filemanager.h \
    core/projectmanager.h \
    core/settingmanager.h \
    gui/codeeditor/codeeditor.h \
    gui/codeeditor/cpplanguagespec.h \
    gui/codeeditor/javalanguagespec.h \
    gui/codeeditor/javascriptlanguagespec.h \
    gui/codeeditor/languagespec.h \
    gui/codeeditor/languagespecfactory.h \
    gui/codeeditor/pythonlanguagespec.h \
    gui/codeeditor/rustlanguagespec.h \
    gui/custom/combobox.h \
    gui/custom/dialog.h \
    gui/custom/flowlayout.h \
    gui/custom/imagelabel.h \
    gui/custom/rotationbutton.h \
    gui/custom/spinbox.h \
    gui/custom/textbrowser.h \
    gui/custom/textedit.h \
    gui/mainwindow/framelesswindowbase.h \
    gui/mainwindow/mainwidget.h \
    gui/mainwindow/titlebar.h \
    gui/noteeditor/codeitemcell.h \
    gui/noteeditor/imageitemcell.h \
    gui/noteeditor/itemcell.h \
    gui/noteeditor/markdownitemcell.h \
    gui/noteeditor/noteeditor.h \
    gui/noteeditor/textitemcell.h \
    gui/popover/popoverwidget.h \
    gui/popover/tagspopover.h \
    gui/setting/appearancesetpage.h \
    gui/setting/codesetpage.h \
    gui/setting/editorsetpage.h \
    gui/setting/exportsetpage.h \
    gui/setting/generalsetpage.h \
    gui/setting/menusetpage.h \
    gui/setting/settingdialog.h \
    gui/setting/settingspage.h \
    gui/setting/shortcutsdialog.h \
    gui/tabs/dynamicnotetab.h \
    gui/tabs/fixednotetab.h \
    gui/tabs/hometab.h \
    gui/tabs/notetab.h \
    gui/tabs/tagonlynotetab.h \
    gui/widgets/fontcombobox.h \
    gui/widgets/logviewer.h \
    gui/widgets/menubar.h \
    gui/widgets/menutoolbutton.h \
    gui/widgets/searchbox.h \
    gui/widgets/tabwidget.h \
    gui/widgets/tagswidget.h \
    gui/widgets/treepanel.h \
    gui/widgets/treewidget.h \
    types/code_types.h \
    types/editor_config.h \
    types/settings_types.h \
    types/sql_table_types.h \ \
    util/fontmanager.h \
    util/logmanager.h \
    util/metactk.h \
    util/sqldatabase.h \
    util/stylemanager.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc

RC_FILE = app.rc
