CERVISIA_VERSION = 2.4.10
INCLUDES = -I./cvsservice -D_BSD_SOURCE $(all_includes)

SUBDIRS = cvsservice . pics

bin_PROGRAMS =
lib_LTLIBRARIES =
kdeinit_LTLIBRARIES = cervisia.la
kde_module_LTLIBRARIES = libcervisiapart.la
noinst_LTLIBRARIES = libcervisia.la

libcervisia_la_SOURCES = annotatedlg.cpp diffdlg.cpp patchoptiondlg.cpp logdlg.cpp \
		   progressdlg.cpp progressdlg.skel resolvedlg.cpp resolvedlg_p.cpp annotateview.cpp \
		   diffview.cpp loglist.cpp logplainview.cpp logtree.cpp annotatectl.cpp   \
		   loginfo.cpp misc.cpp qttableview.cpp tooltip.cpp cervisiasettings.kcfgc \
		   settingsdlg.cpp settingsdlg_advanced.ui
libcervisia_la_COMPILE_FIRST = cvsservice/cvsservice_stub.h cervisiasettings.h

libcervisiapart_la_SOURCES = updateview.cpp protocolview.cpp protocolview.skel \
		   watchdlg.cpp changelogdlg.cpp historydlg.cpp \
		   repositorydlg.cpp commitdlg.cpp checkoutdlg.cpp updatedlg.cpp \
		   tagdlg.cpp mergedlg.cpp cvsdir.cpp repositories.cpp cervisiapart.cpp \
		   addrepositorydlg.cpp addremovedlg.cpp watchersdlg.cpp \
		   updateview_items.cpp updateview_visitors.cpp entry.cpp \
		   entry_status.cpp stringmatcher.cpp cvsinitdlg.cpp ignorelistbase.cpp dirignorelist.cpp \
		   globalignorelist.cpp editwithmenu.cpp logmessageedit.cpp
libcervisiapart_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
libcervisiapart_la_LIBADD = $(LIB_KFILE) $(LIB_KPARTS) $(LIB_KUTILS) \
		   cvsservice/libcvsservice.la libcervisia.la
libcervisiapart_la_COMPILE_FIRST = cvsservice/cvsservice_stub.h cervisiasettings.h

cervisia_la_SOURCES = main.cpp cervisiashell.cpp
cervisia_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) -module $(KDE_PLUGIN)
cervisia_la_LIBADD = $(LIB_KPARTS) $(LIB_KUTILS) cvsservice/libcvsservice.la libcervisia.la
cervisia_la_COMPILE_FIRST = cvsservice/cvsservice_stub.h cervisiasettings.h

man_MANS = cervisia.1

METASOURCES = AUTO
KDE_ICON = AUTO

EXTRA_DIST = cervisia.desktop cervisia.png cervisia-small.png
CLEANFILES = cervisia.1

xdg_apps_DATA = cervisia.desktop

kde_kcfg_DATA = cervisiapart.kcfg

partrcdir = $(kde_datadir)/cervisiapart
partrc_DATA = cervisiaui.rc

shellrcdir = $(kde_datadir)/cervisia
shellrc_DATA = cervisiashellui.rc eventsrc

update_DATA = cervisia.upd
update_SCRIPTS = move_repositories.pl change_colors.pl cervisia-normalize_cvsroot.pl cervisia-change_repos_list.pl
updatedir = $(kde_datadir)/kconf_update

messages: rc.cpp
	$(EXTRACTRC) *.rc >> rc.cpp
	$(XGETTEXT) -C *.cpp *.h -o $(podir)/cervisia.pot

srcdoc:
	$(kde_bindir)/kdoc -a -p -d classdocs -n 'Cervisia' *.h -lqt -lkdecore -lkdeui -lkparts

cervisia.1: $(srcdir)/cervisia.1.in
	sed -e 's%_KDEHTMLDIR_%'${kde_htmldir}'%g;' \
	    -e 's%_KDECONFDIR_%'${kde_confdir}'%g;' \
	  < $(srcdir)/cervisia.1.in > cervisia.1

cervisia.1.in: cervisia.pod
	pod2man --center "Cervisia" --release "${CERVISIA_VERSION}" \
		cervisia.pod > cervisia.1.in
