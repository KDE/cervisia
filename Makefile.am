CERVISIA_VERSION = 2.0
INCLUDES = $(all_includes)
LDFLAGS	= $(all_libraries) $(KDE_RPATH)
AM_CPPFLAGS = -UQT_NO_COMPAT -D_BSD_SOURCE

bin_PROGRAMS = cervisia
lib_LTLIBRARIES = libcervisia.la

libcervisia_la_SOURCES = updateview.cpp protocolview.cpp watchdlg.cpp \
		   settingsdlg.cpp changelogdlg.cpp historydlg.cpp repositorydlg.cpp \
		   commitdlg.cpp checkoutdlg.cpp updatedlg.cpp \
		   tagdlg.cpp mergedlg.cpp \
		   logdlg.cpp logtree.cpp loglist.cpp diffdlg.cpp resolvedlg.cpp diffview.cpp \
		   annotatedlg.cpp annotateview.cpp progressdlg.cpp progressdlg.skel \
		   cvsprogressdlg.cpp tiplabel.cpp listview.cpp \
		   cvsdir.cpp repositories.cpp misc.cpp cervisiapart.cpp qttableview.cpp \
		   addrepositorydlg.cpp
libcervisia_la_LDFLAGS = $(all_libraries) -version-info 1:0:0 -module -no-undefined
libcervisia_la_LIBADD = $(LIB_KFILE) $(LIB_KPARTS) $(LIB_DCOP)

cervisia_SOURCES = main.cpp cervisiashell.cpp
cervisia_LDFLAGS = $(all_libraries) $(KDE_RPATH)
cervisia_LDADD = libcervisia.la

man_MANS = cervisia.1

METASOURCES = AUTO

EXTRA_DIST = cervisia.desktop cervisia.png cervisia-small.png
CLEANFILES = cervisia.1

appsdir = $(kde_appsdir)/Development
apps_DATA = cervisia.desktop

partrcdir = $(kde_datadir)/cervisiapart
partrc_DATA = cervisiaui.rc 

shellrcdir = $(kde_datadir)/cervisia
shellrc_DATA = cervisiashellui.rc

update_DATA = cervisia.upd move_repositories.pl
updatedir = $(kde_datadir)/kconf_update

messages: rc.cpp
	$(EXTRACTRC) *.rc >> rc.cpp
	$(XGETTEXT) -C *.cpp *.h -o $(podir)/cervisia.pot

install-data-local:
	$(mkinstalldirs) $(DESTDIR)/$(kde_icondir)/crystalsvg/32x32/apps
	$(INSTALL_DATA) $(srcdir)/cervisia.png $(DESTDIR)/$(kde_icondir)/crystalsvg/32x32/apps/cervisia.png
	$(mkinstalldirs) $(DESTDIR)/$(kde_icondir)/crystalsvg/16x16/apps
	$(INSTALL_DATA) $(srcdir)/cervisia-small.png $(DESTDIR)/$(kde_icondir)/crystalsvg/16x16/apps/cervisia.png

srcdoc:
	$(kde_bindir)/kdoc -a -p -d classdocs -n 'Cervisia' *.h -lqt -lkdecore -lkdeui -lkparts

cervisia.1: $(srcdir)/cervisia.1.in
	sed -e 's%_KDEHTMLDIR_%'${kde_htmldir}'%g;' \
	    -e 's%_KDECONFDIR_%'${kde_confdir}'%g;' \
	  < $(srcdir)/cervisia.1.in > cervisia.1

cervisia.1.in: cervisia.pod
	pod2man --center "Cervisia" --release "${CERVISIA_VERSION}" \
		cervisia.pod > cervisia.1.in

SUBDIRS = cvsservice
