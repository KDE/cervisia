project(cervisia)

add_subdirectory( cvsservice ) 
add_subdirectory( pics ) 

set(_KDECONFDIR_ ${CONFIG_INSTALL_DIR})
set(_KDEHTMLDIR_ ${HTML_INSTALL_DIR})

configure_file (cervisia.1.cmake ${CMAKE_CURRENT_BINARY_DIR}/cervisia.1 @ONLY)

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

qt4_add_dbus_interfaces( libcervisia_SRCS cvsservice/org.kde.cervisia.cvsservice.xml )
qt4_add_dbus_interfaces( libcervisia_SRCS cvsservice/org.kde.cervisia.cvsjob.xml )
qt4_add_dbus_interfaces( libcervisia_SRCS cvsservice/org.kde.cervisia.repository.xml )


kde4_add_ui3_files(libcervisia_SRCS settingsdlg_advanced.ui )

kde4_add_kcfg_files(libcervisia_SRCS cervisiasettings.kcfgc )


set(cervisia_KDEINIT_SRCS ${libcervisia_SRCS} 
   main.cpp 
   cervisiashell.cpp )


kde4_automoc(${cervisia_KDEINIT_SRCS})

kde4_add_kdeinit_executable( cervisia ${cervisia_KDEINIT_SRCS})

target_link_libraries(kdeinit_cervisia  ${KDE4_KPARTS_LIBS} ${KDE4_KUTILS_LIBS} )

install(TARGETS kdeinit_cervisia  DESTINATION ${LIB_INSTALL_DIR} )

target_link_libraries( cervisia kdeinit_cervisia )
install(TARGETS cervisia  DESTINATION ${BIN_INSTALL_DIR} )

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

qt4_add_dbus_adaptor(cervisiapart_PART_SRCS org.kde.cervisia.protocolview.xml protocolview.h ProtocolView)

kde4_add_plugin(cervisiapart WITH_PREFIX ${cervisiapart_PART_SRCS})



target_link_libraries(cervisiapart  ${KDE4_KPARTS_LIBS}  ${KDE4_KUTILS_LIBS} )

install(TARGETS cervisiapart  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### next target ###############


########### install files ###############

install( FILES cervisia.desktop  DESTINATION  ${XDG_APPS_DIR} )
install( FILES cervisiapart.kcfg  DESTINATION  ${KCFG_INSTALL_DIR} )
install( FILES cervisiaui.rc  DESTINATION  ${DATA_INSTALL_DIR}/cervisiapart )
install( FILES cervisiashellui.rc cervisia.notifyrc  DESTINATION  ${DATA_INSTALL_DIR}/cervisia )
install( FILES cervisia.upd  DESTINATION  ${KCONF_UPDATE_INSTALL_DIR} )
install(PROGRAMS change_colors.pl cervisia-normalize_cvsroot.pl cervisia-change_repos_list.pl DESTINATION  ${KCONF_UPDATE_INSTALL_DIR} )
install( FILES ${CMAKE_CURRENT_BINARY_DIR}/cervisia.1 DESTINATION ${MAN_INSTALL_DIR}/man1)
kde4_install_icons( ${ICON_INSTALL_DIR}   )



#cervisia.1.in: cervisia.pod
#	pod2man --center "Cervisia" --release "${CERVISIA_VERSION}" \
#		cervisia.pod > cervisia.1.in
