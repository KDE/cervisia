INCLUDES = $(all_includes)
LDFLAGS	= $(all_libraries) $(KDE_RPATH)
CXXFLAGS = -g -fpermissive -UQT_NO_COMPAT -D_XOPEN_SOURCE=500 -D_BSD_SOURCE

bin_PROGRAMS = cervisia
lib_LTLIBRARIES = libcervisia.la

libcervisia_la_SOURCES = updateview.cpp protocolview.cpp watchdlg.cpp \
		   settingsdlg.cpp changelogdlg.cpp historydlg.cpp repositorydlg.cpp \
		   repositorysettingsdlg.cpp commitdlg.cpp checkoutdlg.cpp updatedlg.cpp \
		   tagdlg.cpp mergedlg.cpp \
		   logdlg.cpp logtree.cpp loglist.cpp diffdlg.cpp resolvedlg.cpp diffview.cpp \
		   annotatedlg.cpp annotateview.cpp \
		   cvsprogressdlg.cpp tiplabel.cpp listview.cpp \
		   cvsdir.cpp repositories.cpp misc.cpp cervisiapart.cpp qttableview.cpp

noinst_HEADERS = updateview.h protocolview.h watchdlg.h \
		 settingsdlg.h changelogdlg.h historydlg.h repositorydlg.h  repositorysettingsdlg.h \
		 commitdlg.h checkoutdlg.h updatedlg.h tagdlg.h mergedlg.h \
		 logdlg.h logtree.h loglist.h diffdlg.h resolvedlg.h diffview.h \
		 annotatedlg.h annotateview.h \
		 cvsprogressdlg.h tiplabel.h listview.h \
		 cvsdir.h repositories.h misc.h qttableview.h

cervisia_SOURCES = main.cpp cervisiashell.cpp
cervisia_LDFLAGS = $(all_libraries) $(KDE_RPATH)
cervisia_LDADD = libcervisia.la

libcervisia_la_LDFLAGS = $(all_libraries) -version-info 1:0:0 -module -no-undefined
libcervisia_la_LIBADD = $(LIB_KFILE) $(LIB_KPARTS)

METASOURCES = AUTO

EXTRA_DIST = cervisia.desktop cervisia.png cervisia-small.png

apps_DATA = cervisia.desktop
appsdir = $(kde_appsdir)/Development

rcdir = $(kde_datadir)/cervisia
rc_DATA = cervisiaui.rc cervisiashellui.rc

#partdir = $(kde_datadir)/cervisia
#part_DATA = cervisiaui.rc cervisiashellui.rc

messages: rc.cpp
	$(EXTRACTRC) *.rc >> rc.cpp
	$(XGETTEXT) -C *.cpp *.h -o $(podir)/cervisia.pot

install-data-local:
	$(mkinstalldirs) $(DESTDIR)/$(kde_icondir)/hicolor/32x32/apps
	$(INSTALL_DATA) $(srcdir)/cervisia.png $(DESTDIR)/$(kde_icondir)/hicolor/32x32/apps/cervisia.png
	$(mkinstalldirs) $(DESTDIR)/$(kde_icondir)/hicolor/16x16/apps
	$(INSTALL_DATA) $(srcdir)/cervisia-small.png $(DESTDIR)/$(kde_icondir)/hicolor/16x16/apps/cervisia.png

srcdoc:
	$(kde_bindir)/kdoc -a -p -d classdocs -n 'Cervisia' *.h -lqt -lkdecore -lkdeui -lkparts

