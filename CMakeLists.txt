kde4_header()

add_subdirectory( cvsservice ) 
add_subdirectory( pics ) 



########### next target ###############

set(libcervisia_SRCS
   annotatedlg.cpp
   diffdlg.cpp
   patchoptiondlg.cpp
   logdlg.cpp
   progressdlg.cpp
   resolvedlg.cpp
   resolvedlg_p.cpp
   annotateview.cpp
   diffview.cpp
   loglist.cpp
   logplainview.cpp
   logtree.cpp
   annotatectl.cpp
   loginfo.cpp
   misc.cpp
   qttableview.cpp
   tooltip.cpp
   settingsdlg.cpp
	)	

kde4_add_ui3_files(libcervisia_SRCS settingsdlg_advanced.ui )

kde4_add_dcop_skels(libcervisia_SRCS progressdlg.h )

kde4_add_kcfg_files(libcervisia_SRCS cervisiasettings.kcfgc )


set(cervisia_KDEINIT_SRCS ${libcervisia_SRCS} 
   main.cpp 
   cervisiashell.cpp )

kde4_add_dcop_stubs(cervisia_KDEINIT_SRCS
   ${CMAKE_SOURCE_DIR}/cervisia/cvsservice/cvsservice.h
   ${CMAKE_SOURCE_DIR}/cervisia/cvsservice/cvsjob.h
   ${CMAKE_SOURCE_DIR}/cervisia/cvsservice/repository.h )

kde4_automoc(${cervisia_KDEINIT_SRCS})

kde4_add_kdeinit_executable( cervisia ${cervisia_KDEINIT_SRCS})

target_link_libraries(kdeinit_cervisia  ${KDE4_KPARTS_LIBS} kutils cvsservice )

install_targets(${LIB_INSTALL_DIR} kdeinit_cervisia )

target_link_libraries( cervisia kdeinit_cervisia )
install_targets(/bin cervisia )

########### next target ###############

set(cervisiapart_PART_SRCS ${libcervisia_SRCS}
   updateview.cpp 
   protocolview.cpp 
   watchdlg.cpp 
   changelogdlg.cpp 
   historydlg.cpp 
   repositorydlg.cpp 
   commitdlg.cpp 
   checkoutdlg.cpp 
   updatedlg.cpp 
   tagdlg.cpp 
   mergedlg.cpp 
   cvsdir.cpp 
   repositories.cpp 
   cervisiapart.cpp 
   addrepositorydlg.cpp 
   addremovedlg.cpp 
   watchersdlg.cpp 
   updateview_items.cpp 
   updateview_visitors.cpp 
   entry.cpp 
   entry_status.cpp 
   stringmatcher.cpp 
   cvsinitdlg.cpp 
   ignorelistbase.cpp 
   dirignorelist.cpp 
   globalignorelist.cpp 
   editwithmenu.cpp 
   logmessageedit.cpp )

kde4_automoc(${cervisiapart_PART_SRCS})

kde4_add_dcop_skels(cervisiapart_PART_SRCS protocolview.h )

kde4_add_plugin(cervisiapart WITH_PREFIX ${cervisiapart_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} cervisiapart )

target_link_libraries(cervisiapart  ${KDE4_KPARTS_LIBS}  kutils cvsservice  )

install_targets(${PLUGIN_INSTALL_DIR} cervisiapart )


########### next target ###############


########### install files ###############

install_files( ${XDG_APPS_DIR} FILES cervisia.desktop )
install_files( ${KCFG_INSTALL_DIR} FILES cervisiapart.kcfg )
install_files( ${DATA_INSTALL_DIR}/cervisiapart FILES cervisiaui.rc )
install_files( ${DATA_INSTALL_DIR}/cervisia FILES cervisiashellui.rc eventsrc )
install_files( ${KCONF_UPDATE_INSTALL_DIR} FILES cervisia.upd )
install_files( ${KCONF_UPDATE_INSTALL_DIR} FILES change_colors.pl cervisia-normalize_cvsroot.pl cervisia-change_repos_list.pl)

kde4_install_icons( ${ICON_INSTALL_DIR}  hicolor )

kde4_footer()



#original Makefile.am contents follow:

#CERVISIA_VERSION = 2.2.89
#INCLUDES = -I./cvsservice -D_BSD_SOURCE $(all_includes)
#AM_CPPFLAGS = -DKDE3_SUPPORT
#
#SUBDIRS = cvsservice . pics
#
#bin_PROGRAMS =
#lib_LTLIBRARIES =
#kdeinit_LTLIBRARIES = cervisia.la
#kde_module_LTLIBRARIES = libcervisiapart.la
#noinst_LTLIBRARIES = libcervisia.la
#
#libcervisia_la_SOURCES = annotatedlg.cpp diffdlg.cpp patchoptiondlg.cpp logdlg.cpp \
#		   progressdlg.cpp progressdlg.skel resolvedlg.cpp resolvedlg_p.cpp annotateview.cpp \
#		   diffview.cpp loglist.cpp logplainview.cpp logtree.cpp annotatectl.cpp   \
#		   loginfo.cpp misc.cpp qttableview.cpp tooltip.cpp cervisiasettings.kcfgc \
#		   settingsdlg.cpp settingsdlg_advanced.ui
#libcervisia_la_COMPILE_FIRST = cvsservice/cvsservice_stub.h cervisiasettings.h
#
#libcervisiapart_la_SOURCES = updateview.cpp protocolview.cpp protocolview.skel \
#		   watchdlg.cpp changelogdlg.cpp historydlg.cpp \
#		   repositorydlg.cpp commitdlg.cpp checkoutdlg.cpp updatedlg.cpp \
#		   tagdlg.cpp mergedlg.cpp cvsdir.cpp repositories.cpp cervisiapart.cpp \
#		   addrepositorydlg.cpp addremovedlg.cpp watchersdlg.cpp \
#		   updateview_items.cpp updateview_visitors.cpp entry.cpp \
#		   entry_status.cpp stringmatcher.cpp cvsinitdlg.cpp ignorelistbase.cpp dirignorelist.cpp \
#		   globalignorelist.cpp editwithmenu.cpp logmessageedit.cpp
#libcervisiapart_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
#libcervisiapart_la_LIBADD = $(LIB_KFILE) $(LIB_KPARTS) $(LIB_KUTILS) \
#		   cvsservice/libcvsservice.la libcervisia.la
#libcervisiapart_la_COMPILE_FIRST = cvsservice/cvsservice_stub.h cervisiasettings.h
#
#cervisia_la_SOURCES = main.cpp cervisiashell.cpp
#cervisia_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) -module $(KDE_PLUGIN)
#cervisia_la_LIBADD = $(LIB_KPARTS) $(LIB_KUTILS) cvsservice/libcvsservice.la libcervisia.la
#cervisia_la_COMPILE_FIRST = cvsservice/cvsservice_stub.h cervisiasettings.h
#
#man_MANS = cervisia.1
#
#METASOURCES = AUTO
#KDE_ICON = AUTO
#
#EXTRA_DIST = cervisia.desktop cervisia.png cervisia-small.png
#CLEANFILES = cervisia.1
#
#xdg_apps_DATA = cervisia.desktop
#
#kde_kcfg_DATA = cervisiapart.kcfg
#
#partrcdir = $(kde_datadir)/cervisiapart
#partrc_DATA = cervisiaui.rc
#
#shellrcdir = $(kde_datadir)/cervisia
#shellrc_DATA = cervisiashellui.rc eventsrc
#
#update_DATA = cervisia.upd
#update_SCRIPTS = move_repositories.pl change_colors.pl cervisia-normalize_cvsroot.pl cervisia-change_repos_list.pl
#updatedir = $(kde_datadir)/kconf_update
#
#messages: rc.cpp
#	$(EXTRACTRC) *.rc >> rc.cpp
#	$(XGETTEXT) -C *.cpp *.h -o $(podir)/cervisia.pot
#
#srcdoc:
#	$(kde_bindir)/kdoc -a -p -d classdocs -n 'Cervisia' *.h -lqt -lkdecore -lkdeui -lkparts
#
#cervisia.1: $(srcdir)/cervisia.1.in
#	sed -e 's%_KDEHTMLDIR_%'${kde_htmldir}'%g;' \
#	    -e 's%_KDECONFDIR_%'${kde_confdir}'%g;' \
#	  < $(srcdir)/cervisia.1.in > cervisia.1
#
#cervisia.1.in: cervisia.pod
#	pod2man --center "Cervisia" --release "${CERVISIA_VERSION}" \
#		cervisia.pod > cervisia.1.in
